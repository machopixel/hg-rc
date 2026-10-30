# Differential tests for src/sim/unsorted_100d (see difftest.py).
import struct
SOURCES = ['sim/unsorted_100d/nodes.c', 'sim/unsorted_100d/flags.c', 'sim/unsorted_100d/state.c']

def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))

def rnd(ctx, n=5):
    return [ctx.rng.getrandbits(32) for _ in range(n)]

# --- List_Unlink: a list of up to 4 nodes (+4 next, +8 prev, +0x14 owner), the argument is one of them
def g_unlink(ctx):
    rng = ctx.rng
    owner = ctx.buf(0x40)
    n = rng.randrange(1, 5)
    nodes = [ctx.buf(0x40) for _ in range(n)]
    for i, a in enumerate(nodes):
        nxt = nodes[i + 1] if i + 1 < n else 0
        prv = nodes[i - 1] if i > 0 else 0
        w32(ctx, a + 4, nxt, prv)
        w32(ctx, a + 0x14, owner if rng.random() < 0.9 else 0)
    w32(ctx, owner + 0xc, nodes[-1])
    w32(ctx, owner + 0x10, nodes[0])
    if rng.random() < 0.1:   # a corrupt link, as a stray write could leave
        w32(ctx, nodes[0] + 8, rng.choice([0, 0x1000, owner]))
    return [rng.choice(nodes)]

# --- pool: owner+0x750 capacity, +0x754 depth, +0x758 stack of entry numbers, entries of 0x24 at +0xfc
def pool(ctx):
    rng = ctx.rng
    o = ctx.buf(0x1000)
    cap = rng.choice([0, 5, 45, 45, rng.randrange(0, 60)])
    depth = rng.choice([0, 1, 44, 45, 46, 0xffffffff, rng.randrange(0, 45), rng.randrange(0, 45)])
    w32(ctx, o + 0x750, cap, depth)
    w32(ctx, o + 0x758, *[rng.randrange(0, 45) for _ in range(46)])
    return o

def g_take(ctx):
    o = pool(ctx)
    return [o if ctx.rng.random() < 0.95 else 0]

def g_release(ctx):
    rng = ctx.rng
    o = pool(ctx)
    idx = rng.choice([0, 1, 2, 44, 45, 0xffffffff, rng.randrange(0, 50), rng.randrange(0, 50)])
    return [o if rng.random() < 0.95 else 0, idx]

# --- block reader
def g_block(ctx):
    rng = ctx.rng
    b = ctx.buf(0x100)
    size = rng.choice([0x10, 0x14, 0x20, 0x30, 0x7c, rng.randrange(0x10, 0xf0)])
    # sizes below 0x10 make the original copy ~2^30 words until it faults; the C faults sooner, so they are not compared
    w32(ctx, b + 4, size)
    outs = [ctx.buf(0x10) if rng.random() < 0.7 else 0 for _ in range(2)]
    payload = ctx.buf(0x400) if rng.random() < 0.8 else 0
    return [b] + outs + [payload]

# --- flags
def g_key(ctx):
    return [ctx.rng.choice([0x1b, 0x2a, 0x29, 0x1a, 0x2b, 0, 0xffffffff, ctx.rng.getrandbits(8), ctx.rng.getrandbits(32)])]

def g_flags(ctx):
    rng = ctx.rng
    return [rng.choice([0, 1, 2, 3, 0x10, 0x20, 0x100, 0x400, 0x730, 0x700, 0x4, 0x8000, rng.getrandbits(32), rng.getrandbits(12), rng.getrandbits(32) & 0xfffff8cc])]

def g_mode(ctx):
    w32(ctx, 0x10152418, ctx.rng.choice([0, 1, 1, 2, 3, 0xffffffff]))
    return [ctx.u32()]   # the original reads one argument slot it never uses

def g_none(ctx):
    return []

# --- state
def g_copy(ctx):
    for a in (0x101c457c, 0x101c49ac, 0x101c47c8, 0x101c4854, 0x101c49e8, 0x101c4680, 0x10151f8c, 0x10151fa4, 0x10151fb8, 0x10151fcc, 0x10151fe0, 0x10151ff4):
        w32(ctx, a, ctx.rng.getrandbits(32))
    return []

def g_setobj(ctx):
    w32(ctx, 0x10185c50, ctx.rng.getrandbits(32))
    w32(ctx, 0x10185bf8, ctx.rng.getrandbits(32))
    o = ctx.buf(0x80)
    return [o if ctx.rng.random() < 0.97 else 0, ctx.u32()]

def g_preset(ctx):
    rng = ctx.rng
    view = ctx.buf(0x80)
    w32(ctx, 0x1014cae0, view if rng.random() < 0.97 else 0)
    w32(ctx, 0x10185400, *rnd(ctx, 55))
    cur = rng.choice([0xffffffff, 0, 3, 10, rng.randrange(0, 11)])
    w32(ctx, 0x1015241c, cur)
    w32(ctx, 0x1013a014, rng.choice([0, 1]))
    p = rng.choice([cur, cur, -1, 11, 12, 0, 5, 10, 0x80000000, 0x7fffffff, rng.randrange(-2, 14)])
    return [p]

def g_clear(ctx):
    rng = ctx.rng
    o = ctx.buf(0xc40)
    rows = ctx.buf(8 * 0x128)
    w32(ctx, o + 0x84, rows)
    for i in range(8):
        w32(ctx, rows + i * 0x128 + 8, rng.choice([0, 1, 2, 3, 4, rng.getrandbits(32)]))
    w32(ctx, 0x101853c0, *rnd(ctx, 8))
    return [o]

def g_row(ctx):
    rng = ctx.rng
    idx = rng.randrange(0, 4)
    w32(ctx, 0x1017b8c8, idx)
    a = ctx.buf(0x40)
    b = ctx.buf(0x600)
    rows = ctx.buf(4 * 0x128)
    w32(ctx, 0x109e8b40, *[ctx.buf(0x40) for _ in range(idx)], a)  # slots before idx get dummies
    w32(ctx, 0x109e8b40 + idx * 4, a)
    w32(ctx, a + 0x28, b)
    w32(ctx, b + 0x84, rows)
    w32(ctx, b + 0x4fc, rng.randrange(0, 4))
    return [ctx.u32()]

def g_off(ctx):
    w32(ctx, 0x1017e6a8, ctx.u32())
    return [ctx.u32()]

def g_val(ctx):
    return [ctx.u32()]

SPECS = [
    dict(name='List_Unlink', orig=0x100d1562, c='List_Unlink', gen=g_unlink, void=True, trials=600),
    dict(name='SlotPool_Take', orig=0x100d7ee0, c='SlotPool_Take', gen=g_take, trials=600),
    dict(name='SlotPool_Release', orig=0x100d7f41, c='SlotPool_Release', gen=g_release, void=True, trials=600),
    dict(name='Sim_100da020', orig=0x100da020, c='Sim_100da020', gen=g_block, void=True, trials=500),
    dict(name='Sim_100d5c9b', orig=0x100d5c9b, c='Sim_100d5c9b', gen=g_key),
    dict(name='Sim_100dcbe0', orig=0x100dcbe0, c='Sim_100dcbe0', gen=g_flags, trials=500),
    dict(name='Sim_100d2c6f', orig=0x100d2c6f, c='Sim_100d2c6f', gen=g_mode),
    dict(name='Sim_100d1df9', orig=0x100d1df9, c='Sim_100d1df9', gen=g_none),
    dict(name='Sim_100d1cce', orig=0x100d1cce, c='Sim_100d1cce', gen=g_copy, void=True),
    dict(name='Sim_100d252c', orig=0x100d252c, c='Sim_100d252c', gen=g_setobj, void=True),
    dict(name='Sim_100d2545', orig=0x100d2545, c='Sim_100d2545', gen=g_setobj, void=True),
    dict(name='Sim_100d32e5', orig=0x100d32e5, c='Sim_100d32e5', gen=g_preset, void=True, trials=600),
    dict(name='Sim_100d4500', orig=0x100d4500, c='Sim_100d4500', gen=g_clear, void=True),
    dict(name='Sim_100d6392', orig=0x100d6392, c='Sim_100d6392', gen=g_row, void=True),
    dict(name='Sim_100db770', orig=0x100db770, c='Sim_100db770', gen=g_off),
    dict(name='Sim_100db77e', orig=0x100db77e, c='Sim_100db77e', gen=g_val, void=True),
    dict(name='Sim_100e282a', orig=0x100e282a, c='Sim_100e282a', gen=g_val, void=True),
]
