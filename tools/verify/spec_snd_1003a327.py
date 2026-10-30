# Sound with SDL3: 0x1003a327 (pause) against SndSdl_1003a327; events compared (sound_common.py).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/sound_common.py').read())

SOURCES = closure(['sim/sound/sdl_sound.c']) + [AUDIO_STUB]


def g(ctx):
    r = begin(ctx)
    sound_install(ctx)
    sound_block(ctx, r)
    w32(ctx, 0x1015ea7c, r.choice([0, 1]))
    return []


SPECS = [dict(name='SndSdl_1003a327', orig=0x1003a327, c='SndSdl_1003a327', gen=g, void=True, trials=3000)]