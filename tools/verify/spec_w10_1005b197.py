# Round 10: 0x1005b197 (frees rec+0x131 when set, then rec). HeapFree is stubbed by the checker and logged.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1005/w10_misc.c'])


def g(ctx):
    r = begin(ctx)
    rec = newbuf(ctx, 0x140)
    w32(ctx, rec + 0x131, r.choice([0, 0, r.getrandbits(32), 0x12345678]))
    return [rec]


SPECS = [dict(name='Sim_1005b197', orig=0x1005b197, c='Sim_1005b197', gen=g, void=True, trials=300,
              api={'HeapFree': lambda n, a: [0, 1][n % 2]})]
