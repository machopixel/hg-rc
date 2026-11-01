# Network round: "RG" (regeneration / revival), Sim_10040af8 (w20_net.c) in 1.2's rewritten form against 1.2's
# 0x1003474e. Callees are logging stubs on both sides (the network send logs its 3-byte message; Sim12_10089c60 is
# the C's own, stubbed too). 1.2's unit records lack our dword +0x18 (turned into its layout for its run and back).
# Negative checks: NETRG_NEG12=1 builds the C with NETRG_MUTATE (1.0's "RG"), NETRG_NEG12=g with NETG_MUTATE (no guard).
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1004/w20_net.c'])
_neg = os.environ.get('NETRG_NEG12')
if _neg == '1':
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DNETRG_MUTATE']
elif _neg == 'g':
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DNETG_MUTATE']
CALLEES = [(0x1003dbc0, 'Sim_1003dbc0', 1, 0x10031330, ()), (0x1007e950, 'Net_DpSend_1007e950', 4, 0x1006a890, ()),
           (0x1003db56, 'Sim_1003db56', 1, 0x100312c6, ()), (0x1007df6c, 'Sim_1007df6c', 4, 0x1006a0ec, (0,)),
           (0x1003cbb3, 'Sim_1003cbb3', 2, 0x100306a3, ()), (A12(0x10089c60), 'Sim12_10089c60', 0, 0x10089c60, ()),
           (0x1009d200, 'Sim_1009d200', 4, 0x1008a560, ()), (0x100448cd, 'Sim_100448cd', 1, 0x1003719d, ()),
           (0x100aa39f, 'Sim_100aa39f', 1, 0x1009582f, ()), (0x100aa454, 'Sim_100aa454', 1, 0x100958e4, ()),
           (0x100aa799, 'Sim_100aa799', 1, 0x10095c29, ()), (0x100fc780, 'Crt_SPrintf', 3, 0x100da920, (1, 2)),
           (0x10040564, 'Sim_10040564', 1, 0x10033fe8, ())]
for _a, _c, _n, _a12, _s in CALLEES:
    helper(_a, _c, _n, sargs=_s, deref={1: 3} if _a == 0x1007e950 else None)
MASK[(0x1003db56, 0)] = 0xffff
ACTIVE = tuple(c[0] for c in CALLEES)
PAIRS = {0x1015ecd0: 0x10131524, 0x1017b8c8: 0x1014a568, 0x1015eca8: 0x101314f8, 0x1015d004: 0x107caa74,
         0x1015ecd8: 0x1013152c, 0x1015f200: 0x1013196c, 0x1015f1d0: 0x1013193c, 0x109b98c0: 0x107ca7e0,
         0x1015f208: 0x10131974, 0x1015ef10: 0x101316f8, 0x1014cb1c: 0x1011fa24, 0x1015fdba: 0x1013355a,
         0x1015fdbd: 0x1013355d, 0x1015fdd0: 0x10133570, 0x1015d530: 0x101301e8, 0x1015f1d4: 0x10131940,
         0x1015f1e4: 0x10131950, 0x1015f1fc: 0x10131968}
TABLES = [('g_net10Mark', 0x107cb4d0, 10), ('g_w34NetOver', 0x101314c0, 4)]
UNIT = 0x200
ST = {}


def g(ctx):
    r = begin(ctx, *ACTIVE)
    count = r.choice([2, 4, 10])
    self_ = r.randrange(count)
    units = []
    for i in range(20):     # the unit table goes on past the players (a revival's unit + 10 reads there on the host)
        if (i >= count and i < 10 and r.random() < 0.7) or (i != self_ and r.random() < 0.1):
            w32(ctx, 0x109e8b40 + 4 * i, 0)
            continue
        e = newbuf(ctx, UNIT)
        wb(ctx, e, bytes(r.getrandbits(8) for _ in range(UNIT)))
        lives = newbuf(ctx, 4)
        w8(ctx, lives, r.choice([0, 0, 1, 3]))
        w32(ctx, e + 4, r.choice([i, i, i, self_, r.randrange(count)]))
        w32(ctx, e + 0x14, r.choice([3, 3, 1, 0]))
        w32(ctx, e + 0x18, 0)
        w32(ctx, e + 0x164, r.choice([0, 1]))
        w32(ctx, e + 0x168, lives)
        w32(ctx, e + 0x48, newbuf(ctx, 0x40))
        wb(ctx, e + 0x124, b'Pilot%d\0' % i)
        w32(ctx, 0x109e8b40 + 4 * i, e)
        units.append(e)
    ST['units'] = units
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015ecd0, r.choice([1, 2]))
    w32(ctx, 0x1015eca8, newbuf(ctx, 0x10))
    root = newbuf(ctx, 0x200)
    keys = newbuf(ctx, 0x20)
    wb(ctx, keys, bytes(r.getrandbits(8) for _ in range(0x20)))
    w32(ctx, root + 0x16, keys)
    w32(ctx, 0x1015d004, root)
    w16(ctx, 0x1015ecd8, r.getrandbits(16))
    for a in PAIRS:
        if a not in (0x1015ecd0, 0x1017b8c8, 0x1015eca8, 0x1015d004, 0x1015ecd8, 0x109b98c0):
            w32(ctx, a, r.choice([0, 0, 1, r.getrandbits(32)]))
    w32(ctx, 0x109b98c0, newbuf(ctx, 0x10))
    for k in range(10):
        w8(ctx, ctx.m.syms['g_net10Mark'] + k, r.choice([0, 1]))
    w32(ctx, ctx.m.syms['g_w34NetOver'], r.choice([0, 0, 0, 1]))
    who = r.choice([self_, r.randrange(count), r.randrange(count)])
    rets(0x1003db56, r.choice([who, who, r.randrange(count)]))
    rets(0x1003dbc0, r.randrange(count))
    msg = newbuf(ctx, 0x10)
    wb(ctx, msg, b'RG' + bytes([r.choice([self_, self_ + 10, r.randrange(count), r.randrange(count) + 10, 19])]))
    return [msg, who]


def to12_more(m):
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(0x109e8b40, 0x50)))
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
    for a10, a12 in PAIRS.items():
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), 4)))
    for name, a12, n in TABLES:
        m.uc.mem_write(m.syms[name], bytes(m.uc.mem_read(A12(a12), n)))
    for u, old in ST['usave'].items():
        d = bytes(m.uc.mem_read(u, UNIT))
        m.uc.mem_write(u, d[:0x18] + old[0x18:0x1c] + d[0x18:UNIT - 4])


def hooks(m):
    if getattr(m, '_netrg', False):
        return
    m._netrg = True
    for a10, _, _, a12, _ in CALLEES:
        if a10 != A12(a12):             # (a 1.2-only callee is already stubbed at its own address)
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, a10), None, A12(a12), A12(a12))


def to12(m):
    hooks(m)
    to12_more(m)


SPECS = [dict(name='Sim_10040af8_v12', orig12=0x1003474e, c='Sim_10040af8', gen=g, trials=3000, void=True,
              to12=to12, from12=from12_more)]
if os.environ.get('NETRG_DEBUG'):
    from unicorn import UC_HOOK_MEM_FETCH_UNMAPPED
    _h0 = hooks

    def hooks(m):
        if not getattr(m, '_netrgdbg', False):
            m._netrgdbg = True

            def cb(uc, access, address, size, value, ud):
                if ST.get('dbg', 0) < 4:
                    ST['dbg'] = ST.get('dbg', 0) + 1
                    esp = uc.reg_read(UC_X86_REG_ESP)
                    print('FETCH', hex(address), 'last in RG', hex(ST.get('last', 0) - A12(0) + 0x10000000))
                return False
            m.raw.hook_add(UC_HOOK_MEM_FETCH_UNMAPPED, cb)

            def trace(uc, address, size, ud):
                ST['last'] = address
            m.raw.hook_add(UC_HOOK_CODE, trace, None, A12(0x1003474e), A12(0x10034b31))
        _h0(m)
