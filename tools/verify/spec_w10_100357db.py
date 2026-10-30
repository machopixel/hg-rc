# Round 10: 0x100357db (debug overlay of object names). Stubbed on both sides: the filter function [0x1014cb40]
# (IND1), fopen / fscanf / fclose (Sim_100fd850, Sim_100fe450, Crt_FClose), the projection Sim_100d2669 (writes the
# same three floats through its pointers on both runs), the box 0x100eb6e8, the font loader 0x100ba7d5 and the text
# routine 0x100ecea6 (text logged by content). Str_Copy and Str_CompareNoCase run for real (C locale, [0x1017e950] =
# 0). Drawing only; checked exact anyway.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w10_misc.c'])
helper(IND1, None, 1)
helper(0x100fd850, 'Sim_100fd850', 2, sargs=(0, 1))
helper(0x100fe450, 'Sim_100fe450', 3, sargs=(1,))
helper(0x100fd760, 'Crt_FClose', 1)
helper(0x100d2669, 'Sim_100d2669', 3)
helper(0x100eb6e8, None, 6)
helper(0x100ba7d5, None, 3, sargs=(0, 1))
helper(0x100ecea6, None, 6, sargs=(4,))


def e_proj(uc, k, raw, eax):
    p = ST['proj'][k % len(ST['proj'])]
    for i in range(3):
        uc.mem_write(raw[i], struct.pack('<I', p[i]))
    return None


EFFECT[0x100d2669] = e_proj


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


NAMES = [b'TRHP', b'trhp', b'GEAR', b'jaguar', b'', b'TRHPX']


def g(ctx):
    r = begin(ctx, IND1, 0x100fd850, 0x100fe450, 0x100fd760, 0x100d2669, 0x100eb6e8, 0x100ba7d5, 0x100ecea6)
    w32(ctx, 0x1017e950, 0)
    w32(ctx, 0x1014cb40, IND1)
    cam = ctx.buf(0xa0, b''.join(struct.pack('<I', fb(r.uniform(0.2, 3.0))) for _ in range(40)))
    w32(ctx, 0x1014cae0, cam)
    wb(ctx, 0x1015e508, r.choice(NAMES) + b'\0')
    w32(ctx, 0x1015e554, r.choice([0, 0x5555]))
    w32(ctx, 0x10179cc0, r.choice([0, 1]))
    w32(ctx, 0x1015e550, r.choice([0, 1]))
    w32(ctx, 0x109b9900, ctx.buf(8, b'FONT\0\0\0\0'))
    w32(ctx, 0x1014cec4, ctx.buf(8, b'FNT\0\0\0\0\0'))
    lst = newbuf(ctx, 0x10)
    prev = lst + 0
    first = 0
    ents = []
    for i in range(r.randrange(0, 6)):
        e = newbuf(ctx, 0xc0)
        w16(ctx, e, r.choice([0, 0x800, r.getrandbits(16)]))
        w16(ctx, e + 2, r.choice([0x100, 0x150, 0x50, 0x10, r.getrandbits(16)]))
        wb(ctx, e + 0xa4, r.choice(NAMES) + b'\0')
        for off in range(0x44, 0x8c, 4):
            w32(ctx, e + off, fb(r.uniform(-500, 500)) if r.random() < 0.95 else rfloat(r))
        rec = 0
        if r.random() < 0.6:
            rec = newbuf(ctx, 8)
            lead = 0
            if r.random() < 0.7:
                lead = newbuf(ctx, 0x90)
                o = newbuf(ctx, 0xc0)
                wb(ctx, o + 0xa4, r.choice(NAMES) + b'\0')
                w32(ctx, lead + 0x8c, o if r.random() < 0.8 else 0)
            w32(ctx, rec, lead)
        w32(ctx, e + 0x28, rec)
        ents.append(e)
    for i, e in enumerate(ents):
        w32(ctx, e + 8, ents[i + 1] if i + 1 < len(ents) else 0)
    w32(ctx, lst + 8, ents[0] if ents else r.choice([0, 0]))
    w32(ctx, lst + 4, r.choice([0, 1]))
    ST['proj'] = [[fb(r.uniform(-50, 700)), fb(r.uniform(-50, 500)), fb(r.uniform(0.5, 900)) if r.random() < 0.95 else rfloat(r)] for _ in range(6)]
    rets(IND1, r.choice([0, 0, 1]), 0)
    rets(0x100fd850, r.choice([0, 0x4000]))
    rets(0x100d2669, r.choice([1, 1, 0]), 1)
    rets(0x100ba7d5, r.choice([0, 0x7777]))
    if r.random() < 0.03:
        lst = 0
    return [lst]


SPECS = [dict(name='Sim_100357db', orig=0x100357db, c='Sim_100357db', gen=g, void=True, trials=3000)]
