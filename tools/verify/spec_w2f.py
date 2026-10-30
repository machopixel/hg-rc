# Differential tests for the wave 2 functions of letter f (see difftest.py).
import struct

SOURCES = [
    'sim/unsorted_1000/w2f_records.c', 'sim/unsorted_1001/w2f_misc.c', 'sim/unsorted_1002/w2f_misc.c',
    'sim/unsorted_1003/w2f_misc.c', 'sim/unsorted_1004/w2f_table.c', 'sim/unsorted_1005/w2f_globals.c',
    'sim/unsorted_1006/w2f_records.c', 'sim/unsorted_1007/w2f_misc.c', 'sim/unsorted_1008/w2f_misc.c',
    'sim/unsorted_1009/w2f_misc.c', 'sim/unsorted_100a/w2f_links.c', 'sim/unsorted_100b/w2f_tables.c',
    'sim/unsorted_100c/w2f_objects.c', 'sim/unsorted_100d/w2f_records.c',
    # existing converted code the functions above call
    'sim/unsorted_1007/node_flag.c', 'sim/view/objlist.c', 'sim/view/viewrect.c', 'sim/view/matrix.c',
    'sim/util/field.c',
    'sim/unsorted_1010/w2f_mantissa.c', 'sim/unsorted_1010/w2f_ld12.c', 'sim/unsorted_1010/w2f_digits.c',
    'sim/unsorted_1010/w2f_misc.c',
]


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def w16(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<H', v & 0xffff))


def wr(ctx, addr, fmt, *vals):
    ctx.m.uc.mem_write(addr, struct.pack(fmt, *[v & 0xffffffff if fmt[-1] in 'Ii' else v & 0xffff for v in vals]))


def rd32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def rnd(ctx, addr, n):
    ctx.m.uc.mem_write(addr, bytes(ctx.rng.getrandbits(8) for _ in range(n)))


def idx(ctx, hi=0x200):
    rg = ctx.rng
    r = rg.random()
    if r < 0.5: return rg.randrange(0, hi)
    if r < 0.6: return -1
    if r < 0.7: return hi
    if r < 0.8: return rg.randrange(-5, 0)
    return rg.choice([hi - 1, 0, 1, hi + 1, 0x7fffffff, 0x80000000])


def none(ctx):
    return []


# ---- 0x1000xxxx -------------------------------------------------------------
def g_6db8(ctx):
    rg = ctx.rng
    rnd(ctx, 0x10ac1b00, 14 * 512)
    w32(ctx, 0x1015cfe0, rg.choice([-1, 0, 5, 0x1ff, 0x200, rg.getrandbits(32), rg.randrange(0, 0x200)]))
    w32(ctx, 0x1015cfe4, rg.getrandbits(32))
    w32(ctx, 0x1015cfe8, rg.choice([0, 0, 1, 7]))
    return [idx(ctx), rg.choice([-1, -1, idx(ctx)])]


def g_93be(ctx):
    rg = ctx.rng
    p = 0 if rg.random() < 0.12 else ctx.buf(16)
    return [p, ctx.u32() if rg.random() < 0.5 else rg.getrandbits(32)]


def g_95b6(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x60)
    blk = ctx.buf(0x18 + 0x58 * 4 + 16)
    w32(ctx, obj + 0x30, 0 if rg.random() < 0.15 else blk)
    outs = [0 if rg.random() < 0.25 else ctx.buf(8) for _ in range(3)]
    if rg.random() < 0.1: outs[1] = outs[0]
    if rg.random() < 0.05 and outs[0]: outs[2] = blk + 0x18 + 0x58 * 2 + 4
    return [obj, rg.randrange(0, 4)] + outs


def g_c6e3(ctx):
    rg = ctx.rng
    if rg.random() < 0.1:
        return [0, rg.randrange(0, 4), ctx.u32()]
    obj = ctx.buf(0x100)
    recs = ctx.buf(0x128 * 4)
    w32(ctx, obj + 0x84, recs)
    i = rg.randrange(0, 4)
    k = rg.choice([0, 1, 2, 5, 40, 100])
    w32(ctx, recs + i * 0x128 + 4, k)
    want = rd32(ctx, 0x10145508 + k * 0xd6 + 0x10)
    return [obj, i, rg.choice([want, want, want, want + 1, 0, ctx.u32()])]


# ---- 0x1001xxxx -------------------------------------------------------------
def g_10804(ctx):
    rg = ctx.rng
    w32(ctx, 0x1013bb40, rg.choice([0, 1, 2]))
    return [rg.choice([0, 1, 2, 5, 40, rg.randrange(0, 100)])]


def g_13902(ctx):
    return [ctx.rng.getrandbits(32), ctx.rng.getrandbits(32), ctx.rng.getrandbits(32)]


def g_13e3c(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x400)
    if rg.random() < 0.5:
        w16(ctx, obj + 0x198, 0)
    return [obj, rg.getrandbits(32)]


def g_16a4a(ctx):
    return [ctx.rng.choice([0, 1, 2, 3, -1, ctx.rng.getrandbits(32), 0x10001, 0x100000002])]


# ---- 0x1002xxxx -------------------------------------------------------------
def g_27cce(ctx):
    rg = ctx.rng
    n = ctx.buf(0x20)
    others = [ctx.buf(0x20) for _ in range(3)]
    ptr = lambda: 0 if rg.random() < 0.25 else rg.choice(others)
    w16(ctx, n + 2, rg.choice([1, 1, 0, 2, 0xffff]))
    w32(ctx, n + 0xc, ptr())
    w32(ctx, n + 0x10, ptr())
    w32(ctx, 0x109f13f0, rg.choice([n, n, 0, others[0], rg.getrandbits(32)]))
    w32(ctx, 0x109f13f4, rg.choice([n, n, 0, others[1], rg.getrandbits(32)]))
    return [n]


def g_28a07(ctx):
    return [ctx.buf(0x60), ctx.rng.getrandbits(32)]


def g_28a37(ctx):
    rg = ctx.rng
    return [ctx.buf(0x120)] + [rg.getrandbits(32) for _ in range(4)]


# ---- 0x1003xxxx -------------------------------------------------------------
def g_35599(ctx):
    return [ctx.rng.getrandbits(32), ctx.rng.getrandbits(32)]


def g_38a9c(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x700)
    recs = ctx.buf(0x128 * 8)
    w32(ctx, obj + 0x84, recs)
    wr(ctx, obj + 0x5fb, '<I', rg.choice([0, 1, 3, 7, rg.randrange(0, 8)]))
    return [obj]


def g_3cf81(ctx):
    rg = ctx.rng
    keys = [rg.randrange(0, 11) for _ in range(rg.randrange(0, 9))]
    nodes = [ctx.buf(0x20) for _ in keys]
    for i, (n, k) in enumerate(zip(nodes, keys)):
        w32(ctx, n, k)
        w32(ctx, n + 8, rg.getrandbits(32))
        w32(ctx, n + 0x14, nodes[i + 1] if i + 1 < len(nodes) else 0)
    w32(ctx, 0x1015dc14, nodes[0] if nodes else 0)
    return []


# ---- 0x1004xxxx -------------------------------------------------------------
def g_48030(ctx):
    rg = ctx.rng
    rnd(ctx, 0x109b95c0 - 64, 0x60 * 8 + 128)
    w32(ctx, 0x1013b360, rg.choice([-1, -1, 0, 1, 5, 0x5f, 0x5f, 0x60, 0x61, -2, rg.randrange(0, 0x60), rg.getrandbits(32)]))
    return [rg.getrandbits(32), rg.getrandbits(32), rg.getrandbits(32)]


def g_480ab(ctx):
    rnd(ctx, 0x109b95c0, 0x60 * 8)
    w32(ctx, 0x1013b360, ctx.rng.choice([-1, 0, 7, 0x60]))
    return []


def g_480f2(ctx):
    return [ctx.buf(16)]


def g_48422(ctx):
    rg = ctx.rng
    for k in range(-3, 14):
        w32(ctx, 0x109b94e0 + k * 4, ctx.buf(0x20) if rg.random() < 0.9 else 0)
    return [rg.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 10, -1, -2, rg.randrange(-3, 13)])]


# ---- 0x1005xxxx -------------------------------------------------------------
def g_58549(ctx):
    rg = ctx.rng
    w32(ctx, 0x10143fbc, rg.getrandbits(32))
    return [rg.choice([-5, 0, 1, 2, 50, 127, 128, 129, 1000, 0x7fffffff, 0x80000000, rg.getrandbits(32), rg.randrange(-10, 200)])]


def g_585c4(ctx):
    rg = ctx.rng
    w32(ctx, 0x10143fd8, rg.getrandbits(32))
    return [rg.choice([-1, -1, 0, 5, rg.getrandbits(32)])]


def g_4args(ctx):
    return [ctx.rng.getrandbits(32) for _ in range(4)]


def g_1arg(ctx):
    return [ctx.rng.getrandbits(32)]


# ---- 0x1006xxxx -------------------------------------------------------------
def g_6ab0c(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x100)
    recs = ctx.buf(0x128 * 9)
    w32(ctx, obj + 0x84, recs)
    for i in range(9):
        k = rg.choice([-1, -3, 0x1c, 0x26, 0x27, 5, 0x1c, -1, 0x80000000, rg.randrange(0, 60)])
        if rg.random() < 0.3: k = -1
        w32(ctx, recs + i * 0x128 + 4, k)
    return [obj]


# ---- 0x1007xxxx -------------------------------------------------------------
def g_7b02d(ctx):
    rg = ctx.rng
    bank = rg.choice([0, 0, 1])
    w32(ctx, 0x1013bb40, bank)
    base = 0x10473800 + bank * 0x4fc3a
    n = rg.randrange(0, 5)
    w32(ctx, base, rg.choice([n, n, n, rg.randrange(0, 5)]))
    flag = ctx.buf(0x10)
    w16(ctx, flag + 0xa, 0) if rg.random() < 0.8 else None
    w32(ctx, 0x1017b8c4, flag)
    w32(ctx, 0x1015ec58, rg.choice([1, 1, 1, 0]))
    one = rg.randrange(0, 5)
    for i in range(5):
        rec = base + 0x3a + i * 0x13f
        sel = rg.random()
        w = 1 if i == one and sel < 0.7 else rg.choice([0, 0, 0, 1, 2, 255])
        ctx.m.uc.mem_write(rec + 0x74, bytes([w]))
        ctx.m.uc.mem_write(rec + 1, bytes([rg.choice([2, 2, 1, 0, 3])]))
    rows = []
    for r in range(8):
        for c in range(8):
            if rg.random() < 0.4:
                rows.append(rg.choice([0, 1, 100, 0xffff, rg.getrandbits(16)]))
            else:
                rows.append(rg.randrange(0, 30))
    if rg.random() < 0.3:
        # make ties likely
        rows = [rows[0]] * 64
    ctx.m.uc.mem_write(0x1016552c, struct.pack('<64H', *rows))
    w32(ctx, 0x101655ac, rg.getrandbits(32))
    return []


def g_7f0b4(ctx):
    w32(ctx, 0x1015e698, ctx.buf(16))
    w32(ctx, 0x1015e9a0, ctx.rng.getrandbits(32))
    return []


# ---- 0x1008xxxx -------------------------------------------------------------
def g_87b32(ctx):
    w32(ctx, 0x1017c8c4, ctx.rng.getrandbits(32))
    return []


def g_8988f(ctx):
    rg = ctx.rng
    n = rg.randrange(0, 6)
    nodes = [ctx.buf(0x40) for _ in range(n)]
    for i, nd in enumerate(nodes):
        w32(ctx, nd + 0x10, nodes[i + 1] if i + 1 < n else 0)
        r = rg.random()
        if r < 0.3:
            w32(ctx, nd + 0x28, 0)
        else:
            b = ctx.buf(8)
            w32(ctx, b, rg.choice([0, 0, 1, rg.getrandbits(32)]))
            w32(ctx, nd + 0x28, b)
    w32(ctx, 0x1017b998, nodes[0] if nodes else 0)
    w32(ctx, 0x1017c8c4, rg.choice([0, 0, 0, 1, 7]))
    for a in (0x1017c8e0, 0x1017c8d8, 0x1017c8e4):
        w32(ctx, a, rg.choice([0, 5, 0xffffffff, rg.getrandbits(32)]))
    return []


def g_8cc04(ctx):
    rg = ctx.rng
    w32(ctx, 0x109e3e14, rg.choice([639, 799, 0, 1, rg.randrange(0, 2000), rg.getrandbits(32)]))
    w32(ctx, 0x109e3de0, rg.choice([479, 599, 0, 1, rg.randrange(0, 2000), rg.getrandbits(32)]))
    def fx():
        r = rg.random()
        if r < 0.5: return rg.randrange(0, 0x30000)
        if r < 0.6: return rg.choice([0, 0x10000, 0x8000, 0x7fffffff, 0x80000000, 0xffffffff])
        return rg.getrandbits(32)
    rect = ctx.buf(16, struct.pack('<4I', *[fx() for _ in range(4)]))
    return [rg.getrandbits(32), rect, ctx.buf(16)]


# ---- 0x1009xxxx -------------------------------------------------------------
def g_98849(ctx):
    w32(ctx, 0x10143fd8, ctx.rng.getrandbits(32))
    return [ctx.rng.getrandbits(32)]


def g_9885e(ctx):
    rg = ctx.rng
    w32(ctx, 0x10143fd8, rg.getrandbits(32))
    w32(ctx, 0x1015d1b8, ctx.buf(0x40))
    return [rg.getrandbits(32), rg.choice([-1, -1, 0, 3, rg.getrandbits(32)])]


def g_9b2eb(ctx):
    w32(ctx, 0x1015d1ac, ctx.rng.choice([4, 4, 3, 0, 5, ctx.rng.getrandbits(32)]))
    return []


# ---- 0x100axxxx -------------------------------------------------------------
def g_aa7cd(ctx):
    rg = ctx.rng
    n = rg.randrange(1, 6)
    nodes = [ctx.buf(0x10) for _ in range(n)]
    for i, nd in enumerate(nodes):
        w32(ctx, nd, nodes[i + 1] if i + 1 < n else 0)
    return [nodes[0]]


def g_buf16(ctx):
    return [ctx.buf(16)]


# ---- 0x100bxxxx -------------------------------------------------------------
def tables(ctx):
    rg = ctx.rng
    count = rg.choice([0, 1, 3, 6, 8, -1, rg.randrange(0, 9)])
    w32(ctx, 0x1017d218, count)
    rnd(ctx, 0x101a2100, 9 * 0x88)
    for i in range(9):
        base = 0x101a2100 + i * 0x88
        w32(ctx, base + 0x18, rg.choice([0x400, 0, 0x401, 0xfffffbff, rg.getrandbits(32)]))
    return count


def g_b5386(ctx):
    tables(ctx)
    return [ctx.rng.choice([-1, 0, 1, 2, 3, 5, 6, 7, 8, 9, ctx.rng.randrange(-2, 11), 0x7fffffff, 0x80000000])]


def g_bd95a(ctx):
    rg = ctx.rng
    cap = rg.randrange(0, 9)
    cnt = rg.randrange(0, 9)
    arr = ctx.buf(4 * 12)
    ctx.m.uc.mem_write(arr, struct.pack('<12I', *[rg.choice([0, 1, 2, 3, 4, 5]) for _ in range(12)]))
    lst = ctx.buf(16)
    wr(ctx, lst, '<III', cap, cnt, arr)
    return [lst, rg.choice([0, 1, 2, 3, 4, 5, 6, 7])]


def g_bda4a(ctx):
    rg = ctx.rng
    if rg.random() < 0.04:
        return [0]
    cap = rg.randrange(0, 9)
    arr = ctx.buf(4 * 12)
    lst = ctx.buf(16)
    wr(ctx, lst, '<III', cap, rg.getrandbits(32), arr)
    return [lst]


# ---- 0x100cxxxx -------------------------------------------------------------
def g_c275b(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x600)
    other = ctx.buf(0x600)
    w32(ctx, obj + 0, rg.choice([1, 1, 1, 0xa, 0, 5, rg.getrandbits(32)]))
    cur = rg.randrange(0, 8)
    w32(ctx, 0x1017b8c8, cur)
    w32(ctx, obj + 4, cur if rg.random() < 0.2 else rg.choice([cur + 1, 3, 0xffffffff]))
    w32(ctx, obj + 0x28, other)
    w32(ctx, obj + 0x27c, rg.choice([0x10151b40, 0x10151b40, 0, rg.getrandbits(32)]))
    w16(ctx, other + 0x562, rg.getrandbits(16))
    wr(ctx, obj + 0x24a, '<I', rg.choice([0, 0, 1, 0x100, rg.getrandbits(32)]))
    w32(ctx, 0x1015dc78, rg.getrandbits(32))
    return [obj]


def world(ctx):
    """Objects behind the pointer table 0x109e8b40, the 0x50-stride table 0x109e9ba8 and the 0x88-stride table."""
    rg = ctx.rng
    count = tables(ctx)
    objs = []
    for i in range(8):
        o = ctx.buf(0x200)
        objs.append(o)
    for i in range(8):
        o = objs[i]
        w32(ctx, 0x109e8b40 + i * 4, o)
        w32(ctx, o + 0x48, rg.choice([0, objs[rg.randrange(0, 8)], objs[rg.randrange(0, 8)]]))
        w32(ctx, o + 0x8c, rg.getrandbits(32))
        w32(ctx, o + 0xf0, (rg.choice([0x200, 0x400, 0x400, 0x100, 0, 0xf00, 0x600]) | rg.choice([0, 1, 2, 3, 4, 5, 6, 7, 0x1f])))
        w32(ctx, 0x109e9ba8 + i * 0x50, rg.choice([-1, 0, 1, 2, 3, 5, 7, 8, rg.randrange(0, 9)]))
        w32(ctx, 0x101a2100 + i * 0x88 + 0x2c, rg.choice([0, objs[rg.randrange(0, 8)]]))
    w32(ctx, 0x1017b8c8, rg.randrange(0, 8))
    return objs


def g_c38fe(ctx):
    rg = ctx.rng
    world(ctx)
    sel = rg.choice([0x200, 0x400, 0x400, 0x100, 0, 0xf00, 0x600])
    return [(sel | rg.choice([0, 1, 2, 3, 4, 5, 6, 7, 0x1f, 0xff]) | rg.choice([0, 0, 0x8000, 0xf000])) | (rg.getrandbits(32) & 0xffff0000)]


def g_cba68(ctx):
    world(ctx)
    return []


# ---- 0x100dxxxx -------------------------------------------------------------
def g_d3861(ctx):
    return [ctx.rng.getrandbits(32)]


def objrec(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x520)
    recs = ctx.buf(0x128 * 8)
    w32(ctx, obj + 0x84, recs)
    for i in range(8):
        r = recs + i * 0x128
        wr(ctx, r, '<IIII', rg.choice([0xffffffff, 0, 1, 7]), rg.choice([0xffffffff, 0xfffffffe, 0, 1, 2, 3, 40, 100, 0x30]),
           rg.choice([0xffffffff, 0, 1, 2]), rg.choice([0, 1, 2, 3, 1, 2]))
    w32(ctx, obj + 0x4f8, rg.choice([0, 0xffffffff, 1, 2, 5, rg.getrandbits(32)]))
    w32(ctx, obj + 0x4fc, rg.choice([0, 1, 2, 3, 7, 0xffffffff]))
    return obj


def g_d60b6(ctx):
    rg = ctx.rng
    obj = objrec(ctx)
    for k in (0x30, 100, 40, 1, 2, 3):
        a = 0x10145508 + k * 0xd6 + 8
        w32(ctx, a, rg.getrandbits(32) | (4 if rg.random() < 0.7 else 0) & 0xffffffff)
    return [obj, rg.choice([0, 1, 2, 3, -1, 2, 1, rg.getrandbits(32)])]


def g_d6200(ctx):
    return [objrec(ctx)]


# ---- 0x101003a0 -------------------------------------------------------------
def g_rand(ctx):
    w32(ctx, 0x101596f8, ctx.rng.choice([0, 1, 0xffffffff, 0x7fffffff, ctx.rng.getrandbits(32)]))
    return []


# ---- 0x1010xxxx: C runtime number conversion --------------------------------
def g_flags(ctx):
    return [ctx.rng.getrandbits(32)]


def bitpos(ctx):
    rg = ctx.rng
    r = rg.random()
    if r < 0.75: return rg.randrange(0, 96)
    if r < 0.85: return rg.randrange(-40, 0)
    if r < 0.9: return rg.randrange(96, 130)
    return rg.choice([0, 1, 31, 32, 33, 63, 64, 95, 96])


def words3(ctx, base=0):
    rg = ctx.rng
    ws = []
    for _ in range(3):
        r = rg.random()
        ws.append(0 if r < 0.3 else 0xffffffff if r < 0.4 else 1 << rg.randrange(0, 32) if r < 0.6 else rg.getrandbits(32))
    return ctx.buf(64, struct.pack('<3I', *ws) + bytes(52))


def g_bits_n(ctx):
    a = words3(ctx)
    # the buffer is in the middle of argument memory: negative or large bit positions stay inside it
    ctx.m.uc.mem_write(a + 12, bytes(ctx.rng.getrandbits(8) for _ in range(40)))
    return [a, bitpos(ctx)]


def g_copy(ctx):
    return [ctx.buf(16), ctx.buf(16)]


def g_ptr3(ctx):
    return [words3(ctx)]


def g_a50(ctx):
    rg = ctx.rng
    a = words3(ctx)
    n = rg.choice([rg.randrange(0, 96), rg.randrange(0, 40), rg.randrange(-40, 0), rg.randrange(96, 200), 0, 32, 64])
    return [a, n]


def ld12(ctx, near=0x3fff, spread=0x1000, topbit=0.9):
    rg = ctx.rng
    r = rg.random()
    if r < 0.05: e = 0
    elif r < 0.1: e = 0x7fff
    elif r < 0.9: e = max(0, min(0x7ffe, near + rg.randrange(-spread, spread)))
    else: e = rg.getrandbits(15)
    r = rg.random()
    mant = rg.getrandbits(64)
    if rg.random() < topbit: mant |= 1 << 63
    if r < 0.1: mant = 0
    elif r < 0.2: mant = (1 << 63)
    elif r < 0.3: mant = (1 << 64) - 1
    elif r < 0.35: mant = 0xc000000000000000
    word = e | rg.choice([0, 0x8000])
    guard = rg.choice([0, 0x8000, 0x7fff, 0xffff, rg.getrandbits(16)])
    return ctx.buf(16, struct.pack('<HQH', guard, mant, word) + bytes(4))


def fmt_ptr(ctx):
    rg = ctx.rng
    r = rg.random()
    if r < 0.4: return 0x1015bfe8
    if r < 0.8: return 0x1015c000
    vals = [rg.choice([1024, 128, 16, 200, 100]), rg.choice([-1023, -127, -15, -100]), rg.choice([53, 24, 11, 8]),
            rg.choice([11, 8, 5]), rg.choice([0x40, 0x20, 0x10]), rg.choice([1023, 127, 15])]
    return ctx.buf(24, struct.pack('<6i', *vals))


def ext_for(ctx, fmt):
    """An extended value whose exponent is near the range of the format."""
    rg = ctx.rng
    lo, hi = {0x1015bfe8: (-1023, 1024), 0x1015c000: (-127, 128)}.get(fmt, (-120, 130))
    e = rg.choice([lo - 30, lo - 10, lo - 1, lo, lo + 1, hi - 1, hi, hi + 1, rg.randrange(lo, hi), rg.randrange(lo, hi), rg.randrange(lo - 60, hi + 60), 0, -0x3fff])
    a = ld12(ctx, spread=1)
    expw = ((e + 0x3fff) & 0x7fff) | rg.choice([0, 0x8000])
    mant = rg.getrandbits(64) | (1 << 63)
    if rg.random() < 0.3: mant &= ~((1 << rg.randrange(1, 64)) - 1)
    if rg.random() < 0.2: mant = (1 << 64) - 1 - rg.randrange(0, 3)
    if e == -0x3fff and rg.random() < 0.3: mant = 0
    ctx.m.uc.mem_write(a, struct.pack('<HQH', rg.getrandbits(16), mant, expw))
    return a


def g_b10(ctx):
    f = fmt_ptr(ctx)
    return [ext_for(ctx, f), ctx.buf(16), f]


def g_ce0(ctx):
    return [ext_for(ctx, ctx.rng.choice([0x1015bfe8, 0x1015c000])), ctx.buf(16)]


def g_ce0b(ctx):
    return [ext_for(ctx, 0x1015bfe8), ctx.buf(16)]


def g_ce0f(ctx):
    return [ext_for(ctx, 0x1015c000), ctx.buf(16)]


def dbl(ctx):
    rg = ctx.rng
    r = rg.random()
    if r < 0.05: e = 0
    elif r < 0.1: e = 0x7ff
    elif r < 0.8: e = 1023 + rg.randrange(-60, 60)
    else: e = rg.randrange(0, 0x800)
    mant = rg.getrandbits(52)
    q = rg.random()
    if q < 0.1: mant = 0
    elif q < 0.15: mant = 1
    elif q < 0.2: mant = (1 << 52) - 1
    elif q < 0.25: mant = 1 << 51
    return (rg.choice([0, 1]) << 63) | (e << 52) | mant


def g_f80(ctx):
    d = dbl(ctx)
    return [ctx.buf(16), ctx.buf(8, struct.pack('<Q', d))]


def g_f00(ctx):
    d = dbl(ctx)
    return [d & 0xffffffff, d >> 32]


def g_b5c0(ctx):
    rg = ctx.rng
    v = lambda: rg.choice([0, 1, 0xffffffff, 0x80000000, 0x7fffffff, rg.getrandbits(32), rg.getrandbits(32)])
    return [v(), v(), ctx.buf(8)]


def w12(ctx):
    rg = ctx.rng
    ws = [rg.choice([0, 1, 0xffffffff, 0x80000000, rg.getrandbits(32), rg.getrandbits(32)]) for _ in range(3)]
    return ctx.buf(16, struct.pack('<3I', *ws) + bytes(4))


def g_b5f0(ctx):
    return [w12(ctx), w12(ctx)]


def g_w12(ctx):
    return [w12(ctx)]


def g_b6c0(ctx):
    rg = ctx.rng
    n = rg.randrange(1, 25)
    ds = [rg.randrange(0, 10) for _ in range(n)]
    if all(d == 0 for d in ds): ds[rg.randrange(0, n)] = rg.randrange(1, 10)
    if rg.random() < 0.15:
        ds = [rg.choice([0, 9, 0xff, 0x80, 0x7f, rg.getrandbits(8)]) for _ in range(n)]
        ds[0] = 5
    return [ctx.buf(32, bytes(ds) + bytes(32 - n)), n, ctx.buf(16)]


def g_ca40(ctx):
    rg = ctx.rng
    r = rg.random()
    if r < 0.15:
        # exponents summing close to the underflow and overflow limits
        t = rg.choice([0x3fbf, 0x3fc0, 0x3fc5, 0x4000, 0xbffd, 0xbffe, 0xbffc, 0x7ffe])
        ea = rg.randrange(max(1, t - 0x7ffe), min(0x7ffe, t - 1) + 1)
        a = ld12(ctx, near=ea, spread=1)
        b = ld12(ctx, near=t - ea, spread=1)
        return [a, b]
    return [ld12(ctx), ld12(ctx)]


def g_cd00(ctx):
    rg = ctx.rng
    n = rg.choice([rg.randrange(-40, 40), rg.randrange(-400, 400), rg.randrange(-5000, 5000), 0, 1, -1, 7, 8, 9, 16])
    return [ld12(ctx), n, rg.choice([0, 1])]


def g_bfa0(ctx):
    rg = ctx.rng
    r = rg.random()
    hi = rg.getrandbits(32) | 0x80000000
    lo = rg.getrandbits(32)
    e = 0x3fff + rg.randrange(-0x2000, 0x2000) if r < 0.6 else 0x3fff + rg.randrange(-80, 80)
    q = rg.random()
    if q < 0.06: e, hi, lo = 0, 0, 0
    elif q < 0.12: e = 0x7fff; hi, lo = rg.choice([(0x80000000, 0), (0xc0000000, 0), (0x80000001, 0), (0xc0000000, 5), (0xe0000000, 1), (0x80000000, 7), (0xa0000000, 0)])
    elif q < 0.15: e = rg.choice([0, 1, 0x7ffe])
    elif q < 0.2: hi, lo = rg.choice([(0x80000000, 0), (0xffffffff, 0xffffffff), (0xfffffffe, 0)])
    if rg.random() < 0.08: hi = rg.getrandbits(32)
    expw = e | rg.choice([0, 0x8000]) | (rg.getrandbits(16) << 16 if rg.random() < 0.3 else 0)
    nd = rg.choice([17, 17, 15, 6, 0, 1, 2, 10, 20, 21, 22, 30, -1, -3, rg.randrange(-2, 25)])
    return [lo, hi, expw, nd, rg.choice([0, 1, 0x100, 0xff]), ctx.buf(64)]


def g_setmode(ctx):
    rg = ctx.rng
    w32(ctx, 0x10ac385c, rg.choice([64, 70, 32, 0, 100]))
    for k in range(3):
        g = ctx.buf(32 * 8)
        w32(ctx, 0x10ac3720 + 4 * k, g)
    return [rg.choice([0, 1, 2, 31, 32, 33, 63, 64, 69, 70, 99, rg.randrange(0, 100), rg.getrandbits(32)]),
            rg.choice([0x8000, 0x4000, 0x8000, 0x4000, 0, 0x2000, rg.getrandbits(16)])]


def S(name, addr, gen, void=False, trials=400):
    return dict(name=name, orig=addr, c=name, gen=gen, void=void, trials=trials)


SPECS = [
    S('Sim_10006db8', 0x10006db8, g_6db8),
    S('Sim_100093be', 0x100093be, g_93be, void=True),
    S('Sim_100095b6', 0x100095b6, g_95b6, void=True),
    S('Sim_1000c6e3', 0x1000c6e3, g_c6e3),
    S('Sim_10010804', 0x10010804, g_10804, void=True),
    S('Sim_10013902', 0x10013902, g_13902),
    S('Sim_10013e3c', 0x10013e3c, g_13e3c),
    S('Sim_10016a4a', 0x10016a4a, g_16a4a),
    S('Sim_1001f4e4', 0x1001f4e4, none),
    S('Sim_10027cce', 0x10027cce, g_27cce, void=True),
    S('Sim_10027f2e', 0x10027f2e, none, void=True),
    S('Sim_10028a07', 0x10028a07, g_28a07, void=True),
    S('Sim_10028a37', 0x10028a37, g_28a37, void=True),
    S('Sim_10035599', 0x10035599, g_35599, void=True),
    S('Sim_100386d0', 0x100386d0, none, void=True),
    S('Sim_10038a9c', 0x10038a9c, g_38a9c, void=True),
    S('Sim_1003cf81', 0x1003cf81, g_3cf81, void=True),
    S('Sim_10048030', 0x10048030, g_48030),
    S('Sim_100480ab', 0x100480ab, g_480ab, void=True),
    S('Sim_100480f2', 0x100480f2, g_480f2, void=True),
    S('Sim_10048422', 0x10048422, g_48422, void=True),
    S('Sim_100570e7', 0x100570e7, none),
    S('Sim_10057a51', 0x10057a51, none),
    S('Sim_10058549', 0x10058549, g_58549, void=True),
    S('Sim_100585c4', 0x100585c4, g_585c4),
    S('Sim_10058716', 0x10058716, g_4args, void=True),
    S('Sim_1005873d', 0x1005873d, g_1arg, void=True),
    S('Sim_1006ab0c', 0x1006ab0c, g_6ab0c, void=True, trials=600),
    S('Sim_1007b02d', 0x1007b02d, g_7b02d, void=True, trials=1500),
    S('Sim_1007cbb9', 0x1007cbb9, none),
    S('Sim_1007f0b4', 0x1007f0b4, g_7f0b4),
    S('Sim_10087b32', 0x10087b32, g_87b32, void=True),
    S('Sim_1008988f', 0x1008988f, g_8988f, void=True),
    S('Sim_1008cc04', 0x1008cc04, g_8cc04, trials=800),
    S('Sim_10098849', 0x10098849, g_98849),
    S('Sim_1009885e', 0x1009885e, g_9885e),
    S('Sim_1009b2eb', 0x1009b2eb, g_9b2eb),
    S('Sim_100aa7cd', 0x100aa7cd, g_aa7cd),
    S('Sim_100aa7e7', 0x100aa7e7, g_buf16),
    S('Sim_100aa7fc', 0x100aa7fc, g_buf16),
    S('Sim_100b3f8f', 0x100b3f8f, none, void=True),
    S('Sim_100b5386', 0x100b5386, g_b5386),
    S('Sim_100b54b3', 0x100b54b3, g_b5386),
    S('Sim_100b8d5c', 0x100b8d5c, none),
    S('Array_RemoveValue', 0x100bd95a, g_bd95a, trials=800),
    S('Array_Clear', 0x100bda4a, g_bda4a, void=True),
    S('Sim_100c275b', 0x100c275b, g_c275b, void=True, trials=800),
    S('Sim_100c38fe', 0x100c38fe, g_c38fe, trials=800),
    S('Sim_100cba42', 0x100cba42, g_cba68),
    S('Sim_100cba68', 0x100cba68, g_cba68),
    S('Sim_100d3861', 0x100d3861, g_d3861, void=True),
    S('Sim_100d60b6', 0x100d60b6, g_d60b6, trials=800),
    S('Sim_100d6200', 0x100d6200, g_d6200),

    S('Crt_Rand', 0x101003a0, g_rand),
    S('Sim_10108840', 0x10108840, g_flags),
    S('Sim_10108880', 0x10108880, g_bits_n, trials=800),
    S('Sim_101088f0', 0x101088f0, g_bits_n, trials=800),
    S('Sim_10108960', 0x10108960, g_bits_n, trials=1000),
    S('Sim_10108a00', 0x10108a00, g_copy, void=True),
    S('Sim_10108a20', 0x10108a20, g_ptr3, void=True),
    S('Sim_10108a30', 0x10108a30, g_ptr3),
    S('Sim_10108a50', 0x10108a50, g_a50, void=True, trials=800),
    S('Sim_10108b10', 0x10108b10, g_b10, trials=2000),
    S('Sim_10108ce0', 0x10108ce0, g_ce0b, trials=600),
    S('Sim_10108d00', 0x10108d00, g_ce0f, trials=600),
    S('Sim_10108f00', 0x10108f00, g_f00, trials=1000),
    S('Sim_10108f80', 0x10108f80, g_f80, void=True, trials=800),
    S('Sim_1010b5c0', 0x1010b5c0, g_b5c0),
    S('Sim_1010b5f0', 0x1010b5f0, g_b5f0),
    S('Sim_1010b660', 0x1010b660, g_w12, void=True),
    S('Sim_1010b690', 0x1010b690, g_w12, void=True),
    S('Sim_1010b6c0', 0x1010b6c0, g_b6c0, void=True, trials=600),
    S('Sim_1010bfa0', 0x1010bfa0, g_bfa0, trials=2000),
    S('Sim_1010ca40', 0x1010ca40, g_ca40, void=True, trials=2000),
    S('Sim_1010cd00', 0x1010cd00, g_cd00, void=True, trials=1000),
    S('Sim_1010cd90', 0x1010cd90, g_setmode, trials=600),
]
