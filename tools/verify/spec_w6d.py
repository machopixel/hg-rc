# Differential tests for round 6, letter d (see difftest.py and docs/checker.md).
#
# The functions call helpers of the DLL that are not converted yet (or that another hook file patches). Those
# helpers are replaced in the emulator by a stub: a code hook at the helper's own address appends
# ('stub_<address>', arguments) to the checker's call log (compared between the original and the C run like
# the Windows calls), may write results through pointer arguments (the same bytes for both runs), sets eax to a
# value chosen by the generator and returns to the caller by setting EIP and ESP (code bytes are not patched).
# Stack addresses are logged as 'stack'; float arguments of a stub are written to FLOG instead (compared like
# memory, with the float tolerance) and texts are logged by content. Converted callees run as the original code
# (original side) and as their C conversion (C side). Functions that return a float in st(0) are called through
# a machine-code trampoline (original) or a C wrapper in w6d_test.c that stores st(0) at OUTF. Each trampoline
# has its own fixed address: the emulator may keep running an old translation of code that is rewritten.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_1000/w6d_calls.c',
    'sim/unsorted_1000/w6d_select.c',
    'sim/unsorted_1001/w6d_calls.c',
    'sim/unsorted_1001/w6d_slots.c',
    'sim/unsorted_1002/w6d_calls.c',
    'sim/unsorted_1003/w6d_calls.c',
    '../tools/verify/w6d_test.c',
    # existing converted code the functions above call (and what that calls)
    'sim/util/field.c', 'sim/unsorted_1009/w3o_ray.c', 'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1009/w2e_copy.c',
    'sim/unsorted_100f/w2e_string.c', 'sim/view/w2l_lists.c', 'sim/view/objlist.c', 'sim/view/w2d_view.c',
    'sim/view/matrix.c', 'sim/view/viewrect.c', 'sim/unsorted_1008/w3r_grid.c', 'sim/unsorted_1008/w2l_grid.c',
    'sim/unsorted_100a/w5y_misc.c', 'sim/unsorted_100b/w2j_misc.c', 'sim/unsorted_1002/w4v_misc.c', 'sim/win/w4v_heap.c',
    'sim/win/w4a_heap.c', 'sim/unsorted_100a/w5x_calls.c', 'sim/unsorted_1008/cell_link.c', 'sim/unsorted_100a/w2k_list.c',
    'sim/unsorted_1005/w2g_pool.c', 'sim/unsorted_1002/w5z_calls.c', 'sim/unsorted_1002/w5a_misc.c', 'sim/unsorted_1003/w5a_misc.c',
    'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_1000/w2e_names.c', 'sim/unsorted_1001/w2e_names.c',
    'sim/unsorted_1007/w2e_state.c', 'sim/core/w2a_timers.c', 'sim/core/timer.c', 'sim/unsorted_1007/w5a_misc.c',
    'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1003/w2e_misc.c', 'sim/unsorted_1002/w2e_object.c', 'sim/unsorted_1002/w2l_tables.c',
    'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_100a/w5z_calls.c', 'sim/unsorted_1000/w3s_model.c', 'sim/unsorted_1002/w2j_nodes.c',
    'sim/unsorted_100d/w2j_nodes.c', 'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1000/w3o_sphere.c',
    'sim/unsorted_1002/w2e_tables.c', 'sim/unsorted_1004/w4w_net.c', 'sim/unsorted_1002/w5y_misc.c', 'sim/unsorted_1005/w5b_calls.c',
    'sim/view/w3r_matrix.c', 'sim/unsorted_1009/w2h_misc.c', 'sim/unsorted_100b/fields.c', 'sim/util/ring.c',
    'sim/unsorted_1003/setters.c', 'sim/unsorted_1001/w5b_calls.c', 'sim/unsorted_1001/w2h_objects.c', 'sim/unsorted_1005/w2h_tables.c',
    'sim/unsorted_1000/w2j_slots.c', 'sim/unsorted_1004/tables.c', 'sim/util/lookup.c', 'sim/unsorted_1004/w3p_net.c',
    'sim/unsorted_1003/records.c', 'sim/unsorted_1004/w2h_record.c', 'sim/unsorted_1004/w2i_misc.c', 'sim/unsorted_100d/w2i_select.c',
    'sim/unsorted_1007/w2f_misc.c', 'sim/unsorted_1007/w3p_net.c', 'sim/unsorted_1001/w2l_misc.c', 'sim/unsorted_1009/w2l_pool.c',
    'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_100d/w2f_records.c', 'sim/unsorted_1001/w2j_misc.c', 'sim/util/record.c',
    'sim/unsorted_100a/w5c_calls.c', 'sim/unsorted_1006/w3q_quat.c', 'sim/unsorted_1001/w5c_calls.c', 'sim/unsorted_1001/w3t_loader.c',
    'sim/unsorted_1004/w4u_heap.c', 'sim/win/w4u_cdaudio.c', 'sim/unsorted_1000/w2a_fields.c', 'sim/unsorted_1000/w5b_calls.c',
    'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2k_misc.c', 'sim/unsorted_1000/w2l_fields.c', 'sim/unsorted_1000/w5z_calls.c',
    'sim/unsorted_1000/w3t_records.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000       # st(0) results are stored here (by the trampolines and the C test wrappers)
TRAMP = 0x00AF0000      # machine-code trampolines for the original side
CODE = 0x00AF8000       # targets of indirect calls (a `ret`, with a logging hook)
FLOG = 0x00AFD000       # float arguments of the stubbed helpers (32 bytes per call), compared with the float tolerance

# helper address -> number of dword arguments logged
STUBS = {
    0x100ef727: 7, 0x100e86a1: 7, 0x1002ff6e: 1, 0x10039708: 2, 0x1000e737: 12,
    0x100a9ba7: 1, 0x10095e0a: 6, 0x100cbc6b: 7, 0x1000a6f0: 2, 0x100cbdb8: 3, 0x100fc780: 6, 0x100fd7e0: 3,
    0x10016117: 2, 0x10020256: 2, 0x100463bf: 1, 0x100fc970: 2, 0x100fd710: 3, 0x1001fc18: 1, 0x100bc280: 2,
    0x100246b3: 12, 0x10025aee: 6, 0x100d3510: 3, 0x10027fb8: 1, 0x100aa8cb: 1, 0x1002a2f6: 1, 0x1009bd47: 0,
    0x10029125: 1, 0x10031ce0: 3, 0x100f9eb7: 2, 0x100a8816: 2,
}
MASK = {(0x10016117, 1): 0xffff}    # (helper, argument number) -> mask of the bits the helper reads
FARGS = {0x10095e0a: (0, 1, 2, 3, 4, 5), 0x100cbc6b: (0, 1, 2), 0x10025aee: (2, 3, 4, 5)}    # float arguments: logged in FLOG
SARGS = {0x100fd7e0: 2}             # helper -> argument that points to a text: the text is logged
EFFECT = {}         # helper -> function(uc, call number, raw arguments) that writes the helper's results
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def put(ctx, addr, fmt, *vals):
    out = b''
    for f, v in zip(fmt, vals):
        size = struct.calcsize('<' + f)
        out += (v & ((1 << (8 * size)) - 1)).to_bytes(size, 'little')
    ctx.m.uc.mem_write(addr, out)


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def ur32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def ustr(uc, addr, limit=200):
    out = b''
    while len(out) < limit:
        c = bytes(uc.mem_read(addr + len(out), 1))
        if c == b'\0':
            break
        out += c
    return out


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def rfl(ctx):
    """A float bit pattern: ordinary values, zeros, small and big ones, both signs, now and then a NaN."""
    r = ctx.rng
    c = r.random()
    if c < 0.45:
        return fbits(r.uniform(-1000, 1000))
    if c < 0.65:
        return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 2.0, 100.0, -100.0, 1e-7, 5000.0]))
    if c < 0.85:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-10, 10))
    if c < 0.9:
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])
    return r.getrandbits(32) & 0xff7fffff


def rstr(ctx, maxlen, alpha=b'abcXYZ019 _.'):
    n = ctx.rng.choice([0, 1, 2, 3, 4, 5, 7, 8, ctx.rng.randrange(0, maxlen + 1)])
    return bytes(ctx.rng.choice(alpha) for _ in range(min(n, maxlen)))


def cstr(ctx, data, extra=24):
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rb(ctx, extra))


# ---- stubs --------------------------------------------------------------------------------------
def _stub_cb(m, addr):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        nargs = ST.get('nargs', {}).get(addr, STUBS[addr])
        raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
        args = []
        n = len(m.api_log)
        for k, v in enumerate(raw):
            if (addr, k) in MASK:
                v &= MASK[(addr, k)]
            if k in FARGS.get(addr, ()):
                uc.mem_write(FLOG + 32 * n + 4 * k, struct.pack('<I', v))
                v = 'float'
            elif SARGS.get(addr) == k:
                v = ustr(uc, v)
            args.append('stack' if isinstance(v, int) and STACK_LO <= v <= STACK_HI else v)
        m.api_log.append(('stub_%x' % addr, tuple(args)))
        if addr in EFFECT:
            EFFECT[addr](m.uc, n, raw)
        rets = ST['rets'].get(addr) or [0]
        uc.reg_write(UC_X86_REG_EAX, rets[n % len(rets)] & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def _code_cb(m):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        k = (address - CODE) // 16
        nargs = ST.get('code_nargs', 2)
        raw = struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))
        args = tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)
        n = len(m.api_log)
        m.api_log.append(('code_%d' % k, args))
        uc.reg_write(UC_X86_REG_EAX, mix(ST['seed'], k, n))
    return cb


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def begin(ctx):
    """Installs the stubs (once per machine), the x87 state of the game (control word 0x27f) and the answers of
    the stubs for this trial. Must be the first thing a generator does."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w6d', False):
        m._w6d = True
        for addr in STUBS:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr), None, addr, addr)
        m.raw.hook_add(UC_HOOK_CODE, _code_cb(m), None, CODE, CODE + 16 * 64 - 1)
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['seed'] = r.getrandbits(32)
    m.uc.mem_write(FLOG, bytes(0x1000))
    ST['nargs'] = {}
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def code_target(ctx, k):
    """Address of a routine (`ret` with a logging hook) for an indirect call."""
    a = CODE + 16 * k
    ctx.m.uc.mem_write(a, b'\xc3')
    return a


def tramp_st0(ctx, target, nargs, slot):
    """Machine code that calls target(args as dwords) and stores st(0) as a double at OUTF; returns its address.
    Every function has its own slot: the emulator may keep running an old translation of rewritten code."""
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\xdd\x1d' + struct.pack('<I', OUTF)       # fstp qword [OUTF]
    code += b'\x31\xc0\x5d\xc3'
    a = TRAMP + 0x100 * slot
    ctx.m.uc.mem_write(a, code)
    return a


# ---- shared worlds ------------------------------------------------------------------------------
def name_table(ctx):
    """The table of names at 0x10a39200 (as in spec_w2e.py); returns the names."""
    rg = ctx.rng
    n = rg.choice([0, 0, 1, 2, 3, 5, 8])
    w32(ctx, 0x10a38de0, n)
    names = []
    for i in range(0, 12):
        nm = rstr(ctx, 6, b'abc')
        used = rg.choice([0, 1, 1, 2, rg.getrandbits(32)])
        ctx.m.uc.mem_write(0x10a39200 + i * 0x14, struct.pack('<I', used) + nm + b'\0' + rb(ctx, 15 - len(nm)))
        names.append(nm)
    return names


def named_arg(ctx):
    """A name that usually is (or is not) in the table."""
    rg = ctx.rng
    names = name_table(ctx)
    nm = rg.choice(names[1:12]) if rg.random() < 0.6 else rstr(ctx, 6, b'abc')
    return cstr(ctx, nm)


def random_table(ctx):
    """The pseudo-random table 0x101cffc0 and its position [0x1017cd38] (Sim_1009c929)."""
    for i in range(0x7f):
        w32(ctx, 0x101cffc0 + 4 * i, ctx.rng.getrandbits(31))
    w32(ctx, 0x1017cd38, ctx.rng.randrange(0, 0x7f))
    for i in range(16):
        w32(ctx, 0x1012df30 + 4 * i, ctx.u32())


# ---- generators ---------------------------------------------------------------------------------
def g_799a(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017b940, r.choice([0, 0, 1, ctx.u32()]))
    for i in range(0x100):
        w16(ctx, 0x101659e0 + 2 * i, r.getrandbits(16))
    return []


def g_97d3(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x28, r.choice([0, 0, 1, ctx.u32()]))
    prev = obj + 0x2c - 0xc
    for _ in range(r.choice([0, 1, 1, 2, 3, 5])):
        nrec = r.choice([0, 0, 1, 2, 3, 6])
        blk = newbuf(ctx, 0x30 + nrec * 0x28 + 0x40)
        put(ctx, blk + 4, 'hh', r.choice([0, 0, 1, 2, 3, 7, 40]), nrec)
        w32(ctx, blk + 8, 0x30)
        w32(ctx, prev + 0xc, blk)
        prev = blk
    w32(ctx, prev + 0xc, 0)
    return [obj]


def g_e9c9(ctx):
    r = begin(ctx)
    name = named_arg(ctx)
    if r.random() < 0.1:
        w32(ctx, 0x1015d0a4, 0)
    else:
        p = newbuf(ctx, 0x400)
        w32(ctx, 0x1015d0a4, p)
        n = r.choice([0, 1, 2, 4, 8, 15])
        w32(ctx, p + 4, n)
        for i in range(n):
            w32(ctx, p + 0x2d4 + 4 * i, r.choice([0, 1, 2, 3, 4, 5, 6, 11, -1, ctx.u32()]))
    return [name, ctx.u32()]


def g_ea40(ctx):
    r = begin(ctx)
    name = named_arg(ctx)
    random_table(ctx)
    w32(ctx, 0x101321b4, ctx.u32())
    if r.random() < 0.15:
        w32(ctx, 0x1015d0a4, 0)
    else:
        p = newbuf(ctx, 0x40)
        w32(ctx, 0x1015d0a4, p)
        w32(ctx, p + 0x10, r.choice([0, 1, ctx.u32()]))
    return [ctx.u32(), ctx.u32(), name, ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32()]


# ---- 0x10010b92 .. 0x1001fee5 -----------------------------------------------------------------------
def slot_world(ctx, recsize=0x700):
    """Pointer table 0x109e8b40 with records (group at +8, slot at +0x10, state word at +0x1a6, block at +0x28),
    the current index [0x1017b8c8], the record count [0x1015dc64] and the group slot counts (as in spec_w2j.py)."""
    rg = ctx.rng
    count = rg.randrange(1, 9)
    group = rg.randrange(0, 3)
    cur = rg.randrange(0, count)
    pc = rg.choice([0.2, 0.7, 1.0, 1.0])
    recs = []
    for i in range(count):
        rec = newbuf(ctx, recsize)
        g = group if (i == cur or rg.random() < 0.8) else rg.randrange(0, 3)
        put(ctx, rec + 8, 'I', g)
        put(ctx, rec + 0x10, 'I', i + 1 if rg.random() < 0.8 else rg.randrange(0, 12))
        put(ctx, rec + 0x1a6, 'h', 0xc if rg.random() < pc else rg.choice([0, 1, 3, 5, -1]))
        blk = newbuf(ctx, 0x700)
        put(ctx, blk + 0x4f0, 'I', rg.choice([2, 2, 2, 0, 1]))
        w32(ctx, rec + 0x28, blk)
        recs.append(rec)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
    put(ctx, recs[cur] + 8, 'I', group)
    w32(ctx, 0x1015dc64, count if rg.random() < 0.9 else rg.randrange(0, count + 1))
    w32(ctx, 0x1017b8c8, cur)
    for g in range(3):
        w32(ctx, 0x109b7d44 + g * 0x6c, rg.choice([0, 1, 2, 3, 4, 5, 6, 7, 3, 4]))
    return recs


def g_10b92(ctx):
    r = begin(ctx)
    slot_world(ctx)
    w32(ctx, 0x1015d100, r.choice([0, 0, 0, 1, 1, 2, 2, 3, -1, 7, 8, ctx.u32()]))
    return []


def matrix(ctx, addr):
    r = ctx.rng
    vals = [r.uniform(-2, 2) for _ in range(9)] + [r.uniform(-5000, 5000) for _ in range(3)]
    if r.random() < 0.1:
        vals = [struct.unpack('<f', struct.pack('<I', rfl(ctx)))[0] for _ in range(12)]
    wf(ctx, addr, *vals)


def g_1274b(ctx):
    begin(ctx)
    t = newbuf(ctx, 0x100)
    matrix(ctx, t + 0x4c)
    obj = newbuf(ctx, 0x80)
    w32(ctx, obj + 0x48, t)
    return [obj]


def e_cbc6b(uc, n, raw):
    """0x100cbc6b writes a float through each of its four pointer arguments (in argument order)."""
    for k in range(3, 7):
        h = mix(ST['seed'], n, k)
        uc.mem_write(raw[k], struct.pack('<f', ((h % 20001) - 10000) / 64.0))


EFFECT[0x100cbc6b] = e_cbc6b


def g_127f0(ctx):
    begin(ctx)
    a = ctx.buf(16, struct.pack('<4I', rfl(ctx), rfl(ctx), rfl(ctx), rfl(ctx)))
    b = ctx.buf(16, struct.pack('<4I', rfl(ctx), rfl(ctx), rfl(ctx), rfl(ctx)))
    tramp_st0(ctx, 0x100127f0, 2, 1)
    return [a, b]


def g_13516(ctx):
    r = begin(ctx)
    for i in range(8):
        rec = newbuf(ctx, 0x40)
        put(ctx, rec + 0x1c, 'H', r.choice([0, 0x10, 0x10, 0xffff, r.getrandbits(16)]))
        w32(ctx, 0x109e8b40 + 4 * i, rec)
    now = r.choice([0, 100, 5000, -5, 0x7fff, r.getrandbits(32)])
    w32(ctx, 0x1015dc78, now)
    obj = newbuf(ctx, 0x200)
    put(ctx, obj + 0x1da, 'h', r.choice([now - 1, now - 0x21f, now, now + 1, r.getrandbits(16)]))
    w32(ctx, obj + 0xd4, r.choice([fbits(r.uniform(-300, 300)), fbits(100.0), rfl(ctx)]))
    w16(ctx, 0x10a38a74, r.getrandbits(16))
    key = r.randrange(0, 8) | r.choice([0, 0x200, 0x200, 0x8200, r.getrandbits(16) & 0xff00])
    value = r.choice([r.randrange(-300, 300), 100, 0x7fff, -0x8000, r.getrandbits(16)])
    rets(0x1000a6f0, r.choice([0, 0, 1, 2]))
    return [obj, key & 0xffff | (r.getrandbits(16) << 16), value & 0xffff | (r.getrandbits(16) << 16),
            r.choice([0, 1, 1, 1, ctx.u32()])]


def g_1655f(ctx):
    r = begin(ctx)
    c = r.random()
    if c < 0.5:
        x = fbits(r.uniform(-0.02, 0.02))
    elif c < 0.65:
        x = fbits(r.choice([0.0, -0.0, 1e-7, -1e-7, 1.0000000116860974e-07, -1.0000000116860974e-07, 0.015625,
                            -0.015625, 5e-8, -5e-8, 2e-7]))
    else:
        x = rfl(ctx)
    n = r.choice([0, 0, 1, -1, r.randrange(-3000, 3000), r.randrange(-0x10000, 0x10000), ctx.u32()])
    tramp_st0(ctx, 0x1001655f, 3, 2)
    return [x, n & 0xffffffff, r.choice([0, 1, 1, ctx.u32()])]


def g_1448d(ctx):
    r = begin(ctx)
    f = r.choice([fbits(r.uniform(-300, 300)), fbits(r.uniform(-1, 1)), rfl(ctx), fbits(0.0)])
    n = r.choice([0, 1, -1, r.randrange(-3000, 3000), ctx.u32()])
    tramp_st0(ctx, 0x1001448d, 3, 3)
    return [ctx.u32(), f, n & 0xffffffff]


def g_157ca(ctx):
    r = begin(ctx)
    recs = slot_world(ctx)
    for rec in recs:
        wf(ctx, rec + 0x58, r.uniform(-5000, 5000), r.uniform(-5000, 5000), r.uniform(-5000, 5000))
        wf(ctx, r32(ctx, rec + 0x28) + 0x53c, r.uniform(-100, 300))
    for k in range(8):
        e = 0x109a8640 + k * 0x54
        wf(ctx, e + 0x10, r.choice([50.0, 150.0, 1000.0, -3.0]))
        wf(ctx, e + 0x18, r.uniform(-5000, 5000), r.uniform(-5000, 5000), r.uniform(-5000, 5000))
        w32(ctx, 0x109e9ba8 + k * 0x50, r.choice([0, 1, 2, 3, -1, 100]))
    w32(ctx, 0x1017D218, r.choice([3, 3, 1, -1]))
    for i in range(4):
        ent = 0x101A2100 + i * 0x88
        blk = newbuf(ctx, 0x60)
        wf(ctx, blk + 0x44, r.uniform(-5000, 5000), r.uniform(-5000, 5000), r.uniform(-5000, 5000))
        w32(ctx, ent + 0x28, blk if r.random() < 0.8 else 0)
        w32(ctx, ent + 0x2c, 0)
    w32(ctx, 0x1017e5a8, r.choice([0, 1, 5, 0x7c, 0x7d, 0x7f, 0x80, -1, -2]))
    obj = newbuf(ctx, 0x200)
    group = r.choice([0x100, 0x200, 0x400, 0x100, 0x200, 0x400, 0, 0x300, 0x800])
    k = r.randrange(0, len(recs)) if group == 0x200 else r.randrange(0, 8)
    put(ctx, obj + 0x1a8, 'H', group | k | r.choice([0, 0, 0x1000, 0xf000]))
    w32(ctx, obj + 0x28, r32(ctx, recs[r.randrange(0, len(recs))] + 0x28))      # a block with the float +0x53c
    w32(ctx, obj + 4, r.randrange(0, len(recs)))
    w32(ctx, obj + 0x1d2, r.choice([0, 100, 3000, 3500, 10000, -5, r.randrange(-100, 20000)]))
    return [obj]


def e_sprintf(uc, n, raw):
    """0x100fc780 (sprintf) writes a text made from its arguments into the buffer."""
    text = b'line %08x %08x %08x %08x' % tuple(raw[2:6])
    uc.mem_write(raw[0], text + b'\0')


EFFECT[0x100fc780] = e_sprintf


def g_15afd(ctx):
    r = begin(ctx)
    recs = slot_world(ctx)
    cur = r32(ctx, 0x1017b8c8)
    index = r.choice([cur, cur, r.randrange(0, len(recs))])
    rec = recs[index]
    flags = r.choice([0, 1, 5, 0x200, 0x205, 0x10000, ctx.u32()])
    w32(ctx, 0x1015ecd0, r.choice([0, 1, 1, ctx.u32()]))
    w32(ctx, rec + 0x14, r.choice([3, 3, 0, 1]))
    w32(ctx, rec + 0xc, r.choice([cur, cur, index, ctx.u32()]))
    put(ctx, rec + 0x1a8, 'H', r.choice([flags | 0x200, flags | 0x200, flags, r.getrandbits(16)]))
    put(ctx, rec + 0x1a6, 'h', r.choice([3, 3, 0, 0xc]))
    w32(ctx, 0x1015dc78, ctx.u32())
    w32(ctx, 0x101668f4, r.choice([0, 1, 1]))
    w32(ctx, 0x101668f8, r.choice([0, 0x00AFC000, 0x00AFC000]))
    return [index, flags]


def g_178af(ctx):
    r = begin(ctx)
    slot_world(ctx)
    count = r.choice([0, 1, 3, 6])
    w32(ctx, 0x1015f3ac, count)
    for i in range(8):
        nm = rstr(ctx, 6, b'abAB')
        ctx.m.uc.mem_write(0x109b68e0 + i * 0x74, nm + b'\0' + rb(ctx, 0x73 - len(nm)))
    w32(ctx, 0x1013bb40, r.choice([0, 1, 2, 3, 0xf, 0x10, 0x11, -1]))
    a = r.choice([-1, -1, 0, 1, 2, 3, 4, 0x12345, ctx.u32()])
    b = r.choice([0, 1, 2, 3, 5, 7])
    return [a & 0xffffffff, b]


def g_19cc0(ctx):
    r = begin(ctx)
    count = r.choice([0, 1, 3, 10, 24, 25, 26, -1])
    w32(ctx, 0x1015d188, count)
    keys = []
    for i in range(26):
        k = r.choice([r.randrange(-5, 40), r.getrandbits(32)])
        keys.append(k)
        w32(ctx, 0x10a37aa0 + 8 * i, k, ctx.u32())
    w32(ctx, 0x1015d180, r.choice([0, 0, 1, 5, -1, ctx.u32()]))
    w32(ctx, 0x1015d184, ctx.u32())
    key = r.choice([keys[r.randrange(0, 26)], r.randrange(-5, 40), r.getrandbits(16)])
    return [ctx.buf(8, struct.pack('<h', s16(key)) + rb(ctx, 6))]


def s16(v):
    v &= 0xffff
    return v - 0x10000 if v >= 0x8000 else v


def name_tree(ctx, depth):
    r = ctx.rng
    node = newbuf(ctx, 0xc0)
    w32(ctx, node + 0x90, cstr(ctx, rstr(ctx, 6, b'ab')))
    kids = [name_tree(ctx, depth - 1) if depth > 0 and r.random() < 0.6 else 0 for _ in range(2)]
    w32(ctx, node + 4, kids[0])
    w32(ctx, node + 8, kids[1])
    return node


def g_1c107(ctx):
    r = begin(ctx)
    recs = slot_world(ctx)
    index = r.randrange(0, len(recs))
    w32(ctx, recs[index] + 0x48, name_tree(ctx, r.choice([0, 1, 2, 3])) if r.random() < 0.9 else 0)
    w32(ctx, 0x10131750, r.choice([0, 1, 1, 1, ctx.u32()]))
    obj = newbuf(ctx, 0x100)
    cnt = newbuf(ctx, 0x20)
    w32(ctx, cnt + 0xc, r.choice([0, 1, 2, 3, 5, -1]))
    w32(ctx, obj + 0x14, cnt)
    for i in range(6):
        w32(ctx, obj + 0x1c + 0xc * i, newbuf(ctx, 0x20))
    rets(0x100fc970, *[r.choice([0, 0, 0, 1, 5]) for _ in range(5)])
    return [index, obj]


def a_mci(n, a, uc):
    """mciSendCommandA: MCI_STATUS answers a mode (or another value) in the parameter block; now and then fails."""
    msg = a[1]
    h = mix(ST['seed'], n, 77)
    if h % 100 < ST['mci_fail']:
        return 0x111
    if msg == 0x814:
        p = ur32(uc, uc.reg_read(UC_X86_REG_ESP) + 16)
        modes = [0x20d, 0x20e, 0x211, 0x212, 0x20c, 0x213, 1]
        uc.mem_write(p + 4, struct.pack('<I', modes[(h >> 8) % len(modes)]))
    return 0


API = {'mciSendCommandA': a_mci}


def e_sscanf(uc, n, raw):
    """0x100fd710 (sscanf) stores a number through its third argument."""
    uc.mem_write(raw[2], struct.pack('<I', ST['scan'] & 0xffffffff))


EFFECT[0x100fd710] = e_sscanf


def g_1f26e(ctx):
    r = begin(ctx)
    ST['mci_fail'] = r.choice([0, 0, 0, 20])
    ST['scan'] = r.choice([0, 1, 2, 3, 5, 9, -1])
    w32(ctx, 0x1015d1c0, r.choice([0, 0, 1]))
    w32(ctx, 0x101321a8, r.choice([0, 1, 2, 3, 5, 9, -1]))
    w32(ctx, 0x101321ac, ctx.u32())
    w32(ctx, 0x101321c0, r.choice([0, 8, 8, 0xff, ctx.u32()]))
    w32(ctx, 0x1014c9b0, r.choice([1, 2, ctx.u32()]))
    w32(ctx, 0x1017d08c, 1 if r.random() < 0.85 else 0)
    first = r.choice([1, 1, 2, 0])
    last = first + r.randrange(0, 6)
    tbl = newbuf(ctx, 0x80)
    w32(ctx, 0x1017d080, first, last, tbl)
    w32(ctx, 0x1017d068, r.randrange(0, 9), r.getrandbits(8), r.getrandbits(6), r.getrandbits(5))
    w32(ctx, 0x1017d090, ctx.u32(), ctx.u32())
    w32(ctx, 0x1017d078, ctx.u32())
    return []


def chain(ctx, n, size=0x80):
    """n linked elements (next at +0x67); returns them."""
    els = [newbuf(ctx, size) for _ in range(n)]
    for a, b in zip(els, els[1:] + [0]):
        w32(ctx, a + 0x67, b)
    return els


def g_1fb6d(ctx):
    r = begin(ctx)
    els = chain(ctx, r.choice([0, 1, 2, 3, 5]))
    now = r.choice([0, 100, -5, ctx.u32()])
    w32(ctx, 0x1015dc78, now)
    if els:
        w32(ctx, els[0] + 0x2c, r.choice([0, 0, now + 1, now - 1, now, ctx.u32()]))
    w32(ctx, 0x1015d528, els[0] if els else 0)
    w32(ctx, 0x1015d52c, r.choice([0, 0, 1]))
    rets(0x1001fc18, *[r.choice([0, 1, -1, -1, -2]) for _ in range(4)])
    return []


def g_1fe29(ctx):
    r = begin(ctx)
    recs = slot_world(ctx)
    name_table(ctx)
    blk = r32(ctx, recs[r32(ctx, 0x1017b8c8)] + 0x28)
    w32(ctx, blk + 0x613, r.choice([0, 1, 2, 3, 5]))
    w32(ctx, 0x101321c0, r.choice([0, 2, 2, 0xff, ctx.u32()]))
    w32(ctx, 0x101321b8, ctx.u32())
    return []


def g_1fee5(ctx):
    r = begin(ctx)
    els = chain(ctx, r.choice([0, 1, 2, 3, 5, 7]))
    for e in els:
        w32(ctx, e + 0x30, r.choice([0, 0x4f, 0x50, 0x51, 0x100, -1]))
        w32(ctx, e + 0x24, r.choice([0, 0, 0x00AFC100, 0x00AFC200]))
        w32(ctx, e + 0x28, r.choice([0, 0, 1]))
    w32(ctx, 0x1015d528, els[0] if els else 0)
    w32(ctx, 0x1015d530, r.choice([0, 0, 0, 1]))
    return [r.choice([0, 1, 1, ctx.u32()])]


# ---- 0x10024548 .. 0x1002fd4c -----------------------------------------------------------------------
def obj_tree(ctx, depth):
    """Nodes (first child at +4, next sibling at +8) whose +0x8c may point at a list object (as in spec_w5x.py)."""
    r = ctx.rng
    node = zbuf(ctx, 0x100)
    if r.random() < 0.6:
        obj = zbuf(ctx, 0x100)
        put(ctx, obj, 'H', r.choice([0, 0x800, 0x1000, 0x1800, 0x8000, 0xffff]))
        w32(ctx, obj + 0x34, r.choice([4, 0, 1]))
        w32(ctx, node + 0x8c, obj)
    kids = []
    if depth > 0:
        kids = [obj_tree(ctx, depth - 1) for _ in range(r.choice([0, 1, 2, 3]))]
    if kids:
        w32(ctx, node + 4, kids[0])
        for a, b in zip(kids, kids[1:]):
            w32(ctx, a + 8, b)
    return node


def records_world(ctx):
    """The 0x34-byte records at 0x101c57e0 (count [0x1017d1c8]) with trees at +0x30 (as in spec_w5x.py)."""
    r = ctx.rng
    count = r.choice([0, 1, 2, 3, 5])
    w32(ctx, 0x1017d1c8, count)
    w32(ctx, 0x1017d1c4, r.choice([0, 0, 1, -1, count - 1, count - 2, 3]))
    w32(ctx, 0x1017d1d0, 0, 0)
    for i in range(count):
        w32(ctx, 0x101c57e0 + i * 0x34 + 0x30, obj_tree(ctx, r.choice([0, 1, 2])))
    return count


def pool_world(ctx):
    """The entry pool of Sim_100582d2: count 0x109b0eac, top 0x109b0eb0, list ends 0x101668d0/d4 (empty or one)."""
    r = ctx.rng
    count = r.choice([4, 8])
    w32(ctx, 0x109b0eac, count)
    w32(ctx, 0x109b0eb0, r.choice([-1, 0, 1, count - 1, count - 2]))
    n = r.randrange(0, count)
    for i in range(count):
        w32(ctx, 0x109b16e0 + i * 0x4c, 0, 0)
    listed = r.random() < 0.5
    e = 0x109b16e0 + n * 0x4c
    w32(ctx, 0x101668d0, e if listed else 0, e if listed else 0)
    return count, n, listed


def g_24548(ctx):
    r = begin(ctx)
    count = records_world(ctx)
    pcount, pn, listed = pool_world(ctx)
    w32(ctx, 0x1017c8cc, r.choice([0, 1]))
    index = r.randrange(0, 4)
    rec = 0x1096fc80 + index * 0x156
    w32(ctx, rec + 0x60, r.choice([0, 0, 1]))
    w32(ctx, rec + 0x3c, obj_tree(ctx, r.choice([0, 1, 2])))
    w32(ctx, rec + 8, ctx.u32())
    w32(ctx, rec + 0x50, ctx.u32())
    w32(ctx, rec + 0x68, pn if listed else r.choice([pn, pn, -1, pcount, 0]))
    w32(ctx, rec + 0x6c, r.choice([-1, 0, 1, 2, count - 1, count]))
    w32(ctx, 0x10134890, ctx.u32())
    return [ctx.u32(), index, r.choice([0, 1, 1, ctx.u32()])] + [ctx.u32() for _ in range(6)]


def sphere(ctx, x, y, z, rad):
    obj = newbuf(ctx, 0x60)
    wf(ctx, obj + 0x44, x, y, z, rad)
    return obj


def g_257ec(ctx):
    r = begin(ctx)
    cx, cy, cz = r.uniform(-1000, 1000), r.uniform(-1000, 1000), r.uniform(-100, 100)
    w32(ctx, 0x1017d218, 4)
    for i in range(4):
        obj = sphere(ctx, cx + r.uniform(-300, 300), cy + r.uniform(-300, 300), cz + r.uniform(-30, 30),
                     r.uniform(0, 200))
        w32(ctx, 0x101a2128 + i * 0x88, obj if r.random() < 0.85 else 0)
    n = r.choice([0, 1, 2, 4, 6])
    w32(ctx, 0x1015dc68, n)
    for i in range(n):
        e = 0x109e9ba0 + i * 0x50
        put(ctx, e, 'H', r.choice([0, 0, 4, 1, 0xfffb]))
        w32(ctx, e + 8, r.choice([0, 1, 2, 3, 3, -1, 4]))
    w32(ctx, 0x1015dc80, r.choice([1, 2, 3, 0, -1, r.randrange(0, 100)]))
    return [ctx.u32(), fbits(cx), fbits(cy), fbits(cz), fbits(r.uniform(0, 300)),
            r.choice([fbits(r.uniform(-2, 2)), rfl(ctx)])]


def g_25f6b(ctx):
    r = begin(ctx)
    for i in range(0x100):
        w32(ctx, 0x109a5a40 + i * 0x2c + 0x24, 0)
    for _ in range(r.choice([0, 1, 2, 3])):
        i = r.randrange(0, 0x100)
        e = 0x109a5a40 + i * 0x2c
        w32(ctx, e + 0x24, r.choice([0x16, 0x16, 0x15]))
        o = newbuf(ctx, 0x100) if r.random() < 0.8 else 0
        if o:
            w32(ctx, o + 0x8c, r.choice([0, 1, ctx.u32()]))
        w32(ctx, e, o)
    w32(ctx, 0x101480a8, ctx.u32())
    w32(ctx, 0x1015dbd0, ctx.u32())
    obj = newbuf(ctx, 0x80)
    return [obj]


def g_279f1(ctx):
    r = begin(ctx)
    ST['nargs'][0x100bc280] = 5
    magic = ctx.u32()
    w32(ctx, 0x101349e8, magic)
    base = newbuf(ctx, 0x400)
    off = r.choice([0, 0x10, 0x40, 0x100])
    blk = base + off
    first = r.choice([magic, magic, ctx.u32()])
    ln = r.choice([0xc, 8, 0x10, 0x20, 0x30, 4, 0x200])
    w32(ctx, blk, first, ln)
    step = 0xc if first == magic else ln
    nxt = blk + step
    if nxt + 8 <= base + 0x400:
        w32(ctx, nxt + 4, r.choice([8, 0x10, 0x20, 7, 0, 0x100, 0xffffffff, r.randrange(0, 0x80)]))
    obj = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x1b, blk)
    w32(ctx, obj + 0x17, base)
    w32(ctx, obj + 0xf, r.choice([0x400, off + step, off + step + 1, off + step - 1, 0x20, -1]))
    w32(ctx, obj + 0x13, r.choice([0x20, 0x10, 0x100, 8, -1]))
    return [obj]


def g_27e6a(ctx):
    r = begin(ctx)
    if r.random() < 0.1:
        w32(ctx, 0x1015dc30, 0)
        return []
    tab = zbuf(ctx, 0x3f1 * 4)
    for _ in range(r.choice([0, 1, 3, 8])):
        b = r.choice([0, 1, 0x3f0, r.randrange(0, 0x3f1)])
        node = newbuf(ctx, 0x20)
        w32(ctx, node + 8, r32(ctx, tab + 4 * b))
        w32(ctx, tab + 4 * b, node)
    w32(ctx, 0x1015dc30, tab)
    w32(ctx, 0x109f13e0, ctx.u32())
    w32(ctx, 0x109f13f0, ctx.u32(), ctx.u32())
    w32(ctx, 0x1017b8f8, ctx.u32())
    return []


def g_28b90(ctx):
    r = begin(ctx)
    full = r.random() < 0.15
    keys = []
    for i in range(0x80):
        k = ctx.u32() | 1 if (full or r.random() < 0.6) else 0
        keys.append(k)
        e = 0x109eeb00 + i * 0x30
        w32(ctx, e, *[ctx.u32() for _ in range(12)])
        w32(ctx, e + 4, k)
    w32(ctx, 0x1015dc40, ctx.u32())
    # no signalling NaN: the emulator's x87 does not quiet it on fld as the processor does (the C does)
    w32(ctx, 0x1013abc0, r.choice([fbits(0.0296), fbits(-2.5), 0, 0x80000000, 0xffc12345, 0x7fc00000,
                                   0x7f800000, rfl(ctx)]))
    key = r.choice([0, r.choice(keys), r.choice(keys), ctx.u32()])
    return [key, ctx.u32()]


def g_299c8(ctx):
    r = begin(ctx)
    cx, cy, cz = r.uniform(-1000, 1000), r.uniform(-1000, 1000), r.uniform(-100, 100)
    for i in range(0x80):
        w32(ctx, 0x109f09e0 + i * 0x14, 0, 0, 0, 0, 0)
    for _ in range(r.choice([0, 1, 3, 6])):
        i = r.randrange(0, 0x80)
        e = 0x109f09e0 + i * 0x14
        holder = newbuf(ctx, 0x100)
        w32(ctx, holder + 0x8c, sphere(ctx, cx + r.uniform(-300, 300), cy + r.uniform(-300, 300),
                                       cz + r.uniform(-30, 30), r.uniform(0, 200)))
        w32(ctx, e, r.choice([1, 1, 0, ctx.u32()]), holder if r.random() < 0.9 else 0)
        wf(ctx, e + 0xc, r.uniform(-5, 50))
    w32(ctx, 0x1015dc80, r.choice([1, 2, 3, 0, -1, r.randrange(0, 100)]))
    return [fbits(cx), fbits(cy), fbits(cz), fbits(r.uniform(0, 300)), r.choice([fbits(r.uniform(-2, 20)), rfl(ctx)])]


def g_2a1f2(ctx):
    r = begin(ctx)
    ST['code_nargs'] = 2
    index = r.randrange(0, 6)
    w32(ctx, 0x1017b8c8, r.choice([index, index, r.randrange(0, 6)]))
    w32(ctx, 0x109e8b40 + 4 * index, ctx.u32())
    ST['heap'] = [newbuf(ctx, 0x300), newbuf(ctx, 0x300), 0]
    w32(ctx, 0x1015fd38, *[ctx.u32() for _ in range(0x17)])
    return [index, r.choice([0, code_target(ctx, 3), code_target(ctx, 3)])]


def a_heapalloc(n, a):
    return ST['heap'][mix(ST['seed'], n) % len(ST['heap'])]


def g_2c63a(ctx):
    r = begin(ctx)
    ST['code_nargs'] = 1
    o = newbuf(ctx, 0x20)
    w32(ctx, 0x1017b8c4, o)
    put(ctx, o + 9, 'B', r.choice([0, 0, 1, 0x80]))
    w32(ctx, 0x1015ec58, r.choice([0, 1, 1]))
    for i in range(0x1e):
        obj = newbuf(ctx, 0x120)
        w32(ctx, obj + 0x108, code_target(ctx, i % 4) if r.random() < 0.5 else 0)
        w32(ctx, 0x109e88e0 + 4 * i, obj)
    for i in range(7):
        rec = 0
        if r.random() < 0.5:
            rec = newbuf(ctx, 0x40)
            w32(ctx, rec + 0x18, r.choice([0, 0, 1]))
            w32(ctx, rec + 0x14, r.randrange(0, 100))
        w32(ctx, 0x109b94e0 + 4 * i, rec)
    w32(ctx, 0x1015dc30, zbuf(ctx, 0x3f1 * 4))
    w32(ctx, 0x1014cec0, ctx.buf(8, bytes(r.choice(b'ABCxyz') for _ in range(8))))     # a name: positive bytes
    w32(ctx, 0x101c64f0, r.randrange(0, 100))
    w32(ctx, 0x1015dc2c, ctx.u32())
    w32(ctx, 0x1017b8f8, ctx.u32())
    return []


def box_obj(ctx, cx, cy, cz):
    """An object with a box at +0x98 (and the flag 0x200 that makes Sim_1002fa3f keep it) or without."""
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    c = r.random()
    if c < 0.75:
        put(ctx, obj, 'H', r.choice([0x200, 0x201, 0xffff]))
        box = newbuf(ctx, 0x18)
        w = [r.uniform(0, 100) for _ in range(3)]
        vals = [fbits(cx - w[0]), fbits(cx + w[0]), fbits(cy - w[1]), fbits(cy + w[1]), fbits(cz - w[2]),
                fbits(cz + w[2])]
        if r.random() < 0.15:
            vals[r.randrange(0, 6)] = r.choice([0x7fc00000, 0xffc00000])
        w32(ctx, box, *vals)
        w32(ctx, obj + 0x98, box)
    elif c < 0.85:
        put(ctx, obj, 'H', r.choice([0x200, 0]))
        w32(ctx, obj + 0x98, 0)
        w32(ctx, obj + 0x2c, 0)
        ST['heap'] = [0]
    else:
        put(ctx, obj, 'H', 0)
        w32(ctx, obj + 0x98, newbuf(ctx, 0x18))
        w32(ctx, obj + 0x2c, 0)
    return obj


def g_2ce45(ctx):
    r = begin(ctx)
    ST['heap'] = [0]
    cx, cy, cz = r.uniform(-1000, 1000), r.uniform(-1000, 1000), r.uniform(-100, 100)
    obj = box_obj(ctx, cx, cy, cz)

    def coord(c):
        return r.choice([fbits(c + r.uniform(-150, 150)), fbits(c), rfl(ctx)]) if r.random() < 0.9 else 0x7fc00000
    return [obj, coord(cx), coord(cy), coord(cz), newbuf(ctx, 4), newbuf(ctx, 4), newbuf(ctx, 4)]


def model_block(ctx, n, off):
    r = ctx.rng
    blk = newbuf(ctx, 0x18 + 0x58 * max(n, 0) + 0x40)
    put(ctx, blk + 4, 'hh', n, 0)
    w32(ctx, blk + 8, 0x18 + 0x58 * max(n, 0))
    for i in range(max(n, 0)):
        e = blk + 0x18 + 0x58 * i
        vals = [fbits(r.uniform(-1000, 1000)) for _ in range(6)]
        if r.random() < 0.1:
            vals[r.randrange(0, 6)] = r.choice([0x7fc00000, 0xff800000, 0x7f800000, 0x80000000, 0])
        w32(ctx, e, *vals)
    return blk


def g_bbox(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x100)
    if r.random() < 0.1:
        w32(ctx, obj + 0x2c, 0)
    else:
        blk = model_block(ctx, r.choice([0, 1, 2, 3, 5, 8]), 0)
        w32(ctx, obj + 0x2c, blk)
        stamp = ctx.u32()
        w32(ctx, blk + 0x10, stamp)
        w32(ctx, obj + 0x9c, stamp if r.random() < 0.7 else ctx.u32())
        t = newbuf(ctx, 0x100)
        matrix(ctx, t + 0x4c)
        w32(ctx, obj + 0x28, t)
    return [obj] + [newbuf(ctx, 4) for _ in range(6)]


# ---- 0x10031ed3 .. 0x10034dc2 -----------------------------------------------------------------------
def e_31ce0(uc, n, raw):
    """0x10031ce0 changes the segment it is given (most of the time): a new length at +0x30, known (+0x34 = 1)."""
    h = mix(ST['seed'], n, 31)
    if h % 4 == 0:
        return
    length = [fbits(((h >> 4) % 100000) / 16.0), 0x7f7fc99f, 0x7f7fc99d, 0x7fc00000, 0][(h >> 2) % 5 if h % 7 == 0 else 0]
    uc.mem_write(raw[2] + 0x30, struct.pack('<II', length, 1))


EFFECT[0x10031ce0] = e_31ce0


def g_31ed3(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x40)
    seg = newbuf(ctx, 0x40)
    wf(ctx, seg, *[r.uniform(-1000, 1000) for _ in range(12)])
    w32(ctx, seg + 0x30, fbits(r.uniform(0, 3000)), r.choice([1, 1, 1, 0]))
    rets(0x10031ce0, *[r.choice([0, 1, 1, 2]) for _ in range(4)])
    return [obj, ctx.u32(), seg]


def mono_map(ctx):
    """Maps the text screen at 0xb0000 (the original writes there) and makes it part of the compared memory. Must
    run before anything is written in the trial; the page index is rebuilt at once for the new snapshot."""
    m = ctx.m
    if not getattr(m, 'mono_mapped', False):
        m.uc.mem_map(0xb0000, 0x10000)
        m.wr.append((0xb0000, 0x10000))
        m.mono_mapped = True
        m.snap0 = m.dump()
        m._index()


def g_345fc(ctx):
    mono_map(ctx)
    begin(ctx)
    ctx.m.uc.mem_write(0xb0e00, rb(ctx, 0x300))
    return []


def e_f9eb7(uc, n, raw):
    """0x100f9eb7 stores a code address and the code segment through its pointers."""
    uc.mem_write(raw[0], struct.pack('<I', 0x100f4fd4))
    uc.mem_write(raw[1], struct.pack('<I', 0x1b))


EFFECT[0x100f9eb7] = e_f9eb7

API_34CCA = {'GetVersion': lambda n, a: ST['version'], 'VirtualProtect': lambda n, a: ST['protect']}


def g_34cca(ctx):
    r = begin(ctx)
    ST['version'] = r.choice([0xc0000a04, 0x0a280105, 0x80000000, 0x7fffffff])
    ST['protect'] = r.choice([0, 1, 1])
    rets(0x100f9eb7, r.choice([0x4ec6, 0x4ec6, 0, ctx.u32()]))
    for a in (0x1014cb04, 0x1014cb08):
        w32(ctx, a, r.choice([0, 0, 1, ctx.u32()]))
    for a in (0x1014cb18, 0x1014caf8, 0x10138c84, 0x1017d140):
        w32(ctx, a, ctx.u32())
    return []


def g_34dc2(ctx):
    r = begin(ctx)
    other, last, spare = 0x1017b988, 0x1017c818, 0x1017b9d8
    for c in (other, last, spare):
        w32(ctx, c, *([0] * 11))
    w32(ctx, 0x1017c8c4, 0)
    w32(ctx, 0x1015e544, ctx.u32())
    if r.random() < 0.1:
        w32(ctx, 0x1017c8cc, 0)
        return []
    w32(ctx, 0x1017c8cc, other)
    nodes = []
    for _ in range(r.choice([0, 1, 2, 3, 5, 7])):
        node = zbuf(ctx, 0x100)
        kind = r.choice([0x90, 0x70, 0x70, 0x00, 0x50, 0xf0])
        put(ctx, node, 'HH', r.choice([0, 0, 0x800, 0x1000]), kind | r.choice([0, 1, 0xf00, 0x8000]))
        w32(ctx, node + 0x34, r.choice([4, 0, 4, 1]))
        if kind == 0x90:
            root = zbuf(ctx, 0x100)
            w32(ctx, root + 0x8c, node)
            w32(ctx, node + 0x28, root)
        nodes.append(node)
    prev = other
    for node in nodes:
        w32(ctx, prev + 8, node)
        w32(ctx, node + 4, prev)
        prev = node
    return []


# ---- 0x1000bba7, 0x1000ceb7: the 0x128-byte records of a block and the 0xd6-byte table 0x10145508 -----------
NENT = 0x18


def kind_table(ctx):
    """The table of 0xd6-byte entries at 0x10145508 with sane fields (as in spec_w3t.py)."""
    r = ctx.rng
    pool = (0.0, 0.5, 1.0, 2.0, 10.0, 100.0, -1.0)
    for i in range(NENT):
        e = 0x10145508 + i * 0xd6
        ctx.m.uc.mem_write(e, rb(ctx, 0xd6))
        for off in (0x3c, 0x44, 0x4c, 0x50):
            wf(ctx, e + off, r.choice(pool))
        w32(ctx, e + 0x5c, r.choice([0, 1, 2, 5, 10, 100, -3]))
        w32(ctx, e + 0x86, r.choice([0, 0x40, 1, 0x41, r.getrandbits(32)]))
        w32(ctx, e + 0x78, r.choice([0, 1, 5, 7, 0x80000000, r.getrandbits(32)]))
        w32(ctx, e + 8, r.choice([0, 4, 5, 1, 7]))
        w32(ctx, e + 0x10, r.choice([0, 1, 2, 0x3d, 0x42, 0x43, 0x44]))
        w32(ctx, e + 0x14, r.choice([0, 1, 2, 3]))


def weapon_records(ctx):
    """Eight 0x128-byte records (a zeroed record before them, for a selection of -1); returns the address."""
    r = ctx.rng
    zbuf(ctx, 0x140)
    buf = zbuf(ctx, 8 * 0x128 + 0x40)
    for i in range(8):
        o = buf + i * 0x128
        ctx.m.uc.mem_write(o, rb(ctx, 0x30))
        w32(ctx, o, r.choice([-1, 0, 1, 2]))
        w32(ctx, o + 4, r.choice([0, 1, 9, 0xc, 0x10, 0x14, 0x16, 5, 6, -1, 7]))
        w32(ctx, o + 8, r.choice([-1, 0, 1, 1, 1, 2]))
        w32(ctx, o + 0xc, r.choice([0, 1, 2]))
        w32(ctx, o + 0x20, r.choice([0, 1, 1, 5, -1, 3, -4]))
    return buf


def weapon_world(ctx):
    """A unit obj whose block obj[0x28] holds the records; the pointer table for the flags obj[0xf0]."""
    r = ctx.rng
    for i in range(8):
        s = zbuf(ctx, 0x40)
        w32(ctx, s, r.choice([0, 9, 9, 8, 5, 1]))
        w32(ctx, s + 0x14, r.choice([0, 1, 2, 3]))
        w32(ctx, s + 0xc, r.choice([0, 1, 2, 3]))
        w32(ctx, 0x109e8b40 + 4 * i, s)
    kind_table(ctx)
    w32(ctx, 0x1015ec58, r.choice([0, 1, 1]))
    w32(ctx, 0x1015dc78, r.choice([0, 3, 100, -1]))
    base = zbuf(ctx, 0x200)
    w32(ctx, base + 0x14, r.choice([3, 3, 0, 1, 2]))
    w32(ctx, base + 0xc, r.choice([0, 1, 2, 3]))
    put(ctx, base + 0x1aa, 'H', r.choice([0x200, 0x201, 0x202, 0x100, 0x203, r.getrandbits(16)]))
    wf(ctx, base + 0xd8, r.choice([-1.0, 0.0, 0.5, 1.0, 2.0, 5.0, 10.0, 100.0, 50.0]))
    block = zbuf(ctx, 0x600)
    w32(ctx, block, base)
    w32(ctx, block + 0x84, weapon_records(ctx))
    w32(ctx, block + 0x4f8, r.choice([0, 1, 3, 8, 8, 8]))
    w32(ctx, block + 0x4fc, r.choice([0, 1, 2, 3, 5, 7, 7, -1]))
    w32(ctx, block + 0x4dc, r.choice([0, 1, 5, 100, -4]))
    obj = zbuf(ctx, 0x200)
    w32(ctx, obj + 0x28, block)
    k = r.randrange(0, 8)
    w32(ctx, obj + 0xf0, r.choice([0, k, 0x200 | k, 0x600 | k, 0x400, 0x400 | k]))
    w32(ctx, obj + 0x54, ctx.buf(8, struct.pack('<f', r.choice([0.0, 1.0, 5.0, -2.0, 0.1])) + bytes(4)))
    wf(ctx, obj + 0xd8, r.choice([-1.0, 0.0, 0.5, 1.0, 2.0, 5.0, 10.0, 100.0, 50.0]))
    rets(0x1000a6f0, r.choice([0, 0, 1]))
    return obj


def rich_world(ctx, obj):
    """Makes most records acceptable to Sim_1000b9f1 and reachable by Sim_100d5cc1, so that several compete."""
    r = ctx.rng
    block = r32(ctx, obj + 0x28)
    recs = r32(ctx, block + 0x84)
    w32(ctx, 0x1015ec58, 0)
    w32(ctx, 0x1015dc78, 100)
    w32(ctx, block + 0x4dc, 5)
    wf(ctx, r32(ctx, block) + 0xd8, 5.0)
    for i in range(NENT):
        e = 0x10145508 + i * 0xd6
        w32(ctx, e + 8, 4)
        w32(ctx, e + 0x86, 0x40 | r.choice([0, 1]))
        wf(ctx, e + 0x4c, 0.0)
        wf(ctx, e + 0x50, 100.0)
        w32(ctx, e + 0x78, r.choice([1, 2, 5, 7, 100, 0x80000000, r.getrandbits(32)]))
    for i in range(8):
        o = recs + i * 0x128
        w32(ctx, o, 0)
        w32(ctx, o + 8, 1 if r.random() < 0.9 else 0)
        w32(ctx, o + 0xc, 0)
        w32(ctx, o + 0x20, r.choice([1, 2, 3, 5, -1]))


def g_bba7(ctx):
    r = begin(ctx)
    obj = weapon_world(ctx)
    if r.random() < 0.5:
        rich_world(ctx, obj)
    return [obj, newbuf(ctx, 4)]


def g_ceb7(ctx):
    r = begin(ctx)
    obj = weapon_world(ctx)
    block = r32(ctx, obj + 0x28)
    w32(ctx, block + 0x4fc, r.choice([0, 1, 2, 3, 5, 7]))
    return [obj if r.random() > 0.05 else 0]


SPECS = [
    dict(name='Sim_1000799a', orig=0x1000799a, c='Sim_1000799a', gen=g_799a, trials=300, void=True),
    dict(name='Sim_100097d3', orig=0x100097d3, c='Sim_100097d3', gen=g_97d3, trials=400),
    dict(name='Sim_1000e9c9', orig=0x1000e9c9, c='Sim_1000e9c9', gen=g_e9c9, trials=400, void=True),
    dict(name='Sim_1000ea40', orig=0x1000ea40, c='Sim_1000ea40', gen=g_ea40, trials=400),
    dict(name='Sim_10010b92', orig=0x10010b92, c='Sim_10010b92', gen=g_10b92, trials=400, void=True),
    dict(name='Sim_1001274b', orig=0x1001274b, c='Sim_1001274b', gen=g_1274b, trials=300, void=True, fp=True),
    dict(name='Sim_100127f0', orig=TRAMP + 0x100, c='Test_100127f0', gen=g_127f0, trials=400, fp=True),
    dict(name='Sim_10013516', orig=0x10013516, c='Sim_10013516', gen=g_13516, trials=500),
    dict(name='Sim_1001448d', orig=TRAMP + 0x300, c='Test_1001448d', gen=g_1448d, trials=400, fp=True),
    dict(name='Sim_1001655f', orig=TRAMP + 0x200, c='Test_1001655f', gen=g_1655f, trials=600, fp=True),
    dict(name='Sim_100157ca', orig=0x100157ca, c='Sim_100157ca', gen=g_157ca, trials=500, void=True, fp=True),
    dict(name='Sim_10015afd', orig=0x10015afd, c='Sim_10015afd', gen=g_15afd, trials=500, void=True),
    dict(name='Sim_100178af', orig=0x100178af, c='Sim_100178af', gen=g_178af, trials=400),
    dict(name='Sim_10019cc0', orig=0x10019cc0, c='Sim_10019cc0', gen=g_19cc0, trials=400),
    dict(name='Sim_1001c107', orig=0x1001c107, c='Sim_1001c107', gen=g_1c107, trials=400, void=True),
    dict(name='Sim_1001f26e', orig=0x1001f26e, c='Sim_1001f26e', gen=g_1f26e, trials=500, void=True, api=API),
    dict(name='Sim_1001fb6d', orig=0x1001fb6d, c='Sim_1001fb6d', gen=g_1fb6d, trials=400, void=True),
    dict(name='Sim_1001fe29', orig=0x1001fe29, c='Sim_1001fe29', gen=g_1fe29, trials=400),
    dict(name='Sim_1001fee5', orig=0x1001fee5, c='Sim_1001fee5', gen=g_1fee5, trials=500, void=True),
    dict(name='Sim_10024548', orig=0x10024548, c='Sim_10024548', gen=g_24548, trials=500, void=True),
    dict(name='Sim_100257ec', orig=0x100257ec, c='Sim_100257ec', gen=g_257ec, trials=500, void=True, fp=True),
    dict(name='Sim_10025f6b', orig=0x10025f6b, c='Sim_10025f6b', gen=g_25f6b, trials=400),
    dict(name='Sim_100279f1', orig=0x100279f1, c='Sim_100279f1', gen=g_279f1, trials=500),
    dict(name='Sim_10027e6a', orig=0x10027e6a, c='Sim_10027e6a', gen=g_27e6a, trials=300, void=True),
    dict(name='Sim_10028b90', orig=0x10028b90, c='Sim_10028b90', gen=g_28b90, trials=400),
    dict(name='Sim_100299c8', orig=0x100299c8, c='Sim_100299c8', gen=g_299c8, trials=500, void=True, fp=True),
    dict(name='Sim_1002a1f2', orig=0x1002a1f2, c='Sim_1002a1f2', gen=g_2a1f2, trials=400, void=True,
         api={'HeapAlloc': a_heapalloc}),
    dict(name='Sim_1002c63a', orig=0x1002c63a, c='Sim_1002c63a', gen=g_2c63a, trials=300),
    dict(name='Sim_1002ce45', orig=0x1002ce45, c='Sim_1002ce45', gen=g_2ce45, trials=600, void=True,
         api={'HeapAlloc': a_heapalloc}),
    dict(name='Sim_1002fba5', orig=0x1002fba5, c='Sim_1002fba5', gen=g_bbox, trials=500, void=True, fp=True),
    dict(name='Sim_1002fd4c', orig=0x1002fd4c, c='Sim_1002fd4c', gen=g_bbox, trials=500, void=True, fp=True),
    dict(name='Sim_10031ed3', orig=0x10031ed3, c='Sim_10031ed3', gen=g_31ed3, trials=500, fp=True),
    dict(name='Sim_100345fc', orig=0x100345fc, c='Sim_100345fc', gen=g_345fc, trials=300, void=True),
    dict(name='Sim_10034cca', orig=0x10034cca, c='Sim_10034cca', gen=g_34cca, trials=300, void=True, fp=True,
         api=API_34CCA),
    dict(name='Sim_10034dc2', orig=0x10034dc2, c='Sim_10034dc2', gen=g_34dc2, trials=500, void=True),
    dict(name='Sim_1000bba7', orig=0x1000bba7, c='Sim_1000bba7', gen=g_bba7, trials=800, fp=True),
    dict(name='Sim_1000ceb7', orig=0x1000ceb7, c='Sim_1000ceb7', gen=g_ceb7, trials=600),
]
