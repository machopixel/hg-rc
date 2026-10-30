# Sound with SDL3: 0x1003a0e4 (sound start) against SndSdl_1003a0e4; events compared (sound_common.py). The original
# runs without A3D (its CoCreateInstance fails) on the fake DirectSoundCreate ('Open', writes the C's device token)
# and the token's silent SetCooperativeLevel. HGSND.CFG's 3D word is off: with it on the original makes the 3D
# primary buffer and listener, which SDL3 has no counterpart for (DECISIONS.md).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/sound_common.py').read())

SOURCES = closure(['sim/sound/sdl_sound.c']) + [AUDIO_STUB]


def g(ctx):
    r = begin(ctx)
    sound_install(ctx)
    sound_device_install(ctx)
    SND['status'], SND['clip'], SND['new'] = {}, {}, []
    SND['open'] = r.choice([1, 1, 1, 0])
    blk = newbuf(ctx, 0x400)
    w32(ctx, 0x1015d0a4, blk)
    cfg = newbuf(ctx, 0x60)
    w32(ctx, cfg + 0x40, 0)
    w32(ctx, 0x1015d1b8, cfg)
    w32(ctx, 0x109e369c, r.getrandbits(32))
    return [0x109e369c, r.getrandbits(32)]


SPECS = [dict(name='SndSdl_1003a0e4', orig=0x1003a0e4, c='SndSdl_1003a0e4', gen=g, trials=1000)]