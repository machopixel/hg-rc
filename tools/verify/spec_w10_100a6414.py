# Round 10: re-proof of 0x100a6414 (round 5, unsorted_100a/w5c_calls.c) WITH its return value: the original returns
# what 0x100a648c returns and its callers use it (0x100591ff multiplies it), but the old C was void and its old
# spec ignored eax. 0x100a648c is stubbed (its six arguments logged, its answer from rets()). Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100a/w5c_calls.c'])
helper(0x100a648c, None, 6)


def g(ctx):
    r = begin(ctx, 0x100a648c)
    a = newbuf(ctx, 0x40)
    table = newbuf(ctx, 0x58 * 256)
    off = r.randrange(0x18, 0x30)
    w32(ctx, a + 4, off)
    w8(ctx, a + off, r.randrange(0, 256))
    rets(0x100a648c, r.getrandbits(32), r.choice([0, 0x7f, 15]))
    return [a, table]


SPECS = [dict(name='Sim_100a6414', orig=0x100a6414, c='Sim_100a6414', gen=g, trials=1000)]
