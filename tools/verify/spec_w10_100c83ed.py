# Round 10: 0x100c83ed (HUD rate chevrons). The two line routines 0x100ef727 / 0x100e86a1 are stubbed on both sides
# with their seven arguments logged. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100c/w10_misc.c'])
helper(0x100ef727, None, 7)
helper(0x100e86a1, None, 7)


def g(ctx):
    r = begin(ctx, 0x100ef727, 0x100e86a1)
    mid = struct.unpack('<d', bytes(ctx.m.uc.mem_read(0x1012bc18, 8)))[0]
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    w32(ctx, 0x1017e578, r.choice([0, 1, 2, 3, -1 & 0xffffffff, -2 & 0xffffffff]))
    w16(ctx, 0x101659f4, r.getrandbits(16))
    w16(ctx, 0x101659e8, r.getrandbits(16))
    rate = r.choice([mid, mid, mid - 1, mid + 1, mid * 1.0000001, mid - 1e-7])
    rb_ = struct.unpack('<I', struct.pack('<f', rate))[0] if r.random() < 0.95 else rfloat(r)
    return [rb_, r.randrange(-50, 700), r.randrange(-50, 500)]


SPECS = [dict(name='Sim_100c83ed', orig=0x100c83ed, c='Sim_100c83ed', gen=g, void=True, trials=3000)]
