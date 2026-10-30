# Round 11: 0x100b3b16 (push a model part onto the transform stack). No stubs: Mat3x4_SetIdentity, Sim_10060656
# (which runs the original 0x10060220 at its address on both sides) and Mat3x4_Multiply run for real.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100b/w11_node.c'])
NODES = 0x101a1180


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx)
    node = r.choice([0, 1, 5, 31, 32, -1, r.randrange(0, 32)])
    depth = r.choice([0, 0, 1, 2, 3])
    w32(ctx, 0x1017d220, node & 0xffffffff)
    w32(ctx, 0x1017d224, depth)
    n = r.randrange(0, 6)
    w32(ctx, 0x101c42a4, n)
    w32(ctx, 0x1017d204, newbuf(ctx, 4 * 8))
    for k in range(0, 6):
        w32(ctx, 0x101c4220 + 4 * k, r.choice([0xffffffff, r.randrange(0, max(1, node)) if node > 0 else 0xffffffff]))
    for k in range(0, 32):
        w32(ctx, NODES + k * 0x7c + 0x78, r.choice([0xffffffff, r.randrange(0, k)]) if k else 0xffffffff)
        w32(ctx, NODES + k * 0x7c + 0x48, *[fb(r.uniform(-2, 2)) for _ in range(12)])
    w32(ctx, 0x10150800, fb(r.uniform(0.5, 2)), fb(r.uniform(0.5, 2)), fb(r.uniform(0.5, 2)),
        *[r.choice([fb(r.uniform(-360, 360)), 0, fb(90.0)]) for _ in range(6)])
    w32(ctx, 0x10150828, *[r.getrandbits(32) for _ in range(9)])
    part = newbuf(ctx, 0x20)
    w32(ctx, part + 8, *[r.choice([r.randrange(-500, 500), 0, -1]) & 0xffffffff for _ in range(6)])
    return [part]


SPECS = [dict(name='Sim_100b3b16', orig=0x100b3b16, c='Sim_100b3b16', gen=g, void=True, trials=3000)]