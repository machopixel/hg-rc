# Network round: "DM", a remote unit's part damage levels (Sim_10040191, w7m_misc.c) against 1.2's 0x10033a8a. 1.2's
# changes: the game-over guard; the part values are no longer set from the levels (1.2 computes them into a local and
# drops them); a part reported destroyed has its value zeroed, and the second part marks the unit (g_net10Mark). The
# node levels (Sim_100aae7b = 1.2 0x1009630b) and the kill call (0x1006abf6 = 0x1005a9c6) are logging stubs on both
# sides. 1.2's unit records lack our dword +0x18 (turned into its layout for its run and back).
# Negative checks: NETDM_NEG12=1 builds the C with NETDM_MUTATE (1.0's part values), NETDM_NEG12=g with NETG_MUTATE.
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1004/w7m_misc.c'])
CFLAGS = list(globals().get('CFLAGS', [])) + ['-msse2', '-mfpmath=sse']
_neg = os.environ.get('NETDM_NEG12')
if _neg == '1':
    CFLAGS = CFLAGS + ['-DNETDM_MUTATE']
elif _neg == 'g':
    CFLAGS = CFLAGS + ['-DNETG_MUTATE']
CALLEES = [(0x100aae7b, 'Sim_100aae7b', 3, 0x1009630b), (0x1006abf6, None, 3, 0x1005a9c6)]
for _a, _c, _n, _a12 in CALLEES:
    helper(_a, _c, _n)
ACTIVE = tuple(c[0] for c in CALLEES)
PAIRS = {0x1017b8c8: 0x1014a568, 0x1015dc64: 0x10130754}
TABLES = [('g_net10Row', 0x107caa80, 10 * 0x16), ('g_net10Mark', 0x107cb4d0, 10), ('g_w34NetOver', 0x101314c0, 4)]
UNIT = 0x200
ST = {}


def g(ctx):
    r = begin(ctx, *ACTIVE)
    count = r.choice([2, 4, 10])
    self_ = r.randrange(count)
    units = []
    for i in range(12):
        if i >= count or (i != self_ and r.random() < 0.1):
            w32(ctx, 0x109e8b40 + 4 * i, 0)
            continue
        e = newbuf(ctx, UNIT)
        wb(ctx, e, bytes(r.getrandbits(8) for _ in range(UNIT)))
        gear = newbuf(ctx, 0x100)
        parts = newbuf(ctx, 0x11 * 0x3c)
        wb(ctx, parts, bytes(r.getrandbits(8) for _ in range(0x11 * 0x3c)))
        for k in range(0x11):
            wf(ctx, parts + k * 0x3c, r.uniform(0, 500))
            wf(ctx, parts + k * 0x3c + 8, r.uniform(0, 500))
            w32(ctx, parts + k * 0x3c + 0x38, r.randrange(0, 0x10000))
        w32(ctx, gear + 0x88, parts)
        w32(ctx, gear, e)
        w32(ctx, e + 4, r.choice([i, i, r.randrange(count)]))
        w32(ctx, e + 0xc, r.choice([self_, r.randrange(count)]))
        w32(ctx, e + 0x14, r.choice([3, 1, 1, 0]))
        w32(ctx, e + 0x18, 0)
        w16(ctx, e + 0x1c, r.choice([0, 0, 2, 4, r.getrandbits(16)]))
        w32(ctx, e + 0x28, gear)
        w32(ctx, e + 0x48, newbuf(ctx, 0x40))
        w32(ctx, 0x109e8b40 + 4 * i, e)
        units.append(e)
    ST['units'] = units
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015dc64, count)
    for name, _, n in TABLES[:2]:
        wb(ctx, ctx.m.syms[name], bytes(r.getrandbits(8) for _ in range(n)))
    w32(ctx, ctx.m.syms['g_w34NetOver'], r.choice([0, 0, 0, 1]))
    msg = newbuf(ctx, 0x20)
    wb(ctx, msg, bytes(r.getrandbits(8) for _ in range(0x20)))
    w8(ctx, msg + 2, r.choice([self_, r.randrange(count), r.randrange(count), r.randrange(12)]))
    return [msg]


def to12_more(m):
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(0x109e8b40, 0x30)))
    for a10, a12 in PAIRS.items():
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, 4)))
    for name, a12, n in TABLES:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(m.syms[name], n)))
    ST['usave'] = {}
    for u in ST['units']:
        d = bytes(m.uc.mem_read(u, UNIT))
        ST['usave'][u] = d
        m.uc.mem_write(u, d[:0x18] + d[0x1c:] + bytes(4))


def from12_more(m):
    for name, a12, n in TABLES:
        m.uc.mem_write(m.syms[name], bytes(m.uc.mem_read(A12(a12), n)))
    for u, old in ST['usave'].items():
        d = bytes(m.uc.mem_read(u, UNIT))
        m.uc.mem_write(u, d[:0x18] + old[0x18:0x1c] + d[0x18:UNIT - 4])


def to12(m):
    if not getattr(m, '_netdm', False):
        m._netdm = True
        for a10, _, _, a12 in CALLEES:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, a10), None, A12(a12), A12(a12))
    to12_more(m)


SPECS = [dict(name='Sim_10040191_v12', orig12=0x10033a8a, c='Sim_10040191', gen=g, trials=3000, void=True, fp=True,
              to12=to12, from12=from12_more)]
