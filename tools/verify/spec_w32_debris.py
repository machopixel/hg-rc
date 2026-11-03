# Round 32, F: the debris piece update (1.0 0x10029421 -> 1.2 0x1001ef21) against 1.2's own code, on the round-8u
# spec's generator and stubs (hooked at 1.2's callees too, logged under the 1.0 names). 1.2's one change: a piece that
# has not come to rest gets smoke along this frame's move (the smoke spawn 0x1007f8a9, stubbed on both sides: its
# arguments are compared). The C runs with the Vulkan switch and the 3D mode on.
# Negative check: W32_NEG=1 builds the C with W32D2_MUTATE (the smoke's direction from the new position only).
import os
exec(open('tools/verify/spec_w8u.py').read())
HELPERS = {}
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W32_NEG'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW32D2_MUTATE']

F10, F12 = 0x10029421, 0x1001ef21
SMOKE12 = A12(0x1007f8a9)
DSTUBS['Sim12_1007f8a9'] = (SMOKE12, 6, 'i')
SOURCES = SOURCES + [x for x in ['sim/render/w31_tex.c', 'sim/render/w33_mode.c', '../tools/verify/w32_smoke_stub.c'] if x not in SOURCES]


def _hooks(m, pairs):
    if getattr(m, '_w32d', False):
        return
    m._w32d = True
    for addr, (nargs, kind) in STUBS.items():
        if addr in pairs:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr, nargs, kind), None, A12(pairs[addr]), A12(pairs[addr]))
    for cname, (addr, nargs, kind) in DSTUBS.items():
        if addr in pairs:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr, nargs, kind), None, A12(pairs[addr]), A12(pairs[addr]))


def g12(ctx):
    args = g_29421(ctx)
    w32(ctx, 0x1017b940, 1)
    w32(ctx, ctx.m.syms['g_w31Tex'], 1)
    ST['active'].add(SMOKE12)
    return args


_base = [s for s in SPECS if s['name'] == 'Sim_10029421'][0]
_s = v12_spec(dict(_base, gen=g12, trials=3000), F12, windows={0x109eeb00: 0x30 * 0x100},
              skip=set(range(0x109eeb01, 0x109eeb00 + 0x30 * 0x100)),
              to12_more=lambda m: _hooks(m, _s['_v12_pairs']), name='Sim_10029421_v12')
SPECS = [_s]
