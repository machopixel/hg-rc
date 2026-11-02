# DLL removal step 1: the graphics library's record readers, colour-table routines and GIF header readers
# (src/sim/gfx/w37_info.c) against the original routines. Random records with offsets inside them; the whole
# writable memory (the colour table 0x10156dcc included) is compared. 1.2's twins are identical apart from
# addresses (listings aligned instruction by instruction).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w37_common.py').read())

SOURCES = closure(['sim/gfx/w37_info.c'])


def g_rec(ctx):
    start(ctx)
    return [newbuf(ctx, 0x40)]


def table(ctx, r, n=8, slot=0, size=0x18):
    """a table of n shapes: +8 + 8 * k {offset, second dword}; each shape `size` random bytes"""
    t = bytearray(rb(ctx, 8 + 8 * n))
    off = len(t)
    for k in range(n):
        struct.pack_into('<I', t, 8 + 8 * k + slot, off)
        off += size
    return bytes(t) + rb(ctx, n * size)


def g_shape(ctx):
    r = start(ctx)
    return [put(ctx, table(ctx, r)), r.randrange(0, 8)]


def g_glyph(ctx):
    r = start(ctx)
    n = 0x40
    t = bytearray(rb(ctx, 0x10 + 4 * n))
    for k in range(n):
        struct.pack_into('<I', t, 0x10 + 4 * k, len(t) + 4 * r.randrange(0, 16))
    return [put(ctx, bytes(t) + rb(ctx, 0x40)), r.randrange(0, n)]


def g_shapepal(ctx):
    r = start(ctx)
    n = 4
    t = bytearray(rb(ctx, 8 + 8 * n))
    blocks = b''
    for k in range(n):
        if r.random() < 0.25:
            struct.pack_into('<I', t, 8 + 8 * k + 4, 0)
            continue
        struct.pack_into('<I', t, 8 + 8 * k + 4, len(t) + len(blocks))
        c = r.randrange(1, 40)
        blocks += struct.pack('<I', c) + rb(ctx, 4 * c)
    return [put(ctx, bytes(t) + blocks), r.randrange(0, n), newbuf(ctx, 0x300)]


def g_get(ctx):
    r = start(ctx)
    wb(ctx, 0x10156dcc - 0x40, rb(ctx, 0x280))
    i = r.choice([r.randrange(0, 256), r.randrange(-0x20, 0)])
    return [i & 0xffffffff]


def g_set(ctx):
    r = start(ctx)
    return [r.randrange(0, 256), r.getrandbits(32)]


def g_setall(ctx):
    start(ctx)
    return [newbuf(ctx, 0x200)]


def g_code(ctx):
    start(ctx)
    return [newbuf(ctx, 4), newbuf(ctx, 4)]


def gif(ctx, r):
    g = bytearray(rb(ctx, 0xd + 0x300 + 0xa + 0x300 + 0x10))
    g[0xa] = r.getrandbits(8)
    p = 0xd + ((1 << ((g[0xa] & 7) + 1)) * 3 if g[0xa] & 0x80 else 0)
    g[p + 9] = r.getrandbits(8)
    return put(ctx, bytes(g))


def g_gifsize(ctx):
    r = start(ctx)
    return [gif(ctx, r)]


def g_gifpal(ctx):
    r = start(ctx)
    return [gif(ctx, r), newbuf(ctx, 0x300)]


SPECS = [dict(name='Sim_100ecce0', orig=0x100ecce0, c='Sim_100ecce0', gen=g_rec, trials=300),
         dict(name='Sim_100ed8c3', orig=0x100ed8c3, c='Sim_100ed8c3', gen=g_rec, trials=300),
         dict(name='Sim_100eccf3', orig=0x100eccf3, c='Sim_100eccf3', gen=g_glyph, trials=500),
         dict(name='Sim_100ed743', orig=0x100ed743, c='Sim_100ed743', gen=g_shape, trials=500),
         dict(name='Sim_100f1f86', orig=0x100f1f86, c='Sim_100f1f86', gen=g_shape, trials=500),
         dict(name='Sim_100ed7bc', orig=0x100ed7bc, c='Sim_100ed7bc', gen=g_shape, trials=500),
         dict(name='Sim_100f1f52', orig=0x100f1f52, c='Sim_100f1f52', gen=g_shape, trials=500),
         dict(name='Sim_100ed788', orig=0x100ed788, c='Sim_100ed788', gen=g_shape, trials=500),
         dict(name='Sim_100ed7e6', orig=0x100ed7e6, c='Sim_100ed7e6', gen=g_shapepal, trials=1000, void=True),
         dict(name='Sim_100f01ea', orig=0x100f01ea, c='Sim_100f01ea', gen=g_get, trials=500),
         dict(name='Sim_100f01cc', orig=0x100f01cc, c='Sim_100f01cc', gen=g_set, trials=500),
         dict(name='Sim_100f01ad', orig=0x100f01ad, c='Sim_100f01ad', gen=g_setall, trials=200, void=True),
         dict(name='Sim_100f9eb7', orig=0x100f9eb7, c='Sim_100f9eb7', gen=g_code, trials=50),
         dict(name='Sim_100ed70b', orig=0x100ed70b, c='Sim_100ed70b', gen=g_gifsize, trials=1000),
         dict(name='Sim_100ed6aa', orig=0x100ed6aa, c='Sim_100ed6aa', gen=g_gifpal, trials=1000, void=True)]
