# Network round: the player lookup by network key (1.0 0x1003db56, Sim_1003db56) against 1.2's 0x100312c6, which looks
# at 10 keys (1.0: 8). The game record [0x1015d004] (1.2 0x107caa74): +0x16 points at the key words, +0x126 at the
# 0x1e-byte player records (word id at +0x1a); the enable flag [0x1015ec84] = 1.2 0x101314d0. Keys repeat at times,
# so the first match wins on both sides.
# Negative check: NETL_NEG12=1 builds the C with NETL_MUTATE (8 keys); a key found only at 8 or 9 then differs.
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1003/records.c'])
if os.environ.get('NETL_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DNETL_MUTATE']


def g(ctx):
    r = begin(ctx)
    keys = [r.choice([0, 1, 0xfa01, 0x1234, r.getrandbits(16)]) for _ in range(12)]
    root = newbuf(ctx, 0x200)
    kb = newbuf(ctx, 24)
    pb = newbuf(ctx, 12 * 0x1e)
    for i, k in enumerate(keys):
        w16(ctx, kb + 2 * i, k)
        w16(ctx, pb + i * 0x1e + 0x1a, r.getrandbits(16))
    w32(ctx, root + 0x16, kb)
    w32(ctx, root + 0x126, pb)
    w32(ctx, 0x1015d004, root)
    w32(ctx, 0x1015ec84, r.choice([0, 1, 1, 1, 0x100]))
    key = r.choice(keys + [r.getrandbits(16), keys[8], keys[9]])
    return [(key & 0xffff) | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0)]


SPECS = [v12_spec(dict(name='Sim_1003db56', orig=0x1003db56, c='Sim_1003db56', gen=g, trials=3000), 0x100312c6,
                  name='Sim_1003db56_v12')]
