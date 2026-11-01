# Round 33, round 4: the sliding side panel (1.0 0x100ce586) against 1.2's 0x100b26e6, on spec_w23_100ce586.py's
# generator and stubs (hooked at 1.2's callees too, logged under the 1.0 names). 1.2 draws each line through its new
# helper 0x100b1340, the same three-way choice of text routine as 1.0 (0x100f2f01 / 0x100f1e10 / 0x100ecea6 = 1.2
# 0x100d2f4d / 0x100d2984 / 0x100cf096, stubbed there); it runs for real. The units hold their fields from +0x1c 4
# bytes earlier in 1.2 (flags +0x1c -> +0x18, name +0x124 -> +0x120): shifted down by 4 for 1.2's run, back after it;
# the ratings are copied to 1.2's table (0x107caa60).
exec(open('tools/verify/spec_w23_100ce586.py').read())
exec(open('tools/verify/v12_common.py').read())

F10, F12 = 0x100ce586, 0x100b26e6
SHIFT = {}
_g10 = g


def g12(ctx):
    args = _g10(ctx)
    SHIFT.clear()
    for i in range(8):
        u = r32(ctx, 0x109e8b40 + 4 * i)
        if u:
            SHIFT[u] = 0x140
            wb(ctx, u + 0x13b, b'\0')     # a name ends inside the record (the shift moves its last 4 bytes)
    return args


def to12_more(m):
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(0x109e8b40, 0x20)))
    m.uc.mem_write(A12(0x107caa60), bytes(m.uc.mem_read(0x1015d0f0, 0x10)))
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def from12_more(m):
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])


def lm(log_o, log_c):
    """the frame piece record and the sprintf's name pointer into a shifted unit (+0x120 in 1.2) as ours (+0x124)"""
    spans = [(b + 0x18, b + v[0] - 4) for b, v in SHIFT.items()]

    def fix(a):
        if a == A12(0x107f216c):        # the frame piece record, 1.2's copy of 0x109e878c
            return 0x109e878c
        if a == A12(0x107ca800):        # the text palette record, 1.2's copy of 0x109b9920
            return 0x109b9920
        if isinstance(a, int):
            for lo, hi in spans:
                if lo <= a < hi:
                    return a + 4
        return a
    return [(e[0], tuple(fix(a) for a in e[1])) for e in log_o], log_c


_s = v12_spec(dict(SPECS[0], gen=g12), F12, to12_more=to12_more, from12_more=from12_more,
              extra_pairs={0x100f2f01: 0x100d2f4d, 0x100f1e10: 0x100d2984, 0x100ecea6: 0x100cf096,
                           0x1017b940: 0x1014a5e8, 0x101668e8: 0x10138d80},     # the two flags the helper reads
              windows={0x10151d58: 12}, skip=(0x109e8b40, 0x1015d0f0), name='Sim_100ce586_v12')
_s['log_map'] = lm
SPECS = [_s]
