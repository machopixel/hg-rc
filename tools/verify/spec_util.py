# Differential tests for src/sim/util (see difftest.py).
import struct

SOURCES = ['sim/util/lookup.c', 'sim/util/ring.c', 'sim/util/field.c', 'sim/util/record.c']


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))

def w16(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<H', v & 0xffff))

def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))

def rnd(ctx, addr, n):
    wb(ctx, addr, (ctx.rng.getrandbits(8) for _ in range(n)))

def junk16(ctx, v):
    """A 16-bit argument pushed as a dword: the half the function must ignore is garbage."""
    return (v & 0xffff) | (ctx.rng.getrandbits(16) << 16 if ctx.rng.random() < 0.5 else 0)

def ptrtab(ctx, n, size=0x40, fields=None):
    """Pointer table 0x109e8b40 with n records; fields(i, addr) fills each."""
    recs = []
    for i in range(n):
        a = ctx.buf(size)
        if fields: fields(i, a)
        w32(ctx, 0x109e8b40 + 4 * i, a)
        recs.append(a)
    return recs


# ---- field.c ----
def g_p(ctx):
    return [ctx.buf(0x100)]
def g_p_val(ctx):
    return [ctx.buf(0x100), ctx.u32()]
def g_p_or_null_16(ctx):
    return [ctx.buf(0x40) if ctx.rng.random() < 0.8 else 0, junk16(ctx, ctx.rng.getrandbits(16))]
def g_get3(ctx):
    r = ctx.rng
    p = ctx.buf(0x100)
    base = r.choice([0x70, 0x30])
    mode = r.random()
    if mode < 0.6:
        outs = [ctx.buf(4, 'rand') for _ in range(3)]
    elif mode < 0.8:   # the destinations lie inside the source
        outs = [p + base + 4 * r.randrange(0, 3) for _ in range(3)]
    else:
        o = ctx.buf(4)
        outs = [o, o, o]
    return [p] + outs
def g_set3(ctx):
    return [ctx.buf(0x100), ctx.u32(), ctx.u32(), ctx.u32()]
def g_3_get(ctx):
    g = g_get3(ctx)
    return g
def g_set_ptr_byte(ctx):
    p = ctx.buf(0x100)
    q = ctx.buf(0x80)
    w32(ctx, p + 0x54, q)
    return [p, ctx.rng.getrandbits(32)]
def g_clear54(ctx):
    r = ctx.rng
    p = ctx.buf(0x100)
    q = ctx.buf(0x80)
    w32(ctx, p + 0x54, q if r.random() < 0.8 else 0)
    return [p if r.random() < 0.9 else 0]
def g_g1(ctx):
    w32(ctx, 0x1017e5b8, ctx.u32()); return []
def g_u32(ctx):
    return [ctx.u32()]


# ---- ring.c ----
def ring(ctx, idx_addr, tab_addr):
    r = ctx.rng
    idx = r.choice([0, 1, 125, 126, 127, 128, r.randrange(0, 127), r.randrange(0, 127), r.randrange(0, 127)])
    w32(ctx, idx_addr, idx)
    wb(ctx, tab_addr, struct.pack('<140I', *[r.choice([ctx.u32(), r.randrange(0, 1000), r.getrandbits(32)]) for _ in range(140)]))
def g_ring_mod(ctx):
    ring(ctx, 0x1017cd38, 0x101cffc0)
    return [ctx.rng.choice([100, 2, 6, 10, 1, -1, -7, 0, ctx.rng.randrange(1, 5000), ctx.u32()])]
def g_ring(ctx):
    ring(ctx, 0x1017cd3c, 0x101cfce0)
    return []


# ---- lookup.c ----
def g_100c749(ctx):
    r = ctx.rng
    n, k = 6, 5
    arr = ctx.buf(0x128 * n)
    for i in range(n):
        w32(ctx, arr + 0x128 * i + 4, r.randrange(0, k))
    rec = 0x10145508
    rnd(ctx, rec, 0xd6 * k + 8)
    for i in range(k):
        w32(ctx, rec + 0xd6 * i + 0x86, r.choice([0, 0, 1, 0x10, 0x8001, r.getrandbits(32)]))
    p = ctx.buf(0x100)
    w32(ctx, p + 0x84, arr)
    mask = r.choice([1, 0x10, 0x8000, 0xffffffff, 0, 3, r.getrandbits(32)])
    return [p if r.random() < 0.9 else 0, r.randrange(0, n), mask]

def g_10011b9e(ctx):
    r = ctx.rng
    n = r.randrange(0, 9)
    t = ctx.buf(8 * 12)
    for i in range(12):
        wb(ctx, t + 8 * i, struct.pack('<II', ctx.u32(), r.choice([r.randrange(0, 6), 0xffffffff, 0xffff, r.getrandbits(32)])))
    key = r.choice([r.randrange(0, 6), -1, -2, r.randrange(-32768, 32768)])
    return [junk16(ctx, key), t, junk16(ctx, r.choice([n, n, n, -1, 0]))]

def g_10015c61(ctx):
    r = ctx.rng
    n = 8
    def f(i, a):
        w32(ctx, a + 8, r.randrange(0, 3)); w32(ctx, a + 0x10, r.randrange(0, 5))
    ptrtab(ctx, n, 0x40, f)
    w32(ctx, 0x1017b8c8, r.randrange(0, n))
    w32(ctx, 0x1015dc64, r.choice([r.randrange(0, n + 1), n, 0, 0xffffffff]))
    if r.random() < 0.1: w32(ctx, 0x109e8b40 + 4 * r.randrange(0, n), 0)
    return [r.choice([r.randrange(0, 5), 0xffffffff, ctx.u32()])]

def g_10016cb7(ctx):
    r = ctx.rng
    N = 10
    def f(i, a):
        w16(ctx, a + 0x1c, r.choice([0, 0, 2, 4, 6, 0x10, 0x11, 1, 0x8000, 0xffff, r.getrandbits(16)]))
    ptrtab(ctx, N, 0x40, f)
    for g in range(4):
        base = 0x109b7d40 + 0x6c * g
        rnd(ctx, base, 0x6c)
        w32(ctx, base + 4, r.randrange(0, 7)); w32(ctx, base + 8, r.choice([0, 0, 1, 5]))
        for j in range(20): w32(ctx, base + 0x1c + 4 * j, r.randrange(0, N))
    out = ctx.buf(0x40, 'zero')
    return [r.randrange(0, 4), out, r.choice([0, 1, 0, 7])]

def g_10015_pairs(ctx, cnt_addr, ptr_addr, cap):
    r = ctx.rng
    n = r.choice([0, 1, 3, 5, 8, r.randrange(0, 9)])
    t = ctx.buf(8 * 10)
    for i in range(10):
        wb(ctx, t + 8 * i, struct.pack('<II', r.randrange(0, 4), ctx.u32()))
    w32(ctx, cnt_addr, n); w32(ctx, ptr_addr, t)
    return t

def g_100b36d8(ctx):
    g_10015_pairs(ctx, 0x1017d20c, 0x1017d214, 0)
    return [ctx.rng.choice([0, 1, 2, 3, 4, 0xffffffff])]

def g_arrays(ctx, cnt_addr, a_addr, b_addr, signed_key=False):
    r = ctx.rng
    n = r.choice([0, 1, 3, 5, 8, r.randrange(0, 9)])
    A = ctx.buf(4 * 10); B = ctx.buf(4 * 10)
    wb(ctx, A, struct.pack('<10I', *[r.randrange(0, 4) if r.random() < 0.9 else r.getrandbits(32) for _ in range(10)]))
    wb(ctx, B, struct.pack('<10I', *[ctx.u32() for _ in range(10)]))
    w32(ctx, cnt_addr, n if r.random() < 0.95 else -r.randrange(1, 5))
    w32(ctx, a_addr, A); w32(ctx, b_addr, B)
def g_100b3839(ctx):
    g_arrays(ctx, 0x101c42a0, 0x1017d1f8, 0x1017d1f4)
    return [ctx.rng.choice([0, 1, 2, 3, 4, 0xffffffff])]
def g_100b38c5(ctx):
    g_arrays(ctx, 0x101a1168, 0x1017d208, 0x1017d204)
    return [ctx.rng.choice([0, 1, 2, 3, 4, -1, -5, 0x80000000, 0x7fffffff])]

def g_100b399c(ctx):
    return [ctx.rng.choice([0, 1, 7, -1, -100, ctx.rng.randrange(0, 200), ctx.u32()])]

def g_100c3668(ctx):
    r = ctx.rng
    n = r.randrange(0, 6)
    arr = ctx.buf(0x128 * 6)
    for i in range(6):
        a = arr + 0x128 * i
        w32(ctx, a + 8, r.choice([0xffffffff, 0xffffffff, 1, 0, 5]))
        w32(ctx, a + 0x20, r.choice([0, 0, 1, 0x100]))
    p = ctx.buf(0x500 + 8)
    w32(ctx, p + 0x84, arr)
    w32(ctx, p + 0x4f8, n if r.random() < 0.95 else r.choice([-1, 0x10000, 0x8000]))
    return [p]

def g_100f2cec(ctx):
    r = ctx.rng
    base = ctx.buf(0x41000, 'zero')
    tab = ctx.buf(0x600, 'zero')
    wb(ctx, tab, struct.pack('<256H', *[r.choice([0, 0, r.randrange(0, 0x300), r.randrange(0, 0x300)]) for _ in range(256)]))
    ents = [r.randrange(0, 0x40000 - 4) for _ in range(0x1000)]
    wb(ctx, base, struct.pack('<4I', *[r.randrange(0, 0x40000 - 4) for _ in range(4)]))
    # dwords reachable at base+0x10+4*index, index up to 0x3ff+0xff = about 0x4ff
    wb(ctx, base + 0x10, struct.pack('<%dI' % 0x600, *[r.randrange(0, 0x40000) for _ in range(0x600)]))
    wb(ctx, base + 0x1800, struct.pack('<%dI' % 0x1000, *[r.getrandbits(32) for _ in range(0x1000)]))
    w32(ctx, 0x10157590, tab)
    w32(ctx, 0x10157594, r.getrandbits(32))
    v = r.getrandbits(32) if r.random() < 0.03 else r.choice([r.randrange(0, 0x10000), r.randrange(0, 0x100), r.randrange(0, 0x10000)])
    return [base, v]


# ---- record.c ----
def g_1000711c(ctx):
    r = ctx.rng
    for i in range(34):
        a = 0x10ac1b00 + 14 * i
        wb(ctx, a, bytes(r.getrandbits(8) for _ in range(14)))
        w16(ctx, a + 8, r.choice([0xfffe, 0xfffe, 1, 0, r.getrandbits(16)]))
        w16(ctx, a + 4, r.choice([0, 0, 5, r.getrandbits(16)]))
    idx = r.randrange(-30000, 30000) if r.random() < 0.03 else r.choice([r.randrange(0, 33), r.randrange(0, 33), -1])
    return [junk16(ctx, idx), junk16(ctx, r.getrandbits(16))]

def g_10009136(ctx):
    r = ctx.rng
    big = ctx.buf(0x800)
    t = big + 0x200
    cnt = r.choice([0, 1, 2, 5, 0, 3, 0xffff, 0xfffe])
    w16(ctx, t + 4, cnt)
    p = ctx.buf(0x100)
    w32(ctx, p + 0x30, t if r.random() < 0.9 else 0)
    return [p] + [ctx.u32() for _ in range(11)]

def g_10009278(ctx):
    r = ctx.rng
    p = ctx.buf(0x100)
    w32(ctx, p + 0x30, r.choice([0, 7, 0xffffffff]))
    q = ctx.buf(0x100)
    idx = r.randrange(0, 0x40)
    if r.random() < 0.2:
        w16(ctx, q + 2, idx); w32(ctx, q + 4, (2 - idx) & 0xffffffff)   # the byte lands on the count
    elif r.random() < 0.1:
        w16(ctx, q + 2, 0xffff); w32(ctx, q + 4, 0x20)
    else:
        w16(ctx, q + 2, idx); w32(ctx, q + 4, r.randrange(0, 0x40))
    return [p, q, ctx.u32()]

def g_100a88db(ctx):
    for a in (0x1017d170, 0x1017d174, 0x1017d178, 0x1014d1a4):
        w32(ctx, a, ctx.u32() if ctx.rng.random() < 0.5 else ctx.rng.getrandbits(32))
    return []

def g_100ca350(ctx):
    r = ctx.rng
    rnd(ctx, 0x109a8640, 0x54 * 132)
    g = r.choice([0, 1, 5, 126, 127, 128, 0x80, 0xffffffff, 0xfffffffe, r.randrange(0, 128), r.randrange(0, 128)])
    w32(ctx, 0x1017e5a8, g)
    N = 8
    def f(i, a): w32(ctx, a + 8, ctx.u32())
    ptrtab(ctx, N, 0x40, f)
    a = r.choice([r.randrange(0, 0x10000), ctx.u32()]) if r.random() < 0.03 else r.randrange(0, N)
    return [a, ctx.u32(), ctx.u32(), ctx.u32()]


SPECS = [
    # lookup.c
    dict(name='Sim_1000c749', orig=0x1000c749, c='Sim_1000c749', gen=g_100c749, trials=400),
    dict(name='Sim_10011b9e', orig=0x10011b9e, c='Sim_10011b9e', gen=g_10011b9e, trials=500),
    dict(name='Sim_10015c61', orig=0x10015c61, c='Sim_10015c61', gen=g_10015c61, trials=500),
    dict(name='Sim_10016cb7', orig=0x10016cb7, c='Sim_10016cb7', gen=g_10016cb7, trials=500),
    dict(name='Sim_100b36d8', orig=0x100b36d8, c='Sim_100b36d8', gen=g_100b36d8, trials=400),
    dict(name='Sim_100b3839', orig=0x100b3839, c='Sim_100b3839', gen=g_100b3839, trials=400),
    dict(name='Sim_100b38c5', orig=0x100b38c5, c='Sim_100b38c5', gen=g_100b38c5, trials=400),
    dict(name='Sim_100b399c', orig=0x100b399c, c='Sim_100b399c', gen=g_100b399c),
    dict(name='Sim_100c3668', orig=0x100c3668, c='Sim_100c3668', gen=g_100c3668, trials=500),
    dict(name='Sim_100f2cec', orig=0x100f2cec, c='Sim_100f2cec', gen=g_100f2cec, trials=500),
    # ring.c
    dict(name='Sim_1009c929', orig=0x1009c929, c='Sim_1009c929', gen=g_ring_mod, trials=500),
    dict(name='Sim_1009c969', orig=0x1009c969, c='Sim_1009c969', gen=g_ring, trials=400),
    # field.c
    dict(name='Sim_10008c69', orig=0x10008c69, c='Sim_10008c69', gen=g_p_val, void=True),
    dict(name='Sim_10009408', orig=0x10009408, c='Sim_10009408', gen=g_p_or_null_16, void=True),
    dict(name='Sim_100096ee', orig=0x100096ee, c='Sim_100096ee', gen=g_p),
    dict(name='Sim_100096f9', orig=0x100096f9, c='Sim_100096f9', gen=g_p_val, void=True),
    dict(name='Sim_100a9b4c', orig=0x100a9b4c, c='Sim_100a9b4c', gen=g_p_val, void=True),
    dict(name='Sim_100a9b71', orig=0x100a9b71, c='Sim_100a9b71', gen=g_p),
    dict(name='Sim_100a9cfb', orig=0x100a9cfb, c='Sim_100a9cfb', gen=g_get3, void=True),
    dict(name='Sim_100a9d21', orig=0x100a9d21, c='Sim_100a9d21', gen=g_get3, void=True),
    dict(name='Sim_100a9d47', orig=0x100a9d47, c='Sim_100a9d47', gen=g_set3, void=True),
    dict(name='Sim_100c274a', orig=0x100c274a, c='Sim_100c274a', gen=g_set_ptr_byte, void=True),
    dict(name='Sim_100c48c9', orig=0x100c48c9, c='Sim_100c48c9', gen=g_clear54, void=True),
    dict(name='Sim_100cc8ba', orig=0x100cc8ba, c='Sim_100cc8ba', gen=g_u32),
    dict(name='Sim_100cc6e0', orig=0x100cc6e0, c='Sim_100cc6e0', gen=g_g1),
    # record.c
    dict(name='Sim_1000711c', orig=0x1000711c, c='Sim_1000711c', gen=g_1000711c, void=True, trials=500),
    dict(name='Sim_10009136', orig=0x10009136, c='Sim_10009136', gen=g_10009136, void=True, trials=500),
    dict(name='Sim_10009278', orig=0x10009278, c='Sim_10009278', gen=g_10009278, void=True, trials=500),
    dict(name='Sim_100a88db', orig=0x100a88db, c='Sim_100a88db', gen=g_100a88db, void=True),
    dict(name='Sim_100ca350', orig=0x100ca350, c='Sim_100ca350', gen=g_100ca350, trials=600),
]
