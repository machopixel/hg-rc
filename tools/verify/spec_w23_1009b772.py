# Round 23: 0x1009b772, the scene pass's polygon routine (DECISIONS.md round 23). Sim_1009ef50 and 0x10098f8c are
# logging stubs. No local is read before it is written (its pointer to [ebp-0x180] is never used).
# Negative check (mutation): W23_NEG=1 builds the C with W23S_MUTATE (0xb000 skipped like 0x9000), which the call log
# must catch.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w23_scene.c'])
NEG = bool(os.environ.get('W23_NEG'))
if NEG:
    CFLAGS = CFLAGS + ['-DW23S_MUTATE']
helper(0x1009ef50, 'Sim_1009ef50', 3)
helper(0x10098f8c, None, 5)
ACTIVE = (0x1009ef50, 0x10098f8c)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, 0x1015d1ac, r.randrange(0, 6))
    for i in range(6):
        w32(ctx, 0x1014c4c8 + 4 * i, r.choice([0, 1, r.getrandbits(32)]))
    flags = r.randrange(0, 16) << 12 | r.getrandbits(12)
    return [r.choice([0, 3, 4, 7]), newbuf(ctx, 0x24 * 8), flags]


SPECS = [dict(name='Sim_1009b772', orig=0x1009b772, c='Sim_1009b772', gen=g, void=True, trials=300 if NEG else 4000)]
