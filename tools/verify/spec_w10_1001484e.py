# Round 10: 0x1001484e (set an AI order, with the AI log line). Stubbed on both sides: Sim_1009c929, Sim_10052aaf,
# Sim_100ca350, Sim_10084cee (normalises: writes three floats, each logged), Sim_100c4035, Sim_10015a63, Sim_100157ca,
# Sim_10011b9e (an objective name of the generator; key masked to 16 bits) and the log routine Sim_1005f9aa (its text
# logged). The CRT sprintf (integers and strings only), strcpy, strlen, _itoa and sqrt run for real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1001/w10_orders.c'])
STUBS = ((0x1009c929, 'Sim_1009c929', 1), (0x10052aaf, 'Sim_10052aaf', 1), (0x100ca350, 'Sim_100ca350', 4),
         (0x10084cee, 'Sim_10084cee', 3), (0x100c4035, 'Sim_100c4035', 3), (0x10015a63, 'Sim_10015a63', 1),
         (0x100157ca, 'Sim_100157ca', 1), (0x10011b9e, 'Sim_10011b9e', 3))
for _a, _c, _n in STUBS:
    helper(_a, _c, _n)
helper(0x1005f9aa, 'Sim_1005f9aa', 1, sargs=(0,))
helper(0x10084cee, 'Sim_10084cee', 3, deref={0: 4, 1: 4, 2: 4})
MASK[(0x10011b9e, 0)] = 0xffff
MASK[(0x10011b9e, 2)] = 0xffff
ACTIVE = tuple(a for a, _, _ in STUBS) + (0x1005f9aa,)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_norm(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[i], struct.pack('<I', ST['n'][i]))


EFFECT[0x10084cee] = e_norm


def fl(r, sc):
    return fb(r.uniform(-sc, sc)) if r.random() < 0.97 else rfloat(r)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    ST['n'] = [fl(r, 1) for _ in range(3)]
    rets(0x1009c929, r.choice([0, 1, 5]))
    rets(0x10052aaf, r.randrange(0, 8))
    rets(0x100ca350, r.choice([-1 & 0xffffffff, 3, 0x1ffff, 7]))
    name = ctx.buf(16, bytes(r.choice(b'agp_myleader') for _ in range(r.randrange(1, 13))).ljust(16, b'\0'))
    rets(0x10011b9e, name)
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015dc78, r.choice([r.randrange(0, 1000000), r.randrange(0, 100)]))
    for i in range(8):
        e = newbuf(ctx, 0x80)
        w32(ctx, e + 8, r.randrange(0, 3))
        w32(ctx, e + 0x58, fl(r, 1000), fl(r, 1000), fl(r, 1000))
        w32(ctx, 0x109e8b40 + 4 * i, e)
    side = r.randrange(0, 3)
    w32(ctx, 0x104727a0 + 4 * side, r.randrange(0, 4))
    p = 0x1047383a + side * 0x4fc3a + r32(ctx, 0x104727a0 + 4 * side) * 0x13f
    w32(ctx, p + 2, r.choice([0x100, 0x100, 0x200]))
    w32(ctx, p + 0x6f, r.choice([0, 1, 5]))
    obj = newbuf(ctx, 0x260)
    wb(ctx, obj, bytes(r.getrandbits(8) for _ in range(0x260)))
    w32(ctx, obj, r.choice([1, 1, 2]))
    w32(ctx, obj + 4, r.randrange(0, 8))
    w32(ctx, obj + 8, side)
    w16(ctx, obj + 0x1c, r.getrandbits(16))
    rec = newbuf(ctx, 0x500)
    w32(ctx, rec, obj)
    w32(ctx, obj + 0x28, rec)
    w32(ctx, obj + 0x54, newbuf(ctx, 0x60))
    w32(ctx, obj + 0x58, fl(r, 1000), fl(r, 1000), fl(r, 1000))
    w16(ctx, obj + 0x1a6, r.choice([7, 7, 3, r.randrange(0, 16)]))
    w16(ctx, obj + 0x1a8, r.choice([0x100, 0x200, 0x301, r.getrandbits(16)]))
    w16(ctx, obj + 0x1aa, r.choice([0x200 | r.randrange(0, 8), 0x2201, 0x4200, 0x100 | 3, 0, r.getrandbits(16) & 0x9fff]))
    w32(ctx, obj + 0x1d6, r.choice([0x1000, 0, r.getrandbits(32)]))
    order = r.choice(list(range(17)) + [13, 13, 2, 2, 8, 0x10002, 0xffff])
    return [obj, order]


SPECS = [dict(name='Sim_1001484e', orig=0x1001484e, c='Sim_1001484e', gen=g, void=True, trials=4000)]
