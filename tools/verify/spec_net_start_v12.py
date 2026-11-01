# Network round: the network game start (1.0 0x1003dc19, Sim_1003dc19) against 1.2's 0x10031389: 10 players in 1.2's
# own tables (ours: g_net10Heard / Mark / Ready / Left / Db20 / SnStamp / Row / DiSent; 1.0's 8-entry tables at
# 0x1015db00 ...), the whole 0x16-byte row cleared (1.0: 0x11 bytes from +5), the "DI" cache cleared and one more
# 0x100 block, the "DI" buffer (1.2 [0x10131504] = g_net10DiBuf) after the third. 1.2 also allocates its "AK" buffer
# [0x101314e8] after the fifth block; ours is static (round 33), so the heap stand-in hands 1.2 an extra block there
# and the rest in the same order as the C's (and never fails at that call). Sim_1003db56 is stubbed on both sides.
# Negative check: NET10_NEG12=1 builds the C with NET10_MUTATE (1.0's 0x11 bytes per row).
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w10_misc.c'])
if os.environ.get('NET10_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DNET10_MUTATE']
helper(0x1003db56, 'Sim_1003db56', 1)
MASK[(0x1003db56, 0)] = 0xffff

# 1.0 block -> 1.2 block, in the C's allocation order (0: the "DI" buffer)
BLOCKS = [(0x1015ec88, 0x101314d4), (0x1015ec90, 0x101314dc), (0x1015ecb0, 0x10131500), ('g_net10DiBuf', 0x10131504),
          (0x1015ec98, 0x101314e4), (0x1015d084, 0x107cabf8), (0x1015ec9c, 0x101314ec), (0x1015eca0, 0x101314f0),
          (0x1015eca4, 0x101314f4), (0x1015ecbc, 0x10131510), (0x1015eca8, 0x101314f8), (0x1015ecac, 0x101314fc),
          (0x1015ecb4, 0x10131508), (0x1015ecb8, 0x1013150c), (0x1015ecc0, 0x10131514), (0x1015ecc4, 0x10131518),
          (0x1015ecc8, 0x1013151c), (0x1015ec94, 0x101314e0), (0x1015d008, 0x107caa78)]
TABLES = [('g_net10Heard', 0x107cab60, 0x28), ('g_net10Mark', 0x107cb4d0, 10), ('g_net10Ready', 0x107cb430, 10),
          ('g_net10Left', 0x107cb440, 0x28), ('g_net10Db20', 0x107caba0, 0x28), ('g_net10SnStamp', 0x107cb4a0, 0x28),
          ('g_net10Row', 0x107caa80, 10 * 0x16), ('g_net10DiSent', 0x107cac00, 10 * 0xd1)]
OTHER = [(0x1015d004, 0x107caa74), (0x1015ec84, 0x101314d0), (0x1015ec58, 0x101314a0), (0x1017b94c, 0x1014a5f8),
         (0x1016552a, 0x1028cc2a), (0x1015dc64, 0x10130754), (0x1015d088, 0x107cb42c), (0x1015ec8c, 0x101314d8),
         (0x1015d08c, 0x107cb43c), (0x1017b8c8, 0x1014a568), (0x1015ecd8, 0x1013152c), (0x1013acac, 0x1011014c),
         (0x1015ecd0, 0x10131524), (0x1015dc84, 0x10130774), (0x1017b8f8, 0x1014a59c)]
ST = {}
# 1.0's 8-entry tables: no longer used by the C, so not paired with 1.2's (the listing pairs them)
OLD = tuple(range(0x1015d090, 0x1015d098)) + tuple(range(0x1015d1b0, 0x1015d1b8)) + tuple(range(0x1015da40, 0x1015dba0))


def _a(m, g):
    return m.syms[g] if isinstance(g, str) else g


def heap(n, args):
    if ST.get('in12'):
        if n == 5:
            return ST['ak']
        if n > 5:
            n -= 1
    if n == ST['fail']:
        return 0
    return ST['pool'][n % len(ST['pool'])]


def g(ctx):
    r = begin(ctx, 0x1003db56)
    rets(0x1003db56, r.choice([0, 3, 9, 0xffffffff]))
    ST['pool'] = [newbuf(ctx, 0x100) for _ in range(19)]
    ST['ak'] = newbuf(ctx, 0x100)
    ST['akdata'] = bytes(r.getrandbits(8) for _ in range(0x100))
    wb(ctx, ST['ak'], ST['akdata'])
    for p in ST['pool']:
        wb(ctx, p, bytes(r.getrandbits(8) for _ in range(0x100)))
    ST['fail'] = r.choice([99] * 8 + list(range(19)))
    w32(ctx, 0x1015ec58, r.choice([1, 1, 1, 0]))
    w32(ctx, 0x1017b8f8, r.getrandbits(32))
    for name, _, n in TABLES:
        wb(ctx, ctx.m.syms[name], bytes(r.getrandbits(8) for _ in range(n)))
    game = newbuf(ctx, 0x40)
    w16(ctx, game + 0x10, r.choice([0xfa01, r.randrange(0, 10), 3]))
    n = r.choice([0, 1, 4, 8, 10])
    w32(ctx, game + 0x12, n)
    ids = newbuf(ctx, 24)
    for k in range(10):
        w16(ctx, ids + 2 * k, r.choice([0xfa01, r.randrange(0, 10), 3]))
    w32(ctx, game + 0x16, ids)
    w32(ctx, 0x1015dc64, r.randrange(1, 11))
    if r.random() < 0.2:
        w32(ctx, 0x1015d004, game)
        game = 0
    return [game]


def to12_more(m):
    ST['in12'] = True
    for a10, a12 in BLOCKS:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(_a(m, a10), 4)))
    for name, a12, n in TABLES:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(m.syms[name], n)))
    for a10, a12 in OTHER:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, 4)))


def from12_more(m):
    ST['in12'] = False
    m.uc.mem_write(ST['ak'], ST['akdata'])     # 1.2's own "AK" block, which the C does not have
    for a10, a12 in BLOCKS:
        m.uc.mem_write(_a(m, a10), bytes(m.uc.mem_read(A12(a12), 4)))
    for name, a12, n in TABLES:
        m.uc.mem_write(m.syms[name], bytes(m.uc.mem_read(A12(a12), n)))
    for a10, a12 in OTHER:
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), 4)))


def log_map(log_o, log_c):
    """1.2's sixth HeapAlloc (its "AK" buffer) has no counterpart in the C"""
    out, k = [], 0
    for e in log_o:
        if e[0] == 'HeapAlloc':
            k += 1
            if k == 6:
                continue
        out.append(e)
    return out, log_c


SPECS = [v12_spec(dict(name='Sim_1003dc19', orig=0x1003dc19, c='Sim_1003dc19', gen=g, void=True, trials=3000,
                       api={'HeapAlloc': heap}, log_map=log_map), 0x10031389, to12_more=to12_more, from12_more=from12_more,
                  extra_pairs={0x1003db56: 0x100312c6},
                  skip=tuple(a for a, _ in BLOCKS + OTHER if isinstance(a, int)) + OLD,
                  name='Sim_1003dc19_v12')]
