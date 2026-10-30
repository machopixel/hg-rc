# Round 10: 0x100d02bf (HUD pane with three lines). Stubbed on both sides: the pane's method +0x114 (IND1, text
# logged), the font loader 0x100ba7d5, the font height 0x100ecce0, the text routines (text logged by content),
# sprintf (0x100fc780 / Crt_SPrintf: format logged, the double as two words; it writes the same text on both runs
# into the 0x40-byte stack buffer, which the text routine then logs), Sim_100bb2b0 and Sim_100517dc.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100d/w10_misc.c'])
helper(IND1, None, 2, sargs=(1,))
helper(0x100ba7d5, None, 3, sargs=(0, 1))
helper(0x100ecce0, None, 1)
for _a in (0x100f2f01, 0x100f1e10, 0x100ecea6):
    helper(_a, None, 6, sargs=(4,))
helper(0x100fc780, 'Crt_SPrintf', 4, sargs=(1,))
helper(0x100bb2b0, 'Sim_100bb2b0', 2, sargs=(0, 1))
helper(0x100517dc, 'Sim_100517dc', 1)


def e_sprintf(uc, k, raw, eax):
    uc.mem_write(raw[0], b'%08x m\0' % (raw[2] ^ raw[3]))
    return 10


EFFECT[0x100fc780] = e_sprintf


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def txt(ctx, n):
    r = ctx.rng
    return bytes(r.choice(b'ABCdef 0123') for _ in range(r.randrange(0, n))) + b'\0'


def g(ctx):
    r = begin(ctx, IND1, 0x100ba7d5, 0x100ecce0, 0x100f2f01, 0x100f1e10, 0x100ecea6, 0x100fc780, 0x100bb2b0,
              0x100517dc)
    unit = newbuf(ctx, 0x40)
    rec = newbuf(ctx, 0x530)
    obj0 = newbuf(ctx, 0x60)
    w32(ctx, rec, obj0)
    w32(ctx, obj0 + 0x5c, fb(r.uniform(-100, 300)) if r.random() < 0.95 else rfloat(r))
    w32(ctx, rec + 0x520, fb(r.uniform(-100, 100)))
    w32(ctx, rec + 0x510, r.choice([0, 1, 0xffffffff, r.getrandbits(32)]))
    w32(ctx, unit + 0x28, rec)
    idx = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, idx)
    w32(ctx, 0x109e8b40 + 4 * idx, unit)
    pane = newbuf(ctx, 0x120)
    w16(ctx, pane + 4, r.choice([0, 1, 1, 0xffff]))
    wb(ctx, pane + 0x10, txt(ctx, 0x30))
    w32(ctx, pane + 0x114, IND1)
    owner = newbuf(ctx, 0x100)
    w32(ctx, owner + 0xf0, newbuf(ctx, 8))
    w32(ctx, 0x109e8938, owner)
    for a in (0x101c4c1c, 0x101c4c28):
        w32(ctx, a, ctx.buf(24, txt(ctx, 20).ljust(24, b'\0')))
    w32(ctx, 0x109b9900, ctx.buf(16, b'FONT'.ljust(16, b'\0')))
    w32(ctx, 0x1014cec4, ctx.buf(8, b'FNT\0\0\0\0\0'))
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    rets(0x100ba7d5, r.choice([0, 0x12345678, 0x12345678]))
    rets(0x100ecce0, r.randrange(0, 30), r.randrange(0, 30), r.randrange(0, 30))
    return [pane]


SPECS = [dict(name='Sim_100d02bf', orig=0x100d02bf, c='Sim_100d02bf', gen=g, void=True, trials=1500)]
