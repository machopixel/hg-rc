# Round 10: 0x1008dba2 (wrapped HUD text). Stubbed on both sides: the font height 0x100ecce0, the character widths
# 0x100f2cec / 0x100eccf3 (width from the character, the same on both runs) and the three text routines (each
# line logged by content). HeapAlloc answers a buffer of the generator; HeapAlloc / HeapFree are logged by the
# checker. Str_Length, Str_Copy and Rect_ScalePoint (converted) run for real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1008/w10_misc.c'])
helper(0x100ecce0, None, 1)
helper(0x100f2cec, None, 2)
helper(0x100eccf3, None, 2)
for _a in (0x100f2f01, 0x100f1e10, 0x100ecea6):
    helper(_a, None, 6, sargs=(4,))


def e_width(uc, k, raw, eax):
    return ST['wbase'] + (raw[1] & 0xff) % ST['wmod']


EFFECT[0x100f2cec] = e_width
EFFECT[0x100eccf3] = e_width


def g(ctx):
    r = begin(ctx, 0x100ecce0, 0x100f2cec, 0x100eccf3, 0x100f2f01, 0x100f1e10, 0x100ecea6)
    ST['wbase'] = r.choice([0, 1, 4, 8])
    ST['wmod'] = r.choice([1, 3, 7])
    words = [bytes(r.choice(b'abcdefXYZ019') for _ in range(r.randrange(0, 12))) for _ in range(r.randrange(0, 12))]
    seps = [r.choice([b' ', b' ', b'\n', b'\t', b'  ', b'\n\n']) for _ in words]
    text = b''.join(w + s for w, s in zip(words, seps))[:r.choice([20, 60, 200])]
    tbuf = ctx.buf(len(text) + 1, text + b'\0')
    ST['heap'] = ctx.buf(len(text) + 16, b'\xcc' * (len(text) + 16))
    pane = newbuf(ctx, 20)
    left, top = r.randrange(-20, 200), r.randrange(-20, 200)
    w32(ctx, pane + 4, left, top, left + r.randrange(-5, 400), top + r.randrange(-5, 300))
    w32(ctx, 0x1014b9a8, r.randrange(0, 0x20000), r.randrange(0, 0x20000))
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    rets(0x100ecce0, r.choice([0, 1, 8, 12, 40]))
    font = r.choice([0x1234, 0x1234, 0x1234, 0])
    if r.random() < 0.03:
        tbuf = 0
    if r.random() < 0.03:
        pane = 0
    return [pane, tbuf, font]


SPECS = [dict(name='Sim_1008dba2', orig=0x1008dba2, c='Sim_1008dba2', gen=g, void=True, trials=3000,
              api={'HeapAlloc': lambda n, a: ST['heap']})]
