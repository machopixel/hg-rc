# Round 23: 0x1007792c, a text item of a menu (DECISIONS.md round 23). 0x1008cafb, Sim_1008dba2 (its text logged as
# text) and the item's changed callback (int3 padding) are logging stubs. No local is read before it is written.
# Negative check (mutation): W23_NEG=1 builds the C with W23T_MUTATE (state 5 calls the callback too).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1007/w23_menu.c'])
NEG = bool(os.environ.get('W23_NEG'))
if NEG:
    CFLAGS = CFLAGS + ['-DW23T_MUTATE']
PAD = 0x1010d3e6
helper(PAD, None, 2)
helper(0x1008cafb, None, 3)
helper(0x1008dba2, None, 3, sargs=(1,))
ACTIVE = (PAD, 0x1008cafb, 0x1008dba2)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    panel = newbuf(ctx, 0x50)
    w32(ctx, panel, r.choice([0, 0x777, 0x777, 0x777]))
    w32(ctx, panel + 0x48, r.choice([0, 0x88, 0x88]))
    pane = newbuf(ctx, 0x20)
    w32(ctx, pane, r.choice([0, 0, 0x1234]))
    rec = newbuf(ctx, 8)
    w32(ctx, rec, r.choice([pane] * 5 + [0]))
    w32(ctx, rec + 4, r.choice([ctx.buf(30, 'text')] * 5 + [0]))
    item = newbuf(ctx, 0x24)
    w32(ctx, item + 8, r.choice([rec] * 5 + [0]))
    w32(ctx, item + 0x18, r.choice([PAD, PAD, 0]))
    menu = newbuf(ctx, 0x20)
    w32(ctx, menu, r.choice([1, 2, 3, 4, 5, 0]))
    return [r.choice([panel] * 5 + [0]), r.choice([item] * 5 + [0]), r.randrange(0, 4), 10, 20,
            r.choice([menu] * 5 + [0])]


SPECS = [dict(name='Sim_1007792c', orig=0x1007792c, c='Sim_1007792c', gen=g, void=True, trials=300 if NEG else 4000)]
