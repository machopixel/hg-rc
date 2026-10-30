# Round 10: 0x100c97c0 (flight time, st(0) through the trampolines). Game logic: exact, no tolerance.
# Sim_100d6ea7 (aim direction) is stubbed on both sides and writes the same three floats through its pointers;
# Sim_10084c95 (normalise, CRT sqrt), Sim_100a9cfb and the CRT sqrt 0x100fca04 run for real.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100c/w10_misc.c'])
helper(0x100d6ea7, 'Sim_100d6ea7', 5)


def e_dir(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[1 + i], struct.pack('<I', ST['dir'][i]))
    return None


EFFECT[0x100d6ea7] = e_dir


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, 0x100d6ea7)
    st0(ctx, 0x100c97c0, 'Sim_100c97c0', 2)
    ST['dir'] = [fb(r.uniform(-1, 1)) for _ in range(3)] if r.random() < 0.9 else [rfloat(r) for _ in range(3)]
    if r.random() < 0.05:
        ST['dir'] = [0, 0, 0]
    obj = newbuf(ctx, 0x100)
    rec = newbuf(ctx, 0x4c0)
    w32(ctx, obj + 0x28, rec)
    wf(ctx, obj + 0x7c, r.uniform(-300, 300))
    wpn = newbuf(ctx, 0x120)
    kind = r.randrange(0, 40)
    w32(ctx, wpn + 4, kind)
    slot = r.randrange(0, 6)
    w32(ctx, wpn + 0x11c, slot)
    node = 0
    if r.random() < 0.92:
        node = ctx.buf(0x100, b''.join(struct.pack('<I', fb(r.uniform(-500, 500))) for _ in range(0x40)))
    w32(ctx, rec + 0x498 + 4 * slot, node)
    k = 0x10145508 + kind * 0xd6
    wf(ctx, k + 0x3c, r.choice([r.uniform(0, 2000), r.uniform(0, 50), 0.0]))
    if r.random() < 0.05:
        w32(ctx, k + 0x3c, rfloat(r))
    w32(ctx, k + 0x38, r.choice([1, 2, 5, 10, 0, -1, r.randrange(-20, 100)]) & 0xffffffff)
    if r.random() < 0.1:
        w32(ctx, 0x1013abc0, r.choice([fb(9.8), fb(0.0296), 0, rfloat(r)]))
    return [obj, wpn]


SPECS = [dict(name='Sim_100c97c0', orig=TRAMP_O, c='w10_tramp_c', gen=g, trials=4000)]
