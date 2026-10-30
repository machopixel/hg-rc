# Round 10: re-proof of 0x1005ae58 (round 5, unsorted_1005/w5x_calls.c) WITH its return value: the original returns
# what 0x1005ae79 returns and callers that are still original store it; the old C was void. 0x1010d180 (upper-case
# in place) runs for real; 0x1005ae79 is stubbed (the tree and the text logged, its answer from rets()). Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1005/w5x_calls.c'])
helper(0x1005ae79, None, 2, sargs=(1,))


def g(ctx):
    r = begin(ctx, 0x1005ae79)
    s = bytes(r.choice(b'abcXYZ019_') for _ in range(r.randrange(0, 20))) + b'\0'
    rets(0x1005ae79, r.getrandbits(32), 0)
    return [newbuf(ctx, 0x20), ctx.buf(len(s), s)]


SPECS = [dict(name='Sim_1005ae58', orig=0x1005ae58, c='Sim_1005ae58', gen=g, trials=600)]
