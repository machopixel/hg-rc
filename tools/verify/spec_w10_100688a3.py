# Round 10: 0x100688a3. The converted callees (Sim_100a9cfb, Sim_100a9ca3 with its original helpers,
# Sim_10068e1d, Sim_10068d76) run for real on both sides. 0x100687a3 stays original but reads three of its own
# uninitialised locals (the stack garbage differs between the original and the C caller), so it is stubbed:
# its 8 arguments are logged (the three pointers are the caller's own arguments, not locals) and the answer
# comes from rets(). Exact (no tolerance).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w10_misc.c'])
helper(0x100687a3, None, 8)


def fl(r):
    return struct.unpack('<I', struct.pack('<f', r.uniform(-500.0, 500.0)))[0] if r.random() < 0.85 else rfloat(r)


def g(ctx):
    r = begin(ctx, 0x100687a3)
    node = ctx.buf(0x100, b''.join(struct.pack('<I', fl(r)) for _ in range(0x40)))
    w32(ctx, node + 0x94, r.choice([0, 0x8000, r.getrandbits(32)]))
    rec = newbuf(ctx, 0x300)
    w32(ctx, rec + 0x48, node)
    ent = newbuf(ctx, 4)
    w32(ctx, ent, rec)
    obj = newbuf(ctx, 0x540)
    w32(ctx, obj + 0x53c, fl(r))
    outs = [newbuf(ctx, 4) for _ in range(3)]
    if r.random() < 0.05:
        ent = 0
    if r.random() < 0.05:
        obj = 0
    return [obj, ent, fl(r), fl(r), fl(r)] + outs


SPECS = [dict(name='Sim_100688a3', orig=0x100688a3, c='Sim_100688a3', gen=g, trials=1500)]
