# Round 11: 0x10039708 (call method +0x44 of a sound object). The method is a stub (IND1, __stdcall) on both sides.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w11_sound.c'])
helper(IND1, None, 2)
STDCALL.add(IND1)
ACTIVE = (IND1,)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    if r.random() < 0.2:
        return [0, r.getrandbits(32)]
    tbl = newbuf(ctx, 0x60)
    w32(ctx, tbl + 0x44, IND1)
    obj = newbuf(ctx, 0x10)
    w32(ctx, obj, tbl)
    return [obj, r.getrandbits(32)]


SPECS = [dict(name='Sim_10039708', orig=0x10039708, c='Sim_10039708', gen=g, void=True, trials=2000)]