# Round 11: 0x1003a548 (sound shutdown). The object's Release (method +8) is a stub (IND1, __stdcall) on both
# sides; CoUninitialize is the checker's Windows stub (logged).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w11_sound.c'])
helper(IND1, None, 1)
STDCALL.add(IND1)
ACTIVE = (IND1,)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    slot = newbuf(ctx, 4)
    if r.random() < 0.25:
        w32(ctx, slot, 0)
    else:
        tbl = newbuf(ctx, 0x20)
        w32(ctx, tbl + 8, IND1)
        obj = newbuf(ctx, 0x10)
        w32(ctx, obj, tbl)
        w32(ctx, slot, obj)
    return [slot]


SPECS = [dict(name='Sim_1003a548', orig=0x1003a548, c='Sim_1003a548', gen=g, trials=2000)]