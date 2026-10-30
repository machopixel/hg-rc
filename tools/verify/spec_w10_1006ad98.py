# Round 10: 0x1006ad98 (destroy a part of a damage block, recursive). Stubbed on both sides: Sim_100ab054,
# Sim_100aafe4, Sim_1009c929, Sim_1009c969, Sim_100203cf, Sim_1006abf6, Score_RecordKill, Sim_10092f9d (debris; all
# seven arguments logged) and Sim_100a9cfb (writes three floats). The recursion runs the C (or the original). Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w10_misc.c'])
STUBS = ((0x100ab054, 'Sim_100ab054', 2), (0x100aafe4, 'Sim_100aafe4', 2), (0x1009c929, 'Sim_1009c929', 1),
         (0x1009c969, 'Sim_1009c969', 0), (0x100203cf, 'Sim_100203cf', 2), (0x1006abf6, 'Sim_1006abf6', 3),
         (0x1006a12d, 'Score_RecordKill', 2), (0x10092f9d, 'Sim_10092f9d', 7), (0x100a9cfb, 'Sim_100a9cfb', 4))
for _a, _c, _n in STUBS:
    helper(_a, _c, _n)
ACTIVE = tuple(a for a, _, _ in STUBS)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_three(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[1 + i], struct.pack('<I', ST['pt'][(k + i) % 4]))


EFFECT[0x100a9cfb] = e_three


def g(ctx):
    r = begin(ctx, *ACTIVE)
    ST['pt'] = [fb(r.uniform(-500, 500)) for _ in range(4)]
    rets(0x1009c929, r.choice([0, 1]))
    rets(0x1009c969, r.getrandbits(32))
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    unit = newbuf(ctx, 0x60)
    w32(ctx, unit, r.choice([1, 9, 2, 5]))
    w32(ctx, unit + 4, r.choice([self_, r.randrange(0, 8)]))
    w32(ctx, unit + 0xc, r.choice([self_, r.randrange(0, 8)]))
    w32(ctx, unit + 0x14, r.choice([3, 1]))
    w32(ctx, unit + 0x48, newbuf(ctx, 0x20))
    b = newbuf(ctx, 0xc50)
    wb(ctx, b, bytes(r.getrandbits(8) for _ in range(0xc50)))
    w32(ctx, b, unit)
    parts = newbuf(ctx, 0x3c * 16)
    for k in range(16):
        w32(ctx, parts + k * 0x3c, r.getrandbits(32))
        w32(ctx, parts + k * 0x3c + 8, r.getrandbits(32))
        w16(ctx, parts + k * 0x3c + 0x36, r.choice([0, 0, 0, 0x2000, r.getrandbits(16)]))
    w32(ctx, b + 0x88, parts)
    w16(ctx, b + 0x560, r.choice([0, 0x20, r.getrandbits(16)]))
    w32(ctx, b + 0x510, r.choice([-1, 0, 4]) & 0xffffffff)
    w32(ctx, b + 0x4f0, r.choice([0, 2, 4, 5]))
    w32(ctx, b + 0x4c0, newbuf(ctx, 0x80))
    w32(ctx, b + 0x4c4, newbuf(ctx, 0x80))
    return [r.randrange(0, 8), b, r.choice(list(range(0, 18)) + [2, 7, 5, 6]), r.choice([0, 0, 1])]


SPECS = [dict(name='Sim_1006ad98', orig=0x1006ad98, c='Sim_1006ad98', gen=g, void=True, trials=5000)]
