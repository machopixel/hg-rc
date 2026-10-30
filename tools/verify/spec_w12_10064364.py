# Round 12: 0x10064364 (the HUD compass). Stubs on both sides: the loader 0x100ba7d5, Sim_1006419e (rotated blit),
# the HUD library 0x100f0205 / 0x100e91dc / 0x100f2f01 / 0x100f1e10 / 0x100ecea6 (the text logged), Sim_10052be7 and
# Sim_100bb2b0. sprintf runs for real. Without the dial the original prints its unset heading [ebp-0xc], which gets
# the chosen start value 0.0 as its stack leftover.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w12_compass.c'])
helper(0x100ba7d5, None, 3)
helper(0x1006419e, 'Sim_1006419e', 10)
helper(0x100f0205, None, 5)
helper(0x100e91dc, None, 5)
helper(0x100f2f01, None, 6, sargs=(4,))
helper(0x100f1e10, None, 6, sargs=(4,))
helper(0x100ecea6, None, 6, sargs=(4,))
helper(0x10052be7, 'Sim_10052be7', 1)
helper(0x100bb2b0, 'Sim_100bb2b0', 2)
ACTIVE = (0x100ba7d5, 0x1006419e, 0x100f0205, 0x100e91dc, 0x100f2f01, 0x100f1e10, 0x100ecea6, 0x10052be7, 0x100bb2b0)
EBP = frame(2)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    hud = newbuf(ctx, 0x100)
    at = newbuf(ctx, 8)
    w32(ctx, at, r.getrandbits(32), r.getrandbits(32))
    w32(ctx, hud + 0xf0, at)
    w32(ctx, hud + 0xec, r.getrandbits(32))
    w32(ctx, hud + 0xac, r.getrandbits(32), r.getrandbits(32))
    w32(ctx, 0x109e8940, hud)
    for a in (0x109e8480, 0x109e8484, 0x109e8488, 0x109e848c, 0x109e85c8, 0x109b9900, 0x1014cec0, 0x1014cec4,
              0x1015dc2c):
        w32(ctx, a, r.getrandbits(32))
    rets(0x100ba7d5, *[r.choice([0, 0x00c00000, 0x00c00000]) for _ in range(5)])
    rets(0x10052be7, r.choice([0, 1, 2]))
    self_ = r.randrange(8)
    w32(ctx, 0x1017b8c8, self_)
    for i in range(8):
        u = newbuf(ctx, 0x120)
        rec = newbuf(ctx, 0x500)
        w32(ctx, rec + 0x4f0, r.choice([2, 2, 1]))
        w32(ctx, u + 0x28, r.choice([rec, rec, 0]))
        w32(ctx, u + 0x118, r.choice([0xffffffff, r.randrange(0, 8), r.getrandbits(32)]))
        w32(ctx, u + 0xf0, r.choice([0xffffffff, 0x200 | r.randrange(0, 8), 0x400 | r.randrange(8), r.getrandbits(32)]))
        w32(ctx, 0x109e8b40 + 4 * i, r.choice([u, u, u, 0]) if i != self_ else u)
    for i in range(256):
        if r.random() < 0.05 or i < 8:
            w32(ctx, 0x109a8640 + 0x54 * i, r.choice([0, 1]))
    w32(ctx, 0x1015e4d4, fb(r.uniform(-400, 400)))
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    w32(ctx, EBP - 0xc, 0)                   # the chosen start value (0.0) as the original's leftover
    ang = lambda: r.choice([fb(r.uniform(-720, 720)), fb(0.0), fb(359.9), fb(360.0), fb(-1e12), fb(r.uniform(-90, 90))])
    return [ang(), ang()]


SPECS = [dict(name='Sim_10064364', orig=0x10064364, c='Sim_10064364', gen=g, void=True, trials=4000)]