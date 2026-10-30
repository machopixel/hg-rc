# Round 10: 0x1002c1f9 (per-frame cockpit update). Stubbed on both sides: Sim_1006561a, the camera 0x100d1f90 (patched
# by hooks_view.c), Sim_10069f68, Sim_1002c76d, Sim_100999bc, Sim_1009b99b, Sim_1009bc94, Sim_100622f0 (its seven
# arguments logged: the headings and differences as float words) and the panel routines (IND1 = +0x13c, IND2 =
# +0x138, IND3 = +0x140, the panel logged). The CRT fmod and Sim_10084a94 run for real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1002/w10_misc.c'])
for _a, _c, _n in ((0x1006561a, 'Sim_1006561a', 0), (0x100d1f90, None, 1), (0x10069f68, 'Sim_10069f68', 1),
                   (0x1002c76d, 'Sim_1002c76d', 1), (0x100999bc, 'Sim_100999bc', 0), (0x1009b99b, 'Sim_1009b99b', 0),
                   (0x1009bc94, 'Sim_1009bc94', 0), (0x100622f0, 'Sim_100622f0', 7)):
    helper(_a, _c, _n)
for _a in (IND1, IND2, IND3):
    helper(_a, None, 1)


def e_panel(uc, k, raw, eax):
    # the main panel's routine may switch the cockpit off ([0x10144fa0] = 0), which the original tests again
    if ST.get('clear') and k == 0:
        uc.mem_write(0x10144fa0, bytes(4))
    return None


EFFECT[IND1] = e_panel


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def ang(r):
    return fb(r.choice([r.uniform(-720, 720), 0.0, 180.0, -180.0, 360.0, 179.999, 540.0])) if r.random() < 0.95 else rfloat(r)


def g(ctx):
    r = begin(ctx, 0x1006561a, 0x100d1f90, 0x10069f68, 0x1002c76d, 0x100999bc, 0x1009b99b, 0x1009bc94, 0x100622f0,
              IND1, IND2, IND3)
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    unit = newbuf(ctx, 0x100)
    w32(ctx, unit + 4, r.choice([self_, self_, self_ + 1]))
    for off in (0x68, 0xec, 0xe8):
        w32(ctx, unit + off, ang(r))
    for off in (0x58, 0x5c, 0x60, 0xdc, 0xe0, 0xe4):
        w32(ctx, unit + off, fb(r.uniform(-1000, 1000)))
    obj = newbuf(ctx, 0x600)
    w32(ctx, obj, unit)
    w32(ctx, obj + 0xc, ang(r))
    w32(ctx, obj + 0x1c, ang(r))
    w32(ctx, obj + 0x4f0, r.choice([0, 1, 2, 2, 3, 4, 5, 6]))
    w16(ctx, obj + 0x560, r.getrandbits(16))
    w32(ctx, 0x10144fa0, r.choice([1, 1, 1, 0]))
    ST['clear'] = r.random() < 0.2
    w32(ctx, 0x1017e618, r.choice([0, 1]))
    w32(ctx, 0x1015dd10, r.choice([0, 1]))
    w32(ctx, 0x1015f1d8, r.choice([0, 1]))
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    rp = newbuf(ctx, 0x10)
    w8(ctx, rp + 9, r.choice([0, 1]))
    w32(ctx, 0x1017b8c4, rp)
    w32(ctx, 0x1015ef08, r.getrandbits(32))
    main = newbuf(ctx, 0x150)
    w32(ctx, main + 0x13c, IND1 if r.random() < 0.7 else 0)
    w32(ctx, 0x109e8920, main)
    for i in range(30):
        p = newbuf(ctx, 0x150)
        w32(ctx, p + 0x13c, IND1 if r.random() < 0.5 else 0)
        w32(ctx, p + 0x138, IND2 if r.random() < 0.5 else 0)
        w32(ctx, p + 0x140, IND3 if r.random() < 0.5 else 0)
        w32(ctx, 0x109e88e0 + 4 * i, p)
    return [obj]


SPECS = [dict(name='Sim_1002c1f9', orig=0x1002c1f9, c='Sim_1002c1f9', gen=g, void=True, trials=3000)]
