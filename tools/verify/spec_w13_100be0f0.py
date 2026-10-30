# Round 13: 0x100be0f0 (order step of a unit by its state word +0x1f8). Every callee is stubbed on both sides, with
# its arguments logged; the key word is pushed with a leftover upper half, so it is compared as 16 bits.
# Sim_1009c929 answers per call (first the 0x61 roll, then the 0x40 pick).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_100b/w13_orders.c'])
KEYED = (0x100c0670, 0x100c07c5, 0x100c0965, 0x100c1c67, 0x100c0dd7, 0x100c0a2d, 0x100c0b1d, 0x100c0ac4,
         0x100c0bdd, 0x100c0fc4, 0x100c0d9d, 0x100c1049, 0x100c1169, 0x100c2279, 0x100c1e69, 0x100c209c,
         0x100c145b, 0x100c1609, 0x1001403a)
PLAIN = (0x100bf186, 0x100bf28e, 0x100c264a, 0x1001570f, 0x100c3d16, 0x100c2ae5, 0x100c12d7, 0x100c17b7,
         0x100c18c8, 0x100c03d8, 0x100c0423, 0x100c0471, 0x100c0558, 0x100c3eb5, 0x1009c929, 0x100bfa33)
for a in KEYED:
    helper(a, 'Sim_%08x' % a, 2)
    MASK[(a, 1)] = 0xffff
for a in PLAIN:
    helper(a, 'Sim_%08x' % a, 1)
helper(0x100c4f6c, 'Sim_100c4f6c', 0)
helper(0x100c00c7, 'Sim_100c00c7', 4)
MASK[(0x100c00c7, 2)] = 0xffff         # the slot word is pushed as dx
ACTIVE = KEYED + PLAIN + (0x100c4f6c, 0x100c00c7)
T = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def _roll(uc, k, raw, eax):
    return T['roll'][min(k, 1)]


EFFECT[0x1009c929] = _roll


def g(ctx):
    r = begin(ctx, *ACTIVE)
    for a in ACTIVE:
        rets(a, r.choice([0, 0, 1, 1, -1 & 0xffffffff, 2, r.getrandbits(32)]))
    rets(0x100bf186, r.choice([0xffff, 0xffffffff, 0, 3, 0x1b, 0x12345]))
    rets(0x100c3d16, r.choice([0, 2, -2 & 0xffffffff, 0xffff0004, 6]))
    rets(0x1001403a, r.choice([1, 1, 0, 2]))
    rets(0x100c4f6c, r.choice([-1 & 0xffffffff, -1 & 0xffffffff, 0, 5]))
    T['roll'] = [r.choice([0, 1, 2, 3, 5]), r.randrange(0, 64)]
    unit = newbuf(ctx, 0x280)
    rec = newbuf(ctx, 0x580)
    block = newbuf(ctx, 0x60)
    w32(ctx, unit + 0x28, rec)
    w32(ctx, unit + 0x54, block)
    w32(ctx, unit, r.choice([1, 1, 2, 4]))
    w16(ctx, unit + 0x1f8, r.choice(list(range(0, 28)) + [1, 1, 3, 3, 14, 0xffff, 0xffff, 0xfffe, 0x8000, 0x1c, 40]))
    lead = r.randrange(0, 4)
    w16(ctx, unit + 0x1aa, lead | r.choice([0, 0x100, 0x8200]))
    for i in range(4):
        w32(ctx, 0x109e8b40 + 4 * i, newbuf(ctx, 0x300))
    w32(ctx, unit + 0x218, r.choice([0, 1, 2, 3, -1, -2, -7, 8]) & 0xffffffff)
    w32(ctx, unit + 0x21c, r.choice([0, -1 & 0xffffffff, 1]))
    clock = r.choice([100, 1000, 0x7fffff00])
    w32(ctx, 0x1015dc78, clock)
    w32(ctx, unit + 0x204, (clock + r.choice([-5, 0, 5, 600])) & 0xffffffff)
    w32(ctx, unit + 0x200, r.choice([0, 0, clock - 1, clock, clock + 1]))
    w32(ctx, unit + 0x1fc, r.choice([-1 & 0xffffffff, -1 & 0xffffffff, 0, 7]))
    w32(ctx, unit + 0x23a, r.getrandbits(32))
    w32(ctx, rec + 0x510, r.choice([0, 1, -1 & 0xffffffff, 50]))
    w32(ctx, block + 0xc, r.choice([0, fb(r.uniform(-5000, 5000)), fb(1e-3)]))
    return [unit, r.getrandbits(32)]


SPECS = [dict(name='Sim_100be0f0', orig=0x100be0f0, c='Sim_100be0f0', gen=g, void=True, trials=6000)]
