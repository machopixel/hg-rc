# Round 33: Sim_100877bc (the start of display mode 6, the renderer plugin) against 1.2's 0x10071b9c, which in its
# 3DFX / PowerVR mode (g_w33Voodoo = 1.2 [0x1014b954]) sets the screen format record's colour masks to 5:6:5. The
# mask setter (1.0 0x1003772b = 1.2 0x1002bd5b) is stubbed on both sides and logs its arguments; the record
# (1.0 0x109e3880 = 1.2 0x107edf60) is logged as 1.0's address.
# Negative check: W2K_NEG=1 builds the C with W2KV_MUTATE (1.0's empty start).
import os
exec(open('tools/verify/w32_common.py').read())

SOURCES = closure(['sim/unsorted_1008/w2k_misc.c', 'sim/render/w33_mode.c'])
if os.environ.get('W2K_NEG'):
    CFLAGS = list(CFLAGS) + ['-DW2KV_MUTATE']
stub12(0x1003772b, 'Sim_1003772b', 5, 0x1002bd5b)
GLOBALS = [('g_w33Voodoo', 0x1014b954, 4)]
ADDRS = {0x109e3880: 0x107edf60}


def g(ctx):
    r = begin(ctx, 0x1003772b)
    w32(ctx, ctx.m.syms['g_w33Voodoo'], r.choice([0, 1, r.getrandbits(32)]))
    return []


SPECS = [spec12('Sim_100877bc', 0x10071b9c, g, 300)]
