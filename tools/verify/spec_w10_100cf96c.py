# Round 10: 0x100cf96c (HUD speed readout). Stubbed on both sides: Sim_100516c8, the font loader 0x100ba7d5, the
# font height 0x100ecce0, the three text routines (text logged by content: the formatted speed comes from the
# real sprintf, the original's on one side and Crt_SPrintf on the other) and Sim_100bb2b0. Sim_10084a94 runs for
# real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100c/w10_misc.c'])
helper(0x100516c8, 'Sim_100516c8', 1)
helper(0x100ba7d5, None, 3, sargs=(0, 1))
helper(0x100ecce0, None, 1)
for _a in (0x100f2f01, 0x100f1e10, 0x100ecea6):
    helper(_a, None, 6, sargs=(4,))
helper(0x100bb2b0, 'Sim_100bb2b0', 2, sargs=(0, 1))


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def txt(ctx, n):
    r = ctx.rng
    return bytes(r.choice(b'ABCdef 0123') for _ in range(r.randrange(0, n))) + b'\0'


def g(ctx):
    r = begin(ctx, 0x100516c8, 0x100ba7d5, 0x100ecce0, 0x100f2f01, 0x100f1e10, 0x100ecea6, 0x100bb2b0)
    unit = newbuf(ctx, 0x40)
    rec = newbuf(ctx, 0x620)
    for off in (0x548, 0x54c, 0x550):
        w32(ctx, rec + off, fb(r.uniform(-60, 60)) if r.random() < 0.95 else rfloat(r))
    w32(ctx, rec + 0x2c, fb(r.choice([r.uniform(-5, 5), 0.0, -0.0])))
    w16(ctx, rec + 0x562, r.getrandbits(16))
    w32(ctx, rec + 0x613, r.choice([0, 1]))
    w32(ctx, unit + 0x28, rec)
    idx = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, idx)
    w32(ctx, 0x109e8b40 + 4 * idx, unit)
    pane = newbuf(ctx, 0x120)
    w16(ctx, pane + 4, r.choice([0, 1, 1]))
    owner = newbuf(ctx, 0x100)
    w32(ctx, owner + 0xf0, newbuf(ctx, 8))
    w32(ctx, 0x109e8928, owner)
    for a in (0x101c4c68, 0x101c4c6c):
        w32(ctx, a, ctx.buf(24, txt(ctx, 20).ljust(24, b'\0')))
    w32(ctx, 0x109b9900, ctx.buf(16, b'FONT'.ljust(16, b'\0')))
    w32(ctx, 0x1014cec4, ctx.buf(8, b'FNT\0\0\0\0\0'))
    w32(ctx, 0x109e3e1c, r.randrange(0, 1200))
    w32(ctx, 0x109e3de8, r.randrange(0, 1600))
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    rets(0x100ba7d5, r.choice([0, 0x12345678, 0x12345678]))
    rets(0x100ecce0, r.randrange(0, 30))
    return [pane]


SPECS = [dict(name='Sim_100cf96c', orig=0x100cf96c, c='Sim_100cf96c', gen=g, void=True, trials=2000)]
