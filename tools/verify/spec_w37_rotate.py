# DLL removal step 1: the graphics library's rotated / scaled shape draw 0x100e9cd1 (src/sim/gfx/w37_rotate.c)
# against the original. Random shape tables (run-length rows, origins inside and outside the bounds), scratch
# images, panes partly outside their windows, angles over the whole turn (and the untouched identity), scale
# factors from 1/4 to 3, flags 0..3 (remap table 0x10154eac random). Its callees (shape draws, fill, rotation,
# shape size / origin) run for real on both sides (the C its C). Compared: the whole writable memory (the window,
# the scratch image, the vertex table 0x10154fac and the step table 0x10154ffc). Sizes stay where the original's
# divisions cannot overflow (spans below 32768). 1.2's twin is identical apart from addresses.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w37_common.py').read())

SOURCES = closure(['sim/gfx/w37_rotate.c'])


def rle_row(r, width):
    out = bytearray()
    col = 0
    while col < width and r.random() < 0.85:
        k = r.random()
        if k < 0.2:
            n = r.randrange(1, 8)
            out += bytes([1, n])
        elif k < 0.6:
            n = r.randrange(1, 20)
            out += bytes([n * 2, r.randrange(256)])
        else:
            n = r.randrange(1, 12)
            out += bytes([n * 2 + 1]) + bytes(r.randrange(256) for _ in range(n))
        col += n
    out.append(0)
    return bytes(out)


def make_shape(r):
    w, h = r.randrange(1, 48), r.randrange(1, 40)
    x0 = r.choice([0, r.randrange(-w, 1), r.randrange(-60, 20)])
    y0 = r.choice([0, r.randrange(-h, 1), r.randrange(-60, 20)])
    rows = b''.join(rle_row(r, w) for _ in range(h + 2))
    return struct.pack('<6i', 0, 0, x0, y0, x0 + w - 1, y0 + h - 1) + rows, w, h


def g_rotate(ctx):
    r = start(ctx)
    wb(ctx, 0x10154eac, rb(ctx, 0x100))
    win, cells, rows = window(ctx, r, cells=r.choice([320, 100, 64, 178]), rows=r.choice([200, 64, 178, 40]))
    p = pane(ctx, r, win, cells, rows)
    shapes = [make_shape(r) for _ in range(2)]
    table = bytearray(8 + 8 * 2)
    off = len(table)
    for k, (sh, w, h) in enumerate(shapes):
        struct.pack_into('<I', table, 8 + 8 * k, off)
        off += len(sh)
    t = put(ctx, bytes(table) + b''.join(sh for sh, _, _ in shapes))
    k = r.randrange(0, 2)
    w, h = shapes[k][1], shapes[k][2]
    image = newbuf(ctx, w * h + 64)
    if r.random() < 0.1:
        angle, sx, sy = 0, 0x10000, 0x10000
    else:
        angle = r.choice([0, 900, 1800, 2700, r.randrange(0, 3600), r.randrange(-4000, 8000)])
        sx = r.choice([0x10000, r.randrange(0x4000, 0x30000)])
        sy = r.choice([sx, 0x10000, r.randrange(0x4000, 0x30000)])
    return [p, t, k, coord(r, cells), coord(r, rows), image, angle & 0xffffffff, sx, sy, r.randrange(0, 4)]


SPECS = [dict(name='Sim_100e9cd1', orig=0x100e9cd1, c='Sim_100e9cd1', gen=g_rotate, trials=5000)]
