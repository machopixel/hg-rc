# DLL removal step 1: the graphics library's run-length shape encoder (src/sim/gfx/w37_encode.c): the pane encoder
# 0x100ea9f5, its row routine 0x100ead0a and packet writer 0x100eae93, against the original routines. Panes of
# transparent cells with runs, mixed cells and single cells; measuring (no output) and writing. The row routine and
# the packet writer are also run alone on random encoder states (the globals 0x1015365d..0x10153691). Callees run
# for real on both sides (the C its C). The whole writable memory is compared (the encoder's globals, the output).
# 1.2's twins are identical apart from addresses.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w37_common.py').read())

SOURCES = closure(['sim/gfx/w37_encode.c'])


def cells(r, n, key):
    out = bytearray()
    while len(out) < n:
        k = r.random()
        m = r.randrange(1, 20) if r.random() < 0.8 else r.randrange(100, 300)
        if k < 0.35:
            out += bytes([key]) * m
        elif k < 0.6:
            out += bytes([r.randrange(256)]) * m
        elif k < 0.8:
            out += bytes(r.randrange(256) for _ in range(m))
        else:
            out += bytes(r.choice([key, r.randrange(256), r.randrange(4)]) for _ in range(m))
    return bytes(out[:n])


def g_pane(ctx):
    r = start(ctx)
    key = r.randrange(256)
    w, h = r.choice([64, 100, 33, 300]), r.choice([13, 40, 3])
    win, cw, ch = window(ctx, r, cells=w, rows=h)
    if r.random() < 0.1:
        data = bytes([key]) * (w * (h + 2))
    else:
        data = cells(r, w * (h + 2), key)
    wb(ctx, r32(ctx, win), data)
    p = pane(ctx, r, win, cw, ch)
    out = r.choice([0, newbuf(ctx, (w * 3 + 16) * (h + 2) + 0x40)])
    return [p, key | (r.getrandbits(24) << 8), coord(r, cw), coord(r, ch), out]


ENC = 0x1015365d


def state(ctx, r, src):
    out = r.choice([0, newbuf(ctx, 0x2000)])
    w32(ctx, ENC, out)
    w32(ctx, ENC + 4, r.choice([0, r.randrange(1, 300), r.randrange(1, 4)]))
    w32(ctx, ENC + 0x10, out if out else r.choice([0x18, 0x40]))
    w32(ctx, ENC + 0x28, r.choice([0x7fffffff, r.randrange(0, 100)]), r.choice([0x7fffffff, r.randrange(0, 100)]),
        r.choice([0x80000001, r.randrange(0, 200)]), r.choice([0x80000001, r.randrange(0, 100)]))


def g_packet(ctx):
    r = start(ctx)
    buf = newbuf(ctx, 0x800)
    src = buf + r.randrange(0, 0x100)
    state(ctx, r, src)
    w32(ctx, ENC + 8, buf)                                   # the row start
    w32(ctx, ENC + 0x14, src)                                # the source position
    back = r.choice([0, 1, 3, r.randrange(4)])
    # its end: never before the source plus `back`, as the row routine leaves it (a negative count makes both
    # versions copy until they fault, the C only more slowly)
    span = r.choice([r.randrange(0, 4), r.randrange(0, 300)])
    w32(ctx, ENC + 0xc, src + back + span)
    # the pending transparent count: at most the span (mode 3 sets it from the same span, the source still before it)
    w32(ctx, ENC + 4, r.choice([0, r.randrange(0, span + 1)]))
    return [r.choice([0, 1, 2, 3, 4, 5, r.randrange(6)]), back, r.choice([0, r.randrange(0, 100)])]


def g_row(ctx):
    r = start(ctx)
    key = r.randrange(256)
    n = r.choice([0, 1, 2, 3, r.randrange(1, 40), r.randrange(1, 400)])
    row = put(ctx, cells(r, n + 8, key))
    state(ctx, r, row)
    w32(ctx, ENC + 8, row)
    w32(ctx, ENC + 0x14, row)
    return [n, key | (r.getrandbits(24) << 8), r.choice([0, r.randrange(0, 100)])]


SPECS = [dict(name='Sim_100eae93', orig=0x100eae93, c='Sim_100eae93', gen=g_packet, trials=5000, void=True),
         dict(name='Sim_100ead0a', orig=0x100ead0a, c='Sim_100ead0a', gen=g_row, trials=5000, void=True),
         dict(name='Sim_100ea9f5', orig=0x100ea9f5, c='Sim_100ea9f5', gen=g_pane, trials=3000)]
