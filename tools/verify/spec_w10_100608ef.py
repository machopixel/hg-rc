# Round 10, interpreted (DECISIONS.md): 0x100608ef (quaternion slerp). The CRT assert (0x100ff860) is stubbed on both
# sides; the CRT acos / sin of the original against the platform math library of the C (libm()). Exact but for the
# four output floats, which may differ within the stated tolerance. t also takes inf / NaN for the domain wrappers;
# huge finite t is left out (the CRT reduces sin's argument with an 80-bit 2 pi, see DECISIONS.md).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/util/w10_interp.c'])
helper(0x100ff860, None, 3, sargs=(0, 1))
ACTIVE = tuple(HELPERS)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def quat(r):
    v = [r.uniform(-1, 1) for _ in range(4)]
    n = sum(x * x for x in v) ** 0.5 or 1.0
    return [x / n for x in v]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    libm(ctx)
    a = quat(r)
    mode = r.random()
    if mode < 0.15:
        b = list(a)                                   # equal: linear branch
    elif mode < 0.3:
        b = [-x for x in a]                           # opposite: perpendicular branch
    elif mode < 0.4:
        b = [x + r.uniform(-1e-3, 1e-3) for x in a]   # near the thresholds
    else:
        b = quat(r)
    qa, qb = newbuf(ctx, 16), newbuf(ctx, 16)
    w32(ctx, qa, *[fb(x) for x in a])
    w32(ctx, qb, *[fb(x) for x in b])
    t = r.choice([fb(r.uniform(0, 1)), fb(r.uniform(0, 1)), fb(r.uniform(-2, 3)), 0, fb(1.0), 0x7f800000, 0x7fc00000])
    return [qa, qb, t, newbuf(ctx, 16)]


SPECS = [dict(name='Sim_100608ef', orig=0x100608ef, c='Sim_100608ef', gen=g, void=True, trials=15000, fp=True,
              tol=1.2e-7)]
