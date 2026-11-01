# Differential tests for wave 2e (see difftest.py).
import struct

SOURCES = [
    # the converted code of this wave
    'sim/unsorted_100f/w2e_string.c',
    'sim/unsorted_1000/w2e_mono.c', 'sim/unsorted_1000/w2e_names.c',
    'sim/unsorted_1001/w2e_mono.c', 'sim/unsorted_1001/w2e_names.c', 'sim/unsorted_1001/w2e_tables.c',
    'sim/unsorted_1002/w2e_tables.c', 'sim/unsorted_1002/w2e_object.c',
    'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_1003/w2e_misc.c', 'sim/unsorted_1003/w2e_entries.c',
    'sim/unsorted_1004/w2e_strings.c', 'sim/unsorted_1005/w2e_lists.c', 'sim/unsorted_1006/w2e_copy.c',
    'sim/unsorted_1007/w2e_state.c', 'sim/unsorted_1009/w2e_copy.c', 'sim/unsorted_100a/w2e_search.c',
    'sim/unsorted_100b/w2e_strings.c', 'sim/unsorted_100b/w2e_object.c',
    'sim/unsorted_100c/w2e_tables.c', 'sim/unsorted_100d/w2e_init.c', 'sim/unsorted_100d/w2e_names.c',
    'sim/unsorted_100e/w2e_names.c',
    # converted functions they call
    'sim/core/timer.c', 'sim/core/w2a_timers.c', 'sim/unsorted_1003/slots.c',
    'sim/util/lookup.c', 'sim/unsorted_100d/state.c',
]
# The list above predates round 33 (the kill table g_w34Kills): what the files need to link now is added by
# w10_common's closure(), run in its own namespace so that nothing else of w10_common changes this spec.
_w10 = {}
exec(open('tools/verify/w10_common.py').read(), _w10)
SOURCES = _w10['closure'](SOURCES)
# some of the added files have frames over 4 KB: no probe call (rt.c has no __chkstk_ms), as in the other specs
CFLAGS = ['-mno-stack-arg-probe']

ALPHA = b'abAB01 _.-'


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def rnd(ctx, n):
    return bytes(ctx.rng.getrandbits(8) for _ in range(n))


def fill(ctx, addr, size):
    """Random dwords with many small values (counts, indices, flags)."""
    rng = ctx.rng
    words = [rng.choice([rng.getrandbits(32), rng.randrange(0, 40), rng.randrange(0, 40), 0xFFFFFFFF,
                         0x80000000, 0]) for _ in range((size + 3) // 4)]
    w32(ctx, addr, *words)


def rstr(ctx, maxlen, alpha=ALPHA):
    n = ctx.rng.choice([0, 1, 2, 3, 4, 5, 7, 8, 9, ctx.rng.randrange(0, maxlen + 1), ctx.rng.randrange(0, maxlen + 1)])
    return bytes(ctx.rng.choice(alpha) for _ in range(min(n, maxlen)))


def place(ctx, data, off=0, size=None, term=True, zero=False):
    """Puts data (+ terminator) at byte offset `off` of a fresh buffer, random bytes around (or zeros); returns the address."""
    total = max(len(data) + 1 + off + 24, size or 0)
    raw = bytearray(total if zero else rnd(ctx, total))
    raw[off:off + len(data)] = data
    if term:
        raw[off + len(data)] = 0
    a = ctx.buf(total, bytes(raw))
    return a + off


def hi16(ctx):
    return ctx.rng.getrandbits(16) << 16


# ---------------------------------------------------------------------------------------------
# library routines
# ---------------------------------------------------------------------------------------------
def g_strcpy(ctx):
    rg = ctx.rng
    s = rstr(ctx, 40, bytes(range(1, 256)) if rg.random() < 0.2 else ALPHA)
    src = place(ctx, s, rg.randrange(0, 5))
    dst = place(ctx, b'', rg.randrange(0, 5), size=80, term=False)
    return [dst, src]


def g_strncpy(ctx):
    rg = ctx.rng
    s = rstr(ctx, 30)
    src = place(ctx, s, rg.randrange(0, 5))
    dst = place(ctx, b'', rg.randrange(0, 5), size=100, term=False)
    n = rg.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 15, 16, 17, 33, rg.randrange(0, 70), len(s), len(s) + 1, len(s) + 2, len(s) - 1 if s else 0])
    return [dst, src, n]


def g_strlen(ctx):
    s = rstr(ctx, 50)
    return [place(ctx, s, ctx.rng.randrange(0, 5))]


def g_memset(ctx):
    rg = ctx.rng
    dst = place(ctx, b'', rg.randrange(0, 6), size=120, term=False)
    return [dst, rg.choice([0, 0x20, 0xff, -1, 0x1234, rg.getrandbits(32)]),
            rg.choice([0, 1, 2, 3, 4, 5, 7, 8, 9, 31, 32, 33, rg.randrange(0, 90)])]


def pair(ctx, maxlen=30, alpha=ALPHA):
    rg = ctx.rng
    a = rstr(ctx, maxlen, alpha)
    r = rg.random()
    if r < 0.3:
        b = a
    elif r < 0.5 and a:
        b = a[:rg.randrange(0, len(a))]
    elif r < 0.6:
        b = a + rstr(ctx, 5, alpha)
    elif r < 0.8 and a:
        i = rg.randrange(0, len(a))
        b = a[:i] + bytes([rg.choice(list(alpha) + [1, 200, 255])]) + a[i + 1:]
    else:
        b = rstr(ctx, maxlen, alpha)
    return a, b


def g_strcmp(ctx):
    rg = ctx.rng
    alpha = bytes(range(1, 256)) if rg.random() < 0.2 else ALPHA
    a, b = pair(ctx, 30, alpha)
    return [place(ctx, a, rg.randrange(0, 5)), place(ctx, b, rg.randrange(0, 5))]


def g_strncmp(ctx):
    rg = ctx.rng
    a, b = pair(ctx, 30)
    n = rg.choice([0, 1, 2, 3, 4, 5, 8, 16, 33, rg.randrange(0, 35), len(a), len(a) + 1])
    return [place(ctx, a, rg.randrange(0, 5)), place(ctx, b, rg.randrange(0, 5)), n]


def g_memmove(ctx):
    rg = ctx.rng
    n = rg.choice([0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, rg.randrange(0, 90)])
    base = ctx.buf(400, 'rand')
    src = base + 100 + rg.randrange(0, 4)
    r = rg.random()
    if r < 0.5:
        dst = src + rg.randrange(-40, 41)
    elif r < 0.6:
        dst = src
    else:
        dst = base + 220 + rg.randrange(0, 40)
    return [dst, src, n]


# ---------------------------------------------------------------------------------------------
# text screen
# ---------------------------------------------------------------------------------------------
def mono(ctx):
    """Maps the text screen (the original writes to physical 0xb0000) and makes it part of the compared memory."""
    m = ctx.m
    if not getattr(m, 'mono_mapped', False):
        m.uc.mem_map(0xb0000, 0x10000)
        m.wr.append((0xb0000, 0x10000))
        m.mono_mapped = True
        m.snap0 = m.dump()
    wb(ctx, 0xb0000, bytes([0xaa]) * 0x10000)
    w32(ctx, 0x10a3ade8, ctx.rng.choice([0, 1, 1, 1, 0x100]))
    wb(ctx, 0x10a3ae20, rnd(ctx, 0x60))
    wb(ctx, 0x10130ed0, rnd(ctx, 0x60))


def sh(ctx, v):
    return (v & 0xffff) | hi16(ctx)


def g_put(ctx):
    rg = ctx.rng
    mono(ctx)
    s = rstr(ctx, 100, bytes(range(1, 256)) if rg.random() < 0.3 else ALPHA)
    col = rg.choice([0, 1, 40, 78, 79, 79, 80, 81, -1, -5, 0x7fff, 0x8000, rg.randrange(-3, 85)])
    row = rg.choice([0, 1, 12, 23, 24, 24, 25, 26, -1, -9, 0x7fff, rg.randrange(-3, 28)])
    return [place(ctx, s, rg.randrange(0, 4)), sh(ctx, col), sh(ctx, row), sh(ctx, rg.choice([7, 0x78, 0x1ff, rg.getrandbits(16)]))]


def g_mono_none(ctx):
    mono(ctx)
    return []


def g_spinner(ctx):
    rg = ctx.rng
    mono(ctx)
    w32(ctx, 0x1012dca4, rg.choice([1, 1, 2, 3, 4, 0, rg.getrandbits(32)]))
    w32(ctx, 0x1012dca8, rg.choice([0, 1, 2, 3, 3, 0xffffffff, 5]))
    return []


def g_topline(ctx):
    rg = ctx.rng
    mono(ctx)
    s = rstr(ctx, 120, bytes(range(1, 256)) if rg.random() < 0.3 else ALPHA)
    return [place(ctx, s, rg.randrange(0, 4))]


def g_printtext(ctx):
    rg = ctx.rng
    mono(ctx)
    s = rstr(ctx, 130, b'abAB01 \n\n')
    w32(ctx, 0x1015d080, rg.choice([0, 1, 40, 78, 79, 80, 81, rg.randrange(0, 90), 0xffffffff]))
    w32(ctx, 0x1012dcac, rg.choice([3, 4, 20, 22, 23, 24, 0, rg.randrange(0, 30), rg.getrandbits(32)]))
    w32(ctx, 0x1012dc78, rg.getrandbits(32))
    return [place(ctx, s, rg.randrange(0, 4))]


# ---------------------------------------------------------------------------------------------
# name table at 0x10a39200 (0x1000df9c, 0x1000ecb0)
# ---------------------------------------------------------------------------------------------
def name_table(ctx):
    """Returns the names stored in the table."""
    rg = ctx.rng
    n = rg.choice([0, 0, 1, 2, 3, 5, 8])
    w32(ctx, 0x10a38de0, n)
    names = []
    for i in range(0, 12):
        nm = rstr(ctx, 6, b'abc')
        used = rg.choice([0, 1, 1, 2, rg.getrandbits(32)])
        wb(ctx, 0x10a39200 + i * 0x14, struct.pack('<I', used) + nm + b'\0' + rnd(ctx, 15 - len(nm)))
        names.append(nm)
    return names


def g_acquire(ctx):
    rg = ctx.rng
    names = name_table(ctx)
    nm = rg.choice(names[1:12]) if rg.random() < 0.5 else rstr(ctx, 6, b'abc')
    return [place(ctx, nm)]


def g_retire(ctx):
    rg = ctx.rng
    names = name_table(ctx)
    block = ctx.buf(0x310) if rg.random() < 0.9 else 0
    if block:
        for i in range(16):
            w32(ctx, block + 0x54 + i * 4, rg.choice([0, 1, 3, 5]))
            w32(ctx, block + 0x2d4 + i * 4, rg.choice([1, 2, 3, 4, rg.randrange(0, 12), rg.getrandbits(32)]))
    w32(ctx, 0x1015d0a4, block)
    nm = rg.choice(names[1:12]) if rg.random() < 0.6 else rstr(ctx, 6, b'abc')
    return [place(ctx, nm)]


# --- the three selectors (0x10010183, 0x10010267, 0x1001034b) and their caller 0x1000fee8
def selector_state(ctx):
    rg = ctx.rng
    idx = rg.choice([0, 0, 1, 2, 3])
    w32(ctx, 0x1013bb40, idx)
    block = 0x10473800 + idx * 0x4fc3a
    n = rg.choice([0, 1, 2, 4, 6, 10, 11, -1, 0x80000000])
    w32(ctx, block, n)
    for i in range(0, 12):
        e = block + 0x3a + i * 0x13f
        wb(ctx, e + 0x74, bytes([rg.choice([0, 1, 1, 7])]))
        w32(ctx, e + 2, rg.choice([0x8000, 0x8001, 0x8002, 0x8000, 0x8001, 0x8002, 0, rg.getrandbits(32)]))
        wb(ctx, e + 0xad, rstr(ctx, 8, b'xyz') + b'\0')
    wb(ctx, 0x101c5158 - 0, struct.pack('<I', place(ctx, rstr(ctx, 8, b'pqr'))))


def selector_obj(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x100, 'rand')
    for so, ro in ((0x1c, 0x24), (0x44, 0x4c), (0x6c, 0x74)):
        w32(ctx, obj + so, ctx.buf(48, 'rand'))
        rec = ctx.buf(48, 'rand')
        w32(ctx, rec + 8, rg.choice([0, 1, 5, rg.getrandbits(32)]) if rg.random() < 0.2 else rg.randrange(1, 100))
        w32(ctx, obj + ro, rec)
    return obj


def g_selector(off_rec):
    def gen(ctx):
        rg = ctx.rng
        selector_state(ctx)
        obj = selector_obj(ctx)
        r = rg.random()
        if r < 0.08:
            return [obj, 0]
        if r < 0.12:
            return [0, r32(ctx, obj + off_rec)]
        return [obj, r32(ctx, obj + off_rec)]
    return gen


def g_outer(ctx):
    rg = ctx.rng
    selector_state(ctx)
    obj = selector_obj(ctx)
    return [rg.getrandbits(32), obj if rg.random() < 0.9 else 0]


# ---------------------------------------------------------------------------------------------
# the 0x54-byte table at 0x109a8640 and the pointer table at 0x109e8b40 (0x1001570f, 0x10017852, 0x100c4699, 0x100ca417)
# ---------------------------------------------------------------------------------------------
KEYS = [0x11, 0x12, 0x13, 0x2a]


def table54(ctx):
    rg = ctx.rng
    n = rg.choice([-1, 0, 1, 2, 3, 4, 6, 8, rg.randrange(0, 9)])
    w32(ctx, 0x1017e5a8, n)
    for i in range(0, 10):
        e = 0x109a8640 + i * 0x54
        wb(ctx, e, rnd(ctx, 0x54))
        w32(ctx, e + 8, rg.choice(KEYS) | 0x200 if rg.random() < 0.7 else rg.getrandbits(32))
        w16(ctx, e + 0x24, rg.choice([1, 1, 0, 3, 0x101, rg.getrandbits(16)]))
    w32(ctx, 0x1017b8c8, rg.choice([0, 1, 2, 3, 0x77]))
    # records of the pointer table
    cnt = rg.choice([0, 1, 2, 4, 6, 8])
    w32(ctx, 0x1015dc64, cnt)
    ptrs = []
    for i in range(10):
        rec = ctx.buf(0x1f0, 'rand')

        def cb():
            return rg.choice([0, 0x100 | rg.randrange(0, 8), rg.randrange(0, 8), 0x101, 0x1ff, 0x200 | rg.randrange(0, 4), rg.getrandbits(16)])

        w16(ctx, rec + 0x1a8, cb() | (0x200 if rg.random() < 0.5 else 0))
        w16(ctx, rec + 0x1aa, cb())
        w32(ctx, rec + 0xf0, cb())
        w32(ctx, rec + 0x118, cb())
        w32(ctx, rec + 0x1d6, cb() | (0x1000 if rg.random() < 0.3 else 0))
        w32(ctx, rec + 4, rg.choice([0, 1, 2, 3, 0x77, rg.getrandbits(32)]))
        ptrs.append(rec if rg.random() < 0.9 else 0)
    w32(ctx, 0x109e8b40, *ptrs)
    return ptrs


def g_remove(ctx):
    rg = ctx.rng
    table54(ctx)
    a = rg.choice(KEYS)
    b = rg.choice([0x100 | rg.randrange(0, 8), 0x100 | rg.randrange(0, 4), rg.randrange(0, 8), 0x1ff, rg.getrandbits(32)])
    if rg.random() < 0.2:
        a = rg.getrandbits(32)
    return [a, b]


def g_resync(ctx):
    rg = ctx.rng
    ptrs = table54(ctx)
    obj = ctx.buf(0x200, 'rand')
    w32(ctx, obj + 4, rg.choice(KEYS))
    w32(ctx, obj + 0x1d6, rg.choice([0, 0x1000, 0x1003, 0x100, rg.randrange(0, 8), rg.getrandbits(32)]))
    return [obj]


def g_zero1d6(ctx):
    rg = ctx.rng
    table54(ctx)
    obj = ctx.buf(0x200, 'rand')
    w32(ctx, obj + 4, rg.choice(KEYS))
    w32(ctx, obj + 0x1d6, rg.choice([0, 0, 0, 1, 0x1000, rg.getrandbits(32)]))
    return [obj]


def g_collect2(ctx):
    rg = ctx.rng
    ptrs = table54(ctx)
    ptrs = [p if p else ctx.buf(0x1f0, 'rand') for p in ptrs]
    w32(ctx, 0x109e8b40, *ptrs)
    key = rg.randrange(0, 4)
    for p in ptrs:
        if p and rg.random() < 0.6:
            w16(ctx, p + 0x1a8, 0x200 | rg.choice([key, key, rg.randrange(0, 4)]))
    obj = ctx.buf(0x40, 'rand') if rg.random() < 0.92 else 0
    if obj:
        w32(ctx, obj + 4, key)
    wb(ctx, 0x1017e240, rnd(ctx, 0x80))
    return [obj, ctx.buf(8, 'rand')]


def g_tbl_none(ctx):
    rg = ctx.rng
    wb(ctx, 0x10132248, b''.join(struct.pack('<4H', rg.choice([0, 1, 2, 0x4af, 0x4b0, 0xfffe, 0x8000, rg.randrange(0, 0x4b5), rg.randrange(0, 20)]),
                                                 rg.getrandbits(16), rg.getrandbits(16), rg.getrandbits(16)) for _ in range(0x2c)))
    wb(ctx, 0x109f1520, rnd(ctx, 0x2600))
    return []


# ---------------------------------------------------------------------------------------------
# table initialisers (0x10021ac0 ...)
# ---------------------------------------------------------------------------------------------
def g_init_tables(ctx):
    wb(ctx, 0x1096fc80, rnd(ctx, 0x200 * 0x156 + 0x20))
    wb(ctx, 0x109a5a40, rnd(ctx, 0x100 * 0x2c + 0x20))
    wb(ctx, 0x1015dbd8, rnd(ctx, 0x24))
    wb(ctx, 0x101653e0, rnd(ctx, 0x1e0))
    return []


def g_idx2c(ctx):
    wb(ctx, 0x109a5a40, rnd(ctx, 0x100 * 0x2c + 0x40))
    return [ctx.rng.choice([0, 1, 5, 0x7f, 0xff, 0x100, ctx.rng.randrange(0, 0x100), 0xffffffff, 0xfffffff0])]


def g_idx156(ctx):
    wb(ctx, 0x1096fc80, rnd(ctx, 0x200 * 0x156 + 0x40))
    return [ctx.rng.choice([0, 1, 5, 0x1ff, 0x200, ctx.rng.randrange(0, 0x200), 0xffffffff, 0xfffffff0])]


def g_clr1d4(ctx):
    wb(ctx, 0x101653e0, rnd(ctx, 0x1e0))
    return []


def g_memmove3(ctx):
    return g_memmove(ctx)


def g_obj148(ctx):
    return [ctx.buf(0x160, 'rand')]


def g_objstr(ctx):
    rg = ctx.rng
    return [ctx.buf(0x60, 'rand'), place(ctx, rstr(ctx, 50), rg.randrange(0, 4))]


# ---------------------------------------------------------------------------------------------
# 0x1003xxxx
# ---------------------------------------------------------------------------------------------
def g_clr3524(ctx):
    wb(ctx, 0x109e4cc0, rnd(ctx, 0x3540))
    return []


def g_cpy100(ctx):
    rg = ctx.rng
    wb(ctx, 0x10139e58, rnd(ctx, 0x110))
    s = rstr(ctx, 300 if rg.random() < 0.3 else 40)
    return [place(ctx, s, rg.randrange(0, 4))]


def g_path(ctx):
    rg = ctx.rng
    n = rg.choice([0, 1, 2, 3, 5, 8, 12])
    s = b''
    for _ in range(n):
        s += bytes([rg.choice(b'ab\\\\.c')])
    if rg.random() < 0.2:
        s = b'\\' + s
    if rg.random() < 0.1:
        s = s + b'\\'
    return [place(ctx, s, rg.randrange(0, 4), size=200)]


def g_slotobj(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x700, 'rand')
    slots = ctx.buf(8 * 0x128, 'rand')
    for i in range(8):
        w32(ctx, slots + i * 0x128 + 4, rg.choice([-1, -1, 0, 1, 2, 3, rg.randrange(0, 20)]))
        w32(ctx, slots + i * 0x128 + 8, rg.choice([-1, 1, 0, 5]))
        w32(ctx, slots + i * 0x128 + 0x20, rg.choice([-2, 0, 3]))
    ents = ctx.buf(0x32 * 0x24 + 0x10, 'rand')
    for i in range(0x32):
        w16(ctx, ents + i * 0x24 + 2, rg.choice([0, 0, 0, 1, 0xffff, rg.getrandbits(16)]))
    top = ctx.buf(16, 'rand')
    w32(ctx, 0x1017b8c8, rg.choice([0, 1, 7]))
    w32(ctx, top + 4, rg.choice([r32(ctx, 0x1017b8c8), r32(ctx, 0x1017b8c8), 5]))
    w32(ctx, obj + 0, top)
    w32(ctx, obj + 0x84, slots)
    w32(ctx, obj + 0x8c, ents)
    w32(ctx, obj + 0x5fb, rg.choice([0, 1, 2, 5, rg.randrange(0, 20)]))
    objs = [ctx.buf(0x60, 'rand') for _ in range(8)]
    w32(ctx, 0x109e88ec, *objs)
    rec = ctx.buf(0x900, 'rand')
    w32(ctx, rec + 4, rg.getrandbits(32))
    w32(ctx, rec + 0x68c, rg.getrandbits(32))
    # slot image at rec+0xc: +4 (rec+0x10) index into the 0xd6-byte records, +0x38 count, +0x108 entry number
    w32(ctx, rec + 0x10, rg.choice([0, 1, 2, 5, rg.randrange(0, 30)]))
    cnt = rg.choice([0, 1, 2, 5, 0x20, 0x31, 0x32, 0x40, rg.randrange(0, 0x40)])
    w32(ctx, rec + 0x44, cnt)
    e108 = rg.choice([0, 1, 2, 3, rg.randrange(0, 0x30)])
    w32(ctx, rec + 0x114, e108)
    if rg.random() < 0.5:
        w32(ctx, rec + 0x44, e108 + 1)
    w32(ctx, rec + 0x14, rg.choice([-1, 1, 0, 5]))
    w32(ctx, rec + 0x2c, rg.choice([-2, 0, 3]))
    for k in range(0x32):
        w16(ctx, rec + 0x134 + k * 0x24 + 2, rg.choice([0, 0, 1, 0xffff, rg.getrandbits(16)]))
    w16(ctx, rec + 0x136 + e108 * 0x24, rg.choice([0, 0, 1, rg.getrandbits(16)]))
    wb(ctx, 0x10145592, rnd(ctx, 0xd6 * 32))
    return [obj if rg.random() > 0.01 else obj, rec]


def g_const3(ctx):
    rg = ctx.rng
    blk = ctx.buf(0x40, 'rand')
    w32(ctx, blk + 0xc, rg.choice([0, 1, 1, 0x1234]))
    w32(ctx, 0x1015d0a4, blk)
    wb(ctx, 0x109e36e0, rnd(ctx, 0x80))
    return []


def entries_state(ctx):
    rg = ctx.rng
    w32(ctx, 0x1015ea80, rg.choice([0, 1, 2, 5, 0x463, 0x464, 0x465, 0x466, 0xffffffff, rg.randrange(0, 12)]))
    w32(ctx, 0x1015ea84, rg.choice([0, 1, 1, 1]))
    wb(ctx, 0x109b9e20, rnd(ctx, 0x7e * 12 + 0x40))
    wb(ctx, 0x109b9e20 + 0x7e * 0x462, rnd(ctx, 0x7e * 6))


def g_addentry(ctx):
    rg = ctx.rng
    entries_state(ctx)
    name = rstr(ctx, 12)
    a5 = rg.choice([0, 0, 0, 1, 2, 3, 4, 5, -1])
    return [place(ctx, name, rg.randrange(0, 3)), ctx.u32(), ctx.u32(), ctx.u32(),
            rg.choice([0xffffffff, 0xffffffff, 0, 7, ctx.u32()]), a5 & 0xffffffff,
            rg.choice([-1, 0, 1, 2, 5, 0x464, 0x465, 0x466]) & 0xffffffff, (rg.getrandbits(16)) | hi16(ctx), ctx.u32()]


def g_resetentries(ctx):
    wb(ctx, 0x109b9e20, rnd(ctx, 0x7e * 0x466))
    w32(ctx, 0x1015ea80, ctx.u32())
    w32(ctx, 0x1015ea84, ctx.u32())
    return []


# ---------------------------------------------------------------------------------------------
# 0x1004xxxx
# ---------------------------------------------------------------------------------------------
def g_name18(ctx):
    rg = ctx.rng
    ref = rstr(ctx, 30, b'abc')
    wb(ctx, 0x1015d160, ref + b'\0' + rnd(ctx, 40))
    r = rg.random()
    if r < 0.5:
        s = ref
    elif r < 0.7:
        s = ref[:0x18] + b'zz'
    elif r < 0.85 and ref:
        i = rg.randrange(0, len(ref))
        s = ref[:i] + b'X' + ref[i + 1:]
    else:
        s = rstr(ctx, 30, b'abc')
    return [rg.getrandbits(32), place(ctx, s, rg.randrange(0, 4)), rg.getrandbits(32), ctx.buf(0x40, 'rand')]


def g_copy5f(ctx):
    rg = ctx.rng
    ref = rstr(ctx, 40, b'abc')
    wb(ctx, 0x1015db60, ref + b'\0' + rnd(ctx, 40))
    w32(ctx, 0x1015ec50, rg.choice([0, 1, 5]))
    dst = ctx.buf(0x80, 'rand')
    if rg.random() < 0.15:
        return [0, dst]
    src = ctx.buf(0x90, 'rand')
    r = rg.random()
    if r < 0.6:
        wb(ctx, src + 0x17, ref + b'\0')
    elif r < 0.8:
        wb(ctx, src + 0x17, ref[:0x20] + b'Q\0')
    return [src, dst]


def g_none(ctx):
    fill(ctx, 0x1015ed70, 0x130)
    fill(ctx, 0x1013aec4, 8)
    fill(ctx, 0x1015ed20, 0x20)
    fill(ctx, 0x1015ee98, 8)
    return []


def g_scrambled(ctx):
    rg = ctx.rng
    k = rg.choice([0, 1, 2, 3, 5, 8, rg.randrange(0, 14)])
    s = bytes(rg.randrange(1, 256) for _ in range(k))
    wb(ctx, 0x109b94be - 20, rnd(ctx, 21))
    m = k if rg.random() < 0.5 else rg.randrange(0, k + 1)
    for i in range(m):
        wb(ctx, 0x109b94be - i, bytes([s[k - 1 - i] ^ 9]))
    return [place(ctx, s, rg.randrange(0, 4))]


def g_xor10(ctx):
    rg = ctx.rng
    recs = []
    for i in range(16):
        ptrs = []
        for _ in range(2):
            r = rg.random()
            if r < 0.2:
                ptrs.append(0)
            else:
                ptrs.append(place(ctx, rstr(ctx, 15, bytes(range(1, 256)) if rg.random() < 0.3 else ALPHA)))
        recs.append((ptrs[0], ptrs[1]))
    shared = recs[3][0]
    if rg.random() < 0.3:
        recs[4] = (shared, recs[4][1])
    for i, (a, b) in enumerate(recs):
        w32(ctx, 0x1013b384 + i * 12, a, b, rg.getrandbits(32))
    return []


# ---------------------------------------------------------------------------------------------
# 0x100530a9 (list lookup) and 0x1005aa45
# ---------------------------------------------------------------------------------------------
def g_listfind(ctx):
    rg = ctx.rng
    n = rg.choice([0, 1, 2, 3, 4, 6])
    nodes = [ctx.buf(0x40, 'rand') for _ in range(n)]
    ids = [rg.choice([-2, -1, 0, 1, 2, 3]) for _ in range(n)]
    strs = [rstr(ctx, 6, b'ab') for _ in range(n)]
    for i, nd in enumerate(nodes):
        w32(ctx, nd, nodes[i + 1] if i + 1 < n else 0)
        w16(ctx, nd + 4, ids[i])
        wb(ctx, nd + 6, strs[i] + b'\0')
    w32(ctx, 0x1015f3b0, nodes[0] if nodes else 0)
    key = ctx.buf(0x30, 'rand')
    w16(ctx, key, rg.choice([-2, -1, 0, 1, 2, 3, 4]))
    wb(ctx, key + 2, (rg.choice(strs) if strs and rg.random() < 0.6 else rstr(ctx, 6, b'ab')) + b'\0')
    return [key]


def timer_tables(ctx):
    rg = ctx.rng
    for base in (0x10153180, 0x10153280):
        slots = [rg.choice([0, 0, 0, rg.getrandbits(32), 1, 0xfffffff0]) for _ in range(63)]
        w32(ctx, base, *(slots + [0xFFFFFFFF]))
    w32(ctx, 0x10153380, rg.choice([0, 1, 7, 1000, rg.getrandbits(32)]))
    w32(ctx, 0x10153384, rg.choice([0, 1, 7, 1000, rg.getrandbits(32)]))
    w32(ctx, 0x1015317c, rg.getrandbits(10))
    tn = lambda: rg.choice([0, 1, 5, 62, 0x80, 0x81, 0x85, 0xBE, rg.randrange(0, 0x100)])
    w32(ctx, 0x101354d0, tn())
    w32(ctx, 0x101354d4, tn())
    w32(ctx, 0x101354d8, tn())


def g_addpair(ctx):
    rg = ctx.rng
    entries_state(ctx)
    cnt = rg.choice([0, 1, 2, 3, 6, 0x462])
    w32(ctx, 0x101c42a0, cnt)
    A = ctx.buf(0x1000, 'rand')
    B = ctx.buf(0x1000, 'rand')
    w32(ctx, 0x1017d1f8, A)
    w32(ctx, 0x1017d1f4, B)
    off = rg.choice([0, 1, 5, 0x10, 0x100, 0xffffff00])
    w32(ctx, 0x1017e6a8, off)
    # keys the lookups will ask for: first = a1 + off, second = a2 + off; make some of them present in A
    a1 = rg.choice([0, 1, 7, 100, rg.getrandbits(32)])
    a2 = rg.choice([0, 1, 7, 100, rg.getrandbits(32)])
    for i in range(min(cnt, 0x3ff)):
        w32(ctx, A + i * 4, rg.choice([(a1 + off) & 0xffffffff, (a2 + off) & 0xffffffff, rg.getrandbits(32)]))
    for i in range(min(cnt, 0x3ff)):
        w32(ctx, B + i * 4, rg.choice([-1, 0, 1, 5, 0x462, 0x464, 0x465]))
    blk = ctx.buf(0x30, 'rand')
    return [place(ctx, rstr(ctx, 12), rg.randrange(0, 3)), a1, a2, rg.choice([-1, 0, 3, 8, 9, 100]),
            rg.getrandbits(32), rg.getrandbits(32), blk, rg.choice([0, 0, 0, 1, 2, 3, 4])]


# ---------------------------------------------------------------------------------------------
# 0x1006.. 0x1007.. 0x1009.. 0x100a..
# ---------------------------------------------------------------------------------------------
def g_copy30(ctx):
    rg = ctx.rng
    base = ctx.buf(300, 'rand')
    src = base + 100
    dst = rg.choice([src + rg.randrange(-0x30, 0x31), base + 200, base + 201, src])
    return [src, dst]


def g_copy38(ctx):
    rg = ctx.rng
    base = ctx.buf(300, 'rand')
    src = base + 100
    dst = rg.choice([src + rg.randrange(-0x38, 0x39), base + 200, src])
    return [dst, src]


def g_save68(ctx):
    rg = ctx.rng
    wb(ctx, 0x1014cae8, rnd(ctx, 0x68))
    w32(ctx, 0x1014caf8, rg.getrandbits(32))
    dst = ctx.buf(0x100, 'rand')
    if rg.random() < 0.1:
        dst = 0x1014cae8
    elif rg.random() < 0.1:
        dst = 0x1014cae8 + rg.randrange(-0x20, 0x20)
    return [dst]


def g_slotpick(ctx):
    rg = ctx.rng
    timer_tables(ctx)
    bufs = [ctx.buf(0x120, 'rand') for _ in range(2)]
    prios = [rg.choice([0, 1, 5, 5, 5, 7, 0x29b, 0x300, -3, rg.randrange(0, 12)]) for _ in range(2)]
    for i in range(2):
        e = 0x10149d28 + i * 0x3c
        wb(ctx, e, rnd(ctx, 0x3c))
        w32(ctx, e, bufs[i])
        w32(ctx, e + 0xc, rg.choice([0, 1, 1, 1, 2]))
        w32(ctx, e + 0x10, prios[i])
        w32(ctx, e + 0x34, rg.choice([0, 3, 3, 10, -5, rg.getrandbits(32)]))
    w32(ctx, 0x109b98c0, place(ctx, rstr(ctx, 10)))
    text = rstr(ctx, 300 if rg.random() < 0.2 else 40)
    label = rg.choice([0, place(ctx, b''), place(ctx, rstr(ctx, 10, b'qrs'))])
    return [place(ctx, text, rg.randrange(0, 3)), label, rg.choice([0, 5, 100, -7, rg.getrandbits(32)]),
            rg.choice([5, 5, 6, 0, 1, 12, 0x29a, 0x400, -1])]


def g_setk(ctx):
    rg = ctx.rng
    wb(ctx, 0x1017cd50 - 0x30, rnd(ctx, 0x1a * 0x16 + 0x90))
    inner = ctx.buf(16, 'rand')
    w32(ctx, inner + 4, rg.choice([0, 1, 5, 0x18, 0x19, 0x19, 0x1a, 0xffffffff, rg.randrange(0, 0x1c)]))
    outer = ctx.buf(16, 'rand')
    w32(ctx, outer, inner if rg.random() < 0.9 else 0)
    return [outer if rg.random() < 0.95 else 0, place(ctx, rstr(ctx, 6)), rg.getrandbits(32), place(ctx, rstr(ctx, 6))]


def g_search4(ctx):
    rg = ctx.rng
    idx = rg.choice([0, 1, 2, 3])
    ents = []
    cnt = rg.choice([0, 1, 2, 3, 5, 7])
    rec = ctx.buf(0xe + 8 * 0x18 + 8, 'rand')
    w16(ctx, rec + 0xc, cnt)
    for i in range(8):
        nm = bytes(rg.choice(b'ab') for _ in range(4)) + rnd(ctx, 4)
        ents.append(nm[:4])
        wb(ctx, rec + 0xe + i * 0x18, nm)
    w32(ctx, 0x101c65e6 + idx * 0x14a, rec)
    key = rg.choice(ents) if rg.random() < 0.6 else bytes(rg.choice(b'ab') for _ in range(4))
    if rg.random() < 0.5:
        key = key + b'xy'
    return [idx, place(ctx, key, rg.randrange(0, 3))]


# ---------------------------------------------------------------------------------------------
# 0x100bxxxx
# ---------------------------------------------------------------------------------------------
def g_cmp_indirect(ctx):
    rg = ctx.rng
    a, b = pair(ctx, 12)
    pa = ctx.buf(8, struct.pack('<II', place(ctx, a, rg.randrange(0, 3)), 0))
    pb = ctx.buf(8, struct.pack('<II', place(ctx, b, rg.randrange(0, 3)), 0))
    return [pa, pb]


def g_cmp4_at18(ctx):
    rg = ctx.rng
    base = bytes(rg.choice(b'ab') for _ in range(6))
    other = base if rg.random() < 0.5 else bytes(rg.choice(b'ab') for _ in range(6))
    a = ctx.buf(0x40, rnd(ctx, 0x18) + base + b'\0' + rnd(ctx, 0x20 - 7))
    b = ctx.buf(0x40, rnd(ctx, 0x18) + other + b'\0' + rnd(ctx, 0x20 - 7))
    return a, b


def g_100b971d(ctx):
    a, b = g_cmp4_at18(ctx)
    return [a, b + 0x18]


def g_100b9db0(ctx):
    a, b = g_cmp4_at18(ctx)
    return [a, b]


def g_cmp8(ctx):
    rg = ctx.rng
    base = bytes(rg.choice(b'ab') for _ in range(10))
    other = base if rg.random() < 0.4 else (base[:rg.randrange(0, 10)] + bytes(rg.choice(b'ab') for _ in range(10)))[:10]
    return [place(ctx, base, rg.randrange(0, 3)), place(ctx, other, rg.randrange(0, 3))]


def g_trim(ctx):
    rg = ctx.rng
    s = rstr(ctx, 8, b'ab')
    s += bytes(rg.choice(b' \t\r\n\x00x') for _ in range(rg.choice([0, 0, 1, 2, 4, 7])))
    s = s.split(b'\0')[0] if rg.random() < 0.7 else s
    if rg.random() < 0.15:
        s = bytes(rg.choice(b' \t\r\n') for _ in range(rg.randrange(0, 5)))
    return [place(ctx, s, rg.randrange(0, 3), size=64)]


def g_listfind4(ctx):
    rg = ctx.rng
    n = rg.choice([-1, 0, 1, 2, 3, 5])
    strs = [bytes(rg.choice(b'ab') for _ in range(4)) + b'\0' + rnd(ctx, 3) for _ in range(max(n, 0) + 1)]
    ptrs = [ctx.buf(8, s) for s in strs]
    arr = ctx.buf(4 * len(ptrs) + 4, struct.pack('<%dI' % len(ptrs), *ptrs) + b'\0\0\0\0')
    lst = ctx.buf(0x10, 'rand')
    w32(ctx, lst + 4, n)
    w32(ctx, lst + 8, arr)
    key = strs[rg.randrange(0, len(strs))][:4] if rg.random() < 0.6 else bytes(rg.choice(b'ab') for _ in range(4))
    return [place(ctx, key, rg.randrange(0, 3)), lst]


def g_ext(ctx):
    rg = ctx.rng
    n = rg.choice([0, 1, 2, 4, 8, 12])
    s = bytes(rg.choice(b'ab.') for _ in range(n))
    ext = rstr(ctx, 5, b'xyz')
    return [place(ctx, s, rg.randrange(0, 3), size=64), place(ctx, ext, rg.randrange(0, 3))]


def g_objreset(ctx):
    rg = ctx.rng
    obj = ctx.buf(0x300, 'rand')
    blk = ctx.buf(0x600, 'rand')
    w32(ctx, obj, rg.choice([0, 1, 1, 1, 2, 5]))
    w32(ctx, obj + 0x28, blk)
    w32(ctx, blk + 0x538, rg.choice([0, 1, 1, 2]))
    wb(ctx, obj + 0x1b7, bytes([rg.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 0x80, 0xff, 0xfe, rg.getrandbits(8)]) & 0xff]))
    w32(ctx, obj + 0x27c, rg.choice([0, 0, 0x1234]))
    w32(ctx, obj + 0x278, rg.choice([0, 0, 0x55]))
    w32(ctx, obj + 0x26c, rg.getrandbits(32))
    return [obj]


# ---------------------------------------------------------------------------------------------
# 0x100d7fc0, 0x100da4f7, 0x100e2837
# ---------------------------------------------------------------------------------------------
def g_blocks(ctx):
    wb(ctx, 0x10180980, rnd(ctx, 0x80 + 0x80 + 9 * 0x80c))
    return []


NAMES1 = [b'Ab', b'Abc', b'Abcd', b'Abcde', b'Abcdf', b'Ba', b'Bb', b'Bbbbb', b'a', b'z', b'9', b'Zzz', b'Zzy', b'Abce']


def name_node(ctx, name, key=None):
    n = ctx.buf(0x40, 'zero')
    k = struct.unpack('<i', (name + b'\0\0\0\0')[:4])[0] if key is None else key
    w32(ctx, n + 0x10, k)
    wb(ctx, n + 0x14, name + b'\0')
    return n


def build_names(ctx, count, realkeys):
    """Fills a 27-list table by running the original insertion; returns (table, names)."""
    rg = ctx.rng
    table = ctx.buf(27 * 4, 'zero')
    names = []
    for _ in range(count):
        nm = rg.choice(NAMES1)
        key = None if realkeys else rg.choice([1, 2, 3, 5, -4, 100])
        node = name_node(ctx, nm, key)
        ctx.m.call(0x100da4f7, [node, table])
        names.append(nm)
    return table, names


def g_insert(ctx):
    rg = ctx.rng
    real = rg.random() < 0.5
    table, names = build_names(ctx, rg.choice([0, 1, 2, 3, 5, 8, 12]), real)
    nm = rg.choice(NAMES1)
    key = None if real else rg.choice([1, 2, 3, 5, -4, 100])
    return [name_node(ctx, nm, key), table]


def g_find(ctx):
    rg = ctx.rng
    real = rg.random() < 0.8
    table, names = build_names(ctx, rg.choice([0, 1, 2, 3, 5, 8, 12]), real)
    nm = rg.choice(names) if names and rg.random() < 0.6 else rg.choice(NAMES1)
    return [place(ctx, nm, rg.randrange(0, 3), zero=rg.random() < 0.8), table]


def spec(name, addr, gen, trials=500, void=False):
    return dict(name=name, orig=addr, c=name, gen=gen, trials=trials, void=void)


SPECS = [
    # library routines
    spec('Str_Copy', 0x100fc690, g_strcpy, 600),
    spec('Str_CopyN', 0x100fc7f0, g_strncpy, 800),
    spec('Str_Length', 0x100fc8f0, g_strlen, 400),
    spec('Mem_Fill', 0x100fcab0, g_memset, 600),
    spec('Str_Compare', 0x100fcd30, g_strcmp, 800),
    spec('Mem_Move', 0x100fcdc0, g_memmove, 1500),
    spec('Str_CompareN', 0x100fe7f0, g_strncmp, 800),
    # text screen
    spec('Mono_PutText', 0x100346ad, g_put, 800, True),
    spec('Mono_Clear', 0x1003483c, g_mono_none, 300, True),
    spec('Mono_Enable', 0x10007a8e, g_mono_none, 300, True),
    spec('Mono_TickSpinner', 0x10008317, g_spinner, 500, True),
    spec('Mono_PutTopLine', 0x10008390, g_topline, 500, True),
    spec('Mono_ClearLog', 0x10008421, g_mono_none, 300, True),
    spec('Mono_PrintText', 0x1000847d, g_printtext, 800, True),
    spec('Mono_BlankRows', 0x10011be7, g_mono_none, 300, True),
    # name table
    spec('Sim_1000df9c', 0x1000df9c, g_acquire, 800),
    spec('Sim_1000ecb0', 0x1000ecb0, g_retire, 600, True),
    spec('Sim_1000fee8', 0x1000fee8, g_outer, 500),
    spec('Sim_10010183', 0x10010183, g_selector(0x24), 500, True),
    spec('Sim_10010267', 0x10010267, g_selector(0x4c), 500, True),
    spec('Sim_1001034b', 0x1001034b, g_selector(0x74), 500, True),
    # table of 0x54-byte entries
    spec('Sim_1001570f', 0x1001570f, g_resync, 800, True),
    spec('Sim_10017852', 0x10017852, g_zero1d6, 500, True),
    spec('Sim_1001f6c0', 0x1001f6c0, g_tbl_none, 300, True),
    spec('Sim_100c4699', 0x100c4699, g_collect2, 600),
    spec('Sim_100ca417', 0x100ca417, g_remove, 1000),
    # initialisers
    spec('Sim_10021ac0', 0x10021ac0, g_init_tables, 300, True),
    spec('Sim_10021be0', 0x10021be0, g_idx2c, 300, True),
    spec('Sim_10021c3c', 0x10021c3c, g_idx156, 300, True),
    spec('Sim_10025d93', 0x10025d93, g_clr1d4, 300, True),
    spec('Sim_10028571', 0x10028571, g_memmove3, 600),
    spec('Sim_1002858a', 0x1002858a, g_memset, 600),
    spec('Sim_10028810', 0x10028810, g_obj148, 300, True),
    spec('Sim_10028981', 0x10028981, g_objstr, 500, True),
    # 0x1003xxxx
    spec('Sim_10033c24', 0x10033c24, g_clr3524, 300),
    spec('Sim_100348bd', 0x100348bd, g_cpy100, 500),
    spec('Path_StripDirectory', 0x10036334, g_path, 800, True),
    spec('Sim_10038ad5', 0x10038ad5, g_slotobj, 1500, True),
    spec('Sim_10039725', 0x10039725, g_const3, 300, True),
    spec('Sim_1003aa7d', 0x1003aa7d, g_addentry, 1500),
    spec('Sim_1003ac07', 0x1003ac07, g_resetentries, 300, True),
    # 0x1004xxxx
    spec('Sim_10041cc6', 0x10041cc6, g_name18, 600, True),
    spec('Sim_10041d01', 0x10041d01, g_copy5f, 600, True),
    spec('Sim_10043f10', 0x10043f10, g_none, 300, True),
    spec('Sim_10044255', 0x10044255, g_none, 300, True),
    spec('Sim_10048762', 0x10048762, g_scrambled, 800),
    spec('Sim_100487d0', 0x100487d0, g_xor10, 400, True),
    # 0x1005xxxx
    spec('Sim_100530a9', 0x100530a9, g_listfind, 800),
    spec('Sim_1005aa45', 0x1005aa45, g_addpair, 1000, True),
    spec('Sim_10060209', 0x10060209, g_copy30, 600),
    # 0x1007xxxx
    spec('Sim_100787f4', 0x100787f4, g_save68, 400, True),
    spec('Sim_10078857', 0x10078857, g_save68, 400, True),
    spec('Sim_1007df6c', 0x1007df6c, g_slotpick, 1500),
    spec('Sim_1009cfbd', 0x1009cfbd, g_copy38, 600),
    spec('Sim_1009d84e', 0x1009d84e, g_setk, 800, True),
    spec('Sim_100a7da3', 0x100a7da3, g_search4, 800),
    # 0x100bxxxx
    spec('Str_CompareIndirect', 0x100b6b87, g_cmp_indirect, 600),
    spec('Sim_100b971d', 0x100b971d, g_100b971d, 500),
    spec('Sim_100b9db0', 0x100b9db0, g_100b9db0, 500),
    spec('Sim_100bcb52', 0x100bcb52, g_listfind4, 800),
    spec('Sim_100bcba7', 0x100bcba7, g_cmp8, 500),
    spec('Path_ReplaceExtension', 0x100bccff, g_ext, 800, True),
    spec('Sim_100bea42', 0x100bea42, g_objreset, 800, True),
    # 0x100dxxxx / 0x100exxxx
    spec('Sim_100d7fc0', 0x100d7fc0, g_blocks, 300, True),
    spec('Sim_100da4f7', 0x100da4f7, g_insert, 1500),
    spec('Sim_100e2837', 0x100e2837, g_find, 1500),
]
