# Round 30: the sim's 2D drawing library in C (src/sim/render/w30_vfx.c) against the original routines. Windows
# {buffer, pitch in bytes - 1, height - 1} of random sizes, panes inside and partly outside them, random ends and
# colours; the whole buffer is compared. The line's unset return flag [ebp-0x34] (straight lines) is seeded 0, the
# C's value. W30_NEG=1 builds the routines' planted bugs (W30*_MUTATE).
exec(open('tools/verify/w10_common.py').read())

if os.environ.get('W30_NEG'):
    CFLAGS.extend(['-DW30L_MUTATE', '-DW30W_MUTATE', '-DW30S_MUTATE', '-DW30C_MUTATE', '-DW30T_MUTATE',
                   '-DW30E_MUTATE'])

SOURCES = closure(['sim/render/w30_vfx.c'])
ACTIVE = ()

from unicorn.x86_const import UC_X86_REG_GDTR, UC_X86_REG_SS, UC_X86_REG_DS, UC_X86_REG_ES
GDT = 0x00F00F80             # inside the stub page the checker maps


def flat(ctx):
    """The library's routines push and pop ES: a flat 32-bit descriptor table (null, code, data) and data selectors
    loaded, so those segment loads work and the stack stays 32-bit (Unicorn has no table by default)."""
    m = ctx.m
    if getattr(m, '_w30gdt', False):
        return
    m._w30gdt = True
    m.raw.mem_write(GDT, struct.pack('<3Q', 0, 0x00CF9A000000FFFF, 0x00CF92000000FFFF))
    m.raw.reg_write(UC_X86_REG_GDTR, (0, GDT, 0x17, 0))
    for reg in (UC_X86_REG_SS, UC_X86_REG_DS, UC_X86_REG_ES):
        m.raw.reg_write(reg, 0x10)


def window(ctx, r, small=False, big=False):
    cells = r.choice([640, 320] if big else [640, 640, 320, 64, 100])
    rows = r.choice([200, 480] if big else [200, 40, 64] if small else [480, 480, 200, 40, 64])
    pitch = cells * 2
    buf = newbuf(ctx, pitch * (rows + 2) + 64)
    wb(ctx, buf, rb(ctx, pitch * (rows + 2) + 64))
    win = newbuf(ctx, 12)
    w32(ctx, win, buf, r.choice([pitch - 1, pitch - 1, pitch - 1, 0, -1 & 0xffffffff]),
        r.choice([rows - 1, rows - 1, rows - 1, -1 & 0xffffffff]))
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
    return r.choice([r.randrange(0, n), r.randrange(0, n), r.randrange(-n, 2 * n), r.randrange(-3, 4)]) & 0xffffffff


EBP7 = STACK_HI - 0x100 - 4 * 7 - 4 - 4


def g_line(ctx):
    r = begin(ctx, *ACTIVE)
    flat(ctx)
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
    return [p, x0, y0, x1, y1, r.choice([0, 0, 0, 0, 1, 2]), r.getrandbits(32)]


def lut(ctx, r):
    wb(ctx, 0x10156dcc, rb(ctx, 0x200))


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


def make_shape(r):
    x0, y0 = r.randrange(-20, 20), r.randrange(-20, 20)
    w, h = r.randrange(1, 90), r.choice([r.randrange(1, 60), 1, 0, -2])
    rows = b''.join(rle_row(r, w) for _ in range(max(h, 0) + 2))
    return struct.pack('<6i', 0, 0, x0, y0, x0 + w - 1, y0 + h - 1) + rows


def put(ctx, data):
    p = newbuf(ctx, len(data))
    wb(ctx, p, data)
    return p


def g_wipe(ctx):
    r = begin(ctx, *ACTIVE)
    flat(ctx)
    win, cells, rows = window(ctx, r)
    return [pane(ctx, r, win, cells, rows), r.getrandbits(32)]


def g_shape(ctx):
    r = begin(ctx, *ACTIVE)
    flat(ctx)
    lut(ctx, r)
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


def g_shape_raw(ctx):
    r = begin(ctx, *ACTIVE)
    flat(ctx)
    lut(ctx, r)
    win, cells, rows = window(ctx, r, big=True)   # wholly inside, as the clipped routine hands a shape over
    p = newbuf(ctx, 20)
    w32(ctx, p, win, r.randrange(0, 40), r.randrange(0, 20), cells - 1, rows - 1)
    sh = make_shape(r)
    x0, y0, x1, y1 = struct.unpack_from('<4i', sh, 8)
    # inside the window, as the clipped routine hands it over
    sh = sh[:8] + struct.pack('<4i', 0, 0, x1 - x0, max(y1 - y0, -1)) + sh[24:]
    s = put(ctx, sh)
    return [p, s, r.randrange(0, max(1, cells - 170)), r.randrange(0, max(1, rows - 85)), r.getrandbits(32)]


def make_font(ctx, r):
    h = r.choice([r.randrange(1, 16), 8])   # a height of 0 makes the original's row count wrap (no real font)
    n = 0x260                                # two-byte codes reach glyph 0x1ff + 0x2f + 0xff
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


def two_byte(ctx, r):
    tab = bytearray(0x200)
    for k in range(0x100):
        if r.random() < 0.2:
            struct.pack_into('<H', tab, 2 * k, r.randrange(1, 0x30))
    w32(ctx, 0x10157590, put(ctx, bytes(tab)))
    w32(ctx, 0x10157594, r.choice([1, 2]))
    w8(ctx, 0x10157598, r.choice([0xff, r.randrange(256), r.randrange(256)]))


def g_char(ctx):
    r = begin(ctx, *ACTIVE)
    flat(ctx)
    lut(ctx, r)
    win, cells, rows = window(ctx, r, small=True)
    p = pane(ctx, r, win, cells, rows)
    return [p, coord(r, cells), coord(r, rows), make_font(ctx, r), r.randrange(0, 0x100), lookaside(ctx, r)]


def g_string(ctx):
    r = begin(ctx, *ACTIVE)
    flat(ctx)
    lut(ctx, r)
    win, cells, rows = window(ctx, r, small=True)
    p = pane(ctx, r, win, cells, rows)
    return [p, coord(r, cells), coord(r, rows), make_font(ctx, r), text(ctx, r), lookaside(ctx, r)]


def g_char2(ctx):
    r = begin(ctx, *ACTIVE)
    flat(ctx)
    two_byte(ctx, r)
    win, cells, rows = window(ctx, r, small=True)
    p = pane(ctx, r, win, cells, rows)
    return [p, coord(r, cells), coord(r, rows), make_font(ctx, r), r.randrange(0, 0x10000),
            r.choice([0, lookaside(ctx, r)])]


def g_string2(ctx):
    r = begin(ctx, *ACTIVE)
    flat(ctx)
    two_byte(ctx, r)
    win, cells, rows = window(ctx, r, small=True)
    p = pane(ctx, r, win, cells, rows)
    return [p, coord(r, cells), coord(r, rows), make_font(ctx, r), text(ctx, r), r.choice([0, lookaside(ctx, r)])]


SPECS = [dict(name='Sim_100ef727', orig=0x100ef727, c='Sim_100ef727', gen=g_line, trials=30000),
         dict(name='Sim_100f0865', orig=0x100f0865, c='Sim_100f0865', gen=g_wipe, trials=3000),
         dict(name='Sim_100f0205', orig=0x100f0205, c='Sim_100f0205', gen=g_shape, trials=20000),
         dict(name='Sim_100f0727', orig=0x100f0727, c='Sim_100f0727', gen=g_shape_raw, trials=5000),
         dict(name='Sim_100f1c93', orig=0x100f1c93, c='Sim_100f1c93', gen=g_char, trials=10000),
         dict(name='Sim_100f1e10', orig=0x100f1e10, c='Sim_100f1e10', gen=g_string, trials=5000),
         dict(name='Sim_100f2d3c', orig=0x100f2d3c, c='Sim_100f2d3c', gen=g_char2, trials=10000),
         dict(name='Sim_100f2f01', orig=0x100f2f01, c='Sim_100f2f01', gen=g_string2, trials=5000)]
