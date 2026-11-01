# Round 33: Sim_10032a10 (a polygon's colour and shade) against 1.2's 0x10026610, which reads only +0x3c's low byte as
# the light (its other bytes now carry 1.2's colour, Sim_10058c5e) and, darkening, keeps +0x3c & 0xffffff. The
# spec_w9h.py generator, with colour bytes above the light in the stubbed lighting's answers.
# Negative check: W9H_NEG=1 builds the C with W9HV_MUTATE (the colour kept as & 0xffffff00).
import os
exec(open('tools/verify/spec_w9h.py').read())
exec(open('tools/verify/v12_common.py').read())
_ns = {}
exec(open('tools/verify/w10_common.py').read(), _ns)
SOURCES = _ns['closure'](list(MINE))   # spec_w9h.py's list predates rounds 24..33: the closure of its files
if os.environ.get('W9H_NEG'):
    CFLAGS = list(CFLAGS) + ['-DW9HV_MUTATE']


def g12(ctx):
    args = g_2a10(ctx)
    r = ctx.rng
    ST['shades'] = [(s & 0xff) | (r.getrandbits(24) << 8) if r.random() < 0.7 else s for s in ST['shades']]
    return args


# the vertex list's address (ours 0x101c76a0, 1.2's 0x10188d20) is not compared in the log; its entries are
MASK[(0x10058c5e, 2)] = 0
PAIRS12 = {0x100591ff: 0x1004c561, 0x10058c5e: 0x1004bad6, 0x1009c929: 0x10089e69}
_s = [s for s in SPECS if s['name'] == 'Sim_10032a10'][0]
SPECS = [v12_spec(dict(_s, gen=g12), 0x10026610, extra_pairs=PAIRS12, windows={0x101c76a0: 0x20})]
