# Differential tests for the wave 2 batch c (see difftest.py).
import struct

SOURCES = ['sim/unsorted_1004/w2c_nibblesum.c', 'sim/unsorted_1004/fields.c',
           'sim/unsorted_1005/layout.c', 'sim/view/viewrect.c',
           'sim/unsorted_1007/node_flag.c', 'sim/view/objlist.c', 'sim/unsorted_1008/cell_link.c']

def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))

def fix(ctx):
    r = ctx.rng.random()
    if r < 0.4: return ctx.rng.randrange(0, 0x30000)
    if r < 0.5: return ctx.rng.choice([0, 0x10000, 0x8000, 0x7fffffff, 0x80000000, 0xffffffff])
    return ctx.rng.getrandbits(32)

# ---- 0x1004c6f5 family: row of the 0xaa-stride table and a packed word
def g_sum(ctx):
    rg = ctx.rng
    row = rg.choice([rg.randrange(0, 40), rg.randrange(0, 40), rg.randrange(0, 200), ctx.u32()])
    return [row, rg.choice([rg.getrandbits(32), rg.getrandbits(32), rg.getrandbits(20), 0, 0xffffffff, 0xfffff])]

# ---- 0x100505f0 .. 0x10050b66: a structure with the rect pointer at +0xec
def obj(ctx):
    rg = ctx.rng
    o = ctx.buf(0x200, 'rand')
    area = ctx.buf(20, struct.pack('<5I', rg.getrandbits(32), *[fix(ctx) for _ in range(4)]))
    w32(ctx, o + 0xec, area)
    d = rg.random()
    if d < 0.6:
        ctx.m.uc.mem_write(o + 0x100, struct.pack('<hh', rg.randrange(-100, 1200), rg.randrange(-100, 1200)))
    return o

def g_obj(ctx):
    return [obj(ctx)]

def g_obj_sel(ctx):
    rg = ctx.rng
    o = obj(ctx)
    ents = [ctx.buf(0x40, 'rand') for _ in range(3)]
    for e in ents:
        other = ctx.buf(0x630, 'rand')
        if rg.random() < 0.5: w32(ctx, other + 0x613, 0)
        w32(ctx, e + 0x28, other)
    idx = rg.randrange(0, 256)
    w32(ctx, 0x1017b8c8, idx)
    w32(ctx, 0x109e8b40 + idx * 4, rg.choice(ents))
    return [o]

# ---- 0x10074bfa, 0x100751e4: list of nodes found by key
def g_key(ctx):
    rg = ctx.rng
    keys = [rg.randrange(0, 12) for _ in range(rg.randrange(0, 6))]
    nodes = [ctx.buf(0x20, 'rand') for _ in keys]
    for i, (n, k) in enumerate(zip(nodes, keys)):
        w32(ctx, n, k)
        w32(ctx, n + 0x14, nodes[i + 1] if i + 1 < len(nodes) else 0)
    w32(ctx, 0x1015dc14, nodes[0] if nodes else 0)
    w32(ctx, 0x1015dbcc, rg.choice([0, 0, 0, 1, rg.getrandbits(32)]))
    return [rg.choice(keys + [99, 0, 0xffffffff]) if keys else rg.randrange(0, 12)]

# ---- 0x1008a220
def g_cell(ctx):
    rg = ctx.rng
    n = ctx.buf(0xa0, 'zero')
    other = ctx.buf(0x40, 'zero')
    owner = ctx.buf(8, 'zero')
    glob = ctx.buf(0x40, 'zero')
    head = ctx.buf(0x40, 'zero')
    flags = rg.choice([0x800, 0x800, 0x800, 0x801, 0x1800, 0xffff, 0, 0x7ff, rg.getrandbits(16)])
    ctx.m.uc.mem_write(n, struct.pack('<H', flags))
    w32(ctx, n + 4, rg.choice([1, 1, 1, 0, rg.getrandbits(32)]))
    w32(ctx, n + 0x34, rg.choice([4, 3, 0, 5, rg.getrandbits(32)]))
    w32(ctx, n + 0x8c, rg.choice([0, 1, 4, 8, 9, 0xffffffff, rg.randrange(0, 9), rg.randrange(0, 9), rg.getrandbits(32)]))
    w32(ctx, n + 0x90, rg.choice([0, 1, 4, 8, 9, 0xffffffff, rg.randrange(0, 9), rg.randrange(0, 9), rg.getrandbits(32)]))
    if rg.random() < 0.15: w32(ctx, n + 0x14, other)
    if rg.random() < 0.15: w32(ctx, n + 0x18, other)
    r = rg.random()
    if r < 0.2: w32(ctx, n + 0x28, owner)
    elif r < 0.3: w32(ctx, n + 0x28, owner); w32(ctx, owner, 1)
    w32(ctx, 0x1017c8cc, rg.choice([glob, glob, glob, 0]))
    w32(ctx, 0x1017c8c4, rg.choice([0, 1, 1, rg.getrandbits(32)]))
    if rg.random() < 0.6: w32(ctx, glob + 0x10, head)
    if rg.random() < 0.5: w32(ctx, glob + 0x18, other)
    # grid cells and the default cell: some with a chain head, some with a predecessor
    cells = [0x1017b9b0] + [0x1017ba28 + x * 0x18c + y * 0x2c for x in range(9) for y in range(9)]
    for c in cells:
        w32(ctx, c + 0x10, rg.choice([0, 0, head]))
        w32(ctx, c + 0x18, rg.choice([0, 0, other]))
        w32(ctx, c + 0x14, 0)
    return [rg.choice([n, n, n, n, 0])]

SPECS = [
    dict(name='Sim_1004c6f5', orig=0x1004c6f5, c='Sim_1004c6f5', gen=g_sum),
    dict(name='Sim_1004c785', orig=0x1004c785, c='Sim_1004c785', gen=g_sum),
    dict(name='Sim_1004c815', orig=0x1004c815, c='Sim_1004c815', gen=g_sum),
    dict(name='Sim_1004ca99', orig=0x1004ca99, c='Sim_1004ca99', gen=g_sum),
    dict(name='Sim_1004cb29', orig=0x1004cb29, c='Sim_1004cb29', gen=g_sum),
    dict(name='Sim_1004cbb9', orig=0x1004cbb9, c='Sim_1004cbb9', gen=g_sum),
    dict(name='Sim_1004cc49', orig=0x1004cc49, c='Sim_1004cc49', gen=g_sum),
    dict(name='Sim_100505f0', orig=0x100505f0, c='Sim_100505f0', gen=g_obj_sel, void=True, trials=400),
    dict(name='Sim_100508df', orig=0x100508df, c='Sim_100508df', gen=g_obj, void=True),
    dict(name='Sim_1005095b', orig=0x1005095b, c='Sim_1005095b', gen=g_obj, void=True),
    dict(name='Sim_100509df', orig=0x100509df, c='Sim_100509df', gen=g_obj, void=True),
    dict(name='Sim_10050a63', orig=0x10050a63, c='Sim_10050a63', gen=g_obj, void=True, trials=600),
    dict(name='Sim_10050b66', orig=0x10050b66, c='Sim_10050b66', gen=g_obj, void=True, trials=600),
    dict(name='Sim_10074bfa', orig=0x10074bfa, c='Sim_10074bfa', gen=g_key),
    dict(name='Sim_100751e4', orig=0x100751e4, c='Sim_100751e4', gen=g_key, void=True),
    dict(name='Sim_1008a220', orig=0x1008a220, c='Sim_1008a220', gen=g_cell, void=True, trials=1000),
]
