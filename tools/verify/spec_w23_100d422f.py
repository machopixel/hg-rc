# Round 23: 0x100d422f, a cockpit panel of the player's weapon slot (DECISIONS.md round 23). The resource loader /
# release (0x100ba7d5 / 0x100bb2b0) and the shape drawers 0x100f0205 / 0x100e91dc are logging stubs. No local is read
# before it is written.
# Negative check (mutation): W23_NEG=1 builds the C with W23H_MUTATE (the time test becomes >=).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w23_screens.c'])
NEG = bool(os.environ.get('W23_NEG'))
if NEG:
    CFLAGS = CFLAGS + ['-DW23H_MUTATE']
helper(0x100ba7d5, None, 3)
helper(0x100bb2b0, None, 2)
helper(0x100f0205, None, 5)
helper(0x100e91dc, None, 5)
ACTIVE = (0x100ba7d5, 0x100bb2b0, 0x100f0205, 0x100e91dc)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    me = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, me)
    unit = newbuf(ctx, 0x40)
    block = newbuf(ctx, 0x100)
    slots = newbuf(ctx, 8 * 0x128)
    for i in range(8):
        w32(ctx, slots + i * 0x128 + 4, r.choice([-1, 0, 3]))
    w32(ctx, block + 0x84, slots)
    w32(ctx, unit + 0x28, block)
    w32(ctx, 0x109e8b40 + 4 * me, unit)
    panel = newbuf(ctx, 0x100)
    w16(ctx, panel + 4, r.choice([0, 1, 7]))
    w32(ctx, panel + 0xc, r.choice([-1, 0, 3, 7]))
    now = r.randrange(0, 1000)
    w32(ctx, 0x1015dc78, now)
    w32(ctx, panel + 8, r.choice([now, now - 1, now + 1, 0]))
    for a in (0x109b9900, 0x1014cec4, 0x109e84b8, 0x1014cec0):
        w32(ctx, a, r.getrandbits(32))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    rets(0x100ba7d5, *[r.choice([0, 0x1234, 0x5678]) for _ in range(2)])
    return [panel]


SPECS = [dict(name='Sim_100d422f', orig=0x100d422f, c='Sim_100d422f', gen=g, void=True, trials=300 if NEG else 4000)]
