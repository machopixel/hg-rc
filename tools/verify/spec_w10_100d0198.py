# Round 10: 0x100d0198 (HUD pane text). Stubbed on both sides: the pane's method +0x114 (IND1, text logged),
# the font loader 0x100ba7d5 (name and key logged as text), the three text routines (text logged by content),
# Sim_100bb2b0 and Sim_10051532 (converted, called by name). Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100d/w10_misc.c'])
helper(IND1, None, 2, sargs=(1,))
helper(0x100ba7d5, None, 3, sargs=(0, 1))
for _a in (0x100f2f01, 0x100f1e10, 0x100ecea6):
    helper(_a, None, 6, sargs=(4,))
helper(0x100bb2b0, 'Sim_100bb2b0', 2, sargs=(0, 1))
helper(0x10051532, 'Sim_10051532', 1)


def txt(ctx, n):
    r = ctx.rng
    return bytes(r.choice(b'ABCdef 0123') for _ in range(r.randrange(0, n))) + b'\0'


def g(ctx):
    r = begin(ctx, IND1, 0x100ba7d5, 0x100f2f01, 0x100f1e10, 0x100ecea6, 0x100bb2b0, 0x10051532)
    pane = newbuf(ctx, 0x120)
    w16(ctx, pane + 4, r.choice([0, 0, 1, 0xffff, r.getrandbits(16)]))
    wb(ctx, pane + 0x10, txt(ctx, 0x30))
    w32(ctx, pane + 0x114, IND1)
    w32(ctx, 0x109b9900, ctx.buf(16, txt(ctx, 12).ljust(16, b'\0')))
    w32(ctx, 0x1014cec4, ctx.buf(8, b'FNT\0\0\0\0\0'))
    owner = newbuf(ctx, 0x100)
    w32(ctx, owner + 0xf0, newbuf(ctx, 8))
    w32(ctx, 0x109e8934, owner)
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    rets(0x100ba7d5, r.choice([0, 0x12345678, 0x1000]))
    return [pane]


SPECS = [dict(name='Sim_100d0198', orig=0x100d0198, c='Sim_100d0198', gen=g, void=True, trials=1000)]
