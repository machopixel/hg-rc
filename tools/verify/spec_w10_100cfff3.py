# Round 10: 0x100cfff3 (HUD pane text with the own unit's state). Stubbed on both sides: Sim_100510d9, the font
# loader 0x100ba7d5, the pane's method +0x114 (IND1, text pointer argument logged as text), the three text
# routines (text logged), Sim_100bb2b0. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100c/w10_misc.c'])
helper(0x100510d9, 'Sim_100510d9', 1)
helper(IND1, None, 2, sargs=(1,))
helper(0x100ba7d5, None, 3, sargs=(0, 1))
for _a in (0x100f2f01, 0x100f1e10, 0x100ecea6):
    helper(_a, None, 6, sargs=(4,))
helper(0x100bb2b0, 'Sim_100bb2b0', 2, sargs=(0, 1))


def txt(ctx, n):
    r = ctx.rng
    return bytes(r.choice(b'ABCdef 0123') for _ in range(r.randrange(0, n))) + b'\0'


def g(ctx):
    r = begin(ctx, 0x100510d9, IND1, 0x100ba7d5, 0x100f2f01, 0x100f1e10, 0x100ecea6, 0x100bb2b0)
    unit = newbuf(ctx, 0x40)
    rec = newbuf(ctx, 0x570)
    w16(ctx, rec + 0x560, r.getrandbits(16))
    w32(ctx, unit + 0x28, rec)
    idx = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, idx)
    w32(ctx, 0x109e8b40 + 4 * idx, unit)
    pane = newbuf(ctx, 0x120)
    w16(ctx, pane + 4, r.choice([0, 1, 0xffff, r.getrandbits(16)]))
    wb(ctx, pane + 0x10, txt(ctx, 0x30))
    w32(ctx, pane + 0x114, IND1)
    w32(ctx, pane + 0xf0, newbuf(ctx, 8))
    for a in (0x101c4c50, 0x101c4c58, 0x101c4c60):
        w32(ctx, a, ctx.buf(24, txt(ctx, 20).ljust(24, b'\0')))
    w32(ctx, 0x109b9900, ctx.buf(16, txt(ctx, 12).ljust(16, b'\0')))
    w32(ctx, 0x1014cec4, ctx.buf(8, b'FNT\0\0\0\0\0'))
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    rets(0x100ba7d5, r.choice([0, 0x12345678]))
    return [pane]


SPECS = [dict(name='Sim_100cfff3', orig=0x100cfff3, c='Sim_100cfff3', gen=g, void=True, trials=1000)]
