# Network round: 1.2's player update (w9b_net.c), one part per run (NETU_PART), each against 1.2:
#   UP  Net_UpdatePlayers (1.0 0x1004227f) = 1.2's rewritten 0x100360c4
#   RR  Sim12_10036590, the host's round-robin handover of a computer gear
#   DD  Sim12_100362d2, a destroyed computer gear of the local player (kill text, revival)
#   TC  Sim12_100343b5, "TC" with the gear's parts (0xef bytes)
# The other functions each part calls are logging stubs on both sides (texts compared as text); Sim12_100342cf and
# Mem_Move run for real. 1.2's unit records lack our dword +0x18 (turned into its layout for its run and back).
# Negative check: NETU_NEG12=1 builds the C with NETUP_MUTATE (1.0's player update) - for UP.
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

PART = os.environ.get('NETU_PART', 'UP')
SOURCES = closure(['sim/unsorted_1004/w9b_net.c'])
CFLAGS = list(globals().get('CFLAGS', [])) + ['-msse2', '-mfpmath=sse']
if os.environ.get('NETU_NEG12'):
    CFLAGS = CFLAGS + ['-DNETUP_MUTATE']
STATE = (0x1003eebc, 'Net_SendStateMessage', 2, 0x10032782, ())
C12 = lambda f, n: (A12(f), 'Sim12_%08x' % f, n, f, ())
CALLEES = {
    'UP': [STATE, (0x10040376, 'Sim_10040376', 1, 0x10033dec, ()), (0x100408bd, 'Sim_100408bd', 1, 0x100344ef, ()),
           C12(0x10036590, 1), C12(0x100362d2, 1), C12(0x100343b5, 2)],
    'RR': [STATE, (0x10040564, 'Sim_10040564', 1, 0x10033fe8, ()), C12(0x100343b5, 2)],
    'DD': [STATE, (0x100fc780, 'Crt_SPrintf', 5, 0x100da920, (1, 2, 3, 4)), (0x1007df6c, 'Sim_1007df6c', 4, 0x1006a0ec, (0,)),
           (0x10040ab8, 'Sim_10040ab8', 1, 0x1003470e, ()), (0x1009d200, None, 4, 0x1008a560, ()),
           (0x100aa39f, 'Sim_100aa39f', 1, 0x1009582f, ()), (0x100aa454, 'Sim_100aa454', 1, 0x100958e4, ()),
           (0x100aa799, 'Sim_100aa799', 1, 0x10095c29, ()), (0x10040564, 'Sim_10040564', 1, 0x10033fe8, ())],
    'TC': [(0x10040645, 'Sim_10040645', 2, 0x100340ca, ()), (0x1007e920, 'Net_DpSend_1007e920', 3, 0x1006a860, ())],
}[PART]
for _a, _c, _n, _a12, _s in CALLEES:
    helper(_a, _c, _n, sargs=_s, deref={0: 0xef} if _a == 0x1007e920 else None)
ACTIVE = tuple(c[0] for c in CALLEES)
PAIRS = {0x1017b8c8: 0x1014a568, 0x1015dc64: 0x10130754, 0x1015ecec: 0x10131540, 0x1015dc7c: 0x1013076c,
         0x1013ac9c: 0x10110138, 0x1015ecd0: 0x10131524, 0x1015ec78: 0x101314c4, 0x1015d004: 0x107caa74,
         0x1015eca0: 0x101314f0, 0x1017b8c4: 0x1014a564, 0x101c4a04: 0x101873a4, 0x101c49f0: 0x10187390,
         0x109b98c0: 0x107ca7e0}
TABLES = [('g_net10Mark', 0x107cb4d0, 10)]
UNIT = 0x200
ST = {}


def g(ctx):
    r = begin(ctx, *ACTIVE)
    count = r.choice([2, 4, 8, 10])
    self_ = r.randrange(count)
    units = []
    for i in range(10):
        e = newbuf(ctx, UNIT)
        wb(ctx, e, bytes(r.getrandbits(8) for _ in range(UNIT)))
        gear = newbuf(ctx, 0x500)
        parts = newbuf(ctx, 0x11 * 0x3c)
        wb(ctx, parts, bytes(r.getrandbits(8) for _ in range(0x11 * 0x3c)))
        wf(ctx, parts + 0x44, r.choice([0.0, 5e-8, 1e-07, 50.0]))
        w32(ctx, gear + 0x88, parts)
        w32(ctx, gear + 0x4f0, r.choice([0, 2, 3, 4, 5, 6, r.getrandbits(8)]))
        lives = newbuf(ctx, 4)
        w8(ctx, lives, r.choice([0, 1, 2]))
        w32(ctx, e + 4, r.choice([i, i, i, r.randrange(count)]))
        w32(ctx, e + 0xc, r.choice([self_, self_, r.randrange(count), 0xffffffff]))
        w32(ctx, e + 0x14, r.choice([3, 3, 3, 1, 0, 2]))
        w32(ctx, e + 0x18, 0)
        w16(ctx, e + 0x1c, r.choice([0, 2, 4, 6, 0x4000, r.getrandbits(16)]))
        w32(ctx, e + 0x28, gear)
        w32(ctx, e + 0x48, newbuf(ctx, 0x40))
        w32(ctx, e + 0xf0, r.choice([0x200 | r.randrange(count), 0x200 | self_, r.getrandbits(16)]))
        w32(ctx, e + 0x150, r.choice([r.randrange(-2, 12) & 0xffff, self_, i]))
        w32(ctx, e + 0x160, r.choice([0, 0, 1]))
        w32(ctx, e + 0x164, r.choice([0, 0, 1]))
        w32(ctx, e + 0x168, lives)
        wb(ctx, e + 0x124, b'Pilot%d\0' % i)
        w32(ctx, 0x109e8b40 + 4 * i, e)
        units.append(e)
    ST['units'] = units
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015dc64, count)
    frame = r.getrandbits(20)
    w32(ctx, 0x1015dc7c, frame)
    w32(ctx, 0x1015ecec, r.choice([frame, frame + 5, frame - 5]))
    w32(ctx, 0x1013ac9c, 36)
    w32(ctx, 0x1015ecd0, r.choice([1, 2]))
    root = newbuf(ctx, 0x200)
    recs = newbuf(ctx, 10 * 0x1e)
    for k in range(10):
        w16(ctx, recs + k * 0x1e + 0x1a, r.randrange(count))
    # at least one free player in the list (state 0 / 1, no +0x160): the round robin loops for ever otherwise, in
    # both versions
    free = r.randrange(count)
    w16(ctx, recs + r.randrange(count) * 0x1e + 0x1a, free)
    w32(ctx, units[free] + 0x14, r.choice([0, 1]))
    w32(ctx, units[free] + 0x160, 0)
    w32(ctx, root + 0x12, count)
    w32(ctx, root + 0x126, recs)
    w32(ctx, 0x1015d004, root)
    w32(ctx, 0x1015ec78, r.randrange(count))
    w32(ctx, 0x1015eca0, newbuf(ctx, 0x100))
    cfg = newbuf(ctx, 0x10)
    w8(ctx, cfg + 8, r.choice([0, 1]))
    w32(ctx, 0x1017b8c4, cfg)
    for a in (0x101c4a04, 0x101c49f0, 0x109b98c0):
        p = newbuf(ctx, 0x10)
        wb(ctx, p, b'txt%x\0' % (a & 0xff))
        w32(ctx, a, p)
    for k in range(10):
        w8(ctx, ctx.m.syms['g_net10Mark'] + k, r.choice([0, 0, 1]))
    if PART == 'TC':
        return [r.randrange(count), r.randrange(count)]
    if PART in ('RR', 'DD'):
        return [r.randrange(count)]
    return []


def log_map(log_o, log_c):
    def fix(log):
        out = []
        for e in log:
            if e[0] == 'stub_100fc780' and isinstance(e[1], tuple) and isinstance(e[1][1], bytes):
                e = (e[0], e[1][:2 + e[1][1].count(b'%s')])      # only the arguments the text uses
            out.append(e)
        return out
    return fix(log_o), fix(log_c)


def to12_more(m):
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(0x109e8b40, 0x28)))
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


def to12(m):
    if not getattr(m, '_netu', False):
        m._netu = True
        for a10, _, _, a12, _ in CALLEES:
            if a10 != A12(a12):
                m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, a10), None, A12(a12), A12(a12))
    to12_more(m)


F = {'UP': ('Net_UpdatePlayers', 0x100360c4), 'RR': ('Sim12_10036590', 0x10036590),
     'DD': ('Sim12_100362d2', 0x100362d2), 'TC': ('Sim12_100343b5', 0x100343b5)}[PART]
SPECS = [dict(name=F[0] + '_v12', orig12=F[1], c=F[0], gen=g, trials=3000, void=True, fp=True, to12=to12,
              from12=from12_more, log_map=log_map)]
