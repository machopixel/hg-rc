# Round 10: bit-exact proof of the x87 reference Sim_10060681_x87 (src/sim/util/w10_interp_x87.c; DECISIONS.md). Same
# generator and stubs as spec_w10_10060681.py, but compared exactly (no float tolerance): the reference does the math the
# way the original's C runtime does, so it must match the original bit for bit.
exec(open('tools/verify/spec_w10_10060681.py').read())

SOURCES = closure(['sim/util/w10_interp_x87.c'])
SPECS = [dict(SPECS[0], name='Sim_10060681_x87', c='Sim_10060681_x87', fp=False, trials=6000)]