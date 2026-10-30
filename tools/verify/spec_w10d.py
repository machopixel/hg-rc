# Round 10, helper d. Game logic: exact, no tolerance.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1007/w10d_zones.c', 'sim/unsorted_100c/w10d_targets.c'])

helper(0x1000ebfb, 'Sim_1000ebfb', 5, sargs=(0,))
helper(0x100cbc6b, 'Sim_100cbc6b', 7)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def ring(ctx, r):
    """The random ring of Sim_1009c929 (filled at run time in the game)."""
    w32(ctx, 0x1017cd38, r.randrange(0, 0x7f))
    w32(ctx, 0x101cffc0, *[r.randrange(0, 0x8000) for _ in range(0x7f)])


def fl(r, c, sc):
    return fb(c + r.uniform(-sc, sc)) if r.random() < 0.97 else rfloat(r)


# ---- 0x1007a4af: is a side near / inside a row's target -------------------------------------------------
def g_1007a4af(ctx):
    r = begin(ctx, 0x1000ebfb)
    side = r.randrange(0, 4)
    w32(ctx, 0x109b7d40 + side * 0x6c, r.choice([0, 5, 0x80000000, 0xffffffff]))
    w32(ctx, 0x1015ec58, r.choice([0, 0, 1]))
    kind = r.choice([1, 2, 4, 4, 1, 2, 3, 0])
    index = r.randrange(0, 12)
    n = r.choice([0, 1, 2, 3, 5, 8, 12])
    w32(ctx, 0x1015dc64, n)
    w32(ctx, 0x1017b8c8, r.randrange(0, 13))
    cx, cy, cz = (r.uniform(-30000, 30000) for _ in range(3))
    if kind == 2:
        rad = 4000.0
    elif kind == 4:
        rad = 20000.0
    else:
        rad = r.choice([20000.0, 3000.0, 500.0])
    spread = rad * r.choice([0.4, 1.0, 1.4, 3.0])
    ents = []
    for i in range(13):
        e = newbuf(ctx, 0x80)
        ents.append(e)
        w32(ctx, 0x109e8b40 + 4 * i, e)
        w32(ctx, e, r.choice([5, 8, 1, 2, 9]))
        w32(ctx, e + 8, r.choice([side, side, side, side + 1, r.getrandbits(32)]))
        w16(ctx, e + 0x1c, r.choice([0, 0, 0, 2, 4, 6, 0x2000, 0x2000, 0x2004, r.getrandbits(16)]))
        w32(ctx, e + 0x58, fl(r, cx, spread), fl(r, cy, spread), fl(r, cz, spread))
    if kind == 2:
        me = ents[index]
        w32(ctx, me + 0x58, fb(cx), fb(cy), fb(cz))
    elif kind == 4:
        rec = 0x109e9ba0 + index * 0x50
        ix = r.choice([r.randrange(0, 6), -1, 9])
        w32(ctx, rec + 8, ix)
        w32(ctx, 0x1017D218, 6)
        if 0 <= ix <= 6:
            ent = 0x101A2100 + ix * 0x88
            blk = newbuf(ctx, 0x80)
            w32(ctx, blk + 0x44, fb(cx), fb(cy), fb(cz))
            w32(ctx, blk + 0x70, fb(cx), fb(cy), fb(cz))
            c = r.random()
            w32(ctx, ent + 0x28, blk if c < 0.5 else 0)
            w32(ctx, ent + 0x2c, blk if c < 0.8 else 0)
    elif kind == 1:
        rec = 0x109a8640 + index * 0x54
        w32(ctx, rec, r.choice([0, 1, r.getrandbits(32)]))
        w32(ctx, rec + 0x10, r.choice([fb(rad), fb(rad), 0, fb(-5.0), 0x80000000, 0x7fc00000]))
        w32(ctx, rec + 0x18, fb(cx), fb(cy), fb(cz))
        w16(ctx, rec + 0x24, r.choice([0, 0, 0x20, r.getrandbits(16)]))
        w16(ctx, rec + 0x26, r.getrandbits(16))
    ident = newbuf(ctx, 2)
    w8(ctx, ident, index, kind)
    return [ident, side]


# ---- 0x100cada2: can who target index ------------------------------------------------------------------
# Sim_100cbc6b (angles of the offset) is stubbed on both sides: dx, dy, dz logged, it writes four chosen floats.
def e_cbc6b(uc, k, raw, eax):
    for i in range(4):
        uc.mem_write(raw[3 + i], struct.pack('<I', ST['ang'][i]))
    return None


EFFECT[0x100cbc6b] = e_cbc6b


def node(ctx, r):
    n = newbuf(ctx, 0x90)
    w32(ctx, n + 0x70, fl(r, 0, 3000), fl(r, 0, 300), fl(r, 0, 3000))
    w32(ctx, n + 0x8c, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    return n


def g_100cada2(ctx):
    r = begin(ctx, 0x100cbc6b)
    ST['ang'] = [fl(r, 0, 360), r.choice([fb(r.uniform(0, 200000)), fb(r.uniform(0, 80000)), fb(175000.0),
                                          fb(15000.0), fb(75000.0), rfloat(r)]), fl(r, 0, 90), fl(r, 0, 90)]
    n = 8
    w32(ctx, 0x1015dc64, n)
    me = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, me)
    w32(ctx, 0x1017e5b0, r.choice([0, 1]))
    ring(ctx, r)
    for side in range(6):
        w32(ctx, 0x109B7D54 + side * 0x6c, r.choice([0, 1, 2]))
    ents = []
    for i in range(n):
        e = newbuf(ctx, 0x200)
        ents.append(e)
        w32(ctx, 0x109e8b40 + 4 * i, e)
        w32(ctx, e, r.choice([1, 1, 2, 5]))
        w32(ctx, e + 4, r.choice([r.randrange(0, 8), 99]))
        w32(ctx, e + 8, r.randrange(0, 6))
        w32(ctx, e + 0xc, r.choice([me, me, r.randrange(0, 8)]))
        w32(ctx, e + 0x14, r.choice([2, 3, 3, 1, 0]))
        w16(ctx, e + 0x1c, r.choice([0x400, 0x1000, 0x1400, 0x40, 0x440, 0x1440, 0, 0x10, 2, 0x800, 0x1c00,
                                     r.getrandbits(16)]))
        w16(ctx, e + 0x1a6, r.choice([2, 3, 1, 0]))
        w32(ctx, e + 0x1f0, r.choice([0, 1, 1]))
        rec = newbuf(ctx, 0xc00)
        w32(ctx, e + 0x28, rec)
        w16(ctx, rec + 0x562, r.choice([0, 8, r.getrandbits(16)]))
        w32(ctx, rec + 0x520, fl(r, 0, 50))
        w32(ctx, rec + 0x494, r.choice([0, node(ctx, r)]))
        w32(ctx, rec + 0xbdf, r.choice([0, 1, 2, 3, 4, 5, 9, -2, r.getrandbits(32)]))
        w32(ctx, rec + 0xbe3, r.choice([0, 0, 1, 2, -1]))
        w32(ctx, e + 0x48, r.choice([0, node(ctx, r), node(ctx, r), node(ctx, r)]))
        w32(ctx, e + 0x4c, r.choice([0, node(ctx, r)]))
        w32(ctx, e + 0x58, fl(r, 0, 3000), fl(r, 0, 300), fl(r, 0, 3000))
    who = r.choice([me, me, me, r.randrange(0, 8)])
    index = r.choice([r.randrange(0, 8)] * 6 + [-1, 8, 0x7fffffff])
    flags = r.choice([0, 0, 0, 0x10000, 0x20000, 0x40000, 0x60000, 1, 4, 0x10, 2, 0x100, r.getrandbits(32)])
    return [who, index, flags]


# round 24: the C-side copies of the values the original leaves on its stack (no counterpart in the DLL)
W24_KEPT = {'g_w24TargetDy': 4, 'g_w24TargetAngle': 4, 'g_w24TargetSet': 4}

SPECS = [
    dict(name='Sim_1007a4af', orig=0x1007a4af, c='Sim_1007a4af', gen=g_1007a4af, trials=4000),
    dict(name='Sim_100cada2', orig=0x100cada2, c='Sim_100cada2', gen=g_100cada2, trials=6000,
         ignore_syms=W24_KEPT),
]
