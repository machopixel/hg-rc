# Round 31, T2: the cloud layer (1.0 0x100a0880 -> 1.2 0x1008da61) and the sky band (1.0 0x100a0def -> 1.2
# 0x1008dff3) in .TEX mode against 1.2's own code, on the round-22 specs' generators and stubs (v12_common.py:
# the listings' aligned addresses map the globals and stubs). The C runs with the Vulkan switch and the 3D mode
# on; 1.2's file texture descriptions (0x1018e560 / 0x1018b540, handles 0x1018e580 / 0x1018e550) are mapped to
# the C's (g_w31FileTex[1] / [0], handles [0x101cfcb8] / [0x101cfc90]); the cache loader logs its type as text.
# 1.2's render state call before the clouds (its 0xc, the C's 0xd in 1.0's numbering) is logged by both.
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())
_ns = {}
exec(open('tools/verify/spec_w22_100a0880.py').read())
CLOUD_SPEC, CLOUD_GEN = SPECS[0], g
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/render/w22_poly.c', 'sim/render/w31_tex.c'])
if os.environ.get('W31_NEG'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW31S_MUTATE']
SARGS.setdefault(0x100ba7d5, set()).add(1)          # the loader's type as text ("TEX")

SLOT_STATE = 0x109e3d40
FILE = [(0x101cfc90, 0x1018e550, 0x1018b540), (0x101cfcb8, 0x1018e580, 0x1018e560)]  # handle 1.0, handle 1.2, desc 1.2


def _ctex(m, k):
    return m.syms['g_w31FileTex'] + k * 0x1c


def file_to12(m):
    for k, (h10, h12, d12) in enumerate(FILE):
        m.uc.mem_write(A12(d12), bytes(m.uc.mem_read(_ctex(m, k), 0x1c)))
        h = r32_(m, h10)
        w32_(m, A12(h12), A12(d12) if h == _ctex(m, k) else h)
    w32_(m, A12(0x10105048), 1)
    w32_(m, A12(0x1014a5f0), 0)


def file_from12(m):
    for k, (h10, h12, d12) in enumerate(FILE):
        m.uc.mem_write(_ctex(m, k), bytes(m.uc.mem_read(A12(d12), 0x1c)))
        h = r32_(m, A12(h12))
        w32_(m, h10, _ctex(m, k) if h == A12(d12) else h)


def r32_(m, a):
    return struct.unpack('<I', bytes(m.uc.mem_read(a, 4)))[0]


def w32_(m, a, v):
    m.uc.mem_write(a, struct.pack('<I', v & 0xffffffff))


def tex_mode(gen):
    def g2(ctx):
        args = gen(ctx)
        m = ctx.m
        w32(ctx, 0x1017b940, 1)
        w32(ctx, m.syms['g_w31Tex'], 1)
        return args
    return g2


def mk(spec, f12, gen, extra, windows, name):
    s = v12_spec(dict(spec, gen=tex_mode(gen)), f12, windows=windows, extra_pairs=extra,
                 skip=set(range(0x101ccc80, 0x101ccca0)) | set(range(0x101cfca0, 0x101cfcc0)) | {0x101cfc90},
                 to12_more=file_to12, from12_more=file_from12, name=name)
    inv = {}
    for a10, a12 in s['_v12_pairs'].items():
        inv.setdefault(A12(a12), a10)
    for k, (h10, h12, d12) in enumerate(FILE):
        inv[A12(h12)] = h10
        inv[A12(d12)] = h10

    def lm(log_o, log_c):
        def fix(v):
            return inv.get(v, v) if isinstance(v, int) else v
        out = []
        for e in log_o:
            out.append((e[0], tuple(fix(v) for v in e[1])) if isinstance(e[1], tuple) else e)
        return out, log_c
    s['log_map'] = lm
    s['ignore_syms'] = {'g_w31TexLoaded': 4, 'g_w31TexFallback': 4, 'g_w31TexMissing': 4}   # the C's log counts
    return s


SPECS = [
    mk(CLOUD_SPEC, 0x1008da61, CLOUD_GEN, {0x1017d100: 0x1014b9a8, 0x1017d0b0: 0x1014b958},
       {0x1017d0b0: 0x20}, 'Sim_100a0880_tex'),
]
