# Round 10: 0x100d965e (an effect object for a slot of owner). Stubbed on both sides: SlotPool_Take (answers the
# slot made by the generator, or 0), SlotPool_Release, Sim_10090c7d (the object, or 0), Sim_1009c929 (random:
# the answer follows the call number), Sim_100aa39f, Sim_100a9d47, Sim_100aa799. Sim_10084a94 (length, CRT sqrt)
# runs for real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100d/w10_misc.c'])
helper(0x100d7ee0, 'SlotPool_Take', 1)
helper(0x100d7f41, 'SlotPool_Release', 2)
helper(0x10090c7d, 'Sim_10090c7d', 4)
helper(0x1009c929, 'Sim_1009c929', 1)
helper(0x100aa39f, 'Sim_100aa39f', 1)
helper(0x100a9d47, 'Sim_100a9d47', 4)
helper(0x100aa799, 'Sim_100aa799', 1)


def e_rand(uc, k, raw, eax):
    m = raw[0] if raw[0] else 1
    return (k * 2654435761 + ST['seed']) % m


EFFECT[0x1009c929] = e_rand


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, 0x100d7ee0, 0x100d7f41, 0x10090c7d, 0x1009c929, 0x100aa39f, 0x100a9d47, 0x100aa799)
    ST['seed'] = r.getrandbits(16)
    cam = ctx.buf(0x48, b''.join(struct.pack('<I', fb(r.uniform(-1000, 1000))) for _ in range(18)))
    wf(ctx, cam + 0x40, r.choice([r.uniform(0, 3000), 1e9, 0.0]))
    w32(ctx, 0x1014cae0, cam)
    slot = newbuf(ctx, 0x20)
    obj = newbuf(ctx, 0x580)
    node = newbuf(ctx, 0x100)
    rec = newbuf(ctx, 0x60)
    w32(ctx, obj + 0x552, node)
    w32(ctx, node + 0x8c, rec)
    rets(0x100d7ee0, slot if r.random() < 0.9 else 0)
    rets(0x10090c7d, obj if r.random() < 0.9 else 0)
    p = [fb(r.uniform(-1000, 1000)) if r.random() < 0.95 else rfloat(r) for _ in range(3)]
    v = [fb(r.uniform(-300, 300)) if r.random() < 0.95 else rfloat(r) for _ in range(3)]
    return p + v + [r.getrandbits(32)]


SPECS = [dict(name='Sim_100d965e', orig=0x100d965e, c='Sim_100d965e', gen=g, void=True, trials=2000)]
