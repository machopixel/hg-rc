# Round 31, T1: 1.2's .TEX texture path (src/sim/render/w31_tex.c and its users) against 1.2's own code
# (GameRip_12/hgsim.dll, run relocated by difftest: 'orig12'). The C runs in .TEX mode (g_w31Tex, [0x1017b940]);
# to12 copies the state it reads from the 1.0 / C side to 1.2's addresses, from12 copies 1.2's results back, so
# the checker's comparison sees 1.2's results where the C's are. 1.2's texture table is 0x10836d20 (ours:
# g_w31TexNames, same layout); its +0x14 is ours "image is a .TEX file", which 1.2 does not keep: from12 sets it
# for every frame with an image (all images of these specs are .TEX files).
#
# Stubs shared by both sides: the cache loader (1.0 0x100ba7d5 / 1.2 0x100a2bc9: images by name and type from a
# per-trial file set), the renderer's Render and PreloadTexture slots (1.0 0x109e3d30 / 0x109e3d54, 1.2 0x107ee3f0 /
# 0x107ee414: the call and the texture description logged), the clock (C: Plat_TicksMs, 1.2: timeGetTime).
# Mapped in the logs: 1.2's game palette 0x10152a60 and plugin record 0x107ee440 as 1.0's 0x10185760 / 0x109e3dc0;
# 1.2's Render mode 8 as 1.0's 6 (1.2's renderers renumbered the modes: DECISIONS.md, round 31). The C's .XEL
# loads after a missing .TEX (Diego's pick 1a, a fix of 1.2's missing textures) are left out of its log: 1.2 makes
# none, and the spec gives no .XEL either. W31_GetTexImage_fallback checks that fallback against 1.0's getter.
#
# W31_NEG=1 builds the C with W31T_MUTATE (the row mirror leaves the last row alone).
import os, struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_EIP, UC_X86_REG_FPCW

exec(open('tools/verify/w31_map.py').read())

SOURCES = ['sim/render/w31_tex.c', 'sim/render/w2m_texture.c', 'sim/render/w2n_texture.c',
           'sim/render/w3q_texture.c', 'sim/unsorted_1000/w2h_reset.c', 'sim/unsorted_100f/w2e_string.c',
           '../tools/verify/plat_stub.c', 'sim/render/w33_dx12.c']
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']
if os.environ.get('W31_NEG'):
    CFLAGS.append('-DW31T_MUTATE')
if os.environ.get('W33V_NEG'):
    CFLAGS.append('-DW2NV_MUTATE')

BASE12 = 0x20000000


def A12(a):
    return BASE12 + a - 0x10000000


STUB_RENDER, STUB_PRELOAD = 0x00F00A00, 0x00F00A10
LOAD10, LOAD12 = 0x100ba7d5, A12(0x100a2bc9)
TAB12, REC10, REC12 = A12(0x10836d20), 0x10ac1b00, A12(0x108b6d60)
PAL10, PAL12 = 0x10185760, A12(0x10152a60)
PLUG10, PLUG12 = 0x109e3dc0, A12(0x107ee440)
# dwords the functions read or write, 1.0 address -> 1.2 address
DWORDS = [(0x1014ceb8, 0x1011fd48), (0x1017b940, 0x1014a5e8), (0x1015ec58, 0x101314a0), (0x1012da30, 0x10105040),
          (0x1015cfe0, 0x1012d3e0), (0x1015cfe4, 0x1012d3e4), (0x1015cfe8, 0x1012d3e8), (0x1014cb38, 0x1011fa40),
          (0x10143fdc, 0x10118198), (0x10143fe0, 0x1011819c), (0x10143fe4, 0x101181a0), (0x1012da34, 0x10105044)]
ST = {}


def r32(uc, a):
    return struct.unpack('<I', bytes(uc.mem_read(a, 4)))[0]


def w32(uc, a, v):
    uc.mem_write(a, struct.pack('<I', v & 0xffffffff))


def cstr(uc, a, n=24):
    try:
        return bytes(uc.mem_read(a, n)).split(b'\0')[0].upper()
    except Exception:
        return b'?'


def ret(uc, value):
    esp = uc.reg_read(UC_X86_REG_ESP)
    uc.reg_write(UC_X86_REG_EAX, value & 0xffffffff)
    uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
    uc.reg_write(UC_X86_REG_ESP, esp + 4)


def mapv(v):
    return {PAL12: PAL10, PLUG12: PLUG10}.get(v, v)


def desc(uc, handle_ptr, with_key):
    h = r32(uc, handle_ptr)
    w, hh, pix, pal, key = struct.unpack('<iiIIi', bytes(uc.mem_read(h, 20)))
    flag = r32(uc, h + 0x18)
    return (w, hh, pix, mapv(pal), key if with_key else '-', flag)


def ticks(m):
    n = sum(1 for e in m.api_log if e[0] == 'timeGetTime')
    seq = ST['ticks']
    return seq[min(n, len(seq) - 1)]


def install(m):
    if getattr(m, '_w31', False):
        return
    m._w31 = True
    uc = m.uc

    def on_load(uc_, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        name, typ = cstr(uc, r32(uc, esp + 4)), cstr(uc, r32(uc, esp + 8), 8)
        m.api_log.append(('load', (name, typ)))
        ret(uc, ST['files'].get((name, typ), 0))

    def on_render(uc_, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        mode, surf, verts, count, ph = struct.unpack('<5I', bytes(uc.mem_read(esp + 4, 20)))
        mode = 6 if mode == 8 else mode
        m.api_log.append(('render', (mode, mapv(surf), verts, count) + desc(uc, ph, mode == 6)))
        ret(uc, ST['rret'])

    def on_preload(uc_, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        ph, key = struct.unpack('<2I', bytes(uc.mem_read(esp + 4, 8)))
        m.api_log.append(('preload', desc(uc, ph, True) + (key,)))
        ret(uc, 1)

    def on_ticks(uc_, address, size, ud):
        v = ticks(m)
        m.api_log.append(('timeGetTime', ()))
        ret(uc, v)

    for a in (LOAD10, LOAD12):
        m.raw.hook_add(UC_HOOK_CODE, on_load, None, a, a)
    m.raw.hook_add(UC_HOOK_CODE, on_render, None, STUB_RENDER, STUB_RENDER)
    m.raw.hook_add(UC_HOOK_CODE, on_preload, None, STUB_PRELOAD, STUB_PRELOAD)
    m.raw.hook_add(UC_HOOK_CODE, on_ticks, None, m.syms['Plat_TicksMs'], m.syms['Plat_TicksMs'])


API = {'timeGetTime': lambda n, args: ticks_api()}


def ticks_api():
    m = ST['m']
    n = sum(1 for e in m.api_log if e[0] == 'timeGetTime') - 1     # the hook logged this call already
    seq = ST['ticks']
    return seq[min(n, len(seq) - 1)]


# ---- state ---------------------------------------------------------------
def tex_image(ctx):
    r = ctx.rng
    w = r.choice([2, 4, 8, 16, 32, 64, 3])
    h = r.choice([1, 2, 4, 8, 5])
    n = r.choice([1, 22, 200, 256])
    body = bytes(r.getrandbits(8) for _ in range(4 * n + w * h))
    return ctx.buf(12 + len(body), b'1.22' + struct.pack('<4h', r.choice([0, 3]), w, h, n) + body)


def begin(ctx):
    m, r = ctx.m, ctx.rng
    ST['m'] = m
    install(m)
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    sym = m.syms
    w32(m.uc, sym['g_w31Tex'], 1)
    w32(m.uc, 0x1017b940, 1)
    ST['rret'] = r.choice([0, 1])
    t0 = r.getrandbits(32)
    w32(m.uc, sym['g_w31TexStart'], t0)
    step = r.choice([0, 1, 50, 2000, 10000])
    ST['ticks'] = [(t0 + step * k) & 0xffffffff for k in range(64)]
    w32(m.uc, 0x109e3d30, STUB_RENDER)
    w32(m.uc, 0x109e3d54, STUB_PRELOAD)
    m.uc.mem_write(STUB_RENDER, b'\xc3' * 0x20)
    xel = ctx.buf(8, b'XEL\0\0\0\0\0')
    w32(m.uc, 0x1014ceb8, xel)
    w32(m.uc, 0x1012da34, 0)
    for a in (0x10143fdc, 0x10143fe0, 0x10143fe4):
        w32(m.uc, a, r.getrandbits(32))
    # names, files, the table and the records
    ngroups = 4
    names = {}
    files = {}
    tab = bytearray(NG * G12)
    for g in range(ngroups):
        for f in range(r.choice([0, 1, 3, 8, 32])):
            nm = r.choice([b'WX', b'TA', b'Wa']) + b'%02d%02d' % (g, f)
            if r.random() < 0.1:
                nm = b'WX' + nm[2:]
            names[(g, f)] = nm
            if r.random() < 0.8:
                files[(nm.upper(), b'TEX')] = tex_image(ctx)
            o = g * G12 + f * F12
            tab[o:o + len(nm)] = nm
            struct.pack_into('<H', tab, o + 0x12, r.getrandbits(16))
            if r.random() < 0.2:
                struct.pack_into('<II', tab, o + 0x14, 1, tex_image(ctx))
    m.uc.mem_write(sym['g_w31TexNames'], bytes(tab))
    ST['files'] = files
    recs = bytearray(0x200 * 14)
    for i in range(0x200):
        g = r.randrange(0, ngroups)
        f = r.choice([r.randrange(0, 8), 0, 31, -1 if g else 0])
        struct.pack_into('<hhhHhI', recs, i * 14, g, f, 0, r.choice([1, 1, 0, 0x100]),
                         r.choice([1, 1, 0, 3, -2, 0x10]), 0)
    m.uc.mem_write(REC10, bytes(recs))


def to12(m):
    uc = m.uc
    uc.mem_write(TAB12, bytes(uc.mem_read(m.syms['g_w31TexNames'], NG * G12)))
    uc.mem_write(REC12, bytes(uc.mem_read(REC10, 0x200 * 14)))
    for a10, a12 in DWORDS:
        w32(uc, A12(a12), r32(uc, a10))
    w32(uc, A12(0x10105048), 1)                     # 1.2's .TEX flag
    w32(uc, A12(0x1014a5f0), 0)
    w32(uc, A12(0x1014b954), r32(uc, m.syms['g_w33Voodoo']))   # round 33: 1.2's 3DFX / PowerVR mode
    w32(uc, A12(0x1012d3f8), r32(uc, m.syms['g_w31TexStart']))
    w32(uc, A12(0x107ee3f0), STUB_RENDER)
    w32(uc, A12(0x107ee414), STUB_PRELOAD)
    pal = r32(uc, 0x1014ceb8)
    w32(uc, A12(0x1011fd48), pal)


def from12(m):
    uc = m.uc
    tab = bytearray(uc.mem_read(TAB12, NG * G12))
    for o in range(0, NG * G12, F12):
        img = struct.unpack_from('<I', tab, o + 0x18)[0]
        struct.pack_into('<I', tab, o + 0x14, 1 if img else 0)
    uc.mem_write(m.syms['g_w31TexNames'], bytes(tab))
    uc.mem_write(REC10, bytes(uc.mem_read(REC12, 0x200 * 14)))
    for a10, a12 in DWORDS:
        w32(uc, a10, r32(uc, A12(a12)))


def log_map(log_o, log_c):
    out = []
    for e in log_c:
        if e[0] == 'load' and e[1][1] == b'XEL' and out and out[-1] == ('load', (e[1][0], b'TEX')):
            continue
        out.append(e)
    return log_o, out


# ---- generators ----------------------------------------------------------
def g_getimage(ctx):
    begin(ctx)
    r = ctx.rng
    wh = ctx.buf(16)
    idx = r.choice([r.randrange(0, 0x30), r.randrange(0, 0x200), 0x1ff, 0x200, 0x300])
    return [idx, wh, wh + 4, wh + 8]


def g_register(ctx):
    begin(ctx)
    r, uc = ctx.rng, ctx.m.uc
    w32(uc, 0x1015ec58, r.choice([0, 0, 1]))
    w32(uc, 0x1012da30, r.choice([0, 0, 0, 0xfffffffe]))
    w32(uc, 0x1015cfe0, r.choice([0, 1, 2, 3, 0x1ff]))
    w32(uc, 0x1015cfe4, r.choice([0, 1]))
    w32(uc, 0x1014cb38, r.choice([0, 1]))
    name = ctx.buf(12, r.choice([b'WXab', b'wxab', b'TA0101', b'Wxq', b'A']) + b'\0')
    group = r.choice([0xffffffff, 0, 1, 2, 3, r.randrange(0, 0x200), 0x200, 0xfffffff0])
    return [r.choice([name] * 19 + [0]), group, r.getrandbits(16)]   # a null name faults in both (strcpy first)


def g_preload(ctx):
    begin(ctx)
    r = ctx.rng
    # most records flagged for preloading; images of the size classes' widths
    for i in range(0x200):
        a = REC10 + i * 14
        if r.random() < 0.5:
            ctx.m.uc.mem_write(a + 8, struct.pack('<h', r.choice([1, 3])))
    return []


def verts(ctx, n):
    r = ctx.rng
    data = bytearray()
    for _ in range(max(n, 1)):
        v = bytearray(r.getrandbits(8) for _ in range(0x24))
        struct.pack_into('<f', v, 8, r.choice([1.0, 2.5, 100.0, 0.25]))
        struct.pack_into('<I', v, 0x10, r.getrandbits(8))
        data += v
    return ctx.buf(len(data), bytes(data))


def g_sub(extra):
    def g(ctx):
        begin(ctx)
        r = ctx.rng
        n = r.choice([0, 2, 3, 4, 6])
        idx = r.randrange(-0x100, 0x100) if extra != 'raw' else r.randrange(0, 0x200)
        args = [idx & 0xffffffff, n, verts(ctx, n)]
        if extra == 'flag':
            args.append(r.choice([0, 0, 1]))
        w32(ctx.m.uc, ctx.m.syms['g_w33Voodoo'], r.choice([0, 0, 1]))     # round 33: its 3DFX / PowerVR branch
        return args
    return g


def g_fallback(ctx):
    """1.0's getter (no .TEX at all, .XEL images) against W31_GetTexImage in .TEX mode with no .TEX files."""
    begin(ctx)
    r, uc = ctx.rng, ctx.m.uc
    files = {}
    for (nm, typ), img in list(ST['files'].items()):
        if r.random() < 0.8:
            files[(nm, b'XEL')] = ctx.buf(4 + 64, struct.pack('<hh', r.choice([4, 8]), r.choice([2, 8])) +
                                          bytes(r.getrandbits(8) for _ in range(64)))
    ST['files'] = files
    tab = bytearray(uc.mem_read(ctx.m.syms['g_w31TexNames'], NG * G12))
    for o in range(0, NG * G12, F12):        # no images loaded yet: the 1.0 table can carry no kind
        tab[o + 0x14:o + 0x1c] = bytes(8)
    uc.mem_write(ctx.m.syms['g_w31TexNames'], bytes(tab))
    w32(uc, 0x1015cfec, 0)                   # 1.0's fixed-image special case off (1.2 has none)
    wh = ctx.buf(16, 'zero')
    ST['palbuf'] = wh + 8
    return [r.choice([r.randrange(0, 0x30), r.randrange(0, 0x200)]), wh, wh + 4, wh + 8]


def fb_pre_c(m):
    pass


def fb_post_c(m):
    """the C's table back to the 1.0 form; the palette it gave must be the game palette (whenever it gave one)"""
    w31_post_c(m)
    pal = r32(m.uc, ST['palbuf'])
    w32(m.uc, ST['palbuf'], 0 if pal in (0, PAL10) else 0xbad)


def fb_gen(ctx):
    args = g_fallback(ctx)
    # the original reads the 1.0 table: ours copied there in the 1.0 form
    m = ctx.m
    _W31_BASE[0] = m.syms['g_w31TexNames']
    tab = bytes(m.uc.mem_read(m.syms['g_w31TexNames'], NG * G12))
    old = bytearray(NG * G10)
    for g in range(NG):
        for f in range(NF):
            s, d = g * G12 + f * F12, g * G10 + f * F10
            old[d:d + 0x14] = tab[s:s + 0x14]
            old[d + 0x14:d + 0x18] = tab[s + 0x18:s + 0x1c]
    m.uc.mem_write(T10, bytes(old))
    return args


def fb_log_map(log_o, log_c):
    out = [e for e in log_c if not (e[0] == 'load' and e[1][1] == b'TEX')]
    return log_o, out


SPECS = [
    dict(name='W31_GetTexImage', orig12=0x10002391, c='W31_GetTexImage', gen=g_getimage, trials=3000,
         to12=to12, from12=from12, log_map=log_map, api=API),
    dict(name='Sim_10006ba7_tex', orig12=0x1000189d, c='Sim_10006ba7', gen=g_register, trials=3000,
         to12=to12, from12=from12, log_map=log_map, api=API),
    dict(name='Render_PreloadTextures_tex', orig12=0x10001b4c, c='Render_PreloadTextures', gen=g_preload, trials=400,
         void=True, to12=to12, from12=from12, log_map=log_map, api=API),
    dict(name='Render_SubmitVerts_10006609_tex', orig12=0x100010d9, c='Render_SubmitVerts_10006609',
         gen=g_sub(None), trials=2000, to12=to12, from12=from12, log_map=log_map, api=API),
    dict(name='Render_SubmitVerts_10006744_tex', orig12=0x100012c5, c='Render_SubmitVerts_10006744',
         gen=g_sub('flag'), trials=2000, to12=to12, from12=from12, log_map=log_map, api=API),
    dict(name='Render_SubmitVerts_100068d9_tex', orig12=0x10001513, c='Render_SubmitVerts_100068d9',
         gen=g_sub('raw'), trials=2000, to12=to12, from12=from12, log_map=log_map, api=API),
    dict(name='W31_GetTexImage_fallback', orig=0x100072ff, c='W31_GetTexImage', gen=fb_gen, trials=2000,
         post_c=fb_post_c, log_map=fb_log_map, ignore_syms={'g_w31TexNames': NG * G12}),
]

# ---- T2: the file textures' preload (1.0 0x100a2a66 -> 1.2 0x1008fd87) ----
# name cells, image cells, 1.2's descriptions and handles; the C's descriptions are g_w31FileTex[0] / [1]
FILES = [(0x1017d0d0, 0x1014b978, 0x1017d0fc, 0x1014b9a4, 0x101cfc90, 0x1018e550, 0x1018b540),
         (0x1017d0b0, 0x1014b958, 0x1017d100, 0x1014b9a8, 0x101cfcb8, 0x1018e580, 0x1018e560)]


def g_files(ctx):
    """Both images loaded here (an image loaded before is 1.2's bug path: it describes it with a palette from the
    stack; the C's fix is checked by the 1.0 spec, spec_w2m.py)."""
    begin(ctx)
    r, uc = ctx.rng, ctx.m.uc
    files = {}
    for k, (n10, n12, i10, i12, h10, h12, d12) in enumerate(FILES):
        name = r.choice([b'', b'SKY01', b'K10WTDAY', b'CLOUDS'])
        uc.mem_write(n10, name + bytes(0x20 - len(name)))
        w32(uc, i10, 0)
        w32(uc, h10, r.getrandbits(32))
        if name and r.random() < 0.85:
            files[(name.upper(), b'TEX')] = tex_image(ctx)
    ST['files'] = files
    return []


def to12_files(m):
    to12(m)
    uc = m.uc
    for k, (n10, n12, i10, i12, h10, h12, d12) in enumerate(FILES):
        uc.mem_write(A12(n12), bytes(uc.mem_read(n10, 0x20)))
        w32(uc, A12(i12), r32(uc, i10))
        ctex = m.syms['g_w31FileTex'] + k * 0x1c
        uc.mem_write(A12(d12), bytes(uc.mem_read(ctex, 0x1c)))
        h = r32(uc, h10)
        w32(uc, A12(h12), A12(d12) if h == ctex else h)


def from12_files(m):
    from12(m)
    uc = m.uc
    for k, (n10, n12, i10, i12, h10, h12, d12) in enumerate(FILES):
        w32(uc, i10, r32(uc, A12(i12)))
        ctex = m.syms['g_w31FileTex'] + k * 0x1c
        uc.mem_write(ctex, bytes(uc.mem_read(A12(d12), 0x1c)))
        h = r32(uc, A12(h12))
        w32(uc, h10, ctex if h == A12(d12) else h)


def log_map_files(log_o, log_c):
    """1.2's handles as the C's (the preload's description pointer is logged by content already)"""
    return log_map(log_o, log_c)


SPECS.append(dict(name='Render_PreloadFileTextures_tex', orig12=0x1008fd87, c='Render_PreloadFileTextures',
                  gen=g_files, trials=2000, void=True, to12=to12_files, from12=from12_files, log_map=log_map_files,
                  api=API, ignore_syms={'g_w31TexLoaded': 4, 'g_w31TexFallback': 4, 'g_w31TexMissing': 4}))

# the C's per-mission log counts (round 31, T2) have no counterpart in 1.2 / 1.0
for _s in SPECS:
    _ig = dict(_s.get('ignore_syms', {}))
    _ig.update({'g_w31TexLoaded': 4, 'g_w31TexFallback': 4, 'g_w31TexMissing': 4})
    _s['ignore_syms'] = _ig
