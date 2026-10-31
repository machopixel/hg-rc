# Round 31, T2: the sky band (1.0 0x100a0def -> 1.2 0x1008dff3) in .TEX mode against 1.2's own code, on the
# round-22 spec's generator and stubs (v12_common.py; see spec_w31_sky.py for the clouds). The render-state slot
# (1.0 0x109e3d40 / 1.2 0x107ee400) gets a logging stub; 1.2's state 0xc is logged as 1.0's 0xd. The band texture's
# description (1.2 0x1018b540, handle 0x1018e550) is mapped to the C's (g_w31FileTex[0], handle [0x101cfc90]).
import os
exec(open('tools/verify/spec_w22_100a0def.py').read())
SKY_SPEC, SKY_GEN = SPECS[0], g
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/render/w22_poly.c', 'sim/render/w31_tex.c'])
if os.environ.get('W31_NEG'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW31S_MUTATE']
SARGS.setdefault(0x100ba7d5, set()).add(1)          # the loader's type as text ("TEX")
STATE = 0x1010d3e8                                   # int3 padding, as RENDER / DRIVER
helper(STATE, None, 2)
HANDLE10, HANDLE12, DESC12 = 0x101cfc90, 0x1018e550, 0x1018b540


def r32_(m, a):
    return struct.unpack('<I', bytes(m.uc.mem_read(a, 4)))[0]


def w32_(m, a, v):
    m.uc.mem_write(a, struct.pack('<I', v & 0xffffffff))


def _ctex(m):
    return m.syms['g_w31FileTex']


def file_to12(m):
    m.uc.mem_write(A12(DESC12), bytes(m.uc.mem_read(_ctex(m), 0x1c)))
    h = r32_(m, HANDLE10)
    w32_(m, A12(HANDLE12), A12(DESC12) if h == _ctex(m) else h)
    w32_(m, A12(0x10105048), 1)
    w32_(m, A12(0x1014a5f0), 0)
    w32_(m, A12(0x107ee400), STATE)


def file_from12(m):
    m.uc.mem_write(_ctex(m), bytes(m.uc.mem_read(A12(DESC12), 0x1c)))
    h = r32_(m, A12(HANDLE12))
    w32_(m, HANDLE10, _ctex(m) if h == A12(DESC12) else h)


def g2(ctx):
    args = SKY_GEN(ctx)
    r = ctx.rng
    ST['active'].add(STATE)
    w32(ctx, 0x1017b940, 1)
    w32(ctx, ctx.m.syms['g_w31Tex'], 1)
    w32(ctx, 0x109e3d40, STATE)
    w32(ctx, 0x1014cb1c, r.choice([0, 1, 2]))
    wf(ctx, 0x1012ba50, 4.0)   # the real value: 1.2 keeps its copy read-only (the 1.0 spec also tries -10.0)
    return args


s = v12_spec(dict(SKY_SPEC, gen=g2), 0x1008dff3,
             extra_pairs={0x1017d0fc: 0x1014b9a4, 0x1017d0d0: 0x1014b978, 0x1014cb1c: 0x1011fa24,
                          0x100a0880: 0x1008da61, 0x1015d1b8: 0x1012fe4c, 0x1014ceb8: 0x1011fd48},
             windows={0x1017d0d0: 0x20, 0x109e3e00: 0x14},
             skip=set(range(0x101ccc80, 0x101ccca0)) | set(range(0x101cfca0, 0x101cfcc0)) | {HANDLE10},
             to12_more=file_to12, from12_more=file_from12, name='Sim_100a0def_tex')
INV = {}
for _a10, _a12 in s['_v12_pairs'].items():
    INV.setdefault(A12(_a12), _a10)
INV[A12(HANDLE12)] = HANDLE10
INV[A12(DESC12)] = HANDLE10


def lm(log_o, log_c):
    def fix(v):
        return INV.get(v, v) if isinstance(v, int) else v
    out = []
    for e in log_o:
        if isinstance(e[1], tuple):
            vals = tuple(fix(v) for v in e[1])
            if e[0] == 'stub_%x' % STATE and vals and vals[0] == 0xc:
                vals = (0xd,) + vals[1:]
            out.append((e[0], vals))
        else:
            out.append(e)
    return out, log_c


s['log_map'] = lm
s['ignore_syms'] = {'g_w31TexLoaded': 4, 'g_w31TexFallback': 4, 'g_w31TexMissing': 4}
SPECS = [s]
