# Sound with SDL3: 0x10039c00 (per-frame voice update) against SndSdl_10039c00; events compared (sound_common.py).
# Sim_1000e737 (start the queued sound) is a stub on both sides; the pan computation Sim_1000ed77, Sim_10084766 and
# Sound_ErrorName run for real. The 3D path runs with silent 3D objects (no SDL3 counterpart).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/sound_common.py').read())

SOURCES = closure(['sim/sound/sdl_sound.c']) + [AUDIO_STUB]
helper(0x1000e737, 'Sim_1000e737', 12)
ACTIVE = (0x1000e737,)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    sound_install(ctx)
    sound_block(ctx, r)
    w32(ctx, 0x1017b92c, r.choice([0, 0, 1]))
    w32(ctx, 0x1015ea7c, r.choice([0, 0, 1]))
    w32(ctx, 0x101321b8, r.getrandbits(32))
    self_ = r.randrange(4)
    w32(ctx, 0x1017b8c8, self_)
    unit = newbuf(ctx, 0x40)
    w32(ctx, unit + 0x28, newbuf(ctx, 0x560))
    w32(ctx, 0x109e8b40 + 4 * self_, unit)
    cam = newbuf(ctx, 0x20)
    w32(ctx, cam, *[fb(r.uniform(-3000, 3000)) for _ in range(3)])
    w32(ctx, cam + 0xc, fb(r.uniform(-360, 360)))
    w32(ctx, 0x1014cae0, cam)
    return []


SPECS = [dict(name='SndSdl_10039c00', orig=0x10039c00, c='SndSdl_10039c00', gen=g, trials=3000)]