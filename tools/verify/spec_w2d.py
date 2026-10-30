# Differential tests for the wave 2 functions of letter d (see difftest.py).
import struct

SOURCES = [
    'sim/view/viewrect.c', 'sim/view/matrix.c', 'sim/view/objlist.c', 'sim/view/w2d_view.c',
    'sim/util/ring.c', 'sim/util/field.c',
    'sim/unsorted_1003/flags.c', 'sim/unsorted_1003/entry.c', 'sim/unsorted_1003/slots.c', 'sim/unsorted_1003/setters.c',
    'sim/unsorted_100d/flags.c', 'sim/unsorted_100d/w2d_state.c',
    'sim/unsorted_1009/checks.c', 'sim/unsorted_100b/fields.c', 'sim/unsorted_100c/entries.c',
]


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def wr(ctx, addr, fmt, *vals):
    ctx.m.uc.mem_write(addr, struct.pack(fmt, *[v & 0xffffffff for v in vals]))


def pick(ctx, *choices):
    return ctx.rng.choice(choices)


# ---- 0x1008a37f ------------------------------------------------------------
def g_a37f(ctx):
    rg = ctx.rng
    if rg.random() < 0.06:
        w32(ctx, 0x1017c8cc, rg.getrandbits(1))
        return [0]
    nodes = [ctx.buf(0x40) for _ in range(5)]
    node = nodes[0]
    flags = rg.getrandbits(16)
    flags = flags | 0x800 if rg.random() < 0.2 else flags & ~0x800
    ptr = lambda: 0 if rg.random() < 0.2 else rg.choice(nodes[1:])
    wr(ctx, node, '<H', flags)
    wr(ctx, node + 0xc, '<II', ptr(), ptr())     # second list: next? at +0xc, +0x10
    wr(ctx, node + 0x14, '<II', ptr(), ptr())    # first list links
    w32(ctx, 0x1017c8cc, pick(ctx, 0, 1, 1, 0x1234, rg.getrandbits(32)))
    return [node]


# ---- 0x1008f747 ------------------------------------------------------------
def g_f747(ctx):
    rg = ctx.rng
    a, b, out = ctx.buf(20), ctx.buf(20), ctx.buf(20)
    r = rg.random()
    if r < 0.1:
        out = a
    elif r < 0.2:
        out = b
    den = rg.choice([rg.randrange(1, 1000), rg.randrange(1, 1000), 1, 2, 3, 7, 200, 1000, -rg.randrange(1, 1000)])
    step = rg.choice([0, 1, 5, 17, rg.randrange(0, 60), -rg.randrange(1, 30)])
    w32(ctx, 0x1015dc80, step)
    lo = -abs(den)
    hi = abs(den) * 3 // 2
    acc = rg.choice([rg.randrange(lo, hi + 1), rg.randrange(0, abs(den) + 1), abs(den) - step, abs(den) - step - 1,
                     0, rg.randrange(-3000, 3000)])
    state = 1 if rg.random() < 0.85 else rg.choice([0, 2, 0xffffffff, rg.getrandbits(32)])
    prev = 0 if rg.random() < 0.25 else rg.choice([1, 2, rg.getrandbits(32)])
    O = ctx.buf(0x10, struct.pack('<4I', den & 0xffffffff, a, b, out))
    N = ctx.buf(12, struct.pack('<3I', state, prev, acc & 0xffffffff))
    pair = ctx.buf(8, struct.pack('<2I', N, O))
    return [pick(ctx, 0, 0, 1, 1, rg.getrandbits(32)), pair]


# ---- matrix pair ---------------------------------------------------------------
def g_25eb(ctx):
    src = ctx.buf(0x40)
    dst = ctx.buf(0x100)
    return [src if ctx.rng.random() < 0.1 else dst, src]


def g_262a(ctx):
    src = ctx.buf(0x100)
    dst = ctx.buf(0x40)
    return [src, src if ctx.rng.random() < 0.1 else dst]


# ---- unsorted_1009 -------------------------------------------------------------
def g_checks(ctx):
    rg = ctx.rng
    w32(ctx, 0x1014cb38, rg.choice([0, 0x100, 0x200, 0x300, 0x800, 0xb00, 0xf00, rg.getrandbits(32)]))
    w32(ctx, 0x1014cb34, rg.choice([0, 1, 2, rg.getrandbits(32)]))
    w32(ctx, 0x1017d1b0, rg.choice([0, 0, 1, rg.getrandbits(32)]))
    w32(ctx, 0x10152418, rg.choice([0, 1, 1, 2, 0xffffffff, rg.getrandbits(32)]))
    return [ctx.u32()]


# ---- 0x100b3530 ----------------------------------------------------------------
def g_b3530(ctx):
    w32(ctx, 0x1015d0a4, ctx.buf(0x40))
    return [ctx.u32(), ctx.u32()]


# ---- 0x100b53f0 ----------------------------------------------------------------
def g_b53f0(ctx):
    rg = ctx.rng
    limit = rg.choice([0, 3, 8, 20, -1, rg.randrange(-3, 30)])
    w32(ctx, 0x1017d218, limit)
    idx = rg.choice([rg.randrange(-2, 25), rg.randrange(0, max(limit, 0) + 1), limit, limit + 1, 0x7fffffff,
                     0x80000000, 0xffffffff])
    idx &= 0xffffffff
    sidx = idx - (1 << 32) if idx >= 1 << 31 else idx
    entry = (0x101a2100 + sidx * 0x88) & 0xffffffff
    if 0 <= sidx <= 40:
        blk = ctx.buf(0x100)
        obj = ctx.buf(0x100)
        wr(ctx, entry, '<34I', *[rg.getrandbits(32) for _ in range(34)])
        wr(ctx, entry + 0x28, '<II', blk if rg.random() < 0.5 else 0, obj if rg.random() < 0.75 else 0)
    outs = [ctx.buf(4, 'rand') for _ in range(3)]
    if rg.random() < 0.1:
        outs[1] = outs[0]
    return [idx] + outs


# ---- 0x100beea2 ----------------------------------------------------------------
def g_eea2(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x520)
    v = rg.choice([0, 1, 5, 0x13, 0x14, 0x15, 0x40, -1, -0x1000, rg.randrange(1, 0x14), rg.getrandbits(32)])
    w32(ctx, obj + 0x514, v)
    w32(ctx, 0x1017cd38, rg.randrange(0, 0x7f))
    wr(ctx, 0x101cffc0, '<128I', *[rg.choice([rg.getrandbits(32), rg.randrange(0, 100)]) for _ in range(128)])
    return [obj]


# ---- 0x100bf026 ----------------------------------------------------------------
def g_f026(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x200)
    kinds = [1, 5, 8, 0, 2, 7, rg.getrandbits(32)]
    wr(ctx, obj, '<I', rg.choice(kinds))
    idx = rg.randrange(0, 256)
    word = (idx | (0x200 if rg.random() < 0.75 else 0) | (rg.getrandbits(8) << 8 & ~0x200)) & 0xffff
    wr(ctx, obj + 0x1aa, '<H', word)
    for i in {idx, rg.randrange(0, 256)}:
        e = 0 if rg.random() < 0.2 else ctx.buf(8, struct.pack('<2I', rg.choice(kinds), 0))
        w32(ctx, 0x109e8b40 + 4 * i, e)
    return [obj]


# ---- 0x100c459f ----------------------------------------------------------------
def g_c459f(ctx):
    rg = ctx.rng
    f10 = rg.choice([0, 0, 1, 0xffffffff, rg.getrandbits(32)])
    f14 = rg.choice([0xffffffff, 0, 1, 5, rg.getrandbits(32)])
    entry = 0 if rg.random() < 0.1 else ctx.buf(0x20, struct.pack('<8I', *([rg.getrandbits(32)] * 4 + [f10, f14] + [0, 0])))
    want = rg.choice([f14, f14, 0xffffffff, rg.getrandbits(32)])
    return [entry, ctx.buf(8, struct.pack('<2I', rg.getrandbits(32), want))]


# ---- 0x100c48fa ----------------------------------------------------------------
def g_c48fa(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x60)
    wr(ctx, obj + 0x54, '<I', 0 if rg.random() < 0.1 else ctx.buf(0x80))
    return [0 if rg.random() < 0.08 else obj, rg.choice([0, 1, 2, 3, 0xffffffff, rg.getrandbits(32)])]


# ---- 0x100d709d ----------------------------------------------------------------
def g_709d(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x100)
    blk = ctx.buf(0x600)
    other = ctx.buf(0x100)
    wr(ctx, obj + 0x28, '<I', blk)
    wr(ctx, obj + 0x4c, '<I', other)
    for i in range(24):
        wr(ctx, blk + 0x498 + 4 * i, '<I', 0 if rg.random() < 0.25 else ctx.buf(0x100))
    q = ctx.buf(0x130)
    wr(ctx, q, '<I', 0xffffffff if rg.random() < 0.3 else rg.getrandbits(32))
    wr(ctx, q + 0x11c, '<I', rg.randrange(0, 24))
    outs = [ctx.buf(4) for _ in range(3)]
    return [obj] + outs + [q]


# ---- 0x100dc286 ----------------------------------------------------------------
def g_c286(ctx):
    rg = ctx.rng
    count = rg.choice([0, 1, 4, 8, rg.randrange(-2, 12)])
    idx = rg.choice([rg.randrange(-2, count + 2), 0, count - 1, count])
    w32(ctx, 0x1017e6a0, count)
    w32(ctx, 0x1017e6a4, idx)
    n2 = rg.randrange(0, 9)
    w32(ctx, 0x1015ea80, n2)
    wr(ctx, 0x1017eec0 - 16, '<16I', *[rg.choice([rg.randrange(-2, 11), rg.getrandbits(32)]) for _ in range(16)])
    wr(ctx, 0x109b9e20, '<%dI' % (0x7e * 10 // 4 + 1), *[rg.getrandbits(32) for _ in range(0x7e * 10 // 4 + 1)])
    return []


SPECS = [
    dict(name='Sim_1008a37f', orig=0x1008a37f, c='Sim_1008a37f', gen=g_a37f, void=True, trials=600),
    dict(name='Sim_1008f747', orig=0x1008f747, c='Sim_1008f747', gen=g_f747, trials=1000),
    dict(name='Sim_100988b9', orig=0x100988b9, c='Sim_100988b9', gen=g_checks, trials=400),
    dict(name='Sim_10098966', orig=0x10098966, c='Sim_10098966', gen=g_checks, trials=400),
    dict(name='Sim_100989dd', orig=0x100989dd, c='Sim_100989dd', gen=g_checks, trials=400),
    dict(name='Sim_100b3530', orig=0x100b3530, c='Sim_100b3530', gen=g_b3530, void=True, trials=300),
    dict(name='Sim_100b53f0', orig=0x100b53f0, c='Sim_100b53f0', gen=g_b53f0, void=True, trials=600),
    dict(name='Sim_100beea2', orig=0x100beea2, c='Sim_100beea2', gen=g_eea2, trials=500),
    dict(name='Sim_100bf026', orig=0x100bf026, c='Sim_100bf026', gen=g_f026, trials=800),
    dict(name='Sim_100c459f', orig=0x100c459f, c='Sim_100c459f', gen=g_c459f, trials=500),
    dict(name='Sim_100c48fa', orig=0x100c48fa, c='Sim_100c48fa', gen=g_c48fa, void=True, trials=500),
    dict(name='Sim_100d25eb', orig=0x100d25eb, c='Sim_100d25eb', gen=g_25eb, void=True, trials=300),
    dict(name='Sim_100d262a', orig=0x100d262a, c='Sim_100d262a', gen=g_262a, void=True, trials=300),
    dict(name='Sim_100d709d', orig=0x100d709d, c='Sim_100d709d', gen=g_709d, void=True, trials=500),
    dict(name='Sim_100dc286', orig=0x100dc286, c='Sim_100dc286', gen=g_c286, trials=500),
]
