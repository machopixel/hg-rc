# Round 10: re-proof of 0x100a6009 (round 5, unsorted_100a/w5x_calls.c) WITH its return value: the original returns
# what 0x100a5d38 returns and its callers use it; the old C was void. 0x100a5d38 is stubbed (four arguments logged,
# its answer from rets()). Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100a/w5x_calls.c'])
helper(0x100a5d38, None, 4)


def g(ctx):
    r = begin(ctx, 0x100a5d38)
    rets(0x100a5d38, r.getrandbits(32), 0)
    return [r.getrandbits(32), r.getrandbits(32)]


SPECS = [dict(name='Sim_100a6009', orig=0x100a6009, c='Sim_100a6009', gen=g, trials=600)]
