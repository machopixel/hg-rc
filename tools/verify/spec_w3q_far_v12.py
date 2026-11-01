# Sweep (graphics): the three backdrop shapes drawn through the display driver slot [0x1014cb4c] (1.2 [0x1011fa54]) -
# the quad 0x100a291d and triangle 0x100a29da (w3q_driver.c, 1.2 0x1008fc0f / 0x1008fce8) and the pentagon
# 0x100a2823 (w6g_misc.c, 1.2 0x1008faed) - against 1.2, in 1.2's mode (g_w31Tex and the 3D mode). 1.2's change:
# each vertex's z (+8) is 3.4e38 (0x7f7fc99e), the far end. The slot points at a stub that logs, per vertex, the
# fields 1.2 writes (x, y, z, colour +0x10, fog +0x18); 1.2 leaves the others unset (its stack).
# Negative check: W3QZ_NEG12=1 builds the C with W3QZ_MUTATE (1.0's z).
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/render/w3q_driver.c', 'sim/unsorted_100a/w6g_misc.c'])
if os.environ.get('W3QZ_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW3QZ_MUTATE']
helper(IND1, None, 3)


def e_draw(uc, k, raw, eax):
    n, v = raw[0], raw[1]
    words = []
    for i in range(min(n, 8)):
        for off in (0, 4, 8, 0x10, 0x18):
            words.append(ur32(uc, v + i * 0x24 + off))
    return ([('verts', tuple(words))], None)


EFFECT[IND1] = e_draw


def gen(nargs):
    def g(ctx):
        r = begin(ctx, IND1)
        w32(ctx, ctx.m.syms['g_w31Tex'], 1)
        w32(ctx, 0x1017b940, 1)
        w32(ctx, 0x1014cb4c, IND1)
        return [r.randrange(-100, 800) for _ in range(nargs)] + [r.getrandbits(32)]
    return g


PAIRS = {0x1014cb4c: 0x1011fa54, 0x1017b940: 0x1014a5e8}
SPECS = [v12_spec(dict(name=c, orig=a10, c=c, gen=gen(n), trials=1500), a12, extra_pairs=PAIRS, name=c + '_v12')
         for c, a10, a12, n in (('Sim_100a291d', 0x100a291d, 0x1008fc0f, 8), ('Sim_100a29da', 0x100a29da, 0x1008fce8, 6),
                                ('Sim_100a2823', 0x100a2823, 0x1008faed, 10))]
