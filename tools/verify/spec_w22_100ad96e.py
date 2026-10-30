# Round 22: 0x100ad96e, one entry of the draw-item table 0x101c57e0 (DECISIONS.md round 22). Every callee is a
# logging stub: the projection Sim_100d2669 logs the position it gets and answers a depth through its z pointer, the
# sprite and model drawers and 0x100585c4 / 0x100ac9ab log their arguments. No local is read before it is written.
# Negative check: W22_NEG=1 makes the projection answer a different depth in the C run (the second run of a trial),
# which must change the drawer chosen.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w22_items.c'])
NEG = bool(os.environ.get('W22_NEG'))
helper(0x100d2669, 'Sim_100d2669', 3, deref={0: 4, 1: 4, 2: 4})
helper(0x100ad5b7, 'Sim_100ad5b7', 4)
helper(0x100ad87d, 'Sim_100ad87d', 2)
helper(0x100585c4, None, 1)
helper(0x100ac9ab, None, 1)
ACTIVE = (0x100d2669, 0x100ad5b7, 0x100ad87d, 0x100585c4, 0x100ac9ab)
DEPTHS = [0.5, 1.0, 2.0, -3.0, float('nan'), 1e30]


def _project(uc, k, raw, eax):
    seen = ST['logs_seen']
    if not seen or seen[-1] is not ST['log']:
        seen.append(ST['log'])
    z = ST['depth_neg'] if NEG and len(seen) == 2 else ST['depth']
    uc.mem_write(raw[2], struct.pack('<f', z))
    return None


EFFECT[0x100d2669] = _project


def g(ctx):
    r = begin(ctx, *ACTIVE)
    count = r.choice([0, 1, 4, 8])
    w32(ctx, 0x1017d1c8, count)
    idx = r.choice([-1, 0, 1, 3, 7, 8, count, r.randrange(0, 8)])
    w32(ctx, 0x1014d1d8, r.choice([0, 1, 1, 1]))
    w32(ctx, 0x1017b8c8, r.randrange(0, 4))
    w32(ctx, 0x1015ef0c, r.choice([0, 1]))
    wf(ctx, 0x1012bab0, r.choice([1.0, 0.5, 2.0]))
    for i in range(8):
        item = 0x101c57e0 + i * 0x34
        wb(ctx, item, rb(ctx, 0x34))
        w32(ctx, item + 4, r.choice([1, 1, 2, 6, 0, 3]))
        obj = newbuf(ctx, 0x70)
        w32(ctx, obj, r.choice([1, 9, 1, 0, 2]))
        w32(ctx, obj + 4, r.randrange(0, 4))
        for o in (0x58, 0x5c, 0x60):
            wf(ctx, obj + o, r.uniform(-100, 100))
        model = newbuf(ctx, 0x90)
        w32(ctx, model + 0x8c, r.choice([0, newbuf(ctx, 0x60)]))
        w32(ctx, obj + 0x48, model)
        w32(ctx, item + 8, obj)
        for o in (0x14, 0x18, 0x1c):
            wf(ctx, item + o, r.uniform(-50, 50))
    ST['depth'] = r.choice(DEPTHS)
    ST['logs_seen'] = []
    ST['depth_neg'] = -ST['depth'] if ST['depth'] == ST['depth'] else 5.0
    rets(0x100585c4, r.choice([0, 1, 0xffffffff]))
    if NEG:                         # every trial on the projection path, so the moved depth always counts
        w32(ctx, 0x1017d1c8, 8)
        w32(ctx, 0x1014d1d8, 1)
        w32(ctx, 0x1015ef0c, 0)
        idx = r.randrange(0, 8)
        item = 0x101c57e0 + idx * 0x34
        w32(ctx, item + 4, 1)
        obj = r32(ctx, item + 8)
        w32(ctx, obj, 1)
        w32(ctx, r32(ctx, obj + 0x48) + 0x8c, newbuf(ctx, 0x60))
        ST['depth'], ST['depth_neg'] = r.choice([(0.5, 2.0), (2.0, 0.5)])
    return [idx]


SPECS = [dict(name='Sim_100ad96e', orig=0x100ad96e, c='Sim_100ad96e', gen=g, void=True, trials=300 if NEG else 4000)]
