# Round 23: 0x100ce586, the sliding side panel (DECISIONS.md round 23). The resource loader / release, the font
# height, the frame pieces 0x100652e8, Sim_100cd70f (its position pair logged whole) and the three text routines (the
# line logged as text) are logging stubs; the DLL's sprintf (which faults in the checker, as its sscanf does) is a
# stub that formats the "%s: %i" line itself. No local is read before it is written.
# Negative check (mutation): W23_NEG=1 builds the C with W23H_MUTATE2 (the slide moves out when it should move in).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w23_screens.c'])
NEG = bool(os.environ.get('W23_NEG'))
if NEG:
    CFLAGS = CFLAGS + ['-DW23H_MUTATE2']
helper(0x100ba7d5, None, 3)
helper(0x100bb2b0, None, 2)
helper(0x100ecce0, None, 1)
helper(0x100652e8, None, 4)
helper(0x100cd70f, 'Sim_100cd70f', 3, deref={1: 8})
for a in (0x100f2f01, 0x100f1e10, 0x100ecea6):
    helper(a, None, 6, sargs=(4,))
helper(0x100fc780, None, 4, sargs=(1, 2))


def _sprintf(uc, k, raw, eax):
    name = ustr(uc, raw[2])
    num = raw[3] - (1 << 32) if raw[3] & 0x80000000 else raw[3]
    text = name + b': ' + str(num).encode() + b'\0'
    uc.mem_write(raw[0], text)
    return len(text) - 1


EFFECT[0x100fc780] = _sprintf
ACTIVE = (0x100fc780, 0x100ba7d5, 0x100bb2b0, 0x100ecce0, 0x100652e8, 0x100cd70f, 0x100f2f01, 0x100f1e10, 0x100ecea6)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    panel = newbuf(ctx, 0x100)
    w16(ctx, panel + 4, r.choice([0, 1, 1]))
    w32(ctx, panel + 0xf0, newbuf(ctx, 8))
    w32(ctx, 0x10151d54, r.choice([0, 1, 2, 2]))
    rets(0x100ba7d5, r.choice([0, 0x4444, 0x4444, 0x4444]))
    rets(0x100ecce0, r.choice([8, 10, 12]))
    w32(ctx, 0x1017e5e4, r.randrange(-50, 400))
    w32(ctx, 0x1017e5e8, r.choice([0x10000, 0x8000, r.getrandbits(32)]))
    w32(ctx, 0x1015dc80, r.randrange(0, 200))
    w32(ctx, 0x1017e5ec, r.choice([0, 1]))
    w32(ctx, 0x101c64f0, r.randrange(0, 3))
    for k in range(3):
        w32(ctx, 0x10151d58 + 4 * k, r.choice([100, 200, -60, 301]))
    n = r.randrange(0, 8)
    w32(ctx, 0x1015dc64, n)
    for i in range(8):
        u = 0
        if r.random() < 0.85:
            u = newbuf(ctx, 0x140)
            w16(ctx, u + 0x1c, r.choice([0, 0x800, 0x4000, 0x13]))
            wb(ctx, u + 0x124, ctx.buf(1, b'x') and b'Unit%d\0' % i)
        w32(ctx, 0x109e8b40 + 4 * i, u)
        rating = r.getrandbits(16)
        w16(ctx, 0x1015d0f0 + 2 * i, rating)
        w16(ctx, ctx.m.syms['g_w34Rating'] + 2 * i, rating)   # round 33: the C reads 1.2's ratings (same values)
    me = r.randrange(0, 8)          # the player's unit exists in the game (its +0x28 is read)
    if r32(ctx, 0x109e8b40 + 4 * me) == 0:
        w32(ctx, 0x109e8b40 + 4 * me, newbuf(ctx, 0x140))
    w32(ctx, 0x1017b8c8, me)
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    return [panel]


SPECS = [dict(name='Sim_100ce586', orig=0x100ce586, c='Sim_100ce586', gen=g, void=True, trials=300 if NEG else 4000)]
