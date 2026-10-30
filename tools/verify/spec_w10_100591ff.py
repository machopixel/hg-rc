# Round 10: 0x100591ff (shade level). Stubbed on both sides: Sim_100a6414 (light of the polygon, from rets) and
# Sim_1005897b (lamp hits the polygon, from rets); the light position globals are compared in memory. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1005/w10_misc.c'])
helper(0x100a6414, 'Sim_100a6414', 2)
helper(0x1005897b, 'Sim_1005897b', 3)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, 0x100a6414, 0x1005897b)
    cam = ctx.buf(0x20, b''.join(struct.pack('<I', fb(r.uniform(-1000, 1000))) for _ in range(8)))
    w32(ctx, 0x1014cae0, cam)
    for a in (0x101668c0, 0x101668c4, 0x101668c8, 0x109b16c4, 0x109b16c8, 0x109b16c0, 0x10143fc0, 0x101668b8,
              0x101668bc):
        w32(ctx, a, fb(r.uniform(-500, 500)))
    w32(ctx, 0x1014cb1c, r.choice([3, 0, 1, 2]))
    w32(ctx, 0x101668d8, r.choice([0, 0, 1]))
    w32(ctx, 0x109afdcc, r.choice([0, 1]))
    w32(ctx, 0x10143fd4, r.choice([0, 1]))
    w32(ctx, 0x10143fd8, r.choice([0, 1, 1]))
    w32(ctx, 0x101668b4, r.choice([0, 10, 60, 127, 128, 300, r.getrandbits(32)]))
    w32(ctx, 0x10143fbc, r.choice([0, 1, 64, 128, 255, r.getrandbits(32)]))
    w32(ctx, 0x1014cb2c, r.choice([0, 1, 10, 100, 0xffffffff, r.getrandbits(32)]))
    prev = 0x101668d0
    for i in range(r.randrange(0, 6)):
        e = newbuf(ctx, 0x20)
        w32(ctx, prev, e)
        prev = e
    w32(ctx, prev, 0)
    rets(0x100a6414, *[r.choice([0, 1, 5, 0x40, 0x7f, 0x80, r.getrandbits(32)]) for _ in range(3)])
    rets(0x1005897b, r.choice([0, 1]), r.choice([0, 1]), 1)
    d = fb(r.uniform(0, 5000)) if r.random() < 0.95 else rfloat(r)
    b = r.choice([0, 1, 2, 64, 255, 1000, r.getrandbits(32)])
    return [newbuf(ctx, 0x20), newbuf(ctx, 0x20), b, d]


SPECS = [dict(name='Sim_100591ff', orig=0x100591ff, c='Sim_100591ff', gen=g, trials=4000)]
