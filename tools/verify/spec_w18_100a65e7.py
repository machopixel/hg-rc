# Round 18: 0x100a65e7, a model polygon clipped and handed to the renderer (DECISIONS.md round 18). The original's
# never-written [ebp-0x78] (copied into the edge pointer before the table is chosen) gets random leftovers: the C has
# no such copy, so equal results show the leftover is never used. Callees (all converted) are logging stubs:
# Sim_100a5c10, Sim_100a5d25, Render_ClipEdgeNear (0x100a5d38), Sim_100a6009, Sim_100a8943 (returns a draw record);
# the renderer callbacks [0x1014cb44] (IND1: appends a vertex with a random depth to the list 0x101c76a0, sometimes
# clears [0x101cc2a0]) and [0x1014cb48] (IND2) too. Mem_Move runs for real.
# Negative check: W18_NEG=1 patches the original's "mov [ebp-0x2c], eax" at 0x100a6a26 to NOPs, so the leftover copied
# at 0x100a695f becomes the edge pointer the first clipped edge uses: mismatches show the checker would see the
# leftover if any path used it.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w18_poly.c'])
NEG = bool(os.environ.get('W18_NEG'))
for a, c, n in [(0x100a5c10, 'Sim_100a5c10', 2), (0x100a5d25, 'Sim_100a5d25', 1), (0x100a5d38, 'Render_ClipEdgeNear', 4),
                (0x100a6009, 'Sim_100a6009', 2), (0x100a8943, 'Sim_100a8943', 0)]:
    helper(a, c, n)
helper(IND1, None, 1)
helper(IND2, None, 4)
ACTIVE = (0x100a5c10, 0x100a5d25, 0x100a5d38, 0x100a6009, 0x100a8943, IND1, IND2)
CB = {}


def _out(uc, k, raw, eax):
    n = ur32(uc, 0x101c9ea8)
    if n < 0x100:
        rec = CB['vrec'][k % len(CB['vrec'])]
        uc.mem_write(0x101c76a0 + 4 * n, struct.pack('<I', rec))
        uc.mem_write(0x101c9ea8, struct.pack('<I', n + 1))
    if CB['clear'].get(k):
        uc.mem_write(0x101cc2a0, b'\0')
    return None


EFFECT[IND1] = _out


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    m = ctx.m
    rec = newbuf(ctx, 0x10)
    rets(0x100a8943, rec)
    rets(IND2, *[r.getrandbits(32) for _ in range(3)])
    CB['vrec'] = []
    for _ in range(6):
        b = newbuf(ctx, 0x10)
        w32(ctx, b + 8, r.choice([fb(r.uniform(0, 5000)), fb(r.uniform(-10, 10)), rfloat(r)]))
        CB['vrec'].append(b)
    CB['clear'] = {k: r.random() < 0.1 for k in range(12)}
    w32(ctx, 0x1014cb44, IND1)
    w32(ctx, 0x1014cb48, IND2)
    nv = 8
    verts = newbuf(ctx, nv * 0x58)
    for i in range(nv):
        v = verts + i * 0x58
        for off in (0xc, 0x10, 0x14):
            w32(ctx, v + off, fb(r.uniform(-3000, 3000)) if r.random() < 0.95 else rfloat(r))
        w8(ctx, v + 0x30, r.choice([0, 0, 1, 2, 4, 5, 6, 0x10, r.getrandbits(8)]))
    count = r.choice([3, 3, 4, 4, 5, 2, 1, 6])
    poly = newbuf(ctx, 0x30 + count)
    w16(ctx, poly, r.choice([0xb000, 0xc000, 0xd000, 0xe000, 0x1000, 0x0000, 0xa000, 0xf000]) | r.getrandbits(12))
    w16(ctx, poly + 2, count)
    w32(ctx, poly + 4, 0x30)
    w32(ctx, poly + 8, r.getrandbits(32))
    for off in (0x18, 0x1c, 0x20):
        w32(ctx, poly + off, fb(r.uniform(-1, 1)) if r.random() < 0.95 else rfloat(r))
    wb(ctx, poly + 0x30, [r.randrange(nv) for _ in range(count)])
    for a in (0x10185c44, 0x10185c48, 0x10185c4c):
        w32(ctx, a, fb(r.uniform(-1000, 1000)))
    for i in range(9):
        w32(ctx, 0x10185ba0 + 4 * i, fb(r.uniform(-1, 1)))
    w32(ctx, 0x10185c50, fb(r.uniform(0, 50)))
    w32(ctx, 0x10185bf8, fb(r.uniform(500, 4000)))
    w32(ctx, 0x1017d14c, r.choice([0, 0, 1]))
    w32(ctx, 0x101cc2b0, r.randrange(8))
    dst = newbuf(ctx, 0x400)
    w32(ctx, 0x1017d178, dst)
    w32(ctx, 0x101c6740, dst)
    q = newbuf(ctx, 0x80)
    w32(ctx, 0x101cc2ac, q)
    w32(ctx, 0x1017d138, r.randrange(0, 8))
    w32(ctx, 0x1017d17c, r.randrange(0, 10))
    w32(ctx, 0x1017d12c, r.getrandbits(32))
    for a in (0x101cc2a4, 0x101c767c, 0x101cc29c, 0x101c9e9c, 0x1017d13c, 0x101c672c, 0x101c9ea4):
        w32(ctx, a, r.getrandbits(16))
    w32(ctx, frame(2) - 0x78, r.getrandbits(32))
    if NEG:
        m.raw.mem_write(0x100a6a26, bytes([0x90, 0x90, 0x90]))
    return [poly, verts]


SPECS = [dict(name='Sim_100a65e7', orig=0x100a65e7, c='Sim_100a65e7', gen=g, void=True, trials=400 if NEG else 20000)]
