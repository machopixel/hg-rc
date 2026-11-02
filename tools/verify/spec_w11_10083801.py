# Round 11: 0x10083801 (frame buffer to the window with the screen-noise effects). Sim_1009c929 is a stub on both
# sides; the CRT memcpy runs for real on a frame buffer of random bytes (compared afterwards); SetDIBitsToDevice is
# the checker's Windows stub (arguments logged).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1008/w11_ddraw.c'])
helper(0x1009c929, 'Sim_1009c929', 1)
ACTIVE = tuple(HELPERS)


def fx(ctx, r):
    if r.random() < 0.2:
        return 0
    b = newbuf(ctx, 0x20)
    w32(ctx, b, r.randrange(0, 120), r.randrange(0, 20), r.choice([1, 5, r.randrange(1, 300)]),
        r.choice([1, 3, r.randrange(1, 30)]), r.choice([0, 50, 100, r.randrange(101)]), r.choice([0, 1, 7, 40]),
        r.choice([0, 50, 100]), r.choice([0, 50, 100]))
    return b


def g(ctx):
    r = begin(ctx, *ACTIVE)
    rets(0x1009c929, *[r.randrange(0, 100) for _ in range(7)])
    w32(ctx, 0x10179ca0, fx(ctx, r))
    w32(ctx, 0x10179ca4, fx(ctx, r))
    w32(ctx, 0x1017b7e0, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b7dc, r.choice([0, 16, r.randrange(0, 40)]))
    w32(ctx, 0x10144fa0, r.choice([0, 1]))
    w32(ctx, 0x1015e9a0, newbuf(ctx, 0x9000))
    w32(ctx, 0x109e3d14, r.choice([640, 320]))
    w32(ctx, 0x109e3d18, r.choice([480, 200]))
    w32(ctx, 0x1017b3c0, r.getrandbits(32))
    return []


# superseded: the loading screen draws through Vulkan (DECISIONS.md, "The loading screen through Vulkan"; checked natively against GDI by tools/verify/picture_native.c)
# SPECS = [dict(name='Sim_10083801', orig=0x10083801, c='Sim_10083801', gen=g, void=True, trials=3000,
#               api={'SetDIBitsToDevice': lambda n, a: 480 + n})]
SPECS = []
