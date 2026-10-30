# Round 12: 0x100b66a0 (HUD damage display). Stubs on both sides: the loader 0x100ba7d5, the HUD 2D library calls
# 0x100f0205 / 0x100e91dc / 0x100e96c6 / 0x100f01ad / 0x100e96a7 and Sim_100bb2b0. The original's divisor [ebp-0x10],
# tested before it is written on the first pass, gets the chosen start value 1 as its stack leftover.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_100b/w12_hud.c'])
helper(0x100ba7d5, None, 3)
helper(0x100f0205, None, 5)
helper(0x100e91dc, None, 5)
helper(0x100e96c6, None, 5)
helper(0x100f01ad, None, 1)
helper(0x100e96a7, None, 1)
helper(0x100bb2b0, 'Sim_100bb2b0', 2)
ACTIVE = (0x100ba7d5, 0x100f0205, 0x100e91dc, 0x100e96c6, 0x100f01ad, 0x100e96a7, 0x100bb2b0)
EBP = frame(0)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    self_ = r.randrange(4)
    w32(ctx, 0x1017b8c8, self_)
    unit = newbuf(ctx, 0x40)
    rec = newbuf(ctx, 0x100)
    w32(ctx, unit + 0x28, rec)
    w32(ctx, 0x109e8b40 + 4 * self_, unit)
    segs = newbuf(ctx, 0x3c * 12)
    w32(ctx, rec + 0x88, segs)
    for k in range(12):
        s = segs + 0x3c * k
        w32(ctx, s, r.choice([fb(r.uniform(-100, 400)), 0, fb(50.0)]))
        w32(ctx, s + 8, r.choice([fb(r.uniform(-100, 400)), 0, fb(r.uniform(0, 1e9))]))
        w32(ctx, s + 0x38, r.choice([0, 1, 10, 100, 1000, r.randrange(-100, 5000)]) & 0xffffffff)
    for i in range(7):
        w32(ctx, 0x101508a0 + 8 * i, r.randrange(1, 13), r.randrange(0, 6))
    for k in range(6):
        w32(ctx, 0x109e8480 + 4 * k, r.getrandbits(32))
    w32(ctx, 0x109e8498, r.getrandbits(32))
    w32(ctx, 0x1014cec0, r.getrandbits(32))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    w32(ctx, 0x1017692c, r.choice([0, 1, 1]))
    hud = newbuf(ctx, 0x100)
    at = newbuf(ctx, 8)
    w32(ctx, at, r.getrandbits(32), r.getrandbits(32))
    w32(ctx, hud + 0xf0, at)
    w32(ctx, 0x109e88e4, hud)
    w32(ctx, 0x1012bae8, *struct.unpack('<II', struct.pack('<d', r.choice([0.0, 100.0, 1e12]))))
    rets(0x100ba7d5, *[r.choice([0, 0x00c00000, 0x00c00000]) for _ in range(5)])
    w32(ctx, EBP - 0x10, 1)                  # the chosen start value as the original's leftover
    return []


SPECS = [dict(name='Sim_100b66a0', orig=0x100b66a0, c='Sim_100b66a0', gen=g, void=True, trials=4000)]