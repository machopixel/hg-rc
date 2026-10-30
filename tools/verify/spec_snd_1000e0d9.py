# Sound with SDL3: 0x1000e0d9 (find a slot, load a clip into a new voice) against SndSdl_1000e0d9; events compared
# (sound_common.py). The clip loader 0x100ba7d5 and the release Sim_100bb2b0 are stubs on both sides (the loader
# hands out the trial's clip data and writes its size); the chunk search Sim_1000e060 and memcpy run for real, so the
# copied clip bytes are compared as memory.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/sound_common.py').read())

SOURCES = closure(['sim/sound/sdl_sound.c']) + [AUDIO_STUB]
helper(0x100ba7d5, None, 3)
helper(0x100bb2b0, 'Sim_100bb2b0', 2)
ACTIVE = (0x100ba7d5, 0x100bb2b0)


def _load(uc, k, raw, eax):
    uc.mem_write(raw[2], struct.pack('<I', SND['wave_size']))
    return SND['wave'][k % len(SND['wave'])]


EFFECT[0x100ba7d5] = _load


def wave(ctx, r):
    """Clip data: random bytes with the data chunk marker somewhere in the first part, then its length."""
    buf = newbuf(ctx, 0xa00)
    wb(ctx, buf, rb(ctx, 0xa00))
    at = r.choice([0, 0x2c, r.randrange(0, 0x60)])
    if r.random() < 0.9:
        w32(ctx, buf + at, r32(ctx, 0x1012df58), r.choice([0x10, 0x100, 0x800, 0x900, r.randrange(0, 0x800)]))
    return buf


def g(ctx):
    r = begin(ctx, *ACTIVE)
    sound_install(ctx)
    sound_block(ctx, r)
    w32(ctx, 0x109e369c, device(ctx))           # the original creates through it (100399f4)
    w32(ctx, 0x1017b92c, r.choice([0, 0, 1]))
    w32(ctx, 0x1014ceb4, newbuf(ctx, 0x10))
    SND['wave'] = [r.choice([0, wave(ctx, r), wave(ctx, r), wave(ctx, r)])]
    SND['wave_size'] = r.choice([0x40, 0x80, 0x100])
    data = r.choice([0, 0, 0, wave(ctx, r)])
    flags = r.choice([0, 0x100, 0x200, 0x400, 0x2000]) | r.choice([0, 0x100, 0x200, 0x400]) | r.randrange(0, 0x100)
    if r.random() < 0.2:
        flags = r.getrandbits(32)
    sid = r.choice([-1 & 0xffffffff, 0, r.randrange(0, 50), r.randrange(0, 50)])
    blk = r32(ctx, 0x1015d0a4)
    for i in range(1, 16):                      # slots holding the same sound, kept once (0x400) or replaced (0x200)
        if r.random() < 0.15:
            w16(ctx, blk + 0x314 + 2 * i, r.choice([0x400, 0x200, 0x600]) | r.randrange(0, 0x100))
            w32(ctx, blk + 0x2d4 + 4 * i, sid)
    return [sid, data, flags, r.getrandbits(32),
            r.getrandbits(32), r.choice([0, 1, 1, 2]), newbuf(ctx, 0x10)]


SPECS = [dict(name='SndSdl_1000e0d9', orig=0x1000e0d9, c='SndSdl_1000e0d9', gen=g, trials=3000)]