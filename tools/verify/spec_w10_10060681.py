# Round 10, interpreted (DECISIONS.md): 0x10060681 (rotation matrix to pitch / heading / roll). Float stubs answering in
# st(0) on both sides: Sim_10084b57 (horizontal length) and Sim_1008486e (heading). The CRT acos / asin of the original
# against the platform math library of the C (the host's math module through libm()): exact but for the pitch float,
# which may differ within the stated tolerance. Inputs outside acos's domain check the EDOM wrapper.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/util/w10_interp.c'])
FSTUB.update({0x10084b57: ('Sim_10084b57', 2), 0x1008486e: ('Sim_1008486e', 2)})


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx)
    fstubs(ctx)
    libm(ctx)
    mat = newbuf(ctx, 36)
    vals = [fb(r.uniform(-1, 1)) for _ in range(9)]
    vals[5] = r.choice([fb(r.uniform(-0.1, 0.1)), fb(r.uniform(-1, 1)), fb(0.1), fb(-0.1), 0x80000000, 0,
                        0x7fc00000, fb(r.uniform(0.09, 0.11)), fb(-r.uniform(0.09, 0.11))])
    w32(ctx, mat, *vals)
    ST['fvals'] = {0x10084b57: [r.choice([fb(r.uniform(0.004, 1.0)), fb(r.uniform(0, 0.004)), fb(0.003725290298462),
                                          fb(1.0), fb(r.uniform(0.9, 1.0)), fb(r.uniform(1.0, 2.0)), 0x7fc00000])],
                   0x1008486e: [fb(r.uniform(-180, 180)), fb(r.uniform(-180, 180))]}
    outs = [newbuf(ctx, 4) for _ in range(3)]
    return [mat] + outs


SPECS = [dict(name='Sim_10060681', orig=0x10060681, c='Sim_10060681', gen=g, void=True, trials=15000, fp=True,
              tol=1.2e-7)]
