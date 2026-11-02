# DLL removal step 1: the graphics library's 8-bit drawing routines (src/sim/gfx/w37_draw.c) against the original
# routines: random windows and panes (inside, partly outside, empty), coordinates in and around them. The whole
# writable memory is compared. 1.2's twins are identical apart from addresses.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w37_common.py').read())

SOURCES = closure(['sim/gfx/w37_draw.c'])


def g_pixel(ctx):
    r = start(ctx)
    win, cells, rows = window(ctx, r)
    p = pane(ctx, r, win, cells, rows)
    return [p, coord(r, cells), coord(r, rows), r.getrandbits(32)]


def g_read(ctx):
    r = start(ctx)
    win, cells, rows = window(ctx, r)
    p = pane(ctx, r, win, cells, rows)
    return [p, coord(r, cells), coord(r, rows)]


SPECS = [dict(name='Sim_100e84f0', orig=0x100e84f0, c='Sim_100e84f0', gen=g_pixel, trials=5000),
         dict(name='Sim_100e85cb', orig=0x100e85cb, c='Sim_100e85cb', gen=g_read, trials=5000)]


def g_fill(ctx):
    r = start(ctx)
    win, cells, rows = window(ctx, r)
    return [pane(ctx, r, win, cells, rows), r.getrandbits(32)]


def rle_row(r, width):
    out = bytearray()
    col = 0
    while col < width and r.random() < 0.85:
        k = r.random()
        if k < 0.2:
            n = r.randrange(1, 12)
            out += bytes([1, n])
        elif k < 0.6:
            n = r.randrange(1, 40)
            out += bytes([n * 2, r.randrange(256)])
        else:
            n = r.randrange(1, 20)
            out += bytes([n * 2 + 1]) + bytes(r.randrange(256) for _ in range(n))
        col += n
    out.append(0)
    return bytes(out)


def make_shape(r, inside=False):
    x0, y0 = (0, 0) if inside else (r.randrange(-20, 20), r.randrange(-20, 20))
    w, h = r.randrange(1, 90), r.choice([r.randrange(1, 60), 1, 0, -2])
    rows = b''.join(rle_row(r, w) for _ in range(max(h, 0) + 2))
    return struct.pack('<6i', 0, 0, x0, y0, x0 + w - 1, y0 + h - 1) + rows


def g_shape_raw(ctx):
    r = start(ctx)
    win, cells, rows = window(ctx, r, cells=r.choice([640, 320]), rows=r.choice([200, 480]))
    p = newbuf(ctx, 20)
    w32(ctx, p, win, r.randrange(0, 40), r.randrange(0, 20), cells - 1, rows - 1)
    return [p, put(ctx, make_shape(r, True)), r.randrange(0, max(1, cells - 170)), r.randrange(0, max(1, rows - 85)),
            r.getrandbits(32)]


SPECS += [dict(name='Sim_100eb070', orig=0x100eb070, c='Sim_100eb070', gen=g_fill, trials=3000),
          dict(name='Sim_100e95e0', orig=0x100e95e0, c='Sim_100e95e0', gen=g_shape_raw, trials=5000)]


def g_shape(ctx):
    r = start(ctx)
    win, cells, rows = window(ctx, r)
    p = pane(ctx, r, win, cells, rows)
    shapes = [make_shape(r) for _ in range(3)]
    table = bytearray(8 + 8 * 3)
    off = len(table)
    for k, sh in enumerate(shapes):
        struct.pack_into('<I', table, 8 + 8 * k, off)
        off += len(sh)
    t = put(ctx, bytes(table) + b''.join(shapes))
    return [p, t, r.randrange(0, 3), coord(r, cells), coord(r, rows)]


SPECS += [dict(name='Sim_100e91dc', orig=0x100e91dc, c='Sim_100e91dc', gen=g_shape, trials=20000)]


def g_checker(ctx):
    r = start(ctx)
    win, cells, rows = window(ctx, r)
    p = pane(ctx, r, win, cells, rows)
    x0, y0 = coord(r, cells), coord(r, rows)
    if r.random() < 0.3:
        x1, y1 = (x0 + r.randrange(-2, 4)) & 0xffffffff, (y0 + r.randrange(-2, 4)) & 0xffffffff
    else:
        x1, y1 = coord(r, cells), coord(r, rows)
    return [p, x0, y0, x1, y1, r.getrandbits(32)]


SPECS += [dict(name='Sim_100e90a3', orig=0x100e90a3, c='Sim_100e90a3', gen=g_checker, trials=5000)]


def remap(ctx):
    wb(ctx, 0x10154eac, rb(ctx, 0x100))


def g_shape_raw_map(ctx):
    args = g_shape_raw(ctx)
    remap(ctx)
    return args


def g_shape_map(ctx):
    args = g_shape(ctx)
    remap(ctx)
    return args


SPECS += [dict(name='Sim_100e9b9e', orig=0x100e9b9e, c='Sim_100e9b9e', gen=g_shape_raw_map, trials=5000),
          dict(name='Sim_100e96c6', orig=0x100e96c6, c='Sim_100e96c6', gen=g_shape_map, trials=20000)]
