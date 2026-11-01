# Sound round (1.2): the per-frame voice update SndSdl_10039c00 against 1.2's 0x1002dc30, on
# spec_snd_10039c00.py's generator. 1.2's only change: a finished or stopped voice is released until its reference
# count is 0 (0x1002dec9, 0x1002e021); the fake DirectSound's Release answers 0, as an audio.h voice is gone after
# one destroy. The 3D flag (block +0x10) is 0 in every trial here: 1.2's 3D listener / voice functions
# (0x1002d817, 0x1002d961) differ from 1.0's by the unit layout only and have no SDL3 counterpart (DECISIONS.md).
# Negative check: SND_NEG12=1 builds the C with SND_MUTATE12_RELEASE (two destroys).
import os
exec(open('tools/verify/spec_snd_10039c00.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('SND_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DSND_MUTATE12_RELEASE']


def g12(ctx):
    r = begin(ctx, *ACTIVE)
    sound_install(ctx)
    sound_block(ctx, r, three_d=False)
    w32(ctx, 0x1017b92c, r.choice([0, 0, 1]))
    w32(ctx, 0x1015ea7c, r.choice([0, 0, 1]))
    w32(ctx, 0x101321b8, r.getrandbits(32))
    return []


SPECS = [v12_spec(dict(SPECS[0], gen=g12), 0x1002dc30)]
print('pairs', {hex(a): hex(b) for a, b in SPECS[0]['_v12_pairs'].items() if not 0x1002d000 < b < 0x1002e100})
