# Round 15: 0x100dcc12, the world-file (BWD) chunk reader. Each trial builds a small synthetic world file of 1..7
# chunks (every tag the reader knows, with field values in the shipped ranges of DECISIONS.md round 15, plus unknown
# tags) behind a reader record, and runs the original and the C on it. The chunk iterator 0x100279f1, the string and
# memory helpers, 0x100a9cfb / 0x100a9d21 and 0x1010d240 run for real, and so does the recursion: a PITF / VPTF
# chunk opens a nested file (the stub of 0x100278c0 answers a second synthetic file without PITF / VPTF). Every other
# callee is a logging stub; 0x100db78b logs the OBJ record it is given (fields +8..+0x39 and the name), so the
# records HGBT / HGXP build on the stack are compared whole.
#
# Start values (w15.h): the original's stack record dword +0x34 ([ebp-0x2e0] HGBT, [ebp-0x470] HGXP) is set to 0
# before the call, at the top level and one nested level (ebp - 0x88c); the GP gear pointer [ebp-0xcc] is set to an
# address in the checker's stack, which is not compared, so the original's store through it is not seen (the C stores
# nothing there).
# Negative checks: W15_NEG=1 leaves a non-zero colour in the original's stack record (shows the colour reaches the
# compared log); W15_NEG=2 gives PITF / VPTF names of 14..20 characters, which run past the original's 16-byte
# buffer [ebp-0x10c] into its pointer [ebp-0xfc].
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_100d/w15_world.c'])
NEG = int(os.environ.get('W15_NEG') or 0)
SAFE = bool(os.environ.get('W15_SAFE'))
CALLS = [
    (0x1007950a, 'Sim_1007950a', 1), (0x1003e291, 'Net_ProcessMessages', 0), (0x100797f2, 'Sim_100797f2', 0),
    (0x100bc280, None, 2), (0x100ba7d5, None, 3), (0x100d37ea, 'Sim_100d37ea', 2), (0x10084cee, 'Sim_10084cee', 3),
    (0x100da020, 'Sim_100da020', 4), (0x100da098, 'Sim_100da098', 1), (0x100db0ee, 'Sim_100db0ee', 1),
    (0x100dacf9, 'Sim_100dacf9', 1), (0x100daea4, 'Sim_100daea4', 1), (0x100dac0a, 'Sim_100dac0a', 1),
    (0x10048030, 'Sim_10048030', 3), (0x100e2756, 'Sim_100e2756', 3), (0x100b3fd5, 'Sim_100b3fd5', 9),
    (0x100b3b16, 'Sim_100b3b16', 1), (0x100b3f8f, 'Sim_100b3f8f', 0), (0x100b3f94, 'Sim_100b3f94', 1),
    (0x100db78b, 'Sim_100db78b', 6), (0x10019cc0, 'Sim_10019cc0', 1), (0x100db770, 'Sim_100db770', 1),
    (0x100b36d8, 'Sim_100b36d8', 1), (0x100b38c5, 'Sim_100b38c5', 1), (0x100b48de, 'Sim_100b48de', 2),
    (0x100b53bb, 'Sim_100b53bb', 1), (0x100096ee, 'Sim_100096ee', 1), (0x100a9300, 'Sim_100a9300', 1),
    (0x100b3839, 'Sim_100b3839', 1), (0x1002a1f2, 'Sim_1002a1f2', 2), (0x1003acf3, 'Sim_1003acf3', 1),
    (0x1003a990, 'Sim_1003a990', 1), (0x100278c0, 'Sim_100278c0', 2), (0x10027a8f, 'Sim_10027a8f', 1),
    (0x100466b6, 'Sim_100466b6', 1), (0x100462c0, 'Sim_100462c0', 8), (0x1003b417, 'Sim_1003b417', 1),
    (0x100dcbe0, 'Sim_100dcbe0', 1), (0x100dc2d7, 'GameThing_NextIndex', 0), (0x100b391c, 'Sim_100b391c', 1),
    (0x100b4342, 'Sim_100b4342', 3), (0x100bb2b0, 'Sim_100bb2b0', 2), (0x10079664, 'Sim_10079664', 3),
    (0x1003b50f, 'Sim_1003b50f', 2), (0x100aa351, 'Sim_100aa351', 1), (0x100aa406, 'Sim_100aa406', 1),
    (0x100aa4f0, 'Sim_100aa4f0', 2), (0x1000711c, 'Sim_1000711c', 2), (0x100b403d, 'Sim_100b403d', 1),
    (0x100b53f0, 'Sim_100b53f0', 4), (0x100a9ca3, 'Sim_100a9ca3', 4), (0x100a9d47, 'Sim_100a9d47', 4),
    (0x100aa799, 'Sim_100aa799', 1), (0x100aa0da, 'Sim_100aa0da', 5), (0x100dafa8, 'Sim_100dafa8', 2),
    (0x100db205, 'Sim_100db205', 1), (0x100599a0, 'Sim_100599a0', 1), (0x1005ada5, 'Sim_1005ada5', 1),
    (0x1005ae58, 'Sim_1005ae58', 2), (0x1005cd4a, 'Sim_1005cd4a', 2), (0x1005a138, 'Sim_1005a138', 1),
    (0x100526a9, 'Sim_100526a9', 2), (0x100526e3, 'Sim_100526e3', 2), (0x100db77e, 'Sim_100db77e', 1),
    (0x10048112, 'Sim_10048112', 4), (0x100d8936, 'Sim_100d8936', 1), (0x10058575, 'Sim_10058575', 0),
    (0x100585e1, 'Sim_100585e1', 3), (0x100da805, 'Sim_100da805', 1), (0x100daa08, 'Sim_100daa08', 1),
    (0x100b41dc, 'Sim_100b41dc', 4), (0x100cc6ea, 'Sim_100cc6ea', 4),
]
for a, c, n in CALLS:
    helper(a, c, n)
FSTUB[0x10089325] = ('Sim_10089325', 1)
SARGS[0x1007950a] = {0}
SARGS[0x100599a0] = {0}
SARGS[0x1005ae58] = {1}
SARGS[0x10079664] = {0}
DEREF[0x100585e1] = {2: 48}
MASK[(0x10079664, 2)] = 0xffff     # pushed from a 16-bit register: the upper half is left over (the callee masks)
MASK[(0x10048030, 1)] = 0xffff
MASK[(0x10048030, 2)] = 0xffff
MASK[(0x1000711c, 0)] = 0xffff
MASK[(0x1000711c, 1)] = 0xffff
MASK[(0x1003b50f, 1)] = 0xffff
ACTIVE = tuple(a for a, _, _ in CALLS)
T = {}
IMG = {}


def dll(addr, n):
    return bytes(T['ctx'].m.uc.mem_read(addr, n))


def tag(addr):
    return dll(addr, 4)


TAGS = {  # name: tag address
    'PLNT': 0x101349f4, 'CLIM': 0x10134a00, 'HTXT': 0x10134af8, 'VIEW': 0x10134ad4, 'LTBL': 0x10134aec,
    'MUSI': 0x10134ae4, 'ASND': 0x10134ae8, 'AFFL': 0x10134adc, 'TERR': 0x101349fc, 'INIT': 0x10134a10,
    'PALG': 0x101349f8, 'LITE': 0x10134a04, 'GNDM': 0x10134a38, 'HRZM': 0x10134a34, 'SKYM': 0x10134a3c,
    'VWSP': 0x10134a08, 'VWST': 0x10134a0c, 'BTAG': 0x10134b5c, 'ETAG': 0x10134b60, 'STBL': 0x10134a14,
    'STAR': 0x10134ad0, 'PTBL': 0x10134af0, 'FTBL': 0x10134af4, 'MTBL': 0x10134a18, 'FPRJ': 0x10134a30,
    'PLGO': 0x10134a40, 'BLKX': 0x10134a44, 'REPR': 0x10134a48, 'ENDR': 0x10134a4c, 'BLK': 0x10134a50,
    'ELSB': 0x10134a54, 'ENDB': 0x10134a58, 'OBJ': 0x10134a5c, 'ANIM': 0x10134a60, 'SCRG': 0x10134a64,
    'THNG': 0x10134a68, 'GP': 0x10134a6c, 'CPTF': 0x10134a70, 'PITF': 0x10134a74, 'VPTF': 0x10134a78,
    'HUDF': 0x10134a7c, 'MGDF': 0x10134a80, 'EYEO': 0x10134a84, 'POFO': 0x10134ad8, 'GT': 0x10134a88,
    'OBJL': 0x10134a8c, 'BTHG': 0x10134a90, 'HGBT': 0x10134b68, 'XPLO': 0x10134a94, 'HGXP': 0x10134b64,
    'NAVP': 0x10134a98, 'NAVO': 0x10134a9c, 'LITO': 0x10134aa0, 'TSK': 0x10134aa4, 'POS': 0x10134aa8,
    'ROT': 0x10134aac, 'INCL': 0x10134ab0, 'GRP': 0x10134ab4, 'GPS': 0x10134ab8, 'MOFF': 0x10134abc,
    'MON': 0x10134ac0, 'ANM2': 0x10134acc, 'WTHR': 0x10134b40, 'MLIT': 0x10134b48, 'CAMO': 0x10134b4c,
    'MLUT': 0x10134b54, 'TLUT': 0x10134b58, 'REV': 0x101349ec, 'DTBL': 0x101349f0,
}
HEAVY = ['GPS', 'GPS', 'GPS', 'GPS', 'GT', 'GT', 'HGBT', 'HGBT', 'HGXP', 'HGXP', 'GP', 'GP', 'PITF', 'VPTF', 'NAVP',
         'NAVO', 'TSK', 'PALG', 'LITE', 'PLNT', 'XPLO', 'BTHG', 'VWSP', 'SCRG', 'BTAG', 'ETAG', 'MTBL']


# ---- stub effects ------------------------------------------------------------------------------------
def _db78b(uc, k, raw, eax):
    rec = raw[0]
    return [('rec', bytes(uc.mem_read(rec + 8, 0x32)), _cstr(uc, rec + 0x3a, 0x40))], None


def _load(uc, k, raw, eax):
    if raw[2]:
        uc.mem_write(raw[2], struct.pack('<I', 0x100 + k))
    return T['res'][k % len(T['res'])]


def _open(uc, k, raw, eax):
    key = raw[0]
    extra = [('open', bytes(uc.mem_read(key, 2)), _cstr(uc, key + 2, 0x40))]
    return extra, T['nested'][k % len(T['nested'])]


def _three(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[1 + i], struct.pack('<I', T['f'][(k * 3 + i) % len(T['f'])]))


def _vec(uc, k, raw, eax):
    v = struct.unpack('<3f', bytes(uc.mem_read(raw[0], 12)))
    uc.mem_write(raw[0], struct.pack('<3f', v[1] * 0.5, v[2] + 1.0, v[0] - 3.0))


EFFECT[0x100db78b] = _db78b
EFFECT[0x100ba7d5] = _load
EFFECT[0x100278c0] = _open
EFFECT[0x100b53f0] = _three
EFFECT[0x100a9ca3] = _three
EFFECT[0x100b403d] = _vec
GUNS = {0x10152548, 0x10152584, 0x10152590, 0x101525e4, 0x10152604}


def _part(uc, k, raw, eax):
    # the gun parts (read with 0x100a9cfb right after) are mostly found; the other parts are found or not per call
    if T['safe'] and (raw[1] in GUNS or raw[1] in (0x101525a8, 0x101525bc)):
        return T['parts'][k % 3]
    if raw[1] == 0x10152590:     # the second gun of a walker: missing only gives report 0x56
        return T['ae'][k % len(T['ae'])]
    if raw[1] in GUNS:
        return T['parts'][k % 3] if T['ae'][k % len(T['ae'])] or k % 7 else 0
    return T['ae'][k % len(T['ae'])]


EFFECT[0x1005ae58] = _part


# ---- building blocks ----------------------------------------------------------------------------------
def name(r, lo, hi):
    return bytes(r.choice(b'abcdefghijklmnopqrstuvwxyz0123456789_') for _ in range(r.randrange(lo, hi + 1)))


def s16(r):
    return r.choice([-1, -1, 0, 1, 2, 5, 9, r.randrange(-40, 300), r.getrandbits(16) - 0x8000])


def fbits(r):
    return struct.unpack('<I', struct.pack('<f', r.uniform(-500.0, 500.0)))[0]


def chunk(r, t, size, extra=None):
    c = bytearray(rb(T['ctx'], size))
    c[0:4] = tag(TAGS[t]) if t != '?' else struct.pack('<I', r.getrandbits(32))
    c[4:8] = struct.pack('<I', size)
    if extra:
        extra(c)
    return bytes(c)


def put16(c, off, v):
    c[off:off + 2] = struct.pack('<H', v & 0xffff)


def put32(c, off, v):
    c[off:off + 4] = struct.pack('<I', v & 0xffffffff)


def putname(c, off, s, room):
    c[off:off + len(s) + 1] = s + b'\0'


def make_chunk(r, t, nested):
    ctx = T['ctx']
    if t == 'PLNT':
        def x(c):
            for off in range(0x28, 0x50, 4):
                put32(c, off, r.choice([0, 0, 1, -5, r.randrange(-1000, 100000)]))
            put32(c, 8, r.randrange(-100000, 100000))
            put32(c, 0x14, r.choice([0, 1, 50]))
        return chunk(r, t, 0x50, x)
    if t in ('HTXT', 'BTAG'):
        n = name(r, 0, 20)
        return chunk(r, t, 8 + len(n) + 1 + r.randrange(0, 4), lambda c: putname(c, 8, n, 0))
    if t in ('LTBL', 'MUSI', 'ASND', 'MLUT', 'TLUT'):
        n = name(r, 0, 12)
        return chunk(r, t, 0xa + len(n) + 1 + r.randrange(0, 4), lambda c: putname(c, 0xa, n, 0))
    if t == 'PALG':
        def x(c):
            for off in range(8, 0x148, 0x10):
                putname(c, off, name(r, 0, 8), 0x10)
        return chunk(r, t, 0x148, x)
    if t == 'LITE':
        def x(c):
            v = r.choice([None, 999999, 0xd9038])
            for off in (0x10, 0x14, 0x18):
                put32(c, off, v if v is not None and r.random() < 0.9 else r.randrange(-1000, 1000))
            put16(c, 0x1e, r.choice([0, 0, 1, 7]))
            put32(c, 0x20, r.choice([0, -1, 3]))
        return chunk(r, t, 0x24, x)
    if t == 'VWSP':
        n = r.choice([0, 1, 3, 8, 9])
        lst = ctx.buf(0x20 * max(n, 1) + 0x20)
        for i in range(max(n, 1) + 1):
            w32(ctx, lst + i * 0x20, r.choice([-1, 0, 4, 8, 9, r.randrange(0, 9)]))
        return chunk(r, t, 8 + 0x20 * n + r.randrange(0, 0x20), lambda c: put32(c, 8, lst))
    if t == 'FPRJ':
        def x(c):
            put16(c, 8, s16(r)); put16(c, 0xa, s16(r)); put16(c, 0xc, s16(r))
        return chunk(r, t, 0xe, x)
    if t == 'OBJ':
        return chunk(r, t, 0x50, lambda c: putname(c, 0x3a, name(r, 1, 9), 10))
    if t in ('SCRG', 'THNG', 'GP', 'EYEO', 'POFO', 'OBJL', 'BTHG', 'LITO'):
        def x(c):
            put16(c, 8, s16(r))
            put16(c, 0xa, r.choice([-1, 0, 1, 3, 10, 11, r.randrange(-3, 12)]))
        return chunk(r, t, 0xc, x)
    if t in ('PITF', 'VPTF'):
        n = name(r, 14, 20) if NEG == 2 else name(r, 1, 13)
        return chunk(r, t, 0xa + len(n) + 1, lambda c: (put16(c, 8, r.randrange(0, 50)), putname(c, 0xa, n, 0)))
    if t == 'GT':
        def x(c):
            put16(c, 8, s16(r)); put16(c, 0xa, s16(r))
            put16(c, 0x18, r.choice([0, 1, 2, -1, -2, -3, 32000, -32000, r.randrange(-500, 500)]))
            putname(c, 0x20, name(r, 0, 25), 0x16)
            c[0x36:0x4c] = name(r, 22, 22)
        return chunk(r, t, 0x50, x)
    if t in ('HGBT', 'HGXP'):
        base, cnt_off, name_off, ids = (0x20e, 0x23a, 0x23c, 0xe) if t == 'HGBT' else (0x210, 0x23c, 0x23e, 0x10)
        nm = name(r, 1, 9)

        def x(c):
            cnt = r.choice([0, 1, 2, 3, 5, -1])
            put16(c, cnt_off, cnt)
            for i in range(6):
                put16(c, ids + 2 * i, r.choice([-1, 0, r.randrange(0, 600)]))
            put16(c, 0xa, r.choice([-1, 0, 2, 20])); put16(c, 0xc, r.choice([-1, 0, 3, 0x4c, 0x4d, 0x3e]))
            for i in range(3):
                put32(c, base + 0xe + 4 * i, r.choice([0, 65536, -65536, r.randrange(-0x7fffffff, 0x7fffffff)]))
            putname(c, name_off, nm, 0)
        return chunk(r, t, name_off + len(nm) + 1, x)
    if t == 'XPLO':
        def x(c):
            put16(c, 8, s16(r)); put16(c, 0xa, r.choice([-1, 0, 2])); put16(c, 0xc, r.choice([-1, 0, 3, 0x4c, 0x4d]))
        return chunk(r, t, 0xe, x)
    if t == 'NAVP':
        def x(c):
            put32(c, 0x14, r.randrange(-100000, 100000))
            c[0x24:0x3c] = name(r, 24, 24)
        return chunk(r, t, 0x3c, x)
    if t == 'NAVO':
        return chunk(r, t, 0x10, lambda c: put16(c, 0xa, s16(r)))
    if t == 'TSK':
        txt = r.choice([b'12;oneshot', b'5;a,b,c', b'noscolon', b'-1;x', b'7;'])

        def x(c):
            put16(c, 8, r.choice([-1, 0, 2, 5, 6]))
            putname(c, 0xe, txt, 0)
        return chunk(r, t, 0xe + len(txt) + 1, x)
    if t in ('POS', 'ROT'):
        return chunk(r, t, 0x16, lambda c: put16(c, 8, s16(r)))
    if t == 'GPS':
        def x(c):
            c[0xc] = r.choice([0, 1, 5, 15, 16, 20])
            c[0xd] = r.choice([0, 1, 2])
            c[0xe] = r.choice([0, 0, 1, 2])
            put16(c, 0x20, r.getrandbits(16))
            c[0x24:0x2d] = name(r, 1, 8).ljust(9, b'\0') if r.random() < 0.8 else name(r, 9, 9)
            c[0x36:0x4c] = name(r, 22, 22)
            c[0x4c:0x62] = name(r, 22, 22)
        return chunk(r, t, 0x64, x)
    if t == 'MLIT':
        return chunk(r, t, 0x18)
    if t == 'CAMO':
        def x(c):
            put32(c, 8, r.randrange(-3, 12)); put32(c, 0xc, r.randrange(-3, 12))
        return chunk(r, t, 0x10, x)
    if t == 'ANM2':
        return chunk(r, t, 0x20)
    if t == 'BLKX':
        return chunk(r, t, 0x2c)
    if t in ('INIT',):
        return chunk(r, t, 0x18)
    if t in ('VWST', 'CLIM', 'VIEW', 'AFFL', 'PLGO', 'MGDF', 'HUDF', 'ANIM', 'CPTF', 'WTHR', 'INCL', 'GRP', 'BLK',
             'STBL', 'STAR', 'PTBL', 'FTBL', 'MTBL', 'GNDM', 'HRZM', 'SKYM'):
        return chunk(r, t, 0x24)
    return chunk(r, t, 8)


def make_file(r, n, nested):
    ctx = T['ctx']
    data = bytearray(tag(0x101349e8) + rb(ctx, 8))
    for _ in range(n):
        if r.random() < 0.6:
            t = r.choice(HEAVY)
        else:
            t = r.choice(sorted(TAGS) + ['?'])
        if nested and t in ('PITF', 'VPTF'):
            t = 'CPTF'
        data += make_chunk(r, t, nested)
    d = ctx.buf(len(data) + 16, bytes(data) + bytes(16))
    f = ctx.buf(0x40, 'zero')
    wb(ctx, f + 6, name(r, 1, 8) + b'\0')
    w32(ctx, f + 0xf, len(data))
    w32(ctx, f + 0x13, 0x1000)
    w32(ctx, f + 0x17, d)
    w32(ctx, f + 0x1b, d)
    return f


def part(ctx, r):
    p = newbuf(ctx, 0xc0)
    for off in (0x30, 0x34, 0x38, 0x70, 0x74, 0x78):
        w32(ctx, p + off, fbits(r))
    return p


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    T['ctx'] = ctx
    # a safe trial has no missing gear, unit or gun part (each of those makes both versions read address 0), so
    # every path also runs in trials that are compared; W15_SAFE=1 makes every trial safe (coverage of compared runs)
    T['safe'] = SAFE or r.random() < 0.5
    ST['fvals'] = {0x10089325: [fbits(r), fbits(r)]}
    T['f'] = [fbits(r) for _ in range(6)]
    # loader results (GT): the record +0x1d4 +8 / +0xc
    res = newbuf(ctx, 0x200)
    w32(ctx, res + 0x1dc, r.choice([0, 0, 7, r.getrandbits(32)]))
    w32(ctx, res + 0x1e0, r.choice([0, 0, 9, r.getrandbits(32)]))
    T['res'] = [r.choice([res, res, 0])]
    # world state
    cfg = newbuf(ctx, 0x40)
    w32(ctx, cfg + 0xf, r.choice([0, 0, 1234]))
    w32(ctx, cfg + 0x13, r.choice([0, 0, 77]))
    w8(ctx, cfg + 5, r.choice([0, 1]))
    w32(ctx, 0x1017b8c4, cfg)
    w32(ctx, 0x1015ec58, r.choice([0, 0, 1]))
    w32(ctx, 0x1014cb38, r.choice([0, 1]))
    w32(ctx, 0x1014cae0, newbuf(ctx, 0x40))
    w32(ctx, 0x1017e7e8, r.choice([0, 1]))
    w32(ctx, 0x1017d1fc, r.choice([0, 0, 1]))
    w32(ctx, 0x1017d200, r.randrange(0, 4))
    w32(ctx, 0x1017d224, r.choice([0, 1]))
    w32(ctx, 0x1017d1f4, newbuf(ctx, 0x40))
    w32(ctx, 0x101a1160, r.choice([0, 3, 8]))
    w32(ctx, 0x1017e6a0, r.choice([0, 5, 0x95, 0x96]))
    w32(ctx, 0x1015dc64, r.choice([0, 1, 3, 0x17, 0x18, 0x19]))
    w32(ctx, 0x1017b8c8, r.choice([0, 1, 2]))
    w32(ctx, 0x1017e818, r.choice([-1, 0, 7, 0x1ff, 0x200]))
    w32(ctx, 0x1017e81c, r.choice([-1, 0, 9, 0xff, 0x100]))
    w32(ctx, 0x1017e5a8, r.choice([-1, 0, 9, 0x7f, 0x80]))
    w32(ctx, 0x1017e820, r.randrange(0, 5))
    w32(ctx, 0x101c42a0, r.randrange(0, 5))
    w32(ctx, 0x1013bb40, r.choice([0, 1, 5]))
    w32(ctx, 0x1017e6ac, r.choice([0, 0x10000]))
    for grp in (0, 1, 5, 15):
        w32(ctx, 0x109b7d48 + grp * 0x6c, r.choice([0, 1]))
        w32(ctx, 0x109b7d44 + grp * 0x6c, r.randrange(0, 4))
    # gears: the table 0x109e8b40, each with a unit record +0x28
    for i in range(0x1a):
        gear = newbuf(ctx, 0x300)
        w32(ctx, gear + 0x28, r.choice([newbuf(ctx, 0xc00)] * 6 + [0] * (not T['safe'])))
        w32(ctx, 0x109e8b40 + 4 * i, gear if T['safe'] or r.random() < 0.95 else 0)
    cur = newbuf(ctx, 0x300)
    w32(ctx, cur + 0x14, r.choice([0, 0, 1]))
    w32(ctx, cur + 0x28, newbuf(ctx, 0xc00))
    w32(ctx, 0x1017e6b8, r.choice([0, cur, cur]))
    # models, objects, nodes and parts
    code = ctx.buf(8, r.choice([b'D1', b'd', b'G', b'g', b'SA', b'sa', b'SN', b'sn', b'Sx', b'v13x', b'v63x', b'X']).ljust(8, b'\0'))
    model = part(ctx, r)
    w32(ctx, model + 0x90, code)
    w32(ctx, model + 0x8c, newbuf(ctx, 0x40))
    parts = [part(ctx, r) for _ in range(3)]
    rets(0x1003b417, model, model, model, r.choice([0, model]))
    T['parts'] = parts
    T['ae'] = [r.choice(parts + [0, 0]) for _ in range(r.choice([5, 7, 11, 13]))]
    design = newbuf(ctx, 0x100)
    w32(ctx, design + 0xb4, r.choice([1, 2, 3, 4, 5, 8, 9, 10, 6, 7, 0, 11]))
    rets(0x100599a0, design)
    rets(0x1005ada5, r.randrange(0, 4))
    rets(0x1005cd4a, r.choice([0, 1, 1]))
    rets(0x100db770, *[r.randrange(-3, 40) for _ in range(3)])
    objs = [newbuf(ctx, 0x40) for _ in range(2)]
    rets(0x100b36d8, *[r.choice(objs + [0]) for _ in range(5)])
    rets(0x100096ee, *[r.choice(parts + [0]) for _ in range(5)])
    rets(0x100b38c5, *[r.choice([-1, 0, 3]) for _ in range(5)])
    rets(0x100b3839, *[r.choice([-1, 0, 2]) for _ in range(5)])
    rets(0x100b391c, r.choice([-1, 1]))
    rets(0x100b53bb, r.choice(objs + [0]))
    rets(0x100dc2d7, *[r.choice([-1, 0, 4, 10]) for _ in range(3)])
    rets(0x10058575, r.choice([-1, 0, 3]))
    # nested files for PITF / VPTF
    T['nested'] = [make_file(r, r.randrange(1, 4), True), 0, make_file(r, r.randrange(1, 4), True)]
    f = make_file(r, r.randrange(1, 8), False)
    # the original's start values (w15.h), at the top level and one nested level
    top = frame(1)
    scratch = STACK_TOP - 0x00F0000
    for ebp in (top, top - 0x88c):
        colour = r.getrandbits(32) | 1 if NEG == 1 else 0
        w32(ctx, ebp - 0x2e0, colour)
        w32(ctx, ebp - 0x470, colour)
        w32(ctx, ebp - 0xcc, scratch)
    return [f]


SPECS = [dict(name='Sim_100dcc12', orig=0x100dcc12, c='Sim_100dcc12', gen=g, trials=int(os.environ.get("W15_TRIALS") or (400 if NEG else 8000)))]
