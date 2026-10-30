# Round 10: 0x1000c1ea (lead aim). Game logic: exact, no tolerance. Sim_100cbc6b (angles of a vector) is stubbed on
# both sides: its three floats and the pointer d logged, and it writes the same three angles through its pointers.
# The CRT fabs / sqrt run for real.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1000/w10_misc.c'])
helper(0x100cbc6b, 'Sim_100cbc6b', 7)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_ang(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[3 + i], struct.pack('<I', ST['ang'][i]))
    return None


EFFECT[0x100cbc6b] = e_ang


def fl(r, sc):
    return fb(r.uniform(-sc, sc)) if r.random() < 0.95 else rfloat(r)


def g(ctx):
    r = begin(ctx, 0x100cbc6b)
    ST['ang'] = [fb(r.choice([r.uniform(-400, 400), 180.0, -180.0, 179.99, -180.01, 540.0])), fl(r, 90), fl(r, 90)]
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    tid = r.randrange(0, 8)
    tgt = newbuf(ctx, 0x80)
    trec = newbuf(ctx, 0x560)
    w32(ctx, tgt + 0x28, trec)
    for off in (0x58, 0x5c, 0x60):
        w32(ctx, tgt + off, fl(r, 1000))
    for off in (0x548, 0x54c, 0x550):
        w32(ctx, trec + off, r.choice([fl(r, 30), fl(r, 30), 0, fb(5e-8)]))
    w32(ctx, 0x109e8b40 + 4 * tid, tgt)
    unit = newbuf(ctx, 0x200)
    w32(ctx, unit + 4, r.choice([self_, self_ + 1]))
    sel = r.choice([0x200 | tid, 0x200 | tid, 0x100 | tid, 0x300 | tid])
    w32(ctx, unit + 0xf0, sel)
    w16(ctx, unit + 0x1aa, sel)
    for off in (0x58, 0x5c, 0x60):
        w32(ctx, unit + off, fl(r, 1000))
    w32(ctx, unit + 0x68, fl(r, 360))
    w32(ctx, unit + 0xd8, fb(r.uniform(0, 3000)) if r.random() < 0.95 else rfloat(r))
    rec = newbuf(ctx, 0x500)
    w32(ctx, unit + 0x28, rec)
    slots = newbuf(ctx, 0x128 * 4)
    w32(ctx, rec + 0x84, slots)
    si = r.randrange(0, 4)
    w32(ctx, rec + 0x4fc, si)
    kind = r.randrange(0, 40)
    w32(ctx, slots + si * 0x128 + 4, kind)
    k = 0x10145508 + kind * 0xd6
    w32(ctx, k + 0x3c, r.choice([fb(r.uniform(0, 400)), fb(r.uniform(0, 40)), 0]))
    if r.random() < 0.2:      # shot speed exactly |v|: the t = 2.0 branch
        import math
        v = [struct.unpack('<f', bytes(ctx.m.uc.mem_read(trec + o, 4)))[0] for o in (0x548, 0x54c, 0x550)]
        if all(x == x and abs(x) < 1e30 for x in v):
            w32(ctx, k + 0x3c, fb(math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])))
    w32(ctx, k + 0x5c, r.choice([r.randrange(0, 200), 0, 1000]))
    w32(ctx, 0x1015dc80, r.choice([r.randrange(0, 200), 1000]))
    return [unit, newbuf(ctx, 4), newbuf(ctx, 4), newbuf(ctx, 12)]


SPECS = [dict(name='Sim_1000c1ea', orig=0x1000c1ea, c='Sim_1000c1ea', gen=g, void=True, trials=5000)]
