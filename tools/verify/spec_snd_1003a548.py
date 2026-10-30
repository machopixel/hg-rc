# Sound with SDL3: 0x1003a548 (sound shutdown) against SndSdl_1003a548; events compared (sound_common.py). The
# device's Release is the 'Close' event, CoUninitialize answers silently (it went with CoInitialize).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/sound_common.py').read())

SOURCES = closure(['sim/sound/sdl_sound.c']) + [AUDIO_STUB]


def g(ctx):
    r = begin(ctx)
    sound_install(ctx)
    sound_device_install(ctx)
    SND['status'], SND['clip'], SND['new'] = {}, {}, []
    slot = r.choice([0x109e369c, newbuf(ctx, 4)])
    w32(ctx, slot, r.choice([0, ctx.m.syms['SndSdl_Device']]))
    return [slot]


SPECS = [dict(name='SndSdl_1003a548', orig=0x1003a548, c='SndSdl_1003a548', gen=g, trials=500)]