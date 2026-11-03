# Differential tests for round 5, letter c (see difftest.py and docs/checker.md).
#
# These functions call helpers of the DLL that are not converted yet. In the emulator each such helper is
# replaced by a stub: a `ret` written over its first byte plus a code hook that appends ('stub_<address>',
# arguments) to the checker's call log (which is compared between the original and the C run, like the
# Windows calls), optionally writes out-values through pointer arguments (the helper's results), and answers with
# a value chosen by the generator. The stub address is the helper's own address, so the original and the C code
# reach the same stub. Stack addresses are logged as 'stack'.
# Arguments that the helper only reads as 16-bit values are masked to 16 bits in the log (the original pushes
# the top half of a register there; the helper's disassembly reads them with `movsx ..., word ptr`).
import random
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG

SOURCES = [
    'sim/unsorted_1000/w5c_calls.c', 'sim/unsorted_1001/w5c_calls.c', 'sim/unsorted_1002/w5c_calls.c',
    'sim/unsorted_1003/w5c_calls.c', 'sim/unsorted_1003/w3r_misc.c', 'sim/unsorted_100a/w4u_misc.c', 'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1004/w5c_calls.c', 'sim/unsorted_1005/w5c_calls.c',
    'sim/unsorted_1008/w5c_calls.c', 'sim/unsorted_1009/w5c_calls.c', 'sim/unsorted_100a/w5c_calls.c',
    'sim/unsorted_100b/w5c_calls.c', 'sim/unsorted_100c/w5c_calls.c', 'sim/unsorted_100d/w5c_calls.c',
    # existing converted code the functions above call
    'sim/unsorted_1000/w2h_access.c', 'sim/unsorted_1003/w2e_mono.c', 'sim/util/lookup.c',
    'sim/unsorted_1001/w3t_loader.c', 'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_1004/w4u_heap.c',
    'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_1003/flags.c',
    'sim/unsorted_1006/w3q_quat.c', 'sim/unsorted_100b/w2e_strings.c', 'sim/win/w4v_dll.c',
    'sim/util/record.c', 'sim/view/matrix.c',
    # floating-point functions and the converted code they call
    'sim/unsorted_1001/w5c_math.c', 'sim/unsorted_1006/w5c_math.c', 'sim/unsorted_1008/w5c_math.c',
    'sim/unsorted_1009/w5c_math.c', 'sim/unsorted_100c/w5c_math.c', 'sim/unsorted_100d/w5c_math.c',
    '../tools/verify/w5c_test.c',
    'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1002/w2k_misc.c', 'sim/unsorted_1009/w3o_ray.c',
    'sim/unsorted_1009/w3q_misc.c', 'sim/unsorted_100d/w2d_state.c', 'sim/unsorted_100d/w3q_misc.c',
    'sim/util/field.c', 'sim/unsorted_1003/slots.c', 'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_100b/fields.c',
    'sim/unsorted_100d/w2f_records.c',
    'sim/unsorted_1003/setters.c',
    'sim/view/objlist.c',
    'sim/util/ring.c',
    'sim/win/w4a_heap.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
ARENA = 0x00A80000      # blocks that the HeapAlloc stub hands out
OUTF = 0x00AFF000       # float result of the st(0) function (inside argument memory, compared like all memory)
TRAMP = 0x00AF0000      # machine code in argument memory (rewritten for every trial)

# helper address -> (number of dword arguments, {argument index: mask}, [(argument index, bytes written through it)])
STUBS = {
    0x100097d3: (1, {}, []),
    0x10016117: (2, {}, []),
    0x10015630: (4, {1: 0xffff, 2: 0xffff}, []),
    0x100cbdb8: (3, {}, []),
    0x1001b902: (1, {}, []),
    0x1001cedf: (1, {}, []),
    0x100bb2b0: (2, {}, []),
    0x100bc280: (2, {}, []),
    0x10003413: (3, {}, []),      # the cache loader that Sim_1001b705 calls
    0x10028fb0: (2, {}, []),
    0x1002ce45: (7, {}, [(4, 4), (5, 4), (6, 4)]),
    0x1002fba5: (7, {}, []),
    0x1003a548: (1, {}, []),
    0x10038913: (3, {}, []),
    0x100aa799: (1, {}, []),
    0x100ff030: (2, {}, []),
    0x1004db5b: (2, {}, []),
    0x1004d980: (5, {}, []),
    0x100e85cb: (3, {}, []),
    0x1004ee50: (1, {}, []),
    0x100203b8: (1, {}, []),
    0x100ed70b: (1, {}, []),
    0x100a96d8: (1, {}, []),
    0x10099510: (0, {}, []),
    0x100a648c: (6, {}, []),
    0x10060b76: (2, {}, []),
    0x1006123b: (5, {}, [(4, 16)]),
    0x100fc970: (2, {}, []),
    0x100aa8cb: (1, {}, []),
    0x100bd69c: (2, {}, []),
    0x100bd134: (0, {}, []),
    0x100bcd61: (1, {}, []),
    0x100fd850: (2, {}, []),
    0x100c0158: (6, {1: 0xffff}, [(2, 4), (3, 4), (4, 4)]),
    0x10060656: (7, {}, [(0, 48)]),
    0x100d6f92: (4, {}, [(1, 4), (2, 4), (3, 4)]),
    0x1008a5e3: (0, {}, []),
    0x100eb070: (2, {}, []),
    # targets of the indirect calls (a `ret` in the page of the checker's return address, with a hook)
    0x00F00600: (0, {}, []),
    0x00F00610: (0, {}, []),
    0x00F00620: (0, {}, []),
    0x00F00630: (0, {}, []),
}
CB_SLOT, CB_18, CB_14, CB_10 = 0x00F00600, 0x00F00610, 0x00F00620, 0x00F00630
ST = {}


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def put(ctx, addr, fmt, *vals):
    out = b''
    for f, v in zip(fmt, vals):
        size = struct.calcsize('<' + f)
        out += (v & ((1 << (8 * size)) - 1)).to_bytes(size, 'little')
    ctx.m.uc.mem_write(addr, out)


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def s32(v):
    v &= 0xffffffff
    return v - (1 << 32) if v >= 1 << 31 else v


def rstr(ctx, maxlen, alpha=b'abcXYZ019 _.'):
    n = ctx.rng.choice([0, 1, 2, 3, 4, 5, 7, 8, 9, ctx.rng.randrange(0, maxlen + 1), ctx.rng.randrange(0, maxlen + 1)])
    return bytes(ctx.rng.choice(alpha) for _ in range(min(n, maxlen)))


def cstr(ctx, data, extra=24):
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rb(ctx, extra))


def rv(ctx):
    return ctx.rng.choice([0, 1, 2, 0xffffffff, ctx.rng.getrandbits(32)])


# ---- stubs --------------------------------------------------------------------------------------
def _make_cb(m, addr, nargs, mask, outs):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        raw = [struct.unpack('<I', bytes(uc.mem_read(esp + 4 + 4 * k, 4)))[0] for k in range(nargs)]
        args = []
        for k, v in enumerate(raw):
            if k in mask:
                v &= mask[k]
            if STACK_LO <= v <= STACK_HI:
                v = 'stack'
            args.append(v)
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % addr, tuple(args)))
        for j, (k, nbytes) in enumerate(outs):
            data = ST['outs'].get((addr, j))
            if data is None:       # the same bytes for the original and the C run: derived from the trial and the call
                data = random.Random('%s-%x-%d-%d' % (ST['seed'], addr, j, n)).getrandbits(8 * nbytes).to_bytes(nbytes, 'little')
            m.uc.mem_write(raw[k], data[:nbytes].ljust(nbytes, b'\0'))
        eff = EFFECTS.get(addr)
        if eff:
            eff(m, raw)
        rets = ST['rets'].get(addr) or [0]
        uc.reg_write(UC_X86_REG_EAX, rets[n % len(rets)] & 0xffffffff)
    return cb


def _eff_99510(m, raw):
    m.uc.mem_write(0x1017cc3c, struct.pack('<I', ST['eff_val']))


def _eff_60b76(m, raw):
    m.uc.mem_write(raw[0] - 0xc, random.Random('%s-60b76' % ST['seed']).getrandbits(96).to_bytes(12, 'little'))


EFFECTS = {0x10099510: _eff_99510, 0x10060b76: _eff_60b76}


def begin(ctx):
    """Installs the stubs (once per machine) and chooses the answers of this trial."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w5c', False):
        m._w5c = True
        for addr, (nargs, mask, outs) in STUBS.items():
            m.raw.mem_write(addr, b'\xc3')
            m.raw.hook_add(UC_HOOK_CODE, _make_cb(m, addr, nargs, mask, outs), None, addr, addr)
        m.raw.mem_map(0xb0000, 0x10000)         # the text screen that Mono_PutText writes to
        m.wr.append((0xb0000, 0x10000))
        m.snap0 = m.dump()
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['rng'] = r
    ST['seed'] = r.getrandbits(32)
    ST['rets'] = {a: [rv(ctx) for _ in range(4)] for a in STUBS}
    ST['outs'] = {}
    ST['alloc_fail'] = False
    ST['eff_val'] = r.choice([0, 1, 0x1234, r.getrandbits(32)])
    w32(ctx, 0x1017b8f8, r.choice([0x00c10000, 0x00c10000, r.getrandbits(32)]))
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


# ---- shared worlds ------------------------------------------------------------------------------
def slot_world(ctx, with_sub=True):
    """Pointer table 0x109e8b40 with records, the current group and its slot count. Returns (records, ids)."""
    rg = ctx.rng
    count = rg.randrange(1, 9)
    group = rg.randrange(0, 3)
    cur = rg.randrange(0, count)
    recs = []
    ids = []
    for i in range(count):
        rec = newbuf(ctx, 0x40)
        g = group if (i == cur or rg.random() < 0.85) else rg.randrange(0, 3)
        ident = i if rg.random() < 0.8 else rg.randrange(0, 10)
        put(ctx, rec + 8, 'I', g)
        put(ctx, rec + 0x10, 'I', ident)
        if with_sub:
            sub = newbuf(ctx, 0x600)
            put(ctx, sub + 0x4f0, 'I', 2 if rg.random() < 0.7 else rg.choice([0, 1, 3, 2]))
            put(ctx, rec + 0x28, 'I', sub)
        recs.append(rec)
        ids.append(ident)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
    put(ctx, recs[cur] + 8, 'I', group)
    w32(ctx, 0x1015dc64, count if rg.random() < 0.95 else rg.randrange(0, count + 1))
    w32(ctx, 0x1017b8c8, cur)
    return recs, ids


# ---- generators ---------------------------------------------------------------------------------
def g_7ed5(ctx):
    begin(ctx)
    w32(ctx, 0x10a3ade8, ctx.rng.choice([0, 1, 1, 1, 0x100]))
    ctx.m.uc.mem_write(0xb0000, bytes([0xaa]) * 0x10000)
    w32(ctx, 0x1015d03c, ctx.u32())
    return []


def g_7f49(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    link = newbuf(ctx, 0x20)
    put(ctx, obj + 0x30, 'I', 0 if r.random() < 0.2 else link)
    for a in (0x1015d00c, 0x1015d010, 0x1015d014, 0x1015d018):
        w32(ctx, a, ctx.u32())
    rets(0x100097d3, *[r.choice([0, 1, 5, -3 & 0xffffffff, r.getrandbits(32)]) for _ in range(3)])
    return [obj]


def g_slot(ctx):
    begin(ctx)
    r = ctx.rng
    recs, ids = slot_world(ctx)
    for i in range(8):
        w32(ctx, 0x1012ff58 + 4 * i, ctx.u32())
    slot = r.choice(ids + ids + [0, 1, 2, 3, 7, 8, 9, -1, -2, 20, 0x7fffffff])
    if slot < 0:
        slot &= 0xffffffff
    return [slot, ctx.u32()]


def g_13eba(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x300)
    put(ctx, obj + 0x198, 'H', r.choice([0, 0, 1, 1, 2, 3, 7, 20]))
    return [obj, ctx.u32()]


def g_1555c(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x300)
    put(ctx, obj + 0x198, 'H', r.choice([0, 0, 1, 2, 5, 20]))
    return [obj, r.choice([0, 1, 0x7fff, 0x8000, 0xffff, r.getrandbits(16), ctx.u32()]), r.choice([0, 0xffff, r.getrandbits(32)])]


def g_159cf(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x200)
    v = r.choice([0, 0x100, 0x1ff, 0xff, 0x8100, 0x8000, 0xffff, r.getrandbits(16), r.getrandbits(32)])
    if r.random() < 0.4:
        sv = v & 0xffff
        sv = sv - 0x10000 if sv >= 0x8000 else sv
        put(ctx, obj + 0x1d6, 'I', sv)
    return [obj, v]


def g_1bb1f(ctx):
    begin(ctx)
    r = ctx.rng
    # records of the loader area 0x10a1eaa0 (0x80 of 0x320 bytes): the first dword says whether one is in use
    for i in range(0x80):
        w32(ctx, 0x10a1eaa0 + i * 0x320, r.choice([1, 1, 1, 0, 0]) if i < 10 else 1)
    if r.random() < 0.5:
        w32(ctx, 0x10a1eaa0 + r.randrange(0, 0x80) * 0x320, 0)
    name = 0 if r.random() < 0.1 else cstr(ctx, rstr(ctx, 20))
    rec = zbuf(ctx, 0x40)
    put(ctx, rec + 4, 'I', r.choice([0, 0, 1, 2]))
    image = zbuf(ctx, 0x600)
    put(ctx, image + 0xc, 'I', r.choice([0, 0, 1, 2]))
    ST['rets'][0x1001b902] = [0 if r.random() < 0.15 else rec]
    ST['rets'][0x10003413] = [0 if r.random() < 0.2 else image]
    return [name, ctx.u32(), r.choice([0, 1, 1, ctx.u32()])]


def g_1dcff(ctx):
    begin(ctx)
    r = ctx.rng
    p = r.choice([0.0, 0.1, 0.3, 0.9])
    for i in range(0x80):
        w32(ctx, 0x10a1eaa0 + i * 0x320, r.getrandbits(32) if r.random() < p else 0)
    return []


def g_1ffc8(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    put(ctx, obj + 0x24, 'I', 0 if r.random() < 0.3 else newbuf(ctx, 16))
    put(ctx, obj + 0x28, 'I', 0 if r.random() < 0.4 else ctx.u32())
    return [obj]


def g_29ab3(ctx):
    begin(ctx)
    r = ctx.rng
    keys = [r.choice([0, r.randrange(1, 40), ctx.u32()]) for _ in range(0x80)]
    for i, k in enumerate(keys):
        ctx.m.uc.mem_write(0x109f09e0 + i * 0x14, rb(ctx, 4) + struct.pack('<I', k) + rb(ctx, 12))
    ctx.m.uc.mem_write(0x1015dc48, rb(ctx, 20))
    key = r.choice([r.choice(keys), r.choice(keys), 0, r.randrange(1, 40), ctx.u32()])
    return [key, ctx.u32()]


def g_2cde1(ctx):
    begin(ctx)
    r = ctx.rng
    out = newbuf(ctx, 8)
    ok = r.choice([0, 1, 5, 0xffffffff])
    ST['outs'][(0x1002ce45, 1)] = struct.pack('<I', ok) if r.random() < 0.85 else None
    if ST['outs'][(0x1002ce45, 1)] is None:
        del ST['outs'][(0x1002ce45, 1)]
    for a in (0x1015dd40, 0x1015dd44, 0x1015dd48):
        w32(ctx, a, ctx.u32())
    return [ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32(), out]


def g_2fad6(ctx):
    begin(ctx)
    ST['alloc_fail'] = ctx.rng.random() < 0.25
    ctx.m.uc.mem_write(ARENA, bytes(ctx.rng.getrandbits(8) for _ in range(0x1800)))
    return [ctx.u32()]


def g_2fb3d(ctx):
    begin(ctx)
    r = ctx.rng
    ST['alloc_fail'] = r.random() < 0.3
    ctx.m.uc.mem_write(ARENA, bytes(r.getrandbits(8) for _ in range(0x1800)))
    obj = newbuf(ctx, 0x100)
    put(ctx, obj, 'H', r.choice([0, 0x200, 0x1, 0xffff, r.getrandbits(16)]))
    put(ctx, obj + 0x98, 'I', r.choice([0, 0, 0, 0x1234, ctx.u32()]))
    return [0 if r.random() < 0.1 else obj]


def g_3a052(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x109e369c, r.choice([0, 1, ctx.u32()]))
    n = r.choice([0, 1, 2, 5, 10])
    tab = newbuf(ctx, 0x54 + 4 * 12)
    put(ctx, tab + 4, 'I', n)
    for i in range(12):
        put(ctx, tab + 0x54 + 4 * i, 'I', r.choice([0, 0, ctx.u32()]))
    w32(ctx, 0x1015d0a4, r.choice([tab, tab, tab, 0]))
    return []


def g_41247(ctx):
    begin(ctx)
    r = ctx.rng
    cur = r.choice([0, 1, 2, 5, ctx.u32()])
    w32(ctx, 0x1017b8c8, cur)
    recs = []
    for i in range(6):
        rec = newbuf(ctx, 0x60)
        put(ctx, rec + 4, 'I', cur if r.random() < 0.3 else r.choice([0, 1, 7, ctx.u32()]))
        put(ctx, rec + 0xc, 'I', cur if r.random() < 0.4 else r.choice([0, 1, 7, ctx.u32()]))
        put(ctx, rec + 0x14, 'I', r.choice([3, 3, 0, 1, 2]))
        w32(ctx, 0x109e8b40 + 4 * i, 0 if r.random() < 0.15 else rec)
    msg = newbuf(ctx, 16)
    put(ctx, msg + 2, 'B', r.choice([0, 1, 2, 3, 4, 5, 5]))
    return [msg]


def g_474db(ctx):
    begin(ctx)
    r = ctx.rng
    ctx.m.uc.mem_write(0x1015ef50, rb(ctx, 0x50))
    d = rstr(ctx, 12, b'abcDEF')
    ctx.m.uc.mem_write(0x1015f050, d + b'\0' + rb(ctx, 8) if r.random() < 0.7 else bytes(16))
    nm = rstr(ctx, 30, b'abcXYZ01.')
    if r.random() < 0.25:
        pos = r.randrange(0, len(nm) + 1)
        nm = nm[:pos] + r.choice([b'\\', b'/']) + nm[pos:]
    return [cstr(ctx, nm)]


def stream(ctx, cnt=None):
    r = ctx.rng
    f = newbuf(ctx, 0x20)
    data = newbuf(ctx, 64)
    put(ctx, f, 'I', data + r.randrange(0, 8))
    put(ctx, f + 4, 'I', (cnt if cnt is not None else r.choice([-3, -1, 0, 1, 2, 5, 100, 0x80000000])) & 0xffffffff)
    w32(ctx, 0x1015f344, f)
    return f


def g_4dcaa(ctx):
    begin(ctx)
    r = ctx.rng
    stream(ctx)
    return [r.choice([0, 0x41, 0xff, 0x100, 0x1ff, -1 & 0xffffffff, ctx.u32()])]


def g_4e1c6(ctx):
    begin(ctx)
    r = ctx.rng
    stream(ctx)
    w32(ctx, 0x1015f340, r.choice([0, 0, 1, 2, 0x7f, 0xff, 0x100, 0x1ff, r.getrandbits(8), ctx.u32()]))
    w32(ctx, 0x1015f348, r.choice([8, 8, 0, 1, 7, 9, ctx.u32()]))
    ctx.m.uc.mem_write(0x1015f230, rb(ctx, 0x40))
    rets(0x100ff030, *[r.choice([-1 & 0xffffffff, -1 & 0xffffffff, 0, 5, 0x41]) for _ in range(3)])
    rets(0x1004db5b, *[r.choice([0, 0, 1, -1 & 0xffffffff]) for _ in range(3)])
    return []


def g_4edca(ctx):
    begin(ctx)
    w32(ctx, 0x109b8be0, ctx.u32())
    return [ctx.u32(), ctx.u32()]


def g_4f0d6(ctx):
    begin(ctx)
    r = ctx.rng
    cur = ctx.u32()
    w32(ctx, 0x1017b8c8, cur)
    obj = newbuf(ctx, 0x200)
    put(ctx, obj + 4, 'I', cur if r.random() < 0.5 else cur + 1)
    put(ctx, obj + 0x28, 'I', newbuf(ctx, 0x700))
    return [obj]


def g_5264d(ctx):
    begin(ctx)
    r = ctx.rng
    a = r.choice([0, 1, 5, 15, 16, 17, -1, 3, 8])
    b = r.choice([0, 1, 7, 31, 32, 33, -2, 12, 20])
    w32(ctx, 0x1013bb40, a if r.random() < 0.5 else ctx.u32())
    ctx.m.uc.mem_write(0x109b68e0, rb(ctx, 0x74 * 32))
    ctx.m.uc.mem_write(0x109b8420, rb(ctx, 0x74 * 16))
    ctx.m.uc.mem_write(0x109b7d58 - 0x6c, rb(ctx, 0x6c * 17))
    return [a & 0xffffffff, b & 0xffffffff]


NAMES = [b'a', b'ab', b'abc', b'abd', b'', b'xy', b'ABC', b'b']


def name_tree(ctx, depth, names):
    r = ctx.rng
    node = zbuf(ctx, 0x100)
    nm = r.choice(names)
    put(ctx, node + 0x90, 'I', cstr(ctx, nm))
    kids = [name_tree(ctx, depth - 1, names) for _ in range(r.choice([0, 1, 2]))] if depth > 0 else []
    if kids:
        put(ctx, node + 4, 'I', kids[0])
        for a, b in zip(kids, kids[1:]):
            put(ctx, a + 8, 'I', b)
    if r.random() < 0.2:
        put(ctx, node + 8, 'I', name_tree(ctx, 0, names))
    return node


def g_5af06(ctx):
    begin(ctx)
    r = ctx.rng
    root = name_tree(ctx, 3, NAMES)
    name = cstr(ctx, r.choice(NAMES + [b'zzz']))
    return [0 if r.random() < 0.05 else root, name]


def g_8d36c(ctx):
    begin(ctx)
    r = ctx.rng
    rect = newbuf(ctx, 0x20)
    l = r.randrange(-50, 50)
    t = r.randrange(-50, 50)
    w = r.choice([1, 2, 3, 10, 100, 300, -2, -7, r.randrange(1, 300)])
    h = r.choice([2, 3, 10, 100, 300, -2, -9, r.randrange(2, 300)])
    put(ctx, rect + 4, 'iiii', l, t, l + w - 1, t + h - 1)
    packed = (r.choice([0, 1, -1, 0x7fff, -0x8000, r.randrange(-0x8000, 0x8000), r.randrange(-300, 300)]) << 16) & 0xffff0000
    packed |= r.choice([0, 1, 0x7fff, 0xffff, r.getrandbits(16), r.randrange(0, 300)])
    rets(0x100ed70b, packed)
    out = newbuf(ctx, 0x20)
    return [rect, out, ctx.u32()]


def g_988fc(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1015d1b8, newbuf(ctx, 0x40))
    w32(ctx, 0x1014c9fc, ctx.u32())
    return [ctx.u32(), r.choice([0, 1, 2, ctx.u32()])]


def g_9964b(ctx):
    begin(ctx)
    w32(ctx, 0x1017cc3c, ctx.rng.choice([0, 0, 0x1234, ctx.u32()]))
    return []


def g_6414(ctx):
    begin(ctx)
    r = ctx.rng
    a = newbuf(ctx, 0x40)
    table = newbuf(ctx, 0x58 * 6)
    off = r.randrange(0x18, 0x30)
    put(ctx, a + 4, 'I', off)
    put(ctx, a + off, 'B', r.randrange(0, 6))
    return [a, table]


def g_9ba7(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    put(ctx, obj + 0x94, 'I', r.choice([0, 0x8000, 0x8001, 0x7fff, ctx.u32()]))
    return [obj]


def fl(ctx):
    return ctx.rng.choice([0.0, 1.0, -1.0, 0.5, ctx.rng.uniform(-1, 1), ctx.rng.uniform(-3, 3)])


def g_aa127(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    ctx.m.uc.mem_write(obj + 0x3c, struct.pack('<4f', *[fl(ctx) for _ in range(4)]))
    put(ctx, obj + 0x94, 'I', ctx.u32())
    ST['outs'][(0x1006123b, 0)] = struct.pack('<4f', *[fl(ctx) for _ in range(4)])
    return [obj, ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32()]


def key_tree(ctx, depth, keys):
    r = ctx.rng
    node = zbuf(ctx, 0x100)
    rec = zbuf(ctx, 16)
    put(ctx, rec, 'II', r.choice(keys), r.choice(keys))
    put(ctx, node + 0x90, 'I', rec)
    kids = [key_tree(ctx, depth - 1, keys) for _ in range(r.choice([0, 1, 2]))] if depth > 0 else []
    if kids:
        put(ctx, node + 4, 'I', kids[0])
        for a, b in zip(kids, kids[1:]):
            put(ctx, a + 8, 'I', b)
    if r.random() < 0.2:
        put(ctx, node + 8, 'I', key_tree(ctx, 0, keys))
    return node


def g_aad(ctx):
    begin(ctx)
    r = ctx.rng
    keys = [1, 2, 3, 4, 0x1234]
    root = key_tree(ctx, 3, keys)
    key = zbuf(ctx, 16)
    put(ctx, key, 'I', r.choice(keys + [9]))
    rets(0x100fc970, *[r.choice([0, 0, 0, 1]) for _ in range(4)])
    return [0 if r.random() < 0.05 else root, key]


def g_aae0f(ctx):
    begin(ctx)
    r = ctx.rng
    root = name_tree(ctx, 3, NAMES)
    rets(0x100fc970, *[r.choice([0, 0, 0, 1]) for _ in range(5)])
    return [0 if r.random() < 0.05 else root, cstr(ctx, r.choice(NAMES))]


def lvl_tree(ctx, depth):
    r = ctx.rng
    node = zbuf(ctx, 0x100)
    put(ctx, node, 'I', ctx.u32())
    if r.random() < 0.7:
        obj = zbuf(ctx, 0x40)
        put(ctx, obj + 0x26, 'H', r.choice([0, 1, 2, 3, 0xffff, 0x10001 & 0xffff]))
        put(ctx, node + 0x8c, 'I', obj)
    kids = [lvl_tree(ctx, depth - 1) for _ in range(r.choice([0, 1, 2]))] if depth > 0 else []
    if kids:
        put(ctx, node + 4, 'I', kids[0])
        for a, b in zip(kids, kids[1:]):
            put(ctx, a + 8, 'I', b)
    if r.random() < 0.2:
        put(ctx, node + 8, 'I', lvl_tree(ctx, 0))
    return node


def g_ab0d6(ctx):
    begin(ctx)
    r = ctx.rng
    root = lvl_tree(ctx, 3)
    w32(ctx, 0x101c6504, ctx.u32())
    w32(ctx, 0x101c64f4, ctx.u32())
    return [0 if r.random() < 0.05 else root, r.choice([0, 1, 1, 2, 3, 4, -1 & 0xffffffff, 0x10001, ctx.u32()])]


def g_ba5d1(ctx):
    begin(ctx)
    r = ctx.rng
    for a in (0x1017d604, 0x1017d600, 0x1017d5fc):
        w32(ctx, a, r.choice([0, 0x1234, ctx.u32()]))
    return []


def g_bcbcf(ctx):
    begin(ctx)
    r = ctx.rng
    base = rstr(ctx, 20, b'abcXYZ019_')
    ext = r.choice([b'', b'.map', b'.prj', b'.x', b'.mapx'])
    path = cstr(ctx, base + ext, 16)
    w32(ctx, 0x1017e1bc, r.choice([0, 1, 2, 3]))
    ctx.m.uc.mem_write(0x1017dfb8, rb(ctx, 32 * 6))
    for k in range(1, 4):
        ctx.m.uc.mem_write(0x1017dfb8 + 32 * k, b'lib%d.dll\0' % k)
    rets(0x100bcd61, *[r.choice([0, 1, 1, 1]) for _ in range(2)])
    rets(0x100fd850, *[r.choice([0, 0x1234, 0x1234]) for _ in range(2)])
    return [path, ctx.u32(), r.choice([0, 0x41, 0xff, 0x1ff, ctx.u32()])]


def g_bee33(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x220)
    put(ctx, obj + 0x1fc, 'I', r.choice([-1, -1, -2, -2, 0, 5, 0xffff, 0x12345, ctx.u32()]))
    return [obj]


def g_c00c7(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1017e5a8, r.choice([0, 1, 5, 0x7e, 0x7f, 0x80, -1 & 0xffffffff, 0x100]))
    ctx.m.uc.mem_write(0x109a8640, rb(ctx, 0x54 * 0x82))
    for i in range(10):
        w32(ctx, 0x109e8b40 + 4 * i, newbuf(ctx, 0x40))
    obj = newbuf(ctx, 0x100)
    put(ctx, obj + 4, 'I', r.randrange(0, 10))
    for j in range(3):
        if r.random() < 0.7:
            ST['outs'][(0x100c0158, j)] = rb(ctx, 4)
    return [obj, ctx.u32(), r.choice([0, 1, 0x7fff, 0xffff, ctx.u32()]), ctx.u32()]


def g_255e(ctx):
    begin(ctx)
    obj = newbuf(ctx, 0x100)
    return [obj]


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def angle(ctx):
    r = ctx.rng
    return r.choice([0.0, 1e-8, -1e-8, 1e-7, -1e-7, 1.0000001e-7, 0.5, 30.0, -45.0, 45.0, 90.0, 180.0, -180.0, 270.0, 359.0, 720.5,
                     r.uniform(-360, 360), r.uniform(-360, 360), r.uniform(-4, 4), r.uniform(-3000, 3000)])


def tramp_code(target, nargs):
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\xd9\x1d' + struct.pack('<I', OUTF)      # fstp dword [OUTF]
    code += b'\x31\xc0\x5d\xc3'
    return code


def g_144e9(ctx):
    begin(ctx)
    r = ctx.rng
    ctx.m.uc.mem_write(TRAMP, tramp_code(0x100144e9, 1))
    obj = newbuf(ctx, 0x200)
    rec = newbuf(ctx, 0x40)
    block = newbuf(ctx, 0x600)
    cur = r.choice([0, 1, 5, ctx.u32()])
    w32(ctx, 0x1017b8c8, cur)
    put(ctx, obj + 4, 'I', cur if r.random() < 0.5 else cur + 1)
    put(ctx, obj + 0x28, 'I', block)
    put(ctx, block + 0x50c, 'I', r.choice([0, 1, 7]))
    put(ctx, obj + 0x54, 'I', rec)
    if r.random() < 0.5:
        base = r.uniform(-300, 300)
        d = r.choice([45.0, 45.5, 44.5, -45.0, -45.5, -44.5, 0.0, 90.0, -90.0, 179.0, -179.0, 180.0, 359.0, 400.0, -400.0,
                      r.uniform(-60, 60), r.uniform(-200, 200), r.uniform(-720, 720)])
        a, b = base + d, base
    else:
        a, b = r.uniform(-1000, 1000), r.uniform(-1000, 1000)
    wf(ctx, obj + 0x110, a)
    wf(ctx, obj + 0xe8, a)
    wf(ctx, obj + 0x68, b)
    return [obj]


def g_68ce0(ctx):
    begin(ctx)
    r = ctx.rng

    def p():
        return 0 if r.random() < 0.06 else zbuf(ctx, 8)
    return [fb(fl(ctx) * r.choice([1, 1, 100])), fb(fl(ctx) * r.choice([1, 1, 100])), ctx.u32(), fb(angle(ctx)), p(), p(), p()]


def g_84a17(ctx):
    begin(ctx)
    ctx.m.uc.mem_write(0x10370d20, rb(ctx, 0x320 * 4))
    ctx.m.uc.mem_write(0x1036f860, rb(ctx, 0x320 * 4))
    return []


def g_9cf67(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    wf(ctx, obj + 4, fl(ctx) * 10)
    wf(ctx, obj + 0x1c, r.choice([0.0, 1e-8, -1e-8, 1e-7, -1e-7, 1.0000001e-7, 1.0, -1.0, 0.5, 3.0, fl(ctx) * 20]))
    wf(ctx, obj + 0x30, fl(ctx) * 10)
    if r.random() < 0.5:
        put(ctx, obj + 0x34, 'I', r.choice([0, 1]))
    return [obj, fb(fl(ctx) * 10)]


def g_c0379(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    rec = newbuf(ctx, 0x40)
    put(ctx, obj + 0x54, 'I', rec)
    put(ctx, rec + 8, 'I', r.choice([0, 0x80000000, 0x3f800000, 0xbf800000, 0x7fc00000, 0x7f800000, ctx.u32(), 0x3acccccd]))
    f = r.choice([0.0, 5.0, -5.0, 5.0000005, -5.0000005, 4.9999995, 6.0, -6.0, 100.0, -1e9, 1e-3, r.uniform(-10, 10), r.uniform(-10, 10)])
    return [obj, fb(f)]


def g_c312c(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x20)
    put(ctx, obj + 0x34, 'I', r.randrange(0, 6))
    for i in range(6):
        w32(ctx, 0x10138bc8 + i * 12, r.choice([0, 0, 1, ctx.u32()]))
    put(ctx, obj + 2, 'H', r.choice([0x50, 0x5f, 0x40, 0x60, 0xf0, 0x150, r.getrandbits(16)]))
    v = r.choice([0.766, 0.76599997, 0.7660001, 0.0, -0.0, 1.0, -1.0, 1e-8, 1e-7, 0.5, 2.0, r.uniform(-2, 2)])
    wf(ctx, 0x1015dd5c, v)
    return [obj]


def g_cca20(ctx):
    begin(ctx)
    r = ctx.rng
    # the C library's floating point conversion table, as its start-up code (0x100fc640) leaves it
    for k, v in enumerate((0x10100f30, 0x10100af0, 0x10100b80, 0x10100a90, 0x10100b60, 0x10100f30)):
        w32(ctx, 0x10159700 + 4 * k, v)
    n = r.choice([0, 1, -1, 0x9f150, 0x9f150 - 1, -0x9f150, 0x2a6c, -0x2a6c, 0x2a6b, 181, 180, r.randrange(0, 0x9f150 * 5),
                  r.randrange(-0x9f150 * 5, 0), r.randrange(0, 1 << 31), -r.randrange(0, 1 << 31), r.randrange(0, 100000)])
    return [n & 0xffffffff]


def d6_world(ctx):
    r = ctx.rng
    obj = newbuf(ctx, 0x200)
    target = newbuf(ctx, 0x100)
    wf(ctx, target + 0x70, fl(ctx) * 10, fl(ctx) * 10, fl(ctx) * 10)
    put(ctx, obj + 0x4c, 'I', target)
    flt = newbuf(ctx, 8)
    wf(ctx, flt, fl(ctx) * 5)
    w32(ctx, 0x1015ef1c, 0 if r.random() < 0.4 else flt)
    for j in range(1, 4):
        ST['outs'][(0x100d6f92, j - 1)] = struct.pack('<f', fl(ctx) * 20)
    return obj, target


def g_d6f16(ctx):
    begin(ctx)
    obj, target = d6_world(ctx)
    return [obj, newbuf(ctx, 0x100)]


def g_d6ea7(ctx):
    begin(ctx)
    r = ctx.rng
    obj, target = d6_world(ctx)
    block = newbuf(ctx, 0x600)
    put(ctx, obj + 0x28, 'I', block)
    put(ctx, obj + 0xb4, 'I', r.choice([0, 1, 100, 0xffffffff, ctx.u32()]))
    q = newbuf(ctx, 0x140)
    k = r.randrange(0, 4)
    put(ctx, q, 'I', 0xffffffff if r.random() < 0.5 else r.choice([0, 1, 2]))
    put(ctx, q + 0x11c, 'I', k)
    other = newbuf(ctx, 0x100)
    wf(ctx, other + 0x70, fl(ctx) * 10, fl(ctx) * 10, fl(ctx) * 10)
    for i in range(4):
        put(ctx, block + 0x498 + 4 * i, 'I', other)         # the original leaves a, b, c unwritten when this is 0
    return [obj, newbuf(ctx, 8), newbuf(ctx, 8), newbuf(ctx, 8), q]


def g_354ff(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x109e3d2c, r.choice([0, CB_SLOT, CB_SLOT]))
    w32(ctx, 0x1017b940, r.choice([0, 0, 1]))
    w32(ctx, 0x1015e534, r.choice([0, newbuf(ctx, 16), newbuf(ctx, 16)]))
    w32(ctx, 0x109e3e00, r.choice([0, 1, ctx.u32()]))
    w32(ctx, 0x1015e538, ctx.u32())
    w32(ctx, 0x1017d180, r.choice([0, newbuf(ctx, 16)]))
    o1 = zbuf(ctx, 0x40)
    put(ctx, o1 + 0x14, 'I', CB_14)
    put(ctx, o1 + 0x18, 'I', CB_18)
    o2 = zbuf(ctx, 0x40)
    put(ctx, o2 + 0x10, 'I', CB_10)
    w32(ctx, 0x1015e68c, o1 if r.random() < 0.9 else 0)
    w32(ctx, 0x1015e688, o2 if r.random() < 0.8 else 0)
    w32(ctx, 0x1017b920, r.choice([0, 1, 1]) if r.random() > 0.1 else 0)
    if G(ctx, 0x1015e68c) == 0:
        w32(ctx, 0x1017b920, 0)
    w32(ctx, 0x1013a2c8, ctx.u32())
    return []


def G(ctx, addr):
    return r32(ctx, addr)


def API_alloc(n, a):
    return 0 if ST.get('alloc_fail') else ARENA + 0x800 * (n % 3)


def API_lib(n, a):
    return 0x60000000 + 0x1000 * (n + 1)


API = {'HeapAlloc': API_alloc, 'LoadLibraryA': API_lib, 'GetProcAddress': lambda n, a: 0x61000000 + 0x100 * n}


def S(name, addr, gen, trials=400, **kw):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials, api=API)
    d.update(kw)
    return d


SPECS = [
    S('Sim_10007ed5', 0x10007ed5, g_7ed5, void=True),
    S('Sim_10007f49', 0x10007f49, g_7f49, void=True),
    S('Sim_10010496', 0x10010496, g_slot, 500, void=True),
    S('Sim_100105cb', 0x100105cb, g_slot, 500, void=True),
    S('Sim_10010700', 0x10010700, g_slot, 500, void=True),
    S('Sim_10013eba', 0x10013eba, g_13eba),
    S('Sim_1001555c', 0x1001555c, g_1555c, void=True),
    S('Sim_100159cf', 0x100159cf, g_159cf, void=True),
    S('Sim_1001bb1f', 0x1001bb1f, g_1bb1f, 500, void=True),
    S('Sim_1001dcff', 0x1001dcff, g_1dcff, void=True),
    S('Sim_1001ffc8', 0x1001ffc8, g_1ffc8),
    S('Sim_10029ab3', 0x10029ab3, g_29ab3, void=True),
    S('Sim_1002cde1', 0x1002cde1, g_2cde1),
    S('Sim_1002fad6', 0x1002fad6, g_2fad6),
    S('Sim_1002fb3d', 0x1002fb3d, g_2fb3d, void=True),
    S('Sim_1003a052', 0x1003a052, g_3a052),
    S('Sim_10041247', 0x10041247, g_41247, void=True),
    S('Sim_100474db', 0x100474db, g_474db),
    S('Sim_1004dcaa', 0x1004dcaa, g_4dcaa),
    S('Sim_1004e1c6', 0x1004e1c6, g_4e1c6),
    S('Sim_1004edca', 0x1004edca, g_4edca),
    S('Sim_1004f0d6', 0x1004f0d6, g_4f0d6, void=True),
    S('Sim_1005264d', 0x1005264d, g_5264d, void=True),
    S('Sim_1005af06', 0x1005af06, g_5af06),
    S('Sim_1008d36c', 0x1008d36c, g_8d36c),
    S('Sim_100988fc', 0x100988fc, g_988fc, void=True),
    S('Sim_1009964b', 0x1009964b, g_9964b),
    S('Sim_100a6414', 0x100a6414, g_6414, void=True),
    S('Sim_100a9ba7', 0x100a9ba7, g_9ba7, void=True),
    S('Sim_100aa127', 0x100aa127, g_aa127, void=True),
    S('Sim_100aae0f', 0x100aae0f, g_aae0f),
    S('Sim_100aad46', 0x100aad46, g_aad),
    S('Sim_100aadaa', 0x100aadaa, g_aad),
    S('Sim_100ab0d6', 0x100ab0d6, g_ab0d6, void=True),
    S('Sim_100ba5d1', 0x100ba5d1, g_ba5d1, void=True),
    S('Sim_100bcbcf', 0x100bcbcf, g_bcbcf),
    S('Sim_100bee33', 0x100bee33, g_bee33),
    S('Sim_100c00c7', 0x100c00c7, g_c00c7, void=True),
    S('Sim_100d255e', 0x100d255e, g_255e, void=True),
    S('Sim_100354ff', 0x100354ff, g_354ff, 500, void=True),
    S('Sim_100144e9', TRAMP, g_144e9, 500, c='Test_100144e9'),
    S('Sim_10068ce0', 0x10068ce0, g_68ce0, 500, void=True),
    S('Sim_10084a17', 0x10084a17, g_84a17, 30),
    S('Sim_1009cf67', 0x1009cf67, g_9cf67, 500, void=True),
    S('Sim_100c0379', 0x100c0379, g_c0379, 500),
    S('Sim_100c312c', 0x100c312c, g_c312c, 500),
    S('Sim_100cca20', 0x100cca20, g_cca20, 500),
    S('Sim_100d6f16', 0x100d6f16, g_d6f16, 500, void=True),
    S('Sim_100d6ea7', 0x100d6ea7, g_d6ea7, 500, void=True),
]
