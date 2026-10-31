# Round 32, D: the display driver's polygon routine (1.0 0x1009ef50 -> 1.2 0x1008bd80) against 1.2's own code, on
# the round-22 spec's generator and stubs, the C with the Vulkan switch on. 1.2's one change: the flat colour polygons
# (0xa000) in 3D mode also get rhw = 1 / z. 1.2's renderer modes are 1.0's + 2 from 7 on (DECISIONS.md, round 31):
# the renderer stub's mode is logged in 1.0's numbering. The fog constants are read-only in 1.2: the 1.0 side gets
# 1.2's values. Negative check: W32_NEG=1 builds the C with W32R_MUTATE (rhw left as it was).
import os
exec(open('tools/verify/spec_w22_1009ef50.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W32_NEG'):
    CFLAGS = CFLAGS + ['-DW32R_MUTATE']
_b12, _img12, _ = _img(V12_DLL)


def _const12(a, n):
    return bytes(_img12[a - _b12:a - _b12 + n])


def g12(ctx):
    args = g(ctx)
    w32(ctx, ctx.m.syms['g_w31Tex'], 1)
    wb(ctx, 0x1012ba10, _const12(0x1010358c, 4))
    wb(ctx, 0x1012ba14, _const12(0x10103590, 4))
    wb(ctx, 0x1012ba18, _const12(0x10103598, 8))
    wb(ctx, 0x1012ba20, _const12(0x101035a0, 8))
    if ctx.rng.random() < 0.4:
        args[2] = 0xa000 | (args[2] & 0xfff)
        w32(ctx, 0x1017b940, 1)
        w32(ctx, 0x1017b948, 1)
    return args


_s = v12_spec(dict(SPECS[0], gen=g12, trials=3000), 0x1008bd80, extra_pairs={0x10098f30: 0x10086fb0, 0x10006744: 0x100012c5},
              windows={0x101ce4a0: 0x400, 0x101ccca0: 0x400, 0x10185760: 0x400},
              skip={0x1012ba10, 0x1012ba14, 0x1012ba18, 0x1012ba1c, 0x1012ba20, 0x1012ba24})
_INV = {A12(a12): a10 for a10, a12 in _s['_v12_pairs'].items()}


def _lm(log_o, log_c):
    out = []
    for e in log_o:
        if isinstance(e[1], tuple):
            a = tuple(_INV.get(v, v) if isinstance(v, int) else v for v in e[1])
            if e[0] == 'stub_%x' % RENDER and isinstance(a[0], int) and a[0] >= 7:
                a = (a[0] - 2,) + a[1:]
            e = (e[0], a)
        out.append(e)
    return out, log_c


_s['log_map'] = _lm
SPECS = [_s]
