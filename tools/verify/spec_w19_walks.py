# Round 19: 0x10089066 and 0x1008950e, the node-tree walks (DECISIONS.md round 19). The original's recursion runs for
# real; the stack below the call is random (so a leaf whose depth had no non-leaf call yet sees a random leftover,
# almost never the marker, the case the C models as "not the marker"). Random object trees with the flag bits 0x1000 /
# 0x800 make non-leaf calls whose box stays empty (minimum x = the marker 0x7f7fc99e), so later leaves at that depth
# see the marker and answer 0 in both.
# Negative check: W19_NEG=1 writes the marker into the original's frame slot of every depth before the call: leaves
# at depths without an earlier non-leaf call then answer 0 in the original and 1 in the C.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1008/w19_bounds.c'])
NEG = bool(os.environ.get('W19_NEG'))


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def make_tree(ctx, r, depth, budget, flagbits):
    obj = newbuf(ctx, 0x90)
    w16(ctx, obj, r.choice([0, 0, flagbits, 0x1800, r.getrandbits(16)]))
    for off in range(0x38, 0x50, 4):
        w32(ctx, obj + off, fb(r.uniform(-500, 500)) if r.random() < 0.97 else rfloat(r))
    if r.random() < 0.05:
        w32(ctx, obj + 0x28, 0)
        return obj
    node = newbuf(ctx, 0x10)
    w32(ctx, obj + 0x28, node)
    kids = r.randrange(0, 4) if depth < 6 and budget[0] > 0 else 0
    prev = 0
    first = 0
    for _ in range(kids):
        budget[0] -= 1
        ch = newbuf(ctx, 0x90)
        w32(ctx, ch + 8, 0)
        w32(ctx, ch + 0x8c, 0 if r.random() < 0.1 else make_tree(ctx, r, depth + 1, budget, flagbits))
        if prev:
            w32(ctx, prev + 8, ch)
        else:
            first = ch
        prev = ch
    w32(ctx, node + 4, first)
    return obj


def gen(flagbits):
    def g(ctx):
        r = begin(ctx)
        root = make_tree(ctx, r, 0, [r.randrange(1, 25)], flagbits)
        outs = [ctx.buf(4) for _ in range(3)]
        lo = frame(4) - 0x1000
        ctx.m.uc.mem_write(lo, bytes(r.getrandbits(8) for _ in range(0x1000)))
        if NEG:
            for d in range(12):
                w32(ctx, frame(4) - 4 - 0x44 * d, 0x7f7fc99e)
        return [root] + outs
    return g


SPECS = [dict(name='Sim_10089066', orig=0x10089066, c='Sim_10089066', gen=gen(0x1000), trials=400 if NEG else 10000),
         dict(name='Sim_1008950e', orig=0x1008950e, c='Sim_1008950e', gen=gen(0x800), trials=400 if NEG else 10000)]
