# Network round: the "DA" message (a remote unit's state), 1.0 0x1003f3f8 converted from 1.2's 0x10032cbe
# (Sim_1003f3f8, w36_net.c), against 1.2. 1.2's changes: the game-over guard, no skip for a unit of state 3 run by the
# local player, a unit whose second part is gone marked, killers 0..9 (1.0: 0..7) and its 10-player tables
# (ours: g_net10Heard / Db20 / Mark = 1.2 0x107cab60 / 0x107caba0 / 0x107cb4d0). Callees are logging stubs on both
# sides (the ground height Sim_1002d961 = 1.2 0x10022591 answers in st(0) from the generator; the texts are
# compared as text). 1.2's unit records lack our dword +0x18 (turned into its layout for its run and back); the gear
# record is the same in both.
# Negative checks: NETDA_NEG12=1 builds the C with NETDA_MUTATE (no guard), NETDA_NEG12=8 with NETDA_MUTATE8
# (1.0's killer range).
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w36_net.c'])
CFLAGS = list(globals().get('CFLAGS', [])) + ['-msse2', '-mfpmath=sse']
_neg = os.environ.get('NETDA_NEG12')
if _neg == '1':
    CFLAGS = CFLAGS + ['-DNETDA_MUTATE']
elif _neg == '8':
    CFLAGS = CFLAGS + ['-DNETDA_MUTATE8']

CALLEES = [(0x1003db56, 'Sim_1003db56', 1, 0x100312c6, ()), (0x1006abf6, 'Sim_1006abf6', 3, 0x1005a9c6, ()),
           (0x1006a12d, 'Score_RecordKill', 2, 0x10059e9d, ()), (0x100fc780, 'Crt_SPrintf', 5, 0x100da920, (1, 2, 3, 4)),
           (0x1007df6c, 'Sim_1007df6c', 4, 0x1006a0ec, (0,)), (0x1009d200, 'Sim_1009d200', 4, 0x1008a560, ()),
           (0x100aa39f, 'Sim_100aa39f', 1, 0x1009582f, ()), (0x100aa454, 'Sim_100aa454', 1, 0x100958e4, ()),
           (0x100aa799, 'Sim_100aa799', 1, 0x10095c29, ())]
for _a, _c, _n, _a12, _s in CALLEES:
    helper(_a, _c, _n, sargs=_s)
MASK[(0x1003db56, 0)] = 0xffff
FSTUB[0x1002d961] = ('Sim_1002d961', 3)
FSTUB12 = {0x1002d961: A12(0x10022591)}
ACTIVE = tuple(c[0] for c in CALLEES)
TABLES = [('g_net10Heard', 0x107cab60, 0x28), ('g_net10Db20', 0x107caba0, 0x28), ('g_net10Mark', 0x107cb4d0, 10),
          ('g_w34NetOver', 0x101314c0, 4)]
UNIT = 0x200
ST = {}


def fbits(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    ST['fvals'] = {0x1002d961: [fbits(r.choice([0.0, r.uniform(-500, 500), 100.0, -50.0]))]}
    count = r.choice([2, 4, 8, 10, 10])
    self_ = r.randrange(count)
    units = []
    for i in range(10):
        if i >= count or (i != self_ and r.random() < 0.08):
            w32(ctx, 0x109e8b40 + 4 * i, 0)
            continue
        e = newbuf(ctx, UNIT)
        wb(ctx, e, bytes(r.getrandbits(8) for _ in range(UNIT)))
        gear = newbuf(ctx, 0xc80)
        wb(ctx, gear, bytes(r.getrandbits(8) for _ in range(0xc80)))
        for off in (0x548, 0x54c, 0x550, 0x520):
            wf(ctx, gear + off, r.uniform(-100, 100))
        parts = newbuf(ctx, 0x11 * 0x3c)
        wf(ctx, parts + 0x3c + 8, r.choice([0.0, 5e-8, 1e-07, 100.0, r.uniform(0, 500)]))
        w32(ctx, gear + 0x88, parts)
        view = newbuf(ctx, 0x80)
        w32(ctx, gear, e)
        w32(ctx, e + 0x54, view)            # 1.2's +0x50 once shifted
        for off in (0xc15, 0xc19, 0xc1d, 0xc21, 0xc25, 0xc31):
            w32(ctx, gear + off, r.choice([0, 0, 1]))
        w32(ctx, e + 4, r.choice([i, i, i, r.randrange(count)]))
        w32(ctx, e + 8, r.randrange(40))
        w32(ctx, e + 0xc, r.choice([self_, r.randrange(count)]))
        w32(ctx, e + 0x14, r.choice([3, 1, 1, 0]))
        w32(ctx, e + 0x18, 0)
        w32(ctx, e + 0x28, gear)
        w32(ctx, e + 0x48, newbuf(ctx, 0x40))
        w32(ctx, e + 0x15c, r.choice([0, 0, 1]))
        wb(ctx, e + 0x124, b'Pilot%d\0' % i)
        w32(ctx, 0x109e8b40 + 4 * i, e)
        units.append(e)
    ST['units'] = units
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015dc64, count)
    frame = r.randrange(0, 100000)
    w32(ctx, 0x1015dc78, frame)
    w32(ctx, 0x1015dc7c, r.getrandbits(31))
    wf(ctx, 0x1015d088, r.choice([0.0, r.uniform(0, 50), 40.0]))
    w32(ctx, 0x1013ac9c, r.choice([36, 45, 10]))
    w16(ctx, 0x1015ecd8, r.getrandbits(16))
    w32(ctx, 0x1015ecd0, r.choice([1, 2]))
    w32(ctx, 0x1015ec60, 0)
    for k in range(10):
        w32(ctx, ctx.m.syms['g_net10Db20'] + 4 * k, r.choice([0, frame, frame + 5, r.getrandbits(31)]))
        w8(ctx, ctx.m.syms['g_net10Mark'] + k, r.choice([0, 0, 1]))
    w32(ctx, ctx.m.syms['g_w34NetOver'], r.choice([0, 0, 0, 1]))
    for a in (0x101c4a04, 0x101c49f0, 0x101c4a14, 0x109b98c0):
        p = newbuf(ctx, 0x20)
        wb(ctx, p, b'text%x\0' % (a & 0xff))
        w32(ctx, a, p)
    msg = newbuf(ctx, 0x48)
    wb(ctx, msg, bytes(r.getrandbits(8) for _ in range(0x48)))
    w8(ctx, msg + 2, r.choice([self_, r.randrange(count), r.randrange(count), r.randrange(12)]))
    w16(ctx, msg + 3, r.choice([0x400 | r.randrange(256), r.getrandbits(16)]))
    w16(ctx, msg + 5, r.choice([r.randrange(-2, 12) & 0xffff, 8, 9, r.getrandbits(16)]))
    w16(ctx, msg + 7, (r.getrandbits(16) & 0xfff0) | r.choice([4, 5, 0, 1, 2]))
    w32(ctx, msg + 10, r.choice([frame - r.randrange(0, 60), frame + 3, r.getrandbits(31)]))
    for off in range(0xe, 0x42, 4):
        w32(ctx, msg + off, r.choice([r.randrange(-200000, 200000), r.randrange(-100, 100)]) & 0xffffffff)
    for a, _, _, _, _ in CALLEES:
        rets(a, *[r.choice([0, 1, self_, r.randrange(count)]) for _ in range(4)])
    return [msg, r.randrange(10), r.choice([0x42, 0x42, 0x42, 0x40])]


def to12_more(m):
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(0x109e8b40, 0x28)))
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


def log_map(log_o, log_c):
    """the texts with two names leave the fifth formatting argument unused (stack leftovers on both sides)"""
    def fix(log):
        out = []
        for e in log:
            if e[0] == 'stub_100fc780' and isinstance(e[1], tuple) and e[1][1] == b"'%s' %s":
                e = (e[0], e[1][:4])
            # a killer number whose slot is empty: both read the name at null + the name's offset (ours +0x124,
            # 1.2's +0x120), an invalid read on both sides
            if e[0] == 'stub_100fc780' and isinstance(e[1], tuple) and len(e[1]) > 4 and e[1][4] in (('bad', 0x120), ('bad', 0x124)):
                e = (e[0], e[1][:4] + (('bad', 'null name'),))
            out.append(e)
        return out
    return fix(log_o), fix(log_c)


SPECS = [v12_spec(dict(name='Sim_1003f3f8', orig=0x1003f3f8, c='Sim_1003f3f8', gen=g, trials=3000, void=True,
                       fp=True, log_map=log_map), 0x10032cbe, to12_more=to12_more, from12_more=from12_more,
                  windows={0x109e9ba0: 0x50 * 0x100},
                  extra_pairs=dict([(c[0], c[3]) for c in CALLEES] + [(0x109e9ba0, 0x107f2540), (0x1015dc7c, 0x1013076c)]),
                  skip=(0x109e8b40, 0x1015db00, 0x1015db20, 0x1015d1b0), name='Sim_1003f3f8_v12')]
if os.environ.get('NETDA_SYMS'):
    _g0 = g

    def g(ctx):
        if not ST.get('printed'):
            ST['printed'] = 1
            print({k: hex(v) for k, v in ctx.m.syms.items() if 0x40048900 <= v <= 0x40048b00})
        return _g0(ctx)
    SPECS[0]['gen'] = g
