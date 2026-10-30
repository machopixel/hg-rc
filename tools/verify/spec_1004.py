# Differential tests for src/sim/unsorted_1004 (see difftest.py).
import struct
SOURCES = ['sim/unsorted_1004/state.c', 'sim/unsorted_1004/fields.c', 'sim/unsorted_1004/tables.c']

def fill(ctx, addr, size, special=()):
    """Random bytes over a table, with some small or negative dwords mixed in."""
    rng = ctx.rng
    words = []
    for _ in range(size // 4):
        words.append(rng.choice([rng.getrandbits(32), rng.randrange(0, 40), rng.randrange(0, 40),
                                 0xFFFFFFFF, 0x80000000, rng.randrange(-5, 0) & 0xffffffff]))
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(words), *words))

def word(ctx):
    r = ctx.rng.random()
    if r < 0.15: return ctx.rng.choice([0, 1, 0xf, 0xfffff, 0x80000000, 0xffffffff, 0x7fffffff])
    if r < 0.5: return ctx.rng.getrandbits(20)
    return ctx.rng.getrandbits(32)

def g_word(ctx): return [word(ctx)]

def g_mode(ctx):
    rng = ctx.rng
    ctx.m.uc.mem_write(0x10179cd4, struct.pack('<I', rng.choice([0, 0, 1, rng.getrandbits(32)])))
    ctx.m.uc.mem_write(0x1013aee0, struct.pack('<I', rng.choice([0xffffffff, 0xffffffff, 0, 1, 2, rng.getrandbits(32)])))
    ctx.m.uc.mem_write(0x1015ef04, struct.pack('<I', rng.getrandbits(32)))
    return [ctx.u32()]

def g_get(ctx):
    ctx.m.uc.mem_write(0x1015ef04, struct.pack('<I', ctx.u32()))
    return []

def g_copy(ctx):
    rng = ctx.rng
    src = ctx.buf(32)
    dst = ctx.buf(32)
    r = rng.random()
    if r < 0.1: return [0, dst]
    if r < 0.2: return [src, 0]
    if r < 0.25: return [0, 0]
    if r < 0.4:  # overlapping records
        return [src, src + rng.choice([4, 8, 12, 20, -4 + 8, 0])]
    return [src, dst]

def g_t4d330(ctx):
    fill(ctx, 0x101116b2 - 0x20, 0xaa * 40 + 0x40)
    return [ctx.rng.choice([0, 1, 2, 5, 39, ctx.rng.randrange(-3, 40)])]

def g_t52aaf(ctx):
    fill(ctx, 0x109b7d40 - 0x6c * 4, 0x6c * 24)
    return [ctx.rng.choice([0, 1, 15, 16, 17, -1, -3, ctx.rng.randrange(-4, 20)])]

def g_t52be7(ctx):
    rng = ctx.rng
    fill(ctx, 0x109b7d54 - 0x40, 0x6c * 20)
    ptrs = []
    for _ in range(24):
        b = ctx.buf(32)
        ctx.m.uc.mem_write(b + 8, struct.pack('<I', rng.choice([0, 1, 2, 7, 15, rng.randrange(0, 18)])))
        ptrs.append(0 if rng.random() < 0.04 else b)
    ctx.m.uc.mem_write(0x109e8b40, struct.pack('<24I', *ptrs))
    return [rng.randrange(0, 24)]

def g_t52c02(ctx):
    rng = ctx.rng
    ents = [rng.choice([0xffffffff, 0x80000000, rng.randrange(0, 30), rng.randrange(0, 30), rng.getrandbits(32) & 0x1ff])
            for _ in range(20)]
    for i, e in enumerate(ents):
        ctx.m.uc.mem_write(0x109e9bb0 + i * 0x50, struct.pack('<I', e))
    fill(ctx, 0x109b8400, 4 * 520)
    return [rng.choice([0, 1, 19, rng.randrange(0, 20)])]

def spec(name, addr, gen, trials=400, **kw):
    return dict(name=name, orig=addr, c=name, gen=gen, trials=trials, **kw)

SPECS = [
    spec('Sim_100448cd', 0x100448cd, g_mode, void=True),
    spec('Sim_100448ff', 0x100448ff, g_get),
    spec('Sim_10044992', 0x10044992, g_copy, trials=600),
]
for n, a in [('1004bf79', 0x1004bf79), ('1004bf87', 0x1004bf87), ('1004bf98', 0x1004bf98),
             ('1004c30d', 0x1004c30d), ('1004c565', 0x1004c565), ('1004c573', 0x1004c573),
             ('1004c584', 0x1004c584), ('1004c595', 0x1004c595), ('1004c5a6', 0x1004c5a6),
             ('1004c909', 0x1004c909), ('1004c917', 0x1004c917), ('1004c928', 0x1004c928),
             ('1004c939', 0x1004c939), ('1004c94a', 0x1004c94a), ('1004cd8d', 0x1004cd8d),
             ('1004cd9e', 0x1004cd9e), ('1004cdd1', 0x1004cdd1), ('1004cde2', 0x1004cde2)]:
    SPECS.append(spec('Sim_' + n, a, g_word))
SPECS += [
    spec('Sim_1004d330', 0x1004d330, g_t4d330),
    spec('Sim_10052aaf', 0x10052aaf, g_t52aaf),
    spec('Sim_10052be7', 0x10052be7, g_t52be7),
    spec('Sim_10052c02', 0x10052c02, g_t52c02),
]
