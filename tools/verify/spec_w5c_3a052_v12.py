# Sound round (1.2): the sound shutdown Sim_1003a052 against 1.2's 0x1002e0a6, built with HG_MUSIC_FILES as in the
# game. 1.2's change: before the device's shutdown it frees its music stream (0x1002e983([0x10131310]), then
# [0x10131310] = 0); ours closes the MUSIC-folder player there (Music_Close). Both are logged as one stub,
# 'stub_<MUSIC_CLOSE>' without arguments; 1.2's stream handle is nonzero in every trial (it is made by every
# successful sound start). The device shutdown 0x1003a548 (1.2 0x1002e655) and HeapFree are stubs on both sides.
# Negative check: W5C_NEG12=1 builds the C with W5C_MUTATE12 (no music close).
import os
import struct
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w5c_calls.c']) + ['../tools/verify/music_stub.c']
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe', '-DHG_MUSIC_FILES']
if os.environ.get('W5C_NEG12'):
    CFLAGS.append('-DW5C_MUTATE12')
MUSIC_CLOSE = 0x00F00340        # a stand-in 1.0 address for the music close (1.0 has none); paired with 1.2's
helper(MUSIC_CLOSE, 'Music_Close', 0)
helper(0x1003a548, None, 1)
ACTIVE = (MUSIC_CLOSE, 0x1003a548)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    ctx.m.raw.mem_write(MUSIC_CLOSE, b'\xc3')
    w32(ctx, 0x109e369c, r.choice([0, 1, ctx.u32()]))
    n = r.choice([0, 1, 2, 5, 10])
    tab = newbuf(ctx, 0x54 + 4 * 12)
    w32(ctx, tab + 4, n)
    for i in range(12):
        w32(ctx, tab + 0x54 + 4 * i, r.choice([0, 0, ctx.u32()]))
    w32(ctx, 0x1015d0a4, r.choice([tab, tab, tab, 0]))
    w32(ctx, 0x1017b8f8, r.getrandbits(32))
    return []


def to12_more(m):
    m.uc.mem_write(A12(0x10131310), struct.pack('<I', 0x00ae0000))     # 1.2's stream object (its handle only)


SPECS = [v12_spec(dict(name='Sim_1003a052', orig=0x1003a052, c='Sim_1003a052', gen=g, trials=2000),
                  0x1002e0a6, to12_more=to12_more,
                  # the alignment pairs the game heap with the sound block (1.2 pushes them in the other order)
                  extra_pairs={MUSIC_CLOSE: 0x1002e983, 0x1017b8f8: 0x1014a59c},
                  name='Sim_1003a052_v12')]


def lm(log_o, log_c):
    """the shutdown gets the address of the device global: 1.2's own 0x107edeb8, logged as 1.0's 0x109e369c"""
    out = []
    for e in log_o:
        if e[0] == 'stub_1003a548' and e[1] == (A12(0x107edeb8),):
            e = (e[0], (0x109e369c,))
        out.append(e)
    return out, log_c


SPECS[0]['log_map'] = lm
