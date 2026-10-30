# Round 22: 0x100a0def, the view background (DECISIONS.md round 22). Sim_10084766 runs for real; every other callee
# is a logging stub: the matrix filler 0x1006018d writes a random matrix, 0x1005fbb0 logs it whole and answers the
# band / depth / third value through its pointers, the horizon tests answer from rets, the quad / triangle / pentagon
# fills, the library 0x100e5477 (software vertices logged whole), the texture loader, the renderer and driver slots
# (int3 padding; the four haze vertices logged whole) and Sim_100a0880 log their arguments. The original never writes
# the haze vertices' alpha bytes nor the software vertices' +0xc..+0x14: the spec zeroes its locals, as the C does.
# Negative check: W22_NEG=1 leaves random bytes in the original's locals instead, which the vertex logs must catch.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w22_poly.c'])
NEG = bool(os.environ.get('W22_NEG'))
RENDER, DRIVER = 0x1010d3e6, 0x1010d3e7
helper(RENDER, None, 5, deref={2: 4 * 0x24})
helper(DRIVER, None, 3, deref={1: 4 * 0x24})
helper(0x1006018d, None, 1)
helper(0x1005fbb0, None, 4, deref={0: 0x30})
helper(0x10052e60, None, 3)
helper(0x10052f3a, None, 2)
helper(0x10052edc, None, 2)
helper(0x1000656f, None, 0)
helper(0x100a291d, None, 9)
helper(0x100a29da, None, 7)
helper(0x100a2823, None, 11)
helper(0x100e5477, None, 4, deref={3: 4 * 0x18})
helper(0x100ba7d5, None, 3, sargs=(0,))
helper(0x100a0880, 'Sim_100a0880', 0)
ACTIVE = (RENDER, DRIVER, 0x1006018d, 0x1005fbb0, 0x10052e60, 0x10052f3a, 0x10052edc, 0x1000656f, 0x100a291d,
          0x100a29da, 0x100a2823, 0x100e5477, 0x100ba7d5, 0x100a0880)


def _matrix(uc, k, raw, eax):
    uc.mem_write(raw[0], ST['matrix'])
    return None


def _band(uc, k, raw, eax):
    for i, v in enumerate(ST['band']):
        uc.mem_write(raw[1 + i], struct.pack('<f', v))
    return None


def _corner(uc, k, raw, eax):
    bits = ST['corners']
    return (bits >> k) & 1 if k < 4 else ST['extra']


EFFECT[0x1006018d] = _matrix
EFFECT[0x1005fbb0] = _band
EFFECT[0x10052e60] = _corner


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, 0x109e3d30, RENDER)
    w32(ctx, 0x1014cb4c, DRIVER)
    ST['matrix'] = b''.join(struct.pack('<f', r.uniform(-2, 2)) for _ in range(12))
    ST['band'] = [r.uniform(-100, 100), r.uniform(-50, 300), r.uniform(-5, 5)]
    ST['corners'] = r.randrange(0, 16)
    ST['extra'] = r.choice([0, 1])
    v = r.randrange(-50, 700)
    rets(0x10052f3a, *([v, v] if r.random() < 0.4 else [r.randrange(-50, 700) for _ in range(2)]))
    v = r.randrange(-50, 500)
    rets(0x10052edc, *([v, v] if r.random() < 0.4 else [r.randrange(-50, 500) for _ in range(2)]))
    wf(ctx, 0x1012ba50, r.choice([4.0, 4.0, -10.0]))     # data; with 4.0 the reset cannot happen (u <= 1)
    for a, vals in ((0x10185bd0, [0, 10]), (0x10185bec, [639, 300]), (0x10185bf0, [0, 20]), (0x10185be0, [479, 200])):
        w32(ctx, a, r.choice(vals))
    wb(ctx, 0x109e3e00, rb(ctx, 0x14))
    for a in (0x1014cb04, 0x1014cb08, 0x1014cb0c, 0x1014caec, 0x1017d0ac, 0x1014cb38, 0x1017b940, 0x1012da34):
        w32(ctx, a, r.choice([0, 1, 1]))
    w32(ctx, 0x1017d0ac, r.choice([0, 0, 1]))
    w32(ctx, 0x1014cb50, r.randrange(0, 400))
    w32(ctx, 0x10138c74, r.getrandbits(32))
    w32(ctx, 0x10138c78, r.getrandbits(32))
    w32(ctx, 0x10143fdc, r.getrandbits(32))
    w32(ctx, 0x10143fe0, r.getrandbits(32))
    w32(ctx, 0x10143fe4, r.getrandbits(32))
    wf(ctx, 0x1014c9f0, r.uniform(-2, 2))
    w32(ctx, 0x1017d0fc, r.choice([0, newbuf(ctx, 8)]))
    w8(ctx, 0x1017d0d0, r.choice([0, ord('H')]))
    rets(0x100ba7d5, r.choice([0, newbuf(ctx, 8)]))
    rec = newbuf(ctx, 0x40)
    w32(ctx, rec + 0x38, r.choice([0, 1]))
    w32(ctx, 0x1015d1b8, rec)
    view = newbuf(ctx, 0x20)
    wf(ctx, view + 0xc, r.choice([r.uniform(-800, 800), 0.0, 360.0, 180.0]))
    wf(ctx, view + 0x14, r.uniform(-90, 90))
    ebp = frame(1)
    wb(ctx, ebp - 0x17c, rb(ctx, 0x17c) if NEG else bytes(0x17c))
    return [view]


SPECS = [dict(name='Sim_100a0def', orig=0x100a0def, c='Sim_100a0def', gen=g, void=True, trials=300 if NEG else 6000)]
