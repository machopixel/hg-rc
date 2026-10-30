# Round 22: 0x100ac9ab, the star of a draw item (DECISIONS.md round 22). The clip-and-draw routine 0x100ac738 is a
# logging stub that logs the four 0x58-byte clip vertices whole at each of its four calls. The original never writes
# their +0..+8 and +0x31..+0x57 (nothing downstream reads them); the spec gives its stack the C's zeros there.
# Negative check: W22_NEG=1 leaves random bytes there instead, which the vertex log must catch.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w22_items.c'])
NEG = bool(os.environ.get('W22_NEG'))
helper(0x100ac738, 'Render_SubmitClipped_100ac738', 2, deref={0: 4 * 0x58})
ACTIVE = (0x100ac738,)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    item = newbuf(ctx, 0x34)
    for o in (0x14, 0x18, 0x1c):
        wf(ctx, item + o, r.choice([r.uniform(-5000, 5000), r.uniform(-1e9, 1e9), 0.5, -0.5]))
    wf(ctx, item + 0x2c, r.choice([r.uniform(0, 100), 1.0, 3.0, -7.0, r.uniform(-1e6, 1e6)]))
    w32(ctx, 0x101c9ea8, r.getrandbits(32))
    ebp = frame(1)
    wb(ctx, ebp - 0x57b0, rb(ctx, 4 * 0x58) if NEG else bytes(4 * 0x58))
    return [item]


SPECS = [dict(name='Sim_100ac9ab', orig=0x100ac9ab, c='Sim_100ac9ab', gen=g, void=True, trials=300 if NEG else 4000)]
