# Round 18: 0x10020055 (DECISIONS.md round 18). The function is dead (no caller, no pointer to it); its never-written
# slot index [ebp-4] gets a negative leftover before each call, the case the C reproduces (no store), with
# [0x1015d52c] set or not and a slot table at [0x1015d0a4].
# Negative check: W18_NEG=1 gives a non-negative index, where the original stores the volume and the C does not.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1002/w18_misc.c'])
NEG = bool(os.environ.get('W18_NEG'))


def g(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015d52c, r.choice([0, 1, 1, r.getrandbits(32)]))
    w32(ctx, 0x101321b8, r.getrandbits(32))
    tab = newbuf(ctx, 0x294 + 4 * 64)
    w32(ctx, 0x1015d0a4, tab)
    idx = r.randrange(0, 64) if NEG else r.choice([0xffffffff, 0x80000000, r.getrandbits(31) | 0x80000000])
    w32(ctx, frame(0) - 4, idx)
    if NEG:
        w32(ctx, 0x1015d52c, 1)
    return []


SPECS = [dict(name='Sim_10020055', orig=0x10020055, c='Sim_10020055', gen=g, void=True, trials=300 if NEG else 3000)]
