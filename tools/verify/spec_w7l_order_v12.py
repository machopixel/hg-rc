# Tiny sweep (controls and AI): the order-target helper (1.0 0x10015018, Sim_10015018) against 1.2's 0x1000e1a8,
# on spec_w7l.py's generator (rewritten for w10_common's stubs). 1.2's change: selector 0x4200 also takes a unit
# whose state (Sim_10052be7) is 0 in a network game ([0x1015ec58]; 1.2 also tests its [0x1012d4e8], which nothing
# writes); 1.0 tested [0x1015d110], which nothing writes. The four callees are logging stubs on both sides
# (Sim_10052c60 = 1.2 0x10042b80, Sim_10052be7 = 0x10042b07, Sim_10052aaf = 0x100429cf, 0x1007cbcb = 0x10069aac).
# The object holds its fields from +0x1c 4 bytes earlier in 1.2 (+0x1d6 -> +0x1d2): shifted for 1.2's run, back
# after it. Only ax of the answer is compared (the callers read a word).
# Negative check: W7LO_NEG12=1 builds the C with W7LO_MUTATE (1.0's flag).
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1001/w7l_misc.c'])
if os.environ.get('W7LO_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW7LO_MUTATE']
CALLS = [(0x10052c60, 'Sim_10052c60', 2), (0x10052be7, 'Sim_10052be7', 1), (0x10052aaf, 'Sim_10052aaf', 1),
         (0x1007cbcb, None, 1)]
for _a, _c, _n in CALLS:
    helper(_a, _c, _n)
ACTIVE = tuple(a for a, _, _ in CALLS)
SHIFT = {}


def g(ctx):
    r = begin(ctx, *ACTIVE)
    n = r.choice([1, 3, 6, 10])
    w32(ctx, 0x1015dc64, n)
    for i in range(8):
        rec = newbuf(ctx, 0x40) if i < n else 0
        w32(ctx, 0x109e8b40 + 4 * i, rec)
        if rec:
            w32(ctx, rec + 8, r.getrandbits(32))
    obj = newbuf(ctx, 0x200)
    sel = r.choice([0x2100, 0x2101, 0x2200, 0x2201, 0x2202, 0x4200, 0x4200, 0x4200, 0x4201, 0x4202, 0x4300 | 0x200,
                    0x200, 0x2500, 0x100, 0x2400, 0x0123, 0x6201, r.getrandbits(16), 0x0f00 | r.getrandbits(8)])
    if r.random() < 0.2:
        sel |= r.choice([0x8000, 0x0300, 0x0c00])
    key = r.choice([-1, -1, 0, 1, 2, 5, 0x123, -2, r.getrandbits(16)])
    w32(ctx, 0x1017b8c8, r.choice([0, 1, 2, 0xffff, 0x10123, r.getrandbits(32)]))
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    w16(ctx, obj + 0x1d6, r.choice([0xffff, 0x12, 0x312, r.getrandbits(16)]))
    w16(ctx, obj + 4, r.choice([0xffff, 0x12, 0x312, r.getrandbits(16)]))
    w32(ctx, obj + 8, r.getrandbits(32))
    for a in (0x10052c60, 0x10052be7):
        rets(a, *[r.choice([0, 0, 1, 2, 3]) for _ in range(5)])
    rets(0x10052aaf, *[r.choice([0, 1, 2, 0xffff, 0xffffffff, 0x1ffff, r.getrandbits(32)]) for _ in range(4)])
    rets(0x1007cbcb, r.choice([0, 3, 0xffff, 0x1ffff, 0x12345, 0x300]))
    SHIFT.clear()
    SHIFT[obj] = 0x200
    return [obj, sel | (r.getrandbits(16) << 16), (key & 0xffff) | (r.getrandbits(16) << 16)]


def to12_more(m):
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(0x109e8b40, 0x20)))
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def from12_more(m):
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])


SPECS = [v12_spec(dict(name='Sim_10015018', orig=0x10015018, c='Sim_10015018', gen=g, trials=4000,
                       retmask=0xffff), 0x1000e1a8, to12_more=to12_more, from12_more=from12_more,
                  extra_pairs={0x10052c60: 0x10042b80, 0x10052be7: 0x10042b07, 0x10052aaf: 0x100429cf,
                               0x1007cbcb: 0x10069aac, 0x1017b8c8: 0x1014a568, 0x1015dc64: 0x10130754,
                               0x1015ec58: 0x101314a0},
                  skip=(0x109e8b40, 0x1015d110), name='Sim_10015018_v12')]     # 0x1015d110: the listing pairs it with 1.2's network flag
