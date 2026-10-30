# Round 8B, helper 2: bit-exact proofs (no float tolerance) for the functions of rounds 3B, 3r, 6g and 6i that were
# only ever checked with fp=True. See tools/round8b/README.md.
#
# The stubs and generators of the old specs (spec_w2n.py, spec_w3r.py, spec_w6i.py, spec_w6g.py) are reused: each old
# spec file is read and run into its own namespace (the old files are not changed). Their emulator hooks put
# different stubs at the same addresses, so only ONE group may be loaded per run; pick it with the environment
# variable R8B_GROUP = w2n | w3r | w6i | w6g, e.g.
#     set R8B_GROUP=w6g& python tools\verify\difftest.py -j2 C:\Coding\HeavyGear\Project\HGSIM.DLL tools\verify\spec_r8b_h2.py
# Every function is compared EXACTLY (no fp/tol). The DRAWING ones also get a second entry `<name>~tol` with the old
# tolerance, which prints the worst float deviation.
import os
import struct

_HERE = os.path.dirname(os.path.abspath(__file__)) if '__file__' in globals() else os.path.join('tools', 'verify')
GROUP = os.environ.get('R8B_GROUP', 'w6g')


def _load(name):
    ns = {'__name__': 'r8b_' + name}
    path = os.path.join('tools', 'verify', 'spec_%s.py' % name)
    if not os.path.exists(path):
        path = os.path.join(_HERE, 'spec_%s.py' % name)
    exec(compile(open(path).read(), path, 'exec'), ns)
    return ns


OLD = _load(GROUP)
SOURCES = list(OLD['SOURCES'])
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']


def _old(name):
    return next(s for s in OLD['SPECS'] if s['name'] == name)


def exact(name, gen=None, trials=None, **kw):
    """The old entry without fp/tol, with an optional new generator and trial count."""
    d = dict(_old(name))
    d.pop('fp', None)
    d.pop('tol', None)
    if gen is not None:
        d['gen'] = gen
    if trials is not None:
        d['trials'] = trials
    d.update(kw)
    return d


def tolerant(name, gen=None, trials=None):
    """The old entry as it was (fp=True), renamed, to print the worst deviation of a DRAWING function."""
    d = dict(_old(name))
    d['name'] = name + '~tol'
    if gen is not None:
        d['gen'] = gen
    if trials is not None:
        d['trials'] = trials
    return d


# Special float words for the GAME LOGIC functions: quiet NaNs of both signs (the emulator does not quiet signalling
# NaNs, so none are used), infinities, signed zeros, denormals, the extremes of the float range, values near 1.
EDGE = [0x7fc00000, 0xffc00000, 0x7fc0beef, 0x7f800000, 0xff800000, 0x00000000, 0x80000000, 0x00000001, 0x80000001,
        0x007fffff, 0x807fffff, 0x00800000, 0x80800000, 0x7f7fffff, 0xff7fffff, 0x3f800000, 0xbf800000, 0x3f800001,
        0x3f7fffff, 0x1e3ce508, 0x5e5e5e5e]


def efl(ctx):
    r = ctx.rng
    if r.random() < 0.6:
        return r.choice(EDGE)
    e = r.choice([0, 0, 1, 0x7e, 0x7f, 0x80, 0xfe, r.randrange(0, 0xff)])
    return (r.getrandbits(1) << 31) | (e << 23) | r.getrandbits(23)


SPECS = []

if GROUP == 'w2n':
    SPECS = [
        exact('Render_ClipPolygon'), tolerant('Render_ClipPolygon'),
        exact('Render_SubmitClipped_100ac738'), tolerant('Render_SubmitClipped_100ac738'),
        exact('Sim_100ad1f3'), tolerant('Sim_100ad1f3'),
    ]
elif GROUP == 'w3r':
    SPECS = [
        exact('Sim_10052e60', trials=3000),
        exact('Sim_10052f3a', trials=3000),
        exact('Sim_10090099', trials=3000),
    ]
elif GROUP == 'w6i':
    SPECS = [
        exact('Fpu_HasFdivBug', trials=1000),
        exact('Fpu_CheckFdivBug', trials=1000),
    ]
elif GROUP == 'w6g':
    _w32 = OLD['w32']
    _r32 = OLD['r32']

    def seg_edges(ctx, s, extra=()):
        for off in (0, 4, 8, 0xc, 0x10, 0x14) + tuple(extra):
            if ctx.rng.random() < 0.15:
                _w32(ctx, s + off, efl(ctx))

    def g_cc82_e(ctx):
        a = OLD['g_cc82'](ctx)
        if ctx.rng.random() < 0.6:
            seg_edges(ctx, a[0])
        return a

    def g_cdd5_e(ctx):
        r = ctx.rng
        a = OLD['g_cdd5'](ctx)
        if r.random() < 0.6:
            seg_edges(ctx, a[0], (0x24, 0x28, 0x2c))
            if r.random() < 0.5:        # the length against the threshold 0.0 at 0x1012ba04
                _w32(ctx, a[0] + 0x30, r.choice([0, 0x80000000, 1, 0x80000001, 0x7fc00000, 0xffc00000, 0x7f800000,
                                                 0xff800000, 0x00800000, 0x3f800000, efl(ctx)]))
            if r.random() < 0.3:
                a[1] = efl(ctx)
        return a

    def g_ceab_e(ctx):
        r = ctx.rng
        a = OLD['g_ceab'](ctx)
        if r.random() < 0.6:
            seg_edges(ctx, a[0])
            for i in (1, 2, 3):
                if r.random() < 0.3:
                    a[i] = efl(ctx)
            if r.random() < 0.2:        # end == start
                for k in range(3):
                    a[1 + k] = _r32(ctx.m.uc, a[0] + 4 * k)
        return a

    def g_a8c24_e(ctx):
        r = ctx.rng
        a = OLD['g_a8c24'](ctx)
        if a[0] and r.random() < 0.6:
            for off in (8, 0xc, 0x10, 0x18, 0x1c, 0x20):
                if r.random() < 0.3:
                    _w32(ctx, a[0] + off, efl(ctx))
        return a

    def g_ac21a_e(ctx):
        r = ctx.rng
        a = OLD['g_ac21a'](ctx)
        src = a[1]
        if src and r.random() < 0.6:
            for off in (0x58, 0x60, 0x7c):
                if r.random() < 0.4:
                    _w32(ctx, src + off, efl(ctx))
            o = _r32(ctx.m.uc, src + 0x3c)
            for off in (0x70, 0x74, 0x78):
                if r.random() < 0.4:
                    _w32(ctx, o + off, efl(ctx))
        return a

    def obj_edges(ctx, obj):
        r = ctx.rng
        if r.random() < 0.5:            # +0xd4 against 5000.0 (0x1012bb24)
            _w32(ctx, obj + 0xd4, r.choice([0x459c4000, 0x459c3fff, 0x459c4001, 0xc59c4000, 0x7fc00000, 0xffc00000,
                                             0x7f800000, 0xff800000, efl(ctx)]))
        if r.random() < 0.6:            # angle difference +0xe8 - +0x68 against 1.0 (0x1012bb20), NaN, infinities
            a = struct.unpack('<f', struct.pack('<I', _r32(ctx.m.uc, obj + 0x68)))[0]
            c = r.random()
            if c < 0.5:
                d = r.choice([1.0, -1.0, 1.0000001, -1.0000001, 0.99999994, -0.99999994, 0.0, -0.0])
                v = struct.unpack('<I', struct.pack('<f', a + d))[0]
            else:
                v = efl(ctx)
            for off in (0xe8, 0x110):
                if r.random() < 0.7:
                    _w32(ctx, obj + off, v)
            if r.random() < 0.15:
                _w32(ctx, obj + 0x68, efl(ctx))

    def g_c0_e(ctx):
        a = OLD['g_c0'](ctx)
        obj_edges(ctx, a[0])
        return a

    def g_c0w_e(ctx):
        a = OLD['g_c0w'](ctx)
        obj_edges(ctx, a[0])
        return a

    SPECS = [
        exact('Sim_1009cc82', g_cc82_e, 4000),
        exact('Sim_1009cdd5', g_cdd5_e, 4000),
        exact('Sim_1009ceab', g_ceab_e, 4000),
        exact('Sim_100a8c24', g_a8c24_e, 2000),
        exact('Sim_100ac21a', g_ac21a_e, 3000),
        exact('Sim_100c0423', g_c0_e, 2000),
        exact('Sim_100c0471', g_c0_e, 3000),
        exact('Sim_100c0ac4', g_c0w_e, 2000),
        exact('Sim_100c0fc4', g_c0w_e, 3000),
    ]
