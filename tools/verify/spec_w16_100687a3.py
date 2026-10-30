# Round 16: 0x100687a3, the within-range test (DECISIONS.md round 16). Sim_10068716 (the nearest box point) runs for
# real on both sides. The original's three never-written locals [ebp-0x18], [ebp-0x1c], [ebp-4] are given non-zero
# leftovers before each call (the values seen in play with the original code, 0x447a0000 / 0xc2524a26 / 0x0019f53c,
# and random non-zero, non-NaN floats), the case the C reproduces (the test always runs).
# Negative check: W16_NEG=1 lets a leftover be 0.0 or NaN, where the original skips the test and the C does not.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w16_range.c'])
NEG = bool(os.environ.get('W16_NEG'))
SEEN = [0x447a0000, 0xc2524a26, 0x4477b1c0, 0xc23fe200, 0x0019f53c, 0x00483222, 0x00000005, 0x3f800000]


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def leftover(r):
    if NEG and r.random() < 0.5:
        return r.choice([0, 0x80000000, 0x7fc00000, 0xffc00000])
    while True:
        v = r.choice(SEEN + [r.getrandbits(32), fb(r.uniform(-5000, 5000))])
        f = struct.unpack('<f', struct.pack('<I', v))[0]
        if f != 0.0 and f == f:
            return v


def g(ctx):
    r = begin(ctx)
    box = ctx.buf(24)
    lo = [r.uniform(-500, 500) for _ in range(3)]
    vals = []
    for a in lo:
        vals += [a, a + r.choice([0.0, r.uniform(0, 300)])]
    if r.random() < 0.1:
        vals[r.randrange(6)] = r.choice([float('nan'), float('inf')])
    wf(ctx, box, *vals)
    outs = [ctx.buf(4) for _ in range(3)]
    pt = [fb(r.uniform(-800, 800)) if r.random() < 0.9 else rfloat(r) for _ in range(3)]
    radius = r.choice([fb(r.uniform(0, 400)), fb(0.0), fb(-50.0), rfloat(r)])
    ebp = frame(8)
    for off in (0x18, 0x1c, 4):
        w32(ctx, ebp - off, leftover(r))
    args = [box if r.random() < 0.95 else 0] + pt + [radius] + [o if r.random() < 0.97 else 0 for o in outs]
    return args


SPECS = [dict(name='Sim_100687a3', orig=0x100687a3, c='Sim_100687a3', gen=g, trials=500 if NEG else 20000)]
