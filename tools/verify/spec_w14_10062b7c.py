# Round 14: 0x10062b7c (HUD weapon label; round 10 spec spec_w10_10062b7c.py, now with ordinary buffers). Stubbed on both
# sides: Sim_10064824 (writes the screen x / y), Sim_1006519d (text logged) and Sim_100c8ba7. Str_Copy runs for real.
# The labels are 0..8 characters, the shipped range (entries of 0x101362e0). Exact (eax compared).
# Negative check: W14_NEG=1 gives labels of 16..23 characters, which run on into the original's frame and must mismatch.
exec(open('tools/verify/w10_common.py').read())
NEG = bool(os.environ.get('W14_NEG'))

SOURCES = closure(['sim/unsorted_1006/w10_hud.c'])
helper(0x10064824, 'Sim_10064824', 3)
helper(0x1006519d, 'Sim_1006519d', 3, sargs=(2,))
helper(0x100c8ba7, 'Sim_100c8ba7', 4)
ACTIVE = (0x10064824, 0x1006519d, 0x100c8ba7)


def e_pos(uc, k, raw, eax):
    uc.mem_write(raw[1], struct.pack('<i', ST['xy'][0]))
    uc.mem_write(raw[2], struct.pack('<i', ST['xy'][1]))


EFFECT[0x10064824] = e_pos


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    ST['xy'] = (r.randrange(-50, 700), r.randrange(-50, 500))
    rets(0x10064824, r.choice([0, 1, 1, 1]))
    for a in range(0x109e854c, 0x109e857c, 4):
        n = r.choice([16, 19, 21, 23] if NEG else [0, 3, 5, 7, 8, 8])
        w32(ctx, a, ctx.buf(0x40, bytes(r.choice(b'LOCKED RANGE') for _ in range(n)).ljust(0x40, b'\0')))
    w32(ctx, 0x10176930, r.choice([0, 1]))
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x109e8b40 + 4 * self_, newbuf(ctx, 0x40))
    unit = newbuf(ctx, 0x100)
    w32(ctx, unit + 0xf0, r.choice([0x203, 0x203, 0, 0x1203, 0x103]))
    rec = newbuf(ctx, 0x600)
    w32(ctx, rec, unit)
    w16(ctx, rec + 0x560, r.choice([0x80, 0x8000, 0]))
    slots = newbuf(ctx, 0x128 * 4)
    si = r.randrange(0, 4)
    w32(ctx, rec + 0x4fc, si)
    kind = r.randrange(0, 40)
    w32(ctx, slots + si * 0x128 + 4, r.choice([kind, kind, -1]) & 0xffffffff)
    w32(ctx, slots + si * 0x128 + 8, r.choice([1, 1, 0]))
    w32(ctx, rec + 0x84, slots)
    k = 0x10145508 + kind * 0xd6
    w32(ctx, k + 0x10, r.choice([0x3e, 0x3f, 0, 1, 2, 5]))
    w32(ctx, k + 0x86, r.choice([0, 1]))
    w32(ctx, k + 0x4c, fb(r.uniform(0, 100)))
    w32(ctx, k + 0x50, fb(r.uniform(100, 3000)))
    return [rec, fb(r.uniform(-6, 6)), fb(r.uniform(-6, 6)), fb(r.uniform(0, 3500))]


SPECS = [dict(name='Sim_10062b7c', orig=0x10062b7c, c='Sim_10062b7c', gen=g, trials=300 if NEG else 6000)]
