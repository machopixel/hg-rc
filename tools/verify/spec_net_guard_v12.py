# Network round: 1.2's game-over guard in twelve network message handlers, each against its 1.2 twin. 1.2 tests its
# receive guard 0x101314c0 (our g_w34NetOver, set by the message loop from the game-over flag before each message)
# first and ignores the message while it is set; the rest of each handler is unchanged apart from the unit record's
# layout. One handler per run (NETG_PART = its tag, default all in turn is not possible: one v12 spec per run):
#   CH 0x1003ff72 / 0x10033860   WE 0x1004044c / 0x10033ec2   CA 0x100405e6 / 0x1003406a   AI 0x1004094e / 0x1003458e
#   DS 0x10040a37 / 0x10034682   NR 0x10040f2d / 0x10034bbc   OR 0x100410ff / 0x10034d99   DR 0x1004131b / 0x10034fce
#   CO 0x1004170e / 0x100354eb   EG 0x10041a16 / 0x10035801   SN 0x10041b2d / 0x10035923   PU 0x10041247 / 0x10034eef
# Their callees are logging stubs on both sides (text arguments compared as text: the name pointer is +0x124 in our
# unit record and +0x120 in 1.2's). 1.2's unit records are ours without the dword at +0x18: turned into its layout for
# its run and back after it. The unit table 0x109e8b40 is copied to 1.2's 0x107f24c0.
# Negative check: NETG_NEG12=1 builds the C with NETG_MUTATE (no guard).
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

PART = os.environ.get('NETG_PART', 'CH')
H = {'CH': (0x1003ff72, 0x10033860, 'sim/unsorted_1003/w5a_misc.c', 2),
     'WE': (0x1004044c, 0x10033ec2, 'sim/unsorted_1004/w2i_misc.c', 1),
     'CA': (0x100405e6, 0x1003406a, 'sim/unsorted_1004/w2j_misc.c', 1),
     'AI': (0x1004094e, 0x1003458e, 'sim/unsorted_1004/w2h_record.c', 2),
     'DS': (0x10040a37, 0x10034682, 'sim/unsorted_1004/w5a_misc.c', 1),
     'NR': (0x10040f2d, 0x10034bbc, 'sim/unsorted_1004/w2i_misc.c', 1),
     'OR': (0x100410ff, 0x10034d99, 'sim/unsorted_1004/w7m_misc.c', 1),
     'DR': (0x1004131b, 0x10034fce, 'sim/unsorted_1004/w6e_misc.c', 1),
     'CO': (0x1004170e, 0x100354eb, 'sim/unsorted_1004/w8u_camera.c', 2),
     'EG': (0x10041a16, 0x10035801, 'sim/unsorted_1004/w5y_misc.c', 0),
     'SN': (0x10041b2d, 0x10035923, 'sim/unsorted_1004/w6e_misc.c', 2),
     'PU': (0x10041247, 0x10034eef, 'sim/unsorted_1003/w5c_calls.c', 1)}
A10, F12, SRC, NARGS = H[PART]
SOURCES = closure([SRC])
CFLAGS = list(globals().get('CFLAGS', [])) + ['-msse2', '-mfpmath=sse']
if os.environ.get('NETG_NEG12'):
    CFLAGS = CFLAGS + ['-DNETG_MUTATE']

# callee (1.0 address, C function or None, argument count, 1.2 twin, text arguments)
CALLEES = {
    'CH': [(0x100fc780, None, 5, 0x100da920, (1, 2, 3)), (0x1007df6c, 'Sim_1007df6c', 4, 0x1006a0ec, (0,)),
           (0x1000ebfb, 'Sim_1000ebfb', 5, 0x10008723, (0,))],
    'WE': [(0x100d63fa, 'Sim_100d63fa', 1, 0x100bc428, ())],
    'CA': [],
    'AI': [(0x10040721, 'Sim_10040721', 2, 0x100341a6, ())],
    'DS': [(0x10016de7, None, 2, 0x1000ff96, ()), (0x10016632, None, 1, 0x1000f7d8, ())],
    'NR': [],
    'OR': [(0x100fcdc0, 'Mem_Move', 3, 0x100daf60, ()), (0x1003dbc0, 'Sim_1003dbc0', 1, 0x10031330, ()),
           (0x1007e950, 'Net_DpSend_1007e950', 4, 0x1006a890, ()), (0x10015d30, None, 4, 0x1000eec9, ())],
    'DR': [(0x100386d5, None, 3, 0x1002c9c5, ()), (0x100aa799, 'Sim_100aa799', 1, 0x10095c29, ())],
    'CO': [(0x10084a94, 'Sim_10084a94', 1, 0x1006f714, ()), (0x1000ebfb, 'Sim_1000ebfb', 5, 0x10008723, (0,)),
           (0x1006788c, None, 2, 0x1005815c, ())],
    'EG': [(0x1007c42f, None, 0, 0x1006932f, ())],
    'SN': [(0x1002597b, None, 1, 0x1001caf9, ())],
    'PU': [(0x10038913, None, 3, 0x1002cc03, ()), (0x100aa799, None, 1, 0x10095c29, ())],
}[PART]
for _a, _c, _n, _a12, _s in CALLEES:
    helper(_a, _c, _n, sargs=_s)
ACTIVE = tuple(c[0] for c in CALLEES)
MASK[(0x10016de7, 1)] = 0xffff      # DS: both leave the upper half of a register in this 16-bit argument
WINDOWS = {'WE': {0x101853c0: 0x20}, 'SN': {0x1015db80: 0x30}}.get(PART, {})   # WE: the weapon flags; SN: stamps
UNIT = 0x200
ST = {}


def g(ctx):
    r = begin(ctx, *ACTIVE)
    count = r.choice([1, 2, 3, 4, 8, 10])
    self_ = r.randrange(count)
    units = []
    for i in range(12):
        if i >= count or (i != self_ and r.random() < 0.1):     # the own unit always exists
            w32(ctx, 0x109e8b40 + 4 * i, 0)
            continue
        e = newbuf(ctx, UNIT)
        wb(ctx, e, bytes(r.getrandbits(8) for _ in range(UNIT)))
        blk = newbuf(ctx, 0xc50)
        wb(ctx, blk, bytes(r.getrandbits(8) for _ in range(0xc50)))
        w32(ctx, e + 4, r.choice([i, self_, i]))
        w32(ctx, e + 0xc, r.choice([self_, self_, r.randrange(count), -1 & 0xffffffff]))
        w32(ctx, e + 0x14, r.choice([3, 3, 1, 0]))
        w32(ctx, e + 0x18, 0)       # our dword +0x18 is not in 1.2's record (CA clears it in ours): 0 on both sides
        w16(ctx, e + 0x1c, r.choice([0, 0, 2, 4, 6, r.getrandbits(16)]))
        w32(ctx, e + 0x28, blk)
        w32(ctx, blk + 0x84, newbuf(ctx, 8 * 0x128))     # WE: the weapon records
        w32(ctx, e + 0x48, blk + 0x100)
        wb(ctx, e + 0x124, b'Player%d\0' % i)
        w32(ctx, 0x109e8b40 + 4 * i, e)
        units.append(e)
    ST['units'] = units
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015dc64, count)
    w32(ctx, 0x1015ec58, r.choice([0, 1, 1]))
    root = newbuf(ctx, 0x200)
    wb(ctx, root, bytes(r.getrandbits(8) for _ in range(0x200)))
    w32(ctx, root + 0x16, newbuf(ctx, 0x20))
    w32(ctx, 0x1015d004, root)
    w32(ctx, 0x1015ecac, newbuf(ctx, 0x40))
    w32(ctx, ctx.m.syms['g_w34NetOver'], r.choice([0, 0, 1]))
    for a in (0x1015f1fc, 0x1015f1e4, 0x1015f1d0):
        w32(ctx, a, r.choice([0, 0, 1, r.getrandbits(32)]))
    msg = newbuf(ctx, 0x40)
    wb(ctx, msg, bytes(r.getrandbits(8) for _ in range(0x40)))
    w8(ctx, msg + 2, r.choice([self_, r.randrange(count), r.randrange(12)]))
    w8(ctx, msg + 3, r.choice([self_, r.randrange(count), 0]))
    w8(ctx, msg + 4, r.choice([self_, r.randrange(count)]))
    if PART == 'OR' and r.random() < 0.3:
        w16(ctx, msg + 3, 0x4200)
    if PART == 'DS':
        w16(ctx, msg + 3, r.choice([0, 7, 1, 7, 0]))
    if PART == 'SN':
        for k in range(12):
            w32(ctx, 0x1015db80 + 4 * k, r.choice([0, r.getrandbits(31), 0x7fffffff]))
    for a, _, _, _, _ in CALLEES:
        rets(a, *[r.choice([0, 1, 2, self_]) for _ in range(4)])
    return [msg, r.choice([self_, r.randrange(count), r.randrange(10)])][:NARGS] if NARGS else []


def to12_more(m):
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(0x109e8b40, 0x30)))
    m.uc.mem_write(A12(0x101314c0), bytes(m.uc.mem_read(m.syms['g_w34NetOver'], 4)))
    ST['usave'] = {}
    for u in ST['units']:
        d = bytes(m.uc.mem_read(u, UNIT))
        ST['usave'][u] = d
        m.uc.mem_write(u, d[:0x18] + d[0x1c:] + bytes(4))


def from12_more(m):
    for u, old in ST['usave'].items():
        d = bytes(m.uc.mem_read(u, UNIT))
        m.uc.mem_write(u, d[:0x18] + old[0x18:0x1c] + d[0x18:UNIT - 4])


SPECS = [v12_spec(dict(name='Sim_%08x' % A10, orig=A10, c='Sim_%08x' % A10, gen=g, trials=1500, void=True), F12,
                  windows=WINDOWS,
                  to12_more=to12_more, from12_more=from12_more,
                  extra_pairs=dict([(c[0], c[3]) for c in CALLEES] + [(0x1017b8c8, 0x1014a568), (0x1015dc64, 0x10130754),
                                                                       (0x1015ec58, 0x101314a0), (0x1015d004, 0x107caa74)]),
                  skip=(0x109e8b40,), name='Sim_%08x_v12' % A10)]
