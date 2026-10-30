# Round 8B, helper 1: bit-exact re-check (no fp/tol) of the 3x4 matrix functions and the camera
# viewport / object cull functions first converted in round 3 (spec_w3r.py; generators copied from
# there and extended). Built with SSE2 doubles like Diego's build. Float inputs are 32-bit patterns
# that include signed zeros, infinities, denormals, huge values and quiet NaNs with random payloads
# and signs (the emulator does not quiet signalling NaNs, so none are generated).
import struct
from unicorn.x86_const import UC_X86_REG_FPCW

SOURCES = ['sim/view/w3r_matrix.c', 'sim/view/w3r_camera.c', 'sim/view/matrix.c',
           'sim/render/w3r_screen.c', 'sim/render/w3r_draw.c',
           'sim/unsorted_1001/w3r_misc.c', 'sim/core/w3r_wait.c', 'sim/core/timer.c',
           'sim/unsorted_1002/w3r_table.c', 'sim/unsorted_1002/w2i_misc.c',
           'sim/unsorted_1002/w2e_tables.c', 'sim/unsorted_100f/w2e_string.c',
           'sim/unsorted_1003/w3r_mesh.c', 'sim/unsorted_1003/w3r_misc.c',
           'sim/unsorted_1005/w3r_misc.c', 'sim/unsorted_1007/w3r_objects.c',
           'sim/unsorted_1009/w2i_state.c', 'sim/unsorted_1008/w3r_grid.c',
           'sim/unsorted_1008/w2l_grid.c', 'sim/unsorted_100d/w3r_records.c']
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))

def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))

def fb(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def fbits(r, spread=10.0, special=0.08):
    """A random float as its 32-bit pattern; `special` is the share of edge values."""
    c = r.random()
    if c < special:
        k = r.randrange(9)
        s = r.getrandbits(1) << 31
        if k == 0: return s                                          # +-0
        if k == 1: return s | 0x7f800000                             # +-inf
        if k in (2, 3): return s | 0x7fc00000 | r.getrandbits(22) * r.getrandbits(1)  # quiet NaN, payload
        if k == 4: return s | r.randrange(1, 0x800000)               # denormal
        if k == 5: return s | (r.randrange(0xf8, 0xff) << 23) | r.getrandbits(23)   # huge
        if k == 6: return s | (r.randrange(1, 8) << 23) | r.getrandbits(23)          # tiny normal
        if k == 7: return s | 0x7f7fffff                             # max float
        return s | 0x00800000                                         # min normal
    if c < 0.25:
        return fb(r.choice([0.0, 1.0, -1.0, 0.5, 2.0, 16.0, -0.5, 3.0, 1e-3, 1e7]))
    return fb(r.uniform(-spread, spread))


def fwrite(ctx, addr, bits):
    w32(ctx, addr, bits)


def mat(ctx, n=12, special=0.08, spread=10.0):
    r = ctx.rng
    return ctx.buf(n * 4 + 16, struct.pack('<%dI' % n, *[fbits(r, spread, special) for _ in range(n)]))


def begin(ctx):
    ctx.m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)


def mspecial(ctx):
    """Edge share for a whole trial: most trials clean, some with many edge values."""
    return ctx.rng.choice([0.0, 0.0, 0.03, 0.1, 0.3])


# ---- generators -------------------------------------------------------------
def g_5fbb0(ctx):
    r = ctx.rng
    begin(ctx)
    sp = mspecial(ctx)
    m = mat(ctx, 12, sp)
    p = mat(ctx, 3, sp)
    c = r.random()
    if c < 0.4:
        return [m, p, p + 4, p + 8]
    if c < 0.5:                      # point inside the matrix itself (aliasing)
        return [m, m + 4 * r.randrange(12), m + 4 * r.randrange(12), m + 4 * r.randrange(12)]
    return [m, mat(ctx, 1, sp), mat(ctx, 1, sp), mat(ctx, 1, sp)]


def g_5fcdc(ctx):
    r = ctx.rng
    begin(ctx)
    return [mat(ctx, 12, mspecial(ctx)), r.choice([0, 1, 2, 0, 1, 2, 3, 0xffffffff, 7])]


def g_5fe48(ctx):
    r = ctx.rng
    begin(ctx)
    sp = mspecial(ctx)
    a, b = mat(ctx, 12, sp), mat(ctx, 12, sp)
    c = r.random()
    return [a, b, a if c < 0.15 else b if c < 0.3 else mat(ctx, 12, sp)]


def g_60101(ctx):
    r = ctx.rng
    begin(ctx)
    sp = mspecial(ctx)
    a = mat(ctx, 12, sp)
    return [a, a if r.random() < 0.2 else mat(ctx, 12, sp)]


def g_100b403d(ctx):
    r = ctx.rng
    begin(ctx)
    sp = mspecial(ctx)
    w32(ctx, 0x101507f8, r.choice([0xffffffff, 0, 1, 2, 5, 0xffffffff]))
    for i in range(6):
        wb(ctx, 0x101a11c8 + i * 0x7c, struct.pack('<12I', *[fbits(r, 5.0, sp) for _ in range(12)]))
    return [mat(ctx, 3, sp, 3000.0)]


def g_100d2306(ctx):
    r = ctx.rng
    begin(ctx)
    cam = ctx.buf(0x100)
    fwrite(ctx, cam + 0x18, r.choice([fb(r.choice([0.1, 0.5, 1.0, 2.0, 16.0, 17.0, 100.0, -3.0, 0.4999, 8.0, 15.999999, 16.000002,
                                                    0.49999997, r.uniform(0, 30)]))] * 4 + [fbits(r, 30.0, 1.0)]))
    fwrite(ctx, cam + 0x44, r.choice([fb(r.choice([1.0, 0.75, 1.33, 100.0, -1.0, 0.0, r.uniform(0.1, 5)]))] * 4 + [fbits(r, 5.0, 1.0)]))
    vp = r.choice([(0, 639, 0, 479), (0, 319, 0, 239), (10, 11, 3, 3), (5, 5, 7, 7), (r.randrange(-50, 700), r.randrange(-50, 700), r.randrange(-50, 500), r.randrange(-50, 500)), (0x7fffffff, 0x80000000, 0x7ffffff0, -0x7ffffff0)])
    wb(ctx, cam + 0x2c, struct.pack('<4i', *[(x + 2**31) % 2**32 - 2**31 for x in vp]))
    wb(ctx, 0x1036f860, struct.pack('<800f', *[r.uniform(0, 3) for _ in range(800)]))
    wb(ctx, 0x10370d20, struct.pack('<800f', *[r.uniform(-1, 1) for _ in range(800)]))
    w32(ctx, 0x10152418, r.choice([0, 1, -5, 3, 160, r.randrange(-3, 400)]))
    return [cam]


def g_100d280d(ctx):
    r = ctx.rng
    begin(ctx)
    sp = r.choice([0.0, 0.0, 0.02, 0.1])
    def fv(x): return fb(x) if r.random() >= sp else fbits(r, 10.0, 1.0)
    w32(ctx, 0x1015ef0c, r.choice([0, 0, 1]))
    cam_ = ctx.buf(0x100)
    fwrite(ctx, cam_ + 0x18, fv(r.uniform(0.2, 3)))
    fwrite(ctx, cam_ + 0x40, fv(r.choice([100.0, 1000.0, 50.0, r.uniform(1, 2000)])))
    fwrite(ctx, cam_ + 0xa8, fv(r.choice([1.0, 0.5, 2.0, 0.0, r.uniform(0.1, 3)])))
    fwrite(ctx, cam_ + 0xac, fv(r.choice([1.0, 0.5, 2.0, r.uniform(0.1, 3)])))
    fwrite(ctx, cam_ + 0xb0, fv(r.uniform(0.2, 3)))
    w32(ctx, 0x1014cae0, cam_)
    for a in range(0x10185c20, 0x10185c54, 4):
        fwrite(ctx, a, fv(r.uniform(-1, 1)) if r.random() < 0.9 else fbits(r, 2, 0.5))
    fwrite(ctx, 0x10185c50, fv(r.choice([0.5, 1.0, 10.0, -5.0])))
    fwrite(ctx, 0x10185bf8, fv(r.choice([1000.0, 100.0, 5000.0, 10.0])))
    for a in (0x10185c48, 0x10185c44, 0x10185c4c):
        fwrite(ctx, a, fv(r.choice([0.0, r.uniform(-100, 100)])))
    obj = ctx.buf(0xc0)
    spr = r.choice([10.0, 200.0, 2000.0])
    for off in (0x44, 0x48, 0x4c):
        fwrite(ctx, obj + off, fbits(r, spr, sp + 0.02))
    fwrite(ctx, obj + 0x50, fv(r.choice([0.0, 1.0, 5.0, 50.0, r.uniform(0, 100)])))
    wb(ctx, obj + 2, struct.pack('<H', r.choice([0xa0, 0xa5, 0x10, r.getrandbits(16)])))
    wb(ctx, obj + 0xa0, bytes([r.choice([0x57, 0x57, 0x41, r.getrandbits(8)])]))
    return [obj]


def S(name, addr, gen, trials, **kw):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials)
    d.update(kw)
    return d


SPECS = [
    S('Mat3x4_TransformPoint', 0x1005fbb0, g_5fbb0, 4000, void=True),
    S('Mat3_CrossColumn', 0x1005fcdc, g_5fcdc, 4000, void=True),
    S('Mat3_Multiply', 0x1005fe48, g_5fe48, 4000, void=True),
    S('Mat3x4_Multiply', 0x10060008, g_5fe48, 4000, void=True),
    S('Mat3x4_InvertRigid', 0x10060101, g_60101, 4000, void=True),
    S('Sim_100b403d', 0x100b403d, g_100b403d, 3000, void=True),
    S('Sim_100d2306', 0x100d2306, g_100d2306, 3000, void=True),
    S('Sim_100d280d', 0x100d280d, g_100d280d, 4000),
]
