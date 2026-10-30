# Round 10: 0x1006c161 (a shot hits a part of a damage block). Stubbed on both sides: Sim_100aacd5, Sim_100a9cfb (writes
# three floats), Sim_100939a3, Sim_1000ebfb / Sim_1000eb12 (sound names logged as text), Sim_1009c929, Sim_1006abf6,
# Sim_1006b52f, the debug log 0x10083495 (its nine argument words logged) and Sim_100aae7b. The CRT fabs runs for
# real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w10_misc.c'])
STUBS = ((0x100aacd5, 'Sim_100aacd5', 2), (0x100a9cfb, 'Sim_100a9cfb', 4), (0x100939a3, 'Sim_100939a3', 7),
         (0x1009c929, 'Sim_1009c929', 1), (0x1006abf6, 'Sim_1006abf6', 3), (0x1006b52f, 'Sim_1006b52f', 5),
         (0x100aae7b, 'Sim_100aae7b', 3))
for _a, _c, _n in STUBS:
    helper(_a, _c, _n)
helper(0x1000ebfb, 'Sim_1000ebfb', 5, sargs=(0,))
helper(0x1000eb12, 'Sim_1000eb12', 7, sargs=(3,))
helper(0x10083495, None, 9)
ACTIVE = tuple(a for a, _, _ in STUBS) + (0x1000ebfb, 0x1000eb12, 0x10083495)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_three(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[1 + i], struct.pack('<I', ST['p'][i]))


EFFECT[0x100a9cfb] = e_three


def g(ctx):
    r = begin(ctx, *ACTIVE)
    ST['p'] = [fb(r.uniform(-500, 500)) for _ in range(3)]
    rets(0x100aacd5, r.choice([0, 0x7000]))
    rets(0x1009c929, *[r.randrange(0, 100) for _ in range(3)])
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    rp = newbuf(ctx, 4)
    w8(ctx, rp + 1, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b8c4, rp)
    w32(ctx, 0x10179ce0, r.choice([1, 1, 0]))
    w32(ctx, 0x1015ec58, r.choice([0, 0, 1]))
    cam = newbuf(ctx, 0x10)
    w32(ctx, cam, *[fb(r.uniform(-500, 500)) for _ in range(3)])
    w32(ctx, 0x1014cae0, cam)
    unit = newbuf(ctx, 0x80)
    w32(ctx, unit, r.choice([1, 9, 0xa, 2]))
    w32(ctx, unit + 4, r.choice([self_, r.randrange(0, 8)]))
    w32(ctx, unit + 0xc, r.choice([self_, r.randrange(0, 8)]))
    w32(ctx, unit + 0x14, r.choice([3, 1]))
    w16(ctx, unit + 0x1c, r.choice([0x2000, 0, r.getrandbits(16)]))
    w32(ctx, unit + 0x48, newbuf(ctx, 0x20))
    w32(ctx, unit + 0x58, *[fb(r.uniform(-500, 500)) for _ in range(3)])
    b = newbuf(ctx, 0xc60)
    w32(ctx, b, unit)
    w32(ctx, b + 0xbcf, r.choice([0, 0x10]))
    parts = newbuf(ctx, 0x3c * 18)
    for p in range(18):
        q = parts + 0x3c * p
        w32(ctx, q, r.choice([fb(r.uniform(0, 40)), 0, fb(-1.0)]))
        w32(ctx, q + 8, fb(r.choice([r.uniform(-10, 200), 0.0])))
        w16(ctx, q + 0x36, r.choice([0, 0x2000, r.getrandbits(16)]))
        w32(ctx, q + 0x38, r.choice([0, 100, 250]))
    w32(ctx, b + 0x88, parts)
    for p in range(18):
        w32(ctx, b + 0xb67 + 4 * p, r.randrange(-1, 8) & 0xffffffff)
    shot = newbuf(ctx, 0x90)
    w32(ctx, shot + 0x70, r.choice([r.randrange(0, 100), 0, 0xffffff00]))
    w32(ctx, shot + 0x74, fb(r.uniform(0, 3)))
    w32(ctx, shot + 0x78, r.randrange(0, 20))
    w16(ctx, shot + 0x7c, r.choice([0, 1, 2, 3, 4, 5]))
    w32(ctx, shot + 0x86, ctx.buf(8, b'HITSND\0\0'))
    w32(ctx, shot + 0x18, *[fb(r.uniform(-50, 50)) for _ in range(3)])
    if r.random() < 0.03:
        shot = 0
    return [r.randrange(0, 8), b, r.choice([0, 1]), shot, r.choice(list(range(0, 19)) + [5, 6])]


SPECS = [dict(name='Sim_1006c161', orig=0x1006c161, c='Sim_1006c161', gen=g, void=True, trials=6000)]
