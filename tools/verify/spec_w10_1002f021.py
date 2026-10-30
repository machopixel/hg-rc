# Round 10: 0x1002f021 (point in polygon). Game logic: exact, no tolerance. Many trials with points near the
# vertices and edges, shared coordinates (the "equal" paths), and some NaN/odd floats.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1002/w10_misc.c'])


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx)
    n = r.choice([0, 1, 2, 3, 3, 4, 4, 5, 6, 8])
    nv = 12
    verts = newbuf(ctx, 0x58 * nv)
    grid = r.random() < 0.4
    pts = []
    for k in range(nv):
        if grid:
            vx, vy = float(r.randrange(-3, 4)), float(r.randrange(-3, 4))
        else:
            vx, vy = r.uniform(-10, 10), r.uniform(-10, 10)
        pts.append((vx, vy))
        w32(ctx, verts + k * 0x58 + 0x10, fb(vx) if r.random() < 0.97 else rfloat(r))
        w32(ctx, verts + k * 0x58 + 0x14, fb(vy) if r.random() < 0.97 else rfloat(r))
    poly = newbuf(ctx, 0x20)
    w16(ctx, poly + 2, n)
    w32(ctx, poly + 4, 0x10)
    wb(ctx, poly + 0x10, bytes(r.randrange(0, nv) for _ in range(8)))
    if grid:
        px, py = float(r.randrange(-3, 4)), float(r.randrange(-3, 4))
        if r.random() < 0.5:
            px += r.choice([0.0, 0.5, -0.5, 1e-8])
    else:
        px, py = r.uniform(-10, 10), r.uniform(-10, 10)
    return [poly, verts, fb(px) if r.random() < 0.98 else rfloat(r), fb(py) if r.random() < 0.98 else rfloat(r)]


SPECS = [dict(name='Sim_1002f021', orig=0x1002f021, c='Sim_1002f021', gen=g, trials=6000)]
