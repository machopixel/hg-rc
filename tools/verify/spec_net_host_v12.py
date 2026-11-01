# Network round: the host change against 1.2, one part per run (NETHO_PART):
#   CB  the player list callback Sim_10042125 (was original code; __stdcall) = 1.2 0x10035f3e: 1.2 cancels a pending
#       request [0x1015f200] (1.2 0x1013196c) when the host is not the one before the change
#   HC  the host change Sim_100421b0 = 1.2 0x10035fe7: 1.2 keeps the host before the change (ours g_net10OldHost =
#       1.2 0x10110144). Its two session calls are stand-ins logging their arguments (the callback's
#       address, the C's or 1.2's, logged as "callback").
# Sim_1003db56 (= 1.2 0x100312c6) is a logging stub on both sides.
# Negative check: NETHO_NEG12=1 builds the C with NETHOST_MUTATE (no cancel) - for CB.
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

PART = os.environ.get('NETHO_PART', 'CB')
SOURCES = closure(['sim/unsorted_1004/w3p_net.c'])
if os.environ.get('NETHO_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DNETHOST_MUTATE']
ENUM = 0x1010d3f0
COUNT = 0x1010d400
helper(0x1003db56, 'Sim_1003db56', 1)
helper(ENUM, None, 5)
helper(COUNT, None, 1)
MASK[(0x1003db56, 0)] = 0xffff
ACTIVE = (0x1003db56, ENUM, COUNT)
PAIRS = {0x1015dc7c: 0x1013076c, 0x1015ecd8: 0x1013152c, 0x1013acb0: 0x10110150, 0x1015f200: 0x1013196c,
         0x1013acac: 0x1011014c, 0x1015ecd4: 0x10131528, 0x1015eae0: 0x10131374, 0x1015ead8: 0x1013136c,
         0x1015eccc: 0x10131520, 0x1015ecd0: 0x10131524, 0x1015dc84: 0x10130774, 0x1015ec7c: 0x101314c8}
TABLES = [('g_net10Heard', 0x107cab60, 0x28), ('g_net10Left', 0x107cb440, 0x28)]
ST = {}


def g(ctx):
    r = begin(ctx, *ACTIVE)
    for a in PAIRS:
        w32(ctx, a, r.getrandbits(32))
    w16(ctx, 0x1015ecd8, r.randrange(0, 20))
    w16(ctx, 0x1013acb0, r.randrange(0, 20))
    w16(ctx, 0x1013acac, r.randrange(0, 20))
    w32(ctx, 0x1015eae0, ENUM)
    w32(ctx, 0x1015ead8, COUNT)
    for name, _, n in TABLES:
        wb(ctx, ctx.m.syms[name], bytes(r.choice([0, 0, 1]) if (k % 4 == 0) else 0 for k in range(n)))
    rets(0x1003db56, r.choice([-1 & 0xffffffff, r.randrange(10), r.randrange(10)]))
    rets(COUNT, r.choice([1, 2, 3]))
    # the host before the change (both sides keep it themselves; set alike here)
    ST['old'] = r.randrange(0, 20)
    if PART == 'CB':
        return [r.randrange(0, 20), r.choice([0, 1, 1]), r.getrandbits(32), r.getrandbits(32)]
    return [r.randrange(0, 20)]


def to12(m):
    if not getattr(m, '_netho', False):
        m._netho = True
        m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, 0x1003db56), None, A12(0x100312c6), A12(0x100312c6))
    for a10, a12 in PAIRS.items():
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, 4)))
    for name, a12, n in TABLES:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(m.syms[name], n)))
    m.uc.mem_write(A12(0x10110144), struct.pack('<I', ST['old']))
    m.uc.mem_write(A12(0x10131374), struct.pack('<I', ENUM))
    m.uc.mem_write(A12(0x1013136c), struct.pack('<I', COUNT))


def from12(m):
    for a10, a12 in PAIRS.items():
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), 4)))
    m.uc.mem_write(m.syms['g_net10OldHost'], bytes(m.uc.mem_read(A12(0x10110144), 4)))
    for name, a12, n in TABLES:
        m.uc.mem_write(m.syms[name], bytes(m.uc.mem_read(A12(a12), n)))


def g_c(ctx):
    a = g(ctx)
    # the C keeps the old host in a static: set it the same way for the C's run
    w32(ctx, ctx.m.syms['g_net10OldHost'], ST['old'])
    return a


def log_map(log_o, log_c):
    def fix(log, cb):
        return [(e[0], tuple('callback' if v == cb else v for v in e[1])) if isinstance(e[1], tuple) else e
                for e in log]
    return fix(log_o, A12(0x10035f3e)), fix(log_c, ST.get('cb'))


def g_hc(ctx):
    ST['cb'] = ctx.m.syms['Sim_10042125@16'] if 'Sim_10042125@16' in ctx.m.syms else ctx.m.syms.get('Sim_10042125')
    return g_c(ctx)


if PART == 'CB':
    SPECS = [dict(name='Sim_10042125_v12', orig12=0x10035f3e, c='Sim_10042125@16', gen=g_c, trials=3000, void=True,
                  to12=to12, from12=from12)]
else:
    SPECS = [dict(name='Sim_100421b0_v12', orig12=0x10035fe7, c='Sim_100421b0', gen=g_hc, trials=3000, void=True,
                  to12=to12, from12=from12, log_map=log_map)]
