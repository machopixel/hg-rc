# Network round: "NR", the game settings (Sim_10040eb3, w3p_net.c) against 1.2's 0x10034b31: the own state first
# (Net_SendStateMessage = 1.2 0x10032782, a logging stub on both sides) and the two values sent kept where the loop
# compares them ([0x1013aca0] = 1.2 0x1011013c the +0x137 byte, [0x1013aca4] = 0x10110140 the +0x138 byte; 1.0 had
# them crossed). The send (Net_DpSend_1007e920 = 0x1006a860) logs its 6 bytes.
# Negative check: NETNR_NEG12=1 builds the C with NETNR_MUTATE (1.0's).
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1004/w3p_net.c'])
if os.environ.get('NETNR_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DNETNR_MUTATE']
CALLEES = [(0x1003eebc, 'Net_SendStateMessage', 2, 0x10032782), (0x1007e920, 'Net_DpSend_1007e920', 3, 0x1006a860)]
for _a, _c, _n, _a12 in CALLEES:
    helper(_a, _c, _n, deref={0: 6} if _a == 0x1007e920 else None)
ACTIVE = tuple(c[0] for c in CALLEES)
PAIRS = {0x1017b8c8: 0x1014a568, 0x1015d004: 0x107caa74, 0x1015ecc0: 0x10131514, 0x1013aca0: 0x1011013c,
         0x1013aca4: 0x10110140}


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, 0x1017b8c8, r.randrange(10))
    root = newbuf(ctx, 0x200)
    wb(ctx, root, bytes(r.getrandbits(8) for _ in range(0x200)))
    w32(ctx, 0x1015d004, root)
    w32(ctx, 0x1015ecc0, newbuf(ctx, 0x20))
    w32(ctx, 0x1013aca0, r.getrandbits(32))
    w32(ctx, 0x1013aca4, r.getrandbits(32))
    rets(0x1007e920, 1)
    return []


def to12(m):
    if not getattr(m, '_netnr', False):
        m._netnr = True
        for a10, _, _, a12 in CALLEES:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, a10), None, A12(a12), A12(a12))
    for a10, a12 in PAIRS.items():
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, 4)))


def from12(m):
    for a10, a12 in PAIRS.items():
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), 4)))


SPECS = [dict(name='Sim_10040eb3_v12', orig12=0x10034b31, c='Sim_10040eb3', gen=g, trials=1000, to12=to12, from12=from12)]
