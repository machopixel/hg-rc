# Round 11: 0x10039931 (3D sound position of a unit). The object's method +0x30 is a stub (IND1, __stdcall, block
# logged by content) on both sides; Sound_ErrorName (0x10039540) runs for real.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w11_sound.c'])
helper(IND1, None, 3, deref={1: 0x1c})
STDCALL.add(IND1)
ACTIVE = (IND1,)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    rets(IND1, r.choice([0, 1, 0x88780032, 0x80070057, 0xffffffff]))
    if r.random() < 0.1:
        w32(ctx, 0x1015d0a4, 0)
    else:
        snd = newbuf(ctx, 0x140)
        w32(ctx, snd + 0xc, r.choice([0, 1, 1, 1]))
        tbl = newbuf(ctx, 0x40)
        w32(ctx, tbl + 0x30, IND1)
        for k in range(8):
            o = newbuf(ctx, 8)
            w32(ctx, o, tbl)
            w32(ctx, snd + 0x114 + 4 * k, o)
        w32(ctx, 0x1015d0a4, snd)
    if r.random() < 0.1:
        return [r.randrange(8), 0]
    unit = newbuf(ctx, 0x70)
    w32(ctx, unit + 0x28, newbuf(ctx, 0x560))
    return [r.randrange(8), unit]


SPECS = [dict(name='Sim_10039931', orig=0x10039931, c='Sim_10039931', gen=g, void=True, trials=2000)]