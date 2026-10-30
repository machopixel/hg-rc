# Sound with SDL3: 0x100399f4 (create a clip buffer) against SndSdl_100399f4; events compared (sound_common.py).
# The original runs against the fake device (CreateSoundBuffer) and the buffers' QueryInterface, which answers no 3D
# interface, as SDL3 has none.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/sound_common.py').read())

SOURCES = closure(['sim/sound/sdl_sound.c']) + [AUDIO_STUB]


def g(ctx):
    r = begin(ctx)
    sound_install(ctx)
    SND['status'], SND['clip'] = {}, {}
    SND['new'] = [r.choice([0, voice(ctx), voice(ctx)])]
    out, out3d = newbuf(ctx, 4), newbuf(ctx, 4)
    w32(ctx, out, r.getrandbits(32))
    w32(ctx, out3d, r.getrandbits(32))
    return [device(ctx), out, out3d, r.choice([0, 0x100, 0x5622, r.getrandbits(20)]), r.choice([0, 1, 2, 3])]


SPECS = [dict(name='SndSdl_100399f4', orig=0x100399f4, c='SndSdl_100399f4', gen=g, trials=1000)]
