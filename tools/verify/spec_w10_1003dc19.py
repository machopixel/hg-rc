# Round 10: 0x1003dc19 (network game start: score blocks). HeapAlloc hands out blocks of the generator (one may fail);
# Mem_Fill runs for real; Sim_1003db56 is stubbed on both sides (its word argument masked to 16 bits). Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w10_misc.c'])
helper(0x1003db56, 'Sim_1003db56', 1)
MASK[(0x1003db56, 0)] = 0xffff


def heap(n, args):
    if n == ST['fail']:
        return 0
    return ST['pool'][n % len(ST['pool'])]


def g(ctx):
    r = begin(ctx, 0x1003db56)
    rets(0x1003db56, r.choice([0, 3, 0xffffffff]))
    ST['pool'] = [newbuf(ctx, 0x100) for _ in range(18)]
    for p in ST['pool']:
        wb(ctx, p, bytes(r.getrandbits(8) for _ in range(0x100)))
    ST['fail'] = r.choice([99] * 6 + list(range(18)))
    w32(ctx, 0x1015ec58, r.choice([1, 1, 1, 0]))
    w32(ctx, 0x1017b8f8, r.getrandbits(32))
    for a in range(0x1015da45, 0x1015da45 + 8 * 0x16, 4):
        w32(ctx, a, r.getrandbits(32))
    game = newbuf(ctx, 0x40)
    w16(ctx, game + 0x10, r.choice([0xfa01, r.randrange(0, 8), 3]))
    n = r.choice([0, 1, 4, 8])
    w32(ctx, game + 0x12, n)
    ids = newbuf(ctx, 16)
    for k in range(8):
        w16(ctx, ids + 2 * k, r.choice([0xfa01, r.randrange(0, 8), 3]))
    w32(ctx, game + 0x16, ids)
    if r.random() < 0.2:
        w32(ctx, 0x1015d004, game)
        game = 0
    return [game]


SPECS = [dict(name='Sim_1003dc19', orig=0x1003dc19, c='Sim_1003dc19', gen=g, void=True, trials=3000, api={'HeapAlloc': heap})]
