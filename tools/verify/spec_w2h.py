# Differential tests for the wave 2 functions of letter h (see difftest.py).
import struct

SOURCES = [
    'sim/unsorted_1000/w2h_reset.c', 'sim/unsorted_1000/w2h_access.c',
    'sim/unsorted_1001/w2h_tables.c', 'sim/unsorted_1001/w2h_objects.c',
    'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1003/w2h_misc.c',
    'sim/unsorted_1004/w2h_record.c', 'sim/unsorted_1004/w2h_misc.c', 'sim/unsorted_1004/w2h_palette.c',
    'sim/unsorted_1005/w2h_tables.c', 'sim/unsorted_1007/w2h_misc.c', 'sim/unsorted_1008/w2h_misc.c',
    'sim/unsorted_1009/w2h_misc.c', 'sim/unsorted_100a/w2h_misc.c', 'sim/unsorted_100b/w2h_records.c',
    'sim/unsorted_100b/w2h_misc.c', 'sim/unsorted_100c/w2h_misc.c', 'sim/unsorted_100d/w2h_misc.c',
    'sim/unsorted_100f/w2h_mem.c', 'sim/unsorted_1010/w2h_misc.c',
    # already converted callees
    'sim/unsorted_1004/fields.c', 'sim/unsorted_1004/tables.c',
    'sim/unsorted_1008/cell_link.c', 'sim/view/viewrect.c', 'sim/view/matrix.c', 'sim/view/objlist.c',
    'sim/view/w2d_view.c', 'sim/util/ring.c', 'sim/util/field.c',
]

PTRTAB = 0x109e8b40


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def w16(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<H', v & 0xffff))


def w8(ctx, addr, v):
    ctx.m.uc.mem_write(addr, bytes([v & 0xff]))


def rnd(ctx, addr, n):
    ctx.m.uc.mem_write(addr, ctx.rng.getrandbits(8 * n).to_bytes(n, 'little'))


def pick(ctx, *choices):
    return ctx.rng.choice(choices)


def objtab(ctx, n, size=0x300, nullp=0.0):
    """Fills the object table at 0x109e8b40 (256 entries) with n random records; sets the count dword."""
    rg = ctx.rng
    recs = []
    for i in range(256):
        if i < n:
            rec = ctx.buf(size, 'rand')
            recs.append(rec)
            w32(ctx, PTRTAB + 4 * i, 0 if rg.random() < nullp else rec)
        else:
            w32(ctx, PTRTAB + 4 * i, 0)
    w32(ctx, 0x1015dc64, n)
    return recs


# ---- unsorted_1000 -----------------------------------------------------------------
def g_7020(ctx):
    rnd(ctx, 0x10a51ac0, 0x200 * 0x300)
    rnd(ctx, 0x10ac1b00, 0x200 * 0xe + 8)
    for a in (0x1012da30, 0x1015cfe0, 0x1015cfe4, 0x1015cfe8):
        w32(ctx, a, ctx.u32())
    return []


def g_none(ctx):
    return []


def g_89a0(ctx):
    w32(ctx, 0x1015d09c, ctx.u32())
    return []


def g_obj(ctx):
    return [ctx.buf(0x100, 'rand')]


def g_94df(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x80, 'rand')
    link = ctx.buf(0x20, 'rand')
    w32(ctx, obj + 0x30, link if rg.random() < 0.85 else 0)
    a = ctx.buf(4, 'rand')
    b = ctx.buf(4, 'rand')
    return [obj, a if rg.random() < 0.8 else 0, b if rg.random() < 0.8 else 0]


# ---- unsorted_1001 -----------------------------------------------------------------
def g_10063(ctx):
    rnd(ctx, 0x1012ff58, 0x40)
    return [pick(ctx, 0, 1, 7, 8, 9, -1, -3, ctx.rng.randrange(-5, 12), ctx.u32())]


def g_10088(ctx):
    rg = ctx.rng
    rnd(ctx, 0x109b7d40, 0x10 * 0x6c)
    w32(ctx, 0x1013bb40, rg.choice([0, 3, 15, 16, 20, -1, rg.randrange(-3, 20)]))
    return [ctx.u32()]


def g_u32_3(ctx):
    return [ctx.u32(), ctx.u32(), ctx.u32()]


def g_u32_2(ctx):
    return [ctx.u32(), ctx.u32()]


def g_u32_4(ctx):
    return [ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32()]


def build_sets(ctx, obj, key, first_nonzero=True):
    """The structure Sim_100119a7 reads: obj+0x170 -> pointer array, obj+0x174.. three sets."""
    rg = ctx.rng
    arr = ctx.buf(0x100, 'zero')
    w32(ctx, obj + 0x170, arr)
    w16(ctx, obj + 0x1a6, key)
    seen_first = False
    for k in range(3):
        slot = obj + 0x174 + 4 * k
        if rg.random() < 0.2:
            w32(ctx, slot, 0)
            continue
        cnt = rg.randrange(1, 4) if (not seen_first or rg.random() > 0.15) else 0
        if cnt == 0 and not first_nonzero:
            cnt = 1
        seen_first = True
        keys = [rg.choice([key, key, rg.randrange(-2, 4)]) for _ in range(cnt)]
        ns = [rg.randrange(0, 4) for _ in range(cnt)]
        if rg.random() < 0.15:
            ns = [0 for _ in range(cnt)]
        pairs = ctx.buf(4 * max(cnt, 1) + 8, b''.join(struct.pack('<hh', kk, nn) for kk, nn in zip(keys, ns)) + bytes(8))
        total = sum(ns) + 2
        items = ctx.buf(12 * (total + 4), 'zero')
        for j in range(total + 2):
            first = rg.choice([rg.randrange(1, 4), rg.randrange(1, 4), 1, 2, 0]) if rg.random() < 0.85 else 0
            ctx.m.uc.mem_write(items + 12 * j, struct.pack('<hHII', first, rg.randrange(0, 4), rg.getrandbits(32), rg.getrandbits(32)))
        sset = ctx.buf(16, 'zero')
        w16(ctx, sset, cnt)
        w32(ctx, sset + 2, pairs)
        w32(ctx, sset + 6, items)
        w32(ctx, slot, sset)


def g_119a7(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x300, 'rand')
    build_sets(ctx, obj, rg.randrange(0, 4))
    return [obj]


def g_10721(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x300, 'rand')
    src = ctx.buf(0x40, 'rand')
    key = rg.randrange(0, 4)
    build_sets(ctx, obj, key)
    w8(ctx, src + 0xf, key)
    w32(ctx, obj, 0)  # keep the first pointer array entry harmless
    return [obj, src]


def g_4094e(ctx):
    rg = ctx.rng
    n = rg.choice([0, 1, 3, 6])
    recs = objtab(ctx, n, 0x300)
    cur = rg.choice([0, 1, 2, ctx.u32()])
    w32(ctx, 0x1017b8c8, cur)
    src = ctx.buf(0x40, 'rand')
    for r in recs:
        w32(ctx, r + 0x14, rg.choice([3, 3, 3, 1, 0]))
        w32(ctx, r + 0xc, rg.choice([cur, cur, cur ^ 1, rg.getrandbits(32)]))
        key = rg.randrange(0, 4)
        build_sets(ctx, r, key)
        w8(ctx, src + 3 + 0xf, key)
    return [src - 3, rg.choice([rg.randrange(0, max(n, 1)), rg.randrange(-2, 8), 0]) if rg.random() < 0.9 else ctx.u32()]


def g_13f2a(ctx):
    rg = ctx.rng
    n = rg.choice([0, 1, 3, 8, 15, 25, 25])
    key = rg.choice([rg.randrange(0, 6), rg.randrange(0, 25), rg.randrange(0, 25), -rg.randrange(1, 4)])
    obj = ctx.buf(0x40, 'rand')
    w32(ctx, obj + 4, (rg.getrandbits(16) << 16) | (key & 0xffff))
    recs = objtab(ctx, 25, 0x200, nullp=0.08)
    for k in range(4):
        w32(ctx, 0x109b7d54 + 0x6c * k, rg.choice([0, 0, 1, 2]))
    for r in recs:
        w32(ctx, r + 4, rg.choice([rg.randrange(0, 25), rg.randrange(0, 25), key & 0xffffffff]))
        w32(ctx, r + 8, rg.randrange(0, 4))
        w16(ctx, r + 0x1c, rg.choice([0, 0, 0, 1, 8, 6, 2, 4, rg.getrandbits(16)]))
        w16(ctx, r + 0x1aa, ((key | 0x200) & 0xffff) if rg.random() < 0.6 else rg.getrandbits(16))
        w16(ctx, r + 0x1a6, 3 if rg.random() < 0.7 else rg.getrandbits(16))
    w32(ctx, 0x1015dc64, n)
    return [obj]


def g_152fc(ctx):
    rg = ctx.rng
    objtab(ctx, 26, 0x300)
    rnd(ctx, 0x109a8664, 26 * 0x54)
    rnd(ctx, 0x109e9ba0, 26 * 0x50)
    lo = rg.randrange(0, 25)
    kind = rg.choice([0x100, 0x200, 0x400, 0x200, 0x400, 0x100, rg.choice([0, 0x300, 0x800])])
    b = kind | lo
    if rg.random() < 0.06:
        b |= rg.choice([0x1000, 0x2000, 0x8000])
    if rg.random() < 0.3:
        b |= rg.getrandbits(16) << 16
    c = rg.choice([1, 2, 4, 8, 0x100, 0x1000, 0x4000, 0x4000])
    a = rg.choice([rg.randrange(0, 32), ctx.u32()])
    return [a, b, c]


def g_15bc9(ctx):
    rnd(ctx, 0x101302e8, 12 * 4)
    rnd(ctx, 0x10a38a40, 13 * 4)
    return []


def g_16a6b(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x400, 'rand')
    w32(ctx, obj, rg.choice([rg.randrange(1, 11), rg.randrange(1, 11), 0, 11, -1, ctx.u32()]))
    w32(ctx, obj + 4, rg.randrange(0, 3))
    for base in (0x10130cd8, 0x10130d18, 0x10130d58, 0x10130d98, 0x10130dd8, 0x10130e18):
        ctx.m.uc.mem_write(base, b''.join(struct.pack('<h', rg.randrange(0, 13)) for _ in range(32)))
    rnd(ctx, 0x10a38a40, 13 * 4)
    slot = rg.randrange(-4, 8)
    sel = rg.choice([-1, -1, rg.randrange(0, 15), rg.randrange(0, 15)])
    other = rg.choice([rg.randrange(0, 3), rg.randrange(0, 3), 0x10000 | rg.randrange(0, 3)])
    return [obj, slot & 0xffffffff, sel & 0xffffffff, other]


def g_17a05(ctx):
    rg = ctx.rng
    if rg.random() < 0.04:
        return [0]
    obj = ctx.buf(0x100, 'rand')
    cur = rg.choice([0, 1, 5])
    w32(ctx, 0x1017b8c8, cur)
    w32(ctx, obj, rg.choice([1, 9, 1, 9, 2, 0]))
    w32(ctx, obj + 0x14, rg.choice([1, 3, 3, 0, 5]))
    w32(ctx, obj + 0xc, rg.choice([cur, cur, cur + 1]))
    rec = ctx.buf(0x600, 'rand')
    w16(ctx, rec + 0x562, rg.choice([0x60e, 0x8, 0x600, 0, 0x1, 0x608, rg.getrandbits(16)]))
    w16(ctx, rec + 0x560, rg.choice([0x20, 0, rg.getrandbits(16)]))
    w32(ctx, obj + 0x28, rec)
    w32(ctx, obj + 0x54, ctx.buf(0x80, 'rand'))
    return [obj]


# ---- unsorted_1002 -----------------------------------------------------------------
def g_2d890(ctx):
    rg = ctx.rng
    n = ctx.buf(0xa0, 'zero')
    other = ctx.buf(0x40, 'zero')
    owner = ctx.buf(8, 'zero')
    glob = ctx.buf(0x40, 'zero')
    head = ctx.buf(0x40, 'zero')
    flags = rg.choice([0x800, 0x800, 0x800, 0x801, 0x1800, 0xffff, 0, 0x7ff, rg.getrandbits(16)])
    w16(ctx, n, flags)
    w32(ctx, n + 4, rg.choice([1, 1, 1, 0, rg.getrandbits(32)]))
    w32(ctx, n + 0x8c, rg.choice([0, 1, 4, 8, 9, 0xffffffff, rg.randrange(0, 9), rg.randrange(0, 9), rg.getrandbits(32)]))
    w32(ctx, n + 0x90, rg.choice([0, 1, 4, 8, 9, 0xffffffff, rg.randrange(0, 9), rg.randrange(0, 9), rg.getrandbits(32)]))
    nodes = [other, owner, head, glob]
    ptr = lambda: 0 if rg.random() < 0.2 else rg.choice(nodes)
    w32(ctx, n + 0xc, ptr())
    w32(ctx, n + 0x10, ptr())
    w32(ctx, n + 0x14, ptr())
    w32(ctx, n + 0x18, ptr())
    r = rg.random()
    if r < 0.2:
        w32(ctx, n + 0x28, owner)
    w32(ctx, 0x1017c8cc, rg.choice([glob, glob, glob, 0]))
    w32(ctx, 0x1017c8c4, rg.choice([0, 1, 1, rg.getrandbits(32)]))
    if rg.random() < 0.6:
        w32(ctx, glob + 0x10, head)
    if rg.random() < 0.5:
        w32(ctx, glob + 0x18, other)
    cells = [0x1017b9b0] + [0x1017ba28 + x * 0x18c + y * 0x2c for x in range(9) for y in range(9)]
    for c in cells:
        w32(ctx, c + 0x10, rg.choice([0, 0, head]))
        w32(ctx, c + 0x18, rg.choice([0, 0, other]))
        w32(ctx, c + 0x14, 0)
    return [n, rg.choice([4, 4, 3, 0, 5, rg.getrandbits(32)])]


# ---- unsorted_1003 -----------------------------------------------------------------
def g_33540(ctx):
    return [ctx.buf(8, 'rand') if ctx.rng.random() < 0.8 else 0]


def g_36820(ctx):
    rg = ctx.rng
    w32(ctx, 0x109e3d20, rg.choice([0, 1, 2, ctx.u32()]))
    return [rg.choice([0, 0, 1, ctx.u32()]), rg.choice([0, 1, 2, 3, -1, ctx.u32()])]


def g_3830e(ctx):
    rg = ctx.rng
    rnd(ctx, 0x1015e6a0, 256 * 3)
    start = rg.choice([0, 0, rg.randrange(-2, 260), rg.randrange(0, 256), 255, 256])
    count = rg.choice([1, 256, rg.randrange(-1, 260), rg.randrange(1, 40), 256 - start, 257 - start])
    r = rg.random()
    if r < 0.07:
        dest = 0
    elif r < 0.17:
        dest = 0x1015e6a0 + rg.randrange(0, 700)
    else:
        dest = ctx.buf(3 * 258 + 8, 'rand')
    return [start, count, dest]


def g_3b45d(ctx):
    return [ctx.rng.choice([0, 1, 5, 40, ctx.rng.randrange(-5, 40)]), ctx.u32()]


# ---- unsorted_1004 -----------------------------------------------------------------
def g_429d5(ctx):
    rg = ctx.rng
    # The original keeps the caller's upper eax bits when [0x101655ac] is -1; make that input fixed (xor eax,eax).
    ctx.m.uc.mem_write(0x100429e2, b'\x31\xc0')
    idx = rg.choice([-1, -1, 0, 1, 5, rg.randrange(0, 25)])
    recs = objtab(ctx, 25, 0x100)
    for r in recs:
        w32(ctx, r + 8, rg.randrange(0, 4))
    for k in range(4):
        w32(ctx, 0x109b7d54 + 0x6c * k, rg.choice([0, 0, 1]))
    w32(ctx, 0x101655ac, idx)
    return []


def g_47e95(ctx):
    rg = ctx.rng
    p = ctx.buf(16, 'rand')
    w32(ctx, p, rg.choice([0, 100, -100, ctx.u32()]))
    w32(ctx, p + 4, rg.choice([0, 1000, -1000, ctx.u32()]))
    w16(ctx, p + 8, rg.choice([0, 1, 2, 4, 31, 32, 33, 0xffff, -3, rg.getrandbits(16)]))
    return [p]


def g_48bd6(ctx):
    rg = ctx.rng
    tbl = ctx.m.uc.mem_read(0x10144220, 12)
    v = rg.choice([rg.randrange(0x2a, 0x3e), rg.randrange(0x5a, 0x5f), rg.choice(list(tbl)), rg.choice(list(tbl)), rg.randrange(0, 256)])
    p = ctx.buf(8, 'rand')
    w8(ctx, p, v)
    return [p]


def g_bfa9(ctx):
    rg = ctx.rng
    return [rg.choice([ctx.u32(), ctx.u32(), 0x1000, 0xefff, 0x80001000, 0xfffff000])]


def g_c221(ctx):
    rg = ctx.rng
    rnd(ctx, 0x101116a6, 40 * 0xaa)
    rnd(ctx, 0x10124eac, 0x20 * 0x98)
    w32(ctx, 0x10126cf4, ctx.u32())
    return [rg.choice([rg.randrange(0, 30), rg.randrange(-2, 40)]), rg.choice([rg.getrandbits(32), rg.getrandbits(13), rg.getrandbits(16)])]


def g_b7b9(ctx):
    rnd(ctx, 0x1015e6a0, 256 * 3)
    rnd(ctx, 0x109b8c00, 256 * 3 + 4)
    return []


def g_de34(ctx):
    rg = ctx.rng
    w32(ctx, 0x1015f22c, rg.choice([0, 1, 2, 5, 6, 8, 8, 8, 31, 32, 33, 0x105, rg.randrange(0, 12)]))
    dst = ctx.buf(0x300 + 16, 'rand')
    w32(ctx, 0x1015f360, dst)
    ch = lambda: rg.choice([rg.randrange(0, 256), rg.randrange(0, 256), rg.randrange(0, 64), 255, -1, rg.randrange(-300, 300), ctx.u32()])
    return [rg.randrange(0, 250), ch(), ch(), ch()]


def g_ecb8(ctx):
    rg = ctx.rng
    rnd(ctx, 0x1015e6a0, 256 * 3)
    rec = ctx.buf(0x20, 'rand')
    w32(ctx, rec + 0xc, rg.choice([0, 1, 5, 64, 255, 256, 256, rg.randrange(0, 257), -1]))
    w32(ctx, 0x109e3da4, rec)
    w32(ctx, 0x1013b9a8, rg.choice([0, 0, 0, 0, 1]))
    w32(ctx, 0x1015f22c, rg.choice([1, 2, 6, 8, 8, 5]))
    w32(ctx, 0x1015f360, ctx.buf(0x300 + 16, 'rand'))
    return []


# ---- unsorted_1005 -----------------------------------------------------------------
def g_52530(ctx):
    rnd(ctx, 0x109b7d40, 0x10 * 0x6c)
    rnd(ctx, 0x109b8420, 0x10 * 0x74)
    rnd(ctx, 0x109b68e0, 0x74)
    return []


def g_52632(ctx):
    rnd(ctx, 0x109b7d40, 0x10 * 0x6c)
    return [pick(ctx, 0, 1, 15, 16, 17, -1, ctx.rng.randrange(-3, 20), ctx.u32())]


def g_52c60(ctx):
    rg = ctx.rng
    recs = objtab(ctx, 25, 0x100, nullp=0.04)
    for r in recs:
        w32(ctx, r + 8, rg.randrange(0, 4))
    for k in range(4):
        w32(ctx, 0x109b7d54 + 0x6c * k, rg.choice([0, 0, 1, 2]))
    a = rg.randrange(0, 25)
    return [a, a if rg.random() < 0.3 else rg.randrange(0, 25)]


def g_5436d(ctx):
    rg = ctx.rng
    n = rg.choice([0, 1, 2, 3, 5, 8, rg.randrange(-1, 8)])
    ents = b''
    for i in range(max(n, 0)):
        val = rg.getrandbits(32) if rg.random() < 0.5 else rg.choice([0, 0xf0f0f0f0, 0xffffffff, 1])
        d = ctx.buf(8, struct.pack('<2I', val, 0))
        mask = rg.choice([rg.getrandbits(32), 1, 0xf, 0x80000000, rg.getrandbits(4), 0])
        kind = rg.choice([1, 1, 0, 2, 3, rg.getrandbits(32)])
        if rg.random() < 0.8:  # make the entry pass
            if kind == 1:
                val &= ~mask & 0xffffffff
            else:
                val |= mask | 1
                mask |= 1
            ctx.m.uc.mem_write(d, struct.pack('<I', val))
        ents += struct.pack('<3I', d, mask, kind)
    return [n, ctx.buf(max(len(ents), 12) + 12, ents)]


def g_57aa9(ctx):
    rg = ctx.rng
    rnd(ctx, 0x10165ee0, 0x40 * 2 + 8)
    head = rg.choice([0, 1, 0x3e, 0x3f, 0x3f, 0x40, rg.randrange(0, 0x40)])
    tail = head if rg.random() < 0.2 else rg.randrange(0, 0x41)
    w32(ctx, 0x10166868, head)
    w32(ctx, 0x1016686c, tail)
    return [ctx.buf(8, 'rand')]


def g_585e1(ctx):
    rg = ctx.rng
    cnt = rg.choice([0, 1, 3, 10, 20])
    w32(ctx, 0x109b0eac, cnt)
    rnd(ctx, 0x109b16e0, 22 * 0x4c)
    src = ctx.buf(0x60, 'rand') if rg.random() < 0.93 else 0
    idx = rg.choice([rg.randrange(-2, cnt + 3), rg.randrange(0, max(cnt, 1)), 0, 21])
    mask = rg.choice([rg.getrandbits(8), rg.getrandbits(8), 0xff, 0, 0x80, 8, rg.getrandbits(32)])
    return [idx, mask, src]


# ---- unsorted_1007 -----------------------------------------------------------------
def g_7655f(ctx):
    rg = ctx.rng
    keys = [rg.randrange(0, 12) for _ in range(rg.randrange(0, 6))]
    nodes = [ctx.buf(0x20, 'rand') for _ in keys]
    for i, (nd, k) in enumerate(zip(nodes, keys)):
        w32(ctx, nd, k)
        w32(ctx, nd + 0x14, nodes[i + 1] if i + 1 < len(nodes) else 0)
    w32(ctx, 0x1015dc14, nodes[0] if nodes else 0)
    return [rg.choice(keys + [99, 0, 0xffffffff]) if keys else rg.randrange(0, 12)]


def g_79c86(ctx):
    rg = ctx.rng
    cnt = rg.choice([0, 3, 10, 25, 25])
    recs = objtab(ctx, 26, 0x200)
    w32(ctx, 0x1015dc64, cnt)
    rnd(ctx, 0x109e9ba0, 26 * 0x50)
    idx = rg.randrange(0, 25)
    kind = rg.choice([2, 2, 4, 4, 1, 0, 3])
    a = ctx.buf(4, bytes([idx, kind, 0, 0]))

    def link():
        top = rg.choice([0, 0, 0, 0, 0x40000000, 0x80000000, 0xc0000000])
        low = rg.choice([rg.randrange(0, 26), rg.randrange(0, 26), 0xffff, 0xfffe, 0x19, 0x18, rg.getrandbits(16)])
        return top | low | (rg.getrandbits(8) << 16 if rg.random() < 0.3 else 0)
    for r in recs:
        w32(ctx, r + 0x150, link())
        w32(ctx, r + 8, rg.randrange(0, 4))
    for i in range(26):
        w32(ctx, 0x109e9ba0 + 0x50 * i + 0x4c, link())
    return [a, rg.randrange(0, 4)]


def g_7c884(ctx):
    w32(ctx, 0x1015dc78, pick(ctx, 0, 181, -181, 1000, -1000, 0x7fffffff, 0x80000000, ctx.u32(), ctx.u32()))
    return []


# ---- unsorted_1008 .. 100a -----------------------------------------------------------
def g_d405(ctx):
    return [ctx.buf(0x40, 'rand')]


def g_98a80(ctx):
    for a in (0x101c47b0, 0x101c4750, 0x101c4f64, 0x101c4db8, 0x101c510c, 0x101c4fd8, 0x101c5124, 0x101c4698):
        w32(ctx, a, ctx.u32())
    for a in (0x1014bb94, 0x1014bbac, 0x1014bbc0, 0x1014bbd4, 0x1014bbe8, 0x1014bbfc, 0x1014b9e8, 0x1014b9ec):
        w32(ctx, a, ctx.u32())
    return []


def g_u32_1(ctx):
    return [ctx.u32()]


def g_9eb2e(ctx):
    rg = ctx.rng
    v = rg.choice([rg.randrange(-20, 20), ctx.u32()])
    lo = rg.choice([v, v + 1, v - 1, rg.randrange(-20, 20), ctx.u32()])
    hi = rg.choice([v, v + 1, v - 1, rg.randrange(-20, 20), ctx.u32()])
    w32(ctx, 0x1017d080, lo)
    w32(ctx, 0x1017d084, hi)
    return [v]


def g_a88ff(ctx):
    rg = ctx.rng
    buf = ctx.buf(0x200, 'rand')
    top = buf + 0x80
    w32(ctx, 0x1017d174, top + 0x40)
    limit = (top - 0xc8) & 0xffffffff
    w32(ctx, 0x1017d178, rg.choice([limit, limit - 1, limit + 1, 0, 0xffffffff, ctx.u32()]))
    w32(ctx, 0x101c9ea4, ctx.u32())
    return []


def g_aa270(ctx):
    obj = ctx.buf(0x200, 'rand')
    return [obj, ctx.buf(16, 'rand')]


# ---- unsorted_100b .. 100d -----------------------------------------------------------
def fill_records(ctx, n=40):
    rnd(ctx, 0x101a2100, n * 0x88)
    rnd(ctx, 0x101a2100 + 0x401 * 0x88, 0x88)


def g_b391c(ctx):
    rg = ctx.rng
    fill_records(ctx)
    w32(ctx, 0x1017d21c, rg.choice([0, 1, 1, 1]))
    w32(ctx, 0x1017d218, rg.choice([0, 5, 0x401, 0x402, 0x403, rg.randrange(0, 0x410), ctx.u32()]))
    w32(ctx, 0x101a1168, rg.choice([0, 3, 20, rg.randrange(0, 40), -1]))
    w32(ctx, 0x101c42a4, rg.choice([0, 5, 30, 100, rg.randrange(-3, 50)]))
    w32(ctx, 0x1017d204, ctx.buf(0x400, 'rand'))
    w32(ctx, 0x1017d208, ctx.buf(0x400, 'rand'))
    return [ctx.u32()]


def g_b39af(ctx):
    fill_records(ctx)
    return [pick(ctx, 0, 1, 39, 0x401, 0x402, ctx.rng.randrange(0, 45))]


def g_b4073(ctx):
    fill_records(ctx)
    return [pick(ctx, 0, 1, 5, 39, 0x401, ctx.rng.randrange(0, 45))]


def g_b4149(ctx):
    fill_records(ctx)
    w32(ctx, 0x1017d218, ctx.u32())
    w32(ctx, 0x1017d21c, ctx.u32())
    return []


def g_a252(ctx):
    rg = ctx.rng
    s = bytes(rg.choice(b' \t') for _ in range(rg.randrange(0, 8)))
    tail = bytes(rg.choice(b'ab \t\x01\xe0\x7f') for _ in range(rg.randrange(0, 6)))
    r = rg.random()
    if r < 0.15:
        tail = b''
    elif r < 0.3:
        tail = b'\x80' + tail
    return [ctx.buf(len(s) + len(tail) + 8, s + tail + b'\0\0\0\0\0\0\0\0')]


def g_a96a(ctx):
    rg = ctx.rng
    objtab(ctx, 10, 0x300)
    w32(ctx, 0x1017b8c8, rg.randrange(0, 10))
    return []


def g_d624d(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x600, 'rand')
    ents = ctx.buf(0x128 * 5, 'rand')
    w32(ctx, obj + 0x84, ents)
    w32(ctx, obj + 0x4fc, rg.choice([-1, -1, 0, 1, 2, 3, 4]))
    for i in range(5):
        w32(ctx, ents + 0x128 * i + 4, rg.randrange(0, 4))
        w32(ctx, ents + 0x128 * i + 0x20, rg.choice([-1, 0, 1, 5, 0x7fffffff, 0x80000000]))
    for t in range(4):
        w32(ctx, 0x10145534 + 0xd6 * t, rg.choice([-1, 0, 1, 7]))
    return [obj]


def g_swap(ctx):
    rg = ctx.rng
    a = ctx.buf(0x100, 'rand')
    r = rg.random()
    if r < 0.1:
        b = a
    elif r < 0.2:
        b = a + rg.randrange(1, 8)
    else:
        b = ctx.buf(0x100, 'rand')
    return [a, b, rg.choice([0, 1, rg.randrange(0, 70), rg.randrange(0, 70), 100])]


# ---- unsorted_1010 -------------------------------------------------------------------
def g_106900(ctx):
    rg = ctx.rng
    head = 0x10159e38
    n = rg.randrange(0, 4)
    nodes = [ctx.buf(0x20, 'rand') for _ in range(n)]
    ranges = []
    lo = rg.choice([0x00500000, 0x00800000, 0x01000000])
    for i, nd in enumerate(nodes):
        l = lo + 0x100000 * i
        h = l + 0x80000
        ranges.append((l, h))
        w32(ctx, nd, nodes[i + 1] if i + 1 < n else head)
        w32(ctx, nd + 0x10, l)
        w32(ctx, nd + 0x14, h)
    w32(ctx, head, nodes[0] if nodes else head)
    hl = rg.choice([0, 0x00100000, 0xfffffff0])
    w32(ctx, head + 0x10, hl)
    w32(ctx, head + 0x14, hl + rg.choice([0, 0x1000, 0x80000]))
    if ranges and rg.random() < 0.8:
        l, h = rg.choice(ranges)
    else:
        l, h = hl, hl + 0x80000
    page = l + 0x1000 * rg.randrange(0, 0x7f)
    addr = page + rg.choice([0xff, 0x100, 0x100, 0x108, 0x110, 0x2f0, 0x123, 0xff0, 0xfff, rg.getrandbits(12), rg.getrandbits(12) & ~0xf])
    if rg.random() < 0.1:
        addr = rg.choice([l, h, l + 1, h - 1, 0, 0xffffffff])
    return [addr & 0xffffffff, ctx.buf(8, 'rand'), ctx.buf(8, 'rand')]


def g_107fd0(ctx):
    rnd(ctx, 0x1017eac0, 0x120)
    for a in (0x1017ebc4, 0x1017ebc8, 0x1017ebd0, 0x1017ebd4, 0x1017ebd8):
        w32(ctx, a, ctx.u32())
    return []


def g_87b0(ctx):
    rg = ctx.rng
    r = rg.random()
    if r < 0.4:
        return [rg.getrandbits(32)]
    bits = [0x1, 0x2, 0x4, 0x8, 0x10, 0x100, 0x200, 0x10000, 0x20000, 0x40000, 0x80000]
    v = 0
    for b in bits:
        if rg.random() < 0.5:
            v |= b
    return [v | (rg.getrandbits(32) & rg.choice([0, 0xfff00000, 0x0000fc00]))]


SPECS = [
    dict(name='Sim_10007020', orig=0x10007020, c='Sim_10007020', gen=g_7020, void=True, trials=300),
    dict(name='Sim_100071f1', orig=0x100071f1, c='Sim_100071f1', gen=g_none, void=True),
    dict(name='Sim_100089a0', orig=0x100089a0, c='Sim_100089a0', gen=g_89a0),
    dict(name='Sim_1000945e', orig=0x1000945e, c='Sim_1000945e', gen=g_obj),
    dict(name='Sim_100094df', orig=0x100094df, c='Sim_100094df', gen=g_94df, void=True),
    dict(name='Sim_10010063', orig=0x10010063, c='Sim_10010063', gen=g_10063),
    dict(name='Sim_10010088', orig=0x10010088, c='Sim_10010088', gen=g_10088),
    dict(name='Sim_100119a7', orig=0x100119a7, c='Sim_100119a7', gen=g_119a7, void=True, trials=800),
    dict(name='Sim_10013920', orig=0x10013920, c='Sim_10013920', gen=g_u32_3),
    dict(name='Sim_10013e0a', orig=0x10013e0a, c='Sim_10013e0a', gen=g_u32_2),
    dict(name='Sim_10013f2a', orig=0x10013f2a, c='Sim_10013f2a', gen=g_13f2a, trials=800),
    dict(name='Sim_100152fc', orig=0x100152fc, c='Sim_100152fc', gen=g_152fc, trials=1500),
    dict(name='Sim_10015bc9', orig=0x10015bc9, c='Sim_10015bc9', gen=g_15bc9),
    dict(name='Sim_10016a6b', orig=0x10016a6b, c='Sim_10016a6b', gen=g_16a6b, void=True, trials=800),
    dict(name='Sim_10017a05', orig=0x10017a05, c='Sim_10017a05', gen=g_17a05, trials=600),
    dict(name='Sim_10019d96', orig=0x10019d96, c='Sim_10019d96', gen=g_none),
    dict(name='Sim_1001f4dd', orig=0x1001f4dd, c='Sim_1001f4dd', gen=g_none),
    dict(name='Sim_10028289', orig=0x10028289, c='Sim_10028289', gen=g_none, void=True),
    dict(name='Sim_10028a92', orig=0x10028a92, c='Sim_10028a92', gen=g_obj, void=True),
    dict(name='Sim_1002d244', orig=0x1002d244, c='Sim_1002d244', gen=g_u32_4),
    dict(name='Sim_1002d890', orig=0x1002d890, c='Sim_1002d890', gen=g_2d890, void=True, trials=600),
    dict(name='Sim_10033540', orig=0x10033540, c='Sim_10033540', gen=g_33540),
    dict(name='Sim_10036820', orig=0x10036820, c='Sim_10036820', gen=g_36820),
    dict(name='Sim_1003830e', orig=0x1003830e, c='Sim_1003830e', gen=g_3830e, trials=600),
    dict(name='Sim_1003b45d', orig=0x1003b45d, c='Sim_1003b45d', gen=g_3b45d, void=True),
    dict(name='Sim_10040721', orig=0x10040721, c='Sim_10040721', gen=g_10721, void=True, trials=600),
    dict(name='Sim_1004094e', orig=0x1004094e, c='Sim_1004094e', gen=g_4094e, void=True, trials=600),
    dict(name='Sim_100429d5', orig=0x100429d5, c='Sim_100429d5', gen=g_429d5),
    dict(name='Sim_10047e95', orig=0x10047e95, c='Sim_10047e95', gen=g_47e95),
    dict(name='Sim_10048bd6', orig=0x10048bd6, c='Sim_10048bd6', gen=g_48bd6, void=True),
    dict(name='Sim_1004b7b9', orig=0x1004b7b9, c='Sim_1004b7b9', gen=g_b7b9),
    dict(name='Sim_1004bfa9', orig=0x1004bfa9, c='Sim_1004bfa9', gen=g_bfa9),
    dict(name='Sim_1004c221', orig=0x1004c221, c='Sim_1004c221', gen=g_c221),
    dict(name='Sim_1004de34', orig=0x1004de34, c='Sim_1004de34', gen=g_de34, void=True, trials=600),
    dict(name='Sim_1004ecb8', orig=0x1004ecb8, c='Sim_1004ecb8', gen=g_ecb8, void=True),
    dict(name='Sim_10052530', orig=0x10052530, c='Sim_10052530', gen=g_52530, void=True),
    dict(name='Sim_10052632', orig=0x10052632, c='Sim_10052632', gen=g_52632),
    dict(name='Sim_10052c60', orig=0x10052c60, c='Sim_10052c60', gen=g_52c60),
    dict(name='Sim_1005436d', orig=0x1005436d, c='Sim_1005436d', gen=g_5436d, trials=600),
    dict(name='Sim_10057aa9', orig=0x10057aa9, c='Sim_10057aa9', gen=g_57aa9),
    dict(name='Sim_100585e1', orig=0x100585e1, c='Sim_100585e1', gen=g_585e1, void=True, trials=600),
    dict(name='Sim_1007655f', orig=0x1007655f, c='Sim_1007655f', gen=g_7655f),
    dict(name='Sim_10079c86', orig=0x10079c86, c='Sim_10079c86', gen=g_79c86, trials=1000),
    dict(name='Sim_1007c884', orig=0x1007c884, c='Sim_1007c884', gen=g_7c884, void=True),
    dict(name='Sim_1007d8b0', orig=0x1007d8b0, c='Sim_1007d8b0', gen=g_none),
    dict(name='Sim_1007dba3', orig=0x1007dba3, c='Sim_1007dba3', gen=g_none),
    dict(name='Sim_10083490', orig=0x10083490, c='Sim_10083490', gen=g_none, void=True),
    dict(name='Sim_1008d405', orig=0x1008d405, c='Sim_1008d405', gen=g_d405, void=True),
    dict(name='Sim_10098a80', orig=0x10098a80, c='Sim_10098a80', gen=g_98a80, void=True),
    dict(name='Sim_10098e54', orig=0x10098e54, c='Sim_10098e54', gen=g_u32_1),
    dict(name='Sim_1009e5c3', orig=0x1009e5c3, c='Sim_1009e5c3', gen=g_u32_1, void=True),
    dict(name='Sim_1009eb2e', orig=0x1009eb2e, c='Sim_1009eb2e', gen=g_9eb2e),
    dict(name='Sim_100a774e', orig=0x100a774e, c='Sim_100a774e', gen=g_15bc9),
    dict(name='Sim_100a88ff', orig=0x100a88ff, c='Sim_100a88ff', gen=g_a88ff),
    dict(name='Sim_100aa270', orig=0x100aa270, c='Sim_100aa270', gen=g_aa270, void=True),
    dict(name='Sim_100b391c', orig=0x100b391c, c='Sim_100b391c', gen=g_b391c, trials=600),
    dict(name='Sim_100b39af', orig=0x100b39af, c='Sim_100b39af', gen=g_b39af),
    dict(name='Sim_100b4073', orig=0x100b4073, c='Sim_100b4073', gen=g_b4073, void=True),
    dict(name='Sim_100b4149', orig=0x100b4149, c='Sim_100b4149', gen=g_b4149, void=True),
    dict(name='Sim_100bda90', orig=0x100bda90, c='Sim_100bda90', gen=g_obj, void=True),
    dict(name='Sim_100c6fb8', orig=0x100c6fb8, c='Sim_100c6fb8', gen=g_none, void=True),
    dict(name='Sim_100ca96a', orig=0x100ca96a, c='Sim_100ca96a', gen=g_a96a, void=True),
    dict(name='Sim_100cc96c', orig=0x100cc96c, c='Sim_100cc96c', gen=g_none),
    dict(name='Sim_100d624d', orig=0x100d624d, c='Sim_100d624d', gen=g_d624d, trials=600),
    dict(name='Mem_Swap', orig=0x100fded0, c='Mem_Swap', gen=g_swap, void=True),
    dict(name='Sim_10100390', orig=0x10100390, c='Sim_10100390', gen=g_u32_1, void=True),
    dict(name='Sim_10106900', orig=0x10106900, c='Sim_10106900', gen=g_106900, trials=800),
    dict(name='Sim_10107fd0', orig=0x10107fd0, c='Sim_10107fd0', gen=g_107fd0),
    dict(name='Sim_101087b0', orig=0x101087b0, c='Sim_101087b0', gen=g_87b0, trials=600),
]
