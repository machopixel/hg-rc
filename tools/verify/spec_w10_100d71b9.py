# Round 10, interpreted (DECISIONS.md): 0x100d71b9 (aim a node along a direction, optional random spread). Stubbed on
# both sides: Sim_100aa0da (rotation), Sim_100d709d (writes the start position), Sim_1009c929, Sim_10084c95 (its three
# floats logged, writes the normalised ones), Sim_100a9d47, Sim_100a9d7c, and the float stub Sim_1008486e. The CRT asin
# of the original against the platform math library of the C (libm()): exact but for the pitch float passed to
# Sim_100aa0da, which may differ within the stated tolerance. dy outside asin's domain checks the EDOM wrapper.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/util/w10_interp.c'])
for _a, _c, _n in ((0x100aa0da, 'Sim_100aa0da', 5), (0x100d709d, 'Sim_100d709d', 5), (0x1009c929, 'Sim_1009c929', 1),
                   (0x10084c95, 'Sim_10084c95', 3), (0x100a9d47, 'Sim_100a9d47', 4), (0x100a9d7c, 'Sim_100a9d7c', 4)):
    helper(_a, _c, _n)
DEREF[0x10084c95] = {0: 4, 1: 4, 2: 4}
FSTUB.update({0x1008486e: ('Sim_1008486e', 2)})
ACTIVE = tuple(HELPERS)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_pos(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[1 + i], struct.pack('<I', ST['pos'][i]))


def e_norm(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[i], struct.pack('<I', ST['norm'][i]))


EFFECT.update({0x100d709d: e_pos, 0x10084c95: e_norm})


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    libm(ctx)
    rets(0x1009c929, r.randrange(0, 200))
    ST['fvals'] = {0x1008486e: [fb(r.uniform(-180, 180))]}
    ST['pos'] = [fb(r.uniform(-3000, 3000)) for _ in range(3)]
    ST['norm'] = [fb(r.uniform(-1, 1)) for _ in range(3)]
    node = newbuf(ctx, 0x100)
    blk = newbuf(ctx, 0x60)
    w32(ctx, blk + 0x50, fb(r.uniform(0, 500)))
    w32(ctx, node + 0x8c, blk)
    weapon = newbuf(ctx, 0x10)
    w32(ctx, weapon + 8, r.choice([0, 0x40, r.getrandbits(32)]))
    dy = r.choice([fb(r.uniform(-1, 1)), fb(r.uniform(-0.01, 0.01)), fb(1.0), fb(-1.0), 0, fb(r.uniform(0.99, 1)), fb(r.uniform(1, 3)), fb(-r.uniform(1, 3)), 0x7fc00000])
    return [newbuf(ctx, 0x40), node, fb(r.uniform(-1, 1)), dy, fb(r.uniform(-1, 1)), r.choice([0, 1]), weapon,
            newbuf(ctx, 0x40)]


SPECS = [dict(name='Sim_100d71b9', orig=0x100d71b9, c='Sim_100d71b9', gen=g, void=True, trials=15000, fp=True,
              tol=1.2e-7)]
