# DLL removal step 1: the graphics library's angle lookup 0x100ecb3b and point rotation 0x100ecc19
# (src/sim/gfx/w37_math.c) against the original routines: angles in and far outside 0..3600 (the quadrant edges
# included), random points, centres and 16.16 scale factors. Integer only. 1.2's twins are identical apart from
# addresses. The rotation calls the lookup for real on both sides (the C its C).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w37_common.py').read())

SOURCES = closure(['sim/gfx/w37_math.c'])


def angle(r):
    return r.choice([r.randrange(0, 3601), r.choice([0, 0x384, 0x385, 0x708, 0x709, 0xa8c, 0xa8d, 0xe10, 0xe11]),
                     r.randrange(-20000, 20000), r.randrange(-3601, 0)]) & 0xffffffff


def g_wave(ctx):
    r = start(ctx)
    return [angle(r), newbuf(ctx, 4), newbuf(ctx, 4)]


def val(r):
    return r.choice([r.randrange(-700, 700), r.randrange(-40000, 40000), r.getrandbits(32)]) & 0xffffffff


def scale(r):
    return r.choice([0x10000, r.randrange(0, 0x40000), r.randrange(-0x40000, 0x40000), r.getrandbits(32)]) & 0xffffffff


def g_rotate(ctx):
    r = start(ctx)
    pin, ctr = newbuf(ctx, 8), newbuf(ctx, 8)
    w32(ctx, pin, val(r), val(r))
    w32(ctx, ctr, val(r), val(r))
    return [pin, newbuf(ctx, 8), ctr, angle(r), scale(r), scale(r)]


SPECS = [dict(name='Sim_100ecb3b', orig=0x100ecb3b, c='Sim_100ecb3b', gen=g_wave, trials=5000, void=True),
         dict(name='Sim_100ecc19', orig=0x100ecc19, c='Sim_100ecc19', gen=g_rotate, trials=5000)]
