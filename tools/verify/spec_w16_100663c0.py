# Round 16: 0x100663c0 (a unit's move with its collision tests). The segment helpers 0x1009cab0 / 0x1009cbde /
# 0x1009cbfe / 0x1009cd36, the distance 0x10084a94, fabs 0x100fcb70 and 0x1010d240 run for real. The three tests
# 0x10066907 (units), 0x1006754e (ground objects) and 0x1002e362 (world along a segment) are logging stubs that also
# do what the real ones can: write the hit (a gear with an id, an object with a box and a name), move the target
# position, set the push direction [0x1015dd58..60], and for 0x1002e362 shorten the segment (its end and length).
# The segment handed to 0x1002e362 is logged whole (0x38 bytes).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w16_range.c'])
CALLS = [(0x10066907, 'Sim_10066907', 5), (0x1006754e, 'Sim_1006754e', 5), (0x1002e362, 'Sim_1002e362', 2)]
for a, c, n in CALLS:
    helper(a, c, n)
DEREF[0x1002e362] = {0: 0x38}
ACTIVE = tuple(a for a, _, _ in CALLS)
T = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def fl(r, lo=-60.0, hi=60.0):
    if r.random() < 0.02:
        return r.choice([0x7fc00000, 0x7f800000, 0, 0x80000000])
    return fb(r.uniform(lo, hi))


def _move(uc, k, raw, xyz):
    if T['move'][k % len(T['move'])]:
        for i, p in enumerate(xyz):
            uc.mem_write(p, struct.pack('<I', T['f'][(k + i) % len(T['f'])]))


def _units(uc, k, raw, eax):
    if eax:
        uc.mem_write(raw[4], struct.pack('<I', T['gear']))
        _move(uc, k, raw, raw[1:4])
        for i, a in enumerate((0x1015dd58, 0x1015dd5c, 0x1015dd60)):
            uc.mem_write(a, struct.pack('<I', T['dir'][i]))


def _ground(uc, k, raw, eax):
    if eax:
        uc.mem_write(raw[4], struct.pack('<I', T['obj']))
        _move(uc, k, raw, raw[1:4])


def _world(uc, k, raw, eax):
    if eax:
        seg = raw[0]
        if T['shorten']:
            t = struct.unpack('<f', bytes(uc.mem_read(seg + 0x30, 4)))[0] * T['shorten']
            start = struct.unpack('<3f', bytes(uc.mem_read(seg, 12)))
            dirv = struct.unpack('<3f', bytes(uc.mem_read(seg + 0x24, 12)))
            end = [start[i] + dirv[i] * t for i in range(3)]
            uc.mem_write(seg + 0xc, struct.pack('<3f', *end))
            uc.mem_write(seg + 0x18, struct.pack('<3f', *[end[i] - start[i] for i in range(3)]))
            uc.mem_write(seg + 0x30, struct.pack('<f', t))
        uc.mem_write(raw[1], struct.pack('<I', T['wobj']))
        for i, a in enumerate((0x1015dd58, 0x1015dd5c, 0x1015dd60)):
            uc.mem_write(a, struct.pack('<I', T['dir'][i]))


EFFECT[0x10066907] = _units
EFFECT[0x1006754e] = _ground
EFFECT[0x1002e362] = _world


def g(ctx):
    r = begin(ctx, *ACTIVE)
    rets(0x10066907, r.choice([0, 0, 1]))
    rets(0x1006754e, r.choice([0, 0, 1]))
    rets(0x1002e362, r.choice([0, 1, 1]))
    T['move'] = [r.choice([0, 1]) for _ in range(3)]
    T['f'] = [fl(r) for _ in range(5)]
    T['shorten'] = r.choice([0, 0.25, 0.5, 0.999, 1.0])
    T['dir'] = r.choice([[0, 0, 0], [0, 0x80000000, 0], [fl(r, -1, 1), fl(r, -1, 1), fl(r, -1, 1)],
                         [0x7fc00000, 0, 0]])
    gear = newbuf(ctx, 0x40)
    T['gear'] = gear
    obj = newbuf(ctx, 0xb0)
    for off in (0x44, 0x48, 0x4c):
        w32(ctx, obj + off, fl(r))
    wb(ctx, obj + 0xa0, r.choice([b'TL1M', b'tl1m', b'TL1X', b'ABCD']))
    T['obj'] = obj
    T['wobj'] = r.choice([obj, obj, 0])
    unit = newbuf(ctx, 0x100)
    w32(ctx, unit, r.choice([0, 1, 2, 4, 5, 8, 9, 10]))
    w32(ctx, unit + 4, r.randrange(0, 4))
    w32(ctx, 0x1017b8c8, r.randrange(0, 4))
    for off in (0x58, 0x5c, 0x60):
        w32(ctx, unit + off, fl(r, -300, 300))
    rec = newbuf(ctx, 0x600)
    w32(ctx, rec, unit)
    w32(ctx, rec + 0x4f0, r.choice([0, 2, 4, 1]))
    w32(ctx, rec + 0x4f4, r.choice([0, 0, 1, 5]))
    w32(ctx, rec + 0x58c, r.choice([0, 2]))
    for off in (0x520, 0x53c):
        w32(ctx, rec + off, fl(r, 0.0, 20.0))
    for off in (0x554, 0x558, 0x55c):
        w32(ctx, rec + off, fl(r, -30, 30))
    for a in (0x1015dd58, 0x1015dd5c, 0x1015dd60):
        w32(ctx, a, r.choice([0, 0, fl(r, -1, 1)]))
    tiny = r.random() < 0.15
    d = [fb(r.uniform(-1e-8, 1e-8)) if tiny else fl(r, -40, 40) for _ in range(3)]
    outs = [newbuf(ctx, 4) for _ in range(5)]
    return [rec, outs[0], outs[1]] + d + outs[2:]


SPECS = [dict(name='Sim_100663c0', orig=0x100663c0, c='Sim_100663c0', gen=g, trials=15000)]
