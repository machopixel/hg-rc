# Differential tests for round 8B, helper h5: the eight x87 functions of round 5 letter b that were only proven with the
# float tolerance (spec_w5b.py, fp=True), checked here BIT FOR BIT (no fp, no tol) with the real build's -msse2
# -mfpmath=sse. The environment (helper stubs, the C library routines that run for real with the Windows control word
# 0x27f, the st(0) trampoline for Sim_10047e1a) is copied from spec_w5b.py; the generators are those of spec_w5b.py
# widened with signed zeros, quiet NaNs, infinities, denormals and values next to the thresholds the code compares with.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG

SOURCES = [
    'sim/unsorted_1000/w5b_calls.c', 'sim/unsorted_1001/w5b_calls.c', 'sim/unsorted_1002/w5b_calls.c',
    'sim/unsorted_1003/w5b_calls.c', 'sim/unsorted_1004/w5b_calls.c', 'sim/unsorted_1005/w5b_calls.c',
    'sim/unsorted_1006/w5b_calls.c', 'sim/unsorted_1008/w5b_calls.c', 'sim/unsorted_1009/w5b_calls.c',
    'sim/unsorted_100a/w5b_calls.c', 'sim/unsorted_100b/w5b_calls.c', 'sim/unsorted_100c/w5b_calls.c',
    'sim/unsorted_100d/w5b_calls.c', 'sim/unsorted_100e/w5b_calls.c',
    '../tools/verify/w5b_test.c',
    # existing converted code the functions above call
    'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_100f/w2e_string.c',
    'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2j_slots.c',
    'sim/util/lookup.c', 'sim/unsorted_1001/w2j_misc.c',
    'sim/unsorted_1001/w2h_objects.c', 'sim/unsorted_1004/tables.c', 'sim/view/objlist.c',
    'sim/unsorted_1001/w2l_misc.c', 'sim/unsorted_1004/w3p_net.c', 'sim/unsorted_1007/w3p_net.c',
    'sim/unsorted_100a/w2f_links.c', 'sim/unsorted_100a/w2l_small.c',
    'sim/unsorted_1002/w3o_vectors.c', 'sim/unsorted_1002/w3q_collide.c',
    'sim/unsorted_1008/w3s_grid.c', 'sim/unsorted_1002/w2h_misc.c',
    'sim/win/w4a_heap.c', 'sim/win/w4u_gdi.c', 'sim/unsorted_1004/w2l_values.c',
    'sim/view/viewrect.c', 'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_100c/entries.c',
    'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1008/w4v_fade.c',
    # what those call in turn
    'sim/win/w4v_heap.c', 'sim/unsorted_1003/w3q_grid.c', 'sim/unsorted_1003/entry.c', 'sim/unsorted_1003/records.c',
    'sim/unsorted_1004/w2i_misc.c', 'sim/unsorted_1004/w2h_record.c', 'sim/unsorted_1004/fields.c',
    'sim/unsorted_1005/w2h_tables.c', 'sim/unsorted_1007/w2f_misc.c', 'sim/unsorted_1008/w3o_length.c',
    'sim/unsorted_1008/cell_link.c', 'sim/view/w2d_view.c', 'sim/view/w2l_lists.c', 'sim/util/ring.c',
    'sim/util/field.c', 'sim/unsorted_100d/w2f_records.c', 'sim/view/matrix.c', 'sim/unsorted_100d/w2i_select.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000
TRAMP = 0x00F00800
DPSEND = 0x00F00200      # the "dpSend" function pointer target of Sim_100409be (a `ret` with a hook)
CALLBACK = 0x00F00210    # the callback of Sim_100aab03
DRIVER = 0x00F00220      # the driver block function [[0x1015e688] + 0x18] used by Sim_1008c803 and Sim_1008c5e0

ST = {}


# ---- small helpers ------------------------------------------------------------------------------
def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def put(ctx, addr, fmt, *vals):
    out = b''
    for f, v in zip(fmt, vals):
        size = struct.calcsize('<' + f)
        out += (v & ((1 << (8 * size)) - 1)).to_bytes(size, 'little')
    ctx.m.uc.mem_write(addr, out)


def wf(ctx, addr, *v):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(v), *v))


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def s32(v):
    v &= 0xffffffff
    return v - (1 << 32) if v >= 1 << 31 else v


def cstr(ctx, data, extra=24):
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rb(ctx, extra))


def rfl(r, spread=100.0, special=0.12):
    """An ordinary float (no NaN, no infinity)."""
    c = r.random()
    if c < special:
        return r.choice([0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 2.0, 1e-7, -1e-7, 100.0, -100.0, 1e10, -1e10, 3.0e38, 1e-30, 1e5])
    return r.uniform(-spread, spread)


def rfbits(r, spread=100.0, special=0.12):
    return fbits(rfl(r, spread, special))


# ---- stubs --------------------------------------------------------------------------------------
# helper address -> (number of dword arguments, {argument number: mask})
STUBS = {
    0x1000b070: (2, {}), 0x1000b587: (2, {}), 0x10016117: (2, {}), 0x100200d1: (2, {}), 0x10020256: (2, {}),
    0x10017b3e: (2, {}), 0x10016de7: (2, {1: 0xffff}), 0x10016632: (1, {}),
    0x10027fb8: (1, {}), 0x10028d9f: (3, {}), 0x100a77a7: (2, {}), 0x10031acd: (5, {}), 0x100d3830: (2, {}),
    0x100fefe0: (1, {}), 0x100fef30: (1, {}), 0x100bc280: (2, {}), 0x100fd7e0: (3, {}),
    0x10028b90: (2, {}), 0x10062386: (0, {}), 0x1006a12d: (2, {}), 0x10083801: (0, {}), 0x100f01cc: (2, {}),
    0x100eb6e8: (6, {}), 0x100fe830: (4, {}), 0x10105750: (3, {}), 0x100a7f8c: (3, {2: 0xffff}),
    0x10060b76: (2, {}), 0x10060d2e: (2, {}), 0x100aa8cb: (1, {}), 0x1001403a: (2, {1: 0xffff}),
    0x100d3510: (3, {}), 0x100e2aa3: (5, {}), 0x100e2776: (3, {}),
}


def _fread(m, uc, a, n):
    """fread(buf, 1, 1, file): hands out the characters of this trial's line one at a time, then end of file."""
    seq = ST['fread']
    if n < len(seq):
        m.uc.mem_write(a[0], bytes([seq[n]]))
        return 1
    return 0


def _write(m, uc, a, n):
    if n == ST['write_fail']:
        return ST['write_fail_value'] & 0xffffffff
    return a[2]


def _b76(m, uc, a, n):
    """0x10060b76(obj + 0x3c, obj + 0xc): clobbers the dwords obj[0x30..0x38] the caller must put back."""
    m.uc.mem_write(a[1] + 0x24, ST['garbage'])
    return 0


def _e2aa3(m, uc, a, n):
    """0x100e2aa3(a, b, &result, d, &x): stores a result pointer through the third argument."""
    m.uc.mem_write(a[2], struct.pack('<I', ST['e2_result']))
    m.uc.mem_write(a[4], struct.pack('<I', 0x1234))
    return ST['e2_ret']


FNS = {0x100fe830: _fread, 0x10105750: _write, 0x10060b76: _b76, 0x100e2aa3: _e2aa3}


def _make_cb(m, addr, nargs, mask):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        raw = [struct.unpack('<I', bytes(uc.mem_read(esp + 4 + 4 * k, 4)))[0] for k in range(nargs)]
        args = []
        for k, v in enumerate(raw):
            if STACK_LO <= v <= STACK_HI:
                v = 'stack'
            elif k in mask:
                v &= mask[k]
            args.append(v)
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % addr, tuple(args)))
        if addr in FNS:
            val = FNS[addr](m, uc, raw, n)
        else:
            rets = ST['rets'].get(addr) or [0]
            val = rets[n % len(rets)]
        uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
    return cb


def _special_cb(m, kind):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        a = struct.unpack('<6I', bytes(uc.mem_read(esp + 4, 24)))
        n = len(m.api_log)
        if kind == 'dp':
            # dpSend(session, from, to, flag, buf, len): the 7 bytes of the message are part of the log
            try:
                data = bytes(uc.mem_read(a[4], 7))
            except Exception:
                data = b'?'
            m.api_log.append(('dp', (a[0], a[1], a[2], a[3], a[4], a[5], data)))
            uc.reg_write(UC_X86_REG_EAX, ST['dp_ret'] & 0xffffffff)
        elif kind == 'cb':
            m.api_log.append(('callback', (a[0],)))
            uc.reg_write(UC_X86_REG_EAX, 0x5555)
        else:
            m.api_log.append(('driver', (a[0],)))
            uc.reg_write(UC_X86_REG_EAX, 0)
    return cb


def tramp_double(target, nargs):
    """Calls target(args as dwords) and stores st(0) as a double at OUTF."""
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\xdd\x1d' + struct.pack('<I', OUTF)       # fstp qword [OUTF]
    code += b'\x31\xc0\x5d\xc3'
    return code


def begin(ctx):
    """Installs the stubs (once per machine) and chooses the answers of this trial."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w5b', False):
        m._w5b = True
        for addr, (nargs, mask) in STUBS.items():
            m.raw.mem_write(addr, b'\xc3')
            m.raw.hook_add(UC_HOOK_CODE, _make_cb(m, addr, nargs, mask), None, addr, addr)
        for addr, kind in ((DPSEND, 'dp'), (CALLBACK, 'cb'), (DRIVER, 'driver')):
            m.raw.mem_write(addr, b'\xc3')
            m.raw.hook_add(UC_HOOK_CODE, _special_cb(m, kind), None, addr, addr)
        m.raw.mem_write(TRAMP, tramp_double(0x10047e1a, 2))
        m.raw.mem_map(0xb0000, 0x10000)         # the text screen that Mono_PutText writes to
        m.wr.append((0xb0000, 0x10000))
        m.snap0 = m.dump()
        m._index()          # the checker re-indexes when the snapshot changes; do it now, before this trial writes anything
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}
    ST['fread'] = b''
    ST['write_fail'] = -1
    ST['write_fail_value'] = 0
    ST['garbage'] = rb(ctx, 12)
    ST['e2_result'] = 0
    ST['e2_ret'] = 0
    ST['dp_ret'] = r.getrandbits(32)
    ST['alloc'] = 0
    ST['sdib'] = 1


def rets(addr, *values):
    ST['rets'][addr] = list(values)




# ---- special values -----------------------------------------------------------------------------
QNAN = [0x7fc00000, 0xffc00000, 0x7fc12345, 0xffe00001]
SPECIAL_BITS = QNAN + [0x7f800000, 0xff800000, 0x00000000, 0x80000000, 0x00000001, 0x80000001, 0x007fffff, 0x807fffff,
                       0x00800000, 0x80800000, 0x7f7fffff, 0xff7fffff, 0x33d6bf95, 0xb3d6bf95, 0x33d6bf94, 0x33d6bf96]


def xbits(r, spread=100.0, special=0.25):
    """Float bits: mostly ordinary floats (as rfbits), sometimes NaN, infinity, a denormal, a signed zero, the largest
    float or 1e-7 (the threshold 0x1012b5f8 / 0x1012bb34) and its neighbours."""
    if r.random() < special:
        return r.choice(SPECIAL_BITS)
    return rfbits(r, spread)


def near(r, value):
    """The bits of value or of one of its float neighbours."""
    b = fbits(value if abs(value) < 3.4e38 else 3.4e38)
    return (b + r.choice([0, 0, 1, -1, 2, -2])) & 0xffffffff


def wbits(ctx, addr, bits):
    ctx.m.uc.mem_write(addr, struct.pack('<I', bits & 0xffffffff))


def bfl(bits):
    return struct.unpack('<f', struct.pack('<I', bits & 0xffffffff))[0]


# ---- generators ---------------------------------------------------------------------------------
def g_47e1a(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x40)
    period = r.choice([r.uniform(0.5, 400.0), 360.0, 360.0, 1.0, 100.0, 6.283185, 2.0, -360.0, 0.0, -0.0, 1e-40, 3e38,
                       r.uniform(-1000, 1000)])
    pb = fbits(period) if r.random() > 0.1 else r.choice(SPECIAL_BITS)
    wbits(ctx, obj + 0x10, pb)
    p = bfl(pb)
    if p != p or abs(p) == float('inf'):
        p = 360.0
    c = r.random()
    if c < 0.5:
        xb = fbits(r.choice([r.uniform(-2000, 2000), r.uniform(-400, 400), 0.0, -0.0, 180.0, -180.0, 270.0, -270.0, 90.0,
                             360.0, 540.0, 1e-3, 1e5, 1e30, -1e30]))
    elif c < 0.85:      # next to the half period and its odd multiples, where the branches change
        xb = near(r, r.choice([p / 2, -p / 2, p, -p, 3 * p / 2, -3 * p / 2, 5 * p / 2, -5 * p / 2]))
    else:
        xb = r.choice(SPECIAL_BITS)
    return [obj, xb]


def g_62a3a(ctx):
    begin(ctx)
    r = ctx.rng
    for slot in (0x109e893c, 0x109e8940):
        o = zbuf(ctx, 0x100)
        rect = ctx.buf(32, struct.pack('<5i', 0, *[r.choice([0, 1, 100, -100, 639, 479, r.randrange(-2000, 2000)]) for _ in range(4)]))
        put(ctx, o + 0xec, 'I', rect)
        w32(ctx, slot, o)
    for a in (0x10144ec8, 0x10144ed0):
        ctx.m.uc.mem_write(a, struct.pack('<2i', *[r.choice([0, 0x10000, 0x8000, -0x10000, r.randrange(-0x20000, 0x20000)]) for _ in range(2)]))
    w32(ctx, 0x109e3e1c, r.choice([0, 1, -1, 3, 7, 640, 320, 511, -50, 0x7fffffff, -0x80000000, 0x1000001, 0x7ffffff1,
                                   r.randrange(-100000, 100000), ctx.u32(), ctx.u32()]))
    w32(ctx, 0x10144f4c, ctx.u32())
    w32(ctx, 0x10144f38, ctx.u32())
    w32(ctx, 0x1017694c, ctx.u32())
    return []


def angle_int(r):
    return r.choice([0, 0, 1, -1, 45, -45, 90, -90, 180, -180, 270, 360, 359, 361, 720, 1000, -1000, 3, 7, 33,
                     r.randrange(-720, 721), r.randrange(-720, 721), r.randrange(-100000, 100000)])


def g_631ef(ctx):
    begin(ctx)
    r = ctx.rng
    ints = [0, 1, -1, 2, -2, 3, 7, 100, -100, 12345, -12345, 0x7fffffff, -0x80000000, 0x7ffffffe, -0x7fffffff, 1000000,
            0x1000001, -0x1000001, 0x20000003, r.randrange(-1000, 1000), r.randrange(-100000, 100000),
            r.randrange(-2 ** 31, 2 ** 31), r.randrange(-2 ** 31, 2 ** 31)]
    return [ctx.buf(8), ctx.buf(8), r.choice(ints) & 0xffffffff, r.choice(ints) & 0xffffffff, angle_int(r) & 0xffffffff]


def g_68bb2(ctx):
    begin(ctx)
    r = ctx.rng
    c = r.random()
    if c < 0.6:
        ab = fbits(r.choice([0.0, -0.0, 1e-8, -1e-8, 0.5, -0.5, 45.0, 90.0, -90.0, 180.0, 360.0, 1e5, 3e5, r.uniform(-720, 720),
                             r.uniform(-720, 720), r.uniform(-30, 30), r.uniform(-1, 1)]))
    elif c < 0.85:      # next to the threshold 1e-7 on both signs
        ab = near(r, r.choice([1e-7, -1e-7]))
    else:               # no infinity: the sine helper 0x10084766 never ends on it
        ab = r.choice(QNAN + [0x00000001, 0x80000001, 0x007fffff, 0x80000000, 0x00000000])
    outs = [ctx.buf(8) if r.random() > 0.08 else 0 for _ in range(3)]
    return [r.getrandbits(32), xbits(r, 1000.0), xbits(r, 1000.0), ab] + outs


def g_8e687(ctx):
    begin(ctx)
    r = ctx.rng
    left = r.choice([0, 1, -1, 5, -100, 100, r.randrange(-1000, 1000)])
    top = r.choice([0, 1, -1, 7, -100, 100, r.randrange(-1000, 1000)])
    width = r.choice([0, 1, 2, 3, -1, -2, -3, 10, 100, 639, 1000, 0x7fffffff, -0x80000000, r.randrange(-5, 2000),
                      r.randrange(-2 ** 31, 2 ** 31)])
    height = r.choice([0, 1, 2, 3, -1, 10, 100, 479, r.randrange(-5, 2000)])
    rect = ctx.buf(0x20, struct.pack('<5i', 0, left, top, s32(left + width), s32(top + height)) + rb(ctx, 12))
    blk = zbuf(ctx, 0x100)
    if r.random() < 0.8:
        wf(ctx, blk + 0x44, r.choice([0.0, -0.0, 1.0, 0.5, 0.25, -0.5, 1.5, 3.7, 1e3, 1e10, -1e10, 1e30, 0.1, 0.3,
                                      r.uniform(-10, 10), r.uniform(0, 2)]))
    else:
        wbits(ctx, blk + 0x44, r.choice(SPECIAL_BITS))
    w32(ctx, 0x1014cae0, blk)
    return [rect, ctx.u32()]


def obj_world(ctx):
    """An object for the angle/mode functions around 0x100c0558 (see Sim_10015266, Sim_100c48fa, Sim_1000ae00)."""
    r = ctx.rng
    obj = newbuf(ctx, 0x700)
    block = newbuf(ctx, 0x600)
    put(ctx, obj + 0x28, 'I', block)
    put(ctx, block + 0x50c, 'I', r.choice([0, 1, 2]))
    ident = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, ident)
    put(ctx, obj + 4, 'I', ident if r.random() < 0.6 else r.randrange(0, 4))
    for off in (0x68, 0x110, 0xe8):
        c = r.random()
        if c < 0.15:
            wbits(ctx, obj + off, r.choice(SPECIAL_BITS))
        elif c < 0.3:
            wf(ctx, obj + off, r.choice([180.0, -180.0, 360.0, -360.0, 179.99998, -179.99998, 540.0]))
        else:
            wf(ctx, obj + off, rfl(r, 200.0, 0.2))
    rec = newbuf(ctx, 0x100)
    put(ctx, obj + 0x54, 'I', rec if r.random() < 0.9 else 0)
    put(ctx, obj + 0x1f4, 'I', r.choice([0, 0, 1]))
    word = r.choice([0x200 | ident, 0x200 | r.randrange(0, 8), r.getrandbits(16)])
    put(ctx, obj + 0x1aa, 'H', word)
    put(ctx, obj + 0x218, 'I', r.choice([0, 1, 2, 3, ctx.u32()]))
    w32(ctx, 0x101654f2, ctx.u32())
    rets(0x1000b070, *[r.choice([0, 1, r.getrandbits(32)]) for _ in range(3)])
    rets(0x1000b587, *[r.choice([0, 5, r.getrandbits(32)]) for _ in range(3)])
    return obj


def g_c0558(ctx):
    begin(ctx)
    r = ctx.rng
    obj = obj_world(ctx)
    clock = r.choice([0, 100, 5000, ctx.u32()])
    w32(ctx, 0x1015dc78, clock)
    put(ctx, obj + 0x204, 'I', r.choice([clock, clock - 1, clock + 1, 0, 1, 0x7fffffff, 0x80000000, ctx.u32()]) & 0xffffffff)
    return [obj]


def g_c0d40(ctx):
    begin(ctx)
    r = ctx.rng
    obj = obj_world(ctx)
    if r.random() < 0.75:
        base = r.choice([r.uniform(-3000, 3000), r.uniform(-1e7, 1e7), 0.0, -0.0, -2000.0, 1e-40, 3e38, -3e38])
        wf(ctx, obj + 0xe0, base)
        sb = fbits(bfl(fbits(base)) + 2000.0)
        wbits(ctx, obj + 0x5c, r.choice([sb, sb + 1, sb - 1, sb + 2, fbits(base + 1999.5), fbits(base), rfbits(r, 3000.0)]))
    else:
        wbits(ctx, obj + 0xe0, xbits(r, 3000.0, 0.6))
        wbits(ctx, obj + 0x5c, xbits(r, 3000.0, 0.6))
    return [obj, r.choice([0, 1, -1, 0x7fff, -0x8000, r.getrandbits(32)]) & 0xffffffff]


def g_c3892(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x40)
    block = newbuf(ctx, 0x600)
    put(ctx, obj + 0x28, 'I', block)
    put(ctx, block + 0x510, 'I', r.choice([6, 6, 6, 7, 9, 5, 0, -1, ctx.u32()]))
    c = r.random()
    if c < 0.4:
        sb = fbits(r.choice([0.0, 1.0, -1.0, 3.0, -0.5, r.uniform(-100, 100)]))
    elif c < 0.8:
        sb = near(r, r.choice([1e-7, -1e-7]))
    else:
        sb = r.choice(SPECIAL_BITS)
    wbits(ctx, block + 0x540, sb)
    if r.random() < 0.85:
        value = r.choice([0.0, -0.0, 0.5, -0.5, 0.99999994, -0.99999994, 1.5, -1.5, 99.5, -99.5, 1e5, -1e5, 3e9, -3e9,
                          1e10, 4.5e9, 2147483648.0, -2147483648.0, 2147483520.0, -2147483904.0, 4294967296.0,
                          9.2233715e18, -9.2233715e18, 1e19, -1e19, 1e-40, r.uniform(-200, 200), r.uniform(-1000, 1000)])
        vb = fbits(value)
    else:
        vb = r.choice(SPECIAL_BITS)
        value = 0.0
    wbits(ctx, block + 0x4e8, vb)
    if abs(value) < 2 ** 31:
        limit = r.choice([int(value), int(value) + 1, int(value) - 1, 0, 1, -1, 100, -100, ctx.u32()])
    else:
        limit = r.choice([0, 1, -1, ctx.u32(), 0x7fffffff, 0x80000000])
    return [obj, limit & 0xffffffff]


SPECS = [
    dict(name='Sim_10047e1a', orig=TRAMP, c='Test_10047e1a', gen=g_47e1a, trials=3000),
    dict(name='Sim_10062a3a', orig=0x10062a3a, c='Sim_10062a3a', gen=g_62a3a, trials=1500, void=True),
    dict(name='Sim_100631ef', orig=0x100631ef, c='Sim_100631ef', gen=g_631ef, trials=3000, void=True),
    dict(name='Sim_10068bb2', orig=0x10068bb2, c='Sim_10068bb2', gen=g_68bb2, trials=3000, void=True),
    dict(name='Sim_1008e687', orig=0x1008e687, c='Sim_1008e687', gen=g_8e687, trials=2000),
    dict(name='Sim_100c0558', orig=0x100c0558, c='Sim_100c0558', gen=g_c0558, trials=2000, void=True),
    dict(name='Sim_100c0d40', orig=0x100c0d40, c='Sim_100c0d40', gen=g_c0d40, trials=3000),
    dict(name='Sim_100c3892', orig=0x100c3892, c='Sim_100c3892', gen=g_c3892, trials=3000),
]
