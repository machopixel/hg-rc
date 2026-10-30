# Round 12: 0x10055048 (input polling). Stubs on both sides: the device methods (+0x10 IND1, +0x14 IND2, +0x18 IND3,
# which writes the trial's key word when it runs), Sim_1005436d, Sim_10054d98 and ToggleFullScreen. Without an open
# keyboard the original's key word [ebp-4] gets the chosen start value 0 as its stack leftover.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1005/w12_input.c'])
helper(IND1, None, 2)
helper(IND2, None, 3)
helper(IND3, None, 1)
helper(0x1005436d, 'Sim_1005436d', 2)
helper(0x10054d98, 'Sim_10054d98', 1)
helper(0x10037f71, 'ToggleFullScreen', 0)
ACTIVE = (IND1, IND2, IND3, 0x1005436d, 0x10054d98, 0x10037f71)
EBP = frame(0)
T = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def _key(uc, k, raw, eax):
    uc.mem_write(raw[0], struct.pack('<H', T['key']))
    return None


EFFECT[IND3] = _key


def g(ctx):
    r = begin(ctx, *ACTIVE)
    T['key'] = r.choice([0, 0x40d, 0x20, r.getrandbits(16)])
    rets(0x1005436d, *[r.choice([0, 1, 0x101, r.getrandbits(32)]) for _ in range(4)])
    rets(0x10054d98, *[r.choice([0, 0, 1]) for _ in range(3)])
    w32(ctx, 0x1015ec58, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b94c, r.getrandbits(32))
    ndev = r.randrange(0, 4)
    w32(ctx, 0x10164bc0, ndev)
    w32(ctx, 0x1015fdf4, r.randrange(0, 4))
    w32(ctx, 0x10151d50, r.choice([0xfffffffe, 0, 1]))
    vt = newbuf(ctx, 0x20)
    w32(ctx, vt + 0x10, IND1, IND2, IND3)
    for d in range(4):
        w32(ctx, 0x1015f868 + 4 * d, r.choice([0, 1, 1]))
        w32(ctx, 0x1015fd20 + 4 * d, vt)
        w32(ctx, 0x1015f8f0 + 0x74 * d, r.getrandbits(32))
    naxes = r.randrange(0, 4)
    w32(ctx, 0x1015fde8, naxes)
    for i in range(naxes):
        rr = 0x1015fe00 + 0x80 * i
        a = newbuf(ctx, 0x40)
        f = newbuf(ctx, 4)
        w32(ctx, f, fb(r.uniform(-2, 2)))
        p4 = newbuf(ctx, 4)
        w32(ctx, a, f)
        w32(ctx, a + 4, p4)
        w32(ctx, a + 8, r.choice([1, 2, 255, 0x10000]))
        w32(ctx, a + 0xc, r.choice([0, 128, r.randrange(-5000, 5000) & 0xffffffff]))
        w32(ctx, a + 0x10, r.getrandbits(32))
        w8(ctx, a + 0x14, r.choice([0, 1, 4, 0xff]))
        a1a = r.choice([0, 100, -100 & 0xffffffff, -0x30000 & 0xffffffff, 0x20000, r.getrandbits(32)])
        w32(ctx, a + 0x1a, a1a)
        w32(ctx, p4, r.choice([a1a, a1a, 0, 100, r.getrandbits(32)]))
        for off in (0x2a, 0x2e):
            s = newbuf(ctx, 4)
            w8(ctx, s, r.choice([0, 1]))
            w32(ctx, a + off, r.choice([0, s, s]))
        val = newbuf(ctx, 4)
        w32(ctx, val, r.choice([0, 1, -1 & 0xffffffff, 3, -50000 & 0xffffffff, 50000]))
        w32(ctx, rr, r.randrange(0, 4), r.randrange(0, 4), r.choice([0, val]), r.choice([0, 5, -3 & 0xffffffff]),
            r.choice([1, -1 & 0xffffffff, 1000]), a, r.randrange(0, 3))
        w32(ctx, rr + 0x7c, r.choice([0, 1]))
    nb = r.randrange(0, 4)
    w32(ctx, 0x1015fdf0, nb)
    for i in range(nb):
        b = 0x101620c8 + 0x6e * i
        flag = newbuf(ctx, 4)
        w8(ctx, flag, r.getrandbits(8))
        w32(ctx, b, flag)
        w8(ctx, b + 4, r.choice([0, 1]))
        w8(ctx, b + 5, r.choice([0, 1]))
        w32(ctx, b + 6, r.randrange(0, 3))
        w32(ctx, b + 0xa, r.choice([0, 1]))
    w32(ctx, 0x1013f140, r.choice([0, 1]))
    w32(ctx, 0x1013f144, r.randrange(0, 4))
    w32(ctx, 0x1013f148, r.randrange(0, 4))
    w32(ctx, 0x1013f14c, r.randrange(0, 4))
    cfg = newbuf(ctx, 0x70)
    w32(ctx, cfg + 0x64, r.choice([0, 1]))
    w32(ctx, 0x1015d1b8, cfg)
    w32(ctx, EBP - 4, 0)                     # the chosen start value as the original's leftover
    return []


SPECS = [dict(name='Sim_10055048', orig=0x10055048, c='Sim_10055048', gen=g, void=True, trials=15000)]