# Round 11: 0x1007f950 (DirectDraw shutdown). Methods are stubs (__stdcall) on both sides; 0x1007f9da runs for real.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1007/w11_ddraw.c'])
helper(IND1, None, 1)            # Release (+8)
helper(IND2, None, 1)            # RestoreDisplayMode (+0x4c)
helper(IND3, None, 3)            # SetCooperativeLevel (+0x50)
STDCALL.update((IND1, IND2, IND3))
ACTIVE = (IND1, IND2, IND3)


def obj(ctx, r):
    if r.random() < 0.25:
        return 0
    tbl = newbuf(ctx, 0x60)
    w32(ctx, tbl + 8, IND1)
    w32(ctx, tbl + 0x4c, IND2)
    w32(ctx, tbl + 0x50, IND3)
    o = newbuf(ctx, 0x10)
    w32(ctx, o, tbl)
    return o


def g(ctx):
    r = begin(ctx, *ACTIVE)
    for a in (0x1017b390, 0x1017b394, 0x1017b39c, 0x1017b3a8):
        w32(ctx, a, obj(ctx, r))
    for a in (0x1015e698, 0x1015e68c):
        p = newbuf(ctx, 4)
        w32(ctx, p, r.choice([0, 1, r.getrandbits(32)]))
        w32(ctx, a, p)
    w32(ctx, 0x109e3d14, r.choice([0x140, 0x280]))
    w32(ctx, 0x109e3d18, r.choice([0xc8, 0x1e0]))
    w32(ctx, 0x1017b8f0, r.getrandbits(32))
    return []

SPECS = [dict(name='Sim_1007f950', orig=0x1007f950, c='Sim_1007f950', gen=g, void=True, trials=2000)]
