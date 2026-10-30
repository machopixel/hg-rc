# Round 10: 0x100456fc (turns the nodes of some entries of the visible cells towards the camera).
# Stubbed on both sides: 0x10087d83 (patched by hooks_view.c; the generator fills the cell list itself),
# Sim_1009b2eb (answers from rets), Sim_100aa0da (5 arguments logged) and Sim_100aa742. Sim_100a9cfb and
# Sim_1008486e (converted) run for real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1004/w10_misc.c'])
helper(0x10087d83, None, 1)
helper(0x1009b2eb, 'Sim_1009b2eb', 0)
helper(0x100aa0da, 'Sim_100aa0da', 5)
helper(0x100aa742, 'Sim_100aa742', 1)


def fl(r):
    return struct.unpack('<I', struct.pack('<f', r.uniform(-3000.0, 3000.0)))[0] if r.random() < 0.9 else rfloat(r)


def g(ctx):
    r = begin(ctx, 0x10087d83, 0x1009b2eb, 0x100aa0da, 0x100aa742)
    cam = ctx.buf(0x40, b''.join(struct.pack('<I', fl(r)) for _ in range(16)))
    w32(ctx, 0x1014cae0, cam)
    ncell = r.randrange(0, 6)
    for i in range(ncell):
        cell = newbuf(ctx, 0x30)
        prev = cell
        for j in range(r.randrange(0, 6)):
            e = newbuf(ctx, 0x30)
            w16(ctx, e, r.choice([0, 0x1000, r.getrandbits(16)]))
            w16(ctx, e + 2, r.choice([0x10, 0x60, 0x20, 0x30, 0x70, 0x11, 0x6f, r.getrandbits(16)]))
            node = 0
            if r.random() < 0.85:
                node = ctx.buf(0x100, b''.join(struct.pack('<I', fl(r)) for _ in range(0x40)))
            w32(ctx, e + 0x28, node)
            w32(ctx, prev + 0x20, e)
            prev = e
        w32(ctx, prev + 0x20, 0)
        w32(ctx, 0x1036f2a0 + 4 * i, cell)
    w32(ctx, 0x1036f2a0 + 4 * ncell, 0)
    rets(0x1009b2eb, r.choice([0, 1]), r.choice([0, 0, 1]), 0)
    return []


SPECS = [dict(name='Sim_100456fc', orig=0x100456fc, c='Sim_100456fc', gen=g, void=True, trials=1500)]
