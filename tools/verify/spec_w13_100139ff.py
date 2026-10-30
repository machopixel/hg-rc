# Round 13: 0x100139ff (per-frame steering by the order word +0x1a6). Stubs on both sides: Sim_1001403a,
# Sim_100c264a, Sim_10079feb, Sim_10017b3e, Sim_10017aa9, Sim_100c2ae5, Sim_1000ae00, Sim_100178f4, Sim_10015a63,
# Sim_10033c42, Sim_100c45dc, Sim_10015630, and the float answers Sim_1001412d, Sim_100144e9, Sim_10015266.
# Sim_10014061 (the limit) and the DLL's fmod run for real.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1001/w13_steer.c'])
helper(0x1001403a, 'Sim_1001403a', 2)
helper(0x100c264a, 'Sim_100c264a', 1)
helper(0x10079feb, 'Sim_10079feb', 1)
helper(0x10017b3e, 'Sim_10017b3e', 2)
helper(0x10017aa9, 'Sim_10017aa9', 1)
helper(0x100c2ae5, 'Sim_100c2ae5', 1)
helper(0x1000ae00, 'Sim_1000ae00', 2)
helper(0x100178f4, 'Sim_100178f4', 1)
helper(0x10015a63, 'Sim_10015a63', 1)
helper(0x10033c42, 'Sim_10033c42', 1)
helper(0x100c45dc, 'Sim_100c45dc', 3)
helper(0x10015630, 'Sim_10015630', 4)
FSTUB[0x1001412d] = ('Sim_1001412d', 2)
FSTUB[0x100144e9] = ('Sim_100144e9', 1)
FSTUB[0x10015266] = ('Sim_10015266', 1)
MASK[(0x1001403a, 1)] = 0xffff         # the key is pushed as ax: Sim_1001403a takes a 16-bit value
ACTIVE = (0x1001403a, 0x100c264a, 0x10079feb, 0x10017b3e, 0x10017aa9, 0x100c2ae5, 0x1000ae00, 0x100178f4,
          0x10015a63, 0x10033c42, 0x100c45dc, 0x10015630)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    ST['fvals'] = {a: [fb(r.choice([0.0, 0.5, -3.0, r.uniform(-500, 500)])) for _ in range(3)]
                   for a in (0x1001412d, 0x100144e9, 0x10015266)}
    rets(0x10079feb, r.choice([0, 1, 1]))
    rets(0x100c2ae5, r.choice([0, 0, 1]))
    rets(0x10033c42, r.choice([0, 0x00c00000, 0x00c00000]))
    rets(0x100c45dc, r.choice([0, 1, 2, 2, 3]))
    unit = newbuf(ctx, 0x280)
    rec = newbuf(ctx, 0x580)
    block = newbuf(ctx, 0x60)
    w32(ctx, unit + 0x28, rec)
    w32(ctx, unit + 0x54, block)
    w32(ctx, unit, r.choice([1, 1, 2, 4, 8]))
    w32(ctx, unit + 0xc, r.randrange(0, 4))
    w32(ctx, unit + 0x14, r.choice([0, 1, 2, 3, 3]))
    w32(ctx, 0x1017b8c8, r.randrange(0, 4))
    w16(ctx, unit + 0x1a6, r.choice(list(range(0, 18)) + [2, 4, 5, 6, 13, 14, 15, 15, 0xffff, 0x8002]))
    w32(ctx, unit + 0xe8, r.choice([0, fb(r.uniform(-400, 400)), fb(180.0), fb(r.uniform(-1e6, 1e6))]))
    w16(ctx, rec + 0x562, r.choice([0, 1, 2, 0xffff]))
    w32(ctx, unit + 0x23a, r.choice([0, 3, -1 & 0xffffffff, 0x80000000, 7]))
    for i in range(4):
        w32(ctx, 0x109a8650 + 0x54 * i, r.choice([fb(r.uniform(0, 3000)), fb(50.0), 0xffc00000]))
    key = r.choice([0x100, 0x200, 0x400, 0, 0x300]) | r.randrange(0, 4) | r.choice([0, 0x1000, 0x8000])
    return [unit, key | (r.getrandbits(16) << 16)]


SPECS = [dict(name='Sim_100139ff', orig=0x100139ff, c='Sim_100139ff', gen=g, void=True, trials=4000)]
