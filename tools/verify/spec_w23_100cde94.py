# Round 23: 0x100cde94, the F12 objectives screen (DECISIONS.md round 23). The resource loader / release, the font
# height, Sim_1008d6f7 (its text logged), Sim_100ccc60 (its position pair logged whole), the time texts Sim_100cca20 /
# Sim_100ccaa0 and the three text routines (the line logged as text) are logging stubs; the DLL's sprintf (which faults
# in the checker) is a stub that formats the "%s%s" and "%s: %i" lines itself. Status 1 and above 4 draw the line the
# original never writes: the checker's stack starts zeroed, so the original draws an empty line there, as the C does.
# Negative check (mutation): W23_NEG=1 builds the C with W23F_MUTATE (the time to go shown for a zero +0xc).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w23_screens.c'])
NEG = bool(os.environ.get('W23_NEG'))
if NEG:
    CFLAGS = CFLAGS + ['-DW23F_MUTATE']
helper(0x100ba7d5, None, 3)
helper(0x100bb2b0, None, 2)
helper(0x100ecce0, None, 1)
helper(0x1008d6f7, 'Sim_1008d6f7', 6, sargs=(1,))
helper(0x100ccc60, 'Sim_100ccc60', 4, deref={1: 8})
helper(0x100cca20, 'Sim_100cca20', 1)
helper(0x100ccaa0, 'Sim_100ccaa0', 1)
for a in (0x100f2f01, 0x100f1e10, 0x100ecea6):
    helper(a, None, 6, sargs=(4,))
helper(0x100fc780, None, 4, sargs=(1, 2))


def _sprintf(uc, k, raw, eax):
    fmt = ustr(uc, raw[1])
    if fmt == b'%s%s':
        text = ustr(uc, raw[2]) + ustr(uc, raw[3])
    else:
        num = raw[3] - (1 << 32) if raw[3] & 0x80000000 else raw[3]
        text = ustr(uc, raw[2]) + b': ' + str(num).encode()
    uc.mem_write(raw[0], text + b'\0')
    return len(text)


EFFECT[0x100fc780] = _sprintf
ACTIVE = (0x100fc780, 0x100ba7d5, 0x100bb2b0, 0x100ecce0, 0x1008d6f7, 0x100ccc60, 0x100cca20, 0x100ccaa0,
          0x100f2f01, 0x100f1e10, 0x100ecea6)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    panel = newbuf(ctx, 0x100)
    w16(ctx, panel + 4, r.choice([0, 1, 1, 1]))
    pp = newbuf(ctx, 8)
    w32(ctx, pp, r.randrange(-20, 400))
    w32(ctx, pp + 4, r.randrange(-20, 400))
    w32(ctx, panel + 0xf0, pp)
    w32(ctx, panel + 0xec, newbuf(ctx, 0x40))
    w32(ctx, 0x1017e5e0, r.choice([0, 1, 1, 1, 2]))
    rets(0x100ba7d5, r.choice([0, 0x4444, 0x4444, 0x4444]))
    rets(0x100ecce0, r.choice([7, 8, 10, 12, -3]))
    rets(0x100cca20, ctx.buf(1, b'T1:23\0'))
    rets(0x100ccaa0, ctx.buf(1, b'0:45\0'))
    for a, t in ((0x101c4c4c, b'OBJECTIVES'), (0x101c4978, b'Time left '), (0x101c4c38, b'Time since '),
                 (0x101c4c3c, b'Won '), (0x101c4bf0, b'Lost '), (0x101c4bf4, b'Draw ')):
        w32(ctx, a, ctx.buf(1, t + b'\0'))
    w32(ctx, 0x1013bb40, r.randrange(0, 2))
    st = 0x10473800 + r32(ctx, 0x1013bb40) * 0x4fc3a
    wb(ctx, st + 0x38, bytes([r.choice([0, 0, 1, 2, 3, 4, 5, 200])]))
    w32(ctx, st + 4, r.randrange(-100, 100))
    w32(ctx, st + 8, r.randrange(-100, 1000))
    w32(ctx, st + 0xc, r.choice([0, 0, -5, 3, r.randrange(-50, 50)]))
    w32(ctx, 0x1015dc78, r.randrange(0, 20000))
    n = r.randrange(0, 8)
    w32(ctx, 0x1015dc64, n)
    for i in range(8):
        u = 0
        if r.random() < 0.85:
            u = newbuf(ctx, 0x140)
            w16(ctx, u + 0x1c, r.choice([0, 0x800, 0x4000, 0x13]))
            wb(ctx, u + 0x124, b'Unit%d\0' % i)
        w32(ctx, 0x109e8b40 + 4 * i, u)
        rating = r.getrandbits(16)
        w16(ctx, 0x1015d0f0 + 2 * i, rating)
        w16(ctx, ctx.m.syms['g_w34Rating'] + 2 * i, rating)   # round 33: the C reads 1.2's ratings (same values)
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    return [panel]


SPECS = [dict(name='Sim_100cde94', orig=0x100cde94, c='Sim_100cde94', gen=g, void=True, trials=300 if NEG else 4000)]
