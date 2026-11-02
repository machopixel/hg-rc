# DLL removal step 1: the graphics library's row copy 0x100ecedd and GIF decoder 0x100ed491 (with its five
# register-argument helpers 0x100ed327 / 0x100ed36f / 0x100ed388 / 0x100ed3ce / 0x100ed414, static functions of
# src/sim/gfx/w37_gif.c, proven through it) against the original routines. The GIF files are real LZW streams
# (encoded here, code size growth as the decoder sees it, clears when the table fills), with or without global /
# local colour tables, interlaced or not, sometimes with corrupted data bytes; random panes. The decoder runs the
# row copy for real on both sides (the C its C). Compared: the whole writable memory (the window's buffer, the
# state record, the row buffer 0x10153b95, [0x10154ea8]). 1.2's twins are identical apart from addresses.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w37_common.py').read())

SOURCES = closure(['sim/gfx/w37_gif.c'])


def g_row(ctx):
    r = start(ctx)
    win, cells, rows = window(ctx, r)
    p = pane(ctx, r, win, cells, rows)
    n = r.choice([r.randrange(0, cells + 40), r.randrange(1, 20)])
    return [p, coord(r, rows), newbuf(ctx, max(n, 1) + 64), n]


def lzw(r, pixels, minimum):
    """GIF LZW codes as (code, bits), with the decoder's code size: its table lags the encoder's by one entry"""
    clear, eoi = 1 << minimum, (1 << minimum) + 1
    out = []

    def reset():
        return {(i,): i for i in range(clear)}, eoi + 1, minimum + 1, clear * 2, 0

    table, enext, size, limit, emitted = reset()
    out.append((clear, size))
    w = ()
    for p in pixels:
        wk = w + (p,)
        if wk in table:
            w = wk
            continue
        out.append((table[w], size))
        emitted += 1
        if emitted > 1:              # the decoder adds an entry for every code after the first
            dnext = eoi + emitted
            if dnext == limit and size < 12:
                size += 1
                limit <<= 1
        table[wk] = enext
        enext += 1
        w = (p,)
        if enext >= 4000 or r.random() < 0.002:
            out.append((table[w], size))
            out.append((clear, size))
            table, enext, size, limit, emitted = reset()
            w = ()
    if w:
        out.append((table[w], size))
    out.append((eoi, size))
    return out


def pack(codes):
    acc, n, data = 0, 0, bytearray()
    for c, bits in codes:
        acc |= c << n
        n += bits
        while n >= 8:
            data.append(acc & 0xff)
            acc >>= 8
            n -= 8
    if n:
        data.append(acc & 0xff)
    return bytes(data)


def gif(ctx, r):
    w, h = r.randrange(1, 48), r.randrange(1, 32)
    minimum = r.choice([2, 2, 3, 4, 8, r.randrange(2, 9)])
    colours = 1 << minimum
    pixels, last = [], 0
    for _ in range(w * h):
        if r.random() < 0.5:
            last = r.randrange(colours)
        pixels.append(last)
    data = bytearray(pack(lzw(r, pixels, minimum)))
    if r.random() < 0.1:
        for _ in range(r.randrange(1, 4)):
            data[r.randrange(len(data))] = r.getrandbits(8)
    blocks = bytearray()
    for k in range(0, len(data), 255):
        part = data[k:k + 255]
        blocks += bytes([len(part)]) + part
    blocks += bytes([0])
    gflags = r.choice([0, 0x80 | r.randrange(8)]) | (r.getrandbits(7) & 0x78)
    head = b'GIF89a' + struct.pack('<HH', w, h) + bytes([gflags, r.getrandbits(8), 0])
    if gflags & 0x80:
        head += rb(ctx, (1 << ((gflags & 7) + 1)) * 3)
    lflags = r.choice([0, 0x40, 0x80 | r.randrange(8), 0xc0 | r.randrange(8)])
    head += b'\x2c' + struct.pack('<HHHH', r.getrandbits(16), r.getrandbits(16), w, h) + bytes([lflags])
    if lflags & 0x80:
        head += rb(ctx, (1 << ((lflags & 7) + 1)) * 3)
    return put(ctx, head + bytes([minimum]) + bytes(blocks) + rb(ctx, 64)), w, h


def g_gif(ctx):
    r = start(ctx)
    g, w, h = gif(ctx, r)
    win, cells, rows = window(ctx, r)
    p = pane(ctx, r, win, cells, rows)
    return [p, g, newbuf(ctx, 0x502e + 0x40)]


SPECS = [dict(name='Sim_100ecedd', orig=0x100ecedd, c='Sim_100ecedd', gen=g_row, trials=5000),
         dict(name='Sim_100ed491', orig=0x100ed491, c='Sim_100ed491', gen=g_gif, trials=1500)]
