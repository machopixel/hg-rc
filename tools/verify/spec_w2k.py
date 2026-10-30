# Differential tests for the wave 2 functions of letter k (see difftest.py).
import struct

SOURCES = [
    'sim/unsorted_1000/w2k_misc.c', 'sim/unsorted_1001/w2k_misc.c', 'sim/unsorted_1002/w2k_misc.c',
    'sim/unsorted_1003/w2k_misc.c', 'sim/unsorted_1004/fields.c', 'sim/unsorted_1004/w2k_flags.c',
    'sim/unsorted_1004/w2k_hash.c', 'sim/unsorted_1005/w2k_misc.c', 'sim/unsorted_1006/w2k_init.c',
    'sim/unsorted_1007/w2k_misc.c', 'sim/unsorted_1008/w2k_misc.c', 'sim/unsorted_1009/w2k_misc.c',
    'sim/unsorted_100a/w2k_list.c', 'sim/unsorted_100d/w2k_list.c', 'sim/unsorted_100f/w2k_string.c',
    'sim/unsorted_1010/w2k_crt.c',
]

ARGMEM_TOP = 0xAF0000   # trampoline code lives here (ARGMEM is 0xA00000..0xB00000)


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def rd32(ctx, addr, n=1):
    return list(struct.unpack('<%dI' % n, bytes(ctx.m.uc.mem_read(addr, 4 * n))))


def rbytes(ctx, n):
    return bytes(ctx.rng.getrandbits(8) for _ in range(n))


def rw(ctx):
    """A dword that exercises sign, zero and ordinary values."""
    r = ctx.rng
    return r.choice([0, 1, 0xffffffff, 0x80000000, 0x7fffffff, r.getrandbits(32), r.getrandbits(32), r.randrange(0, 64)])


def g_none(ctx):
    return []


# ---- 0x10008e8b ----------------------------------------------------------------
def g_8e8b(ctx):
    r = ctx.rng
    obj = ctx.buf(0x40)
    target = ctx.buf(8)
    w32(ctx, obj + 0x30, 0 if r.random() < 0.3 else target)
    return [obj, rw(ctx)]


# ---- 0x100093ea ----------------------------------------------------------------
def g_93ea(ctx):
    r = ctx.rng
    obj = ctx.buf(8)
    if r.random() < 0.1:
        return [0]
    w16(ctx, obj, r.choice([0, 0xffff, 0x7ef0, 0x8100, r.getrandbits(16), r.getrandbits(16)]))
    return [obj]


# ---- 0x1000c627 ----------------------------------------------------------------
def g_c627(ctx):
    r = ctx.rng
    recs = [ctx.buf(8) for _ in range(4)]
    for rec in recs:
        w32(ctx, rec, r.choice([9, 9, 0, 1, 8, 10, rw(ctx)]))
    w32(ctx, 0x109e8b40, *[r.choice(recs) for _ in range(256)])
    if r.random() < 0.08:
        return [0]
    obj = ctx.buf(0x100)
    w32(ctx, obj + 0xf0, r.choice([0x200 | r.randrange(0, 256), 0x200 | r.randrange(0, 256), r.randrange(0, 256),
                                   rw(ctx), r.getrandbits(32) | 0x200]))
    return [obj]


# ---- 0x1000e060 ----------------------------------------------------------------
def g_e060(ctx):
    r = ctx.rng
    tag = rw(ctx)
    w32(ctx, 0x1012df58, tag)
    data = bytearray(rbytes(ctx, 96))
    if r.random() < 0.8:
        off = r.randrange(0, 70)
        data[off:off + 4] = struct.pack('<I', tag)
    if r.random() < 0.2:                       # an earlier near-miss
        off = r.randrange(0, 60)
        data[off:off + 4] = struct.pack('<I', tag ^ (1 << r.randrange(0, 32)))
    buf = ctx.buf(96, bytes(data))
    cur = ctx.buf(4, struct.pack('<I', buf))
    cnt = ctx.buf(4, struct.pack('<i', r.choice([0, 1, 4, 5, 8, 20, 70, r.randrange(-2, 72), r.randrange(0, 72)])))
    return [cur, cnt]


# ---- 0x1001083a ----------------------------------------------------------------
def g_083a(ctx):
    r = ctx.rng
    w32(ctx, 0x1013bb40, r.choice([0, 1, 2, 3, r.randrange(0, 6)]))
    return [r.choice([0, 0, 1, 2, 5, 17, r.randrange(0, 60), -1, 0x10000 + r.randrange(0, 5)])]


# ---- 0x10013fe1 (the original returns a 16-bit value; a trampoline clears eax first) --------
def g_3fe1(ctx):
    r = ctx.rng
    orig = 0x10013fe1
    code = bytes([0x31, 0xC0]) + bytes([0xFF, 0x74, 0x24, 0x10]) * 4
    rel = (orig - (ARGMEM_TOP + len(code) + 5)) & 0xffffffff
    code += bytes([0xE8]) + struct.pack('<I', rel) + bytes([0x83, 0xC4, 0x10, 0xC3])
    ctx.m.uc.mem_write(ARGMEM_TOP, code)
    sel = r.choice([0x40, 0x25, 0x2a, 0x40, 0x25, 0x2a, r.getrandbits(16), r.getrandbits(32),
                    (r.getrandbits(16) << 16) | r.choice([0x40, 0x25, 0x2a]), 0x41, 0x24, 0x2b])
    return [sel, rw(ctx), rw(ctx), rw(ctx)]


# ---- 0x100155b7 ----------------------------------------------------------------
def g_55b7(ctx):
    r = ctx.rng
    obj = ctx.buf(0x200)
    keys = []
    for i in range(3):
        if r.random() < 0.2:
            w32(ctx, obj + 0x174 + 4 * i, 0)
            continue
        n = r.choice([0, 1, 2, 3, 6, r.randrange(0, 8), 0xffff])       # 0xffff = a negative count
        entries = ctx.buf(40)
        recs = []
        for j in range(min(n, 8) if n < 0x8000 else 3):
            k = r.choice([r.randrange(-5, 6), r.getrandbits(16)])
            keys.append(k)
            recs.append(struct.pack('<hH', k if -32768 <= k < 32768 else k - 65536, r.getrandbits(16)))
        wb(ctx, entries, b''.join(recs))
        hdr = ctx.buf(8)
        w16(ctx, hdr, n)
        w32(ctx, hdr + 2, entries)
        w32(ctx, obj + 0x174 + 4 * i, hdr)
    key = r.choice(keys + [0, 1, -1]) if keys and r.random() < 0.7 else r.randrange(-6, 7)
    key = (key & 0xffff) | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0)
    return [obj, key]


# ---- 0x10015cd2 ----------------------------------------------------------------
def g_5cd2(ctx):
    r = ctx.rng
    recs = [ctx.buf(0x20) for _ in range(4)]
    w32(ctx, 0x109e8b40, *[0 if r.random() < 0.3 else r.choice(recs) for _ in range(300)])
    return [r.choice([r.randrange(0, 300), r.randrange(0, 10), 0xffffffff, r.randrange(0, 300)])]


# ---- 0x10027acf ----------------------------------------------------------------
def g_7acf(ctx):
    r = ctx.rng
    table = rd32(ctx, 0x101349e8, 0x61)
    if r.random() < 0.7:
        return [r.choice(table)]
    return [r.choice([0, 1, 0x445743, rw(ctx), table[0] ^ 0x20202020])]


# ---- setters 0x10028965 / 0x10028a15 / 0x10037c9e ------------------------------------
def g_set_obj(ctx):
    return [ctx.buf(0x100), rw(ctx)]


def g_set_g(ctx):
    return [rw(ctx)]


# ---- 0x1002dc46 ----------------------------------------------------------------
def g_dc46(ctx):
    r = ctx.rng
    obj = ctx.buf(0x40)
    w32(ctx, 0x10138bc8, *[r.choice([0, 0, 1, rw(ctx)]) if i % 3 == 0 else rw(ctx) for i in range(60)])
    w32(ctx, obj + 0x34, r.choice([r.randrange(0, 20), r.randrange(0, 20), 0, 0xffffffff, -1, 100]))
    return [obj]


# ---- 0x10037680 ----------------------------------------------------------------
def g_7680(ctx):
    r = ctx.rng
    w32(ctx, 0x109e38a0, *[r.choice([r.randrange(0, 32), r.randrange(0, 32), 0, 8, 16, 24, 32, 33, r.getrandbits(32)]) for _ in range(4)])
    w32(ctx, 0x109e3890, *[r.choice([r.randrange(0, 9), r.randrange(0, 9), 5, 6, 8, 0, 9, 16, 33, r.getrandbits(32)]) for _ in range(4)])
    return [rw(ctx)]


# ---- 0x1003d863 ----------------------------------------------------------------
def g_d863(ctx):
    w32(ctx, 0x1014cb3c, rw(ctx))
    w32(ctx, 0x1013abf0, rw(ctx))
    w32(ctx, 0x1015ebf4, rw(ctx))
    return []


# ---- global readers --------------------------------------------------------------
def make_reader(addr):
    def g(ctx):
        w32(ctx, addr, rw(ctx))
        return []
    return g


def g_87e6(ctx):
    r = ctx.rng
    rec = ctx.buf(0x100)
    w32(ctx, 0x1015d1b8, rec)
    return [rw(ctx)]


# ---- 0x10045f3b ----------------------------------------------------------------
def g_5f3b(ctx):
    r = ctx.rng
    w32(ctx, 0x1013af48, *[rw(ctx) for _ in range(19)])
    return [r.choice([r.randrange(0, 11), r.randrange(0, 11), 11, 12, 0xffffffff, 0x80000000, r.getrandbits(32)]), rw(ctx), rw(ctx)]


# ---- flag tests ------------------------------------------------------------------
def g_flags(ctx):
    r = ctx.rng
    return [r.choice([r.getrandbits(12), r.getrandbits(12), r.getrandbits(32), 0, 0xfff, 0x100, 0xe00 ^ r.getrandbits(1)])]


def make_sum(base, adds):
    def g(ctx):
        r = ctx.rng
        w32(ctx, base - 0x10 * 0xaa, *[rw(ctx) for _ in range(0x10 * 0xaa // 4 + 1)])
        for a in adds:
            w32(ctx, a, rw(ctx))
        row = r.choice([0, 1, 2, 5, r.randrange(0, 12), r.randrange(0, 12), -1, -3])
        return [row, r.choice([r.getrandbits(12), r.getrandbits(12), r.getrandbits(32), 0, 0xfff, 0xeff, 0x100])]
    return g


ADDS_CE64 = [0x10126300, 0x10126398, 0x10126430, 0x101264c8, 0x10126560, 0x101265f8, 0x10126690, 0x10126728, 0x101267c0, 0x10126858, 0x101268f0]
ADDS_CFB5 = [0x101262f4, 0x1012638c, 0x10126424, 0x101264bc, 0x10126554, 0x101265ec, 0x10126684, 0x1012671c, 0x101267b4, 0x1012684c, 0x101268e4]
ADDS_D106 = [0x101262dc, 0x10126374, 0x1012640c, 0x101264a4, 0x1012653c, 0x101265d4, 0x1012666c, 0x10126704, 0x1012679c, 0x10126834, 0x101268cc]


# ---- 0x1004db86 ----------------------------------------------------------------
def g_db86(ctx):
    r = ctx.rng
    return [r.choice([0, 1, 2, 3, 4, 5, 8, 9, 16, 17, 0x80000000, 0x80000001, 0xffffffff, r.getrandbits(32),
                      1 << r.randrange(0, 32), (1 << r.randrange(0, 32)) + r.randrange(-1, 2), r.randrange(0, 600)])]


# ---- 0x1004e582 ----------------------------------------------------------------
def g_e582(ctx):
    r = ctx.rng
    n = 9973
    heads = [0xffff if r.random() < r.choice([0.15, 0.5, 0.9]) else r.randrange(0, 40) for _ in range(n)]
    nent = 40
    keys = [r.choice([r.randrange(0, 20), r.getrandbits(16), r.getrandbits(32)]) for _ in range(nent)]
    bytes_ = [r.choice([0, 1, 7, r.getrandbits(8)]) for _ in range(nent)]
    hb = ctx.buf(n * 4, struct.pack('<%dI' % n, *heads))
    kb = ctx.buf(nent * 4, struct.pack('<%dI' % nent, *keys))
    bb = ctx.buf(nent, bytes(bytes_))
    w32(ctx, 0x1015f35c, hb)
    w32(ctx, 0x1015f358, kb)
    w32(ctx, 0x1015f354, bb)
    r2 = r.random()
    if r2 < 0.05:
        return [0xffff, rw(ctx)]
    # pick a target entry and make the first probe point at it, usually
    e = r.randrange(0, nent)
    key = keys[e] if r.random() < 0.8 else r.getrandbits(r.choice([8, 16, 32]))
    byte = bytes_[e] | (r.getrandbits(24) << 8 if r.random() < 0.3 else 0) if r.random() < 0.8 else r.getrandbits(r.choice([8, 32]))
    if key == 0xffff:
        key = 0xfffe
    h = (((byte & 0xff) << 8) ^ key) % n
    chain = r.randrange(0, 4)
    for _ in range(chain):
        heads[h] = r.randrange(0, nent)
        h = (h + 0x7f7) % n
    if r.random() < 0.85:
        heads[h] = e
    else:
        heads[h] = 0xffff
    wb(ctx, hb, struct.pack('<%dI' % n, *heads))
    return [key, byte]


# ---- 0x100526a9 ----------------------------------------------------------------
def g_26a9(ctx):
    r = ctx.rng
    limit = r.choice([0, 1, 3, 5, 10, 300, r.randrange(0, 12)])
    w32(ctx, 0x1015dc64, limit)
    recs = [ctx.buf(0x20) for _ in range(4)]
    w32(ctx, 0x109e8b40, *[r.choice(recs) for _ in range(300)])
    return [r.choice([r.randrange(0, 12), r.randrange(0, 12), limit, limit - 1, 0xffffffff, 0x80000000]) & 0xffffffff,
            r.choice([r.randrange(0, 10), r.randrange(0, 10), 8, 7, 0xffffffff, 0x80000000])]


# ---- 0x10052c30 ----------------------------------------------------------------
def g_2c30(ctx):
    r = ctx.rng
    for i in range(16):
        w32(ctx, 0x109a864c + i * 0x54, r.choice([r.randrange(0, 12), r.randrange(0, 12), 0, rw(ctx) if r.random() < 0.05 else 3]))
    w32(ctx, 0x109b7d54, *[rw(ctx) for _ in range(12 * 27)])
    return [r.choice([r.randrange(0, 18), r.randrange(0, 16), 15, 16, 17, 0xffffffff, 0x80000000, r.getrandbits(32)])]


def g_val(ctx):
    return [rw(ctx)]


# ---- 0x1005572c ----------------------------------------------------------------
def g_572c(ctx):
    r = ctx.rng
    w32(ctx, 0x10164bc8 - 0x400, *[rw(ctx) for _ in range(0x500 // 4)])
    return [r.choice([r.randrange(-300, 300), r.randrange(-300, 300), 0xffff, 0x8000, 0x7fff, r.getrandbits(32),
                      (r.getrandbits(16) << 16) | r.randrange(0, 200), (r.getrandbits(16) << 16) | (0x10000 - r.randrange(1, 200))])]


# ---- 0x10057a58 ----------------------------------------------------------------
def g_7a58(ctx):
    r = ctx.rng
    w32(ctx, 0x1015e650, rw(ctx), rw(ctx), rw(ctx), (rw(ctx) & ~0x1000000) | (0x1000000 if r.random() < 0.4 else 0))
    out = 0 if r.random() < 0.15 else ctx.buf(16)
    return [rw(ctx), rw(ctx), out]


# ---- 0x1005b7e0 ----------------------------------------------------------------
def g_b7e0(ctx):
    r = ctx.rng
    obj = ctx.buf(0xc10)
    k = r.choice([0, 1, 2, 3, r.randrange(0, 4)])
    w32(ctx, obj + 0xbdb, k if r.random() < 0.9 else r.choice([0, 1, 5, 6]))
    w32(ctx, 0x10138d70, *[r.choice([r.randrange(0, 7), r.randrange(0, 7), 0, 6]) for _ in range(17)])
    for kk in range(7):
        base = 0x10138db8 + kk * 0x24c
        w32(ctx, base, *[rw(ctx) for _ in range(7 * 20)])
        w32(ctx, base + 0x230 - 0, *[r.choice([0, 1, 2, 5, 10, 20, r.randrange(0, 21)]) for _ in range(7)])
    return [obj]


# ---- 0x1005b923 ----------------------------------------------------------------
def g_b923(ctx):
    r = ctx.rng
    return [r.choice([r.randrange(0, 20), r.randrange(6, 19), r.randrange(6, 19), 0xffffffff, 0x80000000, r.getrandbits(32)])]


# ---- 0x1006271a ----------------------------------------------------------------
def g_271a(ctx):
    w32(ctx, 0x10165be0 - 0x10, *[ctx.rng.choice([0, 1, rw(ctx)]) for _ in range(0x120 // 4)])
    return []


# ---- 0x10076689 ----------------------------------------------------------------
def g_6689(ctx):
    r = ctx.rng
    n = r.choice([0, 1, 2, 3, 5])
    nodes = [ctx.buf(0x20) for _ in range(n)]
    for i, nd in enumerate(nodes):
        w32(ctx, nd, rw(ctx), r.choice([1, 1, 0, 2, 0xffffffff, rw(ctx)]), rw(ctx), rw(ctx), rw(ctx),
            nodes[i + 1] if i + 1 < n else 0)
    w32(ctx, 0x1015dc14, nodes[0] if nodes else 0)
    return []


# ---- 0x10079ecd ----------------------------------------------------------------
def g_9ecd(ctx):
    r = ctx.rng
    idx = r.choice([r.randrange(0, 256), r.randrange(0, 6)])
    tab = r.choice([1, 2, 4, 1, 2, 4, 0, 3, 5, 8, r.getrandbits(8)])
    word = lambda: r.choice([0x20, 0x20, 0, r.getrandbits(16), r.getrandbits(16) | 0x20])
    mask = lambda: r.choice([0xffff, 0x8000, 1 << r.randrange(0, 16), r.getrandbits(16), 0])
    rec1 = 0x109a8640 + idx * 0x54
    w16(ctx, rec1 + 0x24, word(), mask())
    rec2 = ctx.buf(0x40)
    w16(ctx, rec2 + 0x1c, word(), mask())
    w32(ctx, 0x109e8b40 + idx * 4, rec2)
    rec4 = 0x109e9ba0 + idx * 0x50
    w16(ctx, rec4, word())
    w16(ctx, rec4 + 6, mask())
    ident = ctx.buf(4, bytes([idx, tab, 0, 0]))
    return [ident, r.choice([r.randrange(0, 16), r.randrange(0, 40), r.getrandbits(32), 15, 16, 31, 32, 0x120])]


# ---- 0x100877a5 / 0x1008433f -----------------------------------------------------------
def g_77a5(ctx):
    w32(ctx, 0x1015e698, ctx.buf(8))
    w32(ctx, 0x1015e9a0, rw(ctx))
    return []


# ---- 0x1008cd0a ----------------------------------------------------------------
def g_cd0a(ctx):
    r = ctx.rng
    small = lambda: r.choice([r.randrange(-300, 300) & 0xffffffff, rw(ctx), r.randrange(0, 1 << 20), r.randrange(-(1 << 20), 0) & 0xffffffff])
    fix = lambda: r.choice([0x10000, 0x8000, 0xffff0000, 0x18000, rw(ctx), r.randrange(0, 1 << 20), r.randrange(-(1 << 20), 0) & 0xffffffff])
    rect = ctx.buf(0x14, struct.pack('<5I', *[rw(ctx) for _ in range(5)]))
    m = ctx.buf(16, struct.pack('<4I', *[fix() for _ in range(4)]))
    out = ctx.buf(16)
    w32(ctx, rect + 4, small(), small(), small(), small())
    x = r.random()
    if x < 0.1:
        out = m
    elif x < 0.2:
        out = rect + 4
    return [rect, m, out]


# ---- 0x100aa190 ----------------------------------------------------------------
def g_a190(ctx):
    r = ctx.rng
    obj = ctx.buf(0x100)
    src = ctx.buf(16)
    x = r.random()
    if x < 0.1:
        src = obj + 0x3c
    elif x < 0.2:
        src = obj + 0x40
    elif x < 0.25:
        src = obj + 0x38
    w32(ctx, obj + 0x94, r.choice([0, 0xffffffff, 0x4000, 1, rw(ctx), rw(ctx)]))
    return [obj, src]


# ---- 0x100abb1b ----------------------------------------------------------------
def build_dlist(ctx, nodes, prev_off, next_off):
    for i, nd in enumerate(nodes):
        w32(ctx, nd + prev_off, nodes[i - 1] if i > 0 else 0)
        w32(ctx, nd + next_off, nodes[i + 1] if i + 1 < len(nodes) else 0)


def g_bb1b(ctx):
    r = ctx.rng
    n = r.choice([0, 1, 2, 3, 4, 5])
    nodes = [ctx.buf(0x20) for _ in range(n)]
    build_dlist(ctx, nodes, 0xc, 0x10)
    head = ctx.buf(4, struct.pack('<I', nodes[0] if nodes and r.random() < 0.95 else 0))
    tail = ctx.buf(4, struct.pack('<I', nodes[-1] if nodes and r.random() < 0.95 else 0))
    x = r.random()
    if nodes and x < 0.85:
        node = r.choice(nodes)
    elif x < 0.92:
        node = ctx.buf(0x20, 'zero')
    elif x < 0.96:
        node = 0
    else:
        node = ctx.buf(0x20)
    return [node, head, tail]


# ---- 0x100abbe8 ----------------------------------------------------------------
def g_bbe8(ctx):
    r = ctx.rng
    limit = r.choice([0, 1, 2, 3, 6, 10, r.randrange(0, 12)])
    count = r.choice([limit - 1, limit - 1, limit, 0, r.randrange(-1, limit + 1), r.randrange(0, 6)])
    w32(ctx, 0x1017d1c8, limit)
    w32(ctx, 0x1017d1c4, count)
    order = list(range(max(limit, 0)))
    r.shuffle(order)
    chain = order[:r.randrange(0, len(order) + 1)] if order else []
    addr = lambda j: 0x101c57e0 + j * 0x34
    for i, j in enumerate(chain):
        w32(ctx, addr(j) + 0xc, addr(chain[i - 1]) if i > 0 else 0)
        w32(ctx, addr(j) + 0x10, addr(chain[i + 1]) if i + 1 < len(chain) else 0)
    w32(ctx, 0x1017d1d0, addr(chain[0]) if chain and r.random() < 0.95 else 0)
    w32(ctx, 0x1017d1d4, addr(chain[-1]) if chain and r.random() < 0.95 else 0)
    if chain and r.random() < 0.7:
        idx = r.choice(chain)
    else:
        idx = r.choice([r.randrange(-2, 14), limit - 1, limit, 0, 0x7fffffff, 0x80000000])
    return [idx & 0xffffffff]


# ---- 0x100d1648 ----------------------------------------------------------------
def g_1648(ctx):
    r = ctx.rng
    n = r.choice([1, 2, 3, 4])
    nodes = [ctx.buf(0x20) for _ in range(n)]
    build_dlist(ctx, nodes, 4, 8)
    m = r.choice([0, 1, 2, 3])
    old = [ctx.buf(0x20) for _ in range(m)]
    build_dlist(ctx, old, 4, 8)
    owner = ctx.buf(0x20)
    w32(ctx, owner + 0xc, old[0] if old else 0)
    if old and r.random() < 0.85:
        w32(ctx, owner + 0x10, old[-1])
    else:
        w32(ctx, owner + 0x10, 0)
    return [owner, r.choice(nodes)]


# ---- 0x100d2c4f ----------------------------------------------------------------
def g_2c4f(ctx):
    r = ctx.rng
    obj = ctx.buf(8)
    w16(ctx, obj, r.choice([0, 0x1000, 0xefff, 0xffff, r.getrandbits(16), r.getrandbits(16)]))
    return [obj]


# ---- C runtime string functions -----------------------------------------------------
ALPHA = [0, 1, 0x7f, 0x80, 0xff, 0x81, ord('a'), ord('b'), ord('A'), ord(' '), 0x20, 0xfe, 0x40, 0xc0, 0x0a]


def rstr(ctx, maxlen, alpha, zero_p=0.0):
    r = ctx.rng
    n = r.choice([0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, r.randrange(0, maxlen)])
    data = bytes(r.choice([x for x in alpha if x != 0] or [97]) for _ in range(n)) + b'\0'
    pad = rbytes(ctx, 16)
    # embedded junk after the terminator
    return data + pad


def put_string(ctx, data):
    buf = ctx.buf(len(data) + 16 + 4, 'rand')
    off = ctx.rng.choice([0, 0, 1, 2, 3])
    wb(ctx, buf + off, data)
    return buf + off


def g_strchr(ctx):
    r = ctx.rng
    alpha = r.choice([ALPHA, [97, 98, 99], [0x80, 0xff, 0x7f, 1, 0x81]])
    data = rstr(ctx, 40, alpha)
    s = put_string(ctx, data)
    body = [x for x in data[:data.index(0)]]
    c = r.choice([r.choice(body) if body else 0, r.choice(alpha), 0, r.choice(alpha), r.getrandbits(8)])
    c |= r.choice([0, 0, 0, r.getrandbits(24) << 8])
    return [s, c]


def g_strspn(ctx):
    r = ctx.rng
    alpha = r.choice([[97, 98, 99, 100], [0x80, 0xff, 0x7f, 1, 0x81, 97], ALPHA])
    data = rstr(ctx, 40, alpha)
    s = put_string(ctx, data)
    setdata = bytes(r.choice(alpha[:r.randrange(1, len(alpha) + 1)] or [97]) for _ in range(r.choice([0, 1, 2, 3, 5, 9]))) + b'\0' + rbytes(ctx, 8)
    st = put_string(ctx, setdata)
    return [s, st]


# ---- 0x10107080 ----------------------------------------------------------------
def g_7080(ctx):
    r = ctx.rng
    count = r.choice([0, 1, 5, 32, 33, 64, 70, r.randrange(0, 90)])
    w32(ctx, 0x10ac385c, count)
    blocks = [ctx.buf(256) for _ in range(8)]
    w32(ctx, 0x10ac3720, *blocks)
    return [r.choice([r.randrange(0, count + 3), r.randrange(0, 3), count - 1, count, 0xffffffff]) & 0xffffffff]


# ---- 0x101077f0 ----------------------------------------------------------------
def g_77f0(ctx):
    r = ctx.rng
    count = r.choice([0, 1, 2, 3, 5, 8, r.randrange(0, 9)])
    keys = [r.choice([r.randrange(0, 6), rw(ctx)]) for _ in range(12)]
    for i in range(12):
        w32(ctx, 0x1015c148 + i * 12, rw(ctx), keys[i], rw(ctx))
    w32(ctx, 0x1015c1c8, count)
    return [r.choice([keys[r.randrange(0, 12)], keys[0], keys[r.randrange(0, 12)], rw(ctx), r.randrange(0, 6)])]


# ---- 0x1010c940 ----------------------------------------------------------------
def g_c940(ctx):
    r = ctx.rng
    w32(ctx, 0x1017ebc4, r.choice([0, 1, 1, 1, 0xfde8, rw(ctx)]))
    leads = [0x81, 0x82, 0x83, ord('b'), 0x9f, 0xe0, 0xfc, 1]
    r.shuffle(leads)
    leads = leads[:r.randrange(0, 6)]
    table = bytearray(rbytes(ctx, 256))
    for i in range(256):
        table[i] &= ~4
        if i in leads or (r.random() < 0.02):
            table[i] |= 4
    wb(ctx, 0x1017eac1, table)
    alpha = [0x81, 0x82, 0x83, ord('b'), ord('a'), 0x40, 0x9f, 0xe0, 0xfc, 1, 0x20, 0x80, 0xff]
    data = rstr(ctx, 30, alpha)
    s = put_string(ctx, data)
    body = list(data[:data.index(0)])
    if len(body) >= 2 and r.random() < 0.4:
        i = r.randrange(0, len(body) - 1)
        c = (body[i] << 8) | body[i + 1]
    elif body and r.random() < 0.5:
        c = r.choice(body)
    else:
        c = r.choice([0, r.choice(alpha), (r.choice(alpha) << 8) | r.choice(alpha), rw(ctx)])
    return [s, c]


def t(n, **kw):
    return dict(trials=n, **kw)


SPECS = [
    dict(name='Sim_10008e8b', orig=0x10008e8b, c='Sim_10008e8b', gen=g_8e8b, void=True, trials=300),
    dict(name='Sim_100093ea', orig=0x100093ea, c='Sim_100093ea', gen=g_93ea, trials=300),
    dict(name='Sim_1000c627', orig=0x1000c627, c='Sim_1000c627', gen=g_c627, trials=500),
    dict(name='Sim_1000e060', orig=0x1000e060, c='Sim_1000e060', gen=g_e060, void=True, trials=800),
    dict(name='Sim_1001083a', orig=0x1001083a, c='Sim_1001083a', gen=g_083a, void=True, trials=400),
    dict(name='Sim_10013fe1', orig=ARGMEM_TOP, c='Sim_10013fe1', gen=g_3fe1, trials=500),
    dict(name='Sim_100155b7', orig=0x100155b7, c='Sim_100155b7', gen=g_55b7, trials=1000),
    dict(name='Sim_10015cd2', orig=0x10015cd2, c='Sim_10015cd2', gen=g_5cd2, trials=400),
    dict(name='Sim_1001c102', orig=0x1001c102, c='Sim_1001c102', gen=g_none, void=True, trials=300),
    dict(name='Sim_10027acf', orig=0x10027acf, c='Sim_10027acf', gen=g_7acf, trials=500),
    dict(name='Sim_10028965', orig=0x10028965, c='Sim_10028965', gen=g_set_obj, void=True, trials=300),
    dict(name='Sim_10028a15', orig=0x10028a15, c='Sim_10028a15', gen=g_set_obj, void=True, trials=300),
    dict(name='Sim_1002dc46', orig=0x1002dc46, c='Sim_1002dc46', gen=g_dc46, trials=500),
    dict(name='Sim_10037306', orig=0x10037306, c='Sim_10037306', gen=g_none, void=True, trials=300),
    dict(name='Sim_10037680', orig=0x10037680, c='Sim_10037680', gen=g_7680, trials=800),
    dict(name='Sim_10037c9e', orig=0x10037c9e, c='Sim_10037c9e', gen=g_set_g, void=True, trials=300),
    dict(name='Sim_1003d863', orig=0x1003d863, c='Sim_1003d863', gen=g_d863, void=True, trials=300),
    dict(name='Sim_10044311', orig=0x10044311, c='Sim_10044311', gen=make_reader(0x1015ee98), trials=300),
    dict(name='Sim_10045f3b', orig=0x10045f3b, c='Sim_10045f3b', gen=g_5f3b, void=True, trials=800),
    dict(name='Sim_1004c5b7', orig=0x1004c5b7, c='Sim_1004c5b7', gen=g_none, trials=300),
    dict(name='Sim_1004cdaf', orig=0x1004cdaf, c='Sim_1004cdaf', gen=g_flags, trials=400),
    dict(name='Sim_1004cdc0', orig=0x1004cdc0, c='Sim_1004cdc0', gen=g_flags, trials=400),
    dict(name='Sim_1004cdf3', orig=0x1004cdf3, c='Sim_1004cdf3', gen=g_flags, trials=400),
    dict(name='Sim_1004ce04', orig=0x1004ce04, c='Sim_1004ce04', gen=g_flags, trials=400),
    dict(name='Sim_1004ce17', orig=0x1004ce17, c='Sim_1004ce17', gen=g_flags, trials=400),
    dict(name='Sim_1004ce2a', orig=0x1004ce2a, c='Sim_1004ce2a', gen=g_flags, trials=400),
    dict(name='Sim_1004ce3d', orig=0x1004ce3d, c='Sim_1004ce3d', gen=g_flags, trials=400),
    dict(name='Sim_1004ce64', orig=0x1004ce64, c='Sim_1004ce64', gen=make_sum(0x101116be, ADDS_CE64), trials=800),
    dict(name='Sim_1004cfb5', orig=0x1004cfb5, c='Sim_1004cfb5', gen=make_sum(0x101116ba, ADDS_CFB5), trials=800),
    dict(name='Sim_1004d106', orig=0x1004d106, c='Sim_1004d106', gen=make_sum(0x101116a6, ADDS_D106), trials=800),
    dict(name='Sim_1004db86', orig=0x1004db86, c='Sim_1004db86', gen=g_db86, trials=600),
    dict(name='Sim_1004e582', orig=0x1004e582, c='Sim_1004e582', gen=g_e582, trials=1000),
    dict(name='Sim_100526a9', orig=0x100526a9, c='Sim_100526a9', gen=g_26a9, trials=600),
    dict(name='Sim_10052c30', orig=0x10052c30, c='Sim_10052c30', gen=g_2c30, trials=500),
    dict(name='Sim_10052ff0', orig=0x10052ff0, c='Sim_10052ff0', gen=g_val, trials=300),
    dict(name='Sim_10052ffb', orig=0x10052ffb, c='Sim_10052ffb', gen=g_val, trials=300),
    dict(name='Sim_1005571d', orig=0x1005571d, c='Sim_1005571d', gen=g_none, void=True, trials=300),
    dict(name='Sim_1005572c', orig=0x1005572c, c='Sim_1005572c', gen=g_572c, trials=600),
    dict(name='Sim_10057a43', orig=0x10057a43, c='Sim_10057a43', gen=g_none, trials=300),
    dict(name='Sim_10057a58', orig=0x10057a58, c='Sim_10057a58', gen=g_7a58, trials=500),
    dict(name='Sim_1005b7e0', orig=0x1005b7e0, c='Sim_1005b7e0', gen=g_b7e0, void=True, trials=500),
    dict(name='Sim_1005b923', orig=0x1005b923, c='Sim_1005b923', gen=g_b923, trials=500),
    dict(name='Sim_1006271a', orig=0x1006271a, c='Sim_1006271a', gen=g_271a, void=True, trials=300),
    dict(name='Sim_10076689', orig=0x10076689, c='Sim_10076689', gen=g_6689, trials=500),
    dict(name='Sim_10079ecd', orig=0x10079ecd, c='Sim_10079ecd', gen=g_9ecd, trials=1000),
    dict(name='Sim_100877a5', orig=0x100877a5, c='Sim_100877a5', gen=g_77a5, void=True, trials=300),
    dict(name='Sim_100877bc', orig=0x100877bc, c='Sim_100877bc', gen=g_none, trials=300),
    dict(name='Sim_1008433f', orig=0x1008433f, c='Sim_1008433f', gen=g_77a5, trials=300),
    dict(name='Sim_1008cd0a', orig=0x1008cd0a, c='Sim_1008cd0a', gen=g_cd0a, trials=1000),
    dict(name='Sim_100987e6', orig=0x100987e6, c='Sim_100987e6', gen=g_87e6, trials=300),
    dict(name='Sim_10098e39', orig=0x10098e39, c='Sim_10098e39', gen=g_none, void=True, trials=300),
    dict(name='Sim_10098e3e', orig=0x10098e3e, c='Sim_10098e3e', gen=g_none, trials=300),
    dict(name='Sim_1009ca28', orig=0x1009ca28, c='Sim_1009ca28', gen=g_none, trials=300),
    dict(name='Sim_1009e5b9', orig=0x1009e5b9, c='Sim_1009e5b9', gen=make_reader(0x1017d060), trials=300),
    dict(name='Sim_100aa190', orig=0x100aa190, c='Sim_100aa190', gen=g_a190, void=True, trials=600),
    dict(name='Sim_100abb1b', orig=0x100abb1b, c='Sim_100abb1b', gen=g_bb1b, void=True, trials=1000),
    dict(name='Sim_100abbe8', orig=0x100abbe8, c='Sim_100abbe8', gen=g_bbe8, trials=1000),
    dict(name='Sim_100d1648', orig=0x100d1648, c='Sim_100d1648', gen=g_1648, trials=800),
    dict(name='Sim_100d2c4f', orig=0x100d2c4f, c='Sim_100d2c4f', gen=g_2c4f, trials=400),
    dict(name='Crt_StrChr', orig=0x100fd650, c='Crt_StrChr', gen=g_strchr, trials=2000),
    dict(name='Crt_StrSpn', orig=0x100ff160, c='Crt_StrSpn', gen=g_strspn, trials=1500),
    dict(name='Sim_10107080', orig=0x10107080, c='Sim_10107080', gen=g_7080, trials=600),
    dict(name='Sim_101077f0', orig=0x101077f0, c='Sim_101077f0', gen=g_77f0, trials=800),
    dict(name='Crt_MbsChr', orig=0x1010c940, c='Crt_MbsChr', gen=g_c940, trials=2000),
]
