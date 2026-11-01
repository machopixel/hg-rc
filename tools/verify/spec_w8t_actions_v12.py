# Round 33: a unit's per-tick actions (1.0 0x10011681) against 1.2's 0x1000a811, on spec_w8t.py's generator and stubs
# (hooked at 1.2's callees too, logged under the 1.0 names). 1.2's one change: its action gate 0x10010cd2 (ours
# Sim12_10010cd2, proven by spec_w2h_gate_v12.py) in place of 1.0's early-out Sim_10017a05, with the opposite sense
# (0: stop). The gate is stubbed on both sides (its argument compared; the answer 0 or 1). The object and the
# player records hold their fields from +0x1c 4 bytes earlier in 1.2: shifted down by 4 for 1.2's run, back after.
# Negative check: W8T_NEG12=1 builds the C with W8TG_MUTATE (the gate's answer taken in 1.0's sense).
import os
exec(open('tools/verify/spec_w8t.py').read())
exec(open('tools/verify/v12_common.py').read())
_ns = {}
exec(open('tools/verify/w10_common.py').read(), _ns)
SOURCES = _ns['closure'](['sim/unsorted_1001/w8t_actions.c'])
if os.environ.get('W8T_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW8TG_MUTATE']

F10, F12 = 0x10011681, 0x1000a811
GATE12 = A12(0x10010cd2)
USED = (0x10017a05, 0x10052aaf, 0x10017263, 0x10052be7, 0x10015afd, 0x1000bf73, 0x100117f8, 0x1001403a, 0x100125a3)
HELPERS = {a: v for a, v in HELPERS.items() if a in USED}      # only this function's callees are linked
helper(GATE12, 'Sim12_10010cd2', 1)
PAIRS = v12_pairs(F10, F12)
IN12 = [False]


def e_dist12(uc, n, raw, eax):
    """e_dist, with 1.2's field offsets (+0xd0 / +0xd4) during 1.2's run"""
    d = ST['dists']
    o = 0xd0 if IN12[0] else 0xd4
    put32(uc, raw[0] + o, d[(2 * n) % len(d)])
    put32(uc, raw[0] + o + 4, d[(2 * n + 1) % len(d)])


EFFECT[0x1001403a] = e_dist12
SHIFT = {}


def g12(ctx):
    obj, = g_1681(ctx)
    r = ctx.rng
    ST['active'].add(GATE12)
    ST['rets'][GATE12] = [r.choice([0, 1, 1, 1])]
    SHIFT.clear()
    SHIFT[obj] = 0x300
    for i in range(4):
        SHIFT[r32(ctx, 0x109e8b40 + 4 * i)] = 0x300
    return [obj]


def _hooks(m):
    if getattr(m, '_w8t12', False):
        return
    m._w8t12 = True
    order = sorted(HELPERS)
    for a10, a12 in PAIRS.items():
        if a10 in HELPERS and a10 != 0x10017a05:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, a10, order.index(a10)), None, A12(a12), A12(a12))


WIN = [(0x10130e58, 0x10107d90, 0x40), (0x109e8b40, 0x107f24c0, 0x10)]
GLOB = [(a10, a12) for a10, a12 in PAIRS.items() if _writable(V10_DLL, a10) and _writable(V12_DLL, a12)
        and a10 not in (0x10130e58, 0x109e8b40)]


def to12(m):
    _hooks(m)
    IN12[0] = True
    for a10, a12 in GLOB:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, 4)))
    for a10, a12, n in WIN:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, n)))
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def from12(m):
    IN12[0] = False
    for a10, a12 in GLOB:
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), 4)))
    for a10, a12, n in WIN:
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), n)))
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])


SPECS = [dict(name='Sim_10011681_v12', orig12=F12, c='Sim_10011681', gen=g12, trials=3000, void=True, to12=to12,
              from12=from12)]
