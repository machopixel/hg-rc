# Round 10: 0x1004f257 (pick a unit's animation). Stubbed on both sides: Sim_1004f1df (fall kind), Sim_1004f158,
# Sim_1004ee50 and Sim_1000ed52 (sound). Sim_1004f19d runs for real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1004/w10_anim.c'])
for _a, _c, _n in ((0x1004f1df, 'Sim_1004f1df', 1), (0x1004f158, 'Sim_1004f158', 1), (0x1004ee50, 'Sim_1004ee50', 1),
                   (0x1000ed52, 'Sim_1000ed52', 6)):
    helper(_a, _c, _n)
ACTIVE = (0x1004f1df, 0x1004f158, 0x1004ee50, 0x1000ed52)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    rets(0x1004f1df, r.choice([0, 1, 2, 3, 4, 5, -1 & 0xffffffff]))
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    cam = newbuf(ctx, 0x10)
    w32(ctx, cam, *[fb(r.uniform(-100, 100)) for _ in range(3)])
    w32(ctx, 0x1014cae0, cam)
    lims = [struct.unpack('<f', struct.pack('<I', r32(ctx, a)))[0] for a in
            (0x1012b40c, 0x1012b410, 0x1012b418, 0x1012b41c, 0x1012b420, 0x1012b424, 0x1012b428)]
    rec = newbuf(ctx, 0xc50)
    own = newbuf(ctx, 0x160)
    w32(ctx, own + 0x15c, r.choice([0, 0, 1]))
    w32(ctx, rec, own)
    lo, hi = min(lims), max(lims)
    base = r.choice(lims)
    tiers = lims[2:] + [lims[1]]
    w32(ctx, rec + 0x7c, fb(r.choice([lims[0] - 0.001, lims[0] + 1e-5, r.uniform(0, 0.04),
                                     lims[1] + r.choice(tiers) + r.choice([-1e-4, 1e-4, 0.0])])))
    w16(ctx, rec + 0x562, r.choice([0, 0x800, 0x10, 4, 0x400, 1, 0x200, r.getrandbits(16)]))
    w32(ctx, rec + 0x5ff, r.choice([0x100, 0x200, 0]))
    w32(ctx, rec + 0xc3d, r.choice([0, 0, 1]))
    ctl = newbuf(ctx, 0x60)
    for off in (0x57, 0x58, 0x59):
        w8(ctx, ctl + off, r.choice([0, 0, 1]))
    obj = newbuf(ctx, 0xa0)
    w32(ctx, obj, r.choice([1, 4, 0xa, 5, 2]))
    w32(ctx, obj + 4, r.choice([self_, 9]))
    w32(ctx, obj + 0x28, rec)
    w32(ctx, obj + 0x54, ctl)
    w32(ctx, obj + 0x58, *[fb(r.uniform(-100, 100)) for _ in range(3)])
    w32(ctx, obj + 0x88, r.choice([8, 0, 9]))
    w32(ctx, obj + 0x8c, r.choice([0xe, 3, 0x10e]))
    return [obj]


SPECS = [dict(name='Sim_1004f257', orig=0x1004f257, c='Sim_1004f257', gen=g, void=True, trials=15000)]
