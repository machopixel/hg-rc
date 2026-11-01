# Sound round (1.2): the slot search and clip load SndSdl_1000e0d9 against 1.2's 0x10007bf9, on
# spec_snd_1000e0d9.py's generator. 1.2's only change: when the new voice cannot be made (0x1002da24 = 1.0
# 0x100399f4 returns 0) it skips the Lock, the copy and the Unlock (0x10007f0e). A quarter of the trials make
# CreateSoundBuffer / Audio_VoiceCreate fail (no new voice handle).
# Negative check: SND_NEG12=1 builds the C with SND_MUTATE12 (1.0's code, no guard).
import os
exec(open('tools/verify/spec_snd_1000e0d9.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('SND_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DSND_MUTATE12']


def g12(ctx, _g=g):
    import random as _random
    args = _g(ctx)
    if _random.Random(r32(ctx, 0x1015d0a4) ^ args[2]).random() < 0.25:
        SND['new'] = []
    return args


SPECS = [v12_spec(dict(SPECS[0], gen=g12), 0x10007bf9)]
ENTRY10, ENTRY12 = 0x10a39204, A12(0x10835204)      # the clip table (0x1000e510 / 1.2 0x10008038), 0x14 a row


def lm(log_o, log_c):
    """the clip-table row handed to the loader and the release is 1.2's own table: logged as 1.0's row"""
    out = []
    for e in log_o:
        if isinstance(e[1], tuple) and e[1] and isinstance(e[1][0], int) and -0x14 * 64 <= e[1][0] - ENTRY12 < 0x14 * 0x4000:   # id -1 too
            e = (e[0], (e[1][0] - ENTRY12 + ENTRY10,) + tuple(e[1][1:]))
        out.append(e)
    return out, log_c


SPECS[0]['log_map'] = lm
