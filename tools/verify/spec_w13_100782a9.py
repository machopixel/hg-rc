# Round 13: 0x100782a9 (draw method of a view pane). Stubs on both sides: Sim_1009c929 (answers per call),
# Sim_100788ba, Sim_100b66a0, Sim_100aa351, Sim_100aa39f, Sim_100619a0 (the 7-dword camera copy it gets is logged by
# content), Sim_100788fc (label logged as text), Sim_1008d4f1, Sim_10099ef3, Sim_100258d7, Sim_1002590c, the HUD
# fill 0x100eb070, Sim_100c7deb, Sim_100cf96c and Sim_10044b2c (writes the trial's six values). Sim_10044992,
# Sim_100787f4 / Sim_10078857 and Mem_Move run for real.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1007/w13_views.c'])
helper(0x1009c929, 'Sim_1009c929', 1)
helper(0x100788ba, 'Sim_100788ba', 1)
helper(0x100b66a0, 'Sim_100b66a0', 0)
helper(0x100aa351, 'Sim_100aa351', 1)
helper(0x100aa39f, 'Sim_100aa39f', 1)
helper(0x100619a0, 'Sim_100619a0', 5, deref={2: 28})
helper(0x100788fc, 'Sim_100788fc', 3, sargs=(2,))
helper(0x1008d4f1, 'Sim_1008d4f1', 2)
helper(0x10099ef3, 'Sim_10099ef3', 1)
helper(0x100258d7, 'Sim_100258d7', 0)
helper(0x1002590c, 'Sim_1002590c', 0)
helper(0x100eb070, None, 2)
helper(0x100c7deb, 'Sim_100c7deb', 1)
helper(0x100cf96c, 'Sim_100cf96c', 1)
helper(0x10044b2c, 'Sim_10044b2c', 6)
ACTIVE = (0x1009c929, 0x100788ba, 0x100b66a0, 0x100aa351, 0x100aa39f, 0x100619a0, 0x100788fc, 0x1008d4f1,
          0x10099ef3, 0x100258d7, 0x1002590c, 0x100eb070, 0x100c7deb, 0x100cf96c, 0x10044b2c)
T = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def _roll(uc, k, raw, eax):
    return T['roll']


def _sel(uc, k, raw, eax):
    return T['sel'][min(k, 1)]


def _b2c(uc, k, raw, eax):
    for a, v in zip(raw, T['b2c']):
        uc.mem_write(a, struct.pack('<I', v))
    return None


EFFECT[0x1009c929] = _roll
EFFECT[0x100258d7] = _sel
EFFECT[0x10044b2c] = _b2c


def g(ctx):
    r = begin(ctx, *ACTIVE)
    T['roll'] = r.randrange(0, 10)
    selbuf = newbuf(ctx, 0x40)
    T['sel'] = [r.choice([0, 0, selbuf]), r.choice([0, selbuf])]
    T['b2c'] = [r.getrandbits(32) for _ in range(6)]
    pane = newbuf(ctx, 0x120)
    w16(ctx, pane + 4, r.choice([0, 1, 1, 1, 0x8000]))
    w16(ctx, pane + 6, r.choice([0, 1, 1, 2, 3, 0xffff]))
    w32(ctx, pane + 0x30, r.choice(list(range(0, 15)) + [3, 4, 5, 7, 3, 4, 5, 7, -1 & 0xffffffff, 0x80000003]))
    w32(ctx, pane + 0xc, r.getrandbits(32))
    w32(ctx, pane + 0xec, r.getrandbits(32))
    w32(ctx, 0x109e89d8, r.getrandbits(32))
    w32(ctx, 0x10179d24, r.choice([0, 1]))
    player = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, player)
    unit = newbuf(ctx, 0x40)
    rec = newbuf(ctx, 0x20)
    obj = newbuf(ctx, 0x80)
    w32(ctx, 0x109e8b40 + 4 * player, unit)
    w32(ctx, unit + 0x28, rec)
    w32(ctx, rec, obj)
    for off in range(0x48, 0x80, 4):
        w32(ctx, obj + off, fb(r.uniform(-1000, 1000)))
    cam = newbuf(ctx, 0x40)
    for off in range(0, 0x1c, 4):
        w32(ctx, cam + off, r.getrandbits(32))
    w32(ctx, 0x1014cae0, cam)
    for off in range(0, 0x68, 4):
        w32(ctx, 0x1014cae8 + off, r.getrandbits(32))
    w32(ctx, 0x1096fbd0, r.choice([0, 1]))
    flags = newbuf(ctx, 0x10)
    w8(ctx, flags + 9, r.choice([0, 1]))
    w32(ctx, 0x1017b8c4, flags)
    return [pane]


SPECS = [dict(name='Sim_100782a9', orig=0x100782a9, c='Sim_100782a9', gen=g, void=True, trials=5000)]
