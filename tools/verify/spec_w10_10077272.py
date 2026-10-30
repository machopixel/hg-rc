# Round 10: 0x10077272 (cycling menu item). The six callbacks are stubs (IND1..IND6: +0x14, +0x10, +0x1c, +0x20,
# +0x18 and the list's text callback, which answers a text of the generator or 0); the three text routines are
# stubbed with the drawn text logged by content. Str_CopyN / Str_NCat / Str_Length run for real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1007/w10_misc.c'])
helper(IND1, None, 1)            # +0x14 (current index)
helper(IND2, None, 2)            # +0x10
helper(IND3, None, 2)            # +0x1c
helper(IND4, None, 1)            # +0x20
helper(IND5, None, 2)            # +0x18
helper(IND6, None, 6)            # list text callback
for _a in (0x100f2f01, 0x100f1e10, 0x100ecea6):
    helper(_a, None, 6, sargs=(4,))


def txt(ctx, n):
    r = ctx.rng
    return bytes(r.choice(b'ABCdef 0123') for _ in range(r.randrange(0, n))) + b'\0'


def g(ctx):
    r = begin(ctx, IND1, IND2, IND3, IND4, IND5, IND6, 0x100f2f01, 0x100f1e10, 0x100ecea6)
    menu = newbuf(ctx, 0x50)
    w32(ctx, menu, r.choice([0x5000, 0x5000, 0]))
    w32(ctx, menu + 0x48, r.choice([0x6000, 0x6000, 0]))
    item = newbuf(ctx, 0x24)
    w32(ctx, item, r.getrandbits(32))
    w32(ctx, item + 4, r.choice([0, 1, 2, 5, -1, r.getrandbits(32)]) & 0xffffffff)
    w32(ctx, item + 0xc, r.getrandbits(32))
    for off, a in ((0x10, IND2), (0x14, IND1), (0x18, IND5), (0x1c, IND3), (0x20, IND4)):
        w32(ctx, item + off, a if r.random() < 0.7 else 0)
    count = r.choice([0, 1, 2, 3, 5, -1])
    lst = newbuf(ctx, 8 + 4 * 8)
    w32(ctx, lst, IND6 if r.random() < 0.6 else 0)
    w32(ctx, lst + 4, count & 0xffffffff)
    for k in range(8):
        w32(ctx, lst + 8 + 4 * k, ctx.buf(140, txt(ctx, r.choice([10, 130])).ljust(140, b'\0')))
    w32(ctx, item + 8, lst if r.random() < 0.95 else 0)
    state = newbuf(ctx, 0x18 + 0x14 * 8)
    w32(ctx, state, r.choice([1, 2, 3, 4, 5, 0, 6]))
    row = r.randrange(0, 8)
    w32(ctx, state + 0x10, r.choice([row, row, row + 1]))
    w32(ctx, state + 0x18 + row * 0x14, r.choice([5, 5, 4]))
    w32(ctx, 0x1015dc18, r.choice([row + 0x31, row + 0x30]))
    rets(IND1, r.choice([0, 1, 2, 7, -3 & 0xffffffff, r.getrandbits(32)]), 1)
    rets(IND6, ctx.buf(140, txt(ctx, r.choice([10, 130])).ljust(140, b'\0')) if r.random() < 0.7 else 0)
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    if r.random() < 0.03:
        menu = 0
    if r.random() < 0.03:
        item = 0
    if r.random() < 0.03:
        state = 0
    return [menu, item, row, r.randrange(0, 640), r.randrange(0, 480), state]


SPECS = [dict(name='Sim_10077272', orig=0x10077272, c='Sim_10077272', gen=g, void=True, trials=4000)]
