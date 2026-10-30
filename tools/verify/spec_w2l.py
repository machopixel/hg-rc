# Differential tests for wave 2l (see difftest.py): leaf-like functions of many areas and
# the few that call already converted functions (ring, field accessors, object lists, rects).
import struct

SOURCES = [
    # callees that already exist
    'sim/util/ring.c', 'sim/util/field.c', 'sim/view/objlist.c', 'sim/view/viewrect.c',
    'sim/unsorted_1004/fields.c',
    # wave 2l
    'sim/view/w2l_lists.c', 'sim/view/w2l_rects.c',
    'sim/unsorted_1000/w2l_fields.c', 'sim/unsorted_1000/w2l_objects.c',
    'sim/unsorted_1001/w2l_misc.c', 'sim/unsorted_1002/w2l_tables.c',
    'sim/unsorted_1003/w2l_masks.c', 'sim/unsorted_1003/w2l_records.c',
    'sim/unsorted_1004/w2l_values.c', 'sim/unsorted_1005/w2l_kinds.c',
    'sim/unsorted_1006/w2l_slots.c', 'sim/unsorted_1007/w2l_fields.c',
    'sim/unsorted_1008/w2l_grid.c', 'sim/unsorted_1009/w2l_pool.c',
    'sim/util/w2l_ring.c', 'sim/unsorted_100a/w2l_small.c',
    'sim/unsorted_100b/w2l_records.c', 'sim/unsorted_100c/w2l_objects.c',
]


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def w16(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<H', v & 0xffff))


def w8(ctx, addr, v):
    ctx.m.uc.mem_write(addr, bytes([v & 0xff]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def rnd_bytes(ctx, n):
    return bytes(ctx.rng.getrandbits(8) for _ in range(n))


def rnd_dwords(ctx, addr, n, gen=None):
    gen = gen or ctx.u32
    wb(ctx, addr, struct.pack('<%dI' % n, *[gen() & 0xffffffff for _ in range(n)]))


def ptrtab(ctx, n, size, fields=None):
    """Pointer table 0x109e8b40 with n records of `size` bytes; fields(i, addr) fills each."""
    recs = []
    for i in range(n):
        a = ctx.buf(size)
        if fields:
            fields(i, a)
        w32(ctx, 0x109e8b40 + 4 * i, a)
        recs.append(a)
    return recs


def ring(ctx, idx_addr):
    r = ctx.rng
    idx = r.choice([0, 1, 125, 126, 127, 128, r.randrange(0, 127), r.randrange(0, 127), r.randrange(0, 127)])
    w32(ctx, idx_addr, idx)
    wb(ctx, 0x101cffc0, struct.pack('<140I', *[r.choice([ctx.u32(), r.randrange(0, 1000), r.getrandbits(32)]) for _ in range(140)]))


# ---- a world of linked objects (view/w2l.h) ----
ANCHORS = [0x1017ba00, 0x1017b9d8, 0x1017c818, 0x1017b988]


def world(ctx, n=6, free=0.35):
    """Objects (0x100 bytes) and the four anchors in the DLL's data with random links in the
    pairs +4/+8, +0xc/+0x10, +0x14/+0x18, +0x1c/+0x20 and owners at +0x28."""
    r = ctx.rng
    nodes = [ctx.buf(0x100) for _ in range(n)]
    owners = [ctx.buf(0x100) for _ in range(2)]
    pool = nodes + ANCHORS

    def ptr():
        return 0 if r.random() < free else r.choice(pool)

    for a in ANCHORS + nodes:
        if a in nodes:
            wb(ctx, a, rnd_bytes(ctx, 0x98))
        w16(ctx, a, r.choice([0, 0, 0, 0x800, 0x1000, 0x1800, r.getrandbits(16)]))
        for off in (4, 8, 0xc, 0x10, 0x14, 0x18, 0x1c, 0x20):
            w32(ctx, a + off, ptr())
        if a in nodes:  # the anchors are 0x28 apart in the DLL's data: leave their +0x28 alone
            w32(ctx, a + 0x28, r.choice([0, 0, r.choice(owners), r.choice(nodes)]))
    for o in owners:
        w32(ctx, o, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x1017c8cc, r.choice([0, r.choice(nodes), r.choice(ANCHORS), 0x1017b988, 0x1017b988, ctx.u32()]))
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    w32(ctx, 0x1017e678, r.choice([0, r.choice(nodes), ctx.u32()]))
    return nodes, owners


def pick_node(ctx, nodes, allow_zero=0.0, allow_anchor=0.1):
    r = ctx.rng
    x = r.random()
    if x < allow_zero:
        return 0
    if x < allow_zero + allow_anchor:
        return r.choice(ANCHORS)
    return r.choice(nodes)


# ---- unsorted_1000 ----
def g_716a(ctx):
    r = ctx.rng
    n = 8
    for i in range(n):
        a = 0x10ac1b00 + 14 * i
        wb(ctx, a, rnd_bytes(ctx, 14))
        w16(ctx, a, r.choice([0, 1, 2, 3, r.getrandbits(16), 0xffff]))
        w16(ctx, a + 8, r.choice([0, 1, 5, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]))
    wb(ctx, 0x10a51ac0 - 0x300, bytes(r.choice([0, 0, 1, r.getrandbits(8), 0x80]) for _ in range(0x300 * 6)))
    index = r.choice([r.randrange(0, n), r.randrange(0, n), r.randrange(0, n), 0, n - 1,
                      r.randrange(0, n) | (r.getrandbits(16) << 16), r.getrandbits(32), 0xffff])
    value = r.choice([r.randrange(0, 0x20), r.randrange(0, 0x20), 0x1f, 0x20, 0x21, r.getrandbits(16),
                      r.getrandbits(32), r.randrange(0, 4) | (r.getrandbits(16) << 16)])
    return [index, value]


def g_obj_buf(size):
    def g(ctx):
        return [ctx.buf(size)]
    return g


def g_96b2(ctx):
    r = ctx.rng
    obj = ctx.buf(0x40)
    n = r.randrange(0, 9)
    if r.random() < 0.85:
        p = ctx.buf(0x800)
        w16(ctx, p + 6, n if r.random() < 0.9 else r.getrandbits(16))
        w32(ctx, p + 8, 0x40 if r.random() < 0.9 else r.choice([0, 0x20, 0x100]))
        w32(ctx, obj + 0x30, p)
    else:
        w32(ctx, obj + 0x30, 0)
    index = r.choice([r.randrange(0, n + 2), r.randrange(0, n + 2), -1 & 0xffffffff, r.randrange(-3, 12) & 0xffffffff, 0])
    return [obj, index, r.choice([ctx.u32(), r.getrandbits(16), 0xffff])]


def g_world_obj(allow_zero=0.05):
    def g(ctx):
        nodes, _ = world(ctx)
        return [pick_node(ctx, nodes, allow_zero)]
    return g


def g_9787(ctx):
    r = ctx.rng
    nodes, owners = world(ctx)
    obj = pick_node(ctx, nodes, 0.03, 0.05)
    if r.random() < 0.3:
        w32(ctx, 0x1017e678, obj)
    if r.random() < 0.2:
        w32(ctx, 0x1017c8cc, obj)
    return [obj]


def g_c813(ctx):
    r = ctx.rng
    nrec = 6
    for i in range(nrec):
        a = 0x10145508 + i * 0xd6
        wb(ctx, a, rnd_bytes(ctx, 0xd6))
        w32(ctx, a + 0x10, r.choice([0x3d, 0x3c, 0x3e, 0x41, 0x42, 0x43, 0x44, 0x40, r.getrandbits(32), r.randrange(0, 0x50),
                                     0x80000000 | 0x42, 0xffffffff]))
    obj = ctx.buf(0x100)
    ents = ctx.buf(0x128 * 4)
    for e in range(4):
        w32(ctx, ents + 0x128 * e + 4, r.randrange(0, nrec))
    w32(ctx, obj + 0x84, ents)
    return [obj if r.random() < 0.9 else 0, r.choice([0, 1, 2, 3, 3, 0])]


def g_ede4(ctx):
    r = ctx.rng
    x = r.random()
    if x < 0.2:
        w32(ctx, 0x1015d0a4, 0)
    elif x < 0.93:
        w32(ctx, 0x1015d0a4, ctx.buf(0x100) - 0x54 + r.choice([0, 0, 0x54, 4]))
    else:
        w32(ctx, 0x1015d0a4, r.getrandbits(32))
    return []


# ---- unsorted_1001 ----
def g_10124(ctx):
    r = ctx.rng
    a, b = ctx.buf(0x40), ctx.buf(0x40)
    return [a if r.random() > 0.15 else 0, b if r.random() > 0.15 else 0]


def g_128dd(ctx):
    r = ctx.rng
    n = 8

    def f(i, a):
        w32(ctx, a + 8, r.randrange(0, 3))
        hi = r.choice([0x200, 0x200, 0x200, 0, 0x300, 0x1200, 0xf200, 0x8200, 0x100, 0x2a00])
        w16(ctx, a + 0x1aa, hi | r.choice([0, 1, 2, 3, 0x7f, 0xff, r.getrandbits(8)]))
    ptrtab(ctx, n, 0x1c0, f)
    w32(ctx, 0x1015dc64, r.choice([n, n, n, n - 1, 0, r.randrange(0, n + 1), 0xffffffff]))
    return [r.randrange(0, 3), r.choice([0, 1, 2, 3, 0xff, r.randrange(0, 4) | 0x100, 0x100, r.getrandbits(32)]), ctx.u32()]


def g_13040(ctx):
    return [ctx.u32(), ctx.u32() & 0xffff, ctx.u32() & 0xffff]


def g_17aa9(ctx):
    r = ctx.rng
    obj = ctx.buf(0x300)
    w32(ctx, obj + 0x28, ctx.buf(0x600) if r.random() < 0.95 else obj)
    return [obj]


def g_none(ctx):
    return []


# ---- unsorted_1002 ----
def g_20456(ctx):
    r = ctx.rng
    rnd_dwords(ctx, 0x10133700, 12, lambda: r.randrange(0, 40))
    return [r.choice([r.randrange(-2, 9), r.randrange(0, 7), 0, 4, 5, 6, r.getrandbits(32), 0x80000000]),
            r.choice([0xffff, 0xffff, r.randrange(0, 26), r.randrange(-3, 30) & 0xffffffff, 8, 9, 18, 19, 20, ctx.u32()])]


def g_272a0(ctx):
    r = ctx.rng
    rnd_dwords(ctx, 0x109f1484 - 8, 24)
    return [r.choice([r.randrange(-3, 13) & 0xffffffff, r.randrange(0, 10), 0, 9, 10, 0x80000000, 0x7fffffff])]


def g_28973(ctx):
    return [ctx.buf(0x40), ctx.u32()]


def g_28b50(ctx):
    r = ctx.rng
    for i in range(0x80):
        w32(ctx, 0x109eeb04 + i * 0x30, r.choice([1, 2, ctx.u32() | 1, 0xffffffff]))
    x = r.random()
    if x < 0.6:
        w32(ctx, 0x109eeb04 + r.randrange(0, 0x80) * 0x30, 0)
    elif x < 0.7:
        w32(ctx, 0x109eeb04 + 0x7f * 0x30, 0)
    return []


def g_2997e(ctx):
    r = ctx.rng
    for i in range(0x80):
        w32(ctx, 0x109eeb04 + i * 0x30, r.choice([0, 0, r.randrange(0, 40), r.randrange(0, 40), ctx.u32()]))
    w32(ctx, 0x109eeb04 + 0x7f * 0x30, r.choice([0, 77]))
    return [r.choice([r.randrange(0, 40), r.randrange(0, 40), 0, 77, 0xffffffff, ctx.u32()])]


# ---- unsorted_1003 ----
def mask(ctx):
    r = ctx.rng
    return r.choice([0xf800, 0x7e0, 0x1f, 0xff0000, 0xff00, 0xff, 0xff000000, 0x7c00, 0x3e0, 0x8000, 0, 0, 1,
                     0x80000000, 0xffffffff, 0xf0, r.getrandbits(32), r.getrandbits(32), ctx.u32()])


def g_1775f(ctx):
    p = ctx.buf(0x40)
    wb(ctx, p, struct.pack('<12I', *[mask(ctx) for _ in range(4)] + [ctx.u32() for _ in range(8)]))
    return [p]


def g_1772b(ctx):
    p = ctx.buf(0x40)
    return [p, mask(ctx), mask(ctx), mask(ctx), mask(ctx)]


def g_3b8e3(ctx):
    r = ctx.rng
    nodes, _ = world(ctx)
    n = 6
    keys = [r.randrange(0, 3) for _ in range(n)]
    for i in range(n):
        a = 0x109b9e20 + i * 0x7e
        wb(ctx, a, rnd_bytes(ctx, 0x7e))
        w32(ctx, a + 0x60, keys[i])
        for j in range(4):
            w32(ctx, a + 0x50 + 4 * j, 0 if r.random() < 0.35 else r.choice(nodes))
    w32(ctx, 0x1015ea80, r.choice([n, n, n - 1, 2, 0, r.randrange(0, n + 1), 0xffffffff]))
    return [r.choice([0, 1, 2, 2, 3, 7])]


def g_3db30(ctx):
    r = ctx.rng
    n = 8

    def f(i, a):
        wb(ctx, a, rnd_bytes(ctx, 0x40))
    recs = ptrtab(ctx, n, 0x40, f)
    for i in range(n):
        if r.random() < 0.25:
            w32(ctx, 0x109e8b40 + 4 * i, 0)
    if r.random() < 0.04:
        w32(ctx, 0x109e8b40 + 4 * r.randrange(0, n), r.getrandbits(32))
    return [r.choice([r.randrange(0, n), r.randrange(0, n), r.randrange(0, n), n, n + 3, r.randrange(-2, 20) & 0xffffffff])]


# ---- unsorted_1004 ----
def g_47fe0(ctx):
    wb(ctx, 0x109b9920 - 8, rnd_bytes(ctx, 0x100 + 16))
    return []


def g_4b804(ctx):
    r = ctx.rng
    wb(ctx, 0x109b9080, rnd_bytes(ctx, 0x600))
    w32(ctx, 0x1013b98c, r.choice([0, 1, 2, 3, 5, 8, r.randrange(0, 9), r.getrandbits(3), 0x10]))
    a = ctx.buf(0x10)
    b = ctx.buf(0x10) if r.random() > 0.15 else a
    return [a, b]


def word16(ctx):
    r = ctx.rng
    edge = r.choice([0x190, 0x258, 0x2ee, 0x3b6, 0x3b7, 0x3b6 + 0x32, 0x3b6 + 0x31])
    low = r.choice([edge, edge - 1, edge + 1, r.randrange(0, 0x600), r.randrange(0, 0x600), r.getrandbits(16), 0, 0xffff])
    hi = r.choice([0, 0, 1, 2, 3, 0xffff, r.getrandbits(16)])
    return ((hi << 16) | (low & 0xffff)) & 0xffffffff


def g_word(ctx):
    return [word16(ctx)]


def g_4c426(ctx):
    r = ctx.rng
    rnd_dwords(ctx, 0x10124ec4 - 4 * 0x98, 12 * 0x98 // 4, lambda: r.choice([ctx.u32(), r.randrange(0, 10), 0xffffffff]))
    low = word16(ctx) & 0xffff
    hi = r.choice([0, 1, 2, 3, 4, 5, 0xffff, 0xfffe])
    return [(hi << 16) | low]


def g_4ce50(ctx):
    r = ctx.rng
    rnd_dwords(ctx, 0x101116b2 - 3 * 0xaa, 16 * 0xaa // 4)
    return [r.choice([r.randrange(-3, 12) & 0xffffffff, r.randrange(0, 10), 0, 9])]


# ---- unsorted_1005 ----
def g_5b89b(ctx):
    r = ctx.rng
    return [r.choice([r.randrange(0, 0x30), r.randrange(0, 0x30), r.randrange(0, 0x30), r.getrandbits(32), 1, 9, 0xc, 0x1c, 0x29,
                      0xffffffff]), ctx.u32()]


# ---- unsorted_1006 ----
def g_62780(ctx):
    r = ctx.rng
    cur = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, cur)

    def f(i, a):
        obj = ctx.buf(0x1000)
        wb(ctx, obj, rnd_bytes(ctx, 0x100))
        w32(ctx, obj + 0x613, r.choice([0, 1, 2, 3, 3, r.randrange(0, 5)]))
        w32(ctx, a + 0x28, obj)
    ptrtab(ctx, 4, 0x40, f)
    ring(ctx, 0x1017cd38)
    w32(ctx, 0x1015dc80, r.choice([0, 1, 5, 5, 5, 20, 100, r.randrange(0, 200), ctx.u32()]))
    w32(ctx, 0x101c64f0, r.choice([0, 1, 2, r.randrange(0, 4), ctx.u32()]))
    for i in range(10):
        s = 0x10165be8 + i * 0x18
        wb(ctx, s, rnd_bytes(ctx, 0x18))
        w32(ctx, s, r.choice([r.randrange(-50, 300) & 0xffffffff, 0, 1, 5, 400, 0x7fffffff, 0x80000000, ctx.u32()]))
        w32(ctx, s + 4, r.choice([0, 1, 2, 0, 1, 2, r.randrange(-3, 6) & 0xffffffff, 0x7fffffff, 0xffffffff]))
        w32(ctx, s + 0x10, r.choice([0, 1, 1, 2, ctx.u32()]))
        w32(ctx, s + 0x14, r.choice([0, 1, 1, ctx.u32()]))
        p = 0x10144ed8 + i * 8
        w32(ctx, p, r.randrange(0, 20))
        w32(ctx, p + 4, r.randrange(0, 20))
    rnd_dwords(ctx, 0x109e8480, 40)
    return []


# ---- unsorted_1007 ----
def g_73027(ctx):
    obj = ctx.buf(0x100)
    w32(ctx, obj + 0x28, ctx.buf(0x600))
    return [obj]


def g_76676(ctx):
    r = ctx.rng
    p = ctx.buf(0x40)
    w32(ctx, p + 0xc, r.choice([0, 0, 1, ctx.u32()]))
    return [p]


def g_7dc1f(ctx):
    r = ctx.rng
    for a in (0x101c49dc, 0x101c49e0, 0x101c459c, 0x101c45a0, 0x101c5138, 0x101c513c, 0x101c5144, 0x101c47dc,
              0x101c47e0, 0x101c47e8):
        w32(ctx, a, r.getrandbits(32))
    wb(ctx, 0x10179d38, rnd_bytes(ctx, 0x30))
    return []


# ---- unsorted_1008 ----
def g_grid(ctx):
    wb(ctx, 0x1017c840, rnd_bytes(ctx, 0x80))
    w32(ctx, 0x1017c8c4, ctx.rng.choice([0, 1, ctx.u32()]))
    return []


# ---- view/w2l: list functions ----
def g_insert_after(ctx):
    r = ctx.rng
    nodes, _ = world(ctx, free=0.55)
    return [pick_node(ctx, nodes, 0.06, 0.1), pick_node(ctx, nodes, 0.06, 0.1)]


def g_8a871(ctx):
    nodes, _ = world(ctx, free=0.4)
    return [pick_node(ctx, nodes, 0, 0.08)]


def g_8a8f5(ctx):
    nodes, _ = world(ctx, free=0.5)
    return [pick_node(ctx, nodes, 0, 0.05), pick_node(ctx, nodes, 0, 0.2)]


def g_8a14f(ctx):
    nodes, _ = world(ctx, free=0.5)
    return [pick_node(ctx, nodes, 0.08, 0.05)]


def g_8a16d(ctx):
    r = ctx.rng
    nodes, _ = world(ctx, free=0.4)
    node = pick_node(ctx, nodes, 0.05, 0.25)
    if r.random() < 0.25 and node:
        w32(ctx, 0x1017c8cc, node)
    return [node]


def g_8a1ea(ctx):
    r = ctx.rng
    nodes, _ = world(ctx, free=0.5)
    w32(ctx, 0x1017c8cc, 0x1017b988 if r.random() < 0.7 else r.choice([0, nodes[0], 0x1017b9d8]))
    return [pick_node(ctx, nodes, 0.1, 0.05)]


def g_8a40a(ctx):
    r = ctx.rng
    nodes, _ = world(ctx, free=0.5)
    node = pick_node(ctx, nodes, 0.05, 0.05)
    if node and r.random() < 0.3:
        w32(ctx, node + 4, 0)
    return [node]


# ---- view/w2l: rects ----
def g_8d07b(ctx):
    r = ctx.rng
    # the original raises a divide error when a quotient does not fit 32 bits: x < ~10.45M, y < ~6.5M
    def one(limit):
        return r.choice([r.randrange(-limit, limit), r.randrange(-1000, 1000), 0, 319, -319, 199, -199, 1, -1,
                         r.randrange(-65536 * 3, 65536 * 3), limit - 1, -limit, 319 * 1000, 199 * 1000]) & 0xffffffff
    src = ctx.buf(0x10)
    wb(ctx, src, struct.pack('<2I', one(10_400_000), one(6_400_000)))
    dst = ctx.buf(0x10) if r.random() > 0.15 else src
    return [src, dst]


def fixv(ctx):
    r = ctx.rng
    x = r.random()
    if x < 0.3:
        return r.randrange(-0x20000, 0x20000) & 0xffffffff
    if x < 0.6:
        return r.randrange(0, 0x10000)
    if x < 0.7:
        return r.choice([0, 0x10000, 0xffff0000, 0x8000, 0x7fffffff, 0x80000000, 0xffffffff])
    return r.getrandbits(32)


def g_8f867(ctx):
    r = ctx.rng
    n = ctx.buf(0x20)
    o = ctx.buf(0x40)
    rects = [ctx.buf(0x20) for _ in range(3)]
    for a in rects:
        wb(ctx, a, struct.pack('<5I', *[fixv(ctx) for _ in range(5)]))
    out = rects[2] if r.random() > 0.15 else r.choice(rects[:2])
    div = r.choice([r.randrange(50, 3000), r.randrange(50, 3000), 100, 1000, -r.randrange(50, 3000)])
    step = r.choice([0, 1, 5, 5, 20, r.randrange(0, 300)])
    w32(ctx, 0x1015dc80, step)
    if r.random() < 0.9:
        t = r.choice([r.randrange(0, int(abs(div) * 1.3)), r.randrange(0, abs(div)), abs(div) // 2, abs(div) // 2 + 1, abs(div), abs(div) - step])
        t = t if div > 0 else -t
        n2 = t - step
    else:
        n2 = r.randrange(-1000, 1000)
    w32(ctx, n, r.choice([1, 1, 1, 1, 1, 0, 2, ctx.u32()]))
    w32(ctx, n + 4, r.choice([1, 1, 1, 0, 2, ctx.u32()]))
    w32(ctx, n + 8, n2)
    w32(ctx, o, div)
    w32(ctx, o + 4, rects[0])
    w32(ctx, o + 8, rects[1])
    w32(ctx, o + 0xc, out)
    pair = ctx.buf(8)
    w32(ctx, pair, n)
    w32(ctx, pair + 4, o)
    return [r.choice([0, 0, 1, 1, 2, 0xffffffff, ctx.u32()]), pair]


# ---- unsorted_1009 ----
def g_1044e(ctx):
    r = ctx.rng
    k = r.randrange(1, 6)
    nodes = [ctx.buf(0x40) for _ in range(k)]
    for i, a in enumerate(nodes):
        w32(ctx, a, nodes[i - 1] if i > 0 else 0)
        w32(ctx, a + 4, nodes[i + 1] if i + 1 < k else 0)
    if r.random() < 0.15:
        for a in nodes:
            w32(ctx, a, r.choice([0] + nodes))
            w32(ctx, a + 4, r.choice([0] + nodes))
    w32(ctx, 0x1017cc28, nodes[0] if r.random() > 0.08 else r.choice([0, nodes[-1]]))
    w32(ctx, 0x1017cc2c, nodes[-1] if r.random() > 0.08 else r.choice([0, nodes[0]]))
    extra = ctx.buf(0x40)
    w32(ctx, extra, r.choice(nodes + [0]))
    w32(ctx, extra + 4, r.choice(nodes + [0]))
    return [r.choice(nodes + nodes + [extra, 0, nodes[0], nodes[-1]])]


def pool(ctx):
    """Records at 0x10217c20 chained in a random order with the ends in 0x1017cc28 / 0x1017cc2c."""
    r = ctx.rng
    n = r.choice([3, 4, 6, 6])
    recs = [0x10217c20 + 0x57e * i for i in range(n)]
    order = recs[:]
    r.shuffle(order)
    order = order[:r.randrange(1, n + 1)]
    for i, a in enumerate(order):
        w32(ctx, a, order[i - 1] if i > 0 else 0)
        w32(ctx, a + 4, order[i + 1] if i + 1 < len(order) else 0)
    w32(ctx, 0x1017cc28, order[0])
    w32(ctx, 0x1017cc2c, order[-1])
    w32(ctx, 0x1017cc20, n if r.random() > 0.1 else r.randrange(0, n + 3))
    w32(ctx, 0x10215ae8, r.choice([0xffffffff, 0, 1, n - 2, n - 1, n - 1, r.randrange(-1, n + 2) & 0xffffffff, ctx.u32()]))
    rnd_dwords(ctx, 0x10216b80, 12, lambda: r.randrange(0, 10))
    return n


def g_90513(ctx):
    r = ctx.rng
    n = pool(ctx)
    return [r.choice([r.randrange(-1, n + 2) & 0xffffffff, r.randrange(0, n), r.randrange(0, n), 0, n - 1, n, 0x80000000])]


def g_90cc8(ctx):
    r = ctx.rng
    n = pool(ctx)
    w32(ctx, 0x1017cc20, n)
    return [r.choice([r.randrange(-1, n + 2) & 0xffffffff, r.randrange(1, n), r.randrange(1, n), 0, n - 1, n, 0x80000000])]


def g_eb24(ctx):
    w32(ctx, 0x1017d08c, ctx.u32())
    return []


# ---- util ----
def g_9c9a5(ctx):
    r = ctx.rng
    ring(ctx, 0x1017cd40)
    return [r.choice([r.randrange(1, 100), r.randrange(1, 1000), 3, 100, 0x7f, ctx.u32() | 1, r.randrange(-50, 50) or 1,
                      -1 & 0xffffffff])]


# ---- unsorted_100a ----
def g_a78c3(ctx):
    return [ctx.u32(), ctx.u32()]


def g_aa7f1(ctx):
    return [ctx.buf(0x20)]


# ---- unsorted_100b ----
def g_b534b(ctx):
    r = ctx.rng
    nodes, _ = world(ctx)
    for i in range(4):
        a = 0x101a2100 + i * 0x88
        wb(ctx, a, rnd_bytes(ctx, 0x88))
        w32(ctx, a + 0x28, 0 if r.random() < 0.3 else r.choice(nodes))
    return [r.choice([0, 1, 2, 3, 3, 1])]


def g_b5bbb(ctx):
    w32(ctx, 0x1015084c, ctx.u32())
    return [ctx.u32()]


def g_ba21c(ctx):
    r = ctx.rng
    n = r.randrange(1, 24)
    data = bytes(r.choice(b'abcXYZ019_.\\/' + b'  \t\0') for _ in range(n)) + bytes([0])
    s = ctx.buf(40, fill=data.ljust(40, b'Z'))
    return [s + r.choice([0, 0, 0, 1])]


def keytab(ctx):
    r = ctx.rng
    count = r.choice([0, 1, 2, 3, 4, 5, 5, 8])
    recs = ctx.buf(0x12 * 8 + 0x40)
    keys = [r.choice([1, 2, 3, 4, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]) for _ in range(8)]
    for i in range(8):
        a = recs + 0x12 * i
        wb(ctx, a, rnd_bytes(ctx, 0x12))
        w16(ctx, a, keys[i])
        w16(ctx, a + 2, r.choice([1, 2, 3, 4, 4, 5, 6, 0]) if r.random() > 0.05 else r.getrandbits(16))
        for j in range(6):
            w16(ctx, a + 4 + 2 * j, r.choice([r.getrandbits(16), r.randrange(0, 20)]))
    table = ctx.buf(0x10)
    w16(ctx, table, count if r.random() > 0.05 else r.choice([0xffff, 0x8000]))
    w16(ctx, table + 2, r.choice([0, 0, 1, 1, r.randrange(0, 4)]))
    w32(ctx, table + 4, recs)
    return table, keys, count


def g_bf23f(ctx):
    r = ctx.rng
    table, keys, count = keytab(ctx)
    return [table, r.choice([keys[0], keys[1], keys[r.randrange(0, 8)], keys[r.randrange(0, 8)], 0, 0xffff, r.getrandbits(16),
                              r.getrandbits(32), keys[0] | (r.getrandbits(16) << 16)])]


def g_bf0b8(ctx):
    r = ctx.rng
    ring(ctx, 0x1017cd38)
    table, keys, count = keytab(ctx)
    obj = ctx.buf(0x300)
    w16(ctx, obj + 0x1fa, 0xffff if r.random() < 0.4 else r.choice([keys[0], keys[1], keys[r.randrange(0, 8)], r.getrandbits(16), 0]))
    w32(ctx, obj + 0x27c, table)
    return [obj]


# ---- unsorted_100c ----
def g_c3eb5(ctx):
    r = ctx.rng
    obj = ctx.buf(0x600)
    q = ctx.buf(0x300)
    rr = ctx.buf(0x80)
    w32(ctx, obj, q)
    w32(ctx, obj + 0x4f4, r.choice([0, 1, 1, 1, ctx.u32()]))
    w32(ctx, q + 0x54, rr)
    w8(ctx, rr + 0x59, r.choice([0, 1, 1, 2, 0xff, 0x81, r.getrandbits(8)]))
    w32(ctx, q + 0x264, r.choice([0, 0, 0, 1, ctx.u32()]))
    w16(ctx, q + 0x1f8, r.choice([10, 10, 0, 9, 11, 0x800a, r.getrandbits(16)]))
    return [obj]


def g_c4898(ctx):
    r = ctx.rng
    obj = ctx.buf(0x100)
    q = ctx.buf(0x80)
    wb(ctx, q, rnd_bytes(ctx, 0x80))
    w32(ctx, obj + 0x54, q if r.random() > 0.25 else 0)
    return [obj if r.random() > 0.1 else 0]


def g_c4966(ctx):
    r = ctx.rng
    obj = ctx.buf(0x300)
    wb(ctx, obj, bytes(r.choice([1, 5, 0x42]) for _ in range(0x300)))
    w32(ctx, obj + 0x208, r.choice([0, 1, 2]))
    for i in range(3):
        w32(ctx, obj + 0x20c + 4 * i, 0xffffffff if r.random() < 0.5 else r.choice([0, 1, 7, ctx.u32() & 0xfffffffe]))
    out = ctx.buf(8)
    return [obj if r.random() > 0.08 else 0, out]


def g_cb9f3(ctx):
    r = ctx.rng
    n = 4

    def f(i, a):
        w32(ctx, a + 0xf0, r.choice([0x400, 0x4ff, 0x401 | (r.getrandbits(32) & 0xfffff000), 0x300 | r.getrandbits(8),
                                     r.getrandbits(32), 0x800, 0x400 | (r.getrandbits(8))]))
    ptrtab(ctx, n, 0x100, f)
    w32(ctx, 0x1017b8c8, r.randrange(0, n))
    return []


def spec(name, addr, gen, trials=500, void=False):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials)
    if void:
        d['void'] = True
    return d


SPECS = [
    # unsorted_1000
    spec('Sim_1000716a', 0x1000716a, g_716a, 800, True),
    spec('Sim_1000944e', 0x1000944e, g_obj_buf(0x40)),
    spec('Sim_100096b2', 0x100096b2, g_96b2, 600, True),
    spec('Sim_10009707', 0x10009707, g_obj_buf(0x20)),
    spec('Sim_10009764', 0x10009764, g_world_obj(), 800, True),
    spec('Sim_10009787', 0x10009787, g_9787, 800, True),
    spec('Sim_1000c813', 0x1000c813, g_c813, 600),
    spec('Sim_1000ede4', 0x1000ede4, g_ede4, 300, True),
    # unsorted_1001
    spec('Sim_10010124', 0x10010124, g_10124, 300, True),
    spec('Sim_100128dd', 0x100128dd, g_128dd, 800, True),
    spec('Sim_10013040', 0x10013040, g_13040),
    spec('Sim_10017aa9', 0x10017aa9, g_17aa9, 300, True),
    spec('Sim_1001db6a', 0x1001db6a, g_none, 300, True),
    # unsorted_1002
    spec('Sim_10020456', 0x10020456, g_20456, 800, True),
    spec('Sim_100272a0', 0x100272a0, g_272a0),
    spec('Sim_10028973', 0x10028973, g_28973, 300, True),
    spec('Sim_10028b50', 0x10028b50, g_28b50),
    spec('Sim_1002997e', 0x1002997e, g_2997e),
    # unsorted_1003
    spec('Sim_10037670', 0x10037670, g_none, 300, True),
    spec('Sim_1003772b', 0x1003772b, g_1772b, 600, True),
    spec('Sim_1003775f', 0x1003775f, g_1775f, 800, True),
    spec('Sim_1003b8e3', 0x1003b8e3, g_3b8e3, 800, True),
    spec('Sim_1003db30', 0x1003db30, g_3db30, 400, True),
    # unsorted_1004
    spec('Sim_10047fe0', 0x10047fe0, g_47fe0, 300, True),
    spec('Sim_1004b804', 0x1004b804, g_4b804, 500, True),
    spec('Sim_1004c318', 0x1004c318, g_word),
    spec('Sim_1004c325', 0x1004c325, g_word, 800),
    spec('Sim_1004c3dd', 0x1004c3dd, g_word, 800),
    spec('Sim_1004c426', 0x1004c426, g_4c426, 800),
    spec('Sim_1004c95b', 0x1004c95b, g_none, 300),
    spec('Sim_1004ce50', 0x1004ce50, g_4ce50),
    spec('Sim_1004d26d', 0x1004d26d, g_word),
    # unsorted_1005
    spec('Sim_10057a4a', 0x10057a4a, g_none, 300),
    spec('Sim_1005b89b', 0x1005b89b, g_5b89b, 800),
    # unsorted_1006
    spec('Sim_10062780', 0x10062780, g_62780, 1000, True),
    # unsorted_1007
    spec('Sim_10073027', 0x10073027, g_73027),
    spec('Sim_10076676', 0x10076676, g_76676),
    spec('Sim_1007dc1f', 0x1007dc1f, g_7dc1f, 300, True),
    # unsorted_1008
    spec('Sim_10087a43', 0x10087a43, g_none, 300),
    spec('Sim_10087b23', 0x10087b23, g_grid, 300, True),
    spec('Sim_10087b41', 0x10087b41, g_grid, 300, True),
    # view
    spec('ObjList2_InsertAfter', 0x10088d74, g_insert_after, 1000, True),
    spec('ObjList2_Unlink', 0x10088de6, g_8a871, 800, True),
    spec('Sim_1008a871', 0x1008a871, g_8a871, 1000, True),
    spec('Sim_1008a8f5', 0x1008a8f5, g_8a8f5, 1000, True),
    spec('Sim_1008a14f', 0x1008a14f, g_8a14f, 800, True),
    spec('Sim_1008a16d', 0x1008a16d, g_8a16d, 1000, True),
    spec('Sim_1008a1ea', 0x1008a1ea, g_8a1ea, 800, True),
    spec('Sim_1008a40a', 0x1008a40a, g_8a40a, 800, True),
    spec('Sim_1008d07b', 0x1008d07b, g_8d07b, 800),
    spec('Sim_1008f867', 0x1008f867, g_8f867, 1500),
    # unsorted_1009
    spec('Sim_1009044e', 0x1009044e, g_1044e, 800, True),
    spec('Sim_10090513', 0x10090513, g_90513, 800),
    spec('Sim_10090cc8', 0x10090cc8, g_90cc8, 800, True),
    spec('Sim_1009ca21', 0x1009ca21, g_none, 300),
    spec('Sim_1009eb24', 0x1009eb24, g_eb24),
    # util
    spec('Sim_1009c9a5', 0x1009c9a5, g_9c9a5, 500),
    # unsorted_100a
    spec('Sim_100a78c3', 0x100a78c3, g_a78c3, 300, True),
    spec('Sim_100a9012', 0x100a9012, g_none, 300, True),
    spec('Sim_100aa7f1', 0x100aa7f1, g_aa7f1),
    # unsorted_100b
    spec('Sim_100b534b', 0x100b534b, g_b534b, 800, True),
    spec('Sim_100b5bbb', 0x100b5bbb, g_b5bbb),
    spec('Sim_100bf23f', 0x100bf23f, g_bf23f, 800),
    spec('Sim_100bf0b8', 0x100bf0b8, g_bf0b8, 1000),
    # unsorted_100c
    spec('Sim_100c3eb5', 0x100c3eb5, g_c3eb5, 500),
    spec('Sim_100c4898', 0x100c4898, g_c4898, 300, True),
    spec('Sim_100c4966', 0x100c4966, g_c4966, 800),
    spec('Sim_100cb9f3', 0x100cb9f3, g_cb9f3),
]
