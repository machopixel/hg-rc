# Round 10: 0x10096ac3 (makes a moving effect object). Stubbed on both sides: Sim_10090c7d (answers the object
# made by the generator, or 0), Sim_100aa39f, Sim_100a9d47 (4 arguments logged), Sim_100aa799. Sim_10084a94 (length,
# CRT sqrt) runs for real. Exact (no tolerance).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1009/w10_misc.c'])
helper(0x10090c7d, 'Sim_10090c7d', 4)
helper(0x100aa39f, 'Sim_100aa39f', 1)
helper(0x100a9d47, 'Sim_100a9d47', 4)
helper(0x100aa799, 'Sim_100aa799', 1)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, 0x10090c7d, 0x100aa39f, 0x100a9d47, 0x100aa799)
    n = r.randrange(0, 50)
    w32(ctx, 0x1017cc24, r.choice([n, 0x7fffffff, 0xffffffff, n]))
    w32(ctx, 0x1017cc20, r.choice([n + 1, n + 2, n + 20, n, 0x80000000]))
    obj = newbuf(ctx, 0x580)
    node = newbuf(ctx, 0x100)
    rec = newbuf(ctx, 0x60)
    w32(ctx, obj + 0x552, node)
    w32(ctx, node + 0x8c, rec)
    rets(0x10090c7d, obj if r.random() < 0.9 else 0)
    v = [fb(r.uniform(-500, 500)) if r.random() < 0.9 else rfloat(r) for _ in range(3)]
    return [r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)] + v


SPECS = [dict(name='Sim_10096ac3', orig=0x10096ac3, c='Sim_10096ac3', gen=g, void=True, trials=2000)]
