# Round 10: 0x10098f8c (snaps vertex texture coordinates to 0.01 / 0.99, then submits the vertices).
# Render_SubmitVerts_100068d9 (converted, calls the renderer plugin) is stubbed on both sides: its three
# arguments are logged and the whole vertex array is compared in memory afterwards.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1009/w10_misc.c'])
helper(0x100068d9, 'Render_SubmitVerts_100068d9', 3)


def g(ctx):
    r = begin(ctx, 0x100068d9)
    count = r.choice([-5, -1, 0, 1, 2, 3, 4, 5, 8, 12, r.randrange(0, 20)])
    n = max(count, 0) + 1
    verts = newbuf(ctx, n * 0x24)
    for i in range(n):
        for off in (0x1c, 0x20):
            x = r.random()
            if x < 0.5:
                v = struct.unpack('<I', struct.pack('<f', r.uniform(-0.5, 1.5)))[0]
            elif x < 0.6:
                v = r.choice([0x3f000000, 0x3f000001, 0x3effffff, 0x80000000, 0, 0x7fc00000, 0xffc00000])
            else:
                v = rfloat(r)
            w32(ctx, verts + i * 0x24 + off, v)
    return [r.getrandbits(8), count & 0xffffffff, verts, r.getrandbits(32)]


SPECS = [dict(name='Sim_10098f8c', orig=0x10098f8c, c='Sim_10098f8c', gen=g, trials=1500)]
