# 1.2 audit port: the current-objective line (1.0 0x100cd70f) against 1.2's 0x100b1c8f, on spec_w9g.py's generator.
# 1.2's change ported: the objective is copied whole into its own buffer, wraps only when longer than the width
# (1.0: also when equal) and always breaks at the objective's last space (the round-21 stack-leftover wrap is gone).
# 1.2 draws through its helper 0x100b1340 (the same three-way choice of text routine, run for real); the three text
# routines, the font height and the label width are stubbed at their 1.2 twins. Objectives without any space in a
# long text are left out: 1.2 then cuts at a NULL search result (the C keeps the width there, a deviation).
# The objective table of the chosen star (1.0 0x10473800 + star * 0x4fc3a, 1.2 0x10290c80) is copied across.
# Negative check: W9GW_NEG12=1 builds the C with W9GW_MUTATE (1.0's `>=` width test).
import os
exec(open('tools/verify/w10_common.py').read().split('\ndef helper(')[0])     # closure() and its helpers only
_closure = closure
exec(open('tools/verify/spec_w9g.py').read())
exec(open('tools/verify/v12_common.py').read())
SOURCES = _closure(MINE + OTHERS)     # spec_w9g.py's fixed list predates rounds 31..33: its closure
if os.environ.get('W9GW_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW9GW_MUTATE']

STAR12 = 0x10290c80
SPAN = 0x3a + 8 * 0x13f + 0x40


def g12(ctx):
    args = g_d70f(ctx)
    star = r32(ctx, 0x1013bb40)
    base = STAR_BASE + star * STAR_SIZE
    k = r32(ctx, 0x101c64f0)
    cut = r32(ctx, 0x10151d60 + 4 * k)
    for i in range(r32(ctx, base)):
        e = entry(star, i)
        s = bytes(ctx.m.uc.mem_read(e + 0xad, 0x92)).split(b'\0')[0]
        if len(s) > cut and b' ' not in s:      # no space at all: 1.2's NULL cut (not provable)
            ctx.m.uc.mem_write(e + 0xad + len(s) // 2, b' ')
    ST['span'] = (base, STAR12 + star * STAR_SIZE)
    return args


def to12_more(m):
    a10, a12 = ST['span']
    m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, SPAN)))
    m.uc.mem_write(A12(0x10123db8), bytes(m.uc.mem_read(0x10151d60, 12)))


def from12_more(m):
    a10, a12 = ST['span']
    m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), SPAN)))


def lm(log_o, log_c):
    """1.2's text-attribute record [0x107ca800] passed as 1.0's 0x109b9920"""
    out = []
    for e in log_o:
        if isinstance(e[1], tuple):
            e = (e[0], tuple(0x109b9920 if v == A12(0x107ca800) else v for v in e[1]))
        out.append(e)
    return out, log_c


_s = v12_spec(dict(name='Sim_100cd70f', orig=0x100cd70f, c='Sim_100cd70f', gen=g12, trials=3000, void=True),
              0x100b1c8f, to12_more=to12_more, from12_more=from12_more,
              extra_pairs={0x100ecea6: 0x100cf096, 0x100f1e10: 0x100d2984, 0x100f2f01: 0x100d2f4d,
                           0x100ecce0: 0x100ceed0, 0x100ccb0d: 0x100b14a8, 0x1013bb40: 0x10110e58,
                           0x101c64f0: 0x10188b44, 0x1015ec58: 0x101314a0, 0x1017b8c4: 0x1014a564,
                           0x101668e8: 0x10138d80, 0x1017b940: 0x1014a5e8, 0x101c4994: 0x10187334,
                           0x101c498c: 0x1018732c, 0x101c4988: 0x10187328, 0x101c49a8: 0x10187348,
                           0x109b992e: 0x107ca80e})
_s['log_map'] = lm
SPECS = [_s]
