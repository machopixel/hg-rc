# Round 10: 0x10030c87 (segment against the collision grid). Sim_1002e790 (face cut) is stubbed on both sides: it
# logs the face and the points, answers from a hit pattern and, on a hit, shortens the segment copy (writes its
# length +0x30 and the valid flag +0x34 from a list). Sim_1009cfbd (copy) and Sim_1009cbde (length) run for real;
# the copies are compared through the segment. Game logic: exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w10_misc.c'])
helper(0x1002e790, 'Sim_1002e790', 2)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_cut(uc, k, raw, eax):
    hit = ST['hits'][k % len(ST['hits'])]
    if hit:
        seg = struct.unpack('<I', bytes(uc.mem_read(uc.reg_read(UC_X86_REG_ESP) + 12, 4)))[0]
        uc.mem_write(seg + 0x30, struct.pack('<II', ST['lens'][k % len(ST['lens'])], 1))
    return hit


EFFECT[0x1002e790] = e_cut


def box(ctx, a, c, h):
    r = ctx.rng
    vals = []
    for i in range(3):
        lo = c[i] - h * r.uniform(0.5, 1.5)
        vals += [lo, lo + 2 * h * r.uniform(0.5, 1.5)]
    wf(ctx, a, *vals)


def g(ctx):
    r = begin(ctx, 0x1002e790)
    grid = newbuf(ctx, 0x24 + 7 * 0xe0)
    box(ctx, grid, [0, 0, 0], 28000)
    w32(ctx, grid + 0x18, fb(r.choice([0.0, r.uniform(-5000, 5000), 40000.0, -40000.0])))
    w32(ctx, grid + 0x1c, fb(r.uniform(-100, 100)))
    w32(ctx, grid + 0x20, fb(r.choice([0.0, r.uniform(-5000, 5000)])))
    for i in range(7):
        for k in range(7):
            c = grid + 0x24 + i * 0xe0 + k * 0x20
            cx, cz = -28000 + 8000 * i + 4000, -28000 + 8000 * k + 4000
            box(ctx, c, [cx, r.uniform(-200, 200), cz], 4000)
            nodes = 0
            for n in range(r.choice([0, 0, 1, 2, 3])):
                nd = newbuf(ctx, 8)
                w32(ctx, nd, nodes)
                w32(ctx, nd + 4, r.choice([0x5000 + n, 0x5000 + n, 0]))
                nodes = nd
            w32(ctx, c + 0x1c, nodes)
            w32(ctx, c + 0x18, r.choice([0, 1, 1, 1]))
    seg = newbuf(ctx, 0x38)
    a = [r.uniform(-30000, 30000), r.uniform(-300, 300), r.uniform(-30000, 30000)]
    b = [a[i] + r.choice([r.uniform(-9000, 9000), r.uniform(-500, 500)]) for i in range(3)]
    wf(ctx, seg, *(a + b))
    if r.random() < 0.03:
        w32(ctx, seg + 4 * r.randrange(6), rfloat(r))
    ST['hits'] = [r.choice([0, 0, 1]) for _ in range(11)]
    ST['lens'] = [r.choice([fb(r.uniform(0, 9000)), 0x7f7fc99e, 0x7fc00000, fb(r.uniform(0, 9000))]) for _ in range(7)]
    pts = newbuf(ctx, 0x40)
    if r.random() < 0.02:
        grid = 0
    return [grid, pts, seg]


SPECS = [dict(name='Sim_10030c87', orig=0x10030c87, c='Sim_10030c87', gen=g, trials=3000)]
