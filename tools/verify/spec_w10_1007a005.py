# Round 10: 0x1007a005 (is a side's own entity near / inside a row's target, third form). Game logic exact. Stubbed on both
# sides: Sim_1000ebfb (sound; name logged as text) . Sim_10079feb, Sim_100b53f0
# run for real. Generator as for 0x1007a4af (spec_w10d.py); the side's dword 0x109b7d40 + side * 0x6c picks its entity.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1007/w10d_zones.c'])
helper(0x1000ebfb, 'Sim_1000ebfb', 5, sargs=(0,))


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def fl(r, c, sc):
    return fb(c + r.uniform(-sc, sc)) if r.random() < 0.97 else rfloat(r)


# ---- 0x1007a4af: is a side near / inside a row's target -------------------------------------------------
def g(ctx):
    r = begin(ctx, 0x1000ebfb)
    side = r.randrange(0, 4)
    w32(ctx, 0x109b7d40 + side * 0x6c, r.choice([r.randrange(0, 13)] * 5 + [0x80000000, 0xffffffff]))
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
        e = newbuf(ctx, 0x170)
        ents.append(e)
        w32(ctx, 0x109e8b40 + 4 * i, e)
        w32(ctx, e, r.choice([5, 8, 1, 2, 9]))
        w32(ctx, e + 8, r.choice([side, side, side, side + 1, r.getrandbits(32)]))
        w16(ctx, e + 0x1c, r.choice([0, 0, 0, 2, 4, 6, 0x2000, 0x2000, 0x2004, r.getrandbits(16)]))
        w32(ctx, e + 0x58, fl(r, cx, spread), fl(r, cy, spread), fl(r, cz, spread))
        w32(ctx, e + 8, r.choice([side, r.randrange(0, 40), r.getrandbits(32)]))
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



SPECS = [dict(name='Sim_1007a005', orig=0x1007a005, c='Sim_1007a005', gen=g, trials=5000)]

