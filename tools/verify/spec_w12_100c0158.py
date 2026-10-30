# Round 12: 0x100c0158 (position of a selected thing). Stubs on both sides: Sim_100b5386, Sim_100b53f0 (writes the
# three outputs), Sim_100aa863 (hands out a matrix). Mat3x4_TransformPoint runs for real. For a kind other than
# 0x200 / 0x400 the original's unset node [ebp-0x14] gets the chosen start value 0 as its stack leftover.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100c/w12_select.c'])
helper(0x100b5386, 'Sim_100b5386', 1)
helper(0x100b53f0, 'Sim_100b53f0', 4)
helper(0x100aa863, 'Sim_100aa863', 1)
ACTIVE = (0x100b5386, 0x100b53f0, 0x100aa863)
STACK_TOP = 0x00800000
EBP = STACK_TOP - 0x100 - 4 * 6 - 4 - 4
OUT = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def _53f0(uc, k, raw, eax):
    for i, a in enumerate(raw[1:4]):
        uc.mem_write(a, struct.pack('<I', OUT['v'][i]))
    return None


EFFECT[0x100b53f0] = _53f0


def g(ctx):
    r = begin(ctx, *ACTIVE)
    idx = r.randrange(0, 8)
    kind = r.choice([0x200, 0x400, 0x400, 0, 0x100, 0x300, 0xf00])
    sel = (r.getrandbits(32) & ~0xfff) | kind | idx if r.random() < 0.3 else kind | idx
    for i in range(8):
        u = newbuf(ctx, 0x70)
        w32(ctx, u + 0x48, r.getrandbits(32))
        w32(ctx, u + 0x68, r.getrandbits(32))
        w32(ctx, 0x109e8b40 + 4 * i, u)
        w32(ctx, 0x109e9ba8 + 0x50 * i, r.getrandbits(32))
    rets(0x100b5386, r.choice([0, 0, r.getrandbits(32)]))
    m = newbuf(ctx, 0x30)
    w32(ctx, m, *[fb(r.uniform(-2, 2)) for _ in range(12)])
    rets(0x100aa863, m)
    OUT['v'] = [fb(r.uniform(-500, 500)) for _ in range(3)]
    part = r.randrange(-3, 40)
    k = r32(ctx, 0x1012bb18)
    for p in range(-3, 40):
        for o in (0, 4):
            w32(ctx, 0x101516f8 + p * 8 + o, r.choice([k, 0x3f800000, fb(r.uniform(-8, 8)), fb(r.uniform(0.1, 4)),
                                                      0x7fc00000, 0]))
    x, y, out = newbuf(ctx, 4), newbuf(ctx, 4), newbuf(ctx, 4)
    w32(ctx, x, fb(r.uniform(-1000, 1000)))
    w32(ctx, y, fb(r.uniform(-1000, 1000)))
    w32(ctx, out, r.getrandbits(32))
    scale = r.choice([fb(r.uniform(-100, 100)), fb(1.0), 0, fb(r.uniform(0.01, 3))])
    w32(ctx, EBP - 0x14, 0)                  # the chosen start value as the original's leftover
    return [sel, part & 0xffff | (r.getrandbits(16) << 16), x, y, out, scale]


SPECS = [dict(name='Sim_100c0158', orig=0x100c0158, c='Sim_100c0158', gen=g, void=True, trials=4000)]