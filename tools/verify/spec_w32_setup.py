# Round 32, D: the effect pool's setup (1.0 0x100908b3 -> 1.2 0x1007986e) against 1.2's own code, on the round-8x
# spec's generator and stubs. 1.2 builds 0x406-byte records (16 particles of 0x32 bytes, the dword +0x3fa -1) and
# then also calls 0x100790be (the particle types' textures, stubbed here on both sides: Sim12_100790be in the C).
# The C runs with the Vulkan switch and the 3D mode on. 1.2's engine callees are stubbed at their 1.2 addresses
# under the 1.0 names (the aligned listings give the pairs).
import os
exec(open('tools/verify/spec_w8x.py').read())
HELPERS = {}
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W32_NEG'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW32S_MUTATE']

F10, F12 = 0x100908b3, 0x1007986e
TEX10, TEX12 = 0x100790be, 0x100790be      # the log name of the types' setup (a 1.2 function only)
EXTRA = {0x100905e8: 0x100795a3, 0x10215ae8: 0x1018ef30}


def _hooks(m, pairs):
    if getattr(m, '_w32s', False):
        return
    m._w32s = True
    for a10, nargs in list(STUBS.items()) + [(a, n) for _, (a, n) in DSTUBS.items()]:
        if a10 in pairs:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, a10, nargs), None, A12(pairs[a10]), A12(pairs[a10]))
    cb = _stub_cb(m, TEX10, 0)
    m.raw.hook_add(UC_HOOK_CODE, cb, None, A12(TEX12), A12(TEX12))
    m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms['Sim12_100790be'], m.syms['Sim12_100790be'])


def g12(ctx):
    args = g_08b3(ctx)
    m = ctx.m
    w32(ctx, 0x1017b940, 1)
    w32(ctx, m.syms['g_w31Tex'], 1)
    ST['active'].add(TEX10)
    return args


_base = [s for s in SPECS if s['name'] == 'Sim_100908b3'][0]
_s = v12_spec(dict(_base, gen=g12), F12, windows={0x10217c20: 1000 * 0x406, 0x10216b80: 1000 * 4},
              extra_pairs=EXTRA, skip=set(range(0x10217c21, 0x10217c20 + 1000 * 0x406)),
              to12_more=lambda m: _hooks(m, _s['_v12_pairs']), name='Sim_100908b3_v12')
SPECS = [_s]
