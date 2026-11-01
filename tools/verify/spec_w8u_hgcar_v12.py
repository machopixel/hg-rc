# Round 33, round 3: the HGCAR.CFG write (1.0 0x10046f7f) against 1.2's 0x1003951f, on spec_w8u.py's generator and
# stubs (hooked at 1.2's callees too, logged under the 1.0 names). 1.2's one change: 0x21c bytes instead of 0x1d4
# (its record grew by the 10 x 10 kill table; see spec_w2e_hgcar_v12.py for the record itself). The path helper
# 0x100474db runs for real on both sides (1.2's twin 0x10039a7b).
# Negative check: W8U_NEG12=1 builds the C with W8UH_MUTATE (1.0's size).
import os
exec(open('tools/verify/spec_w8u.py').read())
HELPERS = {}
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W8U_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW8UH_MUTATE']

F10, F12 = 0x10046f7f, 0x1003951f
PAIRS = v12_pairs(F10, F12)


def _hooks(m):
    if getattr(m, '_w8u12', False):
        return
    m._w8u12 = True
    for addr, (nargs, kind) in STUBS.items():
        if addr in PAIRS:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr, nargs, kind), None, A12(PAIRS[addr]), A12(PAIRS[addr]))
    for cname, (addr, nargs, kind) in DSTUBS.items():
        if addr in PAIRS:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr, nargs, kind), None, A12(PAIRS[addr]), A12(PAIRS[addr]))


_base = [s for s in SPECS if s['name'] == 'Sim_10046f7f'][0]
# the path helper's buffers (it builds the file's path in 0x1015ef50, 0x100 bytes; 0x1015f050 / 0x1013b2e8 are read)
_s = v12_spec(dict(_base, trials=1000), F12, to12_more=_hooks, name='Sim_10046f7f_v12',
              extra_pairs={0x1015ef50: 0x10131728, 0x1015f050: 0x10131828, 0x1013b2e8: 0x101107a8},
              windows={0x1015ef50: 0x100, 0x1015f050: 0x100, 0x1013b2e8: 0x40})
SPECS = [_s]
