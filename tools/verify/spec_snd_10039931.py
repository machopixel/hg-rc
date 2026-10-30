# Sound with SDL3: 0x10039931 (3D position of a unit's sound) against SndSdl_10039931. The original's 3D buffers are
# the silent 3D stubs (sound_common.py): SDL3 has no 3D buffer, so only the memory (the parameter block) and the event
# log (empty on both sides) are compared. Sound_ErrorName runs for real.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/sound_common.py').read())

SOURCES = closure(['sim/sound/sdl_sound.c']) + [AUDIO_STUB]


def g(ctx):
    r = begin(ctx)
    sound_install(ctx)
    if r.random() < 0.1:
        w32(ctx, 0x1015d0a4, 0)
    else:
        snd = newbuf(ctx, 0x140)
        w32(ctx, snd + 0xc, r.choice([0, 1, 1, 1]))
        for k in range(16):
            w32(ctx, snd + 0x114 + 4 * k, object3d(ctx))
        w32(ctx, 0x1015d0a4, snd)
    if r.random() < 0.1:
        return [r.randrange(16), 0]
    unit = newbuf(ctx, 0x70)
    w32(ctx, unit + 0x28, newbuf(ctx, 0x560))
    return [r.randrange(16), unit]


SPECS = [dict(name='SndSdl_10039931', orig=0x10039931, c='SndSdl_10039931', gen=g, void=True, trials=2000)]