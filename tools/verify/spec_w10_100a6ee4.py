# Round 10: 0x100a6ee4 (edge against a z plane). Sim_100a88ff (the vertex stack) and the CRT fabs run for real.
# Exact (no tolerance).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100a/w10_misc.c'])


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def vert(ctx, z):
    r = ctx.rng
    vals = [fb(r.uniform(-1000, 1000)) if r.random() < 0.93 else rfloat(r) for _ in range(16)]
    vals[2] = z
    return ctx.buf(0x40, struct.pack('<16I', *vals))


def g(ctx):
    r = begin(ctx)
    top = 0x00A80000
    w32(ctx, 0x1017d174, top + r.choice([0x1000, 0x200, 0x108]))
    w32(ctx, 0x1017d178, top)
    za = r.uniform(-100, 100)
    zb = r.choice([za + r.uniform(-50, 50), za, za + 1e-8, za - 5e-8, za + 2e-7])
    a = vert(ctx, fb(za) if r.random() < 0.95 else rfloat(r))
    b = vert(ctx, fb(zb) if r.random() < 0.95 else rfloat(r))
    z = fb(r.uniform(min(za, zb) - 5, max(za, zb) + 5)) if r.random() < 0.95 else rfloat(r)
    w32(ctx, 0x10185c50, r.getrandbits(32))
    return [a, b, z]


SPECS = [dict(name='Sim_100a6ee4', orig=0x100a6ee4, c='Sim_100a6ee4', gen=g, trials=4000)]
