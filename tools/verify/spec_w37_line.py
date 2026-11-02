# DLL removal step 1: the graphics library's 8-bit line routine 0x100e86a1 (src/sim/gfx/w37_line.c) against the
# original, as spec_w30_vfx.py does for the 16-bit one: random windows and panes, ends inside / around / far
# outside, straight, diagonal and steep lines; modes 0 (colour), 1 (a 256-byte table) and negative ones (drawn as
# 0). Modes above 1 (a register callback, no caller) are left out. The straight lines' unset return flag [ebp-0x34]
# is seeded 0, the C's value. The whole buffer is compared. 1.2's twin is identical apart from addresses.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w37_common.py').read())

SOURCES = closure(['sim/gfx/w37_line.c'])
EBP7 = STACK_HI - 0x100 - 4 * 7 - 4 - 4


def g_line(ctx):
    r = start(ctx)
    win, cells, rows = window(ctx, r)
    p = pane(ctx, r, win, cells, rows)
    w32(ctx, EBP7 - 0x34, 0)
    x0, y0 = coord(r, cells), coord(r, rows)
    kind = r.random()
    if kind < 0.15:
        x1, y1 = x0, coord(r, rows)
    elif kind < 0.3:
        x1, y1 = coord(r, cells), y0
    elif kind < 0.4:
        d = r.randrange(-300, 300)
        x1, y1 = (x0 + d) & 0xffffffff, (y0 + r.choice([d, -d])) & 0xffffffff
    else:
        x1, y1 = coord(r, cells), coord(r, rows)
    mode = r.choice([0, 0, 0, 0, 1, -1 & 0xffffffff])
    colour = newbuf(ctx, 0x100) if mode == 1 else r.getrandbits(32)
    return [p, x0, y0, x1, y1, mode, colour]


SPECS = [dict(name='Sim_100e86a1', orig=0x100e86a1, c='Sim_100e86a1', gen=g_line, trials=30000)]


def g_ellipse(ctx):
    r = start(ctx)
    win, cells, rows = window(ctx, r)
    p = pane(ctx, r, win, cells, rows)
    w32(ctx, EBP7 - 0x34, 0)          # the line's slot, for the radius-0 calls (their frame sits lower; harmless)
    rx = r.choice([0, r.randrange(1, 8), r.randrange(1, 120), r.randrange(-3, 0)])
    ry = r.choice([0, r.randrange(1, 8), r.randrange(1, 90), r.randrange(-3, 0)])
    return [p, coord(r, cells), coord(r, rows), rx & 0xffffffff, ry & 0xffffffff, r.getrandbits(32)]


SPECS += [dict(name='Sim_100eb6e8', orig=0x100eb6e8, c='Sim_100eb6e8', gen=g_ellipse, trials=5000),
          dict(name='Sim_100eba29', orig=0x100eba29, c='Sim_100eba29', gen=g_ellipse, trials=5000)]
