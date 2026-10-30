# Round 18: 0x1005ab44, the walk of a gear design's node tree (DECISIONS.md round 18). The trees are the shipped GDF
# designs (HEAVYG.PRJ and the loose ones, the links turned into pointers as 0x10059a34 does) and random small trees
# (names empty, ordinary or 'g'-first, links at random) without the shape that would hand the original's
# never-written [ebp-0x10] to Sim_1005aa45 (a named node in a nameless node's sub-tree chain); the stack below the
# call is random. Sim_1005aa45 (logging stub: name, index, parent, low words of a4 / a5, the 9 transform floats,
# field) and Sim_1010c9f0 (stub that copies the name into a fresh buffer) are stubbed; Crt_StrUpper (C locale) and
# Crt_StrStr run for real.
# Negative check: W18_NEG=1 builds random trees that do have that shape: the original passes its leftover, the C -1.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())
import sys as _sys
_sys.path.insert(0, os.path.join('tools', 'prj'))
import prjtool as _prj

SOURCES = closure(['sim/unsorted_1005/w18_walk.c'])
NEG = bool(os.environ.get('W18_NEG'))
helper(0x1005aa45, 'Sim_1005aa45', 8, sargs=(0,), deref={6: 36})
MASK[(0x1005aa45, 4)] = 0xffff
MASK[(0x1005aa45, 5)] = 0xffff
helper(0x1010c9f0, 'Sim_1010c9f0', 1)
COPY = {}


def _dup(uc, k, raw, eax):
    s = ustr(uc, raw[0]) if raw[0] else b''
    if isinstance(s, tuple):
        s = b''
    a = COPY['bufs'][k % len(COPY['bufs'])]
    uc.mem_write(a, s[:0x3f] + b'\0')
    return a


EFFECT[0x1010c9f0] = _dup

_, _ents, _p = _prj.open_archive('C:/Coding/HeavyGear/GameRip')
GDF = [_prj.read_entry(_p, e) for e in _ents if e.ext.upper() == 'GDF']
for _f in glob.glob('C:/Coding/HeavyGear/GameRip/*.GDF'):
    GDF.append(open(_f, 'rb').read())


def items_of(d):
    n = struct.unpack_from('<i', d, 0x84)[0]
    return [bytearray(d[0x88 + i * 0x438:0x88 + (i + 1) * 0x438]) for i in range(n) if 0x88 + (i + 1) * 0x438 <= len(d)]


def bad_shape(items, field):
    def name(it, k):
        return bytes(it[0x7c + 0x20 * k:0x7c + 0x20 * k + 1])

    found = [False]

    def walk(known, node, depth):
        if node == 0 or node >= len(items) or depth > 60 or found[0]:
            return
        it = items[node]
        e = field + 1 if name(it, field) in (b'g', b'G') else field
        nxt, sub = struct.unpack_from('<II', it, 0x54)[0], struct.unpack_from('<I', it, 0x78)[0]
        if name(it, e) not in (b'', b'\0'):
            if not known:
                found[0] = True
            walk(known, nxt, depth + 1)
            walk(True, sub, depth + 1)
        else:
            walk(known, nxt, depth + 1)
            walk(False, sub, depth + 1)
    walk(True, struct.unpack_from('<I', items[0], 0x78)[0], 0)
    return found[0]


def random_items(r):
    n = r.randrange(2, 12)
    items = []
    for i in range(n):
        it = bytearray(r.getrandbits(8) for _ in range(0x438))
        for k in range(6):
            o = 0x7c + 0x20 * k
            kind = r.random()
            nm = b'' if kind < 0.35 else r.choice([b'g', b'G', b'']) + r.choice([b'pof_a', b'eye_l', b'sol_x', b'dum_1', b'body', b'arm']) + bytes([0x30 + i])
            it[o:o + 0x20] = (nm + b'\0' * 0x20)[:0x20]
        for off in (0x54, 0x78):
            nxt = r.randrange(i + 1, n) if i + 1 < n and r.random() < 0.6 else 0
            struct.pack_into('<I', it, off, nxt)
        for off in range(0x15c, 0x168, 4):
            struct.pack_into('<f', it, off, r.uniform(-50, 50))
        for j in range(10):
            for off in (0x24, 0x28, 0x2c):
                struct.pack_into('<f', it, 0x168 + j * 0x48 + off, r.uniform(-50, 50))
        items.append(it)
    return items


def g(ctx):
    r = begin(ctx, 0x1005aa45, 0x1010c9f0)
    w32(ctx, 0x1017e950, 0)
    COPY['bufs'] = [ctx.buf(0x40) for _ in range(8)]
    field = r.randrange(0, 4)
    while True:
        items = items_of(r.choice(GDF)) if (not NEG and r.random() < 0.5) else random_items(r)
        if items and bad_shape(items, field) == NEG:
            break
    base = ctx.buf(len(items) * 0x438, 'zero')
    for i, it in enumerate(items):
        it = bytearray(it)
        for off in (0x54, 0x78):
            idx = struct.unpack_from('<I', it, off)[0]
            struct.pack_into('<I', it, off, base + idx * 0x438 if 0 < idx < len(items) else 0)
        ctx.m.uc.mem_write(base + i * 0x438, bytes(it))
    counter = ctx.buf(4)
    w32(ctx, counter, r.randrange(0, 50))
    w32(ctx, 0x101c42a0, r.randrange(0, 100))
    lo = frame(6) - 0x3000
    ctx.m.uc.mem_write(lo, bytes(r.getrandbits(8) for _ in range(0x3000)))
    return [r.getrandbits(32) if r.random() < 0.5 else r.randrange(0, 40), counter, base + 0x168, 0,
            r32(ctx, base + 0x78), field]


SPECS = [dict(name='Sim_1005ab44', orig=0x1005ab44, c='Sim_1005ab44', gen=g, void=True, trials=300 if NEG else 4000)]
