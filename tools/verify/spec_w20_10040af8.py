# Round 20: 0x10040af8, the network "RG" message (DECISIONS.md round 20). Every callee (all converted) is a logging
# stub (Sim_1007df6c logs its text); Mem_Move and sprintf run for real. The original's never-written gear pointer
# [ebp-4] gets a leftover pointing into the checker's uncompared stack, as in the round 15 GP case, so its store of 0
# to +0x164 is not compared; the C stores nothing there.
# Negative check: W20_NEG=1 points that leftover at a compared gear record instead (its +0x164 non-zero).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1004/w20_net.c'])
NEG = bool(os.environ.get('W20_NEG'))
for a, c, n in [(0x10040564, 'Sim_10040564', 1), (0x1003dbc0, 'Sim_1003dbc0', 1), (0x1007e950, 'Net_DpSend_1007e950', 4),
                (0x1003cbb3, 'Sim_1003cbb3', 2), (0x1009d200, 'Sim_1009d200', 4), (0x100448cd, 'Sim_100448cd', 1),
                (0x10011093, 'Sim_10011093', 1), (0x100aa39f, 'Sim_100aa39f', 1), (0x100aa454, 'Sim_100aa454', 1),
                (0x100aa799, 'Sim_100aa799', 1)]:
    helper(a, c, n)
helper(0x1007df6c, 'Sim_1007df6c', 4, sargs=(0,))
ACTIVE = (0x10040564, 0x1003dbc0, 0x1007e950, 0x1003cbb3, 0x1009d200, 0x100448cd, 0x10011093, 0x100aa39f, 0x100aa454,
          0x100aa799, 0x1007df6c)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    m = ctx.m
    rets(0x1003dbc0, r.randrange(0, 8) | r.choice([0, 0, 0x10000]))
    own = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, own)
    w32(ctx, 0x1015ecd0, r.choice([0, 1, 1, 2]))
    w32(ctx, 0x1015f200, r.choice([0, 1, 1]))
    flags = newbuf(ctx, 0x10)
    w32(ctx, 0x1017b8c4, flags)
    w32(ctx, 0x1015eca8, newbuf(ctx, 8))
    tab = newbuf(ctx, 0x40)
    words = ctx.buf(0x20040, 'zero')
    for k in range(8):
        w16(ctx, words + 2 * k, r.getrandbits(16))
        w16(ctx, words + 0x20000 + 2 * k, r.getrandbits(16))
    w32(ctx, tab + 0x16, words)
    w32(ctx, 0x1015d004, tab)
    w32(ctx, 0x109b98c0, ctx.buf(16, 'text'))
    gears = []
    for i in range(32):
        g_ = newbuf(ctx, 0x180)
        w32(ctx, g_ + 4, i)
        w32(ctx, g_ + 0xc, r.choice([0, -1, r.randrange(0, 32)]))
        w32(ctx, g_ + 0x14, r.choice([3, 3, 1]))
        regen = newbuf(ctx, 4)
        w8(ctx, regen, r.choice([0, 1, 3]))
        w32(ctx, g_ + 0x168, regen)
        wb(ctx, g_ + 0x124, b'Unit%d\0' % i)
        gears.append(g_)
        w32(ctx, 0x109e8b40 + 4 * i, g_ if (r.random() < 0.95 or i == own) else 0)
    msg = newbuf(ctx, 8)
    w8(ctx, msg + 2, r.choice([own, own + 8, r.randrange(0, 16), r.randrange(8, 32)]))
    if NEG:
        w32(ctx, frame(1) - 4, gears[r.randrange(32)])
    else:
        w32(ctx, frame(1) - 4, frame(1) - 0x800)
    return [msg]


SPECS = [dict(name='Sim_10040af8', orig=0x10040af8, c='Sim_10040af8', gen=g, void=True, trials=300 if NEG else 8000)]
