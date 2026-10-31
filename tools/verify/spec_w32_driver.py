# Round 32, D: the queued polygon to the display (1.0 0x100a539e -> 1.2 0x1009185b) against 1.2's own code, on the
# round-22 spec's generator and stubs, the C with the Vulkan switch and the 3D mode on. 1.2 returns for no polygon,
# keeps only the low byte of the polygon's light dword +0x3c in the vertex light (+0x10) and puts its other three
# bytes in the specular colour (+0x18, under the fog byte). The vertex array 0x101c9ec0 is 1.2's 0x10189140.
# Negative check: W32_NEG=1 builds the C with W32V_MUTATE (red and blue swapped in the specular colour).
import os
exec(open('tools/verify/spec_w22_100a539e.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W32_NEG'):
    CFLAGS = CFLAGS + ['-DW32V_MUTATE']


def g12(ctx):
    args = g(ctx)
    r = ctx.rng
    w32(ctx, 0x1017b940, 1)
    w32(ctx, ctx.m.syms['g_w31Tex'], 1)
    for i in range(8):
        p = r32(ctx, args[0] + 0x10 + 4 * i)
        w32(ctx, p + 0x3c, r.choice([0xff, 0x80, 0, r.getrandbits(32), r.getrandbits(32)]))
    return [0] if r.random() < 0.03 else args


SPECS = [v12_spec(dict(SPECS[0], gen=g12, trials=2000), 0x1009185b, windows={0x101c9ec0: 0x24 * 9})]


_INV = {A12(a12): a10 for a10, a12 in SPECS[0]['_v12_pairs'].items()}


def _lm(log_o, log_c):
    def fix(v):
        return _INV.get(v, v) if isinstance(v, int) else v
    return [(e[0], tuple(fix(v) for v in e[1])) if isinstance(e[1], tuple) else e for e in log_o], log_c


SPECS[0]['log_map'] = _lm
