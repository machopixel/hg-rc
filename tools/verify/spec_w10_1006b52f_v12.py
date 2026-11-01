# Round 33: Sim_1006b52f (a hit on a system cell) against 1.2's 0x1005b5c6, which sends the renderer's fog state as
# (1, 0x200000) - 1.0's code 2; 1.0 passed (0x200000, 1), a code its renderers reject. The spec_w10_1006b52f.py
# generator; 1.2's state codes are logged in 1.0's numbering (+1).
exec(open('tools/verify/spec_w10_1006b52f.py').read())
exec(open('tools/verify/v12_common.py').read())
import os
if os.environ.get('W10F_NEG'):                     # negative check: W10F_MUTATE keeps 1.0's (0x200000, 1)
    CFLAGS = list(CFLAGS) + ['-DW10F_MUTATE']
PAIRS12 = {0x1006abf6: 0x1005a9c6, 0x100203cf: 0x100176df, 0x1009b362: 0x10089262, 0x1003cbb3: 0x100306a3,
           0x1006bf33: 0x1005bfca, 0x10015630: 0x1000e7c9, 0x1017d108: 0x1014b9b0, 0x109e3d40: 0x107ee400,
           0x1014cb1c: 0x1011fa24, 0x10138d70: 0x1010e670, 0x1017b8c8: 0x1014a568, 0x1015ec58: 0x101314a0,
           0x1015d1ac: 0x1018ea7c}


def lm_states(spec):
    def lm(log_o, log_c):
        out = []
        for e in log_o:
            if e[0] == 'stub_%x' % IND1 and e[1] and isinstance(e[1][0], int):
                e = (e[0], (e[1][0] + 1,) + tuple(e[1][1:]))
            out.append(e)
        return out, log_c
    return lm


def g12(ctx):
    args = g(ctx)
    unit = r32(ctx, args[1])
    w32(ctx, unit + 0x50, r32(ctx, unit + 0x54))     # 1.2's unit holds its control block at +0x50 (1.0: +0x54)
    return args


_s = v12_spec(dict(SPECS[0], gen=g12), 0x1005b5c6, extra_pairs=PAIRS12, windows={0x10138d70: 4 * 12})
_s['log_map'] = lm_states(_s)
SPECS = [_s]
