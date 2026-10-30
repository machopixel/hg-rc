# Differential tests for wave 2a (see difftest.py): users of already converted functions.
import struct

SOURCES = [
    'sim/core/timer.c', 'sim/core/w2a_timers.c',
    'sim/view/viewrect.c', 'sim/view/w2a_points.c', 'sim/view/objlist.c',
    'sim/util/ring.c', 'sim/util/field.c', 'sim/util/lookup.c',
    'sim/unsorted_1004/tables.c',
    'sim/unsorted_1000/w2a_fields.c', 'sim/unsorted_1001/w2a_records.c',
]

def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))

def w16(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<H', v & 0xffff))

def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))

def ptrtab(ctx, n, size, fields=None):
    """Pointer table 0x109e8b40 with n records of `size` bytes; fields(i, addr) fills each."""
    recs = []
    for i in range(n):
        a = ctx.buf(size)
        if fields: fields(i, a)
        w32(ctx, 0x109e8b40 + 4 * i, a)
        recs.append(a)
    return recs

def ring(ctx, idx_addr, tab_addr):
    r = ctx.rng
    idx = r.choice([0, 1, 125, 126, 127, 128, r.randrange(0, 127), r.randrange(0, 127), r.randrange(0, 127)])
    w32(ctx, idx_addr, idx)
    wb(ctx, tab_addr, struct.pack('<140I', *[r.choice([ctx.u32(), r.randrange(0, 1000), r.getrandbits(32)]) for _ in range(140)]))

def fix(ctx):
    r = ctx.rng.random()
    if r < 0.25: return ctx.rng.randrange(-0x20000, 0x20000) & 0xffffffff
    if r < 0.5: return ctx.rng.randrange(0, 0x10000)
    if r < 0.6: return ctx.rng.choice([0, 0x10000, 0xffff0000, 0x8000, 0x7fffffff, 0x80000000, 0xffffffff, 0x18000])
    return ctx.rng.getrandbits(32)

# ---- timers ----
def tables(ctx):
    """Random timer tables (some used slots, some free, an end marker, sometimes a full table)."""
    rng = ctx.rng
    for base in (0x10153180, 0x10153280):
        slots = [rng.choice([0, 0, rng.getrandbits(32), 1, 0xfffffff0]) for _ in range(63)]
        mode = rng.random()
        if mode < 0.15:
            slots = [rng.choice([1, 5, rng.getrandbits(32) | 1]) for _ in range(63)]
        elif mode < 0.5:
            slots[rng.randrange(0, 63)] = 0xFFFFFFFF
        elif mode < 0.65:
            slots[rng.randrange(0, 63)] = 0
        ctx.m.uc.mem_write(base, struct.pack('<64I', *(slots + [0xFFFFFFFF])))
    for a in (0x10153380, 0x10153384):
        w32(ctx, a, ctx.rng.choice([0, 1, 7, 1000, ctx.rng.getrandbits(32)]))
    w32(ctx, 0x1015317c, ctx.rng.getrandbits(10))

def tnum(ctx):
    return ctx.rng.choice([0, 1, 5, 62, 0x80, 0x81, 0x85, 0xBE, ctx.rng.randrange(0, 0x100), ctx.rng.randrange(0, 64), 0x80 + ctx.rng.randrange(0, 64)])

def timer_globals(ctx):
    tables(ctx)
    for a in (0x101354d0, 0x101354d4, 0x101354d8):
        w32(ctx, a, tnum(ctx))
    w32(ctx, 0x101354cc, ctx.u32())
    w32(ctx, 0x1015dca0, ctx.rng.choice([0, 0, 1, ctx.u32()]))
    for a in (0x1015dc70, 0x1015dc74, 0x1015dc98, 0x1015dc78, 0x1015dc7c):
        w32(ctx, a, ctx.u32())

def g_timers(ctx):
    timer_globals(ctx); return []

# ---- view point scaling ----
def g_points(ctx):
    for a in (0x109e3e14, 0x109e3de0):
        w32(ctx, a, ctx.rng.choice([639, 799, 319, 479, 0, 1, 599, 1023, ctx.rng.randrange(0, 2000), ctx.rng.getrandbits(32)]))
    wb(ctx, 0x1012dc80, struct.pack('<6I', *[fix(ctx) for _ in range(6)]))
    return []

# ---- unsorted_1000 ----
def g_8c51(ctx):
    p = ctx.buf(0x100)
    return [p]

def g_8c77(ctx):
    r = ctx.rng
    p = ctx.buf(0x100)
    m = r.random()
    if m < 0.15:
        w32(ctx, p + 0x30, 0)
    elif m < 0.9:
        q = p if r.random() < 0.1 else ctx.buf(0x40)
        w32(ctx, p + 0x30, q)
        w32(ctx, q + 0xc, r.choice([0, 0, ctx.u32()]))
    return [p]

def g_c172(ctx):
    r = ctx.rng
    ring(ctx, 0x1017cd38, 0x101cffc0)
    n = 6
    cur = r.randrange(0, n)
    w32(ctx, 0x1017b8c8, cur)
    target = [None]
    def f(i, a):
        w32(ctx, a + 0x28, ctx.buf(0x600))
    ptrtab(ctx, n, 0x40, f)
    p = ctx.buf(0x200)
    rec = ctx.buf(0x600)
    w32(ctx, p + 0x28, rec if r.random() < 0.9 else p)
    w32(ctx, p, r.choice([8, 5, 0, 1, 7, 9, ctx.u32()]))
    w32(ctx, p + 0x1e8, r.choice([r.randrange(-5, 105), r.randrange(0, 101), ctx.u32(), 0, 100, 101]))
    return [p]

def g_c680(ctx):
    r = ctx.rng
    n = 40
    def f(i, a):
        w32(ctx, a, r.choice([8, 5, 0, 1, 8, 5, r.getrandbits(32)]))
    ptrtab(ctx, n, 0x40, f)
    p = ctx.buf(0x100)
    idx = r.choice([r.randrange(0, n), r.randrange(0, n), 0, n - 1])
    flags = idx | r.choice([0x200, 0x200, 0x200, 0]) | (r.getrandbits(32) & 0xfffffd00 if r.random() < 0.5 else 0)
    w32(ctx, p + 0xf0, flags)
    return [p if r.random() < 0.9 else 0]

def g_edce(ctx):
    ring(ctx, 0x1017cd38, 0x101cffc0)
    wb(ctx, 0x1012df30 - 40, struct.pack('<20I', *[ctx.u32() for _ in range(20)]))
    return []

# ---- unsorted_1001 ----
def recs_1c61(ctx, n, extra=None):
    """Records for Sim_10015c61 (+8 group, +0x10 key) plus whatever `extra` fills."""
    r = ctx.rng
    def f(i, a):
        w32(ctx, a + 8, r.randrange(0, 3)); w32(ctx, a + 0x10, r.randrange(0, 5))
        if extra: extra(i, a)
    ptrtab(ctx, n, 0x600, f)
    w32(ctx, 0x1017b8c8, r.randrange(0, n))
    w32(ctx, 0x1015dc64, r.choice([n, n, n, r.randrange(0, n + 1), 0, 0xffffffff]))

def g_100ca(ctx):
    r = ctx.rng
    n = 8
    def extra(i, a):
        w16(ctx, a + 0x19c + 0xa, r.choice([0, 1, 0x7fff, 0x8000, 0xffff, r.getrandbits(16), r.randrange(0, 50)]))
    recs_1c61(ctx, n, extra)
    if r.random() < 0.08: w32(ctx, 0x109e8b40 + 4 * r.randrange(0, n), 0)
    return [r.choice([r.randrange(-1, 8), r.randrange(-1, 8), r.randrange(0, 5), 8, 9, 0x7fffffff, 0x80000000, ctx.u32()])]

def g_10870(ctx):
    r = ctx.rng
    n = 8
    def extra(i, a):
        w16(ctx, a + 0x1a6, r.choice([5, 5, 0, 1, r.getrandbits(16)]))
        hi = r.choice([0x100, 0x200, 0x400, 0x400, 0x200, 0x100, 0, 0x300, 0x800, 0xf00, 0x500])
        w16(ctx, a + 0x1aa, hi | r.choice([0, 1, 3, 0x7f, 0xff, r.getrandbits(8)]) | (r.choice([0, 0, 0x8000, 0x1000, 0xf000]) if r.random() < 0.4 else 0))
    recs_1c61(ctx, n, extra)
    # table entries 0..255 are used as pointers by the 0x200 case
    for i in range(n, 256):
        w32(ctx, 0x109e8b40 + 4 * i, ctx.u32())
    arg = ctx.buf(0x40)
    w32(ctx, arg + 0xc, r.choice([r.randrange(0, 5), r.randrange(0, 5), 0xffffffff, ctx.u32()]))
    if r.random() < 0.05: w32(ctx, 0x109e8b40 + 4 * r.randrange(0, n), 0)
    return [ctx.u32(), arg, ctx.u32(), ctx.u32(), ctx.u32()]

def g_140df(ctx):
    r = ctx.rng
    n = 16
    owners = [ctx.buf(0x600) for _ in range(3)]
    def f(i, a):
        w32(ctx, a + 0x28, r.choice(owners))
    ptrtab(ctx, n, 0x40, f)
    for i in range(n):  # the index Sim_10052aaf takes comes from the first dword of the 0x6C-byte entries
        w32(ctx, 0x109b7d40 + 0x6c * i, r.choice([r.randrange(0, n), r.randrange(0, n), r.randrange(0, n), 0, 0xffffffff]))
    obj = ctx.buf(0x40)
    w32(ctx, obj + 8, r.choice([r.randrange(0, 16), r.randrange(0, 16), r.randrange(0, 20), 0xffffffff, ctx.u32()]))
    return [obj]

def spec(name, addr, gen, trials=500, void=False):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials)
    if void: d['void'] = True
    # Timer_Step's push ds / pop ds cannot run in the emulator (same as spec_core.py)
    if name == 'Sim_1002a8c2': d['nop'] = [0x100e4550, 0x100e4577]
    return d

SPECS = [
    spec('Sim_1002a8c2', 0x1002a8c2, g_timers, 500, True),
    spec('Sim_1002ab26', 0x1002ab26, g_timers, 500, True),
    spec('Sim_1002ab9d', 0x1002ab9d, g_timers),
    spec('Sim_1002abb0', 0x1002abb0, g_timers),
    spec('Sim_1000856d', 0x1000856d, g_points, 400, True),
    spec('Sim_10008c51', 0x10008c51, g_8c51, 300, True),
    spec('Sim_10008c77', 0x10008c77, g_8c77, 500, True),
    spec('Sim_1000c172', 0x1000c172, g_c172, 600, True),
    spec('Sim_1000c680', 0x1000c680, g_c680, 500),
    spec('Sim_1000edce', 0x1000edce, g_edce),
    spec('Sim_100100ca', 0x100100ca, g_100ca, 600),
    spec('Sim_10010870', 0x10010870, g_10870, 800),
    spec('Sim_100140df', 0x100140df, g_140df, 500),
]
