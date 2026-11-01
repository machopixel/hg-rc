# Round 32, F: 1.2's beams against 1.2's own code: railgun (spawn 0x1007a0a6, update 0x10079f7d, draw 0x1007a092),
# accelerator (0x1007acf3, 0x1007a26c, 0x1007a379) and laser (0x1007b956, 0x1007aec2, 0x1007afdc), and the quad
# helpers 0x10086750 / 0x10086810. Stubbed on both sides (w32_common.py): the pool allocation, the node calls, the
# projection (answers visible or not, writes nothing), the random ring, the frame setter, the polygon routine (logs the
# four corners), the impact 0x100246b3 (1.2's 0x1001b3a1, its changed version: the arguments are compared) and the
# sound start / stop (by name).
# Negative check: W32_NEG=1 builds the C with W32B_MUTATE (the outer quad one step thinner) and W32X_MUTATE.
import os
exec(open('tools/verify/w32_common.py').read())

SOURCES = closure(['sim/fx/w32_beam.c', 'sim/fx/w32_family.c', 'sim/fx/w32_partspec.c', 'sim/fx/w32_draw.c'])
CFLAGS = list(CFLAGS) + (['-DW32B_MUTATE', '-DW32X_MUTATE'] if os.environ.get('W32_NEG') else [])
DRV = 0x1010d3e6
helper(DRV, None, 3, deref={1: 4 * 0x24})
stub12(0x10090c7d, 'Sim_10090c7d', 4, 0x10079c54)
stub12(0x100d2669, 'Sim_100d2669', 3, 0x100b87f9)
stub12(0x100aa39f, 'Sim_100aa39f', 1, 0x1009582f)
stub12(0x100a9d47, 'Sim_100a9d47', 4, 0x100951d7)
stub12(0x100aa799, 'Sim_100aa799', 1, 0x10095c29)
stub12(0x100aa351, 'Sim_100aa351', 1, 0x100957e1)
stub12(0x1009c929, 'Sim_1009c929', 1, 0x10089e69)
stub12(0x1000716a, 'Sim_1000716a', 2, 0x1000204c)
stub12(0x100246b3, None, 12, 0x1001b3a1)
stub12(0x1000ec60, 'Sim_1000ec60', 5, 0x10008788, sargs=(0,))
stub12(0x1000ecb0, 'Sim_1000ecb0', 1, 0x100087d8, sargs=(0,))
MASK[(0x1000716a, 1)] = 0xffff
FUNCS.update({0x10079f7d: 'Sim12_10079f7d', 0x1007a092: 'Sim12_1007a092', 0x1007a26c: 'Sim12_1007a26c',
              0x1007a379: 'Sim12_1007a379', 0x1007aec2: 'Sim12_1007aec2', 0x1007afdc: 'Sim12_1007afdc'})
GLOBALS.append((0x1014cb4c, 0x1011fa54, 4))        # the polygon routine's slot
EFFECT[0x1009c929] = e_rand
ALL = (DRV, 0x10090c7d, 0x100d2669, 0x100aa39f, 0x100a9d47, 0x100aa799, 0x100aa351, 0x1009c929, 0x1000716a,
       0x100246b3, 0x1000ec60, 0x1000ecb0)


def world(ctx, r):
    ST['seed'] = r.getrandbits(16)
    w32(ctx, 0x1014cb4c, DRV)
    painter_slot(ctx)
    w32(ctx, 0x1015dc78, r.getrandbits(16))
    w32(ctx, 0x1017b940, 1 if r.random() < 0.9 else 0)
    w32(ctx, ctx.m.syms['g_w31Tex'], 1)
    w32(ctx, 0x1017cc20, r.choice([1000, 1000, 5]))
    w32(ctx, 0x1017cc24, r.choice([0, 3, 4, 999]))
    for t in (0, 2, 3):
        type_row(ctx, t)
    rets(0x100d2669, *[r.choice([0, 1, 1, 1]) for _ in range(5)])


def make_rec(ctx, r, t):
    rec = newbuf(ctx, 0x680)
    node = newbuf(ctx, 0x100)
    block = newbuf(ctx, 0x60)
    shot = newbuf(ctx, 0x40)
    w32(ctx, node + 0x8c, block)
    w32(ctx, rec + 0x332, node)
    w32(ctx, rec + 0x366, shot)
    w32(ctx, rec + 0x402, t)
    w16(ctx, rec + 0x10, r.choice([0, 1, 4, 5, 2, 0xffff]))
    w32(ctx, rec + 0x3fa, r.choice([0xffffffff, 0, 1, 4, 31, 40]))
    w32(ctx, rec + 0x33e, r.getrandbits(16))
    w32(ctx, rec + 0x342, r.choice([0, 5, 30, 0x7fffffff]))
    w32(ctx, rec + 0x346, r.choice([0, 1, 0xfe, 0x1f, r.getrandbits(32)]))
    pts = [r.uniform(-700, 700), r.uniform(-500, 500), r.uniform(1, 5000)]
    pts2 = [r.choice([pts[0], r.uniform(-700, 700)]), r.choice([pts[1], r.uniform(-500, 500)]), r.uniform(1, 5000)]
    if r.random() < 0.15:
        pts2[0] = pts[0] + (pts2[1] - pts[1])       # |dx| = |dy|
    wf(ctx, rec + 0x12, *(pts + pts2))
    return rec


def g_spawn(ctx, t):
    r = begin(ctx, *ALL)
    world(ctx, r)
    rets(0x10090c7d, make_rec(ctx, r, t) if r.random() < 0.9 else 0)
    shot = newbuf(ctx, 0x40)
    w32(ctx, shot + 0x30, r.getrandbits(16))
    w32(ctx, shot + 0x34, r.getrandbits(16))
    if t == 3:
        return [r.getrandbits(32), shot, r.getrandbits(32)]
    if t == 2:
        return [r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)]
    return [r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)]


def g_rec(ctx, t):
    r = begin(ctx, *ALL)
    world(ctx, r)
    rec = make_rec(ctx, r, t)
    w32(ctx, 0x1015dc78, (r32(ctx, rec + 0x33e) + r.choice([0, 5, 30, 31, 100])) & 0xffffffff)
    return [0] if r.random() < 0.02 else [rec]


def g_quad(ctx):
    r = begin(ctx, *ALL)
    world(ctx, r)
    v = newbuf(ctx, 4 * 0x24)
    return [v] + [fb(r.uniform(-700, 700)) for _ in range(8)] + [r.getrandbits(32), r.choice([0, 3, 31, 0x10000]),
                                                                 r.choice([0x10, 0x1ff, 0x200, 0x7123, 0x80000000])]


SPECS = [spec12('Sim12_1007a0a6', 0x1007a0a6, lambda c: g_spawn(c, 0), 2000, void=True),
         spec12('Sim12_10079f7d', 0x10079f7d, lambda c: g_rec(c, 0), 2000),
         spec12('Sim12_1007a092', 0x1007a092, lambda c: g_rec(c, 0), 300),
         spec12('Sim12_1007acf3', 0x1007acf3, lambda c: g_spawn(c, 2), 2000, void=True),
         spec12('Sim12_1007a26c', 0x1007a26c, lambda c: g_rec(c, 2), 2000),
         spec12('Sim12_1007a379', 0x1007a379, lambda c: g_rec(c, c.rng.choice([2, 3])), 4000),
         spec12('Sim12_1007b956', 0x1007b956, lambda c: g_spawn(c, 3), 2000, void=True),
         spec12('Sim12_1007aec2', 0x1007aec2, lambda c: g_rec(c, 3), 2000),
         spec12('Sim12_1007afdc', 0x1007afdc, lambda c: g_rec(c, c.rng.choice([2, 3])), 1000),
         spec12('Sim12_10086810', 0x10086810, g_quad, 2000, void=True)]
