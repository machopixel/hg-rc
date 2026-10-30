# Round 10: 0x10034111 (segment against a box). The converted segment functions (Sim_1009cfd4 with the CRT fabs,
# Sim_1009cbde/cbfe, Sim_1009cd36) run for real on both sides. Exact (game logic, no tolerance).
import math
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w10_misc.c'])


def f32(v):
    return struct.unpack('<f', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx)
    sc = r.choice([2.0, 50.0, 1000.0])
    st = [f32(r.uniform(-sc, sc)) for _ in range(3)]
    if r.random() < 0.15:
        de = [f32(r.choice([0.0, r.uniform(-sc, sc)])) for _ in range(3)]
    else:
        de = [f32(r.uniform(-2 * sc, 2 * sc)) for _ in range(3)]
    en = [f32(st[i] + de[i]) for i in range(3)]
    ln = f32(math.sqrt(sum(x * x for x in de)))
    un = [f32(x / ln) if ln else 0.0 for x in de]
    seg = newbuf(ctx, 0x40)
    wf(ctx, seg, *st)
    wf(ctx, seg + 0xc, *en)
    wf(ctx, seg + 0x18, *de)
    wf(ctx, seg + 0x24, *un)
    if r.random() < 0.2:
        w32(ctx, seg + 0x34, 0)
    else:
        w32(ctx, seg + 0x34, 1)
        wf(ctx, seg + 0x30, ln if r.random() < 0.85 else f32(r.uniform(0.0, 2 * ln + 1)))
    box = newbuf(ctx, 0x18)
    vals = []
    for i in range(3):
        c = r.uniform(-sc, sc)
        h = abs(r.uniform(0, sc))
        lo, hi = f32(c - h), f32(c + h)
        if r.random() < 0.08:
            lo, hi = hi, lo
        vals += [lo, hi]
    wf(ctx, box, *vals)
    if r.random() < 0.05:
        w32(ctx, box + 4 * r.randrange(6), rfloat(r))
    if r.random() < 0.05:
        w32(ctx, seg + 4 * r.randrange(12), rfloat(r))
    if r.random() < 0.03:
        box = 0
    return [r.getrandbits(32), seg, box]


SPECS = [dict(name='Sim_10034111', orig=0x10034111, c='Sim_10034111', gen=g, trials=4000)]
