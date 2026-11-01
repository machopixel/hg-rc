# Round 33, round 3: the objectives update (1.0 0x1007c42f) against 1.2's 0x1006932f, on spec_w9f.py's generator and
# stubs (hooked at 1.2's callees too, logged under the 1.0 names). 1.2's change: the own side wins once its
# objectives are done, without 1.0's [0x1015f1e8] test in a single game and without waiting for pending script
# events ([0x1015d528]). The side table (5 blocks of their header and first 6 rows), the side entry table, the 8
# player records (shifted down by 4 for 1.2's run, back after) and the 0x109e9bec records are copied to 1.2's.
# Negative check: W9F_NEG12=1 builds the C with W9FW_MUTATE (1.0's tests).
import os
exec(open('tools/verify/spec_w9f.py').read())
exec(open('tools/verify/v12_common.py').read())
_ns = {}
exec(open('tools/verify/w10_common.py').read(), _ns)
SOURCES = _ns['closure'](['sim/unsorted_1007/w9f_objectives.c'])
if os.environ.get('W9F_NEG12'):
    CFLAGS = list(CFLAGS) + ['-DW9FW_MUTATE']

F10, F12 = 0x1007c42f, 0x1006932f
USED = (0x1007b5db, 0x1007b4d6, 0x1007b1d0, 0x100419e9, 0x1001661f)
HELPERS = {a: v for a, v in HELPERS.items() if a in USED}       # only this function's callees are linked
WIN = [(side_addr(s), 0x10290c80 + s * SIDE_SIZE, 0x3a + 6 * ROW_SIZE) for s in range(5)] + \
      [(0x104727a0, 0x1028fc20, 0x40), (PT, 0x107f24c0, 0x20), (0x109e9bec, 0x107f258c, 0x50 * 8 + 4)]
SHIFT = {}


def g12(ctx):
    args = g_c42f(ctx)
    SHIFT.clear()
    for i in range(8):
        SHIFT[r32(ctx, PT + 4 * i)] = 0x200
    return args


def to12_more(m):
    for a10, a12, n in WIN:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, n)))
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def from12_more(m):
    for a10, a12, n in WIN:
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), n)))
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])


_base = [s for s in SPECS if s['name'] == 'Sim_1007c42f'][0]
_s = v12_spec(dict(_base, gen=g12, trials=3000), F12, to12_more=to12_more, from12_more=from12_more,
              name='Sim_1007c42f_v12')
SPECS = [_s]
