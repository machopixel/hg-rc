# Round 10: 0x1006b52f (a hit on a system cell of a damage block; recursive for empty cells). Stubbed on both sides:
# Sim_1006abf6, Sim_100203cf, Sim_1009b362, Sim_1003cbb3, Sim_1006bf33, Sim_10015630 and the renderer slot
# [0x109e3d40] (IND1). Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w10_misc.c'])
STUBS = ((0x1006abf6, 'Sim_1006abf6', 3), (0x100203cf, 'Sim_100203cf', 2), (0x1009b362, 'Sim_1009b362', 0),
         (0x1003cbb3, 'Sim_1003cbb3', 2), (0x1006bf33, 'Sim_1006bf33', 3), (0x10015630, 'Sim_10015630', 4))
for _a, _c, _n in STUBS:
    helper(_a, _c, _n)
helper(IND1, None, 2)
ACTIVE = tuple(a for a, _, _ in STUBS) + (IND1,)
CODES = [0, 0x1bbc, 0x1b8a, 0x1bee, 0x1c20, 0x1c52, 0x1c84, 0x1cb6, 0x1ce8, 0x1d1a, 0x1d4c, 0x1d7e, 0x1db0, 0x1de2,
         0x1e14, 0x1e46, 0x1e78, 0x2328, 0x1f40, 0x1eaa, 5,
         0x1bbd, 0x1c21, 0x1c85, 0x1ce9, 0x1d4d, 0x1db1, 0x1e15, 0x1e79, 0x1f41, 0x1eab]     # (codes between the cases)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    w32(ctx, 0x1017d108, r.choice([0, 1]))
    w32(ctx, 0x1015d1ac, r.choice([4, 1]))
    w32(ctx, 0x109e3d40, IND1)
    unit = newbuf(ctx, 0x60)
    w32(ctx, unit, r.choice([1, 1, 1, 9, 2]))
    w32(ctx, unit + 4, r.choice([self_, self_, r.randrange(0, 8)]))
    w32(ctx, unit + 0xc, r.choice([self_, r.randrange(0, 8)]))
    w32(ctx, unit + 0x14, r.choice([3, 1]))
    ctl = newbuf(ctx, 0x60)
    w8(ctx, ctl + 0x59, r.choice([0, 1]))
    w32(ctx, unit + 0x54, ctl)
    b = newbuf(ctx, 0xc60)
    wb(ctx, b, bytes(r.getrandbits(8) for _ in range(0xc60)))
    w32(ctx, b, unit)
    w32(ctx, b + 0x88, r.choice([newbuf(ctx, 0x3c * 16), newbuf(ctx, 0x3c * 16), 0x3c]))
    for off in (0xbab, 0xbaf, 0xbb3, 0xbb7, 0xbbb, 0xbbf, 0xbc3, 0xbc7, 0xc35, 0xbd7):
        w32(ctx, b + off, r.choice([0, 0, 1, 7]))
    w32(ctx, b + 0xbd3, r.choice([0, 0x200, 0x400, r.getrandbits(32)]))
    w32(ctx, b + 0x4f0, r.choice([2, 2, 1]))
    w16(ctx, b + 0x560, r.getrandbits(16))
    w32(ctx, b + 0x500, r.choice([fb(r.uniform(0, 2)), fb(-0.0), fb(-1.0)]))
    part = r.randrange(0, 12)
    k = r.randrange(0, 4)
    for kk in range(16):
        w32(ctx, b + 0x617 + part * 0x50 + kk * 4, r.choice(CODES))
    w32(ctx, b + 0xb67 + part * 4, r.randrange(0, 8))
    for p in range(12):
        w32(ctx, 0x10138d70 + 4 * p, r.randrange(0, 12))
    return [r.randrange(0, 8), b, part, k, r.choice([0, 0, 1])]


SPECS = [dict(name='Sim_1006b52f', orig=0x1006b52f, c='Sim_1006b52f', gen=g, void=True, trials=6000)]
