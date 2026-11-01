# Round 33: the polygon vertex lighting Sim_10058c5e against 1.2's 0x1004bad6: 1.0's light plus a colour per point in
# +0x3c's upper bytes (SKYGND.PAR's ambient, global, light 1 and light 2 colours, white per point light) and the
# point lights also in 3D mode with fog on except for the b000..e000 polygons. The 1.0 spec's generator, with the
# polygon's kind word and the new colours and camera-frame light directions (C globals here, paired with 1.2's).
# Stubs as in the 1.0 spec, hooked at 1.2's twins too (0x100a648c = 1.2 0x1009250c, 0x1005897b = 0x1004b7f3, the fog
# factor 0x10058ac2 = 0x1004b93a).
# Negative check: W10_NEG=1 builds the C with W10L_MUTATE (light 2's blue from its green).
import os
exec(open('tools/verify/spec_w10_10058c5e.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W10_NEG'):
    CFLAGS = list(CFLAGS) + ['-DW10L_MUTATE']
_g10 = g
C12 = [('g_w33Ambient', 0, 0x10138d20, 12), ('g_w33Light1', 0, 0x10138d38, 12), ('g_w33Light2', 0, 0x10138d58, 12),
       ('g_w33Global', 0, 0x10138d6c, 12),
       ('g_w33Light1Cam', 0, 0x101181a4, 4), ('g_w33Light1Cam', 4, 0x10138d44, 4), ('g_w33Light1Cam', 8, 0x10138d48, 4),
       ('g_w33Light2Cam', 0, 0x101181a8, 4), ('g_w33Light2Cam', 4, 0x10138d64, 4), ('g_w33Light2Cam', 8, 0x10138d68, 4)]


def g12(ctx):
    args = _g10(ctx)
    r = ctx.rng
    if args[0]:
        w16(ctx, args[0], r.choice([0, 0x1000, 0x7000, 0xb000, 0xc000, 0xd000, 0xe000, r.getrandbits(16)]))
    for name in ('g_w33Ambient', 'g_w33Light1', 'g_w33Light2', 'g_w33Global'):
        w32(ctx, ctx.m.syms[name], *[r.choice([0, 1, 128, 255, r.randrange(0, 256)]) for _ in range(3)])
    for name in ('g_w33Light1Cam', 'g_w33Light2Cam'):
        w32(ctx, ctx.m.syms[name], *[fl(r, 0, 1) for _ in range(3)])
    return args


def to12_more(m):
    if not getattr(m, '_w10l12', False):
        m._w10l12 = True
        m.raw.hook_add(UC_HOOK_CODE, _fstub_cb(m, 0x10058ac2, 0), None, A12(0x1004b93a), A12(0x1004b93a))
    for name, off, a12, n in C12:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(m.syms[name] + off, n)))


def from12_more(m):
    for name, off, a12, n in C12:
        m.uc.mem_write(m.syms[name] + off, bytes(m.uc.mem_read(A12(a12), n)))


PAIRS12 = {0x100a648c: 0x1009250c, 0x1005897b: 0x1004b7f3,
           # the globals (the aligner mispairs some here: 1.2's function is largely new)
           0x101668b4: 0x10138cf8, 0x10143fbc: 0x10118178, 0x10143fc0: 0x1011817c, 0x101668b8: 0x10138cfc,
           0x101668bc: 0x10138d00, 0x101668c0: 0x10138d04, 0x101668c4: 0x10138d08, 0x101668c8: 0x10138d0c,
           0x101668d0: 0x10138d14, 0x101668d8: 0x10138d1c, 0x109afdcc: 0x107c276c, 0x109b16c4: 0x107c2f84,
           0x109b16c8: 0x107c2f88, 0x109b16c0: 0x107c2f80, 0x10143fd4: 0x10118190, 0x1014cb2c: 0x1011fa34,
           0x10143fcc: 0x10118188, 0x10143fd8: 0x10118194, 0x1014cae0: 0x1011f9e8, 0x1014cb1c: 0x1011fa24,
           0x1014cb38: 0x1011fa40, 0x1017b940: 0x1014a5e8, 0x1017d108: 0x1014b9b0, 0x101c9ea8: 0x10189130,
           0x1012b4b0: 0x10101138, 0x1012b4b4: 0x1010113c}
SPECS = [v12_spec(dict(SPECS[0], gen=g12), 0x1004bad6, extra_pairs=PAIRS12, to12_more=to12_more,
                  from12_more=from12_more)]
