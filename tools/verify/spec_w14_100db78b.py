# Round 14: 0x100db78b (object of a world file's OBJ chunk). Stubs on both sides: the resource loader 0x100ba7d5
# (answers by extension and writes the size), the error report 0x100bc280 (its two- and four-argument forms: the text
# argument logged), Sim_100e2a31 / Sim_100e3ba4 (answer models the trial made), Sim_100bb2b0 / Sim_100b9203 (texts
# logged), Sim_10060656 (writes a matrix made from its arguments), Sim_10009c55 (logs the 48-byte matrix) and the key,
# scene, unit and node helpers. Str_Copy and strncmp run for real. Chunk names are up to 9 characters in the 10-byte
# field and SDF part names up to 9 (the shipped range, DECISIONS.md round 14).
# Negative check: W14_NEG=1 gives SDF part names of 12..15 characters, which run past the original's 12-byte buffer.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_100d/w14_world.c'])
NEG = bool(os.environ.get('W14_NEG'))
CALLS = [
    (0x100ba7d5, None, 3), (0x100bc280, None, 2), (0x100db770, 'Sim_100db770', 1), (0x100b38c5, 'Sim_100b38c5', 1),
    (0x100b39c3, 'Sim_100b39c3', 17), (0x100b3839, 'Sim_100b3839', 1), (0x1003aa7d, 'Sim_1003aa7d', 9),
    (0x100e2776, 'Sim_100e2776', 3), (0x100e282a, 'Sim_100e282a', 1), (0x100e2a31, 'Sim_100e2a31', 4),
    (0x100e3ba4, 'Sim_100e3ba4', 5), (0x100bb2b0, 'Sim_100bb2b0', 2), (0x100b9203, 'Sim_100b9203', 2),
    (0x1002d890, 'Sim_1002d890', 2), (0x100b367e, 'Sim_100b367e', 2), (0x10060656, 'Sim_10060656', 7),
    (0x10009c55, 'Sim_10009c55', 2), (0x10089e91, 'Sim_10089e91', 1), (0x1003129f, 'Sim_1003129f', 1),
    (0x1003079c, 'Sim_1003079c', 1), (0x100b36d8, 'Sim_100b36d8', 1), (0x100096ee, 'Sim_100096ee', 1),
    (0x100a9830, 'Sim_100a9830', 2), (0x100a9b4c, 'Sim_100a9b4c', 2), (0x100096f9, 'Sim_100096f9', 2),
    (0x100aa0da, 'Sim_100aa0da', 5), (0x100a9d47, 'Sim_100a9d47', 4), (0x100aa742, 'Sim_100aa742', 1),
]
for a, c, n in CALLS:
    helper(a, c, n)
SARGS[0x100ba7d5] = {0, 1}
SARGS[0x100bb2b0] = {0, 1}
SARGS[0x100b9203] = {0, 1}
SARGS[0x100b39c3] = {1}
SARGS[0x1003aa7d] = {0}
DEREF[0x10009c55] = {1: 48}
MASK[(0x1003aa7d, 7)] = 0xffff
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


def _report(uc, k, raw, eax):
    if raw[0] != 0x38:
        return None
    esp = uc.reg_read(UC_X86_REG_ESP)
    extra = struct.unpack('<2I', bytes(uc.mem_read(esp + 12, 8)))
    return [('report_38', _cstr(uc, raw[1]), extra)], None


def _mat(uc, k, raw, eax):
    vals = [struct.unpack('<f', struct.pack('<I', x))[0] for x in raw[1:7]]
    uc.mem_write(raw[0], struct.pack('<12f', *[vals[i % 6] * (1.0 + i) for i in range(9)] + vals[3:6]))


EFFECT[0x100ba7d5] = _load
EFFECT[0x100bc280] = _report
EFFECT[0x10060656] = _mat


def name(r, lo, hi, first=b''):
    n = r.randrange(max(lo, len(first), 1), hi + 1)
    return first + bytes(r.choice(b'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_') for _ in range(n - len(first)))


def g(ctx):
    r = begin(ctx, *ACTIVE)
    T['size'] = r.randrange(16, 5000)
    T['wtb'] = [r.choice([0x00c10000, 0x00c10000, 0x00c10000, 0]) for _ in range(3)]
    sdf = ctx.buf(0x200, bytes(0x200))
    T['sdf'] = r.choice([sdf, sdf, 0])
    nparts = r.randrange(0, 11)
    for i in range(1, 10):
        if i < nparts:
            wb(ctx, sdf + 0xc + i * 0x20, (name(r, 12, 15) if NEG else name(r, 1, 9)) + b'\0')
        w32(ctx, sdf + 0x14c + i * 4, r.getrandbits(32))
    w32(ctx, sdf + 0x14c, r.getrandbits(32))
    models = []
    for i in range(4):
        mdl = newbuf(ctx, 0x40)
        w32(ctx, mdl + 0x34, r.choice([0, 5, 8, 3]))
        models.append(r.choice([mdl, mdl, mdl, 0]))
    rets(0x100e2a31, *models)
    rets(0x100db770, r.randrange(-3, 50), r.randrange(0, 50))
    rets(0x100b38c5, r.choice([-1, 0, 5]))
    rets(0x100b3839, r.choice([-1, 0, 7]), r.choice([1, 2]))
    rets(0x100b36d8, r.choice([0, 0x00c60000, 0xffffffff]))
    rets(0x100a9830, 0x00c30000, 0x00c30100)
    chunk = newbuf(ctx, 0x50)
    first = r.choice([b'o', b'O', b'oc', b'OC', b'oC', b'tl1m', b'TL1M', b'tl1', b'x', b''])
    wb(ctx, chunk + 0x3a, name(r, 1, 9, first) + b'\0')
    w16(ctx, chunk + 8, r.randrange(-5, 100))
    w16(ctx, chunk + 0xa, r.choice([-2, -2, -1, 0, 7]))
    w16(ctx, chunk + 0xc, r.choice([-1, 0, 3, 8, 9, 100]))
    for off in range(0xe, 0x32, 4):
        w32(ctx, chunk + off, r.choice([0, 1, -1, r.randrange(-100000, 100000), r.getrandbits(32),
                                        0x7fffffff, -0x80000000, 0x01000001]))
    w32(ctx, chunk + 0x34, r.getrandbits(32))
    w16(ctx, chunk + 0x38, r.getrandbits(16))
    w32(ctx, 0x101507f8, r.choice([-1, 0, 3]))
    n = r.randrange(0, 4)
    w32(ctx, 0x101c42a0, n)
    w32(ctx, 0x101a1160, r.choice([0, 2, 8]))
    w32(ctx, 0x1017d1f8, newbuf(ctx, 0x20))
    w32(ctx, 0x1017d1f4, newbuf(ctx, 0x20))
    return [chunk, r.getrandbits(32), r.getrandbits(32), r.choice([0, 0, 1]), r.choice([0, 0, 1]), r.getrandbits(32)]


SPECS = [dict(name='Sim_100db78b', orig=0x100db78b, c='Sim_100db78b', gen=g, void=True, trials=300 if NEG else 8000)]
