# Round 23: 0x1007760b, a choice item of a menu (DECISIONS.md round 23). The item's five callbacks point at logging
# stubs (int3 padding); the three text routines are logging stubs (the text logged as text). No local is read before
# it is written.
# Negative check (mutation): W23_NEG=1 builds the C with W23M_MUTATE (0xc9 steps forward).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1007/w23_menu.c'])
NEG = bool(os.environ.get('W23_NEG'))
if NEG:
    CFLAGS = CFLAGS + ['-DW23M_MUTATE']
PAD = 0x1010d3e6
CB = {0x10: 2, 0x14: 1, 0x18: 2, 0x1c: 2, 0x20: 1}
for k, (off, n) in enumerate(CB.items()):
    helper(PAD + k, None, n)
for a in (0x100f2f01, 0x100f1e10, 0x100ecea6):
    helper(a, None, 6, sargs=(4,))
ACTIVE = tuple(PAD + k for k in range(len(CB))) + (0x100f2f01, 0x100f1e10, 0x100ecea6)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    panel = newbuf(ctx, 0x50)
    w32(ctx, panel, r.choice([0, 0x777, 0x777, 0x777]))
    w32(ctx, panel + 0x48, r.choice([0, 0x88, 0x88]))
    n = r.choice([0, 1, 3, 5])
    choices = newbuf(ctx, 8 + 4 * 6)
    w32(ctx, choices + 4, n)
    for i in range(6):
        w32(ctx, choices + 8 + 4 * i, ctx.buf(8, 'text'))
    item = newbuf(ctx, 0x24)
    w32(ctx, item, r.getrandbits(32))
    w32(ctx, item + 4, r.randrange(0, 5))
    w32(ctx, item + 8, r.choice([choices, choices, choices, 0]))
    for k, off in enumerate(CB):
        w32(ctx, item + off, r.choice([PAD + k, PAD + k, 0]))
    rets(PAD + 1, *[r.randrange(0, 5) for _ in range(3)])
    rows = r.randrange(0, 4)
    menu = newbuf(ctx, 0x18 + 4 * 0x14)
    w32(ctx, menu, r.choice([1, 2, 2, 3, 4, 5, 0, 7]))
    w32(ctx, menu + 0x10, r.choice([rows, rows, 0, 3]))
    for i in range(4):
        w32(ctx, menu + 0x18 + i * 0x14, r.choice([4, 5, 1, 0]))
    w32(ctx, 0x1015dc18, r.choice([0x20, 0xc8, 0xc9, 0x31 + rows, 0x31, 0x40, 0]))
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    return [r.choice([panel, panel, panel, 0]), r.choice([item] * 9 + [0]), rows, r.randrange(0, 600), r.randrange(0, 400),
            r.choice([menu, menu, menu, 0])]


SPECS = [dict(name='Sim_1007760b', orig=0x1007760b, c='Sim_1007760b', gen=g, void=True, trials=300 if NEG else 6000)]
