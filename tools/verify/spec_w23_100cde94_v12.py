# Round 33, round 4: the F12 objectives screen (1.0 0x100cde94) against 1.2's 0x100b21ce, on spec_w23_100cde94.py's
# generator and stubs (hooked at 1.2's callees too, logged under the 1.0 names). 1.2 draws each line through its
# helper 0x100b1340 (runs for real: the same three-way choice of text routine, 1.2 0x100d2f4d / 0x100d2984 /
# 0x100cf096, stubbed there). The units hold their fields from +0x1c 4 bytes earlier in 1.2: shifted down by 4 for
# 1.2's run, back after it; the mission status records (0x10473800, 0x4fc3a bytes each) are copied to 1.2's
# (0x10290c80) and back, the ratings to 1.2's table (0x107caa60).
exec(open('tools/verify/spec_w23_100cde94.py').read())
exec(open('tools/verify/v12_common.py').read())

F10, F12 = 0x100cde94, 0x100b21ce
SHIFT = {}
_g10 = g


def g12(ctx):
    args = _g10(ctx)
    SHIFT.clear()
    for i in range(8):
        u = r32(ctx, 0x109e8b40 + 4 * i)
        if u:
            SHIFT[u] = 0x140
    return args


def to12_more(m):
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(0x109e8b40, 0x20)))
    m.uc.mem_write(A12(0x107caa60), bytes(m.uc.mem_read(0x1015d0f0, 0x10)))
    for k in range(2):
        m.uc.mem_write(A12(0x10290c80) + k * 0x4fc3a, bytes(m.uc.mem_read(0x10473800 + k * 0x4fc3a, 0x40)))
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def from12_more(m):
    for k in range(2):
        m.uc.mem_write(0x10473800 + k * 0x4fc3a, bytes(m.uc.mem_read(A12(0x10290c80) + k * 0x4fc3a, 0x40)))
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])


def lm(log_o, log_c):
    """1.2's copies of the text palette record and pointers into a shifted unit (+0x120 in 1.2) as ours"""
    spans = [(b + 0x18, b + v[0] - 4) for b, v in SHIFT.items()]

    def fix(a):
        if a == A12(0x107ca800):
            return 0x109b9920
        if isinstance(a, int):
            for lo, hi in spans:
                if lo <= a < hi:
                    return a + 4
        return a
    return [(e[0], tuple(fix(a) for a in e[1])) for e in log_o], log_c


_s = v12_spec(dict(SPECS[0], gen=g12), F12, to12_more=to12_more, from12_more=from12_more,
              extra_pairs={0x100f2f01: 0x100d2f4d, 0x100f1e10: 0x100d2984, 0x100ecea6: 0x100cf096,
                           0x1017b940: 0x1014a5e8, 0x101668e8: 0x10138d80,     # the two flags the helper reads
                           # the listing pairing slips on this function: callees, texts and formats set by hand
                           0x100ccc60: 0x100b15fb, 0x1008d6f7: 0x10076a77, 0x100cca20: 0x100b13bb,
                           0x100ccaa0: 0x100b143b, 0x1015dc78: 0x10130768, 0x101c4c4c: 0x101875ec,
                           0x101c4978: 0x10187318, 0x101c4c38: 0x101875d8, 0x101c4c3c: 0x101875dc,
                           0x101c4bf4: 0x10187594, 0x101c4bf0: 0x10187590, 0x10151df0: 0x10123e48,
                           0x10151df8: 0x10123e50, 0x10151e00: 0x10123e58, 0x10151e10: 0x10123e68,
                           0x10151e08: 0x10123e60, 0x10151e18: 0x10123e70, 0x1017e5e0: 0x1014cb00,
                           0x1013bb40: 0x10110e58, 0x109b9920: 0x107ca800},
              skip=(0x109e8b40, 0x1015d0f0, 0x10151df0, 0x10151df8, 0x10151e00, 0x10151e10, 0x10151e08, 0x10151e18),
              name='Sim_100cde94_v12')     # the formats are compared as text, not copied
_s['log_map'] = lm
SPECS = [_s]
