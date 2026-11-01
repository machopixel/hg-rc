# Sweep (graphics): the frame's view and draw step (1.0 0x100352d5, Sim_100352d5) against 1.2's 0x1002a195, in 1.2's
# mode (g_w31Tex and the 3D mode). 1.2's change: after the background quad or the sky routine, render state 0xc (our
# 0xd: the depth buffer cleared) unless the display kind [0x1014cb1c] is 1, then the world (Sim_100a4c45). Every
# callee is a logging stub on both sides; the driver's quad slot [0x1014cb4c] (1.2 [0x1011fa54]) and the renderer's
# state slot [0x109e3d40] (1.2 [0x107ee400]) point at stubs, their calls logged with their arguments.
# Negative check: W9HD_NEG12=1 builds the C with W9HD_MUTATE (no clear).
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w9h_view.c'])
if os.environ.get('W9HD_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW9HD_MUTATE']
CALLS = [(0x100d2306, None, 1), (0x100d255e, 'Sim_100d255e', 1), (0x100d1f90, None, 1), (0x100a0def, None, 1),
         (0x100a4c45, 'Sim_100a4c45', 0), (0x100071f6, 'Sim_100071f6', 0), (0x100d32e5, 'Sim_100d32e5', 1)]
for _a, _c, _n in CALLS:
    helper(_a, _c, _n)
helper(IND1, None, 3)       # the driver's quad slot (op, vertices, flags)
helper(IND2, None, 2)       # the renderer's state slot (code, value)
ACTIVE = tuple(a for a, _, _ in CALLS) + (IND1, IND2)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, ctx.m.syms['g_w31Tex'], 1)
    w32(ctx, 0x1017b940, r.choice([1, 1, 1, 0]))
    w32(ctx, 0x1013a014, r.choice([0, 1]))
    w32(ctx, 0x1014cae0, newbuf(ctx, 0x100))
    for a in (0x1014cb18, 0x1014cb04, 0x1014cb08):
        w32(ctx, a, r.choice([0, 0, 1]))
    w32(ctx, 0x1014cb1c, r.choice([0, 0, 1, 2]))
    w32(ctx, 0x1015d1ac, r.choice([0, 4, 2]))
    w32(ctx, 0x10138c70, r.getrandbits(32))
    w32(ctx, 0x1014cb4c, IND1)
    w32(ctx, 0x109e3d40, IND2)
    return []


SPECS = [v12_spec(dict(name='Sim_100352d5', orig=0x100352d5, c='Sim_100352d5', gen=g, trials=3000, void=True),
                  0x1002a195,
                  extra_pairs={0x100d2306: 0x100b8496, 0x100d255e: 0x100b86ee, 0x100d1f90: 0x100b8120,
                               0x100a0def: 0x1008dff3, 0x100a4c45: 0x10091065, 0x100071f6: 0x100020d8,
                               0x100d32e5: 0x100b9125, 0x1017b940: 0x1014a5e8, 0x1013a014: 0x1010f67c,
                               0x1014cae0: 0x1011f9e8, 0x1014cb18: 0x1011fa20, 0x1014cb1c: 0x1011fa24,
                               0x1015d1ac: 0x1018ea7c, 0x1014cb04: 0x1011fa0c, 0x1014cb08: 0x1011fa10,
                               0x10138c70: 0x1010e590, 0x1014cb4c: 0x1011fa54, 0x109e3d40: 0x107ee400},
                  name='Sim_100352d5_v12')]


def lm(log_o, log_c):
    """1.2's state codes are 1.0's minus 1 (its 0xc is our 0xd)"""
    out = []
    for e in log_o:
        if e[0] == 'stub_%x' % IND2:
            e = (e[0], (e[1][0] + 1,) + tuple(e[1][1:]))
        out.append(e)
    return out, log_c


SPECS[0]['log_map'] = lm
