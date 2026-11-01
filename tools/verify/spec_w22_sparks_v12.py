# Tiny sweep (AI list): the spark ring Sim_1009fada against 1.2's 0x1008c922, on spec_w22_1009fada.py's generator.
# 1.2's only change is the render mode it hands the renderer slot [0x109e3d30]: 0x12 where 1.0 has 0x10, 1.2's
# renumbering of the same mode (DECISIONS.md round 33, "Render modes renumbered in 1.2"); the C keeps 1.0's number
# for our renderers. The log masks bit 1 of that argument (0x12 / 0x10) and the address of the renderer's state
# block (0x109e3dc0, 1.2 0x107ee440, paired by the listings); the vertices and the texture record are compared whole.
# Negative check: W22_NEG=1 as in spec_w22_1009fada.py.
exec(open('tools/verify/spec_w22_1009fada.py').read())
exec(open('tools/verify/v12_common.py').read())
MASK[(RENDER, 0)] = 0xfffffffd
MASK[(RENDER, 1)] = 0
SPECS = [v12_spec(SPECS[0], 0x1008c922)]
