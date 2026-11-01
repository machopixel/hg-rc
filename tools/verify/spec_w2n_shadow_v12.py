# Round 33: the two projected-polygon draws against 1.2's own code: Render_SubmitClipped_100ac738 (1.2 0x100974e8)
# and Sim_100ad1f3 (1.2 0x10097ff0). 1.2: rhw 1 / z (1.0: 1), Sim_100ad1f3's light and description word 0xff (1.0:
# 0xfe), and in its 3DFX / PowerVR mode (g_w33Voodoo = 1.2 [0x1014b954]) fixed fog bytes and, with fog on, the
# renderer's fog state turned off. 3D mode only here (the software path is 1.0's, unchanged).
# Stubbed on both sides: the clipper (1.0 0x100a0512 = 1.2 0x1008d6f3: answers the trial's count and fills both
# lists of projected vertices), the renderer's Render slot (logs the mode, the vertices and the description) and its
# SetState slot. Render_FogLevel (identical in both) runs for real. 1.2's Render mode 0x11 is logged as our 0xf (its
# renumbering) and its SetState code 1 as our 2.
# Negative check: W2N_NEG=1 builds the C with W2NS_MUTATE (Sim_100ad1f3 keeps 1.0's light).
import os
exec(open('tools/verify/w32_common.py').read())

SOURCES = closure(['sim/render/w2n_polygon.c'])
if os.environ.get('W2N_NEG'):
    CFLAGS = list(CFLAGS) + ['-DW2NS_MUTATE']
RENDER, STATE = 0x1010d3e6, 0x1010d3f2          # stand-ins in the slots
helper(RENDER, None, 5, deref={2: 0x24 * 12, 4: 8})
helper(STATE, None, 2)
stub12(0x100a0512, 'Render_ClipPolygon', 4, 0x1008d6f3)
LIST10, LIST12 = 0x101c76a0, 0x10188d20
GLOBALS = [
    (0x1014cae0, 0x1011f9e8, 4),        # the camera
    (0x1017b940, 0x1014a5e8, 4),        # 3D mode
    (0x1017d108, 0x1014b9b0, 4),        # fog
    (0x109e3d30, 0x107ee3f0, 4),        # Render slot
    (0x109e3d40, 0x107ee400, 4),        # SetState slot
    (0x101c9ea8, 0x10189130, 4),
    ('g_w33Voodoo', 0x1014b954, 4),
]
ADDRS = {0x109e3dc0: 0x107ee440}        # the plugin record, in the logs
ST['n'] = 0


def e_clip(uc, k, raw, eax):
    for base in (LIST10, A12(LIST12)):
        for i, p in enumerate(ST['proj']):
            uc.mem_write(base + 4 * i, struct.pack('<I', p))
    return ST['n']


EFFECT[0x100a0512] = e_clip


def proj(ctx, r):
    p = newbuf(ctx, 0x40)
    wf(ctx, p + 8, r.choice([r.uniform(1, 9000), 0.5, 100.0]))
    w32(ctx, p + 0x18, r.randrange(-50, 700), r.randrange(-50, 500))
    wf(ctx, p + 0x20, r.choice([24.0, 24.0, r.uniform(0, 50)]))
    return p


def world(ctx, r):
    ST['n'] = r.choice([0, 2, 3, 4, 5, 8])
    ST['proj'] = [proj(ctx, r) for _ in range(max(ST['n'], 1))]
    cam = newbuf(ctx, 0x100)
    w32(ctx, cam + 0x3c, r.getrandbits(32), r.getrandbits(32))
    w32(ctx, 0x1014cae0, cam)
    w32(ctx, 0x1017b940, 1)
    w32(ctx, 0x1017d108, r.choice([0, 1]))
    w32(ctx, 0x109e3d30, RENDER)
    w32(ctx, 0x109e3d40, STATE)
    w32(ctx, ctx.m.syms['g_w33Voodoo'], r.choice([0, 1]))


def g_clip(ctx):
    r = begin(ctx, RENDER, STATE, 0x100a0512)
    world(ctx, r)
    verts = newbuf(ctx, 0x58 * 8)
    wf(ctx, verts + 0x10, r.choice([0.0, 100.0, r.uniform(-500, 500)]))
    return [verts, r.choice([3, 4, 6])]


def g_model(ctx):
    r = begin(ctx, RENDER, STATE, 0x100a0512)
    world(ctx, r)
    n = r.choice([3, 4, 5])
    obj = newbuf(ctx, 0x40)
    wf(ctx, obj + 0x18, r.choice([0.0, 0.0, 30.0, r.uniform(-100, 100)]))
    table = newbuf(ctx, 0x58 * 8 + 0x18)
    holder = newbuf(ctx, 0x40)
    w32(ctx, holder + 0x30, table)
    part = newbuf(ctx, 0x40)
    w32(ctx, part + 0x1c, r.choice([0, 0, 0x80000000]))
    w16(ctx, part + 2, n)
    w32(ctx, part + 4, 0x30)
    for i in range(n):
        w8(ctx, part + 0x30 + i, r.randrange(0, 8))
    w32(ctx, part + 0x24, holder)
    return [obj, part]


def lm_modes(spec):
    base = spec['log_map']

    def lm(log_o, log_c):
        log_o, log_c = base(log_o, log_c)
        out = []
        for e in log_o:
            if e[0] == 'stub_%x' % RENDER and e[1] and e[1][0] == 0x11:
                e = (e[0], (0xf,) + tuple(e[1][1:]))
            if e[0] == 'stub_%x' % STATE and e[1] and e[1][0] == 1:
                e = (e[0], (2,) + tuple(e[1][1:]))
            out.append(e)
        return out, log_c
    return lm


def spec_m(name, f12, gen, trials):
    s = spec12(name, f12, gen, trials, void=True)
    g0 = s['gen']

    def g2(ctx):
        a = g0(ctx)
        s['log_map'] = lm_modes(s)
        return a
    s['gen'] = g2
    return s


SPECS = [spec_m('Render_SubmitClipped_100ac738', 0x100974e8, g_clip, 3000),
         spec_m('Sim_100ad1f3', 0x10097ff0, g_model, 3000)]
