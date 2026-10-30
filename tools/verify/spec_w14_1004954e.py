# Round 14: 0x1004954e (the sim's command dispatcher). Every callee is stubbed on both sides: sprintf (copies its
# format, which is the message: the shipped messages have no '%'), Sim_1007df6c (text logged), Sim_10077fd8,
# Sim_1000ebfb, the camera / pane / order helpers, FlashText_Start, Sim_1008b922 (key compared as 16 bits, the
# original pushes ax) and the sound method [0x109e3d40]. Sim_10098e62 is logged without its argument (the C one takes
# none). The messages are the shipped English defaults (DECISIONS.md, round 14), including the 43-character network
# message that runs 4 bytes past the original's 40-byte buffer.
# Negative check: W14_NEG=1 makes the auto-eject / time compression messages 44 characters long; the original then
# overwrites its throttle local [ebp-0x18] and must mismatch.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1004/w14_keys.c'])
NEG = bool(os.environ.get('W14_NEG'))
CALLS = [
    (0x100fc780, None, 2), (0x1007df6c, 'Sim_1007df6c', 4), (0x10077fd8, 'Sim_10077fd8', 1),
    (0x1000ebfb, 'Sim_1000ebfb', 5), (0x10077fff, 'Sim_10077fff', 0), (0x10077d50, 'Sim_10077d50', 0),
    (0x1002c10e, 'Sim_1002c10e', 0), (0x100448ff, 'Sim_100448ff', 0), (0x1009b2eb, 'Sim_1009b2eb', 0),
    (0x100448cd, 'Sim_100448cd', 1), (0x10044a69, 'Sim_10044a69', 2), (0x1009b2fc, 'Sim_1009b2fc', 0),
    (0x1008b922, 'Sim_1008b922', 1), (0x10010951, 'Sim_10010951', 1), (0x10010a73, 'Sim_10010a73', 0),
    (0x10010b0b, 'Sim_10010b0b', 0), (0x10010b38, 'Sim_10010b38', 0), (0x10010b65, 'Sim_10010b65', 0),
    (0x10010b92, 'Sim_10010b92', 0), (0x10010aa0, 'Sim_10010aa0', 0), (0x10010acd, 'Sim_10010acd', 0),
    (0x10010aec, 'Sim_10010aec', 0), (0x1002590c, 'Sim_1002590c', 0), (0x10078177, 'Sim_10078177', 0),
    (0x10077ecd, 'Sim_10077ecd', 0), (0x100ca96a, 'Sim_100ca96a', 0), (0x1009b2b4, 'Sim_1009b2b4', 0),
    (0x1009b3a1, 'Sim_1009b3a1', 1), (0x100654a1, 'FlashText_Start', 1), (0x1009b362, 'Sim_1009b362', 0),
    (0x10074bfa, 'Sim_10074bfa', 1), (0x10074c3d, 'Sim_10074c3d', 1), (0x10098e62, 'Sim_10098e62', 0),
    (0x10015c61, 'Sim_10015c61', 1), (0x10015cd2, 'Sim_10015cd2', 1), (0x10052aaf, 'Sim_10052aaf', 1),
    (0x1006abf6, 'Sim_1006abf6', 3), (0x100d6392, 'Sim_100d6392', 1), (0x1003cbb3, 'Sim_1003cbb3', 2),
    (0x100203cf, 'Sim_100203cf', 2), (0x10019670, 'Sim_10019670', 0), (0x100b8ff9, 'Sim_100b8ff9', 1),
    (IND1, None, 2),
]
for a, c, n in CALLS:
    helper(a, c, n)
SARGS[0x1007df6c] = {0}
MASK[(0x1008b922, 0)] = 0xffff
ACTIVE = tuple(a for a, _, _ in CALLS)

MSG = {0x101c45c0: b'Waiting for players...Function deactivated.', 0x101c45bc: b'Saving HUD configuration',
       0x101c45b4: b'Automatic ejection OFF', 0x101c45b0: b'Automatic ejection ON',
       0x101c45ac: b'Time compression enabled', 0x101c45a8: b'Time compression disabled'}


def _sprintf(uc, k, raw, eax):
    fmt = _cstr(uc, raw[1])
    assert b'%' not in fmt
    uc.mem_write(raw[0], fmt + b'\0')
    return len(fmt)


EFFECT[0x100fc780] = _sprintf

CMDS = list(range(0, 0xd0)) + [0xd0, 0xff, 0x100, 0x15, 0x17, 0x18, 0x10032, 0x1003b, 0x80000000]


def text(ctx, s):
    a = newbuf(ctx, len(s) + 1)
    wb(ctx, a, s + b'\0')
    return a


def g(ctx):
    r = begin(ctx, *ACTIVE)
    rets(0x10015c61, r.choice([0xffffffff, 0, 1, 2, 3]), r.randrange(0, 4), r.choice([0xffffffff, 1, 3]))
    rets(0x10015cd2, r.randrange(0, 8), r.randrange(0, 8))
    rets(0x10052aaf, r.randrange(0, 4), r.randrange(0, 4))
    for a in (0x100448ff, 0x1009b2eb, 0x1002590c, 0x10077ecd):
        rets(a, r.choice([0, 0, 1]), r.choice([0, 1]))
    player = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, player)
    for i in range(4):
        rec = newbuf(ctx, 0x40)
        unit = newbuf(ctx, 0xc50)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
        w32(ctx, rec + 0x28, unit)
        w16(ctx, rec + 0x1c, r.choice([0, 2, 0x2000, 0x2002, r.getrandbits(16)]))
        w32(ctx, rec + 0x14, r.choice([0, 1, 1, 2]))
        for off in (0xc3d, 0xbb3, 0xbab, 0xbaf):
            w32(ctx, unit + off, r.choice([0, 0, 1, r.getrandbits(32)]))
        w32(ctx, unit + 0x4f0, r.choice([0, 3, 7, 3, 7, 1]))
    pane = newbuf(ctx, 0x40)
    w32(ctx, 0x109e88e8, pane)
    w32(ctx, pane + 0x30, r.choice([0, 1, 2, 3, 5, 7, r.getrandbits(32)]))
    for a in (0x109e890c, 0x109e894c, 0x109e8950, 0x109e8954, 0x109e8938, 0x109e8928):
        w32(ctx, a, r.getrandbits(32))
    for a, s in MSG.items():
        if NEG and a in (0x101c45b0, 0x101c45b4, 0x101c45a8, 0x101c45ac):
            s = b'Automatic ejection is now switched on, sir!!'     # 44 characters: past [ebp-0x18]
        elif r.random() < 0.2:
            s = s[:r.randrange(0, len(s) + 1)]
        w32(ctx, a, text(ctx, s))
    w32(ctx, 0x109b98c0, r.choice([0, r.getrandbits(32)]))
    w32(ctx, 0x109e3d40, IND1)
    for a in (0x1015ec58, 0x1015f1e8, 0x1015f1ec, 0x1017d108, 0x109b9e08):
        w32(ctx, a, r.choice([0, 0, 1, r.getrandbits(32)]))
    w32(ctx, 0x1017b94c, r.getrandbits(32))
    for a in (0x10144fa0, 0x1017e5e0, 0x10151d54):
        w32(ctx, a, r.choice([0, 1, 2, 3, 0x7fffffff, 0xffffffff, r.getrandbits(32)]))
    for a in (0x1015d1ac, 0x1014cb1c):
        w32(ctx, a, r.choice([0, 1, 4, r.getrandbits(32)]))
    for a in (0x10144fac, 0x10151cf8, 0x1013a008, 0x1015e550, 0x101452d0, 0x1015dc8c):
        w32(ctx, a, r.choice([0, 1, r.getrandbits(32)]))
    w8(ctx, 0x1015fd91, r.choice([0, 1, 0xff]))
    w8(ctx, 0x1015fd8e, r.getrandbits(8))
    w32(ctx, 0x101115f0, r.choice([0x3dcccccd, 0x3e124925, rfloat(r)]))
    cmd = r.choice(CMDS) if r.random() < 0.6 else r.choice(list(range(0x5d, 0x64)) + [0x54, 0xb0, 0xc4, 0xc5, 0x14, 0x59, 0x5a, 0x2b])
    if NEG:
        cmd = r.choice([0x54, 0xb0])
        w32(ctx, 0x1015ec58, 0)
        w32(ctx, 0x1015f1ec, 1)
        w16(ctx, r32(ctx, 0x109e8b40 + 4 * player) + 0x1c, 0x2000)
    return [cmd | (r.getrandbits(16) << 16 if r.random() < 0.1 else 0)]


SPECS = [dict(name='Sim_1004954e', orig=0x1004954e, c='Sim_1004954e', gen=g, void=True, trials=300 if NEG else 12000)]
