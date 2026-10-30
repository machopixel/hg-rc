# Round 11: 0x10080471 (show the frame). The surface methods are stubs (__stdcall) on both sides: Flip +0x2c (IND1),
# IsLost +0x60 (IND2), Restore +0x6c (IND3), Unlock +0x80 (IND4); Sim_10083495 (the error log, arguments ignored by
# it) is a stub without logged arguments.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1008/w11_ddraw.c'])
helper(IND1, None, 3)
helper(IND2, None, 1)
helper(IND3, None, 1)
helper(IND4, None, 2)
STDCALL.update((IND1, IND2, IND3, IND4))
helper(0x10083495, 'Sim_10083495', 0)
ACTIVE = tuple(HELPERS)
LOST = 0x887601c2


def surface(ctx):
    tbl = newbuf(ctx, 0x90)
    for off, ind in ((0x2c, IND1), (0x60, IND2), (0x6c, IND3), (0x80, IND4)):
        w32(ctx, tbl + off, ind)
    s = newbuf(ctx, 8)
    w32(ctx, s, tbl)
    return s


def g(ctx):
    r = begin(ctx, *ACTIVE)
    rets(IND1, r.choice([0, 0, LOST, 0x88760096]))
    rets(IND2, *[r.choice([0, LOST]) for _ in range(3)])
    rets(IND3, r.choice([0, 0x88760096]))
    rets(IND4, r.choice([0, 0x887600e1]))
    w32(ctx, 0x1017b394, surface(ctx))
    w32(ctx, 0x1017b39c, surface(ctx))
    w32(ctx, 0x1017b3ac, r.choice([0, 1]))
    w32(ctx, 0x10372d44, r.getrandbits(32))
    p = newbuf(ctx, 4)
    w32(ctx, p, 7)
    w32(ctx, 0x1015e698, p)
    return []


SPECS = [dict(name='Sim_10080471', orig=0x10080471, c='Sim_10080471', gen=g, trials=3000)]