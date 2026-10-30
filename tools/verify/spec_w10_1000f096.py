# Round 10: 0x1000f096 (distance to a -10000..0 level). Float, checked exact (no tolerance).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1000/w10_misc.c'])


def g(ctx):
    r = begin(ctx)
    x = r.random()
    if x < 0.3:
        v = struct.unpack('<I', struct.pack('<f', r.uniform(0.0, 1100.0)))[0]
    elif x < 0.45:
        v = struct.unpack('<I', struct.pack('<f', r.choice([50000.0, 49999.99, 50000.01, 1000.0, 999.99, -1000.0])))[0]
    elif x < 0.6:
        v = struct.unpack('<I', struct.pack('<f', r.uniform(-60000.0, 60000.0)))[0]
    else:
        v = rfloat(r)
    return [v]


SPECS = [dict(name='Sim_1000f096', orig=0x1000f096, c='Sim_1000f096', gen=g, trials=2000)]
