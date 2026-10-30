# Round 12: 0x100c2ae5 (contact test of a moving unit). Stubs on both sides: Sim_100c31a6 (the probe), the finders
# Sim_1002dda9 / Sim_100c2851 (they write the thing found only when the trial says so), Sim_10079feb, Sim_100c292c,
# Sim_100c312c, Sim_100c3419, Sim_100c3372, Sim_10015266 (a float answer), Sim_10083495 and Sim_100c275b. sqrt, fabs
# and Math_ClampSymmetric_100c351c run for real. The thing found [ebp-0xc], read unset when only Sim_100c292c
# reports a hit, gets the chosen start value 0 as the original's stack leftover.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_100c/w12_contact.c'])
helper(0x100c31a6, 'Sim_100c31a6', 6)
helper(0x1002dda9, 'Sim_1002dda9', 3)
helper(0x100c2851, 'Sim_100c2851', 4)
helper(0x10079feb, 'Sim_10079feb', 1)
helper(0x100c292c, 'Sim_100c292c', 2)
helper(0x100c312c, 'Sim_100c312c', 1)
helper(0x100c3419, 'Sim_100c3419', 7)
helper(0x100c3372, 'Sim_100c3372', 5)
helper(0x10083495, 'Sim_10083495', 0)
helper(0x100c275b, 'Sim_100c275b', 1)
FSTUB[0x10015266] = ('Sim_10015266', 1)
MASK[(0x100c31a6, 3)] = 0xffff         # the probe number is pushed as cx: Sim_100c31a6 takes a 16-bit value
ACTIVE = (0x100c31a6, 0x1002dda9, 0x100c2851, 0x10079feb, 0x100c292c, 0x100c312c, 0x100c3419, 0x100c3372, 0x10083495,
          0x100c275b)
EBP = frame(1)
T = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def _finder(uc, k, raw, eax):
    if T['write'][k % len(T['write'])]:
        uc.mem_write(raw[1], struct.pack('<I', T['found'][k % len(T['found'])]))
    return None


EFFECT[0x1002dda9] = _finder
EFFECT[0x100c2851] = _finder


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    ST['fvals'] = {0x10015266: [fb(r.choice([0.0, 0.5, 1.0, 2.0, -3.0, r.uniform(-5, 5)]))]}
    T['write'] = [r.random() < 0.5 for _ in range(6)]
    T['found'] = [r.choice([0, 0, r.getrandbits(32), 0x00c00000]) for _ in range(6)]
    rets(0x1002dda9, *[r.choice([0, 0, 0, 1]) for _ in range(5)])
    rets(0x100c2851, *[r.choice([0, 0, 0, 1]) for _ in range(5)])
    rets(0x10079feb, r.choice([0, 0, 1]))
    rets(0x100c292c, *[r.choice([0, 1, 1]) for _ in range(5)])
    rets(0x100c312c, *[r.choice([0, 0, 1]) for _ in range(3)])
    rets(0x100c3419, r.choice([0, 1, 0xffff, 0x10002]))
    rets(0x100c3372, r.choice([0, 1, 0xffff, 3]))
    unit = newbuf(ctx, 0x280)
    rec = newbuf(ctx, 0x580)
    block = newbuf(ctx, 0x60)
    w32(ctx, unit + 0x28, rec)
    w32(ctx, unit + 0x54, block)
    w32(ctx, unit, r.choice([1, 4, 6, 8, 9, 10, 2, 0, 11]))
    w32(ctx, unit + 4, r.randrange(0, 4))
    w32(ctx, 0x1017b8c8, r.randrange(0, 4))
    w32(ctx, rec + 0x4d4, r.choice([0, fb(1.0), fb(r.uniform(-5, 5))]))
    vmax = r.choice([30, 1, 0.01])
    w32(ctx, rec + 0x548, *[fb(r.uniform(-vmax, vmax)) for _ in range(3)])
    w16(ctx, rec + 0x562, r.choice([0, 1, 0xfffe]))
    w32(ctx, block + 8, r.choice([0, fb(r.uniform(-2, 2))]))
    w8(ctx, block + 0x59, r.choice([0, 0, 0, 1]))
    w16(ctx, unit + 0x25e, r.choice([0, 0, 1, -1 & 0xffff, 2]))
    w32(ctx, unit + 0x25a, r.choice([0, 5, 100]))
    w32(ctx, 0x1015dc78, r.choice([10, 10, 1000, -1 & 0xffffffff]))
    w32(ctx, unit + 0x24a, r.choice([0, r.getrandbits(32)]))
    w32(ctx, unit + 0x242, r.choice([0, fb(r.uniform(-500, 3000)), fb(1e-8)]))
    w32(ctx, unit + 0x24e, *[fb(r.uniform(-100, 100)) for _ in range(3)])
    w32(ctx, unit + 0x58, *[fb(r.uniform(-1000, 1000)) for _ in range(3)])
    w32(ctx, unit + 0x260, fb(r.uniform(0, 3)))
    w32(ctx, unit + 0x218, r.getrandbits(32))
    w32(ctx, EBP - 0xc, 0)                   # the chosen start value as the original's leftover
    return [unit]


SPECS = [dict(name='Sim_100c2ae5', orig=0x100c2ae5, c='Sim_100c2ae5', gen=g, trials=6000)]