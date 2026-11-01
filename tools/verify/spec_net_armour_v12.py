# Network round: the armour set-up of a unit's gear (1.0 0x1005b98c, Sim_1005b98c) against 1.2's 0x1004e4cc. 1.2's
# change: in a network game every gear's 17 part records get x2 (1.0: x4 for the own gear and the kind-1 gears,
# others untouched). Single player (own gear: x12 / x6 / x1 by the difficulty byte) is unchanged. The unit table
# 0x109e8b40 (1.2 0x107f24c0) is copied across; the unit's gear pointer is +0x28 in our unit record and +0x24 in
# 1.2's (both set). The difficulty byte stays 0..2 (the factor is unset otherwise, as in both originals).
# Negative check: NETA_NEG12=1 builds the C with NETA_MUTATE (1.0's network rule).
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1005/w3r_misc.c'])
CFLAGS = list(globals().get('CFLAGS', [])) + ['-msse2', '-mfpmath=sse']
if os.environ.get('NETA_NEG12'):
    CFLAGS = CFLAGS + ['-DNETA_MUTATE']


def g(ctx):
    r = begin(ctx)
    n = 4
    index, player = r.randrange(n), r.randrange(n)
    pid = r.getrandbits(32)
    for i in range(n):
        recs = newbuf(ctx, 0x11 * 0x3c + 16)
        for k in range(0x11):
            o = recs + k * 0x3c
            wf(ctx, o, r.choice([r.uniform(0, 3000), 100.0, 0.0, r.uniform(0, 30)]))
            wf(ctx, o + 8, r.choice([r.uniform(0, 3000), 1.0, 0.0, r.uniform(0, 20000)]))
            w32(ctx, o + 0x38, r.getrandbits(32))
        sub = newbuf(ctx, 0x40)
        w32(ctx, sub + 0x14, r.choice([0, 1, 1, 2, 3]))
        data = newbuf(ctx, 0x100)
        w32(ctx, data, sub)
        w32(ctx, data + 0x88, recs)
        unit = newbuf(ctx, 0x40)
        w32(ctx, unit + 8, pid if (i in (index, player) and r.random() < 0.8) else r.getrandbits(32))
        w32(ctx, unit + 0x24, data)
        w32(ctx, unit + 0x28, data)
        w32(ctx, 0x109e8b40 + 4 * i, unit)
    w32(ctx, 0x1017b8c8, player)
    cfg = newbuf(ctx, 16)
    w8(ctx, cfg + 5, r.choice([0, 1, 2]))
    w32(ctx, 0x1017b8c4, cfg)
    w32(ctx, 0x1015ec58, r.choice([0, 1, 1]))
    return [index]


def to12_more(m):
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(0x109e8b40, 0x10)))


SPECS = [v12_spec(dict(name='Sim_1005b98c', orig=0x1005b98c, c='Sim_1005b98c', gen=g, trials=3000, void=True),
                  0x1004e4cc, to12_more=to12_more,
                  extra_pairs={0x1017b8c8: 0x1014a568, 0x1017b8c4: 0x1014a564, 0x1015ec58: 0x101314a0},
                  skip=(0x109e8b40,), name='Sim_1005b98c_v12')]
