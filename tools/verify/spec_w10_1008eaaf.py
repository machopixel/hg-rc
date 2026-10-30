# Round 10: 0x1008eaaf (point on a rect for an angle). The CRT fmod / fabs (exact on doubles), Sim_10084766,
# Sim_1008486e and Sim_1008ec46 (converted) run for real on both sides. Exact.
import math
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1008/w10_misc.c'])


def g(ctx):
    r = begin(ctx)
    wb(ctx, 0x103719a0, struct.pack('<513f', *[math.sin(i * 0.003067961572265625) for i in range(513)]))
    cam = newbuf(ctx, 0x60)
    wf(ctx, cam + 0x44, r.choice([1.0, 1.333333, 0.75, r.uniform(0.0, 3.0)]))
    w32(ctx, 0x1014cae0, cam)
    rect = newbuf(ctx, 0x20)
    a, b = r.randrange(-50, 700), r.randrange(-50, 500)
    w32(ctx, rect + 4, a, b, a + r.randrange(-20, 900), b + r.randrange(-20, 700))
    out = newbuf(ctx, 8)
    x = r.random()
    if x < 0.55:
        ang = struct.unpack('<I', struct.pack('<f', r.uniform(-1000.0, 1000.0)))[0]
    elif x < 0.8:
        ang = struct.unpack('<I', struct.pack('<f', float(r.choice([0, 90, 180, 270, 360, -90, -180, 450, 89.99999, 90.00001, 270.0001, 180.00002]))))[0]
    else:
        ang = rfloat(r)
    return [rect, ang, out]


SPECS = [dict(name='Sim_1008eaaf', orig=0x1008eaaf, c='Sim_1008eaaf', gen=g, trials=3000)]
