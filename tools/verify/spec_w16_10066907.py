# Round 16: 0x10066907 (unit-against-unit collision). Sim_10084a94 (the distance, converted) runs for real; the
# kind test 0x10079feb, the state-4 handler 0x1006863d and the three box tests 0x100688a3 / 0x100689c0 /
# 0x100690d4 (all converted) are logging stubs. The gear table 0x109e8b40 gets 1..8 units close to the moving one,
# with box flags, the "riding" link (+0x84), states and flags varied; the positions, radii and speeds are floats,
# sometimes NaN or infinity.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w16_range.c'])
CALLS = [(0x10079feb, 'Sim_10079feb', 1), (0x1006863d, 'Sim_1006863d', 1), (0x100688a3, 'Sim_100688a3', 8),
         (0x100689c0, 'Sim_100689c0', 8), (0x100690d4, 'Sim_100690d4', 5)]
for a, c, n in CALLS:
    helper(a, c, n)
ACTIVE = tuple(a for a, _, _ in CALLS)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def fl(r, lo=-60.0, hi=60.0):
    x = r.random()
    if x < 0.02:
        return r.choice([0x7fc00000, 0x7f800000, 0xff800000, 0, 0x80000000])
    return fb(r.uniform(lo, hi))


def g(ctx):
    r = begin(ctx, *ACTIVE)
    rets(0x10079feb, *[r.choice([0, 0, 1]) for _ in range(5)])
    rets(0x100688a3, *[r.choice([0, 1, 1, 2]) for _ in range(4)])
    rets(0x100689c0, *[r.choice([0, 1, 1]) for _ in range(4)])
    rets(0x100690d4, *[r.choice([0, 1, 1]) for _ in range(4)])
    n = r.randrange(1, 9)
    w32(ctx, 0x1015dc64, n + r.choice([0, 0, 1]))
    units = []
    for i in range(n + 1):
        unit = newbuf(ctx, 0x2a0)
        rec = newbuf(ctx, 0x600)
        w32(ctx, rec, unit)
        w32(ctx, unit + 0x28, rec if r.random() < 0.95 else 0)
        w32(ctx, unit + 4, i if r.random() < 0.9 else r.randrange(0, 3))
        w32(ctx, unit + 0x29c, r.choice([0, 0, 1, 2]))
        w32(ctx, unit + 0x84, r.choice([0xffffffff, 0, 1, 2, r.randrange(0, 9)]))
        w16(ctx, unit + 0x1c, r.choice([0, 0, 0x4000, 0x2000]))
        for off in (0x58, 0x5c, 0x60):
            w32(ctx, unit + off, fl(r))
        w32(ctx, rec + 0x53c, fl(r, 0.0, 40.0))
        for off in (0x554, 0x558, 0x55c):
            w32(ctx, rec + off, fl(r, -20.0, 20.0))
        w32(ctx, rec + 0x58c, r.choice([0, 2, 2, 1]))
        w16(ctx, rec + 0x560, r.choice([0, 0, 0x100, 0x20]))
        w32(ctx, rec + 0x4f0, r.choice([0, 1, 2, 4, 4, 5]))
        units.append((unit, rec))
    for i in range(0x1a):
        w32(ctx, 0x109e8b40 + 4 * i, units[i][0] if i < len(units) and r.random() < 0.95 else 0)
    selfUnit, selfRec = units[r.randrange(0, len(units))] if r.random() < 0.8 else units[0]
    xyz = [newbuf(ctx, 4) for _ in range(3)]
    for p in xyz:
        w32(ctx, p, fl(r))
    hit = newbuf(ctx, 4)
    for a in (0x1015dd58, 0x1015dd5c, 0x1015dd60):
        w32(ctx, a, r.getrandbits(32))
    return [selfRec] + xyz + [hit]


SPECS = [dict(name='Sim_10066907', orig=0x10066907, c='Sim_10066907', gen=g, trials=12000)]
