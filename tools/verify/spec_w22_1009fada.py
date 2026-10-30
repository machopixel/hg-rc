# Round 22: 0x1009fada, the spark ring of a polygon (DECISIONS.md round 22). The random source 0x1009c929, the line
# drawer 0x100a2b8c and the renderer slot [0x109e3d30] (a stub in int3 padding) log their arguments; the renderer
# stub logs the eight spark vertices and the texture record whole. The original never writes the record's second
# dword or the vertices' alpha bytes: the spec gives its stack the values the C writes (0x40 and 0).
# Negative check: W22_NEG=1 leaves random bytes there instead, which the record / vertex logs must catch.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w22_poly.c'])
NEG = bool(os.environ.get('W22_NEG'))
RENDER = 0x1010d3e6
helper(RENDER, None, 5, deref={2: 8 * 0x24, 4: 8})
helper(0x1009c929, None, 1)
helper(0x100a2b8c, None, 3)
ACTIVE = (RENDER, 0x1009c929, 0x100a2b8c)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, 0x109e3d30, RENDER)
    n = r.choice([1, 2, 3, 4, 5, 8])
    verts = newbuf(ctx, 0x24 * 8)
    for i in range(n):
        v = verts + i * 0x24
        wf(ctx, v, r.choice([r.uniform(-50, 700), 0.0, float('nan')] if r.random() < 0.05 else [r.uniform(-50, 700)]))
        wf(ctx, v + 4, r.uniform(-50, 500))
    rets(0x1009c929, *[r.randrange(0, 100) for _ in range(r.choice([1, 3, 5, 7]))])
    w32(ctx, 0x1017d110, r.choice([0, 1, 7, r.getrandbits(31)]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    ebp = frame(2)
    if NEG:
        wb(ctx, ebp - 0x1234, rb(ctx, 0x1234))
    else:
        wb(ctx, ebp - 0x1210, bytes(8 * 0x24))
        w32(ctx, ebp - 0x1230, 0x40)
    return [n, verts]


SPECS = [dict(name='Sim_1009fada', orig=0x1009fada, c='Sim_1009fada', gen=g, void=True, trials=300 if NEG else 4000)]
