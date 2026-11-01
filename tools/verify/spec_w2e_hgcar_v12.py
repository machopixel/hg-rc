# Round 33, round 3: the HGCAR.CFG record in 1.2's 0x21c-byte layout (Diego: "option 1", DECISIONS.md), against
# 1.2's own code: the mission-start clears Sim_10021ac0 (1.2 0x100186d0) and Sim_10025d93 (0x1001cf11), and the
# save Sim_10025dac (0x1001cf2a), which puts 1.2's record together and hands it to 0x10046f7f (stubbed on both
# sides, the 0x21c bytes it is given logged with the file name).
# Our record is split: 1.0's 0x1d4 bytes at 0x101653e0 (bytes 0..0x14c and the two dwords +0x1cc / +0x1d0) and
# 1.2's 10 x 10 kill table g_w34Kills (0xc8 bytes). 1.2's is one block at 0x1028cae0: 0..0x14c, the table at
# +0x14c, the two dwords at +0x214. Before 1.2's run that block is put together from ours; after it, taken apart
# into ours again (1.0's old table area +0x14c..+0x1cc, unused by 1.2, is cleared by the C and left by 1.2: kept out
# of the comparison by copying the C's rule, cleared when 1.2 cleared its record).
# Negative checks: W2E_NEG12=t builds the C with W2ET_MUTATE (the kill table not cleared), =h with W5YH_MUTATE (1.0's
# table saved).
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1002/w2e_tables.c', 'sim/unsorted_1002/w5y_misc.c'])
_neg = os.environ.get('W2E_NEG12')
if _neg:
    CFLAGS = list(globals().get('CFLAGS', [])) + [{'t': '-DW2ET_MUTATE', 'h': '-DW5YH_MUTATE'}[_neg]]
REC10, REC12 = 0x101653e0, 0x1028cae0
helper(0x10021c3c, 'Sim_10021c3c', 1)
helper(0x10046f7f, None, 2)          # called through its original address (SIM_FN)
SARGS[0x10046f7f] = {0}
MASK[(0x10046f7f, 1)] = 0             # the record's address (1.2's block, our local image): its bytes are logged


def _save(uc, k, raw, eax):
    return [('hgcar', bytes(uc.mem_read(raw[1], 0x21c)))], 0


EFFECT[0x10046f7f] = _save
KILLS = []


def kills(m):
    if not KILLS:
        KILLS.append(m.syms['g_w34Kills'])
    return KILLS[0]


def g_rec(ctx):
    r = begin(ctx, 0x10021c3c, 0x10046f7f)
    wb(ctx, REC10, rb(ctx, 0x1d4))
    wb(ctx, kills(ctx.m), rb(ctx, 0xc8))
    return []


def g_clear(ctx):
    args = g_rec(ctx)
    r = ctx.rng
    for i in range(0x200):
        w32(ctx, 0x1096fc80 + i * 0x156, r.getrandbits(32))
    for a in (0x1015dbe0, 0x1015dbdc, 0x1015dbd8, 0x1015dbe8, 0x1015dbec, 0x1015dbe4, 0x1015dbf0):
        w32(ctx, a, r.getrandbits(32))
    return args


def to12(m):
    rec = bytes(m.uc.mem_read(REC10, 0x1d4))
    kt = bytes(m.uc.mem_read(kills(m), 0xc8))
    m.uc.mem_write(A12(REC12), rec[:0x14c] + kt + rec[0x1cc:0x1d4])
    for a10, a12, n in WIN:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, n)))
    for a10, a12 in GLOB:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, 4)))
    if not getattr(m, '_hgcar12', False):
        m._hgcar12 = True
        for a10, a12 in ((0x10021c3c, 0x1001884c), (0x10046f7f, 0x1003951f)):
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, a10), None, A12(a12), A12(a12))


def from12(m):
    r12 = bytes(m.uc.mem_read(A12(REC12), 0x21c))
    old = bytes(m.uc.mem_read(REC10, 0x1d4))
    cleared = r12 == bytes(0x21c)
    mid = bytes(0x80) if cleared else old[0x14c:0x1cc]
    m.uc.mem_write(REC10, r12[:0x14c] + mid + r12[0x214:0x21c])
    m.uc.mem_write(kills(m), r12[0x14c:0x214])
    for a10, a12, n in WIN:
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), n)))
    for a10, a12 in GLOB:
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), 4)))


WIN = [(0x1096fc80, 0x1078d0e0, 0x200 * 0x156), (0x109a5a40, 0x107b97a0, 0x100 * 0x2c)]
GLOB = [(0x1015dbe0, 0x101306f0), (0x1015dbdc, 0x101306ec), (0x1015dbd8, 0x101306e8), (0x1015dbe8, 0x101306f8),
        (0x1015dbec, 0x101306fc), (0x1015dbe4, 0x101306f4), (0x1015dbf0, 0x10130700)]


def lm(log_o, log_c):
    """1.2's file name copy (0x1010ad14) and ours (0x1013489c) hold the same text, logged by content"""
    return log_o, log_c


SPECS = [dict(name='Sim_10025d93_v12', orig12=0x1001cf11, c='Sim_10025d93', gen=g_rec, trials=300, void=True,
              to12=to12, from12=from12),
         dict(name='Sim_10021ac0_v12', orig12=0x100186d0, c='Sim_10021ac0', gen=g_clear, trials=300, void=True,
              to12=to12, from12=from12),
         dict(name='Sim_10025dac_v12', orig12=0x1001cf2a, c='Sim_10025dac', gen=g_rec, trials=1000, to12=to12,
              from12=from12)]
