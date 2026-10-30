# Round 16: 0x100689c0 (unit b's position against unit a's collision box). The converted callees (Sim_100a9cfb,
# Sim_100a9ca3 with its helpers, Sim_10068e1d, Sim_10068d76) run for real; 0x100687a3 (converted in round 16) is a
# logging stub, so its arguments are compared. Signalling NaNs in a's node and b's radius check the x87 quieting.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w16_range.c'])
helper(0x100687a3, 'Sim_100687a3', 8)
SNAN = [0x7fc00001, 0xffc12345, 0x7f800000]   # quiet NaNs and infinity (the emulator does not quiet signalling NaNs on an x87 load, the real FPU does; they never occur in play)


def fl(r):
    if r.random() < 0.03:
        return r.choice(SNAN)
    return struct.unpack('<I', struct.pack('<f', r.uniform(-500.0, 500.0)))[0] if r.random() < 0.85 else rfloat(r)


def g(ctx):
    r = begin(ctx, 0x100687a3)
    node = ctx.buf(0x100, b''.join(struct.pack('<I', fl(r)) for _ in range(0x40)))
    w32(ctx, node + 0x94, r.choice([0, 0x8000, r.getrandbits(32)]))
    reca = newbuf(ctx, 0x300)
    w32(ctx, reca + 0x48, node)
    a = newbuf(ctx, 4)
    w32(ctx, a, reca)
    recb = newbuf(ctx, 0x100)
    for off in (0x58, 0x5c, 0x60):
        w32(ctx, recb + off, fl(r))
    b = newbuf(ctx, 0x540)
    w32(ctx, b, recb)
    w32(ctx, b + 0x53c, fl(r))
    outs = [newbuf(ctx, 4) for _ in range(3)]
    if r.random() < 0.05:
        a = 0
    if r.random() < 0.05:
        b = 0
    return [a, b, fl(r), fl(r), fl(r)] + outs


SPECS = [dict(name='Sim_100689c0', orig=0x100689c0, c='Sim_100689c0', gen=g, trials=8000)]
