# Round 24: 0x1000aa9e, the AI line-of-fire check (DECISIONS.md round 24). The original reads its never-written
# [ebp-0x64] (dy) and [ebp-4] (dz); the C reads g_w24AimDy / g_w24AimDz, which Sim_100cb4fb hands on from the unit's
# target check (the writer a watched original run showed for 223 of 223 reads). Each trial puts the same random
# values into the original's two stack slots (the frame of a call from this checker: esp at entry STACK_TOP - 0x110,
# ebp 4 lower) and into the C's two globals, so the proof covers the values explicitly: Sim_100cbc6b is a logging
# stub (its three floats logged, its four results written by the stub). The segment builders Sim_100d6f16 (fills
# the 0x38 bytes) and Sim_1009cd36 (its record logged whole), the hit searches Sim_1002dda9 / Sim_100c2851 (write
# the hit node) are stubs; the DLL's fmod runs for real.
# Negative check (mutation): W24_NEG=1 builds the C with W24S_MUTATE (the two values swapped).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1000/w24_sight.c'])
NEG = bool(os.environ.get('W24_NEG'))
if NEG:
    CFLAGS = CFLAGS + ['-DW24S_MUTATE']
helper(0x100cbc6b, 'Sim_100cbc6b', 7)
helper(0x100d6f16, 'Sim_100d6f16', 2)
helper(0x1009cd36, 'Sim_1009cd36', 2, deref={0: 0x38})
helper(0x1002dda9, 'Sim_1002dda9', 3)
helper(0x100c2851, 'Sim_100c2851', 4)
ST_W = {}
STACK_TOP = 0x00800000      # difftest.py's call stack
SLOT_DZ = STACK_TOP - 0x110 - 4 - 4
SLOT_DY = STACK_TOP - 0x110 - 4 - 0x64


def _angles(uc, k, raw, eax):
    for i, v in enumerate(ST_W['out']):
        uc.mem_write(raw[3 + i], struct.pack('<f', v))


def _segment(uc, k, raw, eax):
    uc.mem_write(raw[1], ST_W['seg'])


def _hit(uc, k, raw, eax):
    if eax:
        uc.mem_write(raw[1], struct.pack('<I', ST_W['hit']))


EFFECT[0x100cbc6b] = _angles
EFFECT[0x100d6f16] = _segment
EFFECT[0x1002dda9] = _hit
EFFECT[0x100c2851] = _hit
ACTIVE = (0x100cbc6b, 0x100d6f16, 0x1009cd36, 0x1002dda9, 0x100c2851)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    target, unit = newbuf(ctx, 0x100), newbuf(ctx, 0x100)
    for o in (0x58, 0x5c, 0x60):
        wf(ctx, target + o, r.uniform(-5000, 5000))
        wf(ctx, unit + o, r.uniform(-5000, 5000))
    wf(ctx, target + 0x68, r.uniform(-400, 400))
    wf(ctx, target + 0x74, r.uniform(-400, 400))
    w32(ctx, target + 4, r.randrange(0, 20))
    uid = r.randrange(0, 20) | r.choice([0, 0x400])
    w32(ctx, unit + 4, uid)
    w32(ctx, target + 0xf0, r.choice([uid | 0x200, uid, 0x205]))
    dy, dz = rfloat(r), rfloat(r)       # float bit patterns
    w32(ctx, SLOT_DY, dy)
    w32(ctx, SLOT_DZ, dz)
    w32(ctx, ctx.m.syms['g_w24AimDy'], dy)
    w32(ctx, ctx.m.syms['g_w24AimDz'], dz)
    ST_W['out'] = [r.uniform(-400, 400), r.uniform(0, 30000), r.uniform(-10, 10), r.uniform(-90, 90)]
    ST_W['seg'] = bytes(r.getrandbits(8) for _ in range(0x38))
    node = newbuf(ctx, 0x40)
    w16(ctx, node + 2, r.choice([0, 0x100, 0x200, 0x300]))
    w16(ctx, node + 0x24, r.choice([uid & 0xff, 3, (uid + 1) & 0xff]))
    ST_W['hit'] = r.choice([0, node, node])
    rets(0x1002dda9, r.choice([0, 0, 1]))
    rets(0x100c2851, r.choice([0, 1]))
    return [target, unit, r.choice([0, 1, 1])]


SPECS = [dict(name='Sim_1000aa9e', orig=0x1000aa9e, c='Sim_1000aa9e', gen=g, trials=300 if NEG else 4000)]
