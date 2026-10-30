# Round 22: 0x100a539e, a queued polygon to the display (DECISIONS.md round 22). The plugin vertex array 0x101c9ec0
# is compared as DLL memory; the driver's polygon slot [0x1014cb4c] points at a logging stub (int3 padding), and the
# graphics library functions 0x100e84f0 / 0x100ef727 / 0x100e86a1 and 0x1009fada are logging stubs. No local is read
# before it is written.
# Negative check (mutation): W22_NEG=1 builds the C with W22P_MUTATE, which puts the +0x2b byte 16 bits up instead of
# 24; the copied vertices must differ.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w22_poly.c'])
NEG = bool(os.environ.get('W22_NEG'))
DRIVER = 0x1010d3e6
helper(DRIVER, None, 3)
helper(0x100e84f0, None, 4)
helper(0x100ef727, None, 7)
helper(0x100e86a1, None, 7)
helper(0x1009fada, None, 3)
ACTIVE = (DRIVER, 0x100e84f0, 0x100ef727, 0x100e86a1, 0x1009fada)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, 0x1014cb4c, DRIVER)
    n = r.choice([0, 1, 1, 2, 2, 3, 4, 6, 8])
    rec = newbuf(ctx, 0x10 + 4 * 8)
    w16(ctx, rec, n)
    w16(ctx, rec + 2, r.choice([0x1000, 0xf0, 0x10, 0x2005, 0x3010, 0x5000, 0x9000, 0xa010, 0x4011, 0xe0ff,
                               r.getrandbits(16)]))
    w32(ctx, rec + 0xc, r.getrandbits(32))
    for i in range(8):
        p = newbuf(ctx, 0x40)
        w32(ctx, p + 0x18, r.choice([0, 5, -7, 639, r.getrandbits(32)]))
        w32(ctx, p + 0x1c, r.choice([0, 3, 479, r.getrandbits(32)]))
        w32(ctx, rec + 0x10 + 4 * i, p)
    wb(ctx, 0x101c9ec0, rb(ctx, 0x24 * 9))
    for a in (0x1014cb00, 0x1014cafc, 0x1017b940):
        w32(ctx, a, r.choice([0, 1]))
    w32(ctx, 0x1014cb1c, r.choice([0, 1, 2, 3, 4, 5]))
    return [rec]


if NEG:
    CFLAGS = CFLAGS + ['-DW22P_MUTATE']

SPECS = [dict(name='Sim_100a539e', orig=0x100a539e, c='Sim_100a539e', gen=g, void=True, trials=300 if NEG else 4000)]
