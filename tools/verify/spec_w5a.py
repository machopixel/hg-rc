# Differential tests for round 5, letter a (see difftest.py and docs/checker.md).
#
# These functions call helpers of the DLL that are not converted yet. The helpers are handled in one of two ways:
#  * pure C library code (sprintf, sqrt, fmod, fabs, sin, atan, strstr, stricmp, the sine table lookup) runs
#    for real in the emulator, for the original and for the C version alike, with the Windows control word
#    0x27f (53-bit precision) like the real program;
#  * every other helper is replaced by a stub: a `ret` written over its first byte plus a code hook that appends
#    ('stub_<address>', arguments) to the checker's call log (which is compared between the original and the
#    C run, like the Windows calls) and answers with a value chosen by the generator. Some stubs also write
#    the outputs the real helper would write through its pointer arguments, so that the code behind them
#    is exercised. The stub address is the helper's own address, so the original and the C code reach the
#    same stub. Stack addresses are logged as 'stack'.
# The functions that call the converted Cd_* / Sim_100c476c / Sim_1007df6c ... use worlds modelled on the specs
# of the letters that converted those.
import math
import os
import struct
import sys
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG

SOURCES = [
    'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1001/w5a_misc.c', 'sim/unsorted_1002/w5a_misc.c',
    'sim/unsorted_1003/w5a_misc.c', 'sim/unsorted_1004/w5a_misc.c', 'sim/unsorted_1005/w5a_misc.c',
    'sim/unsorted_1006/w5a_math.c', 'sim/unsorted_1007/w5a_misc.c', 'sim/unsorted_1008/w5a_misc.c',
    'sim/unsorted_1009/w5a_misc.c', 'sim/unsorted_100a/w5a_misc.c', 'sim/unsorted_100b/w5a_misc.c',
    'sim/unsorted_100c/w5a_misc.c',
    '../tools/verify/w5a_test.c',
    # existing converted code the functions above call
    'sim/unsorted_1000/w2e_names.c', 'sim/unsorted_1000/w2i_misc.c', 'sim/unsorted_1000/w2a_fields.c',
    'sim/unsorted_1001/w2e_tables.c', 'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1001/w2e_names.c',
    'sim/core/w2a_timers.c', 'sim/unsorted_1003/w2e_misc.c', 'sim/util/ring.c', 'sim/unsorted_1002/w2e_object.c', 'sim/unsorted_100d/w2f_records.c',
    'sim/unsorted_1002/w2l_tables.c', 'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1002/w2i_state.c',
    'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1003/w2e_mono.c',
    'sim/unsorted_1004/state.c', 'sim/unsorted_1004/w2j_misc.c', 'sim/unsorted_1004/tables.c',
    'sim/unsorted_1007/w2e_state.c', 'sim/unsorted_1008/w3o_length.c',
    'sim/unsorted_100c/w3o_misc.c', 'sim/unsorted_100c/w2e_tables.c',
    'sim/unsorted_100f/w2e_string.c', 'sim/util/lookup.c', 'sim/util/record.c', 'sim/util/field.c',
    'sim/view/objlist.c', 'sim/win/w4a_heap.c', 'sim/win/w4u_cdaudio.c', 'sim/core/timer.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000
DRIVER = 0x00F00200      # the target of the indirect call of Sim_10029125 (a `ret` with a hook)
TRAMP = 0x00F00800

# helper address -> number of dword arguments
STUBS = {
    0x1000e737: 12, 0x10016117: 2, 0x100135d4: 5, 0x100529a2: 4, 0x1009e5d0: 0, 0x1001f790: 3,
    0x10029b1f: 1, 0x1002fba5: 7, 0x100345e0: 0, 0x100345fc: 0, 0x1004954e: 1, 0x10016de7: 2,
    0x10016632: 1, 0x1004deb3: 5, 0x100b7834: 2, 0x1006b52f: 5, 0x10075124: 1, 0x100348d8: 9,
    0x100f01cc: 2, 0x100fd7e0: 2, 0x100a0def: 1, 0x100a4f03: 1, 0x10098f8c: 5, 0x1009db70: 1,
    0x100ef727: 7, 0x100e86a1: 7, 0x10105b50: 3, 0x100aa64a: 1, 0x100b469d: 0, 0x1001403a: 2,
    0x100c4d63: 1, 0x10033eb3: 5, 0x100ff860: 3,
}
# The original passes the upper half of a pointer in the top half of these 16-bit arguments (0x10016de7 masks its
# argument with 0xffff in all six reads, 0x1001403a reads it with movsx word), so only the low half is compared.
MASK = {(0x10016de7, 1): 0xffff, (0x1001403a, 1): 0xffff}
ST = {}
HANDLERS = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def put(ctx, addr, fmt, *vals):
    out = b''
    for f, v in zip(fmt, vals):
        size = struct.calcsize('<' + f)
        out += (v & ((1 << (8 * size)) - 1)).to_bytes(size, 'little')
    ctx.m.uc.mem_write(addr, out)


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def rstr(ctx, maxlen, alpha=b'abcXYZ019 _.'):
    n = ctx.rng.choice([0, 1, 2, 3, 4, 5, 7, 8, 9, ctx.rng.randrange(0, maxlen + 1), ctx.rng.randrange(0, maxlen + 1)])
    return bytes(ctx.rng.choice(alpha) for _ in range(min(n, maxlen)))


def cstr(ctx, data, extra=24):
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rb(ctx, extra))


def hsh(n, salt=0):
    x = (ST['seed'] + n * 0x9E3779B1 + salt * 0x85EBCA6B) & 0xffffffff
    x ^= x >> 15
    x = (x * 0x2C1B3C6D) & 0xffffffff
    x ^= x >> 12
    x = (x * 0x297A2D39) & 0xffffffff
    x ^= x >> 15
    return x


def sane(ctx, spread):
    return ctx.rng.uniform(-spread, spread)


def norm_vec(ctx):
    v = [ctx.rng.uniform(-1, 1) for _ in range(3)]
    n = math.sqrt(sum(x * x for x in v)) or 1.0
    return [x / n for x in v]


# ---- stubs --------------------------------------------------------------------------------------
def _extra(m, uc, addr):
    """Values that are not arguments but must be proven too (logged with the call)."""
    if addr == 0x1004954e:
        p = r32(uc, 0x1017b8c4)
        return (r32(uc, 0x1015ebbc), bytes(uc.mem_read(p + 1, 1))[0] if p else 0)
    return ()


def _make_cb(m, addr, nargs):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        raw = [struct.unpack('<I', bytes(uc.mem_read(esp + 4 + 4 * k, 4)))[0] for k in range(nargs)]
        args = []
        for k, v in enumerate(raw):
            if (addr, k) in MASK:
                v &= MASK[(addr, k)]
            elif STACK_LO <= v <= STACK_HI:
                v = 'stack'
            args.append(v)
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % addr, tuple(args) + _extra(m, uc, addr)))
        h = HANDLERS.get(addr)
        if h is not None:
            val = h(m, raw, n)
        else:
            rets = ST['rets'].get(addr) or [0]
            val = rets[n % len(rets)]
        uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
    return cb


def _driver_cb(m):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        n = len(m.api_log)
        m.api_log.append(('driver', (r32(uc, esp + 4),)))
        rets = ST['driver_rets']
        uc.reg_write(UC_X86_REG_EAX, rets[n % len(rets)] & 0xffffffff)
    return cb


def tramp_float(target, nargs):
    """Calls target(args as dwords) and stores st(0) as a float at OUTF."""
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\xd9\x1d' + struct.pack('<I', OUTF)       # fstp dword [OUTF]
    code += b'\x31\xc0\x5d\xc3'
    return code


# Handlers of stubs that write what the real helper would write.
def h_100135d4(m, raw, n):
    """(obj, key, -3, &word, &word): two 16-bit outputs."""
    h = hsh(n)
    m.uc.mem_write(raw[3], struct.pack('<H', [0, 1, 0xffff, 0x8000, 0x7fff, h & 0xffff, (h >> 16) & 0xffff][h % 7]))
    m.uc.mem_write(raw[4], struct.pack('<H', [0, 2, 0xfffe, 0x8001, 0x1234, (h >> 8) & 0xffff, (h >> 3) & 0xffff][(h >> 5) % 7]))
    return h >> 7


def h_100529a2(m, raw, n):
    """(index, &a, &c, &b): three dword outputs; fails now and then."""
    h = hsh(n)
    if h % 5 == 0:
        return 0
    for k in range(3):
        m.uc.mem_write(raw[1 + k], struct.pack('<I', hsh(n, k + 1) & (0xffffffff if k == 0 else 0xfff)))
    return 1


def h_10033eb3(m, raw, n):
    """(a, b, c, 28000.0f, &out): a count; sometimes writes a pointer to out."""
    h = hsh(n)
    if h % 3:
        m.uc.mem_write(raw[4], struct.pack('<I', 0x109e3e40 if (h >> 4) % 4 else 0))
    return [0, 1, 3, 2][(h >> 8) % 4]


def h_10105b50(m, raw, n):
    """(handle, buffer, count): the number of bytes read; one call of the trial may come back short."""
    if n == ST['short_at']:
        return [0, (raw[2] - 1) & 0xffffffff, (raw[2] + 1) & 0xffffffff, 0xffffffff][hsh(n) % 4]
    return raw[2]


def h_1002fba5(m, raw, n):
    """(obj, six pointers): fills the six floats with two ordered pairs (limits)."""
    h = hsh(n)
    f = []
    for k in range(3):
        lo = ((hsh(n, k + 1) % 2000) - 1000) / 10.0
        hi = lo + (hsh(n, k + 9) % 1000) / 10.0
        f += [lo, hi]
    for k in range(6):
        m.uc.mem_write(raw[1 + k], struct.pack('<f', f[k]))
    return h


HANDLERS.update({0x100135d4: h_100135d4, 0x100529a2: h_100529a2, 0x10033eb3: h_10033eb3,
                 0x10105b50: h_10105b50, 0x1002fba5: h_1002fba5})


def begin(ctx):
    """Installs the stubs (once per machine) and chooses the answers of this trial."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w5a', False):
        m._w5a = True
        for addr, nargs in STUBS.items():
            m.raw.mem_write(addr, b'\xc3')
            m.raw.hook_add(UC_HOOK_CODE, _make_cb(m, addr, nargs), None, addr, addr)
        m.raw.mem_write(DRIVER, b'\xc3')
        m.raw.hook_add(UC_HOOK_CODE, _driver_cb(m), None, DRIVER, DRIVER)
        m.raw.mem_write(TRAMP, tramp_float(0x100c34b7, 2))
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['seed'] = r.getrandbits(32)
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}
    ST['driver_rets'] = [r.choice([0, 1, 2, r.getrandbits(32)]) for _ in range(2)]
    ST['short_at'] = r.choice([-1, -1, -1, 0, 1, 2, 3, 5])
    ST['blk'] = 0


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def api_heapalloc(n, a):
    return ST['blk']


API = {'HeapAlloc': api_heapalloc}


# ---- shared worlds ------------------------------------------------------------------------------
def mono(ctx):
    """Maps the text screen (the original writes to physical 0xb0000) and makes it part of the compared memory."""
    m = ctx.m
    if not getattr(m, 'mono_mapped', False):
        m.uc.mem_map(0xb0000, 0x10000)
        m.wr.append((0xb0000, 0x10000))
        m.mono_mapped = True
        m.snap0 = m.dump()
    wb(ctx, 0xb0000, bytes([0xaa]) * 0x10000)
    w32(ctx, 0x10a3ade8, ctx.rng.choice([0, 1, 1, 1, 0x100]))


def page0(ctx):
    """Maps page 0 (a null pointer is then writable) and makes it part of the compared memory."""
    m = ctx.m
    if not getattr(m, 'page0_mapped', False):
        m.uc.mem_map(0, 0x1000)
        m.wr.append((0, 0x1000))
        m.page0_mapped = True
        m.snap0 = m.dump()
    wb(ctx, 0, bytes([0x55]) * 0x1000)


def ptrtab_world(ctx, n=8, size=0x400, names=True):
    """Pointer table 0x109e8b40 with n records, the count 0x1015dc64 and the current index 0x1017b8c8."""
    r = ctx.rng
    recs = []
    for i in range(n):
        rec = newbuf(ctx, size)
        if names:
            nm = rstr(ctx, 30, b'abcXYZ019 _.')
            wb(ctx, rec + 0x124, nm + b'\0')
        recs.append(rec)
    wb(ctx, 0x109e8b40, struct.pack('<%dI' % n, *recs))
    cur = r.randrange(0, n)
    w32(ctx, 0x1015dc64, n)
    w32(ctx, 0x1017b8c8, cur)
    return recs, cur


def name_table(ctx, extra=()):
    """The table of names at 0x10a39200 (as in spec_w2e.py); some entries are the names in `extra`."""
    rg = ctx.rng
    n = rg.choice([0, 1, 2, 3, 5, 8, 11])
    w32(ctx, 0x10a38de0, n)
    for i in range(0, 12):
        nm = rstr(ctx, 6, b'abc')
        if extra and rg.random() < 0.3:
            nm = rg.choice(extra)
        used = rg.choice([0, 1, 1, 2, rg.getrandbits(32)])
        ctx.m.uc.mem_write(0x10a39200 + i * 0x14, struct.pack('<I', used) + nm + b'\0' + rb(ctx, 15 - len(nm)))


def sine_table(ctx):
    """The 513 floats at 0x103719a0 (entry i = sin(i * pi / 1024)), as Trig_InitTables makes them."""
    wb(ctx, 0x103719a0, struct.pack('<513f', *[math.sin(i * 0.003067961572265625) for i in range(513)]))


def chain(ctx, n, off_next, size=0x20):
    """A singly linked list of n nodes (pointer to the next at off_next); returns the head (0 for n = 0)."""
    head = 0
    for _ in range(n):
        node = newbuf(ctx, size)
        put(ctx, node + off_next, 'I', head)
        head = node
    return head


def ftree(ctx, depth, mk):
    """mk(ctx, kids) -> node; builds a binary tree of the given depth (children are built by this function)."""
    r = ctx.rng
    kids = []
    if depth > 0:
        for _ in range(2):
            kids.append(ftree(ctx, depth - 1, mk) if r.random() < 0.6 else 0)
    else:
        kids = [0, 0]
    return mk(ctx, kids)


def nary(ctx, depth, mk):
    """Tree with the first child at +4 and the next sibling at +8; mk(ctx) makes a node buffer."""
    r = ctx.rng
    node = mk(ctx)
    first = 0
    if depth > 0:
        for _ in range(r.choice([0, 1, 2, 3])):
            kid = nary(ctx, depth - 1, mk)
            put(ctx, kid + 8, 'I', first)
            first = kid
    put(ctx, node + 4, 'I', first)
    return node


# ---- generators ---------------------------------------------------------------------------------
def g_7c45(ctx):
    begin(ctx)
    mono(ctx)
    w32(ctx, 0x1015d028, ctx.u32())
    return []


def g_ebfb(ctx):
    begin(ctx)
    r = ctx.rng
    name_table(ctx, extra=(b'MECHUDON', b'AFXHDZMI'))
    w32(ctx, 0x101321b4, ctx.u32())
    for i in range(16):
        w32(ctx, 0x1012df30 + 4 * i, ctx.u32())
    k = r.random()
    if k < 0.1:
        name = 0
    elif k < 0.2:
        name = cstr(ctx, b'')
    else:
        name = cstr(ctx, rstr(ctx, 6, b'abc') if r.random() < 0.5 else r.choice([b'MECHUDON', b'AFXHDZMI', b'abc']))
    return [name, ctx.u32(), ctx.u32(), r.choice([0, 1, 2, 5, 9, 15, -1, -3, 16, 17, 40]) & 0xffffffff, ctx.u32()]


def g_ed77(ctx):
    begin(ctx)
    r = ctx.rng
    sine_table(ctx)
    cam = zbuf(ctx, 0x40)
    wf(ctx, cam + 0xc, r.choice([0.0, 1.0, 90.0, -90.0, 180.0, 360.0, 720.0, sane(ctx, 400), sane(ctx, 30), sane(ctx, 2000)]))
    w32(ctx, 0x1014cae0, cam)
    v = r.choice([0.0, 1.0, 45.0, 90.0, -90.0, 180.0, 359.9, sane(ctx, 400), sane(ctx, 30), 1e-9, sane(ctx, 2000)])
    return [struct.unpack('<I', struct.pack('<f', v))[0]]


def g_state(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.randrange(2, 9)
    grp = r.randrange(0, 3)
    recs = []
    cur = r.randrange(0, n)
    for i in range(n):
        rec = zbuf(ctx, 0x200)
        sub = zbuf(ctx, 0x600)
        put(ctx, sub + 0x4f0, 'I', r.choice([2, 2, 2, 2, 0, 1, 3, r.getrandbits(32)]))
        put(ctx, rec + 8, 'I', grp if (i == cur or r.random() < 0.8) else r.randrange(0, 3))
        put(ctx, rec + 0x10, 'I', i + 1 if r.random() < 0.85 else r.randrange(0, 12))
        put(ctx, rec + 0x28, 'I', sub)
        recs.append(rec)
    put(ctx, recs[cur] + 8, 'I', grp)
    wb(ctx, 0x109e8b40, struct.pack('<%dI' % n, *recs))
    w32(ctx, 0x1015dc64, n if r.random() < 0.9 else r.randrange(0, n + 1))
    w32(ctx, 0x1017b8c8, cur)
    for i in range(-4, 14):
        w32(ctx, 0x1012ff58 + 4 * i, ctx.u32())
    idx = r.choice([r.randrange(1, n + 1), r.randrange(1, n + 1), r.randrange(1, n + 1), 7, 8, 8, 0, -1, -3, 9, 12, r.randrange(-5, 12)])
    return [idx & 0xffffffff, ctx.u32()]


def g_12a72(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x200)
    put(ctx, obj + 0x1a6, 'H', r.choice([2, 2, 2, 0, 1, 3, 0xffff]))
    put(ctx, obj + 0x1dc, 'I', r.choice([0, 1, ctx.u32()]))
    if r.random() < 0.3:
        key = r.choice([0, 5, 0x1ff, 0xfff, r.getrandbits(12), 0x800])
    else:
        key = r.choice([0x1000, 0x2201, 0x8000, 0xffff, 0x4000, 0xf000, r.getrandbits(16) | 0x1000, r.getrandbits(16)])
    if r.random() < 0.5:
        key |= r.getrandbits(16) << 16
    return [obj, key]


def g_15a63(ctx):
    begin(ctx)
    r = ctx.rng
    recs, cur = ptrtab_world(ctx, 8, 0x400, names=False)
    k = r.randrange(0, 8)
    obj = newbuf(ctx, 0x300)
    put(ctx, obj + 4, 'I', k)
    put(ctx, obj + 0x1d6, 'I', r.choice([0, 0x1000, 0x1001, r.randrange(0, 8), ctx.u32()]))
    cnt = r.choice([-1, 0, 1, 2, 3, 6, 0x20, 0x7e, 0x7f, 0x80, 0x90])
    w32(ctx, 0x1017e5a8, cnt)
    n = max(0, min(cnt, 0x80)) + 2
    for i in range(n):
        e = 0x109a8640 + i * 0x54
        wb(ctx, e, rb(ctx, 0x54))
        put(ctx, e + 0x24, 'H', r.choice([1, 0x401, 0, 2, r.getrandbits(16)]))
        put(ctx, e + 8, 'I', (k | 0x200) if r.random() < 0.5 else ctx.u32())
    for i in range(8):
        put(ctx, recs[i] + 0x1a8, 'H', r.choice([0, 0x100, 0x101, 0x103, r.getrandbits(16)]))
    return [obj]


def g_1f3ac(ctx):
    begin(ctx)
    r = ctx.rng
    first, last = W4['cd_env'](ctx)
    w32(ctx, 0x1015d1c4, r.choice([0, 0x100, 0x169, 0x16a, 0x200, 0xffffffff, ctx.u32()]))
    w32(ctx, 0x1015dc80, r.choice([0, 1, 0x10, 0x69, 0x100, 0x16a, 0xfffffffb, ctx.u32()]))
    w32(ctx, 0x1015d1bc, r.choice([0, 0, 0, 1, 2]))
    w32(ctx, 0x101321c0, r.choice([0, 8, 8, 8, 0xf, 0xffffffff, ctx.u32()]))
    w32(ctx, 0x101321a8, r.choice([0xffffffff, first, last, first + 1, 0, 5, ctx.u32()]) & 0xffffffff)
    rets(0x1009e5d0, *[r.choice([0, 1, 1, 7]) for _ in range(3)])
    return []


def g_203cf(ctx):
    begin(ctx)
    r = ctx.rng
    recs, cur = ptrtab_world(ctx, 8, 0x100, names=False)
    sub = newbuf(ctx, 0x700)
    put(ctx, sub + 0x613, 'I', r.choice([0, 1, 2, 3, ctx.u32()]))
    put(ctx, recs[cur] + 0x28, 'I', sub)
    b = r.choice([-1, -1, 0, 1, 2, 5, 9, r.randrange(-3, 30)])
    return [r.randrange(-3, 30) & 0xffffffff, b & 0xffffffff]


def g_29125(ctx):
    begin(ctx)
    r = ctx.rng
    objs = [r.randrange(1, 0x90) for _ in range(6)]
    for j in range(0x80):
        e = 0x109eeb04 + j * 0x30
        wb(ctx, e, rb(ctx, 0x30))
        if r.random() < 0.1:
            put(ctx, e, 'I', r.choice(objs))
    for i in range(10):
        e = 0x109f09e0 + i * 0x14
        wb(ctx, e, rb(ctx, 0x14))
        put(ctx, e + 4, 'I', r.choice([0, 0] + objs))
        put(ctx, e + 0x10, 'I', DRIVER if r.random() < 0.75 else 0)
    wb(ctx, 0x1015dc48, rb(ctx, 20))
    return [r.choice([0, 1, 2, 3, 5, 9, r.randrange(0, 10)])]


def blockbuf(ctx, nullp=0.1):
    """A block of six floats (limits) for Sim_1002d1c0; sometimes missing."""
    r = ctx.rng
    if r.random() < nullp:
        return 0
    blk = zbuf(ctx, 0x40)
    f = []
    for _ in range(3):
        lo = r.uniform(-50, 10)
        f += [lo, lo + r.uniform(0, 60)]
    wb(ctx, blk, struct.pack('<6f', *f))
    return blk


def g_fa3f(ctx, limits=False):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x200)
    flags = r.choice([0, 0, 0x200, 0x2ff, 0x1ff, ctx.u32() & 0xffff])
    put(ctx, obj, 'H', flags)
    put(ctx, obj + 0x98, 'I', blockbuf(ctx, 0.5))
    ST['blk'] = zbuf(ctx, 0x40) if r.random() < 0.9 else 0
    return [0 if r.random() < 0.05 else obj]


def g_d1c0(ctx):
    r = ctx.rng
    a = g_fa3f(ctx)
    obj = a[0] or newbuf(ctx, 0x200)
    lim = [r.choice([-60.0, -10.0, 0.0, 5.0, 30.0, 70.0, sane(ctx, 80)]) for _ in range(2)]
    bits = [struct.unpack('<I', struct.pack('<f', v))[0] for v in lim]
    return [obj, bits[0], r.getrandbits(32), bits[1]]


def g_d1c0_inrange(ctx):
    """Like g_d1c0 but the object is set up so that the block exists (the float limits can then be hit)."""
    r = ctx.rng
    begin(ctx)
    obj = newbuf(ctx, 0x200)
    put(ctx, obj, 'H', r.choice([0x200, 0x200, 0x3ff]))
    blk = blockbuf(ctx, 0.1)
    put(ctx, obj + 0x98, 'I', blk)
    f = struct.unpack('<6f', bytes(ctx.m.uc.mem_read(blk, 24))) if blk else (0,) * 6

    def pick(lo, hi):
        return r.choice([lo, hi, (lo + hi) / 2, (lo + hi) / 2, r.uniform(lo, hi), r.uniform(lo, hi), r.uniform(lo, hi), lo - 0.001, hi + 0.001, lo - 5, hi + 5, float('nan')])
    a = pick(f[0], f[1])
    b = pick(f[4], f[5])
    fb = lambda v: struct.unpack('<I', struct.pack('<f', v))[0]
    return [obj, fb(a), r.getrandbits(32), fb(b)]


def g_feef(ctx):
    begin(ctx)
    r = ctx.rng
    mode = r.choice([0, 0, 5, 5, 8, 8, 1, 3, r.getrandbits(32)])
    obj = newbuf(ctx, 0x200)
    put(ctx, obj + 0x34, 'I', mode)
    if mode == 5:
        blk = tree31917(ctx, 3)
    elif mode == 8:
        blk = owner305f8(ctx)
    else:
        blk = newbuf(ctx, 0x40)
    if r.random() < 0.08:
        blk = 0
    put(ctx, obj + 0x98, 'I', blk)
    return [0 if r.random() < 0.04 else obj]


def owner305f8(ctx):
    r = ctx.rng
    owner = zbuf(ctx, 0x24 + 7 * 0xe0 + 0x40)
    for i in range(7):
        for j in range(7):
            if r.random() < 0.35:
                put(ctx, owner + 0x24 + i * 0xe0 + j * 32 + 0x1c, 'I', chain(ctx, r.choice([1, 1, 2, 3]), 0, 8))
    return owner


def tree31917(ctx, depth):
    r = ctx.rng
    node = zbuf(ctx, 0x40)
    put(ctx, node + 0x18, 'I', r.choice([0, 0, 0, 1, 5]))
    for k in range(4):
        if depth > 0 and r.random() < 0.5:
            put(ctx, node + 0x1c + 4 * k, 'I', tree31917(ctx, depth - 1))
    return node


def g_31917(ctx):
    begin(ctx)
    return [0 if ctx.rng.random() < 0.05 else tree31917(ctx, ctx.rng.choice([0, 1, 2, 3]))]


def g_3462f(ctx):
    begin(ctx)
    r = ctx.rng
    mono(ctx)
    n = r.choice([0, 1, 5, 0x4f, 0x50, 0x51, 0x60, r.randrange(0, 0x70)])
    data = bytes(r.choice(b'abc xyz0123') if r.random() < 0.8 else r.randrange(1, 256) for _ in range(n))
    return [cstr(ctx, data, 8)]


def g_3ce00(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x40)
    o10 = r.choice([r.randrange(0, 0x80), ctx.u32()])
    put(ctx, obj + 0x10, 'I', o10)
    d = r.choice([0xd, 0xd, r.randrange(0x31, 0x60), ctx.u32()])
    w32(ctx, 0x1015dc18, d)
    c = r.choice([o10, o10, (d - 0x31) & 0xffffffff, (d - 0x31) & 0xffffffff, ctx.u32(), r.randrange(0, 0x80)])
    w32(ctx, 0x1017b8c4, newbuf(ctx, 8))
    w32(ctx, 0x1015ebbc, ctx.u32())
    w32(ctx, 0x1015dc14, chain(ctx, r.choice([0, 1, 2, 3]), 0x14))
    w32(ctx, 0x1015dc10, ctx.u32())
    return [ctx.u32(), ctx.u32(), c, ctx.u32(), ctx.u32(), 0 if r.random() < 0.1 else obj]


def label_world(ctx):
    """The slots of Sim_1007df6c (two of 0x3c bytes at 0x10149d28) and the label pointer 0x109b98c0."""
    r = ctx.rng
    for i in range(2):
        slot = 0x10149d28 + i * 0x3c
        wb(ctx, slot, rb(ctx, 0x3c))
        put(ctx, slot, 'I', newbuf(ctx, 0x120))
        put(ctx, slot + 0xc, 'I', r.choice([0, 1, 1]))
        put(ctx, slot + 0x10, 'I', r.choice([0, 5, 0x20, 0x40, 0x50, 0x299, 0x29a, 0x300, 0xffffffff]))
        put(ctx, slot + 0x34, 'I', r.choice([0, 100, 1000, ctx.u32()]))
    w32(ctx, 0x109b98c0, cstr(ctx, rstr(ctx, 12, b'abc xyz'), 24))


def g_ff72(ctx):
    begin(ctx)
    r = ctx.rng
    ptrtab_world(ctx, 8, 0x400)
    name_table(ctx, extra=(b'MECHUDON',))
    label_world(ctx)
    obj = newbuf(ctx, 0x80)
    wb(ctx, obj + 2, rstr(ctx, 30, b'abcXYZ019 _.') + b'\0')
    return [obj, r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, r.randrange(0, 8)])]


def g_40a37(ctx):
    begin(ctx)
    r = ctx.rng
    p = newbuf(ctx, 8)
    put(ctx, p + 3, 'H', r.choice([0, 0, 7, 7, 1, 0xffff, 0x100, r.getrandbits(16)]))
    return [p]


def g_41c4c(ctx):
    begin(ctx)
    r = ctx.rng
    recs, cur = ptrtab_world(ctx, 8, 0x400)
    label_world(ctx)
    w32(ctx, 0x101c49f8, cstr(ctx, rstr(ctx, 20, b'abcXYZ019 _.')))
    w32(ctx, 0x101655ac, ctx.u32())
    w32(ctx, 0x1015ec74, r.choice([0, 0, 1, 2, ctx.u32()]))
    idx = r.choice([cur, r.randrange(0, 8), r.randrange(0, 8)])
    return [ctx.u32(), idx]


def g_44909(ctx):
    begin(ctx)
    r = ctx.rng
    name_table(ctx, extra=(b'AFXHDZMI',))
    cam = zbuf(ctx, 0x40)
    cur = r.choice([1.0, 2.0, 0.5, 0.0, -1.0, sane(ctx, 5), float('nan')])
    wf(ctx, cam + 0x18, cur)
    w32(ctx, 0x1014cae0, cam)
    f1 = r.choice([cur, cur, 1.0, 2.0, sane(ctx, 5), float('inf')])
    f2 = r.choice([cur, 1.0, 2.0, sane(ctx, 5), 0.0])
    wf(ctx, 0x1013aecc, f1)
    wf(ctx, 0x1013aed0, f2)
    w32(ctx, 0x1015ef04, r.choice([0, 0, 1, 2, 3, ctx.u32()]))
    w32(ctx, 0x1013a014, ctx.u32())
    return [r.choice([0, 0, 1, 1, 2, 0xffffffff])]


def g_44c7a(ctx):
    begin(ctx)
    r = ctx.rng
    name_table(ctx, extra=(b'AFXHDZMI',))
    cam = newbuf(ctx, 0x40)
    wf(ctx, cam + 0x18, r.choice([1.0, 2.0, 0.5, sane(ctx, 5)]))
    w32(ctx, 0x1014cae0, cam)
    w32(ctx, 0x1013aed4, r.choice([3, 3, 0, 1, 2, 4]))
    w32(ctx, 0x1013aed8, r.choice([0, 1, 2, 3, 2]))
    w32(ctx, 0x1013aed0, ctx.u32())
    wf(ctx, 0x1013aecc, r.choice([1.0, 2.0, sane(ctx, 5)]))
    w32(ctx, 0x1015ef04, r.choice([0, 1, 2, 3]))
    w32(ctx, 0x10179cd4, r.choice([0, 0, 1]))
    w32(ctx, 0x1013aee0, r.choice([0xffffffff, 1, 2, 3]))
    wb(ctx, 0x109b9a20, rb(ctx, 0x20))
    wb(ctx, 0x1015dbd8, rb(ctx, 0x20))
    put(ctx, 0x1015dbd8 + 0x18, 'I', r.choice([0, 1, 5]))
    sel = r.choice([-1, -1, 0, 1, 2, 3])
    w32(ctx, 0x10134890, sel)
    for i in range(4):
        wb(ctx, 0x1096fcc0 + i * 0x156, rb(ctx, 0x156))
        put(ctx, 0x1096fcc0 + i * 0x156 + 0x20, 'I', r.choice([0, 1, 0, 7]))
    return []


def g_4ed37(ctx):
    begin(ctx)
    r = ctx.rng
    p = newbuf(ctx, 0x20)
    x0 = r.randrange(-100, 100)
    y0 = r.randrange(-100, 100)
    put(ctx, p + 4, 'I', x0)
    put(ctx, p + 8, 'I', y0)
    put(ctx, p + 0xc, 'I', x0 + r.randrange(-3, 300))
    put(ctx, p + 0x10, 'I', y0 + r.randrange(-3, 300))
    w32(ctx, 0x1013b9a8, r.choice([1, 1, 1, 0, 2, 3, ctx.u32()]))
    w32(ctx, 0x109b8be0, ctx.u32())
    return [0 if r.random() < 0.06 else p]


def g_579ad(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x101668a4, r.choice([0, 0, 0, 1, ctx.u32()]))
    w32(ctx, 0x1017d594, ctx.u32())
    a = newbuf(ctx, 0x7d * 4 + 16)
    b = newbuf(ctx, 0x7d * 4 + 16)
    return [0 if r.random() < 0.2 else a, 0 if r.random() < 0.2 else b]


def g_5ae79(ctx):
    begin(ctx)
    r = ctx.rng

    def mk(ctx_, kids):
        node = zbuf(ctx_, 0xa0)
        put(ctx_, node + 4, 'I', kids[0])
        put(ctx_, node + 8, 'I', kids[1])
        put(ctx_, node + 0x90, 'I', cstr(ctx_, rstr(ctx_, 6, b'abc'), 12))
        return node
    root = ftree(ctx, r.choice([0, 1, 2, 3]), mk)
    key = cstr(ctx, rstr(ctx, 3, b'abc'), 12)
    return [0 if r.random() < 0.05 else root, key]


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def g_60882(ctx):
    begin(ctx)
    r = ctx.rng
    vals = []
    for _ in range(3):
        vals.append(r.choice([0.0, 1.0, -1.0, 0.5, sane(ctx, 100), sane(ctx, 1), sane(ctx, 1e6), 1e-3, -0.0, sane(ctx, 1e18)]))
    if r.random() < 0.1:
        vals = [0.0, 0.0, 0.0]
    ptrs = []
    for v in vals:
        p = ctx.buf(8)
        wf(ctx, p, v)
        ptrs.append(p)
    return ptrs


def g_610ae(ctx):
    begin(ctx)
    r = ctx.rng
    sine_table(ctx)
    page0(ctx)
    out = ctx.buf(0x20)
    ang = r.choice([0.0, 90.0, -90.0, 180.0, 360.0, 720.0, sane(ctx, 720), sane(ctx, 720), sane(ctx, 30), 1e-6])
    return [fbits(sane(ctx, 10)), fbits(sane(ctx, 10)), fbits(sane(ctx, 10)), fbits(ang), 0 if r.random() < 0.1 else out]


def g_68c48(ctx):
    begin(ctx)
    r = ctx.rng
    sine_table(ctx)
    outs = [ctx.buf(8), ctx.buf(8), ctx.buf(8)]
    if r.random() < 0.15:
        outs[r.randrange(0, 3)] = 0
    ang = r.choice([0.0, 0.5, -0.5, 1e-8, -1e-8, 1e-7, 90.0, -90.0, 180.0, 360.0, sane(ctx, 720), sane(ctx, 720), sane(ctx, 5)])
    return [fbits(sane(ctx, 10)), ctx.u32(), fbits(sane(ctx, 10)), fbits(ang)] + outs


def g_69f68(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x600)
    inner = newbuf(ctx, 0x200)
    put(ctx, obj, 'I', inner)
    put(ctx, obj + 0x50c, 'I', r.choice([0, 1, 1, 2, ctx.u32()]))
    wf(ctx, inner + 0x68, r.choice([0.0, 360.0, -360.0, 359.5, sane(ctx, 720), sane(ctx, 720), sane(ctx, 5)]))
    wf(ctx, obj + 0xc, r.choice([0.0, 360.0, -360.0, sane(ctx, 720), sane(ctx, 720), sane(ctx, 5)]))
    return [obj]


def g_6ab98(ctx):
    begin(ctx)
    r = ctx.rng
    recs = zbuf(ctx, 0x3c * 8 + 16)
    for i in range(8):
        put(ctx, recs + i * 0x3c + 0x34, 'H', r.choice([0, 1, 2, 3, 4, 0xffff, 0x8000]))
    b = newbuf(ctx, 0x100)
    put(ctx, b + 0x88, 'I', recs)
    return [ctx.u32(), b, r.randrange(1, 8)]


def g_74c8f(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1015dc14, chain(ctx, r.choice([0, 0, 1, 2, 3, 5]), 0x14))
    w32(ctx, 0x1015dc10, ctx.u32())
    w32(ctx, 0x1017b8f8, ctx.u32() | 0x10000)
    return []


def g_7d8ba(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    w32(ctx, 0x101c49d0, cstr(ctx, rstr(ctx, 0x30, b'abcXYZ019 _.'), 8))
    return [ctx.u32(), obj]


def g_8349a(ctx):
    begin(ctx)
    r = ctx.rng
    vals = [r.choice([0.0, 1.0, -1.0, 0.5, sane(ctx, 100), sane(ctx, 1), sane(ctx, 1e6), 1e-3, -0.0, sane(ctx, 1e20)]) for _ in range(12)]
    p = ctx.buf(64)
    wf(ctx, p, *vals)
    return [p]


def g_846e0(ctx):
    begin(ctx)
    wb(ctx, 0x103719a0, rb(ctx, 0x804))
    wb(ctx, 0x103721c0, rb(ctx, 0x804))
    w32(ctx, 0x103721a0, ctx.u32())
    w32(ctx, 0x103729c0, ctx.u32())
    return []


def g_8799c(ctx):
    begin(ctx)
    r = ctx.rng
    data = bytes(r.choice([0, 1, 2, 0x7f, 0x80, 0xff, r.getrandbits(8), r.getrandbits(8)]) for _ in range(768))
    return [ctx.buf(768 + 8, data + rb(ctx, 8))]


def g_88c1c(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1017b9a0, chain(ctx, r.choice([0, 1, 2, 5]), 0x18, 0x30))
    w32(ctx, 0x1017b998, chain(ctx, r.choice([0, 1, 2, 3, 7]), 0x10, 0x30))
    return [ctx.u32()]


def g_8fce3(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1014cae0, ctx.u32())
    w32(ctx, 0x1017c8c4, r.choice([0, 1, 1]))
    w32(ctx, 0x1017c8cc, r.choice([0, ctx.u32()]))
    w32(ctx, 0x1017c8c8, r.choice([0, newbuf(ctx, 0x20), newbuf(ctx, 0x20)]))
    nodes = [newbuf(ctx, 0x20) for _ in range(5)]
    for n in nodes:
        put(ctx, n + 8, 'I', r.choice([0, 1, 1, 2]))
    pc = r.choice([0.0, 0.1, 0.4])
    for i in range(9):
        for j in range(9):
            w32(ctx, 0x1017ba4c + i * 0x18c + j * 0x2c, r.choice(nodes) if r.random() < pc else 0)
    wb(ctx, 0x1036f2a0, rb(ctx, 0x200))
    return [r.choice([0, 1, 2, 3, ctx.u32()])]


def g_98f30(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1014caf8, r.choice([0, 1, 1, 2, 3, ctx.u32()]))
    a = r.choice([0, 3, 0, 3, 1, 2, 4, 0xf, r.getrandbits(32)])
    a = (a & 0xf) | (r.getrandbits(32) & ~0xf if r.random() < 0.7 else 0)
    return [a, ctx.u32(), ctx.u32(), ctx.u32()]


def g_9d8b9(ctx):
    begin(ctx)
    r = ctx.rng
    q = newbuf(ctx, 0x80)
    put(ctx, q + 4, 'I', r.choice([-3, -1, 0, 1, 5, 0x19, 0x1a, 0x20, r.randrange(-3, 0x1c)]))
    p = ctx.buf(8)
    put(ctx, p, 'I', 0 if r.random() < 0.08 else q)
    return [0 if r.random() < 0.05 else p]


def g_a2b8c(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1017b940, r.choice([0, 1, 1, ctx.u32()]))
    w32(ctx, 0x101cc2bc, ctx.u32())
    w32(ctx, 0x101ccc6c, ctx.u32())
    wb(ctx, 0x101659e0, rb(ctx, 0x400))
    return [ctx.u32(), ctx.u32(), r.choice([0, 1, 5, 0x40, 0x1ff, r.randrange(0, 0x200)])]


def g_a796c(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0, 1, 0x3fff, 0x4000, 0x4001, 0x8000, 0x8001, 0xc000, 0x12345, r.randrange(0, 0x30000)])
    return [ctx.u32(), ctx.buf(64) if r.random() < 0.8 else ctx.u32(), n]


def mk_node(ctx):
    r = ctx.rng
    node = zbuf(ctx, 0xb0)
    if r.random() < 0.7:
        side = newbuf(ctx, 0x40)
        put(ctx, node + 0x8c, 'I', side)
        put(ctx, side + 0x26, 'H', r.choice([0, 1, 2, 5, 0x7f, 0xffff]))
    put(ctx, node + 0x94, 'I', r.choice([0, 1, 1, 2, 3, 0x80000000, ctx.u32()]))
    return node


def g_aa5e8(ctx):
    begin(ctx)
    r = ctx.rng
    root = nary(ctx, r.choice([0, 1, 2, 3]), mk_node)
    return [root, r.choice([0, 1, 0xffff, 0x10000, 0xf0f0, ctx.u32()])]


def g_aa742(ctx):
    begin(ctx)
    r = ctx.rng
    return [nary(ctx, r.choice([0, 1, 2, 3]), mk_node)]


def g_aacd5(ctx):
    begin(ctx)
    r = ctx.rng
    root = nary(ctx, r.choice([0, 1, 2, 3]), mk_node)
    return [root, r.choice([0, 1, 2, 5, 0x7f, 0xffff, 0x10001, ctx.u32()])]


def g_b4342(ctx):
    begin(ctx)
    r = ctx.rng
    cnt = r.choice([0, 1, 2, 5, 8, 10])
    w32(ctx, 0x1017d218, cnt)
    wb(ctx, 0x101a2100, rb(ctx, 0x88 * 14))
    a = r.randrange(-2, cnt + 2)
    b = r.choice([-1, -1, 0, 1, 2, cnt - 1, cnt, cnt + 1, r.randrange(-3, cnt + 2)])
    c = r.choice([0, 1, -1, 0x7fff, 0x8000, 0xffff, ctx.u32()])
    bb = b & 0xffffffff
    if r.random() < 0.3:
        bb = (bb & 0xffff) | (ctx.u32() & 0xffff0000) if -0x8000 <= b < 0x8000 and b >= 0 else bb
    return [a & 0xffffffff, bb, c & 0xffffffff]


def g_bf9a1(ctx):
    begin(ctx)
    obj = newbuf(ctx, 0x240)
    put(ctx, obj + 0x54, 'I', newbuf(ctx, 0x40))
    return [obj]


def cnorm_world(ctx):
    """The world of Sim_100c476c (as in spec_w3o.py): pointer table with ids, the 0x200 records, an object."""
    r = ctx.rng
    objs = []
    for i in range(8):
        o = zbuf(ctx, 0x1000)
        put(ctx, o + 8, 'I', r.randrange(0, 3))
        objs.append(o)
    wb(ctx, 0x109e8b40, struct.pack('<8I', *objs))
    wb(ctx, 0x109b7d54, b''.join(struct.pack('<I', r.randrange(0, 3)) + bytes(0x68) for _ in range(3)))
    w32(ctx, 0x1017b8c8, r.randrange(0, 8))
    holders = [zbuf(ctx, 0x80) for _ in range(6)]
    for o in holders:
        wf(ctx, o + 0x70, *[sane(ctx, 20000) for _ in range(3)])
    data = bytearray(0x200 * 0x156)
    for i in range(0x200):
        o = i * 0x156
        d = norm_vec(ctx)
        struct.pack_into('<I', data, o, r.choice([3, 9, 0xa, 0xc, 3, 9, 0, 1, 5]))
        struct.pack_into('<3f', data, o + 0x18, *d)
        struct.pack_into('<I', data, o + 0x3c, r.choice([0, r.choice(holders), r.choice(holders)]))
        struct.pack_into('<I', data, o + 0x48, r.choice([0, 0x100, r.getrandbits(32) & ~0x8000, 0x8000]))
        struct.pack_into('<I', data, o + 0x50, r.randrange(0, 8))
    wb(ctx, 0x1096fc80, data)
    return objs


def g_c1853(ctx):
    begin(ctx)
    r = ctx.rng
    cnorm_world(ctx)
    o = zbuf(ctx, 0x300)
    put(ctx, o + 4, 'I', r.randrange(0, 8))
    wf(ctx, o + 0x58, *[sane(ctx, 30000) for _ in range(3)])
    w32(ctx, o + 0x1fc, ctx.u32())
    rets(0x100c4d63, *[r.choice([0, 1, 1, 2]) for _ in range(3)])
    return [o]


def g_c34b7(ctx):
    begin(ctx)
    r = ctx.rng
    cur = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, cur)
    obj = zbuf(ctx, 0x400)
    put(ctx, obj + 4, 'I', r.choice([cur, cur, r.randrange(0, 8)]))
    sub = zbuf(ctx, 0x600)
    put(ctx, sub + 0x50c, 'I', r.choice([0, 1, 1, 2]))
    put(ctx, obj + 0x28, 'I', sub)
    wf(ctx, obj + 0x110, sane(ctx, 720))
    wf(ctx, obj + 0xe8, r.choice([sane(ctx, 720), 0.0, 180.0, -180.0, 360.0]))
    wf(ctx, obj + 0x68, r.choice([sane(ctx, 720), 0.0, 180.0, 90.0]))
    put(ctx, obj + 0xf0, 'I', ctx.u32())
    key = r.choice([0, 1, 0x7fff, 0x8000, 0xffff, ctx.u32()])
    return [obj, key]


def g_c4da3(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x80)
    return [0 if r.random() < 0.08 else obj]


def g_c6eae(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x4e0)
    cnt = r.choice([0, 1, 2, 3, 5, 6, 0xffffffff])
    put(ctx, obj + 0x4b4, 'I', cnt)
    names = []
    for i in range(6):
        s = rstr(ctx, 4, b'aAbB')
        names.append(s)
        put(ctx, obj + i * 12 + 4, 'I', cstr(ctx, s, 8))
    if r.random() < 0.6 and names:
        s = r.choice(names)
        s = bytes(c ^ 0x20 if (c | 0x20) in b'ab' and r.random() < 0.5 else c for c in s)
    else:
        s = rstr(ctx, 4, b'aAbB')
    return [obj, cstr(ctx, s, 8)]


def SF(name, orig, c, gen, trials, **kw):
    d = dict(name=name, orig=orig, c=c, gen=gen, trials=trials)
    d.update(kw)
    return d


_here = [a for a in sys.argv if a.endswith('spec_w5a.py')]
_w4 = os.path.join(os.path.dirname(os.path.abspath(_here[0])) if _here else os.path.join(os.getcwd(), 'tools', 'verify'),
                   'spec_w4u.py')
W4 = {}
exec(open(_w4).read(), W4)

SPECS = [
    SF('Sim_10007c45', 0x10007c45, 'Sim_10007c45', g_7c45, 300, void=True),
    SF('Sim_1000ebfb', 0x1000ebfb, 'Sim_1000ebfb', g_ebfb, 400),
    SF('Sim_1000ed77', 0x1000ed77, 'Sim_1000ed77', g_ed77, 500, fp=True, tol=1e-9),
    SF('Sim_1001042f', 0x1001042f, 'Sim_1001042f', g_state, 400, void=True),
    SF('Sim_10010564', 0x10010564, 'Sim_10010564', g_state, 400, void=True),
    SF('Sim_10010699', 0x10010699, 'Sim_10010699', g_state, 400, void=True),
    SF('Sim_10012a72', 0x10012a72, 'Sim_10012a72', g_12a72, 500),
    SF('Sim_10015a63', 0x10015a63, 'Sim_10015a63', g_15a63, 500, void=True),
    SF('Sim_1001f3ac', 0x1001f3ac, 'Sim_1001f3ac', g_1f3ac, 600, void=True, api=W4['API']),
    SF('Sim_100203cf', 0x100203cf, 'Sim_100203cf', g_203cf, 300, void=True),
    SF('Sim_10029125', 0x10029125, 'Sim_10029125', g_29125, 400, void=True),
    SF('Sim_1002d1c0', 0x1002d1c0, 'Sim_1002d1c0', g_d1c0, 400, api=API),
    SF('Sim_1002d1c0_inrange', 0x1002d1c0, 'Sim_1002d1c0', g_d1c0_inrange, 600, api=API),
    SF('Sim_1002fa3f', 0x1002fa3f, 'Sim_1002fa3f', g_fa3f, 400, void=True, api=API),
    SF('Sim_1002feef', 0x1002feef, 'Sim_1002feef', g_feef, 500, void=True),
    SF('Sim_10031917', 0x10031917, 'Sim_10031917', g_31917, 400, void=True),
    SF('Sim_1003462f', 0x1003462f, 'Sim_1003462f', g_3462f, 400, void=True),
    SF('Sim_1003ce00', 0x1003ce00, 'Sim_1003ce00', g_3ce00, 500, void=True),
    SF('Sim_1003ff72', 0x1003ff72, 'Sim_1003ff72', g_ff72, 500, void=True),
    SF('Sim_10040a37', 0x10040a37, 'Sim_10040a37', g_40a37, 300, void=True),
    SF('Sim_10041c4c', 0x10041c4c, 'Sim_10041c4c', g_41c4c, 400, void=True),
    SF('Sim_10044909', 0x10044909, 'Sim_10044909', g_44909, 500, void=True),
    SF('Sim_10044c7a', 0x10044c7a, 'Sim_10044c7a', g_44c7a, 500, void=True),
    SF('Sim_1004ed37', 0x1004ed37, 'Sim_1004ed37', g_4ed37, 300),
    SF('Sim_100579ad', 0x100579ad, 'Sim_100579ad', g_579ad, 300, void=True),
    SF('Sim_1005ae79', 0x1005ae79, 'Sim_1005ae79', g_5ae79, 500),
    SF('Sim_10060882', 0x10060882, 'Sim_10060882', g_60882, 400, void=True, fp=True, tol=1e-9),
    SF('Sim_100610ae', 0x100610ae, 'Sim_100610ae', g_610ae, 500, void=True, fp=True, tol=1e-9),
    SF('Sim_10068c48', 0x10068c48, 'Sim_10068c48', g_68c48, 500, void=True, fp=True, tol=1e-9),
    SF('Sim_10069f68', 0x10069f68, 'Sim_10069f68', g_69f68, 400, void=True, fp=True, tol=1e-9),
    SF('Sim_1006ab98', 0x1006ab98, 'Sim_1006ab98', g_6ab98, 300, void=True),
    SF('Sim_10074c8f', 0x10074c8f, 'Sim_10074c8f', g_74c8f, 300, void=True),
    SF('Sim_1007d8ba', 0x1007d8ba, 'Sim_1007d8ba', g_7d8ba, 300),
    SF('Sim_1008349a', 0x1008349a, 'Sim_1008349a', g_8349a, 300, void=True),
    SF('Trig_InitTables', 0x100846e0, 'Trig_InitTables', g_846e0, 300, fp=True, tol=1e-9),
    SF('Sim_1008799c', 0x1008799c, 'Sim_1008799c', g_8799c, 300),
    SF('Sim_10088c1c', 0x10088c1c, 'Sim_10088c1c', g_88c1c, 300),
    SF('Sim_1008fce3', 0x1008fce3, 'Sim_1008fce3', g_8fce3, 400, void=True),
    SF('Sim_10098f30', 0x10098f30, 'Sim_10098f30', g_98f30, 300, void=True),
    SF('Sim_1009d8b9', 0x1009d8b9, 'Sim_1009d8b9', g_9d8b9, 300, void=True),
    SF('Sim_100a2b8c', 0x100a2b8c, 'Sim_100a2b8c', g_a2b8c, 300, void=True),
    SF('File_ReadChunked', 0x100a796c, 'File_ReadChunked', g_a796c, 500),
    SF('Sim_100aa5e8', 0x100aa5e8, 'Sim_100aa5e8', g_aa5e8, 400, void=True),
    SF('Sim_100aa742', 0x100aa742, 'Sim_100aa742', g_aa742, 400, void=True),
    SF('Sim_100aacd5', 0x100aacd5, 'Sim_100aacd5', g_aacd5, 500),
    SF('Sim_100b4342', 0x100b4342, 'Sim_100b4342', g_b4342, 500, void=True),
    SF('Sim_100bf9a1', 0x100bf9a1, 'Sim_100bf9a1', g_bf9a1, 300, void=True),
    SF('Sim_100c1853', 0x100c1853, 'Sim_100c1853', g_c1853, 500),
    SF('Sim_100c34b7', TRAMP, 'Test_100c34b7', g_c34b7, 500, fp=True, tol=1e-9),
    SF('Sim_100c4da3', 0x100c4da3, 'Sim_100c4da3', g_c4da3, 300),
    SF('Entry_FindByName', 0x100c6eae, 'Entry_FindByName', g_c6eae, 500),
]
