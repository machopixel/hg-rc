# Round 33, round 3: the in-mission / mission-end menu (1.0 0x10048f02, converted from 1.2's 0x1003ad72 as
# Sim_10048f02, w35_menu.c) against 1.2's own code. Every callee is a logging stub on both sides (the key answers
# chosen by the generator); the new reset Sim12_10089c60 runs for real (1.2's 0x10089c60). The player records
# hold their fields from +0x1c 4 bytes earlier in 1.2 (+0x28 unit -> +0x24, +0x168 -> +0x164): shifted down by 4 for
# 1.2's run, back after it; the unit's object (+0x150 -> +0x14c) too. The key's upper half, left over in 1.2's
# registers, is not compared (the callees read a word).
# Negative checks: W35_NEG12=c builds the C with W35C_MUTATE (1.0's clear of the settings byte), =p with W35P_MUTATE
# (1.0's test without the pending flag), =r with W35R_MUTATE (no reset).
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1004/w35_menu.c'])
_neg = os.environ.get('W35_NEG12')
if _neg:
    CFLAGS = list(globals().get('CFLAGS', [])) + [{'c': '-DW35C_MUTATE', 'p': '-DW35P_MUTATE',
                                                   'r': '-DW35R_MUTATE'}[_neg]]
F10, F12 = 0x10048f02, 0x1003ad72
CALLS = [(0x1001fee5, 'Sim_1001fee5', 1), (0x1007df6c, 'Sim_1007df6c', 4), (0x10044a69, 'Sim_10044a69', 2),
         (0x1003cbb3, 'Sim_1003cbb3', 2), (0x10077fff, 'Sim_10077fff', 0), (0x10078177, 'Sim_10078177', 0),
         (0x1006abf6, 'Sim_1006abf6', 3), (0x100488a3, 'Sim_100488a3', 1), (0x10048c84, None, 1),
         (0x1005572c, 'Sim_1005572c', 1), (0x1004954e, 'Sim_1004954e', 1), (0x10074bfa, 'Sim_10074bfa', 1),
         (0x10040ab8, 'Sim_10040ab8', 1), (0x1009d200, None, 4), (0x100448cd, 'Sim_100448cd', 1),
         (0x100fc780, None, 2)]
for _a, _c, _n in CALLS:
    helper(_a, _c, _n)
for _a in (0x100488a3, 0x10048c84, 0x1005572c, 0x1004954e):
    MASK[(_a, 0)] = 0xffff
SARGS[0x100fc780] = {1}
SARGS[0x1007df6c] = {0, 1}         # texts by content (1.2 has its own copies)
MASK[(0x100fc780, 0)] = 0           # our text buffer and 1.2's are on different stack slots
ACTIVE = tuple(a for a, _, _ in CALLS)
SHIFT = {}
G = [0x1017b92c, 0x1015f1d4, 0x1015dc78, 0x1015f1d0, 0x1015f1e4, 0x1015f1fc, 0x1015ebf8, 0x10179cd4, 0x10151d50,
     0x1014cb1c, 0x1015ecd0, 0x1015f208, 0x1017b8b8, 0x1017b8bc, 0x10144fa0, 0x1015ef10, 0x1015ec58, 0x1015f200,
     0x1015d530, 0x101c45c8, 0x101c45c4, 0x109b98c0, 0x101c47f0, 0x1015d1ac, 0x1015d1cc, 0x1015d098]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    now = r.randrange(100, 100000)
    w32(ctx, 0x1015dc78, now)
    w32(ctx, 0x1017b92c, r.choice([0, 0, 0, 0, 1]))
    w32(ctx, 0x1015f1d4, r.choice([0, 0, now - 1, now + 5, -3 & 0xffffffff]))
    w32(ctx, 0x1015f1d0, r.choice([0, 0, now - 1, now - 1, now + 9, 0xffffffff]))
    for a in (0x1015f1e4, 0x1015f1fc, 0x1015ebf8, 0x10179cd4, 0x1015ef10, 0x1015ec58, 0x1015f200):
        w32(ctx, a, r.choice([0, 0, 1]))
    w32(ctx, 0x1015ecd0, r.choice([0, 1, 2, 2]))
    w32(ctx, 0x1015f208, r.choice([0, 1, 2]))
    w32(ctx, 0x10151d50, r.choice([0xfffffffe, 0, 5]))
    for a in (0x1014cb1c, 0x1017b8b8, 0x1017b8bc, 0x10144fa0, 0x1015d530, 0x1015d1ac, 0x1015d1cc, 0x1015d098):
        w32(ctx, a, r.getrandbits(32))
    for a in (0x101c45c8, 0x101c45c4, 0x109b98c0, 0x101c47f0):
        w32(ctx, a, ctx.buf(16, b'TEXT%d\0' % r.randrange(10)))
    for a in (0x1015fdba, 0x1015fdbd, 0x1015fdd0):
        w8(ctx, a, r.getrandbits(8))
    cfg = newbuf(ctx, 0x20)
    w8(ctx, cfg + 8, r.choice([0, 1, 1]))
    w8(ctx, cfg + 0xa, r.choice([0, 1]))
    w32(ctx, 0x1017b8c4, cfg)
    me = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, me)
    SHIFT.clear()
    for i in range(4):
        rec = newbuf(ctx, 0x200)
        lives = newbuf(ctx, 4)
        w8(ctx, lives, r.choice([0, 0, 1, 3]))
        w32(ctx, rec + 0x168, lives)
        unit = newbuf(ctx, 0xc50)
        obj = newbuf(ctx, 0x200)
        w32(ctx, unit, obj)
        w32(ctx, obj + 0x150, r.getrandbits(32))
        w32(ctx, rec + 0x28, unit)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
        SHIFT[rec] = 0x200
        SHIFT[obj] = 0x200
    w16(ctx, 0x1015fd4c, r.getrandbits(16))
    rets(0x10048c84, r.choice([0, 0, 0, 1]))
    rets(0x1005572c, r.choice([0, 0x4b, 0x6b, 0x6b, 0x76, 0x12]))
    return [0, 0, 0]


def to12_more(m):
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(0x109e8b40, 0x10)))
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def from12_more(m):
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])


SPECS = [v12_spec(dict(name='Sim_10048f02', orig=F10, c='Sim_10048f02', gen=g, trials=4000, void=True), F12,
                  to12_more=to12_more, from12_more=from12_more, skip=(0x109e8b40,),
                  extra_pairs={0x1015d1ac: 0x1018ea7c, 0x1015d1cc: 0x1018ea84, 0x1015d098: 0x1018ea74,
                               0x101c45c4: 0x10186f58, 0x101c47f0: 0x10187184, 0x1015fd4c: 0x101334ec},
                  name='Sim_10048f02_v12')]
