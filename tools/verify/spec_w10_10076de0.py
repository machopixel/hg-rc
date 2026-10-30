# Round 10: 0x10076de0 (menu slider item). The five callbacks are stubs (IND1 +0x14, IND2 +0x10, IND3 +0x1c,
# IND4 +0x20, IND5 +0x18). Stubbed on both sides: the shape loader 0x100ba7d5 (name logged as text, a shape or 0),
# the shape size 0x100ed743, the font height 0x100ecce0, the two blitters 0x100f0205 / 0x100e91dc and
# Sim_100bb2b0 (name logged as text). Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1007/w10_misc.c'])
helper(IND1, None, 1)
helper(IND2, None, 2)
helper(IND3, None, 2)
helper(IND4, None, 1)
helper(IND5, None, 2)
helper(0x100ba7d5, None, 3, sargs=(0,))
helper(0x100ed743, None, 2)
helper(0x100ecce0, None, 1)
helper(0x100f0205, None, 5)
helper(0x100e91dc, None, 5)
helper(0x100bb2b0, 'Sim_100bb2b0', 2, sargs=(0,))


def g(ctx):
    r = begin(ctx, IND1, IND2, IND3, IND4, IND5, 0x100ba7d5, 0x100ed743, 0x100ecce0, 0x100f0205, 0x100e91dc,
              0x100bb2b0)
    menu = newbuf(ctx, 0x50)
    w32(ctx, menu, r.choice([0x5000, 0x5000, 0]))
    w32(ctx, menu + 0x48, r.choice([0x6000, 0x6000, 0]))
    item = newbuf(ctx, 0x24)
    w32(ctx, item, r.getrandbits(32))
    w32(ctx, item + 4, r.choice([0, 0x8000, 0x10000, 0xf000, 0x10001, -5, r.getrandbits(32)]) & 0xffffffff)
    w32(ctx, item + 0xc, r.getrandbits(32))
    for off, a in ((0x10, IND2), (0x14, IND1), (0x18, IND5), (0x1c, IND3), (0x20, IND4)):
        w32(ctx, item + off, a if r.random() < 0.7 else 0)
    names = newbuf(ctx, 0x20)
    for k in range(4):
        w32(ctx, names + 8 * k, ctx.buf(12, bytes(r.choice(b'SLIDBAR') for _ in range(9)) + b'\0\0\0'))
    w32(ctx, item + 8, names if r.random() < 0.97 else 0)
    state = newbuf(ctx, 0x18 + 0x14 * 8)
    w32(ctx, state, r.choice([1, 2, 2, 2, 3, 4, 5, 0, 6]))
    row = r.randrange(0, 8)
    w32(ctx, state + 0x10, r.choice([row, row, row + 1]))
    w32(ctx, state + 0x18 + row * 0x14, r.choice([5, 4, 4, 0]))
    w32(ctx, 0x1015dc18, r.choice([row + 0x31, row + 0x30, 0x20, 0xc8, 0xc9, 0xc9]))
    rets(IND1, r.choice([0, 0x8000, 0x10000, 0x1234, -3 & 0xffffffff, r.getrandbits(32)]), 1)
    rets(0x100ba7d5, r.choice([0x7000, 0x7000, 0x7000, 0]), r.choice([0x7100, 0x7100, 0]))
    rets(0x100ed743, r.choice([r.getrandbits(32), r.randrange(0, 300) << 16, 0xffff0000]),
         r.choice([r.getrandbits(32), r.randrange(0, 50) << 16, 0xfffe1234]))
    rets(0x100ecce0, r.choice([r.randrange(0, 20), r.getrandbits(32)]))
    w32(ctx, 0x1014cb38, r.choice([0, 1]))
    w32(ctx, 0x101c64f0, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    w32(ctx, 0x1014cec0, r.getrandbits(32))
    if r.random() < 0.03:
        menu = 0
    if r.random() < 0.03:
        item = 0
    if r.random() < 0.03:
        state = 0
    x = r.choice([r.randrange(-700, 700), r.getrandbits(32)]) & 0xffffffff
    return [menu, item, row, x, r.randrange(0, 480), state]


SPECS = [dict(name='Sim_10076de0', orig=0x10076de0, c='Sim_10076de0', gen=g, void=True, trials=4000)]
