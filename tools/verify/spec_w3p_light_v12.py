# Round 33: Sim_1005874a (the lights into the camera's frame) against 1.2's 0x1004b44a, which also turns SKYGND.PAR's
# two coloured lights' unit directions into the camera's frame (C globals g_w33Light1Dir / 2Dir -> g_w33Light1Cam /
# 2Cam here, paired with 1.2's 0x10138d2c.. / 0x101181a4..). spec_w3p.py's generator plus the two directions.
# Negative check: W3P_NEG=1 builds the C with W3PL_MUTATE (light 1's z left out of its camera-frame x).
import os
exec(open('tools/verify/spec_w3p.py').read())
exec(open('tools/verify/v12_common.py').read())
if 'HELPERS' not in globals():
    HELPERS = {}            # spec_w3p.py stubs on its own; no helper calls to pair here
if os.environ.get('W3P_NEG'):
    CFLAGS = list(CFLAGS) + ['-DW3PL_MUTATE']
SOURCES = list(SOURCES) + ['../tools/verify/w33_light_stub.c']
# round 31's texture files, which spec_w3p.py's list predates
_W31 = ['sim/render/w31_tex.c', '../tools/verify/plat_stub.c']
SOURCES = SOURCES + [x for x in _W31 if x not in SOURCES]
C12 = [('g_w33Light1Dir', 0, 0x10138d2c, 12), ('g_w33Light2Dir', 0, 0x10138d4c, 12),
       ('g_w33Light1Cam', 0, 0x101181a4, 4), ('g_w33Light1Cam', 4, 0x10138d44, 4), ('g_w33Light1Cam', 8, 0x10138d48, 4),
       ('g_w33Light2Cam', 0, 0x101181a8, 4), ('g_w33Light2Cam', 4, 0x10138d64, 4), ('g_w33Light2Cam', 8, 0x10138d68, 4)]


def g12(ctx):
    args = g_5874a(ctx)
    r = ctx.rng
    for name in ('g_w33Light1Dir', 'g_w33Light2Dir'):
        for i in range(3):
            w32(ctx, ctx.m.syms[name] + 4 * i, rfloat_n(r))
    return args


def to12_more(m):
    for name, off, a12, n in C12:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(m.syms[name] + off, n)))


def from12_more(m):
    for name, off, a12, n in C12:
        m.uc.mem_write(m.syms[name] + off, bytes(m.uc.mem_read(A12(a12), n)))


PAIRS12 = {0x109b16c4: 0x107c2f84, 0x109b16c8: 0x107c2f88, 0x109b16c0: 0x107c2f80, 0x109afdcc: 0x107c276c,
           0x10143fc0: 0x1011817c, 0x101668b8: 0x10138cfc, 0x101668bc: 0x10138d00, 0x101668c0: 0x10138d04,
           0x101668c4: 0x10138d08, 0x101668c8: 0x10138d0c, 0x101668d0: 0x10138d14}
_s = [s for s in SPECS if s['name'] == 'Sim_1005874a'][0]
SPECS = [v12_spec(dict(_s, gen=g12, trials=3000), 0x1004b44a, extra_pairs=PAIRS12, to12_more=to12_more,
                  from12_more=from12_more)]
