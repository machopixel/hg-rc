# Round 10, interpreted (DECISIONS.md): 0x1000cbaa (ballistic table of the weapon kinds). No callees but the math: the
# CRT asin of the original against the platform math library of the C (libm()). Exact but for the angle floats of
# the table, which may differ within the stated tolerance. A speed of 0 gives NaN products, which check the EDOM wrapper.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/util/w10_interp.c'])
KINDS = 0x10145508


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx)
    libm(ctx)
    w32(ctx, 0x1013abc0, r.choice([fb(0.0296), fb(r.uniform(0.001, 0.1))]))
    w32(ctx, 0x1015d0a0, r.choice([0, r.randrange(0, 100)]))
    for k in range(0x2e):
        rec = KINDS + k * 0xd6
        w32(ctx, rec + 0x86, r.choice([0, 0x40, 0x40, r.getrandbits(32)]))
        lo = r.uniform(0, 2000)
        w32(ctx, rec + 0x4c, fb(lo))
        w32(ctx, rec + 0x50, fb(lo + r.choice([r.uniform(1, 8000), 0.5, 0.0])))
        w32(ctx, rec + 0x3c, fb(r.choice([r.uniform(1, 40), r.uniform(10, 2000), 0.0])))
        w32(ctx, rec + 0x38, r.choice([1, 1, 1, 0, 2]))
    return []


SPECS = [dict(name='Sim_1000cbaa', orig=0x1000cbaa, c='Sim_1000cbaa', gen=g, void=True, trials=3000, fp=True,
              tol=1.2e-7)]
