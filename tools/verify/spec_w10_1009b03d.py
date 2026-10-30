# Round 10: 0x1009b03d (HUD readout with the heading). Stubbed on both sides: the font loader 0x100ba7d5, the
# three text routines (text logged by content), Sim_100bb2b0 and sprintf (0x100fc780 / Crt_SPrintf: its float
# formatting needs the CRT's _cfltcvt table, which is not set up in the emulator), whose five dwords are logged
# (the format and the %s text by content, the double as its two words) and which writes the same text on both
# runs. The CRT fmod runs for real.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1009/w10_misc.c'])
helper(0x100ba7d5, None, 3, sargs=(0, 1))
for _a in (0x100f2f01, 0x100f1e10, 0x100ecea6):
    helper(_a, None, 6, sargs=(4,))
helper(0x100bb2b0, 'Sim_100bb2b0', 2, sargs=(0, 1))
helper(0x100fc780, 'Crt_SPrintf', 5, sargs=(1, 2))


def e_sprintf(uc, k, raw, eax):
    uc.mem_write(raw[0], b'txt%08x\0' % (raw[3] ^ raw[4]))
    return 11


EFFECT[0x100fc780] = e_sprintf


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def txt(ctx, n):
    r = ctx.rng
    return bytes(r.choice(b'ABCdef 0123:') for _ in range(r.randrange(0, n))) + b'\0'


def g(ctx):
    r = begin(ctx, 0x100ba7d5, 0x100f2f01, 0x100f1e10, 0x100ecea6, 0x100bb2b0, 0x100fc780)
    unit = newbuf(ctx, 0x80)
    idx = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, idx)
    w32(ctx, 0x109e8b40 + 4 * idx, unit)
    hd = r.choice([r.uniform(-720, 720), 0.0, 360.0, -0.0, 359.99, 45.5])
    w32(ctx, unit + 0x68, fb(hd) if r.random() < 0.95 else rfloat(r))
    obj = newbuf(ctx, 0x80)
    w32(ctx, obj, r.getrandbits(32))
    w32(ctx, obj + 0x30, ctx.buf(16, txt(ctx, 10).ljust(16, b'\0')))
    w32(ctx, obj + 0x40, ctx.buf(40, txt(ctx, 30).ljust(40, b'\0')))
    w32(ctx, obj + 0x44, ctx.buf(16, txt(ctx, 8).ljust(16, b'\0')))
    w32(ctx, obj + 0x48, newbuf(ctx, 64))
    v = (hd % 360.0 + 360.0) % 360.0
    w32(ctx, obj + 0x4c, r.choice([fb(v), fb(v), r.getrandbits(32), 0x7fc00000]))
    w32(ctx, obj + 0x18, r.choice([0, 1, 2, r.getrandbits(32)]))
    w32(ctx, obj + 0x1c, r.choice([0, 1, 2]))
    w32(ctx, 0x1014cec4, ctx.buf(8, b'FNT\0\0\0\0\0'))
    w32(ctx, 0x1015d1ac, r.choice([4, 4, 1, 2, 3]))
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    rets(0x100ba7d5, r.choice([0, 0x12345678, 0x12345678]))
    return [obj]


SPECS = [dict(name='Sim_1009b03d', orig=0x1009b03d, c='Sim_1009b03d', gen=g, void=True, trials=2000)]
