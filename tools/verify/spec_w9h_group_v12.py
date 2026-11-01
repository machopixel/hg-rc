# Tiny sweep (AI): the group orders Sim_10017263 against 1.2's 0x10010412, on spec_w9h.py's generator. 1.2's only
# changes are the unit record's fields from +0x1c sitting 4 bytes earlier (+0x28 -> +0x24, +0x1a0 -> +0x19c,
# +0x1a6 -> +0x1a2, +0x1aa -> +0x1a6, +0x1ae -> +0x1aa): the records are shifted for 1.2's run and back after it.
# The C is unchanged (it keeps 1.0's layout until the DLL goes).
# Negative check: W9HG_NEG=1 builds the C with W9HG_MUTATE.
import os
exec(open('tools/verify/spec_w9h.py').read())
exec(open('tools/verify/v12_common.py').read())
_ns = {}
exec(open('tools/verify/w10_common.py').read(), _ns)
SOURCES = _ns['closure'](list(MINE))   # spec_w9h.py's list predates rounds 24..33: the closure of its files
if os.environ.get('W9HG_NEG'):
    CFLAGS = list(CFLAGS) + ['-DW9HG_MUTATE']
SHIFT = {}
STAR12 = 0x10290c80
STAR_SPAN = 0x600       # the selected star's head and its first entries (sel <= 3)


def g12(ctx):
    args = g_7263(ctx)
    SHIFT.clear()
    for i in range(8):
        SHIFT[r32(ctx, 0x109e8b40 + 4 * i)] = 0x300
    ST['star'] = args[0]
    return args


def to12_more(m):
    s = ST['star'] * STAR_SIZE
    m.uc.mem_write(A12(STAR12) + s, bytes(m.uc.mem_read(STAR_BASE + s, STAR_SPAN)))
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def from12_more(m):
    s = ST['star'] * STAR_SIZE
    m.uc.mem_write(STAR_BASE + s, bytes(m.uc.mem_read(A12(STAR12) + s, STAR_SPAN)))
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])


# Sim_10016d83 gets a pointer to the record's word list (+0x1a0, 1.2 +0x19c): logged by its contents
MASK[(0x10016d83, 1)] = 0
LOGFN[0x10016d83] = lambda uc, raw: bytes(uc.mem_read(raw[1], 0x10))
_s = [s for s in SPECS if s['name'] == 'Sim_10017263'][0]
SPECS = [v12_spec(dict(_s, gen=g12), 0x10010412, to12_more=to12_more, from12_more=from12_more,
                  windows={0x109b7d40: 16 * 0x6c, 0x104727a0: 16 * 4, 0x109e8b40: 0x20}, skip=(STAR_BASE,))]
