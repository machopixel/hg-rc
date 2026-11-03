# Differential tests for round 3, letter s (see difftest.py).
#
# Floating point: the C is built with SSE2 doubles (CFLAGS) and does every operation in double, rounding
# to float only where the original stored a float, which is what the x87 unit does with the Windows control
# word 0x27f (53-bit mantissa, set here before every trial). The results are therefore compared bit for bit
# (no fp tolerance).
#
# Stubs (same addresses for the original and the C run): code hooks on `ret` instructions log the arguments of
# every call into a buffer in argument memory (compared afterwards) and set eax. They stand in for callbacks
# stored in records, for the object tests the cell search calls by address (the DLL's code there is replaced by a
# `ret`), for the exported dpReceive function and for a display driver method. Pointers into the stack are
# logged as a marker. What a stub returns depends only on its arguments or on the number of calls so far
# (the log count), so the original and the C run see the same answers.
#
# Functions that leave a value in st(0) are called through a trampoline (original) or a wrapper in
# w3s_test.c (C) that stores it as a double at 0xAF0F00; Sim_100beee4 returns a 16-bit value, compared as
# sign-extended ax (its only caller stores ax).
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW

SOURCES = [
    'sim/unsorted_1000/w3s_model.c', 'sim/unsorted_1001/w3s_group.c', 'sim/unsorted_1002/w3s_callbacks.c',
    'sim/unsorted_1002/w3s_cell.c', 'sim/unsorted_1003/w3s_entry.c', 'sim/unsorted_1004/w3s_records.c',
    'sim/unsorted_1005/w3s_groups.c', 'sim/unsorted_1005/w3s_scale.c', 'sim/unsorted_1007/w3s_input.c',
    'sim/unsorted_1007/w3s_calls.c', 'sim/unsorted_1008/w3s_grid.c', 'sim/unsorted_100b/w3s_array.c',
    'sim/unsorted_100c/w3s_entries.c', 'sim/unsorted_100d/w3s_table.c', 'sim/render/w3s_camera.c',
    '../tools/verify/w3s_test.c',
    # callees that are already converted
    'sim/util/lookup.c', 'sim/util/field.c', 'sim/util/ring.c', 'sim/unsorted_1004/tables.c',
    'sim/unsorted_1004/state.c', 'sim/unsorted_1004/w2e_strings.c', 'sim/unsorted_100f/w2e_string.c',
    'sim/unsorted_1003/entry.c', 'sim/unsorted_1005/w2f_globals.c', 'sim/unsorted_1007/w2k_misc.c',
    'sim/core/timer.c', 'sim/view/w2l_lists.c', 'sim/view/objlist.c', 'sim/unsorted_100c/w2l_objects.c',
    'sim/unsorted_100b/fields.c', 'sim/unsorted_1003/setters.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STUB_BASE = 0x00F00100
NSTUB = 64
THUNKS = {0x100035df: 100, 0x10001e1f: 101, 0x10002775: 102}
TR_BASE = 0x00AF0000
OUTF = 0x00AF0F00
LOGMAX = 150
RECW = 8
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))

def w16(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<H', v & 0xffff))

def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))

def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))

def r32(uc, a):
    return struct.unpack('<I', bytes(uc.mem_read(a, 4)))[0]

def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]

def bitsf(b):
    return struct.unpack('<f', struct.pack('<I', b & 0xffffffff))[0]


# ---- stubs ----------------------------------------------------------------------------------------------
def norm(v):
    return 0x57AC0000 if 0x700000 <= v < 0x800000 else v

def emit(uc, rec):
    log = ST['log']
    n = r32(uc, log)
    if n >= LOGMAX:
        return n
    rec = [norm(v) & 0xffffffff for v in rec] + [0] * (RECW - len(rec))
    uc.mem_write(log + 4 + n * RECW * 4, struct.pack('<%dI' % RECW, *rec[:RECW]))
    uc.mem_write(log, struct.pack('<I', n + 1))
    return n

def _hook(m):
    uc = m.uc

    def cb(uc, address, size, ud):
        try:
            if address in THUNKS:
                k = THUNKS[address]
            else:
                k = (address - STUB_BASE) // 4
            esp = uc.reg_read(UC_X86_REG_ESP)
            a = struct.unpack('<6I', bytes(uc.mem_read(esp + 4, 24)))
            h = ST['h'].get(k)
            n = r32(uc, ST['log'])
            ret = 0
            if h is None:
                emit(uc, [k + 1] + list(a[:ST.get('nargs', 4)]))
                ret = ST['ret'].get(k, 0)
            else:
                ret = h(uc, k, a, n)
            uc.reg_write(UC_X86_REG_EAX, ret & 0xffffffff)
            from unicorn import x86_const
            if address >= 0x40000000:       # a C entry (its code is restored between trials): return from here
                sp = uc.reg_read(UC_X86_REG_ESP)
                uc.reg_write(x86_const.UC_X86_REG_EIP, struct.unpack('<I', bytes(uc.mem_read(sp, 4)))[0])
                uc.reg_write(UC_X86_REG_ESP, sp + 4)
        except Exception as e:
            ST['err'] = repr(e)
            raise
    uc.hook_add(UC_HOOK_CODE, cb, None, STUB_BASE, STUB_BASE + NSTUB * 4)
    # the C calls the three tests directly (Windows calls round, step 1): the same stubs at their C entries
    for t, csym in ((0x100035df, 'Sim_1002d3ca'), (0x10001e1f, 'Sim_1002d479'), (0x10002775, 'Sim_1002cde1')):
        THUNKS[m.syms[csym]] = THUNKS[t]
    for t in THUNKS:
        uc.hook_add(UC_HOOK_CODE, cb, None, t, t)
        if t < 0x40000000:
            uc.mem_write(t, b'\xc3')
    uc.mem_write(STUB_BASE, b'\xc3' * (NSTUB * 4))

def stub(k):
    return STUB_BASE + 4 * k

def begin(ctx):
    m = ctx.m
    ST['nargs'] = 4
    if not getattr(m, '_w3s', False):
        _hook(m)
        m._w3s = True
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    try:
        from unicorn.x86_const import UC_X86_REG_MXCSR
        m.uc.reg_write(UC_X86_REG_MXCSR, 0x1f80)
    except Exception:
        pass
    ST['log'] = ctx.buf(4 + LOGMAX * RECW * 4, 'zero')
    ST['h'] = {}
    ST['ret'] = {k: ctx.rng.choice([0, 1, ctx.rng.getrandbits(32)]) for k in range(NSTUB)}
    ST['rng'] = ctx.rng


def tramp(ctx, slot, orig, nargs, kind='d', sx=False):
    """Original side of a call that leaves a value in st(0) (or a 16-bit value in ax)."""
    addr = TR_BASE + 0x40 * slot
    code = b''.join(b'\xff\x74\x24' + bytes([4 * nargs]) for _ in range(nargs))
    code += b'\xe8' + struct.pack('<i', orig - (addr + len(code) + 5))
    if nargs:
        code += b'\x83\xc4' + bytes([4 * nargs])
    if sx:
        code += b'\x0f\xbf\xc0\xc3'
    else:
        code += b'\xdd\x1d' + struct.pack('<I', OUTF) + b'\x31\xc0\xc3'
    ctx.m.uc.mem_write(addr, code)
    return addr

def TR(slot):
    return TR_BASE + 0x40 * slot


# ---- random data ----------------------------------------------------------------------------------------
def rfloat(r, wild=False):
    c = r.random()
    if wild and c < 0.04:
        return r.choice([float('nan'), float('inf'), float('-inf')])
    if c < 0.15:
        return r.choice([0.0, -0.0, 1.0, -1.0, 0.5, 2.0])
    if c < 0.5:
        return r.uniform(-100, 100)
    if c < 0.75:
        return r.uniform(-1e4, 1e4)
    if c < 0.9:
        return r.uniform(-1, 1)
    return r.choice([1e-20, -1e-20, 3e30, -3e30, 1.5e-39, 16777217.0, 0.1, 0.3])

def fvec(r, n, wild=False):
    return struct.pack('<%df' % n, *[rfloat(r, wild) for _ in range(n)])

def rbytes(r, n):
    return bytes(r.getrandbits(8) for _ in range(n))

def small_float(r):
    return r.choice([0.0, 1.0, 2.0, 3.0, -1.0, 0.5, 4.0, 5.0, 10.0, -2.5])

def ptrtab(ctx, recs):
    for i, a in enumerate(recs):
        w32(ctx, 0x109e8b40 + 4 * i, a)
    w32(ctx, 0x1015dc64, len(recs))


# ---- model blocks (0x100098bb, 0x10009a53, 0x10009c55) ---------------------------------------------------
def model_block(ctx, nv, nn, next_=0):
    r = ctx.rng
    off = 0x18 + nv * 0x58 + r.choice([0, 0, 8])
    b = ctx.buf(off + nn * 0x28 + 16, 'zero')
    wb(ctx, b + 0x18, fvec(r, nv * 22))
    wb(ctx, b + off, fvec(r, nn * 10))
    w16(ctx, b + 4, nv)
    w16(ctx, b + 6, nn)
    w32(ctx, b + 8, off)
    w32(ctx, b + 0xc, next_)
    return b

def g_100098bb(ctx):
    begin(ctx)
    r = ctx.rng
    b = model_block(ctx, r.choice([0, 1, 2, 3, 6]), r.choice([0, 1, 2, 5]))
    return [b, ctx.buf(48, fvec(r, 12))]

def mk_obj_vec(ctx, o):
    r = ctx.rng
    wb(ctx, o + 0x38, fvec(r, 3))
    wb(ctx, o + 0x54, fvec(r, 3))
    wb(ctx, o + 0x70, fvec(r, 3))

def g_10009a53(ctx):
    begin(ctx)
    r = ctx.rng
    o = ctx.buf(0xb0, rbytes(r, 0xb0))
    mk_obj_vec(ctx, o)
    w32(ctx, o + 0x9c, r.choice([0, 5, 0xffffffff, r.getrandbits(32)]))
    return [o, ctx.buf(48, fvec(r, 12))]

def g_10009c55(ctx):
    begin(ctx)
    r = ctx.rng
    o = ctx.buf(0xb0, rbytes(r, 0xb0))
    mk_obj_vec(ctx, o)
    w32(ctx, o + 0x9c, r.choice([0, 5, r.getrandbits(32)]))
    nxt = 0
    for _ in range(r.choice([0, 1, 2, 3])):
        nxt = model_block(ctx, r.choice([0, 1, 3]), r.choice([0, 2]), nxt)
    w32(ctx, o + 0x2c, nxt)
    return [o, ctx.buf(48, fvec(r, 12))]

def g_10008eac(ctx):
    begin(ctx)
    r = ctx.rng
    o = ctx.buf(0x40, rbytes(r, 0x40))
    p = ctx.buf(8, rbytes(r, 8)) if r.random() < 0.7 else 0
    if p and r.random() < 0.3:
        wf(ctx, p, rfloat(r, True))
    w32(ctx, o + 0x30, p)
    return [o]


# ---- groups (0x10012843, 0x100526e3, 0x10052ad6) ----------------------------------------------------------
def group_env(ctx):
    r = ctx.rng
    nrec = r.choice([3, 6, 10])
    recs = []
    for i in range(nrec):
        b = ctx.buf(0x40, rbytes(r, 0x40))
        w32(ctx, b + 0, r.choice([1, 1, 1, 0, 2]))
        w32(ctx, b + 4, r.choice([r.randrange(0, 6), r.randrange(0, 6), -1]))
        w32(ctx, b + 0x10, r.choice([0, 1, 2, 7, 8, r.randrange(0, 9)]))
        w16(ctx, b + 0x1c, r.choice([0, 0, 0, 2, 4, 6, 8, 0x10, 0xfff1]))
        recs.append(b)
    ptrtab(ctx, recs)
    for g in range(16):
        gb = 0x109b7d40 + g * 0x6c
        cnt = r.choice([0, 1, 2, 3, 4, 4])
        w32(ctx, gb, r.choice([r.randrange(-1, nrec), -1, 0]))
        w32(ctx, gb + 4, cnt)
        w32(ctx, gb + 8, r.choice([0, 0, 1]))
        for j in range(5):
            w32(ctx, gb + 0x1c + 4 * j, r.randrange(0, nrec))
    for base in (0x109b8430, 0x109b8450):
        for g in range(16):
            wb(ctx, base + g * 0x74, fvec(r, 8))
    return nrec

def g_10012843(ctx):
    begin(ctx)
    group_env(ctx)
    return [ctx.rng.choice([0, 1, 2, 3, 5, 15, 15])]

def g_100526e3(ctx):
    begin(ctx)
    r = ctx.rng
    nrec = group_env(ctx)
    return [r.choice([0, 1, 2, 7, 15, 15, 16, 20]), r.choice([-1, 0, 1, 2, nrec - 1, nrec, nrec + 2, r.randrange(0, nrec)])]

def g_10052ad6(ctx):
    begin(ctx)
    group_env(ctx)
    return [ctx.rng.choice([0, 1, 2, 3, 5, 15])]


# ---- callbacks (0x10029f50 ...) ------------------------------------------------------------------------
def cb_env(ctx):
    r = ctx.rng
    nrec = r.choice([0, 1, 2, 4, 6])
    recs = []
    for i in range(nrec):
        b = ctx.buf(0x60, rbytes(r, 0x60))
        w32(ctx, b + 0x28, r.choice([0, 0, b + 0x30, r.getrandbits(32), b]))
        for off in (0x30, 0x38, 0x3c, 0x44):
            w32(ctx, b + off, r.choice([0, stub(r.randrange(0, 40)), stub(r.randrange(0, 40))]))
        recs.append(b)
    ptrtab(ctx, recs)
    w32(ctx, 0x1017b8c8, r.randrange(0, max(1, nrec)))
    if r.random() < 0.15:
        w32(ctx, 0x1015dc64, max(0, nrec - 1))
    if nrec == 0 and r.random() < 0.5:
        w32(ctx, 0x1017b8c8, 0)
        w32(ctx, 0x109e8b40, 0)

def g_cb(ctx):
    begin(ctx)
    ST['nargs'] = 1
    cb_env(ctx)
    return []


# ---- grid cell search (0x1002db2d, 0x1004487c, 0x10088775) ---------------------------------------------
def grid_env(ctx, with_objects=True):
    r = ctx.rng
    w32(ctx, 0x1017c8c4, r.choice([0, 1, 1, 1]))
    pool = []
    for _ in range(5):
        cell = ctx.buf(0x40, 'zero')
        chain = 0
        for _ in range(r.choice([0, 1, 2, 3, 4])):
            o = ctx.buf(0x60, rbytes(r, 0x60))
            w32(ctx, o + 0x34, r.choice([0, 5, 8, 5, 8, 0, 1, 3, 12, -1]))
            w32(ctx, o + 0x10, chain)
            chain = o
            kind = ST['kinds']
            # the object tests answer from a table keyed by object address
            ST['obj'][o] = (r.random() < 0.6, r.choice([rfloat(r), rfloat(r), -3.4e38, 0.0, -1e-5, 1e-5, 100.0, 100.0]))
        w32(ctx, cell + 0x10, chain)
        pool.append(cell)
    for c in range(9):
        for rw in range(9):
            w32(ctx, 0x1017ba4c + c * 0x18c + rw * 0x2c, r.choice([0, pool[r.randrange(0, 5)], pool[r.randrange(0, 5)]]))
    w32(ctx, 0x1017c8c8, r.choice([0, pool[0], pool[1]]))
    w32(ctx, 0x1017c8cc, r.choice([0, pool[2], pool[3]]))
    w32(ctx, 0x1015dd40, r.getrandbits(32))
    w32(ctx, 0x1015dd44, r.getrandbits(32))
    w32(ctx, 0x1015dd48, r.getrandbits(32))
    w32(ctx, 0x1015dd4c, r.getrandbits(32))
    w32(ctx, 0x1015dd50, r.getrandbits(32))
    w32(ctx, 0x1015dd54, r.getrandbits(32))

def obj_test(uc, k, a, n):
    # f(object, x, y, z, &result)
    emit(uc, [k + 1, a[0], a[1], a[2], a[3]])
    ok, val = ST['obj'].get(a[0], (False, 0.0))
    if ok:
        uc.mem_write(a[4], struct.pack('<f', val))
    return 1 if ok else 0

def grid_xz(r):
    def coord(bias):
        cell = r.choice([-1, 0, 1, 4, 8, 9, r.randrange(-1, 10)])
        return -bias + 55562.0 * (cell + r.choice([0.0, 0.25, 0.5, 0.99, 0.0001, 1.0])) + r.choice([0, 0, 0.5, -0.5])
    return coord(254008.0), coord(246044.5)

def grid_call_args(ctx):
    r = ctx.rng
    x, z = grid_xz(r)
    if r.random() < 0.05:
        x = rfloat(r, True)
    return x, rfloat(r), z

def gridstart(ctx):
    begin(ctx)
    ST['obj'] = {}
    ST['kinds'] = 0
    for t in THUNKS.values():
        ST['h'][t] = obj_test
    grid_env(ctx)

def g_1002db2d(ctx):
    gridstart(ctx)
    x, y, z = grid_call_args(ctx)
    return [fbits(x), fbits(y), fbits(z)]

def g_1004487c(ctx):
    gridstart(ctx)
    x, y, z = grid_call_args(ctx)
    return [ctx.buf(12, struct.pack('<3f', x, y, z))]

def g_10088775(ctx):
    begin(ctx)
    ST['obj'] = {}
    ST['kinds'] = 0
    grid_env(ctx)
    x, y, z = grid_call_args(ctx)
    return [fbits(x), fbits(z)]


# ---- entries (0x100335fd ..) -----------------------------------------------------------------------------
def entry_env(ctx, ncount=None):
    r = ctx.rng
    n = ncount if ncount is not None else r.choice([0, 1, 2, 3, 5, 8])
    w32(ctx, 0x109e4cc0, n)
    for i in range(10):
        e = 0x109e4cc4 + i * 0x44
        w32(ctx, e + 0x10, r.choice([0, 0, 1, 1, 2, r.getrandbits(32)]))
        wf(ctx, e + 0x20, *[small_float(r) if r.random() < 0.7 else rfloat(r, True) for _ in range(3)])
    return n

def pt(r):
    return [fbits(small_float(r) if r.random() < 0.7 else rfloat(r, r.random() < 0.3)) for _ in range(3)]

def g_100335fd(ctx):
    begin(ctx)
    r = ctx.rng
    entry_env(ctx)
    e = 0 if r.random() < 0.1 else 0x109e4cc4 + r.randrange(0, 8) * 0x44
    if r.random() < 0.3:
        e = ctx.buf(0x44, fvec(r, 17))
    return [e] + pt(r)

def g_1003366e(ctx):
    begin(ctx)
    r = ctx.rng
    entry_env(ctx)
    wf(ctx, 0x109e4cc4 + 0x20 - 0x20 + 0x20, *[0.0] * 0)   # keep entry data as generated
    return [0x109e4cc4 + r.randrange(0, 4) * 0x44] + pt(r) + pt(r)

def g_10033d75(ctx):
    begin(ctx)
    entry_env(ctx)
    return pt(ctx.rng)

def g_10034036(ctx):
    begin(ctx)
    r = ctx.rng
    n = entry_env(ctx)
    cnt = r.choice([0, 1, 2, 3, 5, -1, 8])
    lst = ctx.buf(40, struct.pack('<10i', *[r.randrange(0, 9) for _ in range(10)]))
    return [0 if r.random() < 0.08 else lst, cnt] + pt(r)

def pair_env(ctx):
    r = ctx.rng
    entry_env(ctx)
    objs = []
    for _ in range(2):
        o = ctx.buf(0x70, rbytes(r, 0x70))
        wf(ctx, o + 0x58, *[small_float(r) if r.random() < 0.7 else rfloat(r, True) for _ in range(3)])
        objs.append(o)
    return objs

def g_100c3f0c(ctx):
    begin(ctx)
    r = ctx.rng
    a, b = pair_env(ctx)
    return [0 if r.random() < 0.05 else a, 0 if r.random() < 0.05 else b, 0 if r.random() < 0.05 else 0x109e4cc4 + r.randrange(0, 4) * 0x44]

def g_100c4265(ctx):
    begin(ctx)
    r = ctx.rng
    a, b = pair_env(ctx)
    return [r.choice([-1, 0, 1, 2, 3, 3, 7, 9, 12]), r.choice([0, 1, 2, 3, 8, 9, -1]), ctx.u32(), a]


# ---- 0x10044420 .. records ---------------------------------------------------------------------------
def dbits(d):
    return list(struct.unpack('<2I', struct.pack('<d', d)))

def g_10047b76(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1015dc78, ctx.u32())
    d = r.choice([0.0, -0.0, 0.5, 1.0, 5e-10, 5.5e-10, 5.52e-10, 1e-9, -1e-9, r.uniform(-1, 1), r.uniform(-1e5, 1e5), 1e300, -1e300,
                  float('nan'), float('inf'), r.uniform(0, 1e-8)])
    return [ctx.buf(0x20, rbytes(r, 0x20)), ctx.u32(), ctx.u32()] + dbits(d)

def g_10047c92(ctx):
    a = g_10047b76(ctx)
    return a + [ctx.u32()]

def g_10044420(ctx):
    begin(ctx)
    r = ctx.rng
    recs = [ctx.buf(0x10, rbytes(r, 0x10)) for _ in range(4)]
    ptrtab(ctx, recs)
    w32(ctx, 0x1017b8c8, r.randrange(0, 5))
    w32(ctx, 0x1015dc78, ctx.u32())
    w32(ctx, 0x1015ef00, ctx.u32())
    w32(ctx, 0x10179cd4, r.choice([0, 1]))
    w32(ctx, 0x1013aee0, r.choice([-1, 0, 3, ctx.u32()]))
    for a in (0x109b9a90, 0x109b9b00):
        wb(ctx, a, rbytes(r, 0x120))
    return []

def g_1004f19d(ctx):
    begin(ctx)
    r = ctx.rng
    cam = ctx.buf(0x40, fvec(r, 16))
    w32(ctx, 0x1014cae0, cam)
    return [ctx.buf(0x70, fvec(r, 28)), ctx.buf(16, rbytes(r, 16))]


# ---- scaling (0x100570fb, 0x1005e295) -------------------------------------------------------------------
def g_100570fb(ctx):
    begin(ctx)
    r = ctx.rng
    b = r.choice([0, 5, 100, 1000, r.randrange(0, 70000), 0x7fffffff, -3])
    c = r.choice([0, 7, r.randrange(-1000, 1000), r.getrandbits(32)])
    a = r.choice([c + b + r.randrange(-5, 5), c - b - r.randrange(-5, 5), c + b + r.randrange(0, 200000), c - b - r.randrange(0, 200000),
                  r.getrandbits(32), c, c + 3 * b, c - 3 * b]) & 0xffffffff
    d = r.choice([0.5, 1.0, 2.0, 0.001, 1000.0, -1.0, 65536.0, 0.00001, r.uniform(-3, 3), r.uniform(-1e5, 1e5), 1e300, float('nan'), -0.75])
    return [a, b & 0xffffffff, c & 0xffffffff] + dbits(d)

def g_1005e295(ctx):
    begin(ctx)
    r = ctx.rng
    o = ctx.buf(0xc10, rbytes(r, 0xc10))
    for off in (0xbd3, 0xbcf):
        w32(ctx, o + off, r.choice([0, 0x10, 8, 4, 0x20, 0x40, 0x80, 0xf0, 2, 6, 0xfe, r.getrandbits(32)]))
    w32(ctx, o + 0x60b, r.choice([0, 1, -1, 7, -7, 0x80000000, r.getrandbits(32)]))
    w32(ctx, o + 0xbd7, r.choice([0, 0xffffffff, r.getrandbits(32)]))
    return [o, r.choice([1, 5, 100, 0, -4, 1000]), r.choice([-5, 0, 1, 3, 50, 99, 100, 1000, 500000, 0x7fffffff])]


# ---- input (0x10075371) ------------------------------------------------------------------------------
def g_10075371(ctx):
    begin(ctx)
    r = ctx.rng
    import copy
    for base in (0x10153180, 0x10153280):
        slots = [r.choice([0, 1, 0xfffffff0, r.randrange(0, 400)]) for _ in range(63)]
        wb(ctx, base, struct.pack('<64I', *(slots + [0xffffffff])))
    w32(ctx, 0x10153380, r.choice([0, 100, 300, r.getrandbits(10)]))
    w32(ctx, 0x10153384, r.choice([0, 100, 300, r.getrandbits(10)]))
    w32(ctx, 0x1015317c, r.getrandbits(10) if r.random() < 0.3 else 0)
    w32(ctx, 0x10149948, r.choice([0, 1, 5, 0x80, 0x81, 0x85, r.randrange(0, 0x100)]))
    res = ctx.buf(0x20, rbytes(r, 0x20))
    w32(ctx, res + 4, r.choice([0, 1, 1, 3, r.getrandbits(32)]))
    node = ctx.buf(0x20, 'zero')
    w32(ctx, node + 4, 1)
    w32(ctx, node + 0xc, res)
    w32(ctx, 0x1015dc14, r.choice([node, node, 0]))
    key = r.choice([0, 0, 0x1b, 0x30, 0x39, 0x2f, 0x3a, 0xd, 9, 0x20, 0xc5, 0xc6, 0xc9, 0xca, 0x209, 0x1ff, 0xe, 0xc, -1, r.randrange(-300, 600)])
    w16(ctx, 0x1015fd4c, key)
    w32(ctx, 0x1015dc18, r.getrandbits(32))
    wb(ctx, 0x1015fde6, bytes([r.choice([0, 0, 1, 0xff, 0x80]), r.choice([0, 0, 1, 0xff])]))
    wb(ctx, 0x1015fde2, bytes([r.choice([0, 7])]))
    wb(ctx, 0x1015fde5, bytes([r.choice([0, 7])]))
    wf(ctx, 0x1015fdd8, r.choice([0.0, -9000.0, 9000.0, -8192.0, 8192.0, 100.0, -8191.0, float('nan'), r.uniform(-20000, 20000)]))
    wf(ctx, 0x1015fddc, r.choice([0.0, -9000.0, 9000.0, -8192.0, 8192.0, 100.0, 8193.0, float('nan'), r.uniform(-20000, 20000)]))
    return []


# ---- indirect calls (0x100780cf, 0x1007e984, 0x1008c5e0, 0x100bd76b) -----------------------------------
def g_100780cf(ctx):
    begin(ctx)
    ST['nargs'] = 2
    r = ctx.rng
    objs = {}
    for cell in (0x109e8928, 0x109e8938, 0x109e890c, 0x109e88e8, 0x109e894c, 0x109e8950, 0x109e8954):
        if r.random() < 0.2 and objs:
            o = r.choice(list(objs.values()))
        else:
            o = ctx.buf(0x140, rbytes(r, 0x140))
            w32(ctx, o + 0x11c, stub(r.randrange(0, 50)))
        objs[cell] = o
        w32(ctx, cell, o)
    return []

def dp_receive(uc, k, a, n):
    emit(uc, [k + 1, a[0], a[3], a[4], a[5], n])
    seq = ST['dpseq']
    ret = seq[n] if n < len(seq) else 5
    uc.mem_write(a[1], struct.pack('<I', ST['dpval'][n % 3]))
    uc.mem_write(a[2], struct.pack('<I', 0x77))
    uc.mem_write(a[5], struct.pack('<I', ST['dpout'][n % 3]))
    return ret

def g_1007e984(ctx):
    begin(ctx)
    r = ctx.rng
    ST['h'][7] = dp_receive
    ST['dpseq'] = [r.choice([0, 1, 0xabcd0001, r.getrandbits(32), 0xffff0000]) for _ in range(3)]
    if r.random() < 0.4:
        ST['dpseq'][r.randrange(0, 3)] = 0
    ST['dpval'] = [r.getrandbits(32) for _ in range(3)]
    ST['dpout'] = [r.getrandbits(32) for _ in range(3)]
    w32(ctx, 0x1015eab8, stub(7))
    w32(ctx, 0x1015ecd4, ctx.u32())
    w32(ctx, 0x10372d98, ctx.u32())
    w32(ctx, 0x1015d008, ctx.u32())
    return [ctx.buf(8, rbytes(r, 8))]

def g_1008c5e0(ctx):
    begin(ctx)
    ST['nargs'] = 1
    r = ctx.rng
    n = r.choice([0, 1, 2, 3, 8, 16])
    start = r.choice([0, 0, 1, 5, 200])
    pal = ctx.buf(800, rbytes(r, 800))
    obj = ctx.buf(0x20, rbytes(r, 0x20))
    w32(ctx, obj + 4, pal)
    wb(ctx, obj + 8, bytes([start]))
    wb(ctx, obj + 9, struct.pack('<I', n))
    table = ctx.buf(0x40, 'zero')
    w32(ctx, table + 0x18, stub(9))
    w32(ctx, 0x1015e688, table)
    return [obj]

def cmp_stub(uc, k, a, n):
    emit(uc, [k + 1, a[0], a[1]])
    x, y = a[0], a[1]
    mode = ST['cmpmode']
    if mode == 0:
        return (x > y) - (x < y)
    if mode == 1:
        return 1 if x >= y else -1
    if mode == 2:
        return 0
    if mode == 3:
        return 1
    return 0xfffffffe if (x ^ y) & 1 else 7

def g_100bd76b(ctx):
    begin(ctx)
    r = ctx.rng
    cap = r.choice([1, 2, 4, 8, 0, -1])
    cnt = r.choice([0, 1, 2, 3, 5])
    cnt = min(cnt, max(cap, 0) + 1) if cap < 8 else cnt
    items = ctx.buf(40 + 16, 'zero')
    vals = sorted(r.randrange(1, 40) for _ in range(8))
    wb(ctx, items, struct.pack('<8I', *vals))
    arr = ctx.buf(12, struct.pack('<iiI', cap, cnt, items))
    ST['cmpmode'] = r.choice([0, 0, 0, 1, 2, 3, 4])
    ST['h'][11] = cmp_stub
    return [arr, r.choice([0, r.randrange(1, 40), r.randrange(1, 40), 0x80000001]), stub(11) if r.random() < 0.85 else 0]


# ---- 0x1008a46e ---------------------------------------------------------------------------------------
def g_1008a46e(ctx):
    begin(ctx)
    r = ctx.rng
    spare = ctx.buf(0x100, 'zero')
    def lnk():
        return r.choice([0, 0, spare, ctx.buf(0x100, 'zero')])
    ST['obj'] = {}
    grid_env(ctx)
    owner = ctx.buf(0x100, fvec(r, 64))
    ox = r.uniform(-1, 1)
    gx, gz = grid_xz(r)
    wf(ctx, owner + 0x70, gx, 0.0, gz)
    obj = ctx.buf(0x100, 'zero')
    w16(ctx, obj, r.choice([0x1000, 0x1000, 0x1000, 0, 0x1234, 0x1800]))
    w16(ctx, obj + 2, r.choice([0x70, 0x71, 0x7f, 0, 0xf0, 0x7]))
    for off in (4, 8, 0x14, 0x18, 0x1c, 0x20, 0xc, 0x10):
        w32(ctx, obj + off, lnk())
    w32(ctx, obj + 0x28, owner if r.random() > 0.05 else 0)
    # the neighbours' links need to point back
    return [0 if r.random() < 0.05 else obj]


# ---- projection / camera ---------------------------------------------------------------------------------
def g_1008ff68(ctx):
    begin(ctx)
    r = ctx.rng
    for a in (0x10185bd4, 0x10185bf4, 0x10185bd0, 0x10185bec, 0x10185bf0, 0x10185be0, 0x10185bf4):
        w32(ctx, a, r.choice([0, 100, 240, 320, 639, 479, r.randrange(-100, 700), r.getrandbits(32)]))
    w32(ctx, 0x101c7680, r.getrandbits(8))
    w32(ctx, 0x101cc2a0, r.getrandbits(8))
    w32(ctx, 0x101c9ea4, r.choice([0, 1]))
    w32(ctx, 0x101c9ea8, r.choice([0, 1, 100, 254, 255, 256, 0x7fffffff, 0x80000000, r.randrange(0, 30)]))
    wf(ctx, 0x1036f1d8, r.choice([0.0, 1.0, 2.0, 0.5, 100.0, 1e-30, -3.0, 55562.0, float('nan'), r.uniform(0.1, 50)]))
    v = ctx.buf(0x40, rbytes(r, 0x40))
    wf(ctx, v, r.choice([0.0, 100.0, -100.0, 3e9, -3e9, 1e30, float('nan'), r.uniform(-5000, 5000), r.uniform(-50, 50)]),
       r.choice([0.0, 100.0, -100.0, r.uniform(-5000, 5000), 1e19, -1e19]))
    wb(ctx, v + 0x28, bytes([r.getrandbits(8), r.choice([0, 0, 1, 2, 0x80, r.getrandbits(8)])]))
    return [v]

def g_100d1f90(ctx):
    begin(ctx)
    r = ctx.rng
    cam = ctx.buf(0x100, rbytes(r, 0x100))
    wf(ctx, cam + 0x3c, r.choice([0.5, 4.0, 0.0, -1.0, rfloat(r)]))
    wf(ctx, cam + 0x40, r.choice([100.0, 5000.0, 1e9, 0.0, rfloat(r)]))
    wf(ctx, cam + 4, r.choice([0.0, 1.0, 2.0, 0.5, -1.0, 200.0, rfloat(r, True)]))
    wf(ctx, cam + 0x54, *[rfloat(r, True) for _ in range(3)])
    wf(ctx, cam + 0x60, *[rfloat(r, True) for _ in range(3)])
    wf(ctx, cam + 0x94, rfloat(r, True), rfloat(r, True))
    wf(ctx, 0x10143fc4, r.choice([0.0, 1.0, -1.0, 0.5, 400.0, -400.0, float('nan'), rfloat(r)]))
    wf(ctx, 0x101668cc, r.choice([0.0, 0.25, 0.5, 1.0, -1.0, float('nan')]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    w32(ctx, 0x10143fcc, r.getrandbits(32))
    return [cam]


# ---- 0x100beee4 --------------------------------------------------------------------------------------------
def g_100beee4(ctx):
    begin(ctx)
    r = ctx.rng
    sub = ctx.buf(0x600, rbytes(r, 0x600))
    wb(ctx, sub + 0x560, struct.pack('<H', r.choice([0, 0x20, 0xffff, 0x1f])))
    # partner record
    partner = ctx.buf(0x40, rbytes(r, 0x40))
    w32(ctx, partner, r.choice([1, 1, 5, 8, 0, 2, 3]))
    w32(ctx, partner + 0x28, sub)
    ptrtab(ctx, [partner, partner, partner])
    # own object and its sub-object
    own = ctx.buf(0x540, rbytes(r, 0x540))
    w32(ctx, own + 0x4f4, r.choice([0, 1, 1]))
    q = ctx.buf(0x300, rbytes(r, 0x300))
    pp = ctx.buf(0x60, rbytes(r, 0x60))
    wb(ctx, pp + 0x59, bytes([r.choice([0, 1, 0xff])]))
    w32(ctx, q + 0x54, pp)
    w32(ctx, q + 0x264, r.choice([0, 0, 4]))
    w16(ctx, q + 0x1f8, r.choice([0, 10, 3]))
    w32(ctx, own + 0, q)
    w32(ctx, own + 0x514, r.choice([0, 1, 5, 0x13, 0x14, 0x15, -3]))
    obj = ctx.buf(0x1c0, rbytes(r, 0x1c0))
    w32(ctx, obj, r.choice([1, 1, 1, 1, 0, 2]))
    w32(ctx, obj + 0x28, own)
    w16(ctx, obj + 0x1aa, r.choice([0x200, 0x200, 0x201, 0x2ff, 0, 0x100, 0x202]))
    wf(ctx, obj + 0xd4, r.choice([0.0, 999.0, 1000.0, 1001.0, -5.0, float('nan'), 1e9]))
    wf(ctx, 0x1012bb10, 1000.0)
    w32(ctx, 0x1017cd38, r.randrange(0, 0x80))
    wb(ctx, 0x101cffc0, struct.pack('<128i', *[r.choice([0, 3, 4, 100, 7, r.randrange(0, 1000)]) for _ in range(128)]))
    return [obj]


# ---- 0x100daea4 ----------------------------------------------------------------------------------------
def g_100daea4(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0, 1, 3, 8, 8, 9, 10])
    src = ctx.buf(0x18 + 12 * 12 + 16, rbytes(r, 0x18 + 12 * 12 + 16))
    total = 0x18 + 12 * n + r.choice([0, 0, 5, 11])
    if r.random() < 0.05:
        total = r.choice([0, 0x17, 0xffffffff])
    w32(ctx, src + 4, total)
    wb(ctx, src + 8, r.choice([b'abc\0' + bytes(12), bytes(r.choice(b'xyz01') for _ in range(16)), bytes(16)]))
    w32(ctx, 0x1015f3ac, r.choice([0, 1, 5, 0x1e, 0x1f, 0x20, 0x21, -1]))
    return [src]


# ---------------------------------------------------------------------------------------------------------
def spec_tr(name, orig, c, gen, nargs, trials, slot, **kw):
    """A function that returns in st(0): the original is called through a trampoline."""
    def gen2(ctx, gen=gen, orig=orig, nargs=nargs, slot=slot):
        args = gen(ctx)
        tramp(ctx, slot, orig, nargs)
        return args
    return dict(name=name, orig=TR(slot), c=c, gen=gen2, trials=trials, **kw)

SPECS = [
    spec_tr('Sim_10008eac', 0x10008eac, 'Test_10008eac', g_10008eac, 1, 400, 0),
    dict(name='Sim_100098bb', orig=0x100098bb, c='Sim_100098bb', gen=g_100098bb, void=True, trials=500),
    dict(name='Sim_10009a53', orig=0x10009a53, c='Sim_10009a53', gen=g_10009a53, void=True, trials=500),
    dict(name='Sim_10009c55', orig=0x10009c55, c='Sim_10009c55', gen=g_10009c55, void=True, trials=500),
    dict(name='Sim_10012843', orig=0x10012843, c='Sim_10012843', gen=g_10012843, trials=600),
    dict(name='Sim_10029f50', orig=0x10029f50, c='Sim_10029f50', gen=g_cb, void=True, trials=500),
    dict(name='Sim_1002a019', orig=0x1002a019, c='Sim_1002a019', gen=g_cb, void=True, trials=500),
    dict(name='Sim_1002a086', orig=0x1002a086, c='Sim_1002a086', gen=g_cb, void=True, trials=500),
    dict(name='Sim_1002a10e', orig=0x1002a10e, c='Sim_1002a10e', gen=g_cb, void=True, trials=500),
    spec_tr('Sim_1002db2d', 0x1002db2d, 'Test_1002db2d', g_1002db2d, 3, 1500, 1),
    spec_tr('Entry_DistSq', 0x100335fd, 'Test_100335fd', g_100335fd, 4, 1000, 2),
    dict(name='Sim_1003366e', orig=0x1003366e, c='Sim_1003366e', gen=g_1003366e, trials=1500),
    dict(name='Sim_10033d75', orig=0x10033d75, c='Sim_10033d75', gen=g_10033d75, trials=1500),
    dict(name='Sim_10034036', orig=0x10034036, c='Sim_10034036', gen=g_10034036, trials=1500),
    dict(name='Sim_10044420', orig=0x10044420, c='Sim_10044420', gen=g_10044420, void=True, trials=400),
    spec_tr('Sim_1004487c', 0x1004487c, 'Test_1004487c', g_1004487c, 1, 1000, 3),
    dict(name='Sim_10047b76', orig=0x10047b76, c='Sim_10047b76', gen=g_10047b76, trials=800),
    dict(name='Sim_10047c92', orig=0x10047c92, c='Sim_10047c92', gen=g_10047c92, trials=800),
    dict(name='Sim_1004f19d', orig=0x1004f19d, c='Sim_1004f19d', gen=g_1004f19d, trials=600),
    dict(name='Sim_100526e3', orig=0x100526e3, c='Sim_100526e3', gen=g_100526e3, trials=800),
    dict(name='Sim_10052ad6', orig=0x10052ad6, c='Sim_10052ad6', gen=g_10052ad6, trials=800),
    dict(name='Sim_100570fb', orig=0x100570fb, c='Sim_100570fb', gen=g_100570fb, trials=1500),
    dict(name='Sim_1005e295', orig=0x1005e295, c='Sim_1005e295', gen=g_1005e295, void=True, trials=1500),
    dict(name='Sim_10075371', orig=0x10075371, c='Sim_10075371', gen=g_10075371, void=True, trials=1500),
    dict(name='Sim_100780cf', orig=0x100780cf, c='Sim_100780cf', gen=g_100780cf, void=True, trials=500),
    dict(name='Sim_1007e984', orig=0x1007e984, c='Sim_1007e984', gen=g_1007e984, trials=600),
    dict(name='Sim_10088775', orig=0x10088775, c='Sim_10088775', gen=g_10088775, trials=1500),
    dict(name='Sim_1008a46e', orig=0x1008a46e, c='Sim_1008a46e', gen=g_1008a46e, void=True, trials=1000),
    dict(name='Sim_1008c5e0', orig=0x1008c5e0, c='Sim_1008c5e0', gen=g_1008c5e0, void=True, trials=600),
    dict(name='Sim_1008ff68', orig=0x1008ff68, c='Sim_1008ff68', gen=g_1008ff68, trials=1500),
    dict(name='PtrArray_Insert', orig=0x100bd76b, c='PtrArray_Insert', gen=g_100bd76b, trials=1000),
    dict(name='Sim_100beee4', orig=TR(4), c='Test_100beee4',
         gen=lambda ctx: (lambda a: (tramp(ctx, 4, 0x100beee4, 1, sx=True), a)[1])(g_100beee4(ctx)), trials=1500),
    dict(name='Sim_100c3f0c', orig=0x100c3f0c, c='Sim_100c3f0c', gen=g_100c3f0c, trials=1500),
    dict(name='Sim_100c4265', orig=0x100c4265, c='Sim_100c4265', gen=g_100c4265, trials=1500),
    dict(name='Sim_100c4323', orig=0x100c4323, c='Sim_100c4323', gen=g_100c4265, trials=1500),
    dict(name='Sim_100d1f90', orig=0x100d1f90, c='Sim_100d1f90', gen=g_100d1f90, void=True, trials=1500),
    dict(name='Sim_100daea4', orig=0x100daea4, c='Sim_100daea4', gen=g_100daea4, trials=800),
]
