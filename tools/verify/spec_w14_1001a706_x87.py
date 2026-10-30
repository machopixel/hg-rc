# Round 14: bit-exact proof of the x87 reference Sim_1001a706_x87 (src/sim/util/w10_interp_x87.c; DECISIONS.md). Same
# generator and stubs as spec_w14_1001a706.py, but compared exactly (no float tolerance): the reference does the math
# the way the original's C runtime does, so it must match the original bit for bit.
exec(open('tools/verify/spec_w14_1001a706.py').read())

SOURCES = closure(['sim/util/w10_interp_x87.c'])
SPECS = [dict(SPECS[0], name='Sim_1001a706_x87', c='Sim_1001a706_x87', fp=False, trials=15000)]
