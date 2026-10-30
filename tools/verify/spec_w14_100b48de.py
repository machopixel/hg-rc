# Round 14: 0x100b48de (model loader of a scene object). Stubs on both sides: the resource loader 0x100ba7d5 (answers
# by extension: the trial's WTB / SDF data or nothing, and writes the size), Sim_100e2a31 / Sim_100e3ba4, the error
# report 0x100bc280 (text logged), Sim_100bb2b0 / Sim_100b9203 (texts logged), Sim_10060220 (writes a matrix made from
# its arguments: its sin / cos are the modern-math case), the node and model helpers (Sim_10009c55 / Sim_100a9dc3 log
# the 48-byte matrix they read), and the float answers Sim_1002d961 / Sim_1008486e. Str_Copy, strstr, memset,
# Mat3x4_Multiply and Sim_10060209 run for real. Names are archive names of up to 8 characters and SDF sub-model names
# of up to 9 (the shipped range, DECISIONS.md round 14).
# Negative check: W14_NEG=1 gives sub-model names of 16..19 characters, which run into the original's loop counter.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_100b/w14_model.c'])
NEG = bool(os.environ.get('W14_NEG'))
CALLS = [
    (0x10028b50, 'Sim_10028b50', 0), (0x100093ea, 'Sim_100093ea', 1), (0x100ba7d5, None, 3),
    (0x100e2776, 'Sim_100e2776', 3), (0x100e282a, 'Sim_100e282a', 1), (0x100e2a31, 'Sim_100e2a31', 4),
    (0x100e3ba4, 'Sim_100e3ba4', 5), (0x100bc280, None, 4), (0x100bb2b0, 'Sim_100bb2b0', 2),
    (0x100b9203, 'Sim_100b9203', 2), (0x10060220, 'Sim_10060220', 8), (0x10009c55, 'Sim_10009c55', 2),
    (0x10009408, 'Sim_10009408', 2), (0x1000942e, 'Sim_1000942e', 2), (0x1002d890, 'Sim_1002d890', 2),
    (0x10089e91, 'Sim_10089e91', 1), (0x100a9b4c, 'Sim_100a9b4c', 2), (0x100096f9, 'Sim_100096f9', 2),
    (0x100a9dc3, 'Sim_100a9dc3', 2), (0x100aa799, 'Sim_100aa799', 1), (0x100a9830, 'Sim_100a9830', 2),
    (0x100aa0da, 'Sim_100aa0da', 5), (0x100a9d47, 'Sim_100a9d47', 4), (0x100aa4f0, 'Sim_100aa4f0', 2),
    (0x100aa541, 'Sim_100aa541', 2), (0x100aa592, 'Sim_100aa592', 2), (0x1003129f, 'Sim_1003129f', 1),
    (0x1003079c, 'Sim_1003079c', 1),
]
for a, c, n in CALLS:
    helper(a, c, n)
SARGS[0x100ba7d5] = {0, 1}
SARGS[0x100bc280] = {2}
SARGS[0x100bb2b0] = {0, 1}
SARGS[0x100b9203] = {0, 1}
DEREF[0x10009c55] = {1: 48}
DEREF[0x100a9dc3] = {1: 48}
MASK[(0x10009408, 1)] = 0xffff
MASK[(0x1000942e, 1)] = 0xffff
FSTUB[0x1002d961] = ('Sim_1002d961', 3)
FSTUB[0x1008486e] = ('Sim_1008486e', 2)
ACTIVE = tuple(a for a, _, _ in CALLS)
T = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def _load(uc, k, raw, eax):
    ext = _cstr(uc, raw[1])
    v = T['sdf'] if ext == b'SDF' else T['wtb'][k % len(T['wtb'])]
    if v and raw[2]:
        uc.mem_write(raw[2], struct.pack('<I', T['size'] + k))
    return v


def _mat(uc, k, raw, eax):
    vals = [struct.unpack('<f', struct.pack('<I', x))[0] for x in raw[1:7]]
    m = [vals[i % 6] * (0.5 + 0.25 * i) for i in range(9)] + vals[3:6]
    uc.mem_write(raw[0], struct.pack('<12f', *m))


EFFECT[0x100ba7d5] = _load
EFFECT[0x10060220] = _mat


def name(r, lo, hi, first=b''):
    n = r.randrange(max(lo, len(first), 1), hi + 1)
    return first + bytes(r.choice(b'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_') for _ in range(n - len(first)))


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    ST['fvals'] = {0x1002d961: [fb(r.uniform(-50, 50))], 0x1008486e: [fb(r.uniform(-180, 180)), fb(r.uniform(-90, 90))]}
    T['size'] = r.randrange(16, 5000)
    T['wtb'] = [r.choice([0x00c10000, 0x00c10000, 0x00c10000, 0]) for _ in range(3)]
    sdf = ctx.buf(0x200, bytes(0x200))
    T['sdf'] = r.choice([sdf, sdf, 0])
    nparts = r.randrange(0, 11)
    for i in range(1, 10):
        if i < nparts:
            s = name(r, 16, 19) if NEG else name(r, 1, 9)
            wb(ctx, sdf + 0xc + i * 0x20, s + b'\0')
        w32(ctx, sdf + 0x14c + i * 4, r.getrandbits(32))
    w32(ctx, sdf + 0x14c, r.getrandbits(32))
    rets(0x10028b50, r.choice([0, 1]))
    rets(0x100093ea, r.choice([0, 0x400, r.getrandbits(32)]))
    rets(0x100e2a31, *[r.choice([0x00c20000 + 0x100 * i, 0x00c20000 + 0x100 * i, 0]) for i in range(4)])
    rets(0x100a9830, 0x00c30000, 0x00c30100)
    for p in range(6):
        w32(ctx, 0x101a212c + p * 0x88, r.getrandbits(32))
    idx = r.randrange(0, 6)
    e = 0x101a2100 + idx * 0x88
    first = r.choice([b'o', b'O', b'oc', b'OC', b'oC', b'B', b'x', b''])
    nm = name(r, 1, 8, first)
    wb(ctx, e, rb(ctx, 0x88))
    wb(ctx, e, (nm if r.random() < 0.95 else b'') + b'\0')
    w32(ctx, e + 0x14, r.choice([-2, -2, -1, -1, 0, 3]))
    w32(ctx, e + 0x18, r.choice([0, 0x400, 0x800, 0x5000, 0x8000, 0x8400]) | r.getrandbits(9))
    w32(ctx, e + 0x24, r.choice([-1, -1, 0, 2]))
    w32(ctx, e + 0x28, r.choice([0, 0, 0x00c40000]))
    w32(ctx, e + 0x2c, r.choice([0, 0x00c50000, 0x00c50000]))
    for off in range(0x30, 0x84, 4):
        w32(ctx, e + off, fb(r.uniform(-500, 500)))
    if r.random() < 0.3:
        wb(ctx, e, b'oBOLD' + b'\0')
    for t in range(3):
        w16(ctx, 0x109e9ba0 + t * 0x50, r.choice([0, 0x8000]))
    scene = r.randrange(0, 3)
    for off in range(0, 0x30, 4):
        w32(ctx, 0x101a11c8 + scene * 0x7c + off, fb(r.uniform(-2, 2)))
    for a in (0x1015dd4c, 0x1015dd50, 0x1015dd54):
        w32(ctx, a, fb(r.uniform(-1, 1)))
    w32(ctx, 0x1015267c, r.getrandbits(8))
    return [idx, scene]


SPECS = [dict(name='Sim_100b48de', orig=0x100b48de, c='Sim_100b48de', gen=g, trials=300 if NEG else 8000)]
