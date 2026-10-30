# Round 11: 0x100397e7 (3D sound listener from the camera). The listener's methods +0x28 (block logged by content)
# and +0x44 are stubs (__stdcall) on both sides; Sim_10084766 and Sound_ErrorName run for real.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w11_sound.c'])
helper(IND1, None, 3, deref={1: 0x34})
helper(IND2, None, 1)
STDCALL.update((IND1, IND2))
ACTIVE = (IND1, IND2)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    rets(IND1, r.choice([0, 1, 0x88780032, 0x80070057]))
    rets(IND2, r.choice([0, 0x80004005]))
    self_ = r.randrange(4)
    w32(ctx, 0x1017b8c8, self_)
    unit = newbuf(ctx, 0x40)
    w32(ctx, unit + 0x28, newbuf(ctx, 0x560))
    w32(ctx, 0x109e8b40 + 4 * self_, unit)
    cam = newbuf(ctx, 0x20)
    w32(ctx, cam, *[fb(r.uniform(-3000, 3000)) for _ in range(3)])
    w32(ctx, cam + 0xc, r.choice([fb(r.uniform(-720, 720)), 0, fb(90.0), fb(-90.0)]))
    w32(ctx, 0x1014cae0, cam)
    if r.random() < 0.1:
        w32(ctx, 0x1015d0a4, 0)
    else:
        snd = newbuf(ctx, 0x40)
        tbl = newbuf(ctx, 0x50)
        w32(ctx, tbl + 0x28, IND1)
        w32(ctx, tbl + 0x44, IND2)
        lis = newbuf(ctx, 8)
        w32(ctx, lis, tbl)
        w32(ctx, snd + 0xc, r.choice([lis, lis, lis, 0]))
        w32(ctx, 0x1015d0a4, snd)
    return []


SPECS = [dict(name='Sim_100397e7', orig=0x100397e7, c='Sim_100397e7', gen=g, void=True, trials=3000)]