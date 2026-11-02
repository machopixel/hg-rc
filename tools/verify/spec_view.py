# Differential tests for src/sim/view (see difftest.py).
import struct

SOURCES = ['sim/view/viewrect.c', 'sim/view/matrix.c', 'sim/view/objlist.c']

def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))

def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]

def fix(ctx):
    """A 16.16 fixed point value: small, around one, large, or any 32 bits."""
    r = ctx.rng.random()
    if r < 0.25: return ctx.rng.randrange(-0x20000, 0x20000) & 0xffffffff
    if r < 0.5: return ctx.rng.randrange(0, 0x10000)
    if r < 0.6: return ctx.rng.choice([0, 0x10000, 0xffff0000, 0x8000, 0x7fffffff, 0x80000000, 0xffffffff, 0x18000])
    if r < 0.7: return ctx.rng.randrange(-1000, 1000) & 0xffffffff
    return ctx.rng.getrandbits(32)

def view_size(ctx):
    """Display width - 1 and height - 1."""
    for a in (0x109e3e14, 0x109e3de0):
        w32(ctx, a, ctx.rng.choice([639, 799, 319, 479, 0, 1, 599, 1023, ctx.rng.randrange(0, 2000), ctx.rng.getrandbits(32)]))

def rect(ctx):
    return ctx.buf(20, struct.pack('<5I', ctx.rng.getrandbits(32), *[fix(ctx) for _ in range(4)]))

def pixrect(ctx):
    rg = ctx.rng
    return ctx.buf(20, struct.pack('<5I', rg.getrandbits(32), *[rg.choice([rg.randrange(-50, 700), rg.getrandbits(32)]) & 0xffffffff for _ in range(4)]))

def g_scale_rect(ctx):
    view_size(ctx)
    r = rect(ctx)
    return [ctx.u32(), r, r if ctx.rng.random() < 0.2 else ctx.buf(20)]

def g_scale_point(ctx):
    view_size(ctx)
    p = ctx.buf(8, struct.pack('<2I', fix(ctx), fix(ctx)))
    return [ctx.u32(), p, p if ctx.rng.random() < 0.2 else ctx.buf(8)]

def g_center(ctx):
    view_size(ctx)
    r = pixrect(ctx)
    return [ctx.u32(), r, r if ctx.rng.random() < 0.2 else ctx.buf(20)]

def g_rect_point(ctx):
    a = rect(ctx)
    p = ctx.buf(8, struct.pack('<2I', fix(ctx), fix(ctx)))
    o = ctx.rng.random()
    return [a, p, p if o < 0.1 else (a if o < 0.2 else ctx.buf(8))]

def g_rect_rect(ctx):
    a = rect(ctx)
    r = rect(ctx)
    o = ctx.rng.random()
    return [a, r, r if o < 0.15 else (a if o < 0.3 else ctx.buf(20))]

def g_interp(ctx):
    a, b = rect(ctx), rect(ctx)
    o = ctx.rng.random()
    return [a, b, a if o < 0.1 else (b if o < 0.2 else ctx.buf(20)), fix(ctx), ctx.rng.choice([0, 1, 2, 3, ctx.u32()])]

def g_transpose(ctx):
    s = ctx.buf(48)
    return [s, s if ctx.rng.random() < 0.25 else ctx.buf(48)]

def g_ident(ctx):
    return [ctx.buf(48)]

def node(ctx, linked=None):
    """A list node (0x40 bytes); next at 0x14, prev at 0x18, flags at 0, owner at 0x28."""
    rg = ctx.rng
    n = ctx.buf(0x40, 'zero')
    return n

def g_insert(ctx):
    rg = ctx.rng
    a, b = ctx.buf(0x40, 'zero'), ctx.buf(0x40, 'zero')
    prev, owner, ownd = ctx.buf(0x40, 'zero'), ctx.buf(8, 'zero'), ctx.buf(8, 'zero')
    # node a: usually free, sometimes flagged, linked or owned
    if rg.random() < 0.15: w32(ctx, a, rg.choice([0x800, 0x801, 0x1800]))
    else: w32(ctx, a, rg.getrandbits(16) & 0xf7ff)
    if rg.random() < 0.1: w32(ctx, a + 0x18, prev)
    if rg.random() < 0.1: w32(ctx, a + 0x14, prev)
    r = rg.random()
    if r < 0.3: w32(ctx, a + 0x28, owner)           # owner present, [owner] == 0
    elif r < 0.5: w32(ctx, a + 0x28, ownd); w32(ctx, ownd, rg.getrandbits(32) | 1)
    # target b: usually has a predecessor
    if rg.random() < 0.6: w32(ctx, b + 0x18, prev)
    w32(ctx, b + 0x14, rg.choice([0, ctx.buf(8)]))
    return [rg.choice([a, a, a, 0]), rg.choice([b, b, b, 0])]

def g_unlink(ctx):
    rg = ctx.rng
    n, p, x = ctx.buf(0x40, 'zero'), ctx.buf(0x40, 'zero'), ctx.buf(0x40, 'zero')
    if rg.random() < 0.8: w32(ctx, n + 0x18, p)
    if rg.random() < 0.8: w32(ctx, n + 0x14, x)
    w32(ctx, p + 0x14, n); w32(ctx, x + 0x18, n)
    return [rg.choice([n, n, n, 0])]

def g_const(ctx):
    return []

def g_find(ctx):
    rg = ctx.rng
    keys = [rg.randrange(0, 12) for _ in range(rg.randrange(0, 6))]
    nodes = [ctx.buf(0x20, 'zero') for _ in keys]
    for i, (n, k) in enumerate(zip(nodes, keys)):
        w32(ctx, n, k)
        w32(ctx, n + 0x14, nodes[i + 1] if i + 1 < len(nodes) else 0)
    w32(ctx, 0x1015dc14, nodes[0] if nodes else 0)
    return [rg.choice(keys + [99, 0, 0xffffffff]) if keys else rg.randrange(0, 12)]

def g_push(ctx):
    rg = ctx.rng
    s = ctx.buf(0x20, 'zero')
    arr = ctx.buf(0x40, 'rand')
    w32(ctx, s + 8, arr)
    w32(ctx, s + 0xc, rg.choice([0, 1, 5, 6, 7, 7, 8, 9, 100, 0xffffffff - 1, 0x80000000, rg.randrange(0, 10)]) if rg.random() < 0.9 else rg.getrandbits(32))
    w32(ctx, s, rg.getrandbits(32))
    return [s, ctx.u32()]

def g_wrap(ctx):
    o = ctx.buf(0x40, 'rand')
    w32(ctx, o + 0x30, ctx.rng.choice([0, 1, 12, 13, 13, 14, 15, 0xffffffff, ctx.rng.randrange(0, 20), ctx.rng.getrandbits(32)]))
    return [o]

def g_snapshot(ctx):
    rg = ctx.rng
    ptrs = [ctx.buf(0x40, 'rand') for _ in range(4)]
    for a in (0x109e890c, 0x109e88e8, 0x109e8928, 0x109e8938, 0x109e894c, 0x109e8950, 0x109e8954):
        w32(ctx, a, rg.choice(ptrs))
    return []

def g_kind(ctx):
    rg = ctx.rng
    objs = [ctx.buf(0x40, 'rand') for _ in range(6)]
    ctx.m.uc.mem_write(0x109e8b40, struct.pack('<256I', *[rg.choice(objs) for _ in range(256)]))
    ctx.m.uc.mem_write(0x109e9ba0, bytes(rg.getrandbits(8) for _ in range(256 * 0x50)))
    idv = ctx.buf(4, 'rand')
    ctx.m.uc.mem_write(idv, bytes([rg.getrandbits(8), rg.choice([0, 1, 1, 2, 2, 2, 4, 4, 4, 8, 3, rg.getrandbits(8)])]))
    return [idv]

def g_kind5(ctx):
    return [ctx.rng.choice([5, 8, 5, 8, 0, 1, 4, 6, 7, 9, 0xffffffff, ctx.u32()])]

def g_bits(ctx):
    return [ctx.rng.choice([0, 1, 0xffffffff, 0x80000000, 0xf0f0f0f0, ctx.u32(), ctx.u32(), ctx.rng.getrandbits(32)])]

def g_collect(ctx):
    rg = ctx.rng
    objs = [ctx.buf(0x20, 'zero') for _ in range(5)]
    for o in objs[:3]:
        w32(ctx, o + 8, rg.getrandbits(32))      # non-zero field at 8 (or sometimes zero)
        if rg.random() < 0.3: w32(ctx, o + 8, 0)
    w32(ctx, 0x1017c8c4, rg.choice([0, 1, 1, 1, 1, rg.getrandbits(32)]))
    w32(ctx, 0x1017c8c8, rg.choice([0, objs[0], objs[1], objs[3]]))
    w32(ctx, 0x1017c8cc, rg.choice([0, objs[2], rg.getrandbits(32)]))
    tab = [rg.choice([0, 0, 0, 0, objs[0], objs[1], objs[2], objs[3], objs[4]]) for _ in range(81)]
    blob = bytearray(9 * 0x18c)
    for i in range(9):
        for j in range(9):
            struct.pack_into('<I', blob, i * 0x18c + j * 0x2c, tab[i * 9 + j])
    cur = bytes(ctx.m.uc.mem_read(0x1017ba4c, len(blob)))
    # keep the DLL's own bytes between the entries, replace only the entries
    cur = bytearray(cur)
    for i in range(9):
        for j in range(9):
            o = i * 0x18c + j * 0x2c
            cur[o:o + 4] = blob[o:o + 4]
    ctx.m.uc.mem_write(0x1017ba4c, bytes(cur))
    return []

def g_pp(ctx):
    inner = ctx.buf(8, 'rand')
    return [ctx.buf(4, struct.pack('<I', inner))]

SPECS = [
    dict(name='View_ScaleRect', orig=0x1008ca50, c='View_ScaleRect', gen=g_scale_rect, trials=600),
    dict(name='View_ScalePoint', orig=0x1008ccad, c='View_ScalePoint', gen=g_scale_point, trials=600),
    dict(name='View_CenterRect', orig=0x1008d0da, c='View_CenterRect', gen=g_center, trials=600),
    dict(name='Rect_ScalePoint', orig=0x1008cdc9, c='Rect_ScalePoint', gen=g_rect_point, trials=600),
    dict(name='Rect_ScaleToRect', orig=0x1008cafb, c='Rect_ScaleToRect', gen=g_rect_rect, trials=600),
    dict(name='Rect_Interpolate', orig=0x1008f668, c='Rect_Interpolate', gen=g_interp, trials=600),
    dict(name='Mat3_Transpose', orig=0x1006007d, c='Mat3_Transpose', gen=g_transpose, void=True),
    dict(name='Mat3x4_SetIdentity', orig=0x1006018d, c='Mat3x4_SetIdentity', gen=g_ident, void=True),
    dict(name='List_InsertBefore', orig=0x10088c9f, c='List_InsertBefore', gen=g_insert, void=True, trials=600),
    dict(name='ObjList_Unlink', orig=0x10088d23, c='ObjList_Unlink', gen=g_unlink, void=True),
    dict(name='Sim_10072ff2', orig=0x10072ff2, c='Sim_10072ff2', gen=g_const),
    dict(name='Sim_1007652f', orig=0x1007652f, c='Sim_1007652f', gen=g_find),
    dict(name='Sim_10076624', orig=0x10076624, c='Sim_10076624', gen=g_push),
    dict(name='Sim_10077fd8', orig=0x10077fd8, c='Sim_10077fd8', gen=g_wrap, void=True),
    dict(name='Sim_10078177', orig=0x10078177, c='Sim_10078177', gen=g_snapshot, void=True),
    dict(name='Sim_10079e3b', orig=0x10079e3b, c='Sim_10079e3b', gen=g_kind, trials=600),
    dict(name='Sim_10079feb', orig=0x10079feb, c='Sim_10079feb', gen=g_kind5),
    # dict(name='Bits_Count', orig=0x100819af, c='Bits_Count', gen=g_bits),   # deleted: DirectDraw / DISPDIB driver (DECISIONS.md "dropped features")
    dict(name='Sim_10087c86', orig=0x10087c86, c='Sim_10087c86', gen=g_collect, void=True),
    dict(name='Sim_1008f630', orig=0x1008f630, c='Sim_1008f630', gen=g_pp, void=True),
]
