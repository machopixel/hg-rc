# Round 14: 0x1009d200 (spawn a group's gear at a start point). Stubs on both sides: Sim_100b35b0, Sim_100599a0 (name
# logged), Sim_1009d90e, Sim_10072e35, Sim_1005cd4a, the node helpers (Sim_100a9cfb writes the trial's foot heights),
# Sim_1007caab, Sim_100500ab, Sim_1006da46, Sim_1009c929, Sim_1002c113 and the float answer Sim_1002d961.
# Mat3x4_TransformPoint runs for real. Start zones have 1..8 points, as the zone loader 0x100daea4 allows (a count of 0
# divides by zero in Sim_1009c929 in both).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1009/w14_spawn.c'])
CALLS = [(0x100b35b0, 'Sim_100b35b0', 0), (0x100599a0, 'Sim_100599a0', 1), (0x1009d90e, 'Sim_1009d90e', 3),
         (0x10072e35, 'Sim_10072e35', 1), (0x1005cd4a, 'Sim_1005cd4a', 2), (0x100aa39f, 'Sim_100aa39f', 1),
         (0x100aa799, 'Sim_100aa799', 1), (0x1007caab, 'Sim_1007caab', 1), (0x100500ab, 'Sim_100500ab', 1),
         (0x1006da46, 'Sim_1006da46', 1), (0x1009c929, 'Sim_1009c929', 1), (0x100a9d47, 'Sim_100a9d47', 4),
         (0x100aa0da, 'Sim_100aa0da', 5), (0x1002c113, 'Sim_1002c113', 0), (0x100a9cfb, 'Sim_100a9cfb', 4),
         (0x100a9d7c, 'Sim_100a9d7c', 4)]
for a, c, n in CALLS:
    helper(a, c, n)
SARGS[0x100599a0] = {0}
FSTUB[0x1002d961] = ('Sim_1002d961', 3)
ACTIVE = tuple(a for a, _, _ in CALLS)
T = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def _feet(uc, k, raw, eax):
    for i, a in enumerate(raw[1:4]):
        uc.mem_write(a, struct.pack('<I', T['feet'][(k * 3 + i) % len(T['feet'])]))


EFFECT[0x100a9cfb] = _feet


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    ST['fvals'] = {0x1002d961: [fb(r.uniform(-50, 50))]}
    T['feet'] = [r.choice([fb(r.uniform(-5, 5)), 0x7f7fc99e, fb(0.0)]) for _ in range(9)]
    side = r.randrange(0, 4)
    zone = 0x109b8420 + side * 0x74
    cnt = r.randrange(1, 9)
    w32(ctx, zone + 0x70, cnt)
    pts = [(r.choice([0.0, 100.0, r.uniform(-1000, 1000)]), r.choice([0.0, r.uniform(-1000, 1000)])) for _ in range(8)]
    for i, (x, z) in enumerate(pts):
        w32(ctx, zone + 0x10 + 4 * i, fb(x))
        w32(ctx, zone + 0x30 + 4 * i, fb(z))
        w32(ctx, zone + 0x50 + 4 * i, fb(r.uniform(-180, 180)))
    m = 0x109b7d7c + side * 0x6c
    if r.random() < 0.5:
        w32(ctx, m, fb(1), 0, 0, 0, fb(1), 0, 0, 0, fb(1), 0, 0, 0)
    else:
        w32(ctx, m, *[fb(r.uniform(-1, 1)) for _ in range(9)] + [fb(r.uniform(-100, 100)) for _ in range(3)])
    rets(0x1009c929, r.randrange(0, cnt), r.randrange(0, cnt))
    ngroups = r.randrange(1, 6)
    w32(ctx, 0x1015dc64, ngroups)
    index = r.choice([r.randrange(0, ngroups), r.randrange(0, ngroups), -1, 0x19, 30])
    for gi in range(max(ngroups, 6)):
        grp = newbuf(ctx, 0x80)
        w32(ctx, 0x109e8b40 + 4 * gi, grp)
        w16(ctx, grp + 0x1c, r.getrandbits(16))
        x, z = r.choice(pts)
        w32(ctx, grp + 0x58, fb(x), r.choice([0, fb(5.0)]), fb(z))
        w32(ctx, grp + 0x28, 0)
    if 0 <= index < 0x19:
        if index >= max(ngroups, 6):
            w32(ctx, 0x109e8b40 + 4 * index, newbuf(ctx, 0x80))
        grp = r32(ctx, 0x109e8b40 + 4 * index)
        rec = newbuf(ctx, 0xc60)
        unit = newbuf(ctx, 0x300)
        blk = newbuf(ctx, 0x60)
        w32(ctx, grp + 0x28, r.choice([rec, rec, rec, 0]))
        w32(ctx, rec, unit)
        w32(ctx, unit + 0x54, blk)
        w32(ctx, unit + 8, side)
        w32(ctx, unit + 4, r.randrange(0, 8))
        w32(ctx, unit + 0x5c, r.choice([0, fb(5.0)]))
        w32(ctx, unit + 0x280, r.choice([0, 1, -1, cnt - 1]))
        w32(ctx, rec + 0x4c0, r.choice([0, 0x00c10000]))
        w32(ctx, rec + 0x4c4, r.choice([0, 0x00c20000]))
        w16(ctx, rec + 0x562, r.choice([0, 0x2000, 0x4000, 0x6000]))
        wb(ctx, rec + 0xbeb, b'HUNTER\0')
    w32(ctx, 0x1017b8c8, r.choice([index, 0, 2]))
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    return [index & 0xffffffff, r.choice([0, 1]), r.getrandbits(32), r.getrandbits(32)]


SPECS = [dict(name='Sim_1009d200', orig=0x1009d200, c='Sim_1009d200', gen=g, trials=10000)]
