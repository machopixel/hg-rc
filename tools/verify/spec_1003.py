# Differential tests for src/sim/unsorted_1003 (see difftest.py).
import struct
SOURCES = ['sim/unsorted_1003/flags.c', 'sim/unsorted_1003/entry.c', 'sim/unsorted_1003/slots.c',
           'sim/unsorted_1003/records.c', 'sim/unsorted_1003/setters.c', 'sim/unsorted_1003/bitmap.c',
           'sim/unsorted_1003/scramble.c']

def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))

def mask(ctx):
    r = ctx.rng
    return r.choice([0x100, 0x200, 0x800, 0xb00, 0, 0xffffffff, ctx.u32(), 1 << r.randrange(32)])

def set_flags(ctx):
    w32(ctx, 0x1014cb38, ctx.rng.choice([0xb00, 0, 0xffffffff, ctx.u32()]))
    w32(ctx, 0x1014cb34, ctx.rng.choice([0, 1, ctx.u32()]))

def g_mask(ctx):
    set_flags(ctx); return [mask(ctx)]
def g_mask_on(ctx):
    set_flags(ctx); return [mask(ctx), ctx.rng.choice([0, 1, 2, 0xffffffff, ctx.u32()])]
def g_none(ctx):
    set_flags(ctx); return []

# ---- 0x44-byte entry table at 0x109e4cc4 ----
def entry_ptr(ctx):
    """An entry in a buffer (fields +0x10/+0x14 random or the special value -1), or NULL."""
    r = ctx.rng
    if r.random() < 0.15: return 0
    b = bytearray(r.getrandbits(8) for _ in range(0x44))
    b[0x14:0x18] = struct.pack('<I', r.choice([0xffffffff, 0, 5, r.getrandbits(32)]))
    b[0x10:0x14] = struct.pack('<I', r.choice([0, 1, r.getrandbits(32)]))
    return ctx.buf(0x44, bytes(b))

def g_entry(ctx):
    return [entry_ptr(ctx)]
def g_entry2(ctx):
    r = ctx.rng
    src = 0 if r.random() < 0.15 else ctx.buf(16)
    return [entry_ptr(ctx), src]
def g_index44(ctx):
    r = ctx.rng
    n = r.choice([0, 1, 5, 20, 0x7fffffff, 0x40000000, r.getrandbits(32), 0xffffffff])
    w32(ctx, 0x109e4cc0, n)
    return [r.choice([0, 1, 4, 19, 20, n, n - 1, n + 1, -1, -5, 0x3c3c3c3d, r.randrange(-3, 30), ctx.u32()]) & 0xffffffff]

# ---- 0x7e-byte table at 0x109b9e20 ----
def table7e(ctx, special_keys=()):
    r = ctx.rng
    n = r.choice([0, 1, 3, 8, 16, 40])
    w32(ctx, 0x1015ea80, n)
    for i in range(40):
        e = bytearray(r.getrandbits(8) for _ in range(0x7e))
        e[0:4] = struct.pack('<I', r.choice([0xfffffffe, 0xfffffffe, 0xffffffff, 0, r.getrandbits(32)]))
        e[0x48:0x4c] = struct.pack('<I', r.choice([0xffffffff, 0xffffffff, 0xfffffffe, 0, r.getrandbits(32)]))
        e[0x60:0x64] = struct.pack('<I', r.choice(special_keys + (r.randrange(0, 6),)) if special_keys else r.randrange(0, 6))
        ctx.m.uc.mem_write(0x109b9e20 + i * 0x7e, bytes(e))
    return n

def g_acf3(ctx):
    table7e(ctx); return [ctx.buf(16)]
def g_b417(ctx):
    n = table7e(ctx)
    return [ctx.rng.choice([0, 1, 2, n - 1, n, n + 1, -1, 0x80000000, ctx.rng.randrange(-2, 45)]) & 0xffffffff]
def g_b485(ctx):
    table7e(ctx)
    return [ctx.rng.choice([0, 1, 2, 3, 4, 5, 6, ctx.u32()])]
def g_b50f(ctx):
    table7e(ctx)
    r = ctx.rng
    return [r.choice([0, 1, 7, 39, r.randrange(0, 40), r.randrange(0, 40), -1 & 0xffffffff]), ctx.u32()]

# ---- structure behind 0x1015d004 ----
def root(ctx):
    """Root block with 8 keys and 8..12 records of 0x1e bytes."""
    r = ctx.rng
    count = r.choice([0, 1, 4, 8, 11, 0x100000, 0xffffffff, 0x80000000])
    nrec = r.choice([8, 12])
    keys = [r.choice([0, 1, 2, 3, 0x1234, 0xffff, r.getrandbits(16)]) for _ in range(8)]
    kbuf = ctx.buf(16, struct.pack('<8H', *keys))
    recs = bytearray(r.getrandbits(8) for _ in range(0x1e * nrec))
    ids = [r.choice([0, 1, 2, 3, 0x1234, 0xffff, r.getrandbits(16)]) for _ in range(nrec)]
    for i in range(nrec):
        recs[i * 0x1e + 0x1a:i * 0x1e + 0x1c] = struct.pack('<H', ids[i])
    rbuf = ctx.buf(len(recs), bytes(recs))
    rt = bytearray(0x140)
    rt[0x12:0x16] = struct.pack('<I', count if count < 13 or r.random() < 0.1 else r.choice([nrec, 3]))
    rt[0x16:0x1a] = struct.pack('<I', kbuf)
    rt[0x126:0x12a] = struct.pack('<I', rbuf)
    rb = ctx.buf(0x140, bytes(rt))
    w32(ctx, 0x1015d004, rb)
    w32(ctx, 0x1015ec84, r.choice([1, 1, 1, 0, 0xffffffff]))
    return keys, ids

def g_db56(ctx):
    keys, ids = root(ctx)
    r = ctx.rng
    return [r.choice(keys + [0, 1, 0x7fff, 0x8000, 0xffff, r.getrandbits(32)]) | (r.choice([0, 0, 0x10000, 0xabcd0000]))]
def g_dbc0(ctx):
    keys, ids = root(ctx)
    r = ctx.rng
    return [r.choice(ids + [0, 1, 0x10001, 0xffff, ctx.u32()])]

# ---- single-variable setters ----
def g_3217c(ctx):
    w32(ctx, 0x1015dd6c, ctx.u32()); return []
def g_4855(ctx):
    w32(ctx, 0x1015e4f8, ctx.u32())
    return [ctx.rng.choice([0, 1, 2, 3, 4, 5, -1, 0x80000000, 0x7fffffff, ctx.rng.randrange(-3, 8)]) & 0xffffffff]
def g_a52b(ctx):
    r = ctx.rng
    p = ctx.buf(0x40)
    w32(ctx, 0x1015d0a4, p)
    return [ctx.u32()]

def g_815f(ctx):
    ctx.m.uc.mem_write(0x109e38c0, bytes(ctx.rng.getrandbits(8) for _ in range(0x30)))
    return [ctx.u32(), ctx.rng.choice([0, 1, 200, -200 & 0xffffffff, 0x80000000, ctx.u32()])]

def g_c7b0(ctx):
    r = ctx.rng
    n = r.choice([0, 1, 2, 70, 71, 200, 512, r.randrange(0, 600)])
    return [ctx.buf(max(n, 1) + 8), n, r.choice([0, 1, 69, 70, 71, 0xffffffff, ctx.u32(), r.randrange(0, 500)])]

SPECS = [
    dict(name='Sim_1003327f', orig=0x1003327f, c='Sim_1003327f', gen=g_mask, void=True),
    dict(name='Sim_10033291', orig=0x10033291, c='Sim_10033291', gen=g_mask),
    dict(name='Sim_100332a3', orig=0x100332a3, c='Sim_100332a3', gen=g_mask_on, void=True),
    dict(name='Sim_100332d2', orig=0x100332d2, c='Sim_100332d2', gen=g_none),
    dict(name='Sim_1003356a', orig=0x1003356a, c='Sim_1003356a', gen=g_entry),
    dict(name='Sim_10033580', orig=0x10033580, c='Sim_10033580', gen=g_entry),
    dict(name='Sim_1003359f', orig=0x1003359f, c='Sim_1003359f', gen=g_entry2),
    dict(name='Sim_100335cf', orig=0x100335cf, c='Sim_100335cf', gen=g_entry2),
    dict(name='Sim_10033c42', orig=0x10033c42, c='Sim_10033c42', gen=g_index44, trials=600),
    dict(name='Sim_1003acf3', orig=0x1003acf3, c='Sim_1003acf3', gen=g_acf3, void=True),
    dict(name='Sim_1003b417', orig=0x1003b417, c='Sim_1003b417', gen=g_b417, trials=500),
    dict(name='Sim_1003b485', orig=0x1003b485, c='Sim_1003b485', gen=g_b485, trials=500),
    dict(name='Sim_1003b50f', orig=0x1003b50f, c='Sim_1003b50f', gen=g_b50f, void=True),
    dict(name='Sim_1003db56', orig=0x1003db56, c='Sim_1003db56', gen=g_db56, trials=600),
    dict(name='Sim_1003dbc0', orig=0x1003dbc0, c='Sim_1003dbc0', gen=g_dbc0, trials=600),
    dict(name='Sim_1003217c', orig=0x1003217c, c='Sim_1003217c', gen=g_3217c, void=True),
    dict(name='Sim_10034855', orig=0x10034855, c='Sim_10034855', gen=g_4855),
    dict(name='Sim_1003a52b', orig=0x1003a52b, c='Sim_1003a52b', gen=g_a52b, void=True),
    dict(name='Dib_InitHeader', orig=0x1003815f, c='Dib_InitHeader', gen=g_815f, void=True),
    dict(name='Sim_1003c7b0', orig=0x1003c7b0, c='Sim_1003c7b0', gen=g_c7b0, void=True, trials=600),
]
