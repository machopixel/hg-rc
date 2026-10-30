# Round 10: 0x10026057 (the running blast). Stubbed on both sides: Sim_100255de (its seven argument words logged; the
# original passes a seventh 0) and Sim_100257ec. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1002/w10_misc.c'])
helper(0x100255de, 'Sim_100255de', 6)
helper(0x100257ec, 'Sim_100257ec', 6)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, 0x100255de, 0x100257ec)
    w32(ctx, 0x1015dc0c, r.choice([0, -5 & 0xffffffff, 100, 3000]))
    w32(ctx, 0x1015dc80, r.choice([0, 16, 33]))
    w32(ctx, 0x1015dbf4, fb(r.uniform(0, 5000)))
    w32(ctx, 0x1015dbd0, fb(r.uniform(0, 8000)))
    for a in (0x1015dbf8, 0x1015dbfc, 0x1015dc00):
        w32(ctx, a, fb(r.uniform(-1000, 1000)))
    return []


SPECS = [dict(name='Sim_10026057', orig=0x10026057, c='Sim_10026057', gen=g, void=True, trials=2000)]
