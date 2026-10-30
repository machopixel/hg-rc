# Round 8B, helper 4: bit-exact proofs (no float tolerance) for the float functions of rounds 5a and 5z that were
# only ever checked with fp=True (see tools/round8b/README.md). Self-contained: the helpers and generators needed
# are copied from spec_w5a.py / spec_w5z.py (unchanged there) and extended with the special float values
# (signed zeros, quiet NaNs, denormals, values at and next to the thresholds).
#
# Helpers of the DLL:
#  * the pure library code (sqrt 0x100fca04, fabs 0x100fcb70, fmod 0x100fcb30, _ftol 0x100fcb08, the sine table
#    lookup 0x10084766, sin 0x100ff644, atan 0x10100044) runs for real in the emulator for both versions, with the
#    Windows control word 0x27f (53-bit precision) like the real program;
#  * 0x100ff860 (assert), 0x1001403a (set the object's key) and 0x100a9ba7 (refresh a transform) are stubs: a `ret`
#    over the first byte plus a code hook that appends ('stub_<address>', arguments) to the checker's call log
#    (compared like the Windows calls); stack addresses are logged as 'stack'.
# The sine table lookup loops forever on infinities (x - 360 == x), so no infinity ever reaches it.
#
# NOTE Trig_InitTables (0x100846e0) is listed for the record only: the emulator computes fsin/fpatan in double,
# the real x87 returns 64-bit-mantissa results that the original stores/multiplies directly, so an emulator
# proof says nothing; it is unhooked (conv_w5a.inc).
import math
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG

SOURCES = [
    # spec_w5a.py
    'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1001/w5a_misc.c', 'sim/unsorted_1002/w5a_misc.c',
    'sim/unsorted_1003/w5a_misc.c', 'sim/unsorted_1004/w5a_misc.c', 'sim/unsorted_1005/w5a_misc.c',
    'sim/unsorted_1006/w5a_math.c', 'sim/unsorted_1007/w5a_misc.c', 'sim/unsorted_1008/w5a_misc.c',
    'sim/unsorted_1009/w5a_misc.c', 'sim/unsorted_100a/w5a_misc.c', 'sim/unsorted_100b/w5a_misc.c',
    'sim/unsorted_100c/w5a_misc.c',
    '../tools/verify/w5a_test.c',
    'sim/unsorted_1000/w2e_names.c', 'sim/unsorted_1000/w2i_misc.c', 'sim/unsorted_1000/w2a_fields.c',
    'sim/unsorted_1001/w2e_tables.c', 'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1001/w2e_names.c',
    'sim/core/w2a_timers.c', 'sim/unsorted_1003/w2e_misc.c', 'sim/util/ring.c', 'sim/unsorted_1002/w2e_object.c',
    'sim/unsorted_100d/w2f_records.c',
    'sim/unsorted_1002/w2l_tables.c', 'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1002/w2i_state.c',
    'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1003/w2e_mono.c',
    'sim/unsorted_1004/state.c', 'sim/unsorted_1004/w2j_misc.c', 'sim/unsorted_1004/tables.c',
    'sim/unsorted_1007/w2e_state.c', 'sim/unsorted_1008/w3o_length.c',
    'sim/unsorted_100c/w3o_misc.c', 'sim/unsorted_100c/w2e_tables.c',
    'sim/unsorted_100f/w2e_string.c', 'sim/util/lookup.c', 'sim/util/record.c', 'sim/util/field.c',
    'sim/view/objlist.c', 'sim/win/w4a_heap.c', 'sim/win/w4u_cdaudio.c', 'sim/core/timer.c',
    # spec_w5z.py (without the files already listed)
    'sim/unsorted_1000/w5z_calls.c', 'sim/unsorted_1001/w5z_calls.c', 'sim/unsorted_1002/w5z_calls.c',
    'sim/unsorted_1004/w5z_calls.c', 'sim/unsorted_1006/w5z_calls.c', 'sim/unsorted_1007/w5z_calls.c',
    'sim/unsorted_1008/w5z_calls.c', 'sim/unsorted_1009/w5z_calls.c', 'sim/unsorted_100a/w5z_calls.c',
    'sim/unsorted_100b/w5z_calls.c', 'sim/unsorted_100c/w5z_calls.c', 'sim/unsorted_100d/w5z_calls.c',
    'sim/win/w4v_heap.c', 'sim/view/w2l_lists.c', 'sim/view/w2d_view.c',
    'sim/view/matrix.c', 'sim/view/viewrect.c', 'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_100d/w2j_nodes.c',
    'sim/unsorted_1000/w3s_model.c', 'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1008/w4w_display.c',
    'sim/unsorted_1000/w2j_slots.c', 'sim/unsorted_1001/w2j_misc.c',
    'sim/unsorted_1005/w3s_groups.c', 'sim/unsorted_100b/w4v_wrappers.c', 'sim/unsorted_1000/w2e_mono.c',
    'sim/unsorted_1007/node_flag.c', 'sim/unsorted_1001/w3s_group.c',
    'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_100c/w4v_timers.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000        # Test_100c34b7 (w5a_test.c) and the trampoline store the st(0) result here
TRAMP = 0x00F00800

# helper address -> number of dword arguments
STUBS = {0x100ff860: 3, 0x1001403a: 2, 0x100a9ba7: 1}
# 0x1001403a reads its key argument as a word (the original passes garbage in the top half)
MASK = {(0x1001403a, 1): 0xffff}
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<H', v & 0xffff))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def put(ctx, addr, fmt, *vals):
    out = b''
    for f, v in zip(fmt, vals):
        size = struct.calcsize('<' + f)
        out += (v & ((1 << (8 * size)) - 1)).to_bytes(size, 'little')
    ctx.m.uc.mem_write(addr, out)


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


# ---- float values ---------------------------------------------------------------------------------
def fb(f):
    """Bits of a Python float rounded to float32."""
    return struct.unpack('<I', struct.pack('<f', f))[0]


def bf(b):
    return struct.unpack('<f', struct.pack('<I', b & 0xffffffff))[0]


def nxt(f, k=1):
    """The float32 k steps above f (k < 0: below), by bit arithmetic on the ordered representation."""
    b = fb(f)
    o = b if b < 0x80000000 else -(b & 0x7fffffff)
    o += k
    return o & 0xffffffff if o >= 0 else (0x80000000 | (-o))


NEG0 = 0x80000000
QNANS = [0x7fc00000, 0xffc00000, 0x7fc12345, 0xffe00001, 0x7fffffff, 0x7fc00001]
DENORMS = [0x00000001, 0x80000001, 0x007fffff, 0x807fffff, 0x00400000, 0x00012345]


def special(r):
    """A special float32 (bits): signed zeros, quiet NaNs, denormals, tiny and huge normals."""
    return r.choice([0, NEG0, r.choice(QNANS), r.choice(DENORMS), 0x00800000, 0x80800000, 0x7f7fffff, 0xff7fffff,
                     fb(1e-30), fb(-1e-30), fb(3e38), fb(-3e38)])


def inf_bits(r):
    return r.choice([0x7f800000, 0xff800000])


def fval(ctx, spread, extra=(), inf=False):
    """Bits of a float: mostly ordinary values within +-spread, sometimes special values and given picks
    (and infinities when asked)."""
    r = ctx.rng
    c = r.random()
    if inf and c < 0.03:
        return inf_bits(r)
    if c < 0.12:
        return special(r)
    if c < 0.3 and extra:
        return r.choice(extra)
    if c < 0.4:
        return fb(r.choice([0.0, 1.0, -1.0, 0.5, -0.5, 2.0, 90.0, -90.0, 180.0, -180.0, 360.0, -360.0, 270.0]))
    if c < 0.5:
        return r.getrandbits(32) & 0x7fffffff | (r.getrandbits(1) << 31) if r.random() < 0.3 else fb(r.uniform(-1, 1))
    return fb(r.uniform(-spread, spread))


def finite_for_sintab(b):
    """Keeps values out of the sine table lookup's endless loop (infinities) and its very long loops (huge)."""
    f = bf(b)
    if math.isnan(f):
        return b
    if math.isinf(f) or abs(f) > 1e6:
        return fb(math.copysign(1e6, f) if f == f else 0.0)
    return b


def wfb(ctx, addr, *bits):
    w32(ctx, addr, *bits)


# ---- stubs ---------------------------------------------------------------------------------------
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
        m.api_log.append(('stub_%x' % addr, tuple(args)))
        rets = ST['rets'].get(addr) or [0]
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


def begin(ctx):
    """Installs the stubs (once per machine), the control word 0x27f and the answers of this trial."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_r8b_h4', False):
        m._r8b_h4 = True
        for addr, nargs in STUBS.items():
            m.raw.mem_write(addr, b'\xc3')
            m.raw.hook_add(UC_HOOK_CODE, _make_cb(m, addr, nargs), None, addr, addr)
        m.raw.mem_write(TRAMP, tramp_float(0x100c34b7, 2))
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}


def sine_table(ctx):
    """The 513 floats at 0x103719a0 (entry i = sin(i * pi / 1024)), as Trig_InitTables makes them."""
    wb(ctx, 0x103719a0, struct.pack('<513f', *[math.sin(i * 0.003067961572265625) for i in range(513)]))


# ---- generators -----------------------------------------------------------------------------------
def g_ed77(ctx):
    """Sim_1000ed77(v): trunc(sintab(cam[+0xc] - v) * 10000) clamped to +-7500."""
    begin(ctx)
    r = ctx.rng
    sine_table(ctx)
    cam = zbuf(ctx, 0x40)
    a = finite_for_sintab(fval(ctx, r.choice([30, 400, 2000]), [fb(48.590378), fb(131.4096), fb(-48.590378)]))
    v = finite_for_sintab(fval(ctx, r.choice([30, 400, 2000])))
    if r.random() < 0.2:
        # differences right at the clamp (sin = 0.75 at about 48.59 degrees) and at the table's 90/180 splits
        v = finite_for_sintab(nxt(bf(a) - r.choice([48.590378, 131.4096, -48.590378, 90.0, 180.0, 0.0]),
                                  r.randrange(-3, 4)))
    w32(ctx, cam + 0xc, a)
    w32(ctx, 0x1014cae0, cam)
    return [v]


def g_60882(ctx):
    """Sim_10060882(&x, &y, &z): normalises (x, y, z) in place."""
    begin(ctx)
    r = ctx.rng
    vals = []
    for _ in range(3):
        c = r.random()
        if c < 0.12:
            vals.append(special(r))
        elif c < 0.15:
            vals.append(inf_bits(r))
        else:
            vals.append(fb(r.choice([0.0, 1.0, -1.0, 0.5, r.uniform(-100, 100), r.uniform(-1, 1), r.uniform(-1e6, 1e6),
                                     1e-3, -0.0, r.uniform(-1e18, 1e18), r.uniform(-1e-20, 1e-20)])))
    if r.random() < 0.08:
        vals = [r.choice([0, NEG0]) for _ in range(3)]
    if r.random() < 0.08:
        vals = [r.choice(QNANS) for _ in range(3)]
    ptrs = []
    for v in vals:
        p = ctx.buf(8)
        w32(ctx, p, v)
        ptrs.append(p)
    if r.random() < 0.05:
        ptrs = [ptrs[0]] * 3            # aliased: the same float three times
    return ptrs


def g_610ae(ctx):
    """Sim_100610ae(a, b, c, ang, out): quaternion of a rotation by ang about (a, b, c)."""
    begin(ctx)
    r = ctx.rng
    sine_table(ctx)
    out = ctx.buf(0x20)
    ang = finite_for_sintab(fval(ctx, 720, [fb(1e-6), fb(2e-7), nxt(2e-7, -1), fb(-2e-7), fb(180.0), fb(-180.0),
                                            fb(720.0), nxt(180.0, 1), nxt(360.0, -1)]))
    args = [fval(ctx, 10, inf=True), fval(ctx, 10, inf=True), fval(ctx, 10, inf=True), ang]
    if r.random() < 0.1:
        args[:3] = r.choice([[fb(1.0), 0, 0], [0, fb(1.0), 0], [0, 0, fb(1.0)], [0, 0, 0]])
    return args + [out]


def g_610ae_null(ctx):
    """The same with out == 0: the assert is called first (stub), then the writes go to page 0 (mapped here)."""
    m = ctx.m
    if not getattr(m, 'page0_mapped', False):
        m.uc.mem_map(0, 0x1000)
        m.wr.append((0, 0x1000))
        m.page0_mapped = True
        m.snap0 = m.dump()
        m._index()
    wb(ctx, 0, bytes([0x55]) * 0x1000)
    args = g_610ae(ctx)
    return args[:4] + [0]


def g_68c48(ctx):
    """Sim_10068c48(a, bits, c, ang, p1, p2, p3): rotates (a, c) by ang when |ang| >= 1e-7."""
    begin(ctx)
    r = ctx.rng
    sine_table(ctx)
    outs = [ctx.buf(8), ctx.buf(8), ctx.buf(8)]
    if r.random() < 0.1:
        outs[r.randrange(0, 3)] = 0
    thr = 1.0000000116860974e-07
    ang = finite_for_sintab(fval(ctx, 720, [fb(thr), nxt(thr, -1), nxt(thr, 1), fb(-thr), nxt(-thr, 1),
                                            nxt(-thr, -1), fb(1e-8), fb(-1e-8), fb(90.0), fb(-90.0), fb(270.0)]))
    return [fval(ctx, 10, inf=True), ctx.u32(), fval(ctx, 10, inf=True), ang] + outs


def g_69f68(ctx):
    """Sim_10069f68(obj): inner[+0x110] = fmod(fmod(inner[+0x68] + obj[+0xc], 360) + 360, 360) unless +0x50c == 1."""
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x600)
    inner = newbuf(ctx, 0x200)
    put(ctx, obj, 'I', inner)
    put(ctx, obj + 0x50c, 'I', r.choice([0, 1, 1, 2, ctx.u32()]) if r.random() < 0.3 else 0)
    picks = [fb(360.0), fb(-360.0), nxt(360.0, -1), nxt(360.0, 1), nxt(-360.0, 1), fb(359.5), fb(720.0), fb(1e7)]
    a = fval(ctx, 720, picks)
    b = fval(ctx, 720, picks)
    if r.random() < 0.04:
        a = inf_bits(r)
    if r.random() < 0.15:
        b = fb(r.choice([0.0, -0.0, 360.0, -360.0]) - bf(a)) if not math.isnan(bf(a)) and abs(bf(a)) < 1e30 else b
    w32(ctx, inner + 0x68, a)
    w32(ctx, obj + 0xc, b)
    return [obj]


def g_c34b7(ctx):
    """Sim_100c34b7(obj, key) through the st(0) trampoline: fmod(Sim_10015266(obj) + 360, 360) under another key."""
    begin(ctx)
    r = ctx.rng
    cur = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, cur)
    obj = zbuf(ctx, 0x400)
    put(ctx, obj + 4, 'I', r.choice([cur, cur, r.randrange(0, 8)]))
    sub = zbuf(ctx, 0x600)
    put(ctx, sub + 0x50c, 'I', r.choice([0, 1, 1, 2]))
    put(ctx, obj + 0x28, 'I', sub)
    near = [fb(180.0), nxt(180.0, 1), nxt(180.0, -1), fb(-180.0), nxt(-180.0, 1), nxt(-180.0, -1), fb(360.0),
            nxt(360.0, -1), fb(-360.0), fb(0.0), NEG0]
    w32(ctx, obj + 0x110, fval(ctx, 720, near, inf=True))
    w32(ctx, obj + 0xe8, fval(ctx, 720, near, inf=True))
    w32(ctx, obj + 0x68, fval(ctx, 720, near) if r.random() < 0.5 else r.choice([0, NEG0, fb(180.0), fb(90.0)]))
    put(ctx, obj + 0xf0, 'I', ctx.u32())
    key = r.choice([0, 1, 0x7fff, 0x8000, 0xffff, ctx.u32()])
    return [obj, key]


# ---- 0x100aa307 (spec_w5z.py) ----------------------------------------------------------------------
def fvec(ctx, n):
    r = ctx.rng
    out = []
    for _ in range(n):
        c = r.random()
        if c < 0.01:
            out.append(inf_bits(r))
        elif c < 0.06:
            out.append(special(r))
        elif c < 0.2:
            out.append(fb(r.choice([0.0, 1.0, -1.0, 0.5, 2.0])))
        elif c < 0.6:
            out.append(fb(r.uniform(-100, 100)))
        elif c < 0.7:
            out.append(fb(r.uniform(-1e30, 1e30)))
        else:
            out.append(fb(r.uniform(-1, 1)))
    return struct.pack('<%dI' % n, *out)


def model_block(ctx, nv, nn):
    r = ctx.rng
    off = 0x18 + nv * 0x58 + r.choice([0, 0, 8])
    b = ctx.buf(off + nn * 0x28 + 16, 'zero')
    wb(ctx, b + 0x18, fvec(ctx, nv * 22))
    wb(ctx, b + off, fvec(ctx, nn * 10))
    w16(ctx, b + 4, nv)
    w16(ctx, b + 6, nn)
    w32(ctx, b + 8, off)
    return b


def g_aa307(ctx):
    """Sim_100aa307(obj): 0x100a9ba7(obj[+0x28]) (stub), then the model block obj[+0x30] transformed by +0x4c of it."""
    r = ctx.rng
    begin(ctx)
    obj = newbuf(ctx, 0x200)
    blk = model_block(ctx, r.choice([0, 1, 2, 3, 6]), r.choice([0, 1, 2, 5]))
    t = zbuf(ctx, 0x4c + 48 + 16)
    wb(ctx, t + 0x4c, fvec(ctx, 12))
    w32(ctx, obj + 0x28, t)
    w32(ctx, obj + 0x30, blk)
    w32(ctx, obj + 0x9c, ctx.u32())
    return [obj]


def SF(name, orig, c, gen, trials, **kw):
    d = dict(name=name, orig=orig, c=c, gen=gen, trials=trials)
    d.update(kw)
    return d


# every entry is compared exactly (no fp / tol)
SPECS = [
    SF('Sim_1000ed77', 0x1000ed77, 'Sim_1000ed77', g_ed77, 3000),
    SF('Sim_10060882', 0x10060882, 'Sim_10060882', g_60882, 3000, void=True),
    SF('Sim_100610ae', 0x100610ae, 'Sim_100610ae', g_610ae, 3000, void=True),
    SF('Sim_100610ae_null', 0x100610ae, 'Sim_100610ae', g_610ae_null, 300, void=True),
    SF('Sim_10068c48', 0x10068c48, 'Sim_10068c48', g_68c48, 3000, void=True),
    SF('Sim_10069f68', 0x10069f68, 'Sim_10069f68', g_69f68, 3000, void=True),
    SF('Sim_100c34b7', TRAMP, 'Test_100c34b7', g_c34b7, 3000),
    SF('Sim_100aa307', 0x100aa307, 'Sim_100aa307', g_aa307, 2000, void=True),
]
