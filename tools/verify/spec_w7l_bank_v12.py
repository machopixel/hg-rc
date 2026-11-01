# Sweep (sound): queue a sound of a bank (1.0 0x100200d1, Sim_100200d1) against 1.2's 0x100173e1. 1.2's change: bank
# numbers 0..4 (1.0: 0..2 and 4); bank 3's index (1.0 0x10133710, 1.2 0x1010a188) is never written in either
# version, -1, so bank 3 queues nothing. The sound-name pointer table (0x101336b0, 16 entries = 1.2 0x1010a128) and
# the bank index / toggle pairs (0x101336f8.. = 1.2 0x1010a170..) are copied across and back; the play call 0x1001f790
# (1.2 0x10016aa0) is a logging stub (its two records compared as pointers: the same values in both runs).
# Negative check: W7LB_NEG12=1 builds the C with W7LB_MUTATE (1.0's banks), with bank 3's index set to 0 so a queued
# bank-3 sound shows.
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1002/w7l_misc.c'])
_neg = os.environ.get('W7LB_NEG12')
if _neg:
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW7LB_MUTATE']
helper(0x1001f790, None, 3)
WIN = [(0x101336b0, 0x1010a128, 16 * 4), (0x101336f8, 0x1010a170, 6 * 8)]


def g(ctx):
    r = begin(ctx, 0x1001f790)
    names = [newbuf(ctx, 10 * 0x18) for _ in range(16)]
    w32(ctx, 0x101336b0, *names)
    for a in range(6):
        w32(ctx, 0x101336f8 + 8 * a, r.choice([-1, 0, 2, 5, 15]), r.choice([0, 1]))
    if _neg:
        w32(ctx, 0x10133710, 0)
    return [r.choice([-2, -1, 0, 1, 2, 3, 3, 4, 5]), r.choice([-1, 0, 0, 1, 5, 9, 10])]


def to12_more(m):
    for a10, a12, n in WIN:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, n)))


def from12_more(m):
    for a10, a12, n in WIN:
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), n)))


SPECS = [v12_spec(dict(name='Sim_100200d1', orig=0x100200d1, c='Sim_100200d1', gen=g, trials=3000, void=True),
                  0x100173e1, to12_more=to12_more, from12_more=from12_more,
                  extra_pairs={0x1001f790: 0x10016aa0}, skip=(0x101336b0, 0x101336f8, 0x10133700, 0x10133704,
                                                              0x10133708, 0x1013370c, 0x10133710, 0x10133714,
                                                              0x10133718, 0x1013371c, 0x101336fc),
                  name='Sim_100200d1_v12')]
