# Round 10: 0x1003aecb (switch a record's model level). Every callee is converted and stubbed on both sides with its
# arguments logged (the word arguments of Sim_10009408 / Sim_1000942e / Sim_1000941e masked to 16 bits): the loader
# 0x100ba7d5 (name logged as text, writes its count), Sim_100e2776, Sim_100e282a, Sim_100e2a31, Sim_100bb2b0,
# Sim_100a9830, Sim_100a9a1a, Sim_100096ee, Sim_100a9b4c, Sim_100096f9, Sim_10089e91, Sim_100aa4f0, Sim_100a9d47,
# Sim_1008a40a, Sim_1008a37f, Sim_1008a46e, Sim_1002d890, Sim_100093ea, Sim_100093be, Sim_100aa351, Sim_100aa406,
# Sim_100aa799. HeapAlloc hands out a block; Str_Length / Str_Copy run for real. Exact (eax compared).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w10_model.c'])
STUBS = ((0x100e2776, 'Sim_100e2776', 3), (0x100e282a, 'Sim_100e282a', 1), (0x100e2a31, 'Sim_100e2a31', 4),
         (0x100a9830, 'Sim_100a9830', 2), (0x100a9a1a, 'Sim_100a9a1a', 2), (0x100096ee, 'Sim_100096ee', 1),
         (0x100a9b4c, 'Sim_100a9b4c', 2), (0x100096f9, 'Sim_100096f9', 2), (0x10009408, 'Sim_10009408', 2),
         (0x1000942e, 'Sim_1000942e', 2), (0x1000941e, 'Sim_1000941e', 2), (0x10089e91, 'Sim_10089e91', 1),
         (0x100aa4f0, 'Sim_100aa4f0', 2), (0x100a9d47, 'Sim_100a9d47', 4), (0x1008a40a, 'Sim_1008a40a', 1),
         (0x1008a37f, 'Sim_1008a37f', 1), (0x1008a46e, 'Sim_1008a46e', 1), (0x1002d890, 'Sim_1002d890', 2),
         (0x100093ea, 'Sim_100093ea', 1), (0x100093be, 'Sim_100093be', 2), (0x100aa351, 'Sim_100aa351', 1),
         (0x100aa406, 'Sim_100aa406', 1), (0x100aa799, 'Sim_100aa799', 1))
for _a, _c, _n in STUBS:
    helper(_a, _c, _n)
helper(0x100ba7d5, None, 3, sargs=(0,))
helper(0x100bb2b0, 'Sim_100bb2b0', 2, sargs=(0,))
for _a in (0x10009408, 0x1000942e, 0x1000941e):
    MASK[(_a, 1)] = 0xffff
ACTIVE = tuple(a for a, _, _ in STUBS) + (0x100ba7d5, 0x100bb2b0)


def e_load(uc, k, raw, eax):
    uc.mem_write(raw[2], struct.pack('<I', ST['loaded']))


EFFECT[0x100ba7d5] = e_load


def heap(n, args):
    return ST['heap']


def model(ctx, r):
    m = newbuf(ctx, 0x40)
    w16(ctx, m + 2, r.choice([0x50, 0x70, 0xa0, 0x10, r.getrandbits(16)]))
    return m


def g(ctx):
    r = begin(ctx, *ACTIVE)
    ST['loaded'] = r.getrandbits(32)
    ST['heap'] = newbuf(ctx, 0x20)
    for a in (0x100a9830, 0x100a9a1a, 0x100e2a31):
        rets(a, r.choice([model(ctx, r), model(ctx, r), 0]))
    rets(0x100ba7d5, r.choice([0x7000, 0x7000, 0]))
    rets(0x100096ee, model(ctx, r))
    w32(ctx, 0x1014cedc, r.getrandbits(32))
    w32(ctx, 0x1017b8f8, r.getrandbits(32))
    for i in range(8):
        e = newbuf(ctx, 0x60)
        w16(ctx, e + 0x1c, r.choice([0, 0x4000, r.getrandbits(16)]))
        w32(ctx, e + 0x48, newbuf(ctx, 0x20))
        w32(ctx, 0x109e8b40 + 4 * i, e)
    for ri in range(4):
        rec = 0x109b9e20 + ri * 0x7e
        w32(ctx, rec, r.randrange(0, 8))
        w32(ctx, rec + 4, r.choice([-1, 0, 1, 2, 3]) & 0xffffffff)
        for lv in range(4):
            wb(ctx, rec + 8 + lv * 16, bytes(r.choice(b'MODELabc') for _ in range(r.randrange(1, 15))).ljust(16, b'\0'))
            w32(ctx, rec + 0x50 + lv * 4, r.choice([0, 0, model(ctx, r)]))
            w16(ctx, rec + 0x74 + lv * 2, r.choice([0x70, 0xa0, 0x10, 0x50, r.getrandbits(16)]))
        w32(ctx, rec + 0x48, r.choice([-2, -2, r.randrange(0, 4)]) & 0xffffffff)
        w16(ctx, rec + 0x4c, r.getrandbits(16))
        w16(ctx, rec, r.getrandbits(16) & 0xff07 if r.random() < 0.0 else r32(ctx, rec) & 0xffff)
        node = newbuf(ctx, 0xa0)
        w32(ctx, rec + 0x60, r.choice([0, node]))
        w32(ctx, rec + 0x64, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32))
        w16(ctx, rec + 0x7c, r.choice([0, 0, 0, 1]))
    return [r.randrange(0, 4), r.randrange(0, 4), r.choice([0, 0, r.getrandbits(32)])]


SPECS = [dict(name='Sim_1003aecb', orig=0x1003aecb, c='Sim_1003aecb', gen=g, trials=5000, api={'HeapAlloc': heap})]
