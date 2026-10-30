# Sound with SDL3: 0x100397e7 (3D listener from the camera) against SndSdl_100397e7. The original's listener methods
# are the silent 3D stubs (sound_common.py): SDL3 has no listener, so only the memory (the parameter block) and the
# event log (empty on both sides) are compared. Sim_10084766 and Sound_ErrorName run for real.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/sound_common.py').read())

SOURCES = closure(['sim/sound/sdl_sound.c']) + [AUDIO_STUB]


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx)
    sound_install(ctx)
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
        w32(ctx, snd + 0xc, r.choice([object3d(ctx), object3d(ctx), object3d(ctx), 0]))
        w32(ctx, 0x1015d0a4, snd)
    return []


SPECS = [dict(name='SndSdl_100397e7', orig=0x100397e7, c='SndSdl_100397e7', gen=g, void=True, trials=3000)]