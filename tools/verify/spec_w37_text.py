# DLL removal step 1: the graphics library's 8-bit character and string routines (src/sim/gfx/w37_text.c) against
# the original routines, on spec_w30_vfx.py's kind of windows, panes, fonts and lookasides (8-bit windows: the
# pitch is the row's byte count). The whole buffer is compared. 1.2's twins are identical apart from addresses.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w37_common.py').read())

SOURCES = closure(['sim/gfx/w37_text.c'])


def window(ctx, r):
    cells = r.choice([640, 320, 64, 100, 33])
    rows = r.choice([200, 40, 64, 13])
    buf = newbuf(ctx, cells * (rows + 2) + 64)
    win = newbuf(ctx, 12)
    w32(ctx, win, buf, r.choice([cells - 1, cells - 1, cells - 1, 0, -1 & 0xffffffff]),
        r.choice([rows - 1, rows - 1, rows - 1, 0, -1 & 0xffffffff]))
    return win, cells, rows


def pane(ctx, r, win, cells, rows):
    p = newbuf(ctx, 20)
    x0 = r.choice([0, 0, r.randrange(-20, cells), r.randrange(0, cells)])
    y0 = r.choice([0, 0, r.randrange(-20, rows), r.randrange(0, rows)])
    x1 = r.choice([cells - 1, cells - 1, r.randrange(-5, cells + 20)])
    y1 = r.choice([rows - 1, rows - 1, r.randrange(-5, rows + 20)])
    w32(ctx, p, win, x0 & 0xffffffff, y0 & 0xffffffff, x1 & 0xffffffff, y1 & 0xffffffff)
    return p


def coord(r, n):
    return r.choice([r.randrange(0, n), r.randrange(0, n), r.randrange(-n, 2 * n), r.randrange(-3, 4),
                     r.randrange(-40, 0)]) & 0xffffffff


def make_font(ctx, r):
    h = r.choice([r.randrange(1, 16), 8])     # a height of 0 makes the row count wrap (no real font)
    n = 0x100
    glyphs = []
    for _ in range(n):
        w = r.choice([0, r.randrange(1, 12), r.randrange(1, 30)])
        glyphs.append(struct.pack('<i', w) + rb(ctx, w * h + 8))
    head = bytearray(0x10 + 4 * n)
    struct.pack_into('<i', head, 8, h)
    off = len(head)
    for k, gl in enumerate(glyphs):
        struct.pack_into('<I', head, 0x10 + 4 * k, off)
        off += len(gl)
    return put(ctx, bytes(head) + b''.join(glyphs))


def lookaside(ctx, r):
    t = bytearray(rb(ctx, 256))
    for k in range(256):
        if r.random() < 0.3:
            t[k] = 0xff
    return put(ctx, bytes(t))


def text(ctx, r):
    n = r.randrange(0, 12)
    return put(ctx, bytes(r.randrange(1, 256) for _ in range(n)) + bytes(2))


def g_char(ctx):
    r = start(ctx)
    win, cells, rows = window(ctx, r)
    p = pane(ctx, r, win, cells, rows)
    return [p, coord(r, cells), coord(r, rows), make_font(ctx, r), r.randrange(0, 0x100),
            r.choice([0, lookaside(ctx, r)])]


def g_string(ctx):
    r = start(ctx)
    win, cells, rows = window(ctx, r)
    p = pane(ctx, r, win, cells, rows)
    return [p, coord(r, cells), coord(r, rows), make_font(ctx, r), text(ctx, r), r.choice([0, lookaside(ctx, r)])]


SPECS = [dict(name='Sim_100ecd13', orig=0x100ecd13, c='Sim_100ecd13', gen=g_char, trials=10000),
         dict(name='Sim_100ecea6', orig=0x100ecea6, c='Sim_100ecea6', gen=g_string, trials=3000)]
