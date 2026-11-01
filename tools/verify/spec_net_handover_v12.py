# Network round: the damage of computer gears run from another machine, against 1.2, one part per run (NETH_PART):
#   TCR  the "TC" receive Sim_1004084a = 1.2 0x10034468 (game-over guard; the 17 parts from +0x23 of the 0xef bytes)
#   TCS  the "AI" send Sim_100408bd = 1.2 0x100344ef (0x21 bytes), with 1.2's new "DI" send Sim12_10033c90 inside
#   DIR  the "DI" receive Sim12_10033d7b = 1.2 0x10033d7b
# NETDI_FIX (w3p_net.c, Diego's option B) is built off here unless NETH_FIX=1: off must match 1.2 exactly; on, 1.2's
# code gets exactly the two fixes patched in (patch12) and must match again.
# Callees are logging stubs on both sides (Sim_10040721 = 0x100341a6, Sim_10040564 = 0x10033fe8, Sim_10040645 =
# 0x100340ca; the network send Net_DpSend_1007e920 = 0x1006a860 logs its message whole). 1.2's unit records lack our
# dword +0x18 (turned into its layout for its run and back).
# Negative check: NETH_NEG12=1 builds the C with NETG_MUTATE (no guard: TCR, DIR with the fix on) and NETH_MUTATE (no
# "DI" sent: TCS).
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

PART = os.environ.get('NETH_PART', 'TCR')
FIX = os.environ.get('NETH_FIX') == '1'
SOURCES = closure(['sim/unsorted_1004/w3p_net.c'])
CFLAGS = list(globals().get('CFLAGS', [])) + ['-DNETDI_FIX=%d' % FIX]
if os.environ.get('NETH_NEG12'):
    CFLAGS = CFLAGS + ['-DNETG_MUTATE', '-DNETH_MUTATE']
CALLEES = [(0x10040721, 'Sim_10040721', 2, 0x100341a6), (0x10040564, 'Sim_10040564', 1, 0x10033fe8),
           (0x10040645, 'Sim_10040645', 2, 0x100340ca), (0x1007e920, 'Net_DpSend_1007e920', 3, 0x1006a860)]
for _a, _c, _n, _a12 in CALLEES:
    helper(_a, _c, _n, deref={0: 0xd1} if _a == 0x1007e920 else None)
ACTIVE = tuple(c[0] for c in CALLEES)
TABLES = [('g_net10DiSent', 0x107cac00, 10 * 0xd1), ('g_w34NetOver', 0x101314c0, 4)]
PTRS = [(0x1015ecb0, 0x10131500), ('g_net10DiBuf', 0x10131504), (0x1015eca4, 0x101314f4)]
UNIT = 0x200
ST = {}


def _a(m, x):
    return m.syms[x] if isinstance(x, str) else x


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
        w32(ctx, gear + 0x88, parts)
        w32(ctx, e + 4, r.choice([i, i, r.randrange(count)]))
        w32(ctx, e + 0xc, r.choice([self_, self_, r.randrange(count)]))
        w32(ctx, e + 0x14, r.choice([3, 3, 0, 1]))
        w32(ctx, e + 0x18, 0)
        w32(ctx, e + 0x28, gear)
        w32(ctx, 0x109e8b40 + 4 * i, e)
        units.append(e)
    ST['units'] = units
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015dc64, count)
    for a, _ in PTRS:
        b = newbuf(ctx, 0x100)
        wb(ctx, b, bytes(r.getrandbits(8) for _ in range(0x100)))
        w32(ctx, _a(ctx.m, a), b)
    sent = ctx.m.syms['g_net10DiSent']
    for k in range(10):
        wb(ctx, sent + k * 0xd1, bytes(r.getrandbits(8) for _ in range(0xd1)))
    w32(ctx, ctx.m.syms['g_w34NetOver'], r.choice([0, 0, 0, 1]))
    rets(0x1007e920, 1)
    msg = newbuf(ctx, 0x100)
    wb(ctx, msg, bytes(r.getrandbits(8) for _ in range(0x100)))
    w8(ctx, msg + 2, r.choice([self_, r.randrange(count), r.randrange(count), r.randrange(12)]))
    w8(ctx, msg + 4, r.choice([self_, self_, r.randrange(count)]))
    if PART == 'TCS':
        idx = r.choice([r.randrange(count), r.randrange(count), self_, -1, 12])
        if r.random() < 0.3 and 0 <= idx < count and U32_(ctx, 0x109e8b40 + 4 * idx):
            # the same message as last time: nothing sent
            u = U32_(ctx, 0x109e8b40 + 4 * idx)
            ST['same'] = (idx, u)
        return [idx & 0xffffffff]
    return [msg]


def U32_(ctx, a):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(a, 4)))[0]


def to12_more(m):
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(0x109e8b40, 0x30)))
    for name, a12, n in TABLES:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(m.syms[name], n)))
    for a, a12 in PTRS:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(_a(m, a), 4)))
    m.uc.mem_write(A12(0x1014a568), bytes(m.uc.mem_read(0x1017b8c8, 4)))
    m.uc.mem_write(A12(0x10130754), bytes(m.uc.mem_read(0x1015dc64, 4)))
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


def patch12(m):
    """With NETH_FIX=1, 1.2's code gets exactly Diego's two fixes, so the C must match it again: the "DI" tag into
    the "DI" buffer ([0x10131500] -> [0x10131504] at 0x10033ca4) and the receive's test (state == 0) == 3 turned into
    state == 3 (0x10033db3..0x10033dbe: mov eax, [edx+0x14] / nops / cmp eax, 3, its je kept)."""
    v = struct.unpack('<I', bytes(m.raw.mem_read(A12(0x10033ca5), 4)))[0]
    m.raw.mem_write(A12(0x10033ca5), struct.pack('<I', v + 4))
    m.raw.mem_write(A12(0x10033db3), bytes.fromhex('8b4214909090909090' + '83f803'))


def hooks(m):
    if getattr(m, '_neth', False):
        return
    m._neth = True
    if FIX:
        patch12(m)
    for a10, _, _, a12 in CALLEES:
        m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, a10), None, A12(a12), A12(a12))


def to12(m):
    hooks(m)
    to12_more(m)


F = {'TCR': ('Sim_1004084a', 0x10034468, 1), 'TCS': ('Sim_100408bd', 0x100344ef, 1),
     'DIR': ('Sim12_10033d7b', 0x10033d7b, 1)}[PART]
SPECS = [dict(name=F[0] + '_v12' + ('_fix' if FIX else ''), orig12=F[1], c=F[0], gen=g, trials=3000, void=True,
              to12=to12, from12=from12_more)]
