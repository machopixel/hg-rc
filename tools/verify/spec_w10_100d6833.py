# Round 10: 0x100d6833 (weapon block aim state). No callees but the CRT fmod / fabs, which run for real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100d/w10_misc.c'])


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def fl(r, c, sc):
    return fb(c + r.uniform(-sc, sc)) if r.random() < 0.97 else rfloat(r)


def g(ctx):
    r = begin(ctx)
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015dc80, r.randrange(0, 60))
    cam = newbuf(ctx, 0x20)
    w32(ctx, cam + 0x18, fl(r, 1.0, 0.5))
    w32(ctx, 0x1014cae0, cam)
    kind = r.randrange(0, 40)
    k = 0x10145508 + kind * 0xd6
    w32(ctx, k + 0x86, r.choice([1, 1, 0x21, 0, 0x20, r.getrandbits(32)]))
    rmin = r.choice([0.0, r.uniform(0, 300)])
    rmax = rmin + r.choice([r.uniform(0, 3000), 0.0])
    w32(ctx, k + 0x4c, fb(rmin))
    w32(ctx, k + 0x50, fb(rmax))
    node = newbuf(ctx, 0xa0)
    w32(ctx, node + 0x90, ctx.buf(4, bytes([r.choice(b'SXT'), 0, 0, 0])))
    unit = newbuf(ctx, 0x200)
    w32(ctx, unit, r.choice([1, 9, 2, 5]))
    w32(ctx, unit + 0xc, r.choice([self_, r.randrange(0, 8)]))
    w32(ctx, unit + 0x14, r.choice([2, 3, 1, 0]))
    w32(ctx, unit + 0x1e8, r.choice([0, 1]))
    w32(ctx, unit + 0x48, node)
    w32(ctx, unit + 0xf0, r.choice([0x201, 0x201, 0x1201, 0x301, 0, r.getrandbits(32)]))
    d = r.choice([r.uniform(0, rmax * 1.3 + 1), r.uniform(0, 4000)])
    cx, cy, cz = (r.uniform(-5000, 5000) for _ in range(3))
    w32(ctx, unit + 0xdc, fb(cx), fb(cy), fb(cz))
    w32(ctx, unit + 0x58, fl(r, cx + d * 0.6, 1), fl(r, cy + d * 0.0, 1), fl(r, cz + d * 0.8, 1))
    hd = r.uniform(-400, 400)
    w32(ctx, unit + 0x68, fl(r, hd, 0))
    w32(ctx, unit + 0xe8, fl(r, hd + 180, r.choice([1, 30, 400])))
    w32(ctx, unit + 0x74, fl(r, 0, r.choice([1, 30, 400])))
    w32(ctx, unit + 0xec, fl(r, 0, 400))
    b = newbuf(ctx, 0xc00)
    w32(ctx, b, unit)
    w32(ctx, b + 0x1c, fl(r, -r32(ctx, unit + 0xec) * 0 + 0, r.choice([1, 30, 400])))
    w16(ctx, b + 0x560, r.choice([0, 0x40, 0x80, 0xc0, 0x8040, r.getrandbits(16)]))
    w32(ctx, b + 0x4d8, r.choice([0, 0x16a, 5, 0x100, -3 & 0xffffffff]))
    w32(ctx, b + 0xbcf, r.getrandbits(32))
    slots = newbuf(ctx, 0x128 * 4)
    si = r.choice([0, 1, 2, 3, -1])
    w32(ctx, b + 0x4fc, si & 0xffffffff)
    for i in range(4):
        w32(ctx, slots + 0x128 * i + 4, kind)
    w32(ctx, b + 0x84, slots + 0x128)
    return [b]


SPECS = [dict(name='Sim_100d6833', orig=0x100d6833, c='Sim_100d6833', gen=g, void=True, trials=6000)]
