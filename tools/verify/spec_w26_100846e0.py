# Round 26: 0x100846e0, the sine / arctangent tables (DECISIONS.md round 26). The C copies the tables captured from
# the original on the game machine (tools/round26/trigcap.c). Here the original runs in the checker's emulator, whose
# x87 fsin / fpatan are Unicorn's: a cross-check that the emulated original agrees with the captured values.
# Negative check (mutation): W26_NEG=1 builds the C with W26T_MUTATE (one bit of one sine entry flipped).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1008/w5a_misc.c'])
NEG = bool(os.environ.get('W26_NEG'))
if NEG:
    CFLAGS = CFLAGS + ['-DW26T_MUTATE']


def g(ctx):
    begin(ctx)
    for a in (0x103719a0, 0x103721c0):
        for i in range(0x201):
            w32(ctx, a + 4 * i, ctx.rng.getrandbits(32))
    return []


SPECS = [dict(name='Trig_InitTables', orig=0x100846e0, c='Trig_InitTables', gen=g, trials=20)]
