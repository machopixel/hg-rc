# Round 33, round 4: the weapon mount (1.0 0x10038ad5) against 1.2's 0x1002cdc5, on spec_w2e.py's generator (the
# gear's selected slot +0x5fb also -1, as 1.2's gear set-up starts it). 1.2's change: with +0x5fb -1 the free slot
# found becomes the selected one (the record's +0x128 and the gear's +0x5fb); otherwise the record takes +0x5fb as
# in 1.0. Everything runs for real (the copies, the name copy 0x10028981 = 1.2 0x1001e531); the gear and the weapon
# record have the same field offsets in 1.2. The name-pointer table and the 0xd6-byte name records are copied to
# 1.2's addresses and back.
# Negative check: W2EW_NEG12=1 builds the C with W2EW_MUTATE (1.0's copy alone).
import os
exec(open('tools/verify/spec_w2e.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W2EW_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW2EW_MUTATE']

F10, F12 = 0x10038ad5, 0x1002cdc5
HELPERS = globals().get('HELPERS', {})     # spec_w2e.py stubs nothing
_ns = {}
exec(open('tools/verify/w10_common.py').read(), _ns)
SOURCES = _ns['closure'](['sim/unsorted_1003/w2e_misc.c'])     # only what the mount needs (spec_w2e.py's list is a whole wave)
_g10 = g_slotobj


def g12(ctx):
    obj, rec = _g10(ctx)
    if ctx.rng.random() < 0.5:
        w32(ctx, obj + 0x5fb, -1)
    return [obj, rec]


WIN = [(0x109e88ec, 0x107f22ac, 8 * 4), (0x10145592, 0x1011937a, 0xd6 * 32)]


def to12_more(m):
    for a10, a12, n in WIN:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, n)))


def from12_more(m):
    for a10, a12, n in WIN:
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), n)))


SPECS = [v12_spec(dict(name='Sim_10038ad5', orig=F10, c='Sim_10038ad5', gen=g12, trials=3000, void=True), F12,
                  to12_more=to12_more, from12_more=from12_more, extra_pairs={0x1017b8c8: 0x1014a568},
                  skip=(0x109e88ec, 0x10145592), name='Sim_10038ad5_v12')]
