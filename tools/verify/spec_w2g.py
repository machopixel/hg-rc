# Differential tests for wave 2g (see difftest.py): leaf functions from 0x100071da..0x1010b5a0.
import struct

SOURCES = [
    'sim/unsorted_1000/w2g_fields.c',
    'sim/unsorted_1001/w2g_fields.c',
    'sim/unsorted_1002/w2g_records.c',
    'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1003/records.c',
    'sim/unsorted_1004/w2g_state.c',
    'sim/unsorted_1005/w2g_pool.c',
    'sim/unsorted_1007/w2g_stubs.c',
    'sim/unsorted_1008/w2g_rect.c',
    'sim/unsorted_1009/w2g_objects.c',
    'sim/unsorted_100a/w2g_misc.c',
    'sim/unsorted_100b/w2g_misc.c',
    'sim/unsorted_100c/w2g_object.c',
    'sim/unsorted_100d/w2g_misc.c',
    'sim/unsorted_100f/w2g_crt.c',
    'sim/unsorted_1010/w2g_crt.c',
    'sim/view/viewrect.c',
]


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))

def w16(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<H', v & 0xffff))

def w8(ctx, addr, v):
    ctx.m.uc.mem_write(addr, bytes([v & 0xff]))

def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))

def rnd(ctx, addr, size):
    """Random bytes (so that a function that reads them has something to read)."""
    wb(ctx, addr, bytes(ctx.rng.getrandbits(8) for _ in range(size)))

def sm(ctx, lo=-3, hi=12):
    return ctx.rng.randrange(lo, hi) & 0xffffffff

def pick(ctx, *a):
    return ctx.rng.choice(a)

def maybe(ctx, p):
    return ctx.rng.random() < p

def nullable(ctx, size, p_null=0.1):
    return 0 if maybe(ctx, p_null) else ctx.buf(size)


# ---------------------------------------------------------------- 0x1000
def g_100071da(ctx):
    r = ctx.rng
    idx = r.randrange(-100, 300) & 0xffff
    if maybe(ctx, 0.3):
        idx |= r.getrandbits(16) << 16
    return [idx, ctx.u32()]

def g_10008ef8(ctx):
    obj = ctx.buf(0x80)
    w32(ctx, obj + 0x30, 0 if maybe(ctx, 0.2) else ctx.buf(0x40))
    return [obj]

def g_1000941e(ctx):
    return [ctx.buf(0x80), ctx.u32()]

def g_10009613(ctx):
    r = ctx.rng
    obj = ctx.buf(0x80)
    if maybe(ctx, 0.1):
        w32(ctx, obj + 0x30, 0)
        return [obj, sm(ctx, 0, 5), nullable(ctx, 8), nullable(ctx, 8), nullable(ctx, 8), sm(ctx, 0, 30)]
    block = ctx.buf(0x900)
    w32(ctx, obj + 0x30, block)
    off = r.choice([0x20, 0x40, 0x100, r.randrange(0x10, 0x100)])
    w32(ctx, block + 8, off)
    index = r.randrange(0, 5)
    for i in range(6):
        rec = block + off + i * 0x28
        w16(ctx, rec, r.getrandbits(16))
        w16(ctx, rec + 2, r.choice([0, 1, 3, 8, 20, r.randrange(0, 60), r.randrange(0, 200)]))
        w32(ctx, rec + 4, r.randrange(0x10, 0x200))
        rnd(ctx, rec + 8, 0x20)
    rnd(ctx, block + off + 6 * 0x28, 0x500)
    out = nullable(ctx, 0x400, 0.15)
    return [obj, index, nullable(ctx, 8), nullable(ctx, 8), out,
            r.choice([0, 1, 5, 20, 100, 255, 1000, r.randrange(0, 70), 0x7fffffff])]

def g_1000971b(ctx):
    p = ctx.buf(16)
    return [p, pick(ctx, ctx.u32(), 0, 0xffffffff, 0x10f, 0xfffffef0, ctx.rng.getrandbits(32))]

def g_1000afe7(ctx):
    r = ctx.rng
    v = pick(ctx, r.getrandbits(32), r.getrandbits(16) | 0x200, r.getrandbits(16) | 0x200, r.getrandbits(8), 0x200)
    cur = pick(ctx, v & 0xff, v & 0xff, r.randrange(0, 256), ctx.u32(), (v & 0xff) | 0x100)
    w32(ctx, 0x1017b8c8, cur)
    return [v]

def g_1000df50(ctx):
    rnd(ctx, 0x10a38de0, 4)
    rnd(ctx, 0x10a39200, 0x14 * 256 + 8)
    return []

def g_1000e510(ctx):
    return [pick(ctx, sm(ctx, -5, 300), ctx.u32(), 0, 255, 256)]


# ---------------------------------------------------------------- 0x1001
def g_10010145(ctx):
    a = pick(ctx, ctx.buf(8), 0, ctx.u32() | 1)
    b = ctx.buf(0x40)
    w32(ctx, b + 8, nullable(ctx, 0x20, 0.2))
    w32(ctx, 0x10149a90, ctx.u32())
    if maybe(ctx, 0.1):
        b = 0
    return [a, b]

def g_10013e20(ctx):
    obj = ctx.buf(0x200)
    return [obj, ctx.u32()]

def g_10016c73(ctx):
    r = ctx.rng
    a = r.choice([0, 1, 2, 3, r.randrange(0, 4)])
    lo = r.choice([0, 1, 5, 0x13e, r.randrange(0, 0x140)])
    hi = r.choice([0, 0, 0, r.randrange(1, 40), 0xffff])
    c = (hi << 16) | lo
    row = r.randrange(-3, 60)
    w16(ctx, 0x104727a0 + a * 4, row & 0xffff)
    addr = (a * 0x4fc3a + 0x10473800 + row * 0x13f + 0x3a + c * 2 + 0xef) & 0xffffffff
    rnd(ctx, addr - 4, 8)
    return [a, ctx.u32(), c]

def g_1001c618(ctx):
    r = ctx.rng
    return [r.choice([r.randrange(0, 40), r.randrange(0, 40), r.getrandbits(32), (r.getrandbits(24) << 8) | r.randrange(0, 30)])]


# ---------------------------------------------------------------- 0x1002
def g_100289a2(ctx):
    dst = ctx.buf(0x200)
    src = ctx.buf(0x40)
    w32(ctx, src, *[ctx.u32() for _ in range(5)])
    return [dst, src]

def g_one_obj(ctx):
    return [ctx.buf(0x40)]

def g_1002a16b(ctx):
    rnd(ctx, 0x109e9ba0, 0x50 * 256)
    return [pick(ctx, sm(ctx, 0, 254), 0, 253, 254, 255)]

def g_1002a1c1(ctx):
    rnd(ctx, 0x109e9ba0, 0x50 * 256)
    return []

def g_1002b92d(ctx):
    r = ctx.rng
    for i in range(30):
        w32(ctx, 0x109e8660 + 0x14 * i, ctx.u32(),
            r.randrange(-3_000_000, 3_000_000), r.randrange(-2_000_000, 2_000_000),
            r.randrange(-3_000_000, 3_000_000), r.randrange(-2_000_000, 2_000_000))
        w32(ctx, 0x109e8240 + 8 * i, r.getrandbits(32), r.getrandbits(32))
        if maybe(ctx, 0.6):
            q = ctx.buf(0x20)
            block = ctx.buf(0x20)
            w32(ctx, q + 4, block)
            for k in (4, 8, 0xc):
                rect = ctx.buf(0x20)
                w32(ctx, rect, ctx.u32(), *[r.getrandbits(32) for _ in range(4)])
                w32(ctx, block + k, rect)
            w32(ctx, 0x10138158 + 4 * i, q)
        else:
            w32(ctx, 0x10138158 + 4 * i, 0)
    w32(ctx, 0x109e3e14, pick(ctx, 639, 799, 319, r.randrange(0, 2000), r.getrandbits(32)))
    w32(ctx, 0x109e3de0, pick(ctx, 479, 599, 199, r.randrange(0, 2000), r.getrandbits(32)))
    return []


# ---------------------------------------------------------------- 0x1003
def g_100332e9(ctx):
    w32(ctx, 0x1014cb34, ctx.u32())
    return [ctx.u32(), pick(ctx, 0, 0, 1, ctx.u32())]

def g_1003c860(ctx):
    r = ctx.rng
    n = r.choice([0, 1, 2, 70, 71, 72, r.randrange(0, 300)])
    buf = ctx.buf(320)
    return [buf, n, pick(ctx, ctx.u32(), r.getrandbits(32), r.randrange(0, 200), 0xffffffff)]

def g_1003cba3(ctx):
    w32(ctx, 0x109b9e08, ctx.u32())
    return [ctx.u32()]


# ---------------------------------------------------------------- 0x1004
def g_10041e61(ctx):
    r = ctx.rng
    n = r.randrange(2, 7)
    w32(ctx, 0x1015ec58, 0 if maybe(ctx, 0.08) else pick(ctx, 1, 1, 1, 2, ctx.u32() | 1))
    w32(ctx, 0x1015ec84, 0 if maybe(ctx, 0.05) else 1)
    cur = r.randrange(0, n)
    w32(ctx, 0x1017b8c8, cur)
    nflag = 8
    for i in range(n):
        o = ctx.buf(0x200)
        rnd(ctx, o, 0x200)
        w16(ctx, o + 0x1c, ctx.u32())
        w32(ctx, o + 4, pick(ctx, r.randrange(0, n), cur, cur, ctx.u32()))
        w32(ctx, o + 8, r.randrange(0, nflag))
        w32(ctx, o + 0x14, pick(ctx, 3, 3, 0, 1, ctx.u32()))
        w32(ctx, 0x109e8b40 + 4 * i, o)
    for k in range(nflag):
        w32(ctx, 0x109b7d54 + 0x6c * k, ctx.u32())
    m = r.randrange(0, 8)
    ids = [r.randrange(0, n) for _ in range(m)]
    recs = ctx.buf(0x1e * (m + 4))
    recbase = recs + 0x1e                       # one record of padding below, for the index -1 case
    rnd(ctx, recs, 0x1e * (m + 4))
    for i in range(m):
        w16(ctx, recbase + 0x1e * i + 0x1a, ids[i])
        w8(ctx, recbase + 0x1e * i + 0x1c, pick(ctx, 0, 1, 2, 0, 1, r.getrandbits(8)))
    # the current object's id (what Sim_1003dbc0 searches for) sits in one of the records, usually
    if m and maybe(ctx, 0.85):
        w16(ctx, recbase + 0x1e * r.randrange(0, m) + 0x1a, cur)
    root = ctx.buf(0x400)
    rnd(ctx, root, 0x400)
    w32(ctx, root + 0x12, m if maybe(ctx, 0.95) else pick(ctx, -1, 0, 100))
    w32(ctx, root + 0x126, recbase)
    w8(ctx, root + 0x137, pick(ctx, 0, 1, 2))
    w8(ctx, root + 0x138, pick(ctx, 0, 1, 2))
    w32(ctx, 0x1015d004, root)
    st = ctx.buf(0x20)
    w8(ctx, st + 0xa, pick(ctx, 0, 1, 2))
    w8(ctx, st + 8, r.getrandbits(8))
    w32(ctx, 0x1017b8c4, st)
    w32(ctx, 0x1015dc64, pick(ctx, n, n, n, r.randrange(0, n + 1), 0))
    w32(ctx, 0x1013bb40, ctx.u32())
    w32(ctx, 0x1015ec54, ctx.u32())
    return []

def g_1004f1df(ctx):
    r = ctx.rng
    v = r.choice([r.randrange(0, 30), r.randrange(0, 30), r.randrange(-3, 300) & 0xffffffff, r.getrandbits(32)])
    if maybe(ctx, 0.3):
        v = (v & 0xff) | (r.getrandbits(24) << 8)
    return [v]


# ---------------------------------------------------------------- 0x1005 (entry pool and its list)
POOL = 0x109b16e0
def pool_entry(n):
    return POOL + 0x4c * n

def setup_pool(ctx, n=None, with_list=True, p_list=0.6, counts=False):
    """Entries 0..n-1, a random subset of them linked in a list, the stack and its top."""
    r = ctx.rng
    n = n or r.randrange(2, 9)
    w32(ctx, 0x109b0eac, n if maybe(ctx, 0.9) else r.choice([0, 1, n + 2, n]))
    top = r.choice([-1, 0, r.randrange(-1, n), r.randrange(-1, n), n - 1, n - 2, n])
    w32(ctx, 0x109b0eb0, top)
    w32(ctx, 0x109b12c0, *[r.randrange(0, n) for _ in range(n + 3)])
    for i in range(n):
        rnd(ctx, pool_entry(i), 0x4c)
        w32(ctx, pool_entry(i) + 0xc, i if maybe(ctx, 0.85) else ctx.u32())
        w32(ctx, pool_entry(i), 0, 0)
        if counts:
            w32(ctx, pool_entry(i) + 0x44, pick(ctx, 0xffffffff, 0xffffffff, r.randrange(0, 20), r.randrange(-3, 5) & 0xffffffff))
            w32(ctx, pool_entry(i) + 0x48, r.randrange(-5, 25) & 0xffffffff)
    members = [i for i in range(n) if maybe(ctx, p_list)] if with_list else []
    r.shuffle(members)
    for a, b in zip(members, members[1:]):
        w32(ctx, pool_entry(a), pool_entry(b))      # next
        w32(ctx, pool_entry(b) + 4, pool_entry(a))  # previous
    if members:
        w32(ctx, 0x101668d0, pool_entry(members[0]))
        w32(ctx, 0x101668d4, pool_entry(members[-1]))
    else:
        mode = r.random()
        w32(ctx, 0x101668d0, 0 if mode < 0.8 else pool_entry(0))
        w32(ctx, 0x101668d4, 0 if mode < 0.9 else pool_entry(0))
    return n, members

def g_100570f1(ctx):
    return []

def g_10057f53(ctx):
    rnd(ctx, 0x1015e650, 16)
    return []

def g_100581a0(ctx):
    n, members = setup_pool(ctx)
    return [pick(ctx, pool_entry(ctx.rng.randrange(0, n)), pool_entry(ctx.rng.randrange(0, n)), 0)]

def g_1005820d(ctx):
    r = ctx.rng
    n, members = setup_pool(ctx, p_list=0.75)
    if members and maybe(ctx, 0.9):
        e = pool_entry(r.choice(members))
    elif maybe(ctx, 0.3):
        e = 0
    else:
        e = pool_entry(r.randrange(0, n))
    return [e]

def g_100582d2(ctx):
    r = ctx.rng
    setup_pool(ctx, p_list=0.75)
    return [pick(ctx, r.randrange(-1, 10), r.randrange(0, 6), r.randrange(0, 6), ctx.u32())]

def g_1005833d(ctx):
    setup_pool(ctx)
    return []

def g_10058493(ctx):
    n, members = setup_pool(ctx, counts=True, p_list=0.8)
    w32(ctx, 0x1015dc80, pick(ctx, 1, 2, 5, ctx.rng.randrange(0, 10), ctx.rng.randrange(-3, 3) & 0xffffffff))
    return []

def g_10058575(ctx):
    setup_pool(ctx)
    w32(ctx, 0x10143fd8, pick(ctx, 0, 1, 1, 1, ctx.u32()))
    return []


# ---------------------------------------------------------------- 0x1007, 0x1008
def g_none(ctx):
    return []

def g_1008cf1f(ctx):
    r = ctx.rng
    src = ctx.buf(0x20)
    lim_x, lim_y = 319 * 32768, 199 * 32768
    def val(lim):
        return pick(ctx, r.randrange(-lim, lim), r.randrange(-1000, 1000), r.randrange(-lim, lim), -lim, lim - 1, 0, -1, 1) & 0xffffffff
    w32(ctx, src, ctx.u32(), val(lim_x), val(lim_y), val(lim_x), val(lim_y))
    dst = src if maybe(ctx, 0.15) else ctx.buf(0x20)
    return [src, dst]


# ---------------------------------------------------------------- 0x1009
def setup_objlist(ctx):
    """The list head/tail at 0x1017cc28 / 0x1017cc2c: empty, one object, several, or half-empty."""
    r = ctx.rng
    mode = r.random()
    if mode < 0.3:
        w32(ctx, 0x1017cc28, 0); w32(ctx, 0x1017cc2c, 0)
    elif mode < 0.4:
        w32(ctx, 0x1017cc28, 0); w32(ctx, 0x1017cc2c, ctx.buf(16))
    elif mode < 0.5:
        w32(ctx, 0x1017cc28, ctx.buf(16)); w32(ctx, 0x1017cc2c, 0)
    else:
        a = ctx.buf(16)
        b = a if maybe(ctx, 0.3) else ctx.buf(16)
        w32(ctx, 0x1017cc28, a); w32(ctx, 0x1017cc2c, b)

OBJPOOL = 0x10217c20
def setup_objpool(ctx, init_blocks=False):
    r = ctx.rng
    n = 6
    w32(ctx, 0x10215ae8, pick(ctx, -1, 0, r.randrange(-1, n), r.randrange(0, n), r.randrange(0, n)))
    w32(ctx, 0x10216b80, *[r.randrange(0, n) for _ in range(n + 2)])
    w32(ctx, 0x1014b9b8, pick(ctx, 0, 1, 1, ctx.u32()))
    for i in range(n):
        o = OBJPOOL + 0x57e * i
        rnd(ctx, o, 0x57e)
        if init_blocks:
            blk = ctx.buf(0x100)
            rnd(ctx, blk, 0x100)
            p = ctx.buf(0x100)
            w32(ctx, p + 0x8c, blk)
            w32(ctx, o + 0x552, p)
    setup_objlist(ctx)

def g_100903e0(ctx):
    setup_objlist(ctx)
    return [pick(ctx, ctx.buf(16), ctx.buf(16), 0)]

def g_10090581(ctx):
    setup_objpool(ctx)
    return [pick(ctx, 0, 1, ctx.u32())]

def g_10090c7d(ctx):
    setup_objpool(ctx)
    return [ctx.u32(), ctx.u32(), ctx.u32(), pick(ctx, 0, 1, ctx.u32())]

def g_10091071(ctx):
    return [pick(ctx, 0, 1, ctx.u32(), 0x80000000, 0xffffffff)]

def g_10091085(ctx):
    r = ctx.rng
    setup_objpool(ctx, True)
    limit = r.randrange(0, 20)
    w32(ctx, 0x1017cc20, limit)
    w32(ctx, 0x1017cc24, pick(ctx, limit - 2, limit - 3, r.randrange(-1, 20), limit - 1, 0xffffffff))
    w32(ctx, 0x1015dc78, ctx.u32())
    w32(ctx, 0x10215ae8, pick(ctx, -1, 0, 1, 2, 3, 4, 5))
    w32(ctx, 0x1014b9b8, 0)      # the constructors pass force = 1, so this one must not matter
    return [ctx.u32(), ctx.u32(), ctx.u32()]

def g_1009201f(ctx):
    a = g_10091085(ctx)
    return a + [ctx.u32()]

def g_gfxobj(ctx):
    w32(ctx, 0x1015d1b8, ctx.buf(0x100) if not maybe(ctx, 0.04) else 0)

def g_100987fa(ctx):
    g_gfxobj(ctx)
    return [ctx.u32(), ctx.u32()]

def g_10098881(ctx):
    w32(ctx, 0x1014d1d8, ctx.u32())
    return [ctx.u32()]

def g_10098896(ctx):
    g_gfxobj(ctx)
    w32(ctx, 0x1014d1d8, ctx.u32())
    return [ctx.u32(), pick(ctx, -1, -1, 0, 1, 5, ctx.u32())]

def g_10098a18(ctx):
    g_gfxobj(ctx)
    w32(ctx, 0x10152418, ctx.u32())
    w32(ctx, 0x1014cb34, ctx.u32())
    return [ctx.u32(), pick(ctx, 0, 0, 1, ctx.u32())]

def g_1009b2b4(ctx):
    r = ctx.rng
    w32(ctx, 0x1015d1ac, pick(ctx, sm(ctx, -5, 6), sm(ctx, -5, 6), ctx.u32()))
    w32(ctx, 0x1015d1cc, pick(ctx, sm(ctx, -5, 6), sm(ctx, -5, 6), ctx.u32()))
    return []

def g_1009c710(ctx):
    for a in (0x101c528c, 0x101c4bcc, 0x101c4f78, 0x101c45f4, 0x101c4b90):
        w32(ctx, a, ctx.u32())
    rnd(ctx, 0x1014c31c, 0x1014c490 - 0x1014c31c)
    return []

def g_1009e5ac(ctx):
    w32(ctx, 0x1017d064, ctx.u32())
    return [ctx.u32()]


# ---------------------------------------------------------------- 0x100a, 0x100b, 0x100c
def g_100a8943(ctx):
    r = ctx.rng
    cur = pick(ctx, r.getrandbits(32), r.randrange(0, 1000), 0xffffffe0, 0xffffff00)
    w32(ctx, 0x1017d178, cur)
    w32(ctx, 0x1017d174, pick(ctx, (cur + 0x10 + 0xc8) & 0xffffffff, (cur + 0x10 + 0xc7) & 0xffffffff,
                              (cur + 0x10 + 0xc9) & 0xffffffff, r.randrange(0, 2000), r.getrandbits(32)))
    w32(ctx, 0x101c9ea4, ctx.u32())
    return []

def g_100aa807(ctx):
    return [pick(ctx, ctx.u32(), 0, 0xfffffff8, ctx.rng.randrange(0, 1000))]

def g_100ab89a(ctx):
    r = ctx.rng
    for i in range(3):
        w32(ctx, 0x1014d1b8 + 8 * i, r.randrange(-5, 12) & 0xffffffff, r.randrange(-5, 12) & 0xffffffff)
    w32(ctx, 0x101c64f0, ctx.u32())
    pt = ctx.buf(8)
    w32(ctx, pt, pick(ctx, r.randrange(-3, 14), r.randrange(0, 12), 0x80000000, ctx.u32()) & 0xffffffff,
        pick(ctx, r.randrange(-3, 14), r.randrange(0, 12), 0x7fffffff, ctx.u32()) & 0xffffffff)
    return [pt]

def g_100ac5da(ctx):
    w32(ctx, 0x1017d1e0, ctx.u32())
    return [pick(ctx, sm(ctx, -5, 25), sm(ctx, -2, 18), ctx.u32(), 0, 1, 15, 16)]

def g_100ac603(ctx):
    w32(ctx, 0x1014d1d8, ctx.u32())
    return [pick(ctx, 0xffffffff, 0xffffffff, 0, 5, ctx.u32())]

def g_100b3547(ctx):
    for a in (0x101c47bc, 0x101c45e4, 0x101c4858, 0x101c4f90, 0x101c4758, 0x101c4fd8):
        w32(ctx, a, ctx.u32())
    rnd(ctx, 0x10150634, 0x1015069c + 4 - 0x10150634)
    return []

def g_100b367e(ctx):
    r = ctx.rng
    w32(ctx, 0x1017d20c, pick(ctx, sm(ctx, 0, 20), sm(ctx, 0, 20), sm(ctx, -2, 3), ctx.u32()))
    w32(ctx, 0x1017d210, pick(ctx, sm(ctx, 0, 20), sm(ctx, 0, 20), sm(ctx, -2, 25), ctx.u32()))
    w32(ctx, 0x1017d214, ctx.buf(0x400) + 0x40)
    return [ctx.u32(), ctx.u32()]

def g_100cb9a4(ctx):
    r = ctx.rng
    n = 5
    for i in range(n):
        o = ctx.buf(0x100)
        f = pick(ctx, r.getrandbits(32), 0x200 | r.getrandbits(8), 0x200 | r.getrandbits(8), 0x200 | r.getrandbits(8) | (r.getrandbits(20) << 12), 0x100, 0x300)
        w32(ctx, o + 0xf0, f)
        w32(ctx, 0x109e8b40 + 4 * i, o)
    w32(ctx, 0x1017b8c8, r.randrange(0, n))
    return []


# ---------------------------------------------------------------- 0x100d
def g_100d1a60(ctx):
    r = ctx.rng
    for a in (0x10144fa0, 0x101452d0, 0x1015f1d8, 0x109b9e08):
        w32(ctx, a, ctx.u32())
    w32(ctx, 0x1014cb1c, pick(ctx, 1, 1, 0, 2, ctx.u32()))
    code = pick(ctx, 0x2b, 0x54, 0x58, 0xc4, 0xc5, 0x2a, 0x2c, 0x53, 0x55, 0x57, 0x59, 0xc3, 0xc6, 0, r.randrange(0, 256),
                r.randrange(0, 256), ctx.u32(), 0xffffffff)
    return [code]

def g_100d2c86(ctx):
    w32(ctx, 0x10152418, ctx.u32())
    return [ctx.u32(), pick(ctx, 0, 0, 1, ctx.u32())]

def g_100d3a67(ctx):
    w32(ctx, 0x1017e65c, ctx.u32())
    return []

def g_100d635e(ctx):
    r = ctx.rng
    obj = ctx.buf(0x100)
    tab = ctx.buf(0x128 * 8)
    rnd(ctx, tab, 0x128 * 8)
    w32(ctx, obj + 0x84, tab)
    return [obj, r.randrange(0, 6), pick(ctx, sm(ctx, -3, 6), sm(ctx, 0, 3), 0x7fffffff, 0x80000000, ctx.u32())]

def g_100d67af(ctx):
    r = ctx.rng
    obj = ctx.buf(0x600)
    rnd(ctx, obj, 0x600)
    tab = ctx.buf(0x128 * 8)
    rnd(ctx, tab, 0x128 * 8)
    n = r.randrange(0, 5)
    w32(ctx, obj + 0x84, tab)
    w32(ctx, obj + 0x4fc, n)
    for i in range(7):
        w32(ctx, tab + 0x128 * i, pick(ctx, 0xffffffff, r.randrange(0, 20), 1, 0xffffffff))
        w32(ctx, tab + 0x128 * i + 0xc, pick(ctx, 0, 1, 2, 3, sm(ctx, -1, 5), ctx.u32()))
    return [obj]


# ---------------------------------------------------------------- 0x100f, 0x1010 (C runtime)
def g_abs(ctx):
    return [pick(ctx, ctx.u32(), 0, 1, 0xffffffff, 0x80000000, 0x7fffffff, sm(ctx, -50, 50))]

def text(ctx, alphabet, maxlen):
    n = ctx.rng.randrange(0, maxlen)
    s = bytes(ord(ctx.rng.choice(alphabet)) for _ in range(n)) + b'\0'
    a = ctx.buf(maxlen + 8)
    wb(ctx, a, s)
    return a, n

def g_100fd560(ctx):
    r = ctx.rng
    alpha = 'abc ,;.'
    s, n = text(ctx, alpha, 24)
    delims = ''.join(c for c in ' ,;.' if maybe(ctx, 0.5))
    d = ctx.buf(16)
    wb(ctx, d, delims.encode() + b'\0')
    if maybe(ctx, 0.3):
        # continue a previous call: the saved position is somewhere inside (or at the end of) a string
        w32(ctx, 0x1017e884, s + r.randrange(0, n + 1))
        return [0, d]
    w32(ctx, 0x1017e884, ctx.u32())
    return [s, d]

def g_10101d48(ctx):
    r = ctx.rng
    hi = pick(ctx, r.getrandbits(32), 0x7ff00000 | r.getrandbits(20), 0x7ff00000 | (r.getrandbits(1) << 31),
              0xfff00000, 0x7fe00000 | r.getrandbits(20), 0x3ff00000, 0)
    return [ctx.u32(), hi]

def g_10103830(ctx):
    r = ctx.rng
    return [pick(ctx, r.getrandbits(32), r.getrandbits(8), 1 << r.randrange(0, 8), r.getrandbits(8) & 0xdf, r.getrandbits(8) & 0xd7, r.getrandbits(16) << 8)]

def g_1010b270(ctx):
    r = ctx.rng
    s, n = text(ctx, 'abcd', 40)
    return [s, pick(ctx, r.randrange(0, n + 3), r.randrange(0, n + 3), n, n + 1, 0, 1, 0xffffffff, 0x7fffffff)]

def g_1010b5a0(ctx):
    r = ctx.rng
    n = r.randrange(0, 30)
    s = ctx.buf(100)
    wb(ctx, s, b''.join(struct.pack('<H', r.choice([1, 0x41, 0xff00, 0x100, 0xffff, r.randrange(1, 65536)])) for _ in range(n)) + b'\0\0')
    return [s]


def spec(name, addr, gen, trials=400, void=False):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials)
    if void:
        d['void'] = True
    return d

SPECS = [
    spec('Sim_100071da', 0x100071da, g_100071da, 400, True),
    spec('Sim_10008ef8', 0x10008ef8, g_10008ef8),
    spec('Sim_1000941e', 0x1000941e, g_1000941e, 400, True),
    spec('Sim_10009613', 0x10009613, g_10009613, 800, True),
    spec('Sim_1000971b', 0x1000971b, g_1000971b, 400, True),
    spec('Sim_1000afe7', 0x1000afe7, g_1000afe7),
    spec('Sim_1000df50', 0x1000df50, g_1000df50, 300, True),
    spec('Sim_1000e510', 0x1000e510, g_1000e510),
    spec('Sim_10010145', 0x10010145, g_10010145, 400, True),
    spec('Sim_10013e20', 0x10013e20, g_10013e20),
    spec('Sim_10016c73', 0x10016c73, g_10016c73, 600),
    spec('Sim_1001c618', 0x1001c618, g_1001c618, 600),
    spec('Sim_100289a2', 0x100289a2, g_100289a2, 400, True),
    spec('Sim_10028a84', 0x10028a84, g_one_obj, 300, True),
    spec('Sim_1002a16b', 0x1002a16b, g_1002a16b, 400, True),
    spec('Sim_1002a1c1', 0x1002a1c1, g_1002a1c1, 300, True),
    spec('Sim_1002b92d', 0x1002b92d, g_1002b92d, 400, True),
    spec('Sim_100332e9', 0x100332e9, g_100332e9, 300, True),
    spec('Sim_1003c860', 0x1003c860, g_1003c860, 600, True),
    spec('Sim_1003cba3', 0x1003cba3, g_1003cba3),
    spec('Sim_10041e61', 0x10041e61, g_10041e61, 1000, True),
    spec('Sim_1004f1df', 0x1004f1df, g_1004f1df, 500),
    spec('Sim_100570f1', 0x100570f1, g_100570f1, 300),
    spec('Sim_10057f53', 0x10057f53, g_10057f53, 300, True),
    spec('Sim_100581a0', 0x100581a0, g_100581a0, 600, True),
    spec('Sim_1005820d', 0x1005820d, g_1005820d, 800, True),
    spec('Sim_100582d2', 0x100582d2, g_100582d2, 800),
    spec('Sim_1005833d', 0x1005833d, g_1005833d, 600),
    spec('Sim_10058493', 0x10058493, g_10058493, 1000, True),
    spec('Sim_10058575', 0x10058575, g_10058575, 600),
    spec('Sim_100585b3', 0x100585b3, g_100582d2, 600),
    spec('Sim_1007cbb4', 0x1007cbb4, g_none, 300, True),
    spec('Sim_1007d941', 0x1007d941, g_none, 300),
    spec('Sim_1008cf1f', 0x1008cf1f, g_1008cf1f, 500),
    spec('Sim_100903e0', 0x100903e0, g_100903e0, 500, True),
    spec('Sim_10090581', 0x10090581, g_10090581, 600),
    spec('Sim_10090c7d', 0x10090c7d, g_10090c7d, 600),
    spec('Sim_10091071', 0x10091071, g_10091071, 300),
    spec('Sim_10091085', 0x10091085, g_10091085, 800, True),
    spec('Sim_1009201f', 0x1009201f, g_1009201f, 800, True),
    spec('Sim_100987fa', 0x100987fa, g_100987fa, 300, True),
    spec('Sim_10098881', 0x10098881, g_10098881),
    spec('Sim_10098896', 0x10098896, g_10098896, 300, True),
    spec('Sim_10098a18', 0x10098a18, g_10098a18, 300, True),
    spec('Sim_10098e62', 0x10098e62, g_none, 300),
    spec('Sim_10098e74', 0x10098e74, g_none, 300),
    spec('Sim_1009b2b4', 0x1009b2b4, g_1009b2b4, 500, True),
    spec('Sim_1009c710', 0x1009c710, g_1009c710, 300, True),
    spec('Sim_1009e5ac', 0x1009e5ac, g_1009e5ac, 300, True),
    spec('Sim_100a8943', 0x100a8943, g_100a8943, 500),
    spec('Sim_100aa807', 0x100aa807, g_100aa807),
    spec('Sim_100ab89a', 0x100ab89a, g_100ab89a, 600, True),
    spec('Sim_100ac5da', 0x100ac5da, g_100ac5da, 400, True),
    spec('Sim_100ac603', 0x100ac603, g_100ac603),
    spec('Sim_100b3547', 0x100b3547, g_100b3547, 300, True),
    spec('Sim_100b367e', 0x100b367e, g_100b367e, 600),
    spec('Sim_100b6590', 0x100b6590, g_none, 300),
    spec('Sim_100b65b2', 0x100b65b2, g_none, 300, True),
    spec('Sim_100cb9a4', 0x100cb9a4, g_100cb9a4, 500),
    spec('Sim_100d1a60', 0x100d1a60, g_100d1a60, 600),
    spec('Sim_100d2c86', 0x100d2c86, g_100d2c86, 300, True),
    spec('Sim_100d3a67', 0x100d3a67, g_100d3a67),
    spec('Sim_100d635e', 0x100d635e, g_100d635e, 500, True),
    spec('Sim_100d67af', 0x100d67af, g_100d67af, 600, True),
    spec('Sim_100da428', 0x100da428, g_none, 300, True),
    spec('Math_AbsInt_100fd100', 0x100fd100, g_abs),
    spec('Str_NextToken_100fd560', 0x100fd560, g_100fd560, 1000),
    spec('Math_AbsInt_101003d0', 0x101003d0, g_abs),
    spec('Sim_10101d48', 0x10101d48, g_10101d48),
    spec('Sim_10103830', 0x10103830, g_10103830, 500),
    spec('Str_LengthMax_1010b270', 0x1010b270, g_1010b270, 600),
    spec('Wstr_Length_1010b5a0', 0x1010b5a0, g_1010b5a0, 400),
]

# round 32: the pool layout follows W31_TexMode() (off here: the 1.0 layout)
SOURCES = SOURCES + [x for x in ['sim/render/w31_tex.c'] if x not in SOURCES]

# round 32: the 1.0 effect spawns call 1.2's in 1.2 mode (off here): stand-ins, and the mode switch
SOURCES = SOURCES + [x for x in ['sim/render/w31_tex.c', '../tools/verify/w32_smoke_stub.c'] if x not in SOURCES]
