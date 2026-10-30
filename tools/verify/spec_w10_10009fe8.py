# Round 10: 0x10009fe8 (sphere against a segment, result in st(0) through the trampolines). Game logic: exact,
# no tolerance. Sim_1009cbde / Sim_1009cbfe (converted, the segment length) run for real on both sides.
import math
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1000/w10_misc.c'])


def f32(v):
    return struct.unpack('<f', struct.pack('<f', v))[0]


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx)
    st0(ctx, 0x10009fe8, 'Sim_10009fe8', 5)
    sc = r.choice([1.0, 10.0, 300.0, 5000.0])
    st = [f32(r.uniform(-sc, sc)) for _ in range(3)]
    en = [f32(st[i] + r.uniform(-sc, sc)) for i in range(3)]
    de = [f32(en[i] - st[i]) for i in range(3)]
    ln = f32(math.sqrt(sum(x * x for x in de)))
    seg = newbuf(ctx, 0x40)
    wf(ctx, seg, *st)
    wf(ctx, seg + 0xc, *en)
    wf(ctx, seg + 0x18, *de)
    un = [f32(x / ln) if ln else 0.0 for x in de]
    wf(ctx, seg + 0x24, *un)
    x = r.random()
    if x < 0.15:
        w32(ctx, seg + 0x34, 0)
    else:
        w32(ctx, seg + 0x34, 1)
        wf(ctx, seg + 0x30, ln if r.random() < 0.8 else f32(r.uniform(0.0, 2 * ln + 1)))
    if r.random() < 0.05:
        w32(ctx, seg + 0x30, rfloat(r))
    # centre: on the line, near it, behind the start or anywhere
    k = r.uniform(-0.5, 1.5)
    c = [f32(st[i] + k * de[i] + r.uniform(-1, 1) * r.choice([0.0, 0.01, 0.1, 1.0]) * sc) for i in range(3)]
    if r.random() < 0.2:
        c = [f32(r.uniform(-2 * sc, 2 * sc)) for _ in range(3)]
    rad = f32(r.choice([r.uniform(0.0, sc), r.uniform(0.0, 0.1 * sc), 0.0, 1e-7, 9.9e-8, -1.0]))
    args = [fb(c[0]), fb(c[1]), fb(c[2]), fb(rad)]
    if r.random() < 0.05:
        args[r.randrange(4)] = rfloat(r)
    return args + [seg]


SPECS = [dict(name='Sim_10009fe8', orig=TRAMP_O, c='w10_tramp_c', gen=g, trials=4000)]
