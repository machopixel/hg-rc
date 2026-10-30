# Round 22: 0x100a0880, the sky band (DECISIONS.md round 22). Sim_10084766 runs for real on both sides (proven, round
# 8); Sim_100a88db, Render_ClipPolygon (logging the four 0x58-byte clip vertices whole), the texture loader 0x100ba7d5
# and the drawer 0x100a0339 are logging stubs. The original never writes the clip vertices' +0..+8, +0x20..+0x2b and
# +0x30..+0x57 (Render_ClipPolygon writes +0x20.. itself; nothing reads +0..+8): the spec gives its stack the C's zeros.
# Negative check: W22_NEG=1 leaves random bytes there instead, which the vertex log must catch.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w22_poly.c'])
NEG = bool(os.environ.get('W22_NEG'))
helper(0x100a88db, 'Sim_100a88db', 0)
helper(0x100a0512, 'Render_ClipPolygon', 4, deref={0: 4 * 0x58})
helper(0x100ba7d5, None, 3, sargs=(0,))
helper(0x100a0339, None, 4)
ACTIVE = (0x100a88db, 0x100a0512, 0x100ba7d5, 0x100a0339)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w8(ctx, 0x1017d0b0, r.choice([0, ord('S'), ord('S')]))
    wb(ctx, 0x1017d0b1, b'KY\0')
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    cam = newbuf(ctx, 0x20)
    for o in (0, 4, 8):
        wf(ctx, cam + o, r.uniform(-5000, 5000))
    wf(ctx, cam + 0xc, r.choice([r.uniform(-800, 800), r.uniform(0, 360), 0.0, 360.0]))   # degrees, wrapped by 360
    wf(ctx, cam + 0x10, r.uniform(-60, 60))
    w32(ctx, 0x1014cae0, cam)
    wf(ctx, 0x1014c9c8, r.choice([1000.0, 2500.0, 37.5]))
    wf(ctx, 0x1014c9e8, r.choice([256.0, 100.0, 7.0]))
    wf(ctx, 0x1014c9f8, r.choice([0.01, 0.5]))
    w32(ctx, 0x1015dc80, r.randrange(0, 200))
    wf(ctx, 0x1017d104, r.choice([0.0, 0.5, 0.99, 1e6]))
    wf(ctx, 0x1014c9f4, r.uniform(0, 10))
    w32(ctx, 0x1014c9c0, r.getrandbits(32))
    w32(ctx, 0x1014cb34, r.getrandbits(32))
    w32(ctx, 0x10185c50, r.getrandbits(32))
    rets(0x100a0512, r.choice([0, 3, 4, 6]))
    w32(ctx, 0x1017d100, r.choice([0, newbuf(ctx, 8)]))
    tex = newbuf(ctx, 0x10)
    rets(0x100ba7d5, r.choice([0, tex, tex]))
    ebp = frame(0)
    wb(ctx, ebp - 0x184, rb(ctx, 4 * 0x58) if NEG else bytes(4 * 0x58))
    return []


SPECS = [dict(name='Sim_100a0880', orig=0x100a0880, c='Sim_100a0880', gen=g, void=True, trials=300 if NEG else 4000)]
