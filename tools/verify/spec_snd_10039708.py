# Sound with SDL3: 0x10039708 (pitch) against SndSdl_10039708; events compared (sound_common.py).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/sound_common.py').read())

SOURCES = closure(['sim/sound/sdl_sound.c']) + [AUDIO_STUB]


def g(ctx):
    r = begin(ctx)
    sound_install(ctx)
    SND['status'], SND['clip'], SND['new'] = {}, {}, []
    return [r.choice([0, voice(ctx), voice(ctx)]), r.choice([0, 22050, 11025, 99, 100, 100000, 100001, 200000, r.getrandbits(32)])]


SPECS = [dict(name='SndSdl_10039708', orig=0x10039708, c='SndSdl_10039708', gen=g, void=True, trials=1000)]