# Differential tests for the wave 2 functions of letter i (see difftest.py).
import struct

SOURCES = [
    'sim/unsorted_1000/w2i_misc.c',
    'sim/unsorted_1001/w2i_misc.c', 'sim/unsorted_1004/tables.c',
    'sim/unsorted_1002/w2i_state.c', 'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1002/w2i_setup.c',
    'sim/unsorted_1003/w2i_misc.c',
    'sim/unsorted_1004/w2i_misc.c', 'sim/unsorted_1004/w2i_pack.c',
    'sim/unsorted_1005/layout.c', 'sim/unsorted_1005/w2i_layout.c', 'sim/view/viewrect.c',
    'sim/unsorted_1007/w2i_map.c',
    'sim/unsorted_1008/w2i_rect.c',
    'sim/unsorted_1009/w2i_state.c',
    'sim/unsorted_100a/w2i_misc.c',
    'sim/unsorted_100b/w2i_misc.c',
    'sim/unsorted_100c/w2i_misc.c',
    'sim/unsorted_100d/w2i_chain.c', 'sim/unsorted_100d/w2i_table.c', 'sim/unsorted_100d/w2i_select.c',
    'sim/unsorted_1010/w2i_crt.c',
]


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def w16(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<H', v & 0xffff))


def w8(ctx, addr, v):
    ctx.m.uc.mem_write(addr, bytes([v & 0xff]))


def wr(ctx, addr, fmt, *vals):
    ctx.m.uc.mem_write(addr, struct.pack(fmt, *[v & 0xffffffff if fmt[-1] == 'I' else v for v in vals]))


def rnd(ctx, addr, n):
    """Random bytes into the DLL's data."""
    ctx.m.uc.mem_write(addr, bytes(ctx.rng.getrandbits(8) for _ in range(n)))


def sm(ctx, lo=-20, hi=20):
    return ctx.rng.randrange(lo, hi) & 0xffffffff


def none(ctx):
    return []


# ---- 0x1000 .. 0x1001 ----------------------------------------------------------
def g_obj_short(ctx):
    return [ctx.buf(0x40), ctx.rng.choice([ctx.u32(), ctx.rng.getrandbits(16), 0xffff, 0x8000])]


def g_9558(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x40)
    blk = ctx.buf(0x58 * 10 + 0x40)
    w32(ctx, obj + 0x30, 0 if rg.random() < 0.15 else blk)
    outs = [0 if rg.random() < 0.25 else ctx.buf(4) for _ in range(3)]
    if rg.random() < 0.1:
        outs[1] = outs[0]
    idx = rg.choice([0, 1, 2, 5, 9, rg.randrange(0, 10), -1, -2, rg.randrange(-3, 3)])
    return [obj, idx & 0xffffffff] + outs


def g_3u32(ctx):
    return [ctx.u32(), ctx.u32(), ctx.u32()]


def g_u32(ctx):
    return [ctx.u32()]


def g_15cf8(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x200)
    if rg.random() < 0.5:
        w16(ctx, obj + 0x1a0, 0)
    return [obj, ctx.u32(), ctx.u32(), ctx.u32()]


def g_15c0e(ctx):
    rg = ctx.rng
    objs = []
    for i in range(8):
        o = ctx.buf(0x200)
        if rg.random() < 0.5:
            w16(ctx, o + 0x1a0, 0)
        w32(ctx, 0x109e8b40 + 4 * i, o)
        objs.append(o)
    for i in range(16):
        v = rg.choice([0xffffffff, 0xffff, 0x1ffff, rg.randrange(0, 8), rg.randrange(0, 8), rg.randrange(0, 8), rg.getrandbits(32) & 0x7])
        w32(ctx, 0x109b7d40 + i * 0x6c, v)
    obj = ctx.buf(0x200)
    w32(ctx, obj + 8, rg.choice([rg.randrange(0, 16), rg.randrange(0, 16), 16, 17, 100]))
    w16(ctx, obj + 0xf0, ctx.u32())
    rec = ctx.buf(0x10)
    return [obj, rec]


def g_19d8c(ctx):
    w32(ctx, 0x1015d184, ctx.u32())
    return []


def sorted_table(ctx, count, lo, hi, stride=0x39):
    rg = ctx.rng
    keys = sorted(rg.randrange(lo, hi + 1) for _ in range(count))
    tab = ctx.buf(4 + stride * max(count, 1) + 0x20)
    w32(ctx, tab, count & 0xffffffff)
    for i, k in enumerate(keys):
        w32(ctx, tab + 4 + i * stride, k)
    return tab, keys


def g_1c075(ctx):
    rg = ctx.rng
    b = rg.choice([1, 2, 3, 7, 10, 25, 100, -7, -1000, rg.randrange(1, 60), rg.randrange(1, 60), rg.randrange(1, 60), 0])
    mod = max(abs(b), 1)
    count = rg.choice([0, 1, 2, 3, 5, 8, 12, rg.randrange(0, 12), -1])
    tab, keys = sorted_table(ctx, max(count, 0), -(mod - 1), mod - 1)
    if count < 0:
        w32(ctx, tab, count)
    if rg.random() < 0.5 and keys:
        a = rg.choice(keys) + mod * rg.randrange(-3, 4)
    else:
        a = rg.choice([ctx.u32(), rg.randrange(-200, 200), rg.getrandbits(32)])
    return [a & 0xffffffff, b & 0xffffffff, 0 if rg.random() < 0.06 else tab]


def g_1db13(ctx):
    rg = ctx.rng
    count = rg.choice([0, 1, 2, 3, 5, 8, rg.randrange(1, 9), rg.randrange(1, 9), -1])
    n = max(count, 0)
    tab = ctx.buf(4 + 0x39 * max(n, 1) + 0x20)
    w32(ctx, tab, count)
    ok = rg.randrange(0, max(n, 1))
    for i in range(n):
        v = rg.choice([0, 1, 2, 5, 0xffffffff, rg.getrandbits(32), 3])
        if i == ok:
            v = rg.choice([0, 1])
        w32(ctx, tab + 4 + i * 0x39 + 0xd, v)
    idx = rg.choice([-2, -1, 0, 1, count - 1, count, count + 1, rg.randrange(-1, max(count, 1)), rg.randrange(-1, max(count, 1))])
    return [0 if rg.random() < 0.06 else tab, idx & 0xffffffff]


# ---- 0x1002 ----------------------------------------------------------------
def entries_1096(ctx, n=6):
    for i in range(n):
        rnd(ctx, 0x1096fcc0 + i * 0x156, 0x156)
        for off in (8, 0x20):
            w32(ctx, 0x1096fcc0 + i * 0x156 + off, ctx.rng.choice([0, 0, 1, 5, ctx.rng.getrandbits(32)]))


def g_258d7(ctx):
    rg = ctx.rng
    entries_1096(ctx)
    w32(ctx, 0x10134890, rg.choice([0xffffffff, 0, 1, 2, 3, 4, 5, rg.randrange(0, 6), rg.randrange(0, 6)]))
    return []


def g_2590c(ctx):
    rg = ctx.rng
    entries_1096(ctx)
    cur = rg.choice([0xffffffff, 0, 1, 2, 3, 4, 5, rg.randrange(0, 6), rg.randrange(0, 6), rg.randrange(0, 6)])
    w32(ctx, 0x1013488c, cur)
    glob = rg.choice([0, 1, 7, rg.getrandbits(32)])
    w32(ctx, 0x1017b8c8, glob)
    w32(ctx, 0x10134890, rg.choice([0xffffffff, 3, rg.getrandbits(32)]))
    if 0 <= cur < 6 and rg.random() < 0.7:
        w32(ctx, 0x1096fcc0 + cur * 0x156 + 0x10, glob)
        if rg.random() < 0.8:
            w32(ctx, 0x1096fcc0 + cur * 0x156 + 8, rg.choice([1, 2, 0xff]))
    return []


def g_28a74(ctx):
    return [ctx.buf(0x20), ctx.u32()]


def g_2990f(ctx):
    rg = ctx.rng
    rnd(ctx, 0x109eeb00, 0x30 * 12)
    return [rg.choice([0, 1, 2, 5, 9, rg.randrange(0, 12), rg.randrange(0, 12)])]


def g_2ba28(ctx):
    rg = ctx.rng
    which = rg.choice([0, 1, 0, 1, 2, 0xffffffff, ctx.u32() & 0xff])
    preset = rg.choice([0, 1, 2, 3, rg.randrange(0, 4)])
    w32(ctx, 0x101c64f0, preset)
    nsets = 8
    rnd(ctx, 0x101355c0, 0x258 * nsets)
    rnd(ctx, 0x10135f20, 0xf0 * nsets)
    rnd(ctx, 0x101362e0, 0x770 * nsets)
    rnd(ctx, 0x10151d58, 0x20)
    rnd(ctx, 0x109e8000, 0x700)
    w32(ctx, 0x1014cb38, rg.choice([0, 0, 1, 0xb00, rg.getrandbits(32)]))
    w32(ctx, 0x1015e53c, 0)
    return [which]


def g_2d263(ctx):
    return [ctx.u32(), ctx.u32()]


# ---- 0x1003 ----------------------------------------------------------------
def g_33555(ctx):
    return [0 if ctx.rng.random() < 0.3 else ctx.buf(16)]


def g_35444(ctx):
    rg = ctx.rng
    w32(ctx, 0x1015e53c, rg.getrandbits(32))
    return [rg.choice([-1, 0, 1, 5, 9, 10, 11, 12, rg.randrange(-3, 15), ctx.u32()]) & 0xffffffff]


def g_3a53b(ctx):
    w32(ctx, 0x1015d0a4, ctx.buf(0x40))
    return []


def slot_table(ctx, count, n=12):
    rnd(ctx, 0x109b9e20, 0x7e * n)
    w32(ctx, 0x1015ea80, count)
    for i in range(n):
        w32(ctx, 0x109b9e20 + i * 0x7e + 4, ctx.rng.choice([0, 1, 2, 5, 10, 20, ctx.rng.randrange(0, 30)]))


def g_3b471(ctx):
    rg = ctx.rng
    slot_table(ctx, rg.choice([0, 3, 8, 12]))
    return [rg.choice([0, 1, 5, 11, rg.randrange(0, 12), -1, -2]) & 0xffffffff, ctx.u32()]


def g_3b4d0(ctx):
    rg = ctx.rng
    count = rg.choice([0, 1, 5, 8, 12, rg.randrange(-1, 12)])
    slot_table(ctx, count & 0xffffffff)
    return [rg.choice([-2, -1, 0, 1, count - 1, count, count + 1, rg.randrange(0, 12), rg.randrange(0, 12)]) & 0xffffffff]


# ---- 0x1004 ----------------------------------------------------------------
def table_1005(ctx):
    rnd(ctx, 0x10145508, 0xd6 * 6)
    for i in range(6):
        w32(ctx, 0x10145508 + i * 0xd6 + 8, ctx.rng.choice([0, 4, 5, 0xf, ctx.rng.getrandbits(32)]))


def make_block(ctx, count=None, current=None, kmax=8):
    """A block as the record selection functions use it (0x4f8 count, 0x4fc selection, 0x84 -> 8 records)."""
    rg = ctx.rng
    blk = ctx.buf(0x640)
    recs = ctx.buf(0x128 * 12)
    owner = ctx.buf(0x100)
    table_1005(ctx)
    w32(ctx, blk, owner)
    w32(ctx, blk + 0x84, recs)
    w32(ctx, blk + 0x4f8, rg.choice([1, 3, 8, 8, 8, 0, 0xffffffff]) if count is None else count)
    w32(ctx, blk + 0x4fc, rg.randrange(0, 8) if current is None else current)
    w16(ctx, blk + 0x560, rg.getrandbits(16))
    glob = rg.choice([0, 1, 2, 3, 5])
    w32(ctx, 0x1017b8c8, glob)
    w32(ctx, owner + 4, rg.choice([glob, glob, rg.getrandbits(8)]))
    w32(ctx, owner + 0xf0, rg.getrandbits(32))
    w32(ctx, 0x10179ccc, rg.choice([0, 1, 1]))
    w32(ctx, 0x1015ec58, rg.choice([0, 1]))
    for i in range(8):
        w32(ctx, 0x101853c0 + 4 * i, rg.choice([0, 1, 1, 7]))
        r = recs + i * 0x128
        rnd_rec = [rg.choice([0xffffffff, 0, 5]),
                   rg.choice([0xffffffff, 0, 1, 2, 3, 4, 5, rg.randrange(0, 6)]),
                   rg.choice([0xffffffff, 0, 1, 2, 3]),
                   rg.randrange(0, kmax) if rg.random() < 0.85 else rg.randrange(0, 4)]
        for off, v in zip((0, 4, 8, 0xc), rnd_rec):
            w32(ctx, r + off, v)
    # make several records share the slot number so that matches happen
    shared = rg.randrange(0, 8)
    for i in range(8):
        if rg.random() < 0.4:
            w32(ctx, recs + i * 0x128 + 0xc, shared)
    return blk


def g_4044c(ctx):
    rg = ctx.rng
    blk = make_block(ctx)
    p = ctx.buf(16)
    idx = rg.randrange(0, 6)
    ent = ctx.buf(0x40)
    for i in range(10):
        e = ctx.buf(0x40)
        w32(ctx, e + 0x14, rg.choice([3, 3, 0, 1]))
        w32(ctx, e + 0xc, rg.choice([rg.getrandbits(3), 3, 0]))
        w32(ctx, e + 0x28, blk)
        w32(ctx, 0x109e8b40 + 4 * i, 0 if rg.random() < 0.1 else e)
    w8(ctx, p + 2, idx)
    w16(ctx, p + 3, rg.choice([rg.getrandbits(16), rg.getrandbits(8), 0, 0xff, 1, 0x8001]))
    w32(ctx, 0x1015dc64, rg.choice([0, 3, 6, 7, 10, 256, 0xffffffff, rg.randrange(0, 12)]))
    return [p]


def g_40645(ctx):
    return [ctx.buf(0x40), ctx.buf(0x260)]


def g_40f2d(ctx):
    w32(ctx, 0x1015d004, ctx.buf(0x200))
    return [ctx.buf(16)]


def g_4f0a6(ctx):
    rnd(ctx, 0x109b8b60, 0x80)
    return []


def nib(ctx, base, span=16):
    rg = ctx.rng
    return rg.choice([base + rg.randrange(0, span), base + rg.randrange(0, span), base, base + 15, ctx.u32(), rg.randrange(-3, 40)]) & 0xffffffff


def g_pack4(ctx):
    return [nib(ctx, 6), nib(ctx, 9), nib(ctx, 0xb), ctx.rng.choice([0, 1, 2, ctx.u32(), 0x100, 0x10000])]


def g_pack5a(ctx):
    return [nib(ctx, 0xe), nib(ctx, 0xe), nib(ctx, 0xe), nib(ctx, 0x16), nib(ctx, 0x13)]


def g_pack5b(ctx):
    return [nib(ctx, 0x18), nib(ctx, 0x18), nib(ctx, 0x18), nib(ctx, 0x1d), nib(ctx, 0x20)]


def g_flags11(ctx):
    rg = ctx.rng
    return [rg.choice([0, 0, 1, 2, ctx.u32(), 0x100, 0x10000, 0xffffffff]) for _ in range(11)]


def g_2u32(ctx):
    return [ctx.u32(), ctx.u32()]


def g_bac0(ctx):
    rg = ctx.rng
    rnd(ctx, 0x1011168c, 0xaa * 8)
    for i in range(8):
        w32(ctx, 0x1011168c + i * 0xaa, rg.choice([0, 1, 2, 3, 4, 5, 6, 8, 10, 11, 0xffffffff, 0x80000000, rg.randrange(-3, 14) & 0xffffffff]))
    return [rg.choice([0, 1, 2, 3, 4, 5, 6, 7, rg.randrange(0, 8)])]


def g_d285(ctx):
    rg = ctx.rng
    rnd(ctx, 0x101116c6, 0xaa * 8)
    for i in range(8):
        w32(ctx, 0x101116c6 + i * 0xaa, rg.choice([rg.randrange(0xd, 0x1d), rg.randrange(0xd, 0x1d), 0, 0xf, 0x1a, 0x1b, 0xffffffff, rg.getrandbits(32)]))
    return [rg.choice([0, 1, 2, 3, 4, 5, 6, 7, rg.randrange(0, 8)])]


# ---- 0x1005 ----------------------------------------------------------------
def fix(ctx):
    r = ctx.rng.random()
    if r < 0.4: return ctx.rng.randrange(0, 0x30000)
    if r < 0.5: return ctx.rng.choice([0, 0x10000, 0x8000, 0x7fffffff, 0x80000000, 0xffffffff])
    return ctx.rng.getrandbits(32)


def layout_obj(ctx):
    rg = ctx.rng
    o = ctx.buf(0x200, 'rand')
    area = ctx.buf(20, struct.pack('<5I', rg.getrandbits(32), *[fix(ctx) for _ in range(4)]))
    w32(ctx, o + 0xec, area)
    ctx.m.uc.mem_write(o + 0x100, struct.pack('<hh', rg.randrange(-100, 1200), rg.randrange(-100, 1200)))
    return o


def layout_sel(ctx):
    rg = ctx.rng
    ents = [ctx.buf(0x40, 'rand') for _ in range(3)]
    for e in ents:
        other = ctx.buf(0x630, 'rand')
        if rg.random() < 0.5:
            w32(ctx, other + 0x613, 0)
        w32(ctx, e + 0x28, other)
    idx = rg.randrange(0, 256)
    w32(ctx, 0x1017b8c8, idx)
    w32(ctx, 0x109e8b40 + idx * 4, rg.choice(ents))


def g_50671(ctx):
    rnd(ctx, 0x109e8780, 0x30)
    rnd(ctx, 0x10151d68, 0x60)
    return [layout_obj(ctx)]


def g_50c5b(ctx):
    rg = ctx.rng
    layout_sel(ctx)
    rnd(ctx, 0x109e8780, 0x30)
    rnd(ctx, 0x10151d68, 0x60)
    shared = layout_obj(ctx) if rg.random() < 0.15 else None
    for a in (0x109e8940, 0x109e8918, 0x109e8928, 0x109e88e8, 0x109e890c, 0x109e8938, 0x109e894c, 0x109e8950, 0x109e8954):
        w32(ctx, a, shared if (shared and rg.random() < 0.5) else layout_obj(ctx))
    return []


# ---- 0x1007 ----------------------------------------------------------------
def g_765a7(ctx):
    rg = ctx.rng
    stack = ctx.buf(0x20)
    arr = ctx.buf(0x40)
    w32(ctx, stack + 8, arr)
    w32(ctx, stack + 0xc, rg.choice([0, 1, 1, 2, 5, 8, 0xffffffff, rg.randrange(0, 9)]))
    w32(ctx, 0x1017e5ec, rg.choice([0, 1, 5, rg.getrandbits(32)]))
    return [stack]


MAP = 0x1047383a


def map_row(a, b):
    return MAP + a * 0x4fc3a + b * 0x13f


def fill_maps(ctx, nmaps=3, nrows=4):
    """Random bytes in the rows used, with small values so that 5 and 6 turn up."""
    rg = ctx.rng
    for a in range(nmaps):
        for b in range(nrows):
            ctx.m.uc.mem_write(map_row(a, b), bytes(rg.choice([0, 5, 6, 5, 6, 7, 1, 0]) for _ in range(0x13f)))


def cells(ctx, a, b, n=8):
    rg = ctx.rng
    row = map_row(a, b)
    for i in range(n):
        base = row + 7 + i * 12
        w32(ctx, base, rg.choice([1, 2, 3, 1, 2, 3, 0, 4]))
        w32(ctx, base + 4, rg.randrange(0, 4))
        w32(ctx, base + 8, rg.choice([rg.randrange(0, 3), rg.getrandbits(32)]))


def g_7b42c(ctx):
    rg = ctx.rng
    fill_maps(ctx)
    a, b = rg.randrange(0, 3), rg.randrange(0, 4)
    cells(ctx, a, b)
    return [a, b, rg.choice([0, 1, 2, 3, 4, 5, 6, 7, rg.randrange(0, 8)])]


def g_7b4d6(ctx):
    rg = ctx.rng
    fill_maps(ctx)
    a, b = rg.randrange(0, 3), rg.randrange(0, 4)
    cells(ctx, a, b)
    row = map_row(a, b)
    w8(ctx, row, rg.choice([0, 1, 5, 6, 2, 3]))
    w32(ctx, row + 2, rg.choice([0, 0x10, 0x8000, 0x8001, 0x8002, 0x8003, 5, rg.getrandbits(32)]))
    w8(ctx, row + 6, rg.choice([0, 1, 1, 0xff]))
    n = rg.randrange(0, 9)
    for i in range(n, 8):  # terminate the cell list early sometimes
        if rg.random() < 0.7:
            w32(ctx, row + 7 + i * 12, 0)
    return [a, b]


def g_7caab(ctx):
    rg = ctx.rng
    a = rg.randrange(0, 3)
    fill_maps(ctx)
    base = 0x10473800 + a * 0x4fc3a
    n = rg.choice([0, 1, 2, 3, 4, 0xffffffff])
    w32(ctx, base, n)
    for i in range(4):
        cell = base + 0x3a + i * 0x13f
        rnd(ctx, cell, 0xb0)
        w32(ctx, cell + 2, rg.choice([0x40000, 0x40000, 0x40000, 0, rg.getrandbits(32)]))
        w8(ctx, cell, rg.choice([5, 5, 5, 3, 0, 6]))
        w16(ctx, cell + 0xa9, rg.choice([0, 1, 2, 0xffff, rg.randrange(0, 3)]))
        w16(ctx, cell + 0xab, rg.choice([0, 1, 2, 3, 0xffff, rg.randrange(0, 4)]))
    w32(ctx, 0x1015dc78, ctx.u32())
    w32(ctx, 0x1037305c, ctx.u32())
    return [a]


# ---- 0x1008 .. 0x100c ---------------------------------------------------------
def g_8d181(ctx):
    rg = ctx.rng
    vals = [rg.choice([fix(ctx), rg.randrange(0, 0x40000), rg.randrange(-0x20000, 0x20000)]) & 0xffffffff for _ in range(4)]
    if rg.random() < 0.5:
        x0 = rg.randrange(0, 0x30000)
        y0 = rg.randrange(0, 0x30000)
        vals = [x0, y0, x0 + rg.randrange(0, 0x30000), y0 + rg.randrange(0, 0x30000)]
    rect = ctx.buf(20, struct.pack('<5I', rg.getrandbits(32), *vals))
    out = ctx.buf(20)
    r = rg.random()
    if r < 0.12:
        out = rect
    return [rect, out, fix(ctx), fix(ctx)]


def g_9b30f(ctx):
    rg = ctx.rng
    w32(ctx, 0x1015d1ac, rg.choice([0xffffffff, 0, 1, 2, 3, 4, 7, rg.randrange(-1, 6), ctx.u32()]))
    w32(ctx, 0x1015d1cc, rg.getrandbits(32))
    w32(ctx, 0x1015d098, rg.getrandbits(32))
    return [rg.choice([0xffffffff, 0, 1, 2, 3, 4, 7, rg.randrange(-1, 6), ctx.u32()]) & 0xffffffff]


def g_9c9e5(ctx):
    rg = ctx.rng
    rnd(ctx, 0x101cfce0, 4 * 140)
    w32(ctx, 0x1017cd44, rg.choice([0, 1, 125, 126, 127, 128, 130, rg.randrange(0, 127), rg.randrange(0, 127), rg.randrange(0, 127), 0xfffffffe]))
    return []


def g_a9b7f(ctx):
    return [ctx.buf(0x100)]


def g_aabfd(ctx):
    rg = ctx.rng
    node = ctx.buf(0x20)
    n = rg.choice([0, 1, 2, 3, 6])
    chain = [ctx.buf(0x20) for _ in range(n)]
    for i, c in enumerate(chain):
        w32(ctx, c + 8, chain[i + 1] if i + 1 < n else 0)
    w32(ctx, node + 4, chain[0] if chain else 0)
    return [node]


def g_b3520(ctx):
    w32(ctx, 0x1015d0a4, ctx.buf(0x40))
    return [ctx.u32()]


def g_b4300(ctx):
    rg = ctx.rng
    idx = rg.choice([0, 1, 2, 10, 20, rg.randrange(0, 40)])
    rnd(ctx, 0x101a2100, 0x88 * 41)
    return [idx, rg.choice([ctx.u32(), rg.randrange(0, 16), 0xf, 0x10, 0xffff])]


def g_c3702(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x60)
    blk = ctx.buf(0x40)
    w32(ctx, obj + 0x54, blk)
    return [obj, rg.choice([0, 1, 0x10000, 0x12340000, 0x8000, 0xffff, 0xff00, ctx.u32(), rg.getrandbits(32)])]


def g_c6f0e(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x4c0)
    count = rg.choice([0, 1, 3, 8, 0xffffffff, rg.randrange(0, 20)])
    w32(ctx, obj + 0x4b4, count)
    return [obj, rg.choice([0xffffffff, 0, 1, 2, 7, count, count - 1, rg.randrange(-2, 20)]) & 0xffffffff]


# ---- 0x100d ----------------------------------------------------------------
def make_nodes(ctx, n):
    rg = ctx.rng
    nodes = [ctx.buf(0x40) for _ in range(n)]
    for i, nd in enumerate(nodes):
        w32(ctx, nd + 4, rg.choice([0, 0] + [nodes[j] for j in range(i)]) if i else 0)
        w32(ctx, nd + 8, rg.choice([0, 0] + [nodes[j] for j in range(i)]) if i else 0)
    return nodes


def g_d16e4(ctx):
    rg = ctx.rng
    nodes = make_nodes(ctx, rg.randrange(1, 7))
    owner = ctx.buf(0x40)
    w32(ctx, owner + 0x10, 0 if rg.random() < 0.12 else rg.choice(nodes))
    w32(ctx, owner + 0xc, 0 if rg.random() < 0.5 else rg.choice(nodes))
    return [owner, rg.choice(nodes)]


def g_d1834(ctx):
    rg = ctx.rng
    nodes = make_nodes(ctx, rg.randrange(1, 7))
    owner = ctx.buf(0x40)
    w32(ctx, owner + 4, 0 if rg.random() < 0.4 else rg.choice(nodes))
    w32(ctx, owner + 0x14, 0 if rg.random() < 0.3 else rg.choice(nodes))
    return [owner, rg.choice(nodes)]


def g_d339e(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x60)
    for off in (0x2c, 0x30, 0x34, 0x38, 0x4c, 0x50):
        w32(ctx, obj + off, rg.choice([rg.randrange(-100, 100), rg.randrange(-3, 4), ctx.u32(), 0x7fffffff, 0x80000000]) & 0xffffffff)
    o1, o2 = ctx.buf(4), ctx.buf(4)
    if rg.random() < 0.08:
        o2 = o1
    return [obj, o1, o2]


def g_d3142(ctx):
    rg = ctx.rng
    rnd(ctx, 0x10185400, 0x240)
    if rg.random() < 0.12:
        return [0x10185400 + rg.choice([4, 8, 0x14, 0x18, 0x28, 0x1c8])]
    return [ctx.buf(0x14)]


def g_sel1(ctx):
    return [make_block(ctx)]


def g_5cc1(ctx):
    return [make_block(ctx), ctx.rng.choice([0, 1, 1])]


def g_5f40(ctx):
    blk = make_block(ctx, count=ctx.rng.choice([1, 3, 8, 8, 0, 0xffffffff]))
    return [blk]


def g_63d4(ctx):
    rg = ctx.rng
    blk = make_block(ctx)
    ent = ctx.buf(0x40)
    w32(ctx, ent + 0x28, blk)
    idx = rg.randrange(0, 40)
    w32(ctx, 0x1017b8c8, idx)
    w32(ctx, 0x109e8b40 + 4 * idx, ent)
    return []


def g_63fa(ctx):
    rg = ctx.rng
    blk = make_block(ctx)
    for i in range(8):  # +4 is also a table index here: keep most in range
        pass
    return [blk]


# ---- 0x1010 ----------------------------------------------------------------
def g_1a70(ctx):
    rg = ctx.rng
    data = ctx.buf(32)
    off = rg.choice([0, 2, 4, 8, 1, 6])
    return [ctx.buf(4, struct.pack('<I', data + off))]


def g_3800(ctx):
    rg = ctx.rng
    keys = [rg.randrange(0, 40) for _ in range(27)]
    for i, k in enumerate(keys):
        wr(ctx, 0x101598b0 + i * 8, '<II', k, rg.getrandbits(32))
    return [rg.choice([rg.choice(keys), rg.choice(keys), rg.randrange(0, 45), ctx.u32()])]


def g_6d80(ctx):
    rg = ctx.rng
    hdr = ctx.buf(0x80, 'rand')
    region = ctx.buf(0x400)
    zero = rg.choice([0.5, 0.8, 0.95])
    data = bytearray(0 if rg.random() < zero else rg.randrange(1, 256) for _ in range(0x3f8))
    data += bytes([0xff] * 8)
    ctx.m.uc.mem_write(region, bytes(data))
    idx = rg.choice([0, 1, 2, 3, 4, 0xffffffff])
    low = rg.randrange(0, 0x1000)
    base = (region - (idx * 0x1000 + low)) & 0xffffffff
    w32(ctx, hdr + 0x10, base)
    off = rg.choice([rg.randrange(8, 0xf8), rg.randrange(0xd0, 0xf8), rg.randrange(0xe8, 0xf8), 8])
    block = region + off
    size = rg.choice([0, 1, 3, 8, 12, rg.randrange(0, 40)])
    w8(ctx, block, size)
    mode = rg.random()
    if mode < 0.25:
        new = rg.randrange(0, size + 1) if size else 0
    elif mode < 0.35:
        new = size
    elif mode < 0.9:
        new = size + rg.randrange(1, 30)
    else:
        new = rg.choice([0x100, 0x1ff, 0xf8 - off, 0xf8 - off + size, 0xf9 - off + size, ctx.u32()])
    end = block + new
    cur = rg.choice([block, block - 1, block + 1, end - 1, end, end + 1, region + 8, region + 0xf7, 0, rg.getrandbits(32), block + size])
    w32(ctx, region, cur)
    w32(ctx, region + 4, rg.getrandbits(32))
    # clear the bytes between the old and new end most of the time
    if rg.random() < 0.7 and 0 < new < 0x300:
        s = block + size - region
        e = min(block + new - region, 0x3f8)
        if s < e:
            ctx.m.uc.mem_write(region + s, bytes(e - s))
    return [hdr, region, block, new & 0xffffffff]


def g_70b0(ctx):
    rg = ctx.rng
    sec = rg.choice([rg.randrange(0, 61), rg.randrange(0, 61), 0, 59, rg.randrange(-3, 100)])
    vals = [sec, rg.randrange(0, 60), rg.randrange(0, 24), rg.randrange(1, 32),
            rg.choice([rg.randrange(0, 12), rg.randrange(0, 12), 0, 11, rg.randrange(-2, 16)]),
            rg.choice([rg.randrange(0, 300), rg.randrange(70, 130), 99, 100, 1999 - 1900, 0, rg.randrange(-300, 3000), ctx.u32()]),
            rg.choice([rg.randrange(0, 7), rg.randrange(0, 7), 6, rg.randrange(-1, 9)])]
    rnd(ctx, 0x1017e978, 0x40)
    return [ctx.buf(0x20, struct.pack('<7i', *[(v - (1 << 32) if v >= 1 << 31 else v) for v in vals]) + bytes(4))]


def g_71a0(ctx):
    rg = ctx.rng
    return [ctx.buf(16), rg.choice([rg.randrange(-200, 200), rg.randrange(0, 100), ctx.u32(), 0, 9, 10, 99, 100]) & 0xffffffff]


def g_7f70(ctx):
    rg = ctx.rng
    return [rg.choice([932, 936, 949, 950, 931, 933, 935, 937, 948, 951, 932 + 0x12, 932 + 0x13, 0, 1252, ctx.u32(), rg.randrange(925, 960)])]


def spec(name, addr, gen, void=False, trials=400, **kw):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials)
    if void:
        d['void'] = True
    d.update(kw)
    return d


SPECS = [
    spec('Sim_100071f6', 0x100071f6, none, void=True, trials=300),
    spec('Sim_1000942e', 0x1000942e, g_obj_short, void=True, trials=300),
    spec('Sim_10009558', 0x10009558, g_9558, void=True, trials=500),
    spec('Sim_10012ad9', 0x10012ad9, g_3u32, trials=300),
    spec('Sim_10015cf8', 0x10015cf8, g_15cf8, void=True, trials=400),
    spec('Sim_10015c0e', 0x10015c0e, g_15c0e, trials=600),
    spec('Sim_10019d8c', 0x10019d8c, g_19d8c, trials=300),
    spec('Sim_1001c075', 0x1001c075, g_1c075, trials=1000),
    spec('Sim_1001db13', 0x1001db13, g_1db13, trials=800),
    spec('Sim_100258d7', 0x100258d7, g_258d7, trials=400),
    spec('Sim_1002590c', 0x1002590c, g_2590c, trials=500),
    spec('Sim_1002850e', 0x1002850e, none, void=True, trials=300),
    spec('Sim_10028513', 0x10028513, g_u32, trials=300),
    spec('Sim_1002895a', 0x1002895a, g_u32, trials=300),
    spec('Sim_10028a74', 0x10028a74, g_28a74, void=True, trials=300),
    spec('Sim_1002990f', 0x1002990f, g_2990f, void=True, trials=400),
    spec('Sim_1002ba28', 0x1002ba28, g_2ba28, void=True, trials=500),
    spec('Sim_1002d263', 0x1002d263, g_2d263, trials=300),
    spec('Sim_10033555', 0x10033555, g_33555, trials=300),
    spec('Sim_10035444', 0x10035444, g_35444, void=True, trials=400),
    spec('Sim_10037675', 0x10037675, none, trials=300),
    spec('Sim_1003a53b', 0x1003a53b, g_3a53b, trials=300),
    spec('Sim_1003b471', 0x1003b471, g_3b471, void=True, trials=400),
    spec('Sim_1003b4d0', 0x1003b4d0, g_3b4d0, trials=600),
    spec('Sim_1004044c', 0x1004044c, g_4044c, void=True, trials=1500),
    spec('Sim_10040645', 0x10040645, g_40645, void=True, trials=300),
    spec('Sim_10040f2d', 0x10040f2d, g_40f2d, void=True, trials=300),
    spec('Sim_1004bac0', 0x1004bac0, g_bac0, trials=800),
    spec('Sim_1004bf28', 0x1004bf28, g_pack4, trials=400),
    spec('Sim_1004c2f7', 0x1004c2f7, g_2u32, trials=300),
    spec('Sim_1004c501', 0x1004c501, g_pack5a, trials=400),
    spec('Sim_1004c8a5', 0x1004c8a5, g_pack5b, trials=400),
    spec('Sim_1004ccd9', 0x1004ccd9, g_flags11, trials=600),
    spec('Sim_1004d257', 0x1004d257, g_2u32, trials=300),
    spec('Sim_1004d285', 0x1004d285, g_d285, trials=500),
    spec('Sim_1004f0a6', 0x1004f0a6, g_4f0a6, void=True, trials=300),
    spec('Sim_10050671', 0x10050671, g_50671, void=True, trials=500),
    spec('Sim_10050c5b', 0x10050c5b, g_50c5b, void=True, trials=400),
    spec('Sim_10057900', 0x10057900, none, trials=300),
    spec('Sim_100765a7', 0x100765a7, g_765a7, trials=500),
    spec('Sim_1007b42c', 0x1007b42c, g_7b42c, trials=800),
    spec('Sim_1007b4d6', 0x1007b4d6, g_7b4d6, trials=1000),
    spec('Sim_1007caab', 0x1007caab, g_7caab, void=True, trials=500),
    spec('Sim_1007dbaa', 0x1007dbaa, none, trials=300),
    spec('Sim_10083495', 0x10083495, none, void=True, trials=300),
    spec('Rect_ScaleAroundCenter', 0x1008d181, g_8d181, trials=800),
    spec('Sim_1009b30f', 0x1009b30f, g_9b30f, void=True, trials=600),
    spec('Sim_1009c9e5', 0x1009c9e5, g_9c9e5, trials=500),
    spec('Sim_100a9b7f', 0x100a9b7f, g_a9b7f, trials=300),
    spec('Sim_100aabfd', 0x100aabfd, g_aabfd, trials=400),
    spec('Sim_100b3520', 0x100b3520, g_b3520, trials=300),
    spec('Sim_100b4300', 0x100b4300, g_b4300, void=True, trials=400),
    spec('Sim_100c3702', 0x100c3702, g_c3702, void=True, trials=400),
    spec('Sim_100c6f0e', 0x100c6f0e, g_c6f0e, trials=500),
    spec('Sim_100d16e4', 0x100d16e4, g_d16e4, trials=800),
    spec('Sim_100d1834', 0x100d1834, g_d1834, trials=800),
    spec('Sim_100d3142', 0x100d3142, g_d3142, void=True, trials=400),
    spec('Sim_100d339e', 0x100d339e, g_d339e, void=True, trials=500),
    spec('Sim_100d5cc1', 0x100d5cc1, g_5cc1, void=True, trials=1500),
    spec('Sim_100d5de5', 0x100d5de5, g_sel1, void=True, trials=1500),
    spec('Sim_100d5f40', 0x100d5f40, g_5f40, void=True, trials=1500),
    spec('Sim_100d63d4', 0x100d63d4, g_63d4, void=True, trials=800),
    spec('Sim_100d63fa', 0x100d63fa, g_63fa, void=True, trials=1000),
    spec('Sim_100d82f1', 0x100d82f1, none, void=True, trials=300),
    spec('Sim_10101a70', 0x10101a70, g_1a70, trials=300),
    spec('Sim_10103800', 0x10103800, g_3800, trials=500),
    spec('Sim_10106d80', 0x10106d80, g_6d80, trials=2500),
    spec('Time_FormatTm', 0x101070b0, g_70b0, trials=800),
    spec('Text_PutTwoDigits', 0x101071a0, g_71a0, trials=500),
    spec('Sim_10107f70', 0x10107f70, g_7f70, trials=400),
    spec('Sim_101083b0', 0x101083b0, g_u32, void=True, trials=300),
]
