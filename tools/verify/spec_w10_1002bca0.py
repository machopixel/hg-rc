# Round 10: 0x1002bca0 (cockpit setup). HeapAlloc (the import 0x10ac5658) hands out blocks of the generator in turn.
# Stubbed on both sides: Sim_10028810 (fills a panel's routine slots with the indirect stubs below), Sim_1002b68f,
# Sim_10050c5b, Sim_10077e34, Sim_10062a3a, Sim_10099928 and 0x1008cdc9 (both dwords logged, the second written).
# The panel routines are logging stubs: IND1 +0x10c, IND2 +0x118, IND3 +0x120, IND4 +0x124, IND5 +0x11c,
# IND6 +0x130, IND7 +0x134. Exact.
exec(open('tools/verify/w10_common.py').read())

IND7 = 0x00F00480
SOURCES = closure(['sim/unsorted_1002/w10_misc.c'])
for _a, _c, _n in ((0x10028810, 'Sim_10028810', 1), (0x1002b68f, 'Sim_1002b68f', 1), (0x10050c5b, 'Sim_10050c5b', 0),
                   (0x10077e34, 'Sim_10077e34', 0), (0x10062a3a, 'Sim_10062a3a', 0), (0x10099928, 'Sim_10099928', 0)):
    helper(_a, _c, _n)
helper(0x1008cdc9, None, 3, deref={1: 4, 2: 4})
for _a in (IND1, IND2, IND3, IND4, IND5):
    helper(_a, None, 2)
helper(IND6, None, 1)
helper(IND7, None, 1)
SLOTS = ((0x10c, IND1), (0x118, IND2), (0x120, IND3), (0x124, IND4), (0x11c, IND5), (0x130, IND6), (0x134, IND7))


def e_init(uc, k, raw, eax):
    for off, a in SLOTS:
        uc.mem_write(raw[0] + off, struct.pack('<I', a))


def e_read(uc, k, raw, eax):
    uc.mem_write(raw[2], struct.pack('<I', ST['val']))


EFFECT.update({0x10028810: e_init, 0x1008cdc9: e_read})


def heap(n, args):
    pool = ST['pool']
    return pool[n % len(pool)]


def g(ctx):
    r = begin(ctx, 0x10028810, 0x1002b68f, 0x10050c5b, 0x10077e34, 0x10062a3a, 0x10099928, 0x1008cdc9,
              IND1, IND2, IND3, IND4, IND5, IND6, IND7)
    ST['pool'] = [newbuf(ctx, 0x148) for _ in range(31)]
    ST['val'] = r.getrandbits(32)
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    unit = newbuf(ctx, 0x40)
    w32(ctx, unit + 0x28, r.getrandbits(32))
    w32(ctx, 0x109e8b40 + 4 * self_, unit)
    rp = newbuf(ctx, 0x10)
    w8(ctx, rp + 9, r.choice([0, 1]))
    w32(ctx, 0x1017b8c4, rp)
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    w32(ctx, 0x1017b8f8, r.getrandbits(32))
    w32(ctx, 0x109e3e1c, r.choice([r.getrandbits(32), r.randrange(0, 2000), 0x7fff, 0x8000, 0xffff]))
    for i in range(30):
        w32(ctx, 0x101381d0 + 4 * i, r.getrandbits(32))
        w32(ctx, 0x10138158 + 4 * i, r.getrandbits(32))
        w32(ctx, 0x109e8960 + 4 * i, r.getrandbits(32))
    return []


SPECS = [dict(name='Sim_1002bca0', orig=0x1002bca0, c='Sim_1002bca0', gen=g, void=True, trials=1500,
              api={'HeapAlloc': heap})]
