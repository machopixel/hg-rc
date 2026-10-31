# Round 32, D: the particle draw Sim12_10078c29 and its sort comparison Sim12_10078be0 (1.2 only) against 1.2's own
# code. Stubbed on both sides: the frame setter (1.0 0x1000716a = 1.2 0x1000204c), the random ring (1.0 0x1009c929 =
# 1.2 0x10089e69; its answers follow the call number) and the polygon routine (the C's slot [0x1014cb4c], 1.2's
# [0x1011fa54]), which logs the four corners whole. The C library qsort runs for real (1.0's for the C, 1.2's for
# 1.2): the screen points are compared as memory after the sort.
# Negative check: W32_NEG=1 builds the C with W32D_MUTATE (the frame is not set).
import os, struct
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/fx/w32_draw.c'])
if os.environ.get('W32_NEG'):
    CFLAGS = list(CFLAGS) + ['-DW32D_MUTATE']
DRV = 0x1010d3e6
helper(DRV, None, 3, deref={1: 4 * 0x24})
helper(0x1000716a, 'Sim_1000716a', 2)
helper(0x1009c929, 'Sim_1009c929', 1)
MASK[(0x1000716a, 1)] = 0xffff
PAIRS12 = {0x1000716a: 0x1000204c, 0x1009c929: 0x10089e69}


def e_rand(uc, k, raw, eax):
    m = raw[0] if raw[0] else 1
    return (k * 2654435761 + ST['seed']) % m


EFFECT[0x1009c929] = e_rand


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, DRV, 0x1000716a, 0x1009c929)
    ST['seed'] = r.getrandbits(16)
    count = r.choice([0, 1, 2, 3, 5, 9, 16])
    rec = newbuf(ctx, 0x406)
    for i in range(16):
        w32(ctx, rec + i * 0x32 + 0x30, r.choice([0, 1, 5, 31, 32, r.getrandbits(32)]))
        w32(ctx, rec + i * 0x32 + 0x38, r.choice([0, 1, 8, 16, 33, -3, r.randrange(-100, 100), r.getrandbits(32)]))
    pts = newbuf(ctx, 0x10 * 16)
    depths = [r.uniform(1, 5000) for _ in range(3)]
    for i in range(16):
        p = pts + i * 0x10
        wf(ctx, p, r.choice([r.uniform(-50, 700), r.uniform(0, 640)]))
        wf(ctx, p + 4, r.choice([r.uniform(-50, 500), r.uniform(0, 480)]))
        wf(ctx, p + 8, r.choice(depths + [r.uniform(-10, 9000), float('nan')] if r.random() < 0.05 else depths +
                                [r.uniform(1, 9000)]))
        w32(ctx, p + 0xc, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    flags = r.choice([0, 0x10, 0x100, 0x1ff, 0x105, 0x200, 0x1234, 0x2105, 0x5180, 0xa010, 0x80000105,
                      r.getrandbits(16), r.getrandbits(32)])
    w32(ctx, 0x1014cb4c, DRV)
    painter_slot(ctx)
    return [rec, pts, flags, count]


def to12(m):
    if not getattr(m, '_w32d', False):
        m._w32d = True
        for a10, a12 in PAIRS12.items():
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, a10), None, A12(a12), A12(a12))
    m.uc.mem_write(A12(0x1011fa54), struct.pack('<I', DRV))


SPECS = [dict(name='Sim12_10078c29', orig12=0x10078c29, c='Sim12_10078c29', gen=g, void=True, trials=3000,
              to12=to12, from12=lambda m: None)]
