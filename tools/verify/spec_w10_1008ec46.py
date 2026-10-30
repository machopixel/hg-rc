# Round 10: 0x1008ec46 (a point moved by a mode and an angle, written to out). Exact (no tolerance).
# Sim_10084766 (converted, sine table) runs for real on both sides, with the table 0x103719a0 filled.
import math
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1008/w10_misc.c'])


def g(ctx):
    r = begin(ctx)
    if not getattr(ctx.m, '_w10_sine', False):
        pass
    wb(ctx, 0x103719a0, struct.pack('<513f', *[math.sin(i * 0.003067961572265625) for i in range(513)]))
    cam = newbuf(ctx, 0x60)
    wf(ctx, cam + 0x44, r.choice([1.0, 1.333333, 0.75, r.uniform(0.0, 3.0), r.uniform(-2.0, 2.0)]))
    if r.random() < 0.05:
        w32(ctx, cam + 0x44, rfloat(r))
    w32(ctx, 0x1014cae0, cam)
    rect = newbuf(ctx, 0x20)
    a = r.randrange(-50, 700)
    w32(ctx, rect + 4, a)
    w32(ctx, rect + 0xc, a + r.choice([r.randrange(-20, 900), r.getrandbits(32) - (1 << 31), 0, 1, 2, 3]))
    pt = newbuf(ctx, 8)
    w32(ctx, pt, r.randrange(-1000, 1000), r.randrange(-1000, 1000))
    out = newbuf(ctx, 8)
    mode = r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xffffffff, r.getrandbits(32)])
    x = r.random()
    if x < 0.6:
        ang = struct.unpack('<I', struct.pack('<f', r.uniform(-720.0, 720.0)))[0]
    elif x < 0.7:
        ang = struct.unpack('<I', struct.pack('<f', float(r.choice([0, 90, 180, 270, 360, -90, 45]))))[0]
    else:
        ang = rfloat(r)
    return [rect, pt, mode, ang, out]


SPECS = [dict(name='Sim_1008ec46', orig=0x1008ec46, c='Sim_1008ec46', gen=g, trials=2000)]
