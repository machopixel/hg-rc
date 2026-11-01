# Round 33: the dispatcher's command 0xc5 (the display-kind toggle, [0x1014cb1c]) against 1.2's 0x1003b3ba: 1.2 sends
# the renderer's fog state as (1, 0x200000) / (1, 0) - 1.0's code 2; 1.0 passed (0x200000, 1/0), a code its
# renderers reject - and in its 3DFX / PowerVR mode (g_w33Voodoo = 1.2 [0x1014b954]) state 0xe (1.0's 0xf) on / off.
# The spec_w14_1004954e.py generator with the command fixed to 0xc5 and the network off (1.2 refuses most commands in
# a network game, 1.0 does not: not ported here). 1.2's state codes are logged in 1.0's numbering (+1).
# Negative check: W14_NEG12=1 builds the C with W14KV_MUTATE (1.0's (0x200000, 0) kept).
import os
exec(open('tools/verify/spec_w14_1004954e.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W14_NEG12'):
    CFLAGS = list(CFLAGS) + ['-DW14KV_MUTATE']
PAIRS12 = {0x100654a1: 0x10056a61, 0x100203cf: 0x100176df, 0x1014cb1c: 0x1011fa24, 0x1014cb20: 0x1011fa28,
           0x1017d108: 0x1014b9b0, 0x109e3d40: 0x107ee400, 0x1017b8c8: 0x1014a568,
           0x1015ec58: 0x101314a0}


def to12_more(m):
    m.uc.mem_write(A12(0x1014b954), bytes(m.uc.mem_read(m.syms['g_w33Voodoo'], 4)))
    # 1.2's player records hold the same fields 4 bytes earlier (unit +0x24, flags +0x18): its table points 4 in
    for i in range(4):
        rec = struct.unpack('<I', bytes(m.uc.mem_read(0x109e8b40 + 4 * i, 4)))[0]
        m.uc.mem_write(A12(0x107f24c0) + 4 * i, struct.pack('<I', rec + 4))


def g12(ctx):
    g(ctx)
    w32(ctx, 0x1015ec58, 0)
    w32(ctx, ctx.m.syms['g_w33Voodoo'], ctx.rng.choice([0, 1]))
    return [0xc5]


def lm_states(spec):
    def lm(log_o, log_c):
        out = []
        for e in log_o:
            if e[0] == 'stub_%x' % IND1 and e[1] and isinstance(e[1][0], int):
                e = (e[0], (e[1][0] + 1,) + tuple(e[1][1:]))
            out.append(e)
        return out, log_c
    return lm


_s = v12_spec(dict(SPECS[0], gen=g12, trials=3000), 0x1003b3ba, extra_pairs=PAIRS12, to12_more=to12_more,
              skip=(0x109e8b40,))
_s['log_map'] = lm_states(_s)
SPECS = [_s]
