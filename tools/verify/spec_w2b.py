# Differential tests for wave 2b (see difftest.py).
import struct
SOURCES = ['sim/core/timer.c', 'sim/core/w2b_timers.c',
           'sim/unsorted_1003/entry.c', 'sim/unsorted_1003/records.c',
           'sim/unsorted_1003/w2b_entries.c', 'sim/unsorted_1003/w2b_misc.c',
           'sim/unsorted_100d/nodes.c', 'sim/unsorted_100d/flags.c', 'sim/unsorted_100d/state.c',
           'sim/util/field.c', 'sim/util/lookup.c', 'sim/util/record.c',
           'sim/view/objlist.c',
           'sim/unsorted_1004/fields.c', 'sim/unsorted_1004/w2b_fields.c', 'sim/unsorted_1004/w2b_checks.c']

def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))

def fill(ctx, addr, size):
    rng = ctx.rng
    words = [rng.choice([rng.getrandbits(32), rng.randrange(0, 40), rng.randrange(0, 40), 0xFFFFFFFF,
                         0x80000000, rng.randrange(-5, 0) & 0xffffffff]) for _ in range(size // 4)]
    w32(ctx, addr, *words)

# --- timers (0x1002abc3, 0x1002abd6, 0x1002ac1b)
def timer_tables(ctx):
    rng = ctx.rng
    for base in (0x10153180, 0x10153280):
        slots = [rng.choice([0, 0, 0, rng.getrandbits(32), 1, 0xfffffff0]) for _ in range(63)]
        w32(ctx, base, *(slots + [0xFFFFFFFF]))
    w32(ctx, 0x10153380, rng.choice([0, 1, 7, 1000, rng.getrandbits(32)]))
    w32(ctx, 0x10153384, rng.choice([0, 1, 7, 1000, rng.getrandbits(32)]))
    tn = lambda: rng.choice([0, 1, 5, 62, 0x80, 0x81, 0x85, 0xBE, rng.randrange(0, 0x100)])
    w32(ctx, 0x101354d0, tn()); w32(ctx, 0x101354d4, tn()); w32(ctx, 0x101354d8, tn())
    w32(ctx, 0x1015dc98, rng.getrandbits(32)); w32(ctx, 0x1015dc78, rng.getrandbits(32))

def g_timers(ctx):
    timer_tables(ctx); return []

# --- entry table (0x10033c67, 0x10033ced)
def g_entries(ctx):
    rng = ctx.rng
    count = rng.choice([0, 1, 2, 5, 8, 12, rng.randrange(0, 13), -1, -5, 0x80000000])
    w32(ctx, 0x109e4cc0, count)
    vals = [rng.choice([0xffffffff, 0, 1, 2, 3, 4, rng.randrange(0, 8), rng.getrandbits(32)]) for _ in range(14)]
    for i, v in enumerate(vals):
        w32(ctx, 0x109e4cc4 + i * 0x44 + 0x14, v)
        w32(ctx, 0x109e4cc4 + i * 0x44 + 0x10, rng.getrandbits(32))
    return [rng.choice([-1, 0, 1, 2, 3, 4, 7, rng.choice(vals), rng.choice(vals), rng.choice(vals), rng.getrandbits(32)])]

# --- Sim_10035469: the preset selector with argument 0
def g_preset0(ctx):
    rng = ctx.rng
    view = ctx.buf(0x80)
    w32(ctx, 0x1014cae0, view if rng.random() < 0.97 else 0)
    w32(ctx, 0x10185400, *[rng.getrandbits(32) for _ in range(55)])
    w32(ctx, 0x1015241c, rng.choice([0xffffffff, 0, 0, 3, 10, rng.randrange(0, 11)]))
    w32(ctx, 0x1013a014, rng.choice([0, 1]))
    return []

# --- Sim_10038d65: list of objects (first dword = key, +0x690 next), argument is a key
def g_objlist(ctx):
    rng = ctx.rng
    src = ctx.buf(0x100)
    n = rng.choice([0, 1, 2, 3, 5])
    objs = [ctx.buf(0x6a0) for _ in range(n)]
    keys = [rng.choice([src, rng.getrandbits(32), rng.randrange(0, 8)]) for _ in objs]
    for i, o in enumerate(objs):
        w32(ctx, o, keys[i])
        w32(ctx, o + 0x690, objs[i + 1] if i + 1 < n else 0)
    w32(ctx, 0x109e3760, objs[0] if objs else 0)
    arg = rng.choice([src, src, src, keys[0] if keys else 0, rng.getrandbits(32)])
    if arg != src and rng.random() < 0.5:
        arg = src
    return [arg]

# --- table lookups (0x1004c085 family)
def g_sum(ctx):
    rng = ctx.rng
    fill(ctx, 0x101116a0, 0xaa * 40 + 0x40)
    fill(ctx, 0x10124ea0, 0x98 * 30 + 0x40)
    row = rng.choice([0, 1, 2, 5, 39, rng.randrange(0, 40), rng.randrange(-3, 40)])
    return [row & 0xffffffff, ctx.u32() if rng.random() < 0.3 else rng.getrandbits(32)]

def g_sum1(ctx):
    rng = ctx.rng
    fill(ctx, 0x101116a0, 0xaa * 40 + 0x40)
    fill(ctx, 0x10124ea0, 0x98 * 30 + 0x40)
    row = rng.choice([0, 1, 2, 5, 39, rng.randrange(0, 40), rng.randrange(-3, 40)])
    hi = rng.choice([0, 1, 2, 7, 29, rng.randrange(0, 30), rng.randrange(-2, 30), rng.getrandbits(16)]) & 0xffff
    return [row & 0xffffffff, (hi << 16) | rng.getrandbits(16)]

# --- Sim_10042908
def g_check(ctx):
    rng = ctx.rng
    w32(ctx, 0x1015ec84, rng.choice([1, 1, 1, 0]))
    w32(ctx, 0x1015ec58, rng.choice([1, 1, 1, 0, 5]))
    root = ctx.buf(0x200)
    count = rng.choice([0, 1, 4, 8, rng.randrange(0, 10)])
    keys = ctx.buf(0x40)
    recs = ctx.buf(0x1e * 10)
    for i in range(10):
        ctx.m.uc.mem_write(recs + i * 0x1e + 0x1a, struct.pack('<H', rng.choice([rng.randrange(0, 24), rng.randrange(0, 24), 0xffff])))
    ctx.m.uc.mem_write(keys, struct.pack('<16H', *[rng.choice([0xfa01, 0xfa01, 0, 1, rng.getrandbits(16)]) for _ in range(16)]))
    w32(ctx, root + 0x12, count)
    w32(ctx, root + 0x16, keys + 16)     # keys[-1] stays readable
    w32(ctx, root + 0x126, recs)
    w32(ctx, 0x1015d004, root)
    ptrs = []
    for _ in range(24):
        o = ctx.buf(0x200)
        bptr = ctx.buf(16)
        w32(ctx, o + 0x160, rng.choice([0, 0, 0, 1, rng.getrandbits(32)]))
        w32(ctx, o + 0x164, rng.choice([0, 0, 1, rng.getrandbits(32)]))
        w32(ctx, o + 0x168, bptr if rng.random() < 0.97 else 0)
        ctx.m.uc.mem_write(bptr, bytes([rng.choice([0, 0, 1, 255, rng.getrandbits(8)])]))
        ctx.m.uc.mem_write(o + 0x1c, struct.pack('<H', rng.choice([0, 2, 0xe, 0x10, rng.getrandbits(16)])))
        ptrs.append(o)
    w32(ctx, 0x109e8b40, *ptrs)
    idb = ctx.buf(8)
    ctx.m.uc.mem_write(idb, bytes([rng.randrange(0, 24) if rng.random() < 0.97 else rng.getrandbits(8),
                                   rng.choice([2, 2, 2, 2, 4, 0, 1, rng.getrandbits(8)])]))
    return [idb]

def spec(name, addr, gen, trials=400, **kw):
    return dict(name=name, orig=addr, c=name, gen=gen, trials=trials, **kw)

SPECS = [
    spec('Sim_1002abc3', 0x1002abc3, g_timers, void=True),
    spec('Sim_1002abd6', 0x1002abd6, g_timers, void=True),
    spec('Sim_1002ac1b', 0x1002ac1b, g_timers),
    spec('Sim_10033c67', 0x10033c67, g_entries, trials=600),
    spec('Sim_10033ced', 0x10033ced, g_entries, trials=600),
    spec('Sim_10035469', 0x10035469, g_preset0, void=True),
    spec('Sim_10038d65', 0x10038d65, g_objlist, void=True, trials=500),
    spec('Sim_10042908', 0x10042908, g_check, trials=1500),
]
for n in ['1004c085', '1004c161', '1004c1c1', '1004c297']:
    SPECS.append(spec('Sim_' + n, int(n, 16), g_sum))
for n in ['1004c3ad', '1004c4a1', '1004c4d1']:
    SPECS.append(spec('Sim_' + n, int(n, 16), g_sum1))
