# Round 33: the group selection (1.0 0x10016632) against 1.2's 0x1000f7d8, on spec_w9d.py's generator and stubs
# (hooked at 1.2's callees too, logged under the 1.0 names). 1.2's one change: in a network game the entry of the
# list whose number is [0x1014c99c] (1.2 0x1011f8b0) is never held back. The player records hold their fields from
# +0x1c 4 bytes earlier in 1.2 (+0x1ae -> +0x1aa): shifted down by 4 before 1.2's run and back after it.
# Negative check: W9D_NEG12=1 builds the C with W9DN_MUTATE (1.0's test).
import os
exec(open('tools/verify/spec_w9d.py').read())
HELPERS = {}
exec(open('tools/verify/v12_common.py').read())
_ns = {}
exec(open('tools/verify/w10_common.py').read(), _ns)
SOURCES = _ns['closure'](['sim/unsorted_1001/w9d_select.c'])   # spec_w9d.py's list predates later rounds
USED = (0x10052aaf, 0x10012843, 0x10016cb7, 0x10016a4a, 0x10016a6b, 0x1001555c, 0x10017852)
DSTUBS = {k: v for k, v in DSTUBS.items() if v[0] in USED}        # only this function's callees are linked
STUBS = {}
if os.environ.get('W9D_NEG12'):
    CFLAGS = list(CFLAGS) + ['-DW9DN_MUTATE']

F10, F12 = 0x10016632, 0x1000f7d8
SHIFT = {}
WIN = []                 # (1.0 address, 1.2 address, length) copied to 1.2 and back


def g12(ctx):
    index, = g_6632(ctx)
    r = ctx.rng
    w32(ctx, 0x1015ec58, r.choice([1, 1, 0]))
    w32(ctx, 0x1014c99c, r.choice([-1, 0, 1, 2, 3]) & 0xffffffff)
    n = r32(ctx, 0x1015dc64)
    SHIFT.clear()
    for i in range(n):
        SHIFT[r32(ctx, 0x109e8b40 + 4 * i)] = 0x200
    del WIN[:]
    t = index * 0x4fc3a
    WIN.extend([(0x10473800 + t - 0x140, 0x10290c80 + t - 0x140, 0x900), (0x104727a0, 0x1028fc20, 0x40),
                (0x1015d120, 0x1012d4f8, 0x40), (0x109b7d48, 0x107c8ee8, 0x10 * 0x6c),
                (0x109e8b40, 0x107f24c0, 0x20)])
    return [index]


def _hooks(m, pairs):
    if getattr(m, '_w9d12', False):
        return
    m._w9d12 = True
    for addr, nargs in STUBS.items():
        if addr in pairs:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr, nargs), None, A12(pairs[addr]), A12(pairs[addr]))
    for cname, (addr, nargs) in DSTUBS.items():
        if addr in pairs:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr, nargs), None, A12(pairs[addr]), A12(pairs[addr]))


def to12_more(m):
    _hooks(m, _s['_v12_pairs'])
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


_base = [s for s in SPECS if s['name'] == 'Sim_10016632'][0]
_s = v12_spec(dict(_base, gen=g12, trials=3000), F12, to12_more=to12_more, from12_more=from12_more,
              extra_pairs={0x1014c99c: 0x1011f8b0, 0x1015ec58: 0x101314a0, 0x1017b8c8: 0x1014a568,
                           0x1015dc64: 0x10130754},
              skip=(0x109e8b40, 0x1015d120, 0x104727a0, 0x109b7d48, 0x10473800), name='Sim_10016632_v12')
SPECS = [_s]
