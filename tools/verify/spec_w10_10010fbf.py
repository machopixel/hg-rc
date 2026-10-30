# Round 10: 0x10010fbf (a float of obj's record times a factor by kind), returned in st(0): checked through
# the trampolines (st(0) stored at OUTF), exact (no tolerance).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1001/w10_misc.c'])


def g(ctx):
    r = begin(ctx)
    st0(ctx, 0x10010fbf, 'Sim_10010fbf', 1)
    rec = newbuf(ctx, 0x600)
    w32(ctx, rec + 0x53c, rfloat(r))
    obj = newbuf(ctx, 0x40)
    w32(ctx, obj, r.choice([3, 4, 5, 6, 7, 8, 9, 10, 0, 1, 2, 11, 0xffffffff, r.getrandbits(32)]))
    w32(ctx, obj + 0x28, rec)
    return [obj]


SPECS = [dict(name='Sim_10010fbf', orig=TRAMP_O, c='w10_tramp_c', gen=g, trials=1500)]
