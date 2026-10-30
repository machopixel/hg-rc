# Round 12: 0x100648a0 and 0x10064d05 (HUD markers). Stubs on both sides: Sim_100d2669 (projects in place: writes
# the trial's screen point, answers on / off screen), Sim_1008e266 (writes the clipped point), Sim_10063415,
# Sim_10065216 (name logged). Sim_1000946e and Str_Copy run for real. The original's palette [ebp-0x14], unset on
# screen for a kind other than 0..2, gets the chosen start value 0 as its stack leftover.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w12_markers.c'])
helper(0x100d2669, 'Sim_100d2669', 3)
helper(0x1008e266, 'Sim_1008e266', 3)
helper(0x10063415, 'Sim_10063415', 5)
helper(0x10065216, 'Sim_10065216', 4, sargs=(2,))
ACTIVE = (0x100d2669, 0x1008e266, 0x10063415, 0x10065216)
T = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def _proj(uc, k, raw, eax):
    for a, v in zip(raw, T['proj']):
        uc.mem_write(a, struct.pack('<I', v))
    return T['on']


def _clip(uc, k, raw, eax):
    uc.mem_write(raw[2], struct.pack('<ii', *T['clip']))
    return raw[2]


EFFECT[0x100d2669] = _proj
EFFECT[0x1008e266] = _clip


def common(ctx, r):
    T['on'] = r.choice([0, 1])
    T['proj'] = [fb(r.uniform(-50, 700)), fb(r.uniform(-50, 500)), r.choice([fb(r.uniform(0.5, 900)), fb(-3.0), 0])]
    edge = r.randrange(100, 700)
    w32(ctx, 0x109e3e0c, edge)
    T['clip'] = [r.choice([0, edge + 1, r.randrange(1, 700)]), r.choice([0, r.randrange(1, 500)])]
    rets(0x10063415, r.choice([0, 1, 1]))
    for k in range(8):
        s = newbuf(ctx, 16)
        wb(ctx, s, b'HOLF%d\0' % k)
        w32(ctx, 0x109e85a0 + 4 * k, s)
    cam = newbuf(ctx, 0x100)
    w32(ctx, cam + 0x94, fb(r.uniform(100, 600)))
    w32(ctx, 0x1014cae0, cam)
    w32(ctx, 0x1015dc38, r.choice([0, 2, 8, 50]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    return r.choice([0, 1, 2, 3, -1, 7]) & 0xffffffff


def g_648a0(ctx):
    r = begin(ctx, *ACTIVE)
    kind = common(ctx, r)
    unit = newbuf(ctx, 0x70)
    rec = newbuf(ctx, 0x580)
    w32(ctx, unit + 0x28, rec)
    w32(ctx, unit + 4, r.getrandbits(32))
    w32(ctx, unit + 0x58, fb(r.uniform(-1000, 1000)), fb(r.uniform(-50, 300)), fb(r.uniform(-1000, 1000)))
    w16(ctx, rec + 0x562, r.choice([0, 8, 0xffff, 0x10]))
    w32(ctx, rec + 0x520, fb(r.uniform(0, 30)))
    w32(ctx, rec + 0x53c, fb(r.uniform(0, 40)))
    w32(ctx, frame(2) - 0x14, 0)             # the chosen start value as the original's leftover
    return [unit, kind]


def g_64d05(ctx):
    r = begin(ctx, *ACTIVE)
    kind = common(ctx, r)
    thing = newbuf(ctx, 0x90)
    model = newbuf(ctx, 0x60)
    w32(ctx, model + 0x44, fb(r.uniform(-1000, 1000)), fb(r.uniform(-50, 300)), fb(r.uniform(-1000, 1000)),
        fb(r.uniform(0, 40)))
    w32(ctx, thing + 0x8c, r.choice([model, model, model, 0]))
    w32(ctx, frame(3) - 0x14, 0)
    return [r.getrandbits(32), r.choice([thing, thing, thing, 0]), kind]


SPECS = [dict(name='Sim_100648a0', orig=0x100648a0, c='Sim_100648a0', gen=g_648a0, void=True, trials=4000),
         dict(name='Sim_10064d05', orig=0x10064d05, c='Sim_10064d05', gen=g_64d05, void=True, trials=4000)]