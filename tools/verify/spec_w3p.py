# Differential tests for round 3 of letter p (see difftest.py): network message senders, small x87 float
# functions, and the fog/visibility/light functions of the renderer path.
#
# Stubs in the emulator (one `ret` each at fixed addresses, the same for the original and the C run, with a
# code hook that appends the arguments to a log buffer which is compared afterwards with all other memory):
#   dp      the network DLL's dpSend (dword 0x1015eab4): 6 arguments and the first bytes of the message buffer
#   count   dword 0x1015ead8 (session -> count): returns a per-trial value
#   fn5     dword 0x1015eae0: 5 arguments; may change the word 0x1015ecd8 like a real call could
#   fn3     dword 0x1015eb20: 3 arguments
#   v12c, v14, v10   function pointers stored in objects (offsets 0x12c / 0x14 / 0x10)
#   cmp     the compare callback of the sorted-list insert (signed compare of the two dwords, or a hash)
#   drv     the display driver slot 0x1014cb4c: count, mode and the 36-byte vertices
# Float functions: the control word is set to 0x27f (53-bit precision, exceptions masked, the Windows default) and the
# FPU stack is emptied before every trial. All float results are compared bit for bit (stricter than the fp=True tolerance:
# the C does the same double arithmetic as the 53-bit x87 original, so no deviation is needed). Functions that return a
# float in st(0) are called through a trampoline that stores st(0) into eax (original) / a C wrapper (tools/verify/w3p_test.c).
import math
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG

SOURCES = [
    'sim/unsorted_1000/w3p_misc.c', 'sim/unsorted_1002/w3p_objects.c', 'sim/unsorted_1003/w3p_misc.c',
    'sim/unsorted_1003/w3p_net.c', 'sim/unsorted_1004/w3p_net.c', 'sim/unsorted_1004/w3p_calc.c',
    'sim/unsorted_1006/w3p_box.c', 'sim/unsorted_1007/w3p_net.c', 'sim/unsorted_1009/w3p_box.c',
    'sim/unsorted_100b/w3p_misc.c', 'sim/unsorted_100c/w3p_misc.c', 'sim/unsorted_100d/w3p_vec.c',
    'sim/unsorted_100e/w3p_misc.c', 'sim/render/w3p_fog.c', 'sim/render/w3p_view.c',
    # callees that are already converted
    'sim/unsorted_1003/records.c', 'sim/unsorted_1004/tables.c', 'sim/unsorted_1004/w2h_record.c',
    'sim/unsorted_1004/w2i_misc.c', 'sim/unsorted_1001/w2h_objects.c', 'sim/unsorted_1005/w2h_tables.c', 'sim/unsorted_100d/w2i_select.c', 'sim/unsorted_1007/w2f_misc.c',
    'sim/unsorted_100f/w2e_string.c', 'sim/util/ring.c', 'sim/render/w2m_texture.c', 'sim/render/w2n_texture.c',
    'sim/render/w2n_project.c', 'sim/render/w2n_polygon.c', '../tools/verify/w3p_test.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STUB0 = 0x00F00A00
STUB_NAMES = ['dp', 'count', 'fn5', 'fn3', 'v12c', 'v14', 'v10', 'cmp', 'drv']
STUB = {n: STUB0 + 16 * i for i, n in enumerate(STUB_NAMES)}
STUB_BY_ADDR = {a: n for n, a in STUB.items()}
TRAMP = 0x00AF0000
LOGCAP = 0x20000
STACK_TOP = 0x00800000
ST = {}


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))

def w16(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<H', v & 0xffff))

def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))

def wf(ctx, addr, f):
    ctx.m.uc.mem_write(addr, struct.pack('<f', f))

def r32(uc, a):
    return struct.unpack('<I', bytes(uc.mem_read(a, 4)))[0]

def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]

def bitsf(b):
    return struct.unpack('<f', struct.pack('<I', b & 0xffffffff))[0]

def read_or(uc, addr, n):
    try:
        return bytes(uc.mem_read(addr, n))
    except Exception:
        return None


def rfloat(r):
    """A float32 word: ordinary values, zeros, huge, tiny, NaN (quiet and signalling), infinities, random bits."""
    c = r.random()
    if c < 0.45: return fbits(r.uniform(-100, 100))
    if c < 0.55: return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 2.0, 100.0, 1e-3]))
    if c < 0.62: return fbits(r.uniform(-1e6, 1e6))
    if c < 0.68: return r.choice([0x7fc00000, 0xffc00000, 0x7f800001, 0xff800123, 0x7fa00000])
    if c < 0.73: return r.choice([0x7f800000, 0xff800000])
    if c < 0.78: return r.choice([0x00000001, 0x007fffff, 0x80000001, 0x00800000])
    if c < 0.82: return r.choice([0x7f7fffff, 0xff7fffff, 0x7e000000])
    return r.getrandbits(32)


def rfloat_n(r):
    """Ordinary floats only (no NaN or infinity)."""
    c = r.random()
    if c < 0.7: return fbits(r.uniform(-100, 100))
    if c < 0.85: return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, 2.0, 1e-3, 50.0]))
    return fbits(r.uniform(-1e5, 1e5))


# ---- log and stubs ----------------------------------------------------------
def emit(uc, items):
    log = ST['log']
    pos = r32(uc, log)
    out = b''.join(struct.pack('<I', v & 0xffffffff) for v in items)
    if pos + len(out) > LOGCAP:
        uc.mem_write(log + 4 + LOGCAP, struct.pack('<I', 0xdead))
        return
    uc.mem_write(log + 4 + pos, out)
    uc.mem_write(log, struct.pack('<I', pos + len(out)))


def sgn(v):
    return v - (1 << 32) if v & 0x80000000 else v


def _hook(m):
    uc = m.uc

    def cb(uc, address, size, ud):
        try:
            name = STUB_BY_ADDR.get(address)
            if name is None:
                return
            esp = uc.reg_read(UC_X86_REG_ESP)
            a = struct.unpack('<6I', bytes(uc.mem_read(esp + 4, 24)))
            ret = ST['ret'].get(name, 0)
            idx = STUB_NAMES.index(name) + 1
            if name == 'dp':
                n = a[5] if 0 < a[5] <= 0x40 else 0x10
                d = read_or(uc, a[4], n)
                emit(uc, [idx] + list(a) + [0xBADBAD if d is None else len(d)])
                if d:
                    emit(uc, [int.from_bytes(d[i:i + 4].ljust(4, b'\0'), 'little') for i in range(0, len(d), 4)])
            elif name == 'count':
                emit(uc, [idx, a[0]])
            elif name == 'fn5':
                emit(uc, [idx] + list(a[:5]))
                if ST.get('clobber') is not None:
                    uc.mem_write(0x1015ecd8, struct.pack('<H', ST['clobber']))
            elif name == 'fn3':
                emit(uc, [idx] + list(a[:3]))
            elif name == 'v12c':
                emit(uc, [idx] + list(a[:2]))
            elif name == 'v14':
                emit(uc, [idx])
            elif name == 'v10':
                emit(uc, [idx] + list(a[:3]))
            elif name == 'cmp':
                emit(uc, [idx, a[0], a[1]])
                if ST['cmphash']:
                    h = ((a[0] * 2654435761) ^ (a[1] * 40503) ^ ST['seed']) & 0xffffffff
                    ret = (0, 1, 0xffffffff)[(h >> 9) % 3]
                else:
                    x, y = sgn(a[0]), sgn(a[1])
                    ret = 0xffffffff if x < y else (0 if x == y else 1)
            elif name == 'drv':
                n = a[0]
                emit(uc, [idx, a[0], a[2]])
                d = read_or(uc, a[1], max(0, min(n, 300)) * 36)
                emit(uc, [0xBADBAD] if d is None else list(struct.unpack('<%dI' % (len(d) // 4), d)))
            uc.reg_write(UC_X86_REG_EAX, ret & 0xffffffff)
        except Exception as e:
            ST['err'] = repr(e)
            raise
    uc.hook_add(UC_HOOK_CODE, cb, None, STUB0, STUB0 + 0x100)
    uc.mem_write(STUB0, b'\xc3' * 0xe0)


def begin(ctx):
    """Stubs (once), fresh log, FPU state, the plugin/driver function pointers, a uniform stack fill."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w3p', False):
        _hook(m)
        m._w3p = True
    uc = m.uc
    uc.mem_write(STUB0 + 0xe0, b'\xdb\xe3\xf4')            # fninit
    uc.emu_start(STUB0 + 0xe0, STUB0 + 0xe2)
    uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    ST['log'] = ctx.buf(4 + LOGCAP + 8, 'zero')
    ST['ret'] = {s: r.choice([0, 1, 2, 5, 0xffffffff, r.getrandbits(32)]) for s in STUB_NAMES}
    ST['ret']['count'] = r.choice([0, 1, 2, 3, 5, 8, 9])
    ST['seed'] = r.getrandbits(32)
    ST['cmphash'] = r.random() < 0.3
    ST['clobber'] = r.choice([None, None, r.getrandbits(16)])
    w32(ctx, 0x1015eab4, STUB['dp'])
    w32(ctx, 0x1015ead8, STUB['count'])
    w32(ctx, 0x1015eae0, STUB['fn5'])
    w32(ctx, 0x1015eb20, STUB['fn3'])
    w32(ctx, 0x1014cb4c, STUB['drv'])
    pat = r.choice([0, 0, 0x55, 0xA5, 0x3F, 0x7B])
    uc.mem_write(STACK_TOP - 0x20000, bytes([pat]) * (0x20000 - 0x10))


# ---- network environment ----------------------------------------------------
MSG_VARS = [0x1015ec90, 0x1015ec9c, 0x1015eca4, 0x1015ecbc, 0x1015eca8, 0x1015ecc0, 0x1015ecac, 0x1015ecb4,
            0x1015ecb8, 0x1015ec98, 0x1015d084, 0x1015ec94, 0x1015ecc4, 0x1015ecc8, 0x1015ecb0]
NPL = 10


def net_env(ctx, nplayers=None):
    """Message buffers, session, word globals, the pointer table with player records, the list record."""
    r = ctx.rng
    begin(ctx)
    for v in MSG_VARS:
        w32(ctx, v, ctx.buf(0x100))
    w32(ctx, 0x1015ecd4, r.choice([0, 1, 0x1234, r.getrandbits(32)]))
    w16(ctx, 0x1013acac, r.choice([0, 1, 7, r.getrandbits(16)]))
    w16(ctx, 0x1013acb0, r.getrandbits(16))
    w16(ctx, 0x1015ecd8, r.choice([0, 1, 7, r.getrandbits(16)]))
    w16(ctx, 0x1015ec7c, r.getrandbits(16))
    self_i = r.randrange(0, NPL)
    w32(ctx, 0x1017b8c8, self_i)
    w32(ctx, 0x1015dc64, r.choice([NPL, NPL, 8, 4, r.randrange(0, 12)]))
    w32(ctx, 0x1015ec84, r.choice([1, 1, 1, 0]))
    for a in (0x1015ecd0, 0x1015eccc, 0x1015ec74, 0x1015ec58, 0x1015ece8, 0x1015dc84, 0x1017b94c, 0x1015ec68, 0x1015d08c, 0x101655ac):
        w32(ctx, a, r.choice([0, 1, 2, r.getrandbits(32)]))
    w32(ctx, 0x1015ecd0, r.choice([0, 1, 1, 2]))
    w32(ctx, 0x1013bb40, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b8c4, ctx.buf(0x20, bytes([r.getrandbits(8)] * 10 + [r.choice([0, 0, 1])] + [0] * 21)))
    wb(ctx, 0x1013ac00, bytes(r.getrandbits(8) for _ in range(0x300)))   # the message headers and word globals
    w16(ctx, 0x1013acac, r.choice([0, 1, 7, r.getrandbits(16)]))
    w16(ctx, 0x1013acb0, r.getrandbits(16))
    # block 17 entries
    ents = []
    recs = []
    for i in range(NPL):
        block = ctx.buf(0xd00)
        arr = ctx.buf(17 * 0x3c + 8)
        for k in range(17):
            o = arr + k * 0x3c
            wf(ctx, o, bitsf(rfloat_n(r)))
            wf(ctx, o + 8, bitsf(rfloat_n(r)))
            w32(ctx, o + 0x38, r.choice([0, 1, 2, 3, 7, 100, 0xffffffff, r.getrandbits(8)]))
        w32(ctx, block + 0x88, arr)
        for k in range(8):
            w32(ctx, block + 0xbf5 + 4 * k, r.choice([0, 0, 1, r.getrandbits(32)]))
        w32(ctx, block + 0x4fc, r.choice([0xffffffff, 0, 1, 2, 3]))
        w32(ctx, block + 0x84, ctx.buf(8 * 0x128 + 8))
        for off in (0x4ec, 0x554, 0x558, 0x55c):
            wf(ctx, block + off, bitsf(rfloat_n(r)))
        rec = ctx.buf(0x600)
        w32(ctx, rec + 4, i)
        w32(ctx, rec + 8, r.choice([0, 1, 2, 3]))
        w32(ctx, rec + 0xc, r.choice([self_i, self_i, self_i, r.randrange(0, NPL), 0xffffffff, r.randrange(0, 20)]))
        w32(ctx, rec + 0x14, r.choice([1, 1, 1, 3, 3, 3, 0, 2]))
        w32(ctx, rec + 0x28, block)
        w32(ctx, rec + 0x170, ctx.buf(0x80, 'zero'))      # for Sim_100119a7 (called by Sim_10040721): empty sets
        for k in range(3):
            w32(ctx, rec + 0x174 + 4 * k, 0)
        w32(ctx, rec + 0x150, r.getrandbits(32))
        recs.append(rec)
    table = [recs[i] if r.random() > 0.04 else 0 for i in range(NPL)]
    if r.random() > 0.5:
        table[self_i] = recs[self_i]
    wb(ctx, 0x109e8b40, struct.pack('<%dI' % NPL, *table))
    # relation table (dword per record kind: 0x6c bytes apart)
    wb(ctx, 0x109b7d40, b''.join(struct.pack('<I', r.choice([0, 0, 1, 1, 2, 5])) + bytes(0x68) for _ in range(6)))
    # list record
    n = r.choice([0, 1, 2, 3, 4, 8, r.randrange(0, 9)])
    root = ctx.buf(0x400)
    keys = ctx.buf(40)
    lrecs = ctx.buf(8 * 0x1e + 16)
    for i in range(8):
        w16(ctx, keys + 2 * i, r.getrandbits(16) if r.random() < 0.5 else r.randrange(0, 4))
        w16(ctx, lrecs + i * 0x1e + 0x1a, r.randrange(0, NPL) if r.random() < 0.9 else r.getrandbits(3))
    w32(ctx, root + 0x12, n)
    w32(ctx, root + 0x16, keys)
    w32(ctx, root + 0x126, lrecs)
    wb(ctx, root + 0x137, bytes([r.getrandbits(8), r.getrandbits(8)]))
    w32(ctx, 0x1015d004, root)
    # ratings 8x8 words (row stride 16) and result words, marks
    wb(ctx, 0x1016552c, bytes(r.getrandbits(8) for _ in range(0x80)))
    wb(ctx, 0x1015d0f0, bytes(r.getrandbits(8) for _ in range(16)))
    wb(ctx, 0x1015d090, bytes(r.choice([0, 1, 1, 0xff]) for _ in range(16)))
    wb(ctx, 0x109e9ba0, bytes(r.getrandbits(8) for _ in range(0x50 * 254 + 4)))
    wb(ctx, 0x1015da45, bytes(r.getrandbits(8) for _ in range(0x16 * NPL + 20)))
    return self_i


def small_text(ctx, maxlen=None):
    r = ctx.rng
    n = r.choice([0, 1, 5, 20, 0x4e, 0x4f, 0x50, 0x51, r.randrange(0, 0x4f), r.randrange(0, 0x4f), r.randrange(0, 0x4f)])
    b = bytearray(0x70)
    for i in range(n):
        b[i] = r.choice(b'abcXYZ019 _.')
    b[n] = 0
    return ctx.buf(0x70, bytes(b))


# ---- generators: network ---------------------------------------------------
def g_7e920(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1015ecd4, r.choice([0, 1, r.getrandbits(32)]))
    w16(ctx, 0x1013acac, r.getrandbits(16))
    buf = ctx.buf(0x80)
    return [buf, r.choice([0, 1, 5, 0x40, 0x12340010, 0xffff0003, r.getrandbits(32), r.randrange(0, 0x80)]), r.choice([0, 1, r.getrandbits(32)])]


def g_7e950(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1015ecd4, r.choice([0, 1, r.getrandbits(32)]))
    w16(ctx, 0x1013acac, r.getrandbits(16))
    buf = ctx.buf(0x80)
    return [r.choice([0, 3, 0xffff, 0x12345, r.getrandbits(32)]), buf, r.choice([0, 1, 5, 0x40, 0x12340010, r.getrandbits(32), r.randrange(0, 0x80)]), r.choice([0, 1, r.getrandbits(32)])]


def g_3fcdc(ctx):
    r = ctx.rng
    net_env(ctx)
    return [r.choice([-4, -3, -2, -1, -1, 0, 1, 5, r.randrange(0, NPL), r.randrange(0, NPL), -5, 100]), small_text(ctx)]


def force_rec(ctx, i, state=3, owner_self=True):
    """Make table entry i a valid record in the given state (and owned by self)."""
    uc = ctx.m.uc
    rec = r32(uc, 0x109e8b40 + 4 * i)
    if rec == 0:
        return
    w32(ctx, rec + 0x14, state)
    if owner_self:
        w32(ctx, rec + 0xc, r32(uc, 0x1017b8c8))


def g_idx(ctx):
    r = ctx.rng
    net_env(ctx)
    i = r.randrange(0, NPL)
    if r.random() < 0.6:
        force_rec(ctx, i, 3, r.random() < 0.8)
        return [i]
    return [r.choice([r.randrange(0, NPL), r.randrange(0, NPL), r.randrange(0, 12), -1, 0x100])]


def g_3ffe9(ctx):
    r = ctx.rng
    net_env(ctx)
    return [r.randrange(0, NPL) if r.random() < 0.9 else r.randrange(0, 12)]


def g_msg(ctx):
    r = ctx.rng
    net_env(ctx)
    msg = ctx.buf(0x40)
    i = r.randrange(0, NPL)
    if r.random() < 0.7:
        force_rec(ctx, i, 3, False)
    wb(ctx, msg + 2, bytes([i if r.random() < 0.9 else r.getrandbits(8)]))
    wb(ctx, msg + 4, bytes([ST_self(ctx) & 0xff if r.random() < 0.8 else r.getrandbits(8)]))
    return [msg]


def g_409be(ctx):
    r = ctx.rng
    net_env(ctx)
    words = ctx.buf(8)
    return [r.getrandbits(8) if r.random() < 0.5 else r.getrandbits(32), words if r.random() < 0.7 else 0]


def g_byte(ctx):
    net_env(ctx)
    return [ctx.rng.choice([0, 1, 0x7f, 0x80, 0xff, ctx.rng.getrandbits(32)])]


def g_40f5f(ctx):
    r = ctx.rng
    net_env(ctx)
    a = ctx.buf(16)
    j = r.randrange(0, NPL)
    if r.random() < 0.5:
        force_rec(ctx, j, 3, False)
        w32(ctx, r32(ctx.m.uc, 0x109e8b40 + 4 * j) + 0xc, r.randrange(0, NPL)) if r32(ctx.m.uc, 0x109e8b40 + 4 * j) else None
    w = r.choice([0x4200, 0x4200, 0x4200, j, j, j, r.randrange(0, NPL), r.randrange(0, NPL), r.randrange(0, NPL), 0x10000 | 0x4200, r.getrandbits(16), r.getrandbits(32), 0x1100 | r.randrange(0, NPL)])
    return [a, w, r.getrandbits(32), r.getrandbits(32)]


def g_dword(ctx):
    net_env(ctx)
    return [ctx.rng.getrandbits(8) if ctx.rng.random() < 0.5 else ctx.rng.getrandbits(32), ctx.u32()]


def g_dword2(ctx):
    net_env(ctx)
    return [ctx.u32(), ctx.u32(), ctx.u32()]


def g_none_net(ctx):
    net_env(ctx)
    return []


def g_1558c(ctx):
    r = ctx.rng
    net_env(ctx)
    msg = ctx.buf(0x40)
    return [msg, r.choice([0, 1, 7, r.randrange(0, 8), r.randrange(0, 8), r.randrange(-3, 12)])]


def g_415eb(ctx):
    r = ctx.rng
    net_env(ctx)
    return [r.randrange(0, NPL) if r.random() < 0.8 else r.getrandbits(8), rfloat(r), rfloat(r), rfloat(r)]


def g_418e3(ctx):
    r = ctx.rng
    net_env(ctx)
    w16(ctx, 0x1015ecd8, r.choice([0, 1, 2, 3, 4, 5, 6, 7]))
    return [r.getrandbits(32), r.choice([r.randrange(0, NPL), r.randrange(0, NPL), r.randrange(-2, 20)])]


def g_421b0(ctx):
    net_env(ctx)
    return [ctx.rng.choice([0, 1, 0xffff, ctx.rng.getrandbits(32)])]


# ---- other small functions -------------------------------------------------
def g_2c6e2(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1017b8c8, r.randrange(0, 4))
    rec = ctx.buf(16)
    w32(ctx, rec + 4, r.choice([r.randrange(0, 4), ctx.u32()]) if r.random() > 0.55 else ST_self(ctx))
    holder = ctx.buf(8)
    w32(ctx, holder, rec)
    objs = []
    for i in range(0x1e):
        o = ctx.buf(0x140)
        w16(ctx, o + 6, r.choice([0, 1, 5, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]))
        w32(ctx, o + 0x12c, STUB['v12c'])
        objs.append(o)
    wb(ctx, 0x109e88e0, struct.pack('<30I', *objs))
    w32(ctx, 0x1017cd38, r.randrange(0, 0x7f))                  # the ring index of Sim_1009c929
    wb(ctx, 0x101cffc0, bytes(r.getrandbits(8) for _ in range(0x7f * 4 + 8)))
    return [holder, r.choice([0, 1, 2, r.getrandbits(32)])]


def ST_self(ctx):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(0x1017b8c8, 4)))[0]


def g_1b01f(ctx):
    r = ctx.rng
    begin(ctx)
    def f():
        return rfloat(r) if r.random() < 0.25 else fbits(r.choice([r.uniform(-5, 5), r.uniform(0, 5), 0.0, -0.0, 1.0]))
    return [f(), f(), f(), f()]


def g_noargs(ctx):
    begin(ctx)
    return []


def g_e6c5(ctx):
    r = ctx.rng
    begin(ctx)
    n = r.randrange(0, 12)
    t = ctx.buf(0x300)
    w32(ctx, t + 4, n)
    w32(ctx, 0x1015d0a4, t if r.random() < 0.9 else 0)
    w32(ctx, 0x1015dc78, ctx.u32())
    return []


def g_ccc4(ctx):
    r = ctx.rng
    net_env(ctx)
    w32(ctx, 0x1015dc64, r.choice([NPL, 4, 0, 1]))
    return []


def g_47aa0(ctx):
    r = ctx.rng
    begin(ctx)
    p = ctx.buf(16)
    w32(ctx, 0x1015dc78, ctx.u32())
    c = r.random()
    if c < 0.2:
        d = r.choice([0.0, -0.0, 1.0 / 181, 0.5 / 181, -1.0, 0.99 / 181, 1e300, -1e300, float('inf'), float('nan'), 1e-300, 1e10])
    elif c < 0.6:
        d = r.uniform(-3, 3)
    else:
        d = r.uniform(-1e8, 1e8)
    return [p, ctx.u32(), ctx.u32()] + list(struct.unpack('<2I', struct.pack('<d', d)))


def ratio_pair(r):
    c = r.random()
    b = r.choice([1, 2, 3, 4, 5, 8, 10, 100, 0, -1, -7, r.randrange(1, 1000), r.getrandbits(31), 0x7fffffff, 0x80000000])
    if c < 0.5:
        a = int(b * r.choice([0.4, 0.74, 0.75, 0.76, 0.89, 0.9, 0.91, 1.0, 1.04, 1.05, 1.06, 1.09, 1.1, 1.11, 1.14, 1.15, 1.16, 1.19, 1.2, 1.21, 1.25, 1.26, 1.27, 2.0, -1.0])) if b < 0x40000000 else r.getrandbits(31)
    elif c < 0.8:
        a = r.randrange(0, 2000)
    else:
        a = r.choice([0, 1, -1, 0x7fffffff, 0x80000000, r.getrandbits(32)])
    return a & 0xffffffff, b & 0xffffffff


def g_4bce5(ctx):
    r = ctx.rng
    begin(ctx)
    p = ctx.buf(8)
    w32(ctx, p, r.choice([0, 1, 5, -3, 0x7fffffff, 0x80000000, r.getrandbits(32)]))
    a, b = ratio_pair(r)
    c, d = ratio_pair(r)
    return [p, a, b, c, d]


def g_4be11(ctx):
    begin(ctx)
    a, b = ratio_pair(ctx.rng)
    return [a, b]


def g_5874a(ctx):
    r = ctx.rng
    begin(ctx)
    m = ctx.buf(0x40)
    for i in range(12):
        wf(ctx, m + 4 * i, bitsf(rfloat_n(r)) if r.random() < 0.9 else bitsf(rfloat(r)))
    for a in (0x109b16c4, 0x109b16c8, 0x109b16c0):
        w32(ctx, a, rfloat_n(r) if r.random() < 0.9 else rfloat(r))
    w32(ctx, 0x109afdcc, r.choice([0, 0, 1]))
    nodes = [ctx.buf(0x40) for _ in range(r.choice([0, 1, 2, 5]))]
    for i, nd in enumerate(nodes):
        w32(ctx, nd, nodes[i + 1] if i + 1 < len(nodes) else 0)
        for off in (0x14, 0x18, 0x1c):
            w32(ctx, nd + off, rfloat_n(r))
    w32(ctx, 0x101668d0, nodes[0] if nodes else 0)
    return [m]


def vis_env(ctx):
    r = ctx.rng
    begin(ctx)
    for a in range(0x10185c20, 0x10185c44, 4):
        w32(ctx, a, fbits(r.uniform(-1, 1)))
    for a in (0x10185c48, 0x10185c44, 0x10185c4c):
        w32(ctx, a, fbits(r.choice([0.0, r.uniform(-50, 50)])))
    w32(ctx, 0x10185c50, fbits(r.choice([0.0, 1.0, 4.0, 10.0, -1.0])))
    w32(ctx, 0x10185bf8, fbits(r.choice([100.0, 1000.0, 50.0, 0.0, 5000.0])))
    for a in (0x1036f1c0, 0x1036f1c4, 0x1036f1c8, 0x1036f1cc):
        w32(ctx, a, fbits(r.choice([0.0, 0.0, 0.5, -0.5, 2.0, r.uniform(-3, 3)])))


def vis_obj(ctx):
    r = ctx.rng
    o = ctx.buf(0x80)
    w16(ctx, o, r.choice([0, 0, 0, 0x1000, 0x1001, r.getrandbits(16)]))
    for off in (0x44, 0x48, 0x4c):
        w32(ctx, o + off, fbits(r.choice([r.uniform(-300, 300), r.uniform(-2000, 2000), r.uniform(-20, 20), 0.0])) if r.random() < 0.95 else rfloat(r))
    w32(ctx, o + 0x50, fbits(r.choice([r.uniform(0, 20), r.uniform(0, 300), 0.0, 1.0, -2.0])) if r.random() < 0.95 else rfloat(r))
    return o


def g_8fdd7(ctx):
    vis_env(ctx)
    return [vis_obj(ctx)]


def g_9b47d(ctx):
    r = ctx.rng
    vis_env(ctx)
    o = vis_obj(ctx)
    w16(ctx, o + 2, r.choice([0x100, 0x100, 0x130, 0x170, 0x30, 0x70, 0x200, 0x010, r.getrandbits(16)]))
    w16(ctx, o + 0x24, r.randrange(0, 4))
    recs = [ctx.buf(0x40) for _ in range(4)]
    for rr in recs:
        w16(ctx, rr + 0x1c, r.choice([0, 0x16, 0x1, 0x2, 0x4, 0x10, 0x8, r.getrandbits(16), 0x8002]))
    wb(ctx, 0x109e8b40, struct.pack('<4I', *recs))
    return [o]


def g_box(ctx):
    r = ctx.rng
    begin(ctx)
    return [ctx.buf(8)] + [rfloat_n(r) if r.random() < 0.85 else rfloat(r) for _ in range(6)]


def boxfl(ctx):
    r = ctx.rng
    vals = []
    for _ in range(3):
        lo = r.uniform(-10, 10)
        hi = lo + r.choice([0.0, r.uniform(0, 20)])
        vals += [fbits(lo), fbits(hi)]
    if r.random() < 0.1:
        vals = [rfloat(r) for _ in range(6)]
    return ctx.buf(24, struct.pack('<6I', *vals))


def pointf(r):
    return fbits(r.uniform(-15, 15)) if r.random() < 0.85 else rfloat(r)


def g_clamp(ctx):
    r = ctx.rng
    begin(ctx)
    a = pointf(r)
    lo = fbits(r.uniform(-10, 10)) if r.random() < 0.9 else rfloat(r)
    hi = fbits(bitsf(lo) + r.uniform(0, 10)) if (r.random() < 0.9 and bitsf(lo) == bitsf(lo)) else rfloat(r)
    gen_tramp(ctx, 0x100686ea, 3, 'f')
    return [a, lo, hi]


def g_68716(ctx):
    r = ctx.rng
    begin(ctx)
    outs = ctx.buf(16, 'zero')
    return [boxfl(ctx) if r.random() > 0.05 else 0, pointf(r), pointf(r), pointf(r),
            outs if r.random() > 0.05 else 0, outs + 4 if r.random() > 0.05 else 0, outs + 8 if r.random() > 0.05 else 0]


def g_68b10(ctx):
    r = ctx.rng
    begin(ctx)
    box = boxfl(ctx)
    vals = struct.unpack('<6f', bytes(ctx.m.uc.mem_read(box, 24)))
    def p(i):
        c = r.random()
        if c < 0.4 and vals[i] == vals[i]:
            return fbits(r.uniform(vals[i], vals[i + 1] if vals[i + 1] >= vals[i] else vals[i]))
        if c < 0.55:
            return fbits(vals[i])
        return pointf(r)
    return [box if r.random() > 0.05 else 0, p(0), p(2), p(4)]


def g_b3fd5(ctx):
    r = ctx.rng
    begin(ctx)
    return [r.choice([0, 1, -1, 0x7fffffff, 0x80000000, 16777217, r.randrange(-1000, 1000), r.getrandbits(32)]) for _ in range(9)]


def g_e2756(ctx):
    r = ctx.rng
    begin(ctx)
    return [r.choice([0, 1, -1, 0x7fffffff, 0x80000000, 16777217, r.randrange(-1000, 1000), r.getrandbits(32)]) for _ in range(3)]


def g_bdc5d(ctx):
    r = ctx.rng
    begin(ctx)
    cap = r.choice([1, 2, 5, 12, 40, 0, r.randrange(0, 20)])
    n = r.choice([0, 0, 1, 3, cap, r.randrange(0, 15)])
    n = min(n, 40)
    items = ctx.buf(4 * 64)
    vals = sorted(r.randrange(-50, 50) for _ in range(n)) if r.random() < 0.8 else [r.getrandbits(32) for _ in range(n)]
    wb(ctx, items, struct.pack('<%dI' % n, *[v & 0xffffffff for v in vals]) if n else b'')
    lst = ctx.buf(16, struct.pack('<3I', cap & 0xffffffff, n, items))
    key = r.choice([r.randrange(-60, 60), r.getrandbits(32)])
    return [lst, key & 0xffffffff, STUB['cmp']]


def g_c05cb(ctx):
    r = ctx.rng
    begin(ctx)
    gen_tramp(ctx, 0x100c05cb, 1, 'f')
    if r.random() < 0.1:
        return [0]
    block = ctx.buf(0x600)
    w32(ctx, block + 0x4fc, r.choice([0xffffffff, 0, 1, 2, 3]))
    w32(ctx, block + 0x84, ctx.buf(4 * 0x128 + 8))
    obj = ctx.buf(0x40, bytes(r.getrandbits(8) for _ in range(0x28)) + struct.pack('<I', block) + bytes(0x14))
    for k in range(4):
        w32(ctx, block + 0x84 and 0, 0) if False else None
    base = struct.unpack('<I', bytes(ctx.m.uc.mem_read(block + 0x84, 4)))[0]
    for k in range(4):
        w32(ctx, base + k * 0x128 + 4, r.choice([0, 1, 2, 3, 0x10, 0x100000, r.getrandbits(32)]) if False else r.choice([0, 1, 2, 3, 100]))
    wb(ctx, 0x10145508, bytes(r.getrandbits(8) for _ in range(0xd6 * 4 + 0x60)))
    for k in range(101):
        pass
    for k in (0, 1, 2, 3, 100):
        for off in (0x4c, 0x50):
            w32(ctx, 0x10145508 + k * 0xd6 + off, rfloat_n(r) if r.random() < 0.9 else rfloat(r))
    return [obj]


def g_c351c(ctx):
    r = ctx.rng
    begin(ctx)
    gen_tramp(ctx, 0x100c351c, 2, 'f')
    x = pointf(r)
    return [x, fbits(r.uniform(-8, 8)) if r.random() < 0.85 else rfloat(r)]


def g_db3f7(ctx):
    r = ctx.rng
    begin(ctx)
    outs = ctx.buf(16, 'zero')
    def v():
        c = r.random()
        if c < 0.5: return fbits(r.uniform(-1000, 1000))
        if c < 0.7: return fbits(r.uniform(-1e-3, 1e-3))
        if c < 0.8: return fbits(r.choice([0.0, -0.0, 1e-9]))
        if c < 0.9: return fbits(r.uniform(-1e12, 1e12))
        return rfloat(r)
    w32(ctx, 0x1012bd8c, 0) if False else None
    return [v(), v(), v(), outs, outs + 4, outs + 8]


def g_db5c6(ctx):
    r = ctx.rng
    begin(ctx)
    outs = ctx.buf(16, 'zero')
    def v():
        return fbits(r.uniform(-100, 100)) if r.random() < 0.9 else rfloat(r)
    return [v() for _ in range(9)] + [outs, outs + 4, outs + 8]


# ---- fog -------------------------------------------------------------------
def g_fog(ctx):
    r = ctx.rng
    begin(ctx)
    def g(vals):
        return fbits(r.choice(vals)) if r.random() < 0.85 else rfloat(r)
    w32(ctx, 0x10143fc8, g([0.0, 1.0, 50.0, 100.0, 1e-3, -5.0]))
    w32(ctx, 0x10143fc4, g([0.0, 1.0, 0.5, 400.0, -1.0, 1e5]))
    w32(ctx, 0x10185bf8, g([0.0, 100.0, 1000.0, 5000.0, 20.0, 50.0]))
    w32(ctx, 0x101668cc, g([0.0, 0.25, 0.5, 1.0, 2.0, 0.75]))
    gen_tramp(ctx, 0x10058ac2, 2, 'f')
    return [fbits(r.choice([0.0, 1.0, 5.0, 50.0, 100.0, 400.0, 1000.0, 20.0, -3.0, r.uniform(-10, 6000), r.uniform(0, 200)])) if r.random() < 0.9 else rfloat(r),
            fbits(r.choice([0.0, 1.0, -1.0, 0.5, 3.0, 400.0, r.uniform(-10, 100)])) if r.random() < 0.9 else rfloat(r)]


# ---- square on the ground (100ad5b7), env copied from spec_w2n.py -----------------------
def proj_globals(ctx):
    r = ctx.rng
    wide = r.random() < 0.7
    w32(ctx, 0x10185bd4, r.choice([320, 160, 0, r.randrange(-100, 700)]))
    w32(ctx, 0x10185bf4, r.choice([240, 100, 0, r.randrange(-100, 500)]))
    w32(ctx, 0x10185bd0, 0 if wide else r.randrange(-50, 300))
    w32(ctx, 0x10185bec, 639 if wide else r.randrange(100, 700))
    w32(ctx, 0x10185bf0, 0 if wide else r.randrange(-50, 200))
    w32(ctx, 0x10185be0, 479 if wide else r.randrange(100, 600))
    w32(ctx, 0x101c7680, r.getrandbits(8))
    w32(ctx, 0x101cc2a0, r.getrandbits(8))
    w32(ctx, 0x101c673c, r.choice([0, 5, 0xffffffff, r.getrandbits(32)]))


def matrix(ctx):
    r = ctx.rng
    c = r.random()
    if c < 0.5:
        m = [1, 0, 0, 0, 1, 0, 0, 0, 1]
    elif c < 0.75:
        a = r.uniform(-3, 3)
        m = [math.cos(a), 0, -math.sin(a), 0, 1, 0, math.sin(a), 0, math.cos(a)]
    else:
        m = [r.uniform(-2, 2) for _ in range(9)]
    wb(ctx, 0x10185ba0, struct.pack('<9f', *m))
    pos = [r.choice([0.0, 0.0, r.uniform(-50, 50)]) for _ in range(3)]
    wf(ctx, 0x10185c48, pos[0])
    wf(ctx, 0x10185c44, pos[1])
    wf(ctx, 0x10185c4c, pos[2])


def g_ad5b7(ctx):
    r = ctx.rng
    begin(ctx)
    proj_globals(ctx)
    cap = r.choice([4, 12, 40, 600, 600])
    buf = ctx.buf(0x40 * 620)
    top = buf + 0x40 * 620
    w32(ctx, 0x1017d174, top)
    w32(ctx, 0x1017d178, top - 0x40 * cap - 0xc8)
    matrix(ctx)
    w32(ctx, 0x101c9ea4, 1 if r.random() < 0.93 else 0)
    w32(ctx, 0x101c9ea8, r.choice([0, 0, 7, 300]))
    wf(ctx, 0x10185c50, r.uniform(-5, 5))
    cam = ctx.buf(0x100)
    wf(ctx, cam + 0x3c, r.choice([0.5, 1.0, 4.0, 10.0, 0.0, -1.0]))
    wf(ctx, cam + 0x40, r.choice([0.0, 100.0, 1000.0, 5000.0, 1e9, -5.0]))
    wf(ctx, cam + 0x94, r.choice([1.0, 320.0, 0.5, 1000.0, -2.0]))
    w32(ctx, 0x1014cae0, cam)
    w32(ctx, 0x10143fc8, fbits(r.choice([0.0, 1.0, 50.0, 100.0])))
    w32(ctx, 0x10143fc4, fbits(r.choice([0.0, 1.0, 0.5, 400.0, -1.0])))
    w32(ctx, 0x10185bf8, fbits(r.choice([0.0, 100.0, 1000.0, 5000.0, 20.0])))
    w32(ctx, 0x101668cc, fbits(r.choice([0.0, 0.25, 0.5, 1.0, 2.0])))
    w32(ctx, 0x10143fcc, fbits(r.choice([255.0, 128.0, 1.0, 0.5])))
    w32(ctx, 0x1017b940, r.choice([0, 1, 1]))
    return [fbits(r.uniform(-60, 60)), fbits(r.choice([0.0, 1.0, 20.0, 400.0, r.uniform(-5, 600)])), fbits(r.uniform(-20, 100)),
            fbits(r.choice([1.0, 5.0, 20.0, 60.0, 0.0, -3.0, r.uniform(0, 100)]))]


# ---- trampolines for the functions that return a float in st(0) -----------------
def gen_tramp(ctx, orig, nargs, kind):
    code = b''
    for _ in range(nargs):
        code += bytes([0xFF, 0x74, 0x24, 4 * nargs])
    code += bytes([0xE8]) + struct.pack('<i', orig - (TRAMP + len(code) + 5))
    code += bytes([0x83, 0xC4, 4 * nargs, 0x83, 0xEC, 0x04, 0xD9, 0x1C, 0x24, 0x58, 0xC3])
    ctx.m.uc.mem_write(TRAMP, code)


SPECS = [
    dict(name='Net_DpSend_1007e920', orig=0x1007e920, c='Net_DpSend_1007e920', gen=g_7e920, trials=400),
    dict(name='Net_DpSend_1007e950', orig=0x1007e950, c='Net_DpSend_1007e950', gen=g_7e950, trials=400),
    dict(name='Sim_1003fcdc', orig=0x1003fcdc, c='Sim_1003fcdc', gen=g_3fcdc, trials=800),
    dict(name='Sim_1003ffe9', orig=0x1003ffe9, c='Sim_1003ffe9', gen=g_3ffe9, trials=800, void=True),
    dict(name='Sim_10040376', orig=0x10040376, c='Sim_10040376', gen=g_idx, trials=500, void=True),
    dict(name='Sim_10040564', orig=0x10040564, c='Sim_10040564', gen=g_idx, trials=500, void=True),
    dict(name='Sim_1004084a', orig=0x1004084a, c='Sim_1004084a', gen=g_msg, trials=500, void=True),
    dict(name='Sim_100408bd', orig=0x100408bd, c='Sim_100408bd', gen=g_idx, trials=500, void=True),
    dict(name='Sim_100409be', orig=0x100409be, c='Sim_100409be', gen=g_409be, trials=400),
    dict(name='Sim_10040ab8', orig=0x10040ab8, c='Sim_10040ab8', gen=g_byte, trials=400),
    dict(name='Sim_10040eb3', orig=0x10040eb3, c='Sim_10040eb3', gen=g_none_net, trials=400),
    dict(name='Sim_10040f5f', orig=0x10040f5f, c='Sim_10040f5f', gen=g_40f5f, trials=800, void=True),
    dict(name='Sim_10041203', orig=0x10041203, c='Sim_10041203', gen=g_dword, trials=400),
    dict(name='Sim_100412cb', orig=0x100412cb, c='Sim_100412cb', gen=g_dword2, trials=400),
    dict(name='Sim_100413c7', orig=0x100413c7, c='Sim_100413c7', gen=g_none_net, trials=600, void=True),
    dict(name='Sim_10041444', orig=0x10041444, c='Sim_10041444', gen=g_none_net, trials=600, void=True),
    dict(name='Sim_1004158c', orig=0x1004158c, c='Sim_1004158c', gen=g_1558c, trials=500, void=True),
    dict(name='Sim_100415eb', orig=0x100415eb, c='Sim_100415eb', gen=g_415eb, trials=600),
    dict(name='Sim_1004185b', orig=0x1004185b, c='Sim_1004185b', gen=g_none_net, trials=300),
    dict(name='Sim_10041888', orig=0x10041888, c='Sim_10041888', gen=g_none_net, trials=400),
    dict(name='Sim_100418e3', orig=0x100418e3, c='Sim_100418e3', gen=g_418e3, trials=800, void=True),
    dict(name='Sim_10041997', orig=0x10041997, c='Sim_10041997', gen=g_none_net, trials=400),
    dict(name='Sim_100419e9', orig=0x100419e9, c='Sim_100419e9', gen=g_none_net, trials=300),
    dict(name='Sim_10041a47', orig=0x10041a47, c='Sim_10041a47', gen=g_none_net, trials=400),
    dict(name='Sim_10041c00', orig=0x10041c00, c='Sim_10041c00', gen=g_none_net, trials=400, void=True),
    dict(name='Sim_100420a0', orig=0x100420a0, c='Sim_100420a0', gen=g_none_net, trials=500),
    dict(name='Sim_100421b0', orig=0x100421b0, c='Sim_100421b0', gen=g_421b0, trials=500, void=True),
    dict(name='Sim_1002c6e2', orig=0x1002c6e2, c='Sim_1002c6e2', gen=g_2c6e2, trials=500, void=True),
    dict(name='Sim_1000b01f', orig=0x1000b01f, c='Sim_1000b01f', gen=g_1b01f, trials=600),
    dict(name='Sim_1000e6c5', orig=0x1000e6c5, c='Sim_1000e6c5', gen=g_e6c5, trials=300, void=True),
    dict(name='Sim_1003ccc4', orig=0x1003ccc4, c='Sim_1003ccc4', gen=g_ccc4, trials=400, void=True),
    dict(name='Sim_10047aa0', orig=0x10047aa0, c='Sim_10047aa0', gen=g_47aa0, trials=600),
    dict(name='Sim_1004bce5', orig=0x1004bce5, c='Sim_1004bce5', gen=g_4bce5, trials=1500, void=True),
    dict(name='Sim_1004be11', orig=0x1004be11, c='Sim_1004be11', gen=g_4be11, trials=1500),
    dict(name='Sim_1005874a', orig=0x1005874a, c='Sim_1005874a', gen=g_5874a, trials=500, void=True),
    dict(name='Render_FogFactor_10058ac2', orig=TRAMP, c='Test_Fog', gen=g_fog, trials=1500),
    dict(name='Math_ClampFloat_100686ea', orig=TRAMP, c='Test_Clamp', gen=g_clamp, trials=1000),
    dict(name='Sim_10068716', orig=0x10068716, c='Sim_10068716', gen=g_68716, trials=600),
    dict(name='Sim_10068b10', orig=0x10068b10, c='Sim_10068b10', gen=g_68b10, trials=800),
    dict(name='Sim_1008fdd7', orig=0x1008fdd7, c='Sim_1008fdd7', gen=g_8fdd7, trials=1500),
    dict(name='Sim_1009b47d', orig=0x1009b47d, c='Sim_1009b47d', gen=g_9b47d, trials=800),
    dict(name='Sim_1009cab0', orig=0x1009cab0, c='Sim_1009cab0', gen=g_box, trials=400, void=True),
    dict(name='Sim_100ad5b7', orig=0x100ad5b7, c='Sim_100ad5b7', gen=g_ad5b7, trials=800, void=True),
    dict(name='Sim_100b3fd5', orig=0x100b3fd5, c='Sim_100b3fd5', gen=g_b3fd5, trials=500, void=True),
    dict(name='Sim_100bdc5d', orig=0x100bdc5d, c='Sim_100bdc5d', gen=g_bdc5d, trials=800),
    dict(name='Sim_100c05cb', orig=TRAMP, c='Test_100c05cb', gen=g_c05cb, trials=500),
    dict(name='Math_ClampSymmetric_100c351c', orig=TRAMP, c='Test_ClampSym', gen=g_c351c, trials=800),
    dict(name='Sim_100db3f7', orig=0x100db3f7, c='Sim_100db3f7', gen=g_db3f7, trials=1000, void=True),
    dict(name='Sim_100db5c6', orig=0x100db5c6, c='Sim_100db5c6', gen=g_db5c6, trials=800, void=True),
    dict(name='Sim_100e2756', orig=0x100e2756, c='Sim_100e2756', gen=g_e2756, trials=400, void=True),
]
