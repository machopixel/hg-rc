# Round 22: 0x1009ef50, the display driver's polygon routine (DECISIONS.md round 22). Every callee is a logging stub:
# the renderer slot [0x109e3d30] (int3 padding) logs the vertices and the texture record whole, the graphics library
# 0x100e4780 / 0x100e5477 and 0x100a2d91 log the software vertices they get, the others their arguments. The
# software vertex buffers 0x101ce4a0 / 0x101ccca0 and the plugin vertices are compared as memory.
# The original never writes its texture record ([ebp-0x3c], and [ebp-0x38] on the 0x1000 / 0xa000 paths) nor its
# local [ebp-4] that it pushes as the fourth argument of 0x10006609: the spec gives the record the C's 0 and masks
# that argument (0x10006609 reads three).
# Negative check: W22_NEG=1 leaves random bytes in the original's record, which the record log must catch.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w22_poly.c', 'sim/render/w22_items.c'])
NEG = bool(os.environ.get('W22_NEG'))
RENDER = 0x1010d3e6
helper(RENDER, None, 5, deref={2: 8 * 0x24, 4: 8})
helper(0x10090f17, None, 1)
helper(0x100ad96e, 'Sim_100ad96e', 1)
helper(0x100a2d91, None, 2)
helper(0x100e4780, None, 3, deref={2: 8 * 0x18})
helper(0x100e5477, None, 4, deref={3: 8 * 0x18})
helper(0x10098f30, None, 4)
helper(0x10006744, None, 4)
helper(0x10006609, None, 7)
helper(0x100a2b8c, None, 3)
MASK[(0x10006609, 3)] = 0
ACTIVE = (RENDER, 0x10090f17, 0x100ad96e, 0x100a2d91, 0x100e4780, 0x100e5477, 0x10098f30, 0x10006744, 0x10006609,
          0x100a2b8c)
KINDS = [0, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000, 0x8000, 0x9000, 0xa000, 0xb000, 0xc000, 0xd000,
         0xe000, 0xf000]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, 0x109e3d30, RENDER)
    n = r.choice([0, 1, 2, 3, 4, 5, 8])
    verts = newbuf(ctx, 0x24 * 8)
    for i in range(8):
        v = verts + i * 0x24
        wf(ctx, v, r.choice([r.uniform(-100, 700), r.uniform(-20000, 20000), float('nan')] if r.random() < 0.1
                            else [r.uniform(-100, 700)]))
        wf(ctx, v + 4, r.choice([r.uniform(-100, 500), r.uniform(-20000, 20000)]))
        w32(ctx, v + 0x10, r.choice([r.randrange(0, 0x30), r.getrandbits(32), r.randrange(0, 256)]))
        wf(ctx, v + 0x1c, r.uniform(-1, 2))
    flags = r.choice(KINDS) | r.getrandbits(12)
    for a in (0x1017b940, 0x1017b948, 0x1017d108, 0x1014caf4, 0x1014cb34):
        w32(ctx, a, r.choice([0, 1]))
    for a in (0x10143fdc, 0x10143fe0, 0x10143fe4):
        w32(ctx, a, r.choice([0x100, 0x80, 0xc0, 0x1ff]))
    wf(ctx, 0x1012ba10, r.choice([0.5, 1.0, -5.0, 100.0]))
    wf(ctx, 0x1012ba14, r.choice([255.0, 3.0]))
    wb(ctx, 0x1012ba18, struct.pack('<d', r.choice([255.0, 1.5])))
    wb(ctx, 0x1012ba20, struct.pack('<d', r.choice([0.0, 0.5])))
    rets(0x100a2d91, *[r.choice([0, 2, 3, 5]) for _ in range(2)])
    ebp = frame(3)
    if NEG:
        wb(ctx, ebp - 0x3c, rb(ctx, 8))
    else:
        wb(ctx, ebp - 0x3c, bytes(8))
    return [n, verts, flags]


SPECS = [dict(name='Sim_1009ef50', orig=0x1009ef50, c='Sim_1009ef50', gen=g, void=True, trials=300 if NEG else 6000)]
