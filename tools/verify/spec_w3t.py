# Differential tests for round 3, letter t (see difftest.py).
#
# C is built with SSE doubles like Diego's build (CFLAGS below); the x87 originals run with the
# Windows control word 0x27f (53-bit precision), so the float results are expected to match
# bit for bit and are compared exactly (no fp=True) except where noted.
# Stubs in the emulator (same address for the original and the C run), each logs its arguments
# into a buffer that is compared afterwards:
#   0xF00400 display driver method [[0x1015e688]+0x18] (Sim_1004b7d2), logs arg and the dword 0x1013b98c
#   0xF00410.. callbacks stored in objects (Sim_10029fba), 0xF00440 / 0xF00450 compare functions
#   (float in st0 / int in eax) of the sorted-array inserts, 0x100ba7d5 the cache loader.
# Functions returning a float in st(0) are called through a trampoline (original) or a C wrapper
# (w3t_test.c) that stores st(0) as a float at 0xAFF000.
import math
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG

SOURCES = [
    'sim/unsorted_1000/w3t_records.c', 'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1001/w3t_loader.c',
    'sim/unsorted_1002/w3t_misc.c', 'sim/unsorted_1003/w3t_bounds.c', 'sim/unsorted_1004/w3t_tables.c',
    'sim/unsorted_1006/w3t_misc.c', 'sim/unsorted_1008/w3t_rect.c', 'sim/unsorted_1009/w3t_layout.c',
    'sim/unsorted_100a/w3t_misc.c', 'sim/unsorted_100b/w3t_sorted.c', 'sim/unsorted_100c/w3t_misc.c',
    'sim/unsorted_100d/w3t_plane.c',
    # existing converted code the functions above call
    'sim/unsorted_100d/w2f_records.c', 'sim/unsorted_100d/w2i_select.c', 'sim/unsorted_100f/w2e_string.c',
    'sim/unsorted_1004/fields.c', 'sim/view/viewrect.c',
    '../tools/verify/w3t_test.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

DRV = 0x00F00400
CB = 0x00F00410          # 4 callbacks, 8 bytes apart
CMPF = 0x00F00440
CMPI = 0x00F00450
LOADER = 0x100ba7d5
TRAMP = 0x00F00800
OUTF = 0x00AFF000
LOGCAP = 0x20000
STACK_TOP = 0x00800000
ST = {}


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))

def wf(ctx, addr, f):
    ctx.m.uc.mem_write(addr, struct.pack('<f', f))

def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))

def r32(uc, a):
    return struct.unpack('<I', bytes(uc.mem_read(a, 4)))[0]

def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]

def emit(uc, vals):
    log = ST['log']
    pos = r32(uc, log)
    out = b''.join(struct.pack('<I', v & 0xffffffff) for v in vals)
    if pos + len(out) > LOGCAP:
        return
    uc.mem_write(log + 4 + pos, out)
    uc.mem_write(log, struct.pack('<I', pos + len(out)))


def tramp_code(target, nargs):
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\xd9\x1d' + struct.pack('<I', OUTF)      # fstp dword [OUTF]
    code += b'\x31\xc0\x5d\xc3'
    return code

TRAMPS = {0x10014061: 1, 0x10015266: 1, 0x1004bc65: 2, 0x10068e55: 3, 0x100c3725: 1}
TRAMP_AT = {a: TRAMP + 0x40 * i for i, a in enumerate(sorted(TRAMPS))}


def cstr(uc, addr, n=64):
    try:
        d = bytes(uc.mem_read(addr, n))
    except Exception:
        return [0xBADBAD]
    d = d.split(b'\0')[0]
    return list(d) + [0x100]


def _hook(m):
    uc = m.uc

    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        a = struct.unpack('<4I', bytes(uc.mem_read(esp + 4, 16)))
        ret = ST['ret']
        if address == DRV:
            emit(uc, [1, a[0], r32(uc, 0x1013b98c), r32(uc, a[0]), r32(uc, a[0] + 4)])
        elif CB <= address < CB + 0x20:
            emit(uc, [2 + (address - CB) // 8, a[0]])
        elif address == CMPF:
            emit(uc, [6, a[0], a[1]])
        elif address == CMPI:
            emit(uc, [7, a[0], a[1]])
        elif address == LOADER:
            emit(uc, [8, a[1], a[2]] + cstr(uc, a[0]))
            pool = ST['pool']
            k = (sum(cstr(uc, a[0])) + ST['calls']) % len(pool)     # depends only on the name, so both runs get the same image
            ret = pool[k]
        uc.reg_write(UC_X86_REG_EAX, ret & 0xffffffff)

    uc.hook_add(UC_HOOK_CODE, cb, None, DRV, CMPI + 1)
    uc.hook_add(UC_HOOK_CODE, cb, None, LOADER, LOADER)
    uc.mem_write(DRV, b'\xc3' * 0x40)
    uc.mem_write(CMPF, b'\xdb\x44\x24\x04\xda\x64\x24\x08\xc3')      # fild [esp+4]; fisub [esp+8]; ret
    uc.mem_write(CMPI, b'\x8b\x44\x24\x04\x2b\x44\x24\x08\xc3')      # mov eax,[esp+4]; sub eax,[esp+8]; ret
    uc.mem_write(LOADER, b'\xc3')
    for a, n in TRAMPS.items():
        uc.mem_write(TRAMP_AT[a], tramp_code(a, n))


def begin(ctx):
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w3t', False):
        _hook(m)
        m._w3t = True
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['log'] = ctx.buf(4 + LOGCAP + 8, 'zero')
    ST['ret'] = r.choice([0, 1, r.getrandbits(32)])
    ST['pool'] = []
    ST['calls'] = 0
    m.uc.mem_write(STACK_TOP - 0x20000, bytes([r.choice([0, 0x55, 0xA5])]) * (0x20000 - 0x180))


def rnd(ctx, n):
    return bytes(ctx.rng.getrandbits(8) for _ in range(n))

def pick(r, *c):
    return r.choice(c)

def flt(r, extra=()):
    c = r.random()
    if c < 0.03: return float('nan')
    if c < 0.05: return r.choice([float('inf'), float('-inf')])
    if c < 0.6: return r.choice([0.0, -0.0, 0.5, 1.0, 2.0, 10.0, 100.0, -1.0, -100.0, 3.0] + list(extra))
    return r.uniform(-200, 200)

def smallint(r):
    return r.choice([0, 1, 2, 3, 5, -1, -2, 100, -100, r.randrange(-50, 50), r.randrange(-1000, 1000)])


# ---- shared structures ------------------------------------------------------
NENT = 0x18

def entries(ctx, flt_pool=(0.0, 0.5, 1.0, 2.0, 10.0, 100.0, -1.0)):
    """The table of 0xd6-byte entries at 0x10145508 with sane fields."""
    r = ctx.rng
    for i in range(NENT):
        e = 0x10145508 + i * 0xd6
        wb(ctx, e, rnd(ctx, 0xd6))
        for off in (0x3c, 0x44, 0x4c, 0x50):
            wf(ctx, e + off, r.choice(flt_pool))
        w32(ctx, e + 0x5c, r.choice([0, 1, 2, 5, 10, 100, -3]))
        w32(ctx, e + 0x86, r.choice([0, 0x40, 1, 0x41, r.getrandbits(32)]))
        w32(ctx, e + 0x78, r.choice([0, 1, 5, 0x80000000, r.getrandbits(32)]))
        w32(ctx, e + 8, r.choice([0, 4, 5, 1, 7]))
        w32(ctx, e + 0x14, r.choice([0, 1, 2, 3]))


def records(ctx, n, cur=None):
    """n 0x128-byte records; returns the address."""
    r = ctx.rng
    buf = ctx.buf(max(n, 1) * 0x128 + 0x40, 'zero')
    for i in range(max(n, 1)):
        o = buf + i * 0x128
        wb(ctx, o, rnd(ctx, 0x30))
        w32(ctx, o, r.choice([-1, 0, 1, 2]))
        w32(ctx, o + 4, r.choice([0, 1, 2, 3, 0x14, 0x16, 5, 6, -1, 7]))
        w32(ctx, o + 8, r.choice([-1, 0, 1, 1, 2]))
        w32(ctx, o + 0xc, r.choice([0, 1, 2]))
        w32(ctx, o + 0x20, r.choice([0, 1, 1, 5]))
    return buf


def owner_struct(ctx):
    r = ctx.rng
    n = r.choice([0, 1, 2, 3, 5, 8])
    o = ctx.buf(0x600, 'zero')
    w32(ctx, o + 0x84, records(ctx, 8))
    w32(ctx, o + 0x4f8, n)
    w32(ctx, o + 0x4fc, r.choice([-1, 0, 1, 2, 3, 7]))
    wf(ctx, o + 0x4e8, r.choice([0.0, 1.0, 5.0, 100.0, -3.0]))
    w32(ctx, o + 0x4dc, r.choice([0, 1, 5, 100, -4]))
    return o


# ---- 0x1000xxxx ---------------------------------------------------------------
def mk_obj(ctx, owner=None):
    r = ctx.rng
    o = ctx.buf(0x600, 'zero')
    wb(ctx, o, rnd(ctx, 0x20))
    wf(ctx, o + 0xd8, r.choice([-1.0, 0.0, 0.5, 1.0, 2.0, 5.0, 10.0, 100.0, 50.0, float('nan')]))
    w32(ctx, o + 4, r.choice([0, 1, 2, 3]))
    w32(ctx, o + 0xf0, r.choice([-1, 0, 0, 0x1000, 0x2000, 5]))
    w32(ctx, o + 0x1e4, r.choice([0, 1, 5, 10, 100, 1000, -5]))
    cell = ctx.buf(8, struct.pack('<f', r.choice([0.0, 1.0, 5.0, -2.0])) + b'\0' * 4)
    w32(ctx, o + 0x54, cell)
    base = ctx.buf(0x200, 'zero')
    w32(ctx, base + 0x14, r.choice([3, 3, 0, 1, 2]))
    w32(ctx, base + 0xc, r.choice([0, 1, 2, 3]))
    w32(ctx, base + 0x1aa, r.choice([0x200, 0x201, 0x202, 0x100, 0x203, r.getrandbits(16)]))
    w32(ctx, o, base)
    w32(ctx, o + 0x28, owner if owner else owner_struct(ctx))
    # records and state of the object itself (Sim_1000b9f1 reads obj+0x84 ...)
    w32(ctx, o + 0x84, records(ctx, 8))
    w32(ctx, o + 0x4f8, r.choice([0, 1, 3, 8]))
    w32(ctx, o + 0x4fc, r.choice([-1, 0, 1, 2, 3, 7]))
    w32(ctx, o + 0x4dc, r.choice([0, 1, 5, 100, -4]))
    return o


def ptrtab(ctx):
    r = ctx.rng
    for i in range(8):
        if r.random() < 0.7:
            s = ctx.buf(0x40, 'zero')
            w32(ctx, s + 0x14, r.choice([0, 1, 2, 3]))
            w32(ctx, s + 0xc, r.choice([0, 1, 2, 3]))
            w32(ctx, 0x109e8b40 + 4 * i, s)
        else:
            w32(ctx, 0x109e8b40 + 4 * i, 0)


def g_a9bf(ctx):
    begin(ctx)
    r = ctx.rng
    obj = ctx.buf(0x100)
    ent = ctx.buf(0x100)
    pool = (0.0, 0.5, 1.0, 2.0, 10.0, 100.0, -1.0, float('nan'))
    for off in (0x3c, 0x4c, 0x50):
        wf(ctx, ent + off, r.choice(pool))
    w32(ctx, ent + 0x5c, r.choice([0, 1, 2, 5, 10, 20, -3, 1 << 30]))
    w32(ctx, ent + 0x86, r.choice([0, 0x40, r.getrandbits(32)]))
    wf(ctx, obj + 0xd8, r.choice(pool + (3.0, 20.0, 5.0)))
    return [obj, ent]


def g_b9f1(ctx):
    begin(ctx)
    r = ctx.rng
    entries(ctx)
    ptrtab(ctx)
    w32(ctx, 0x1015ec58, r.choice([0, 1, 1]))
    w32(ctx, 0x1015dc78, r.choice([0, 3, 100, -1]))
    obj = mk_obj(ctx)
    w32(ctx, obj + 0x4fc, r.choice([-1, 0, 1, 2, 3, 7, 7]))
    outs = [r.choice([0, ctx.buf(8)]) for _ in range(4)]
    return [obj if r.random() > 0.03 else 0] + outs


def g_bf73(ctx):
    begin(ctx)
    r = ctx.rng
    entries(ctx)
    w32(ctx, 0x1017b8c8, r.choice([0, 1, 2, 3]))
    obj = mk_obj(ctx)
    return [obj, ctx.buf(8)]


def g_cd55(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0, 1, 3, 6])
    w32(ctx, 0x1015d0a0, n)
    for i in range(6):
        e = 0x10a3a600 + i * 0xa8
        wb(ctx, e, rnd(ctx, 0xa8))
        w32(ctx, e, r.choice([0, 1, 2, 3]))
        wf(ctx, e + 4, r.choice([0.0, 0.5, 1.0, 2.0, 10.0, -1.0, 100.0]))
    for k in range(6):
        wf(ctx, 0x10145554 + k * 0xd6, r.choice([0.0, 1.0, 5.0, -3.0, 0.5]))
    value = r.choice([0.0, 1.0, 1.5, 2.0, 5.0, 10.0, 100.0, -5.0, r.uniform(-30, 30), 1e30, -1e30, float('nan')])
    return [r.choice([0, 1, 2, 3, 4]), fbits(value), ctx.buf(8)]


def g_ce13(ctx):
    begin(ctx)
    r = ctx.rng
    entries(ctx)
    n = r.choice([0, 1, 2, 4, 8])
    recs = records(ctx, 8)
    skip = recs + 0x128 * r.randrange(0, 8) if r.random() < 0.8 else records(ctx, 1)
    obj = mk_obj(ctx)
    return [obj, recs, skip, n]


# ---- 0x1001xxxx ---------------------------------------------------------------
def g_139e(ctx):
    begin(ctx)
    r = ctx.rng
    obj = mk_obj(ctx)
    wb(ctx, obj + 0x1b8, bytes([r.choice([0, 1, 2, 3, 0xff]), r.choice([0, 1, 2, 3, 0xff, 0xfe, 2])]))
    w32(ctx, obj, r.choice([0, 1, 1, 2]))
    tab = ctx.buf(16 * 0x1c, 'rand')
    for i in range(16):
        w32(ctx, tab + i * 0x1c + 8, r.choice([0, 0, 1, 5]))
        w32(ctx, tab + i * 0x1c + 0xc, r.choice([0, 1, 50, 100, -7, 1000, 1 << 30]))
    w32(ctx, 0x10130cd4, tab + 3 * 0x1c)
    return [obj]


def g_14061(ctx):
    begin(ctx)
    r = ctx.rng
    for i in range(256):
        c = r.random()
        v = fbits(r.choice([100.0, 150.0, 99.0, 50.0, 10000.0, float('nan'), -5.0])) if c < 0.7 else r.getrandbits(32)
        w32(ctx, 0x109a8650 + i * 0x54, v)
    word = (r.choice([0x100, 0x200, 0x400, 0x300, 0, 0x800, 0xf00, 0x500]) | r.choice([0, 1, 0x7f, 0xff, r.getrandbits(8)])) | (r.choice([0, 0x8000, 0x1000]))
    return [word | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0)]


def g_15266(ctx):
    begin(ctx)
    r = ctx.rng
    obj = ctx.buf(0x200, 'zero')
    own = ctx.buf(0x600, 'zero')
    w32(ctx, own + 0x50c, r.choice([0, 1]))
    w32(ctx, obj + 0x28, own)
    w32(ctx, obj + 4, r.choice([0, 1]))
    w32(ctx, 0x1017b8c8, r.choice([0, 1]))
    ang = (0.0, 90.0, 180.0, 270.0, -180.0, -270.0, 359.5, 400.0, -400.0, 540.0, 1e30, float('nan'), 180.5, -180.5)
    for off in (0x68, 0xe8, 0x110):
        wf(ctx, obj + off, r.choice(ang) if r.random() < 0.7 else r.uniform(-720, 720))
    return [obj]


def g_be08(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0, 1, 1, 2, 3, 4, 6])
    period = r.choice([100, 360, 7, 1000, -50, 1, r.randrange(2, 500)])
    tbl = ctx.buf(8 + 16 * 8, 'zero')
    w32(ctx, tbl, n)
    keys = sorted(r.randrange(0, max(2, abs(period))) for _ in range(n)) if r.random() < 0.8 else [r.randrange(-50, 500) for _ in range(n)]
    for k in range(n):
        w32(ctx, tbl + 4 + 16 * k, keys[k])
        wb(ctx, tbl + 8 + 16 * k, struct.pack('<3f', *[r.choice([0.0, 1.0, -2.5, 100.0, r.uniform(-50, 50)]) for _ in range(3)]))
    t = r.choice([r.randrange(-500, 1000), r.choice(keys) if keys else 0, r.choice(keys) + period if keys else 5])
    out = ctx.buf(16, 'zero') if r.random() > 0.05 else 0
    return [t, period, tbl, out]


def mk_loader_image(ctx):
    r = ctx.rng
    nb = r.choice([0, 1, 2, 3])
    d = bytearray(0x10)
    struct.pack_into('<I', d, 0xc, nb)
    for _ in range(nb):
        blk = bytearray(rnd(ctx, 0x20))
        ln = r.choice([0, 3, 7, 8, 12, 20])
        for i in range(ln):
            blk[i] = r.randrange(0x41, 0x5a)
        blk[ln] = 0
        d += blk
        c1 = r.choice([0, 1, 2, 3])
        d += struct.pack('<I', c1) + rnd(ctx, 16 * c1)
        c2 = r.choice([0, 1, 2])
        d += struct.pack('<I', c2) + rnd(ctx, 0x14 * c2)
    c3 = r.choice([0, 1, 2])
    d += struct.pack('<I', c3) + rnd(ctx, 0x39 * c3) + rnd(ctx, 32)
    return ctx.buf(len(d) + 64, bytes(d) + b'\0' * 64)


def g_b705(ctx):
    begin(ctx)
    r = ctx.rng
    ST['pool'] = [mk_loader_image(ctx) for _ in range(3)] + [0]
    ST['calls'] = r.choice([0, 0, 1, 3])
    w32(ctx, 0x1015d18c, r.getrandbits(32))
    name = ctx.buf(40, 'text')
    obj = ctx.buf(0x340, 'rand')
    return [name if r.random() > 0.05 else 0, obj if r.random() > 0.05 else 0]


def g_b880(ctx):
    begin(ctx)
    r = ctx.rng
    ST['pool'] = [mk_loader_image(ctx) for _ in range(3)] + [0]
    ST['calls'] = r.choice([0, 0, 1, 3])
    w32(ctx, 0x1015d18c, r.getrandbits(32))
    busy = r.choice([0, 1, 5, 0x7f, 0x80])
    for i in range(0x80):
        w32(ctx, 0x10a1eaa0 + i * 0x320, 1 if i < busy else 0)
    for i in (busy, busy + 1):
        if i < 0x80:
            wb(ctx, 0x10a1eaa0 + i * 0x320 + 4, rnd(ctx, 0x100))
            w32(ctx, 0x10a1eaa0 + i * 0x320, 0)
    return [ctx.buf(40, 'text')]


# ---- 0x1002xxxx / 0x1004b7d2 --------------------------------------------------
def drv_setup(ctx):
    r = ctx.rng
    tab = ctx.buf(0x40, 'zero')
    for i in range(16):
        w32(ctx, tab + 4 * i, DRV)
    w32(ctx, 0x1015e688, tab)
    w32(ctx, 0x1013b98c, r.choice([0, 1, 7, r.getrandbits(32)]))
    w32(ctx, 0x1013b990, r.choice([0, 1, 7, r.getrandbits(32)]))
    wb(ctx, 0x109b8c00, rnd(ctx, 8))
    w32(ctx, 0x1015d1b8, ctx.buf(0x100, 'rand'))


def val15(r):
    v = r.choice([0, 1, 2, 7, 100, 4369, 0xffff, 0x10000, 0x7fffffff, -1, -7, -4369, 0x80000000, r.randrange(-100000, 100000), r.getrandbits(32)])
    return v


def g_4b7d2(ctx):
    begin(ctx); drv_setup(ctx)
    return [ctx.rng.choice([0, 5, -1, ctx.rng.getrandbits(32)])]

def g_219c8(ctx):
    begin(ctx); drv_setup(ctx)
    r = ctx.rng
    v = val15(r)
    if r.random() < 0.3:
        # equal to the stored scaled value
        p = (v if v < 0x80000000 else v - (1 << 32)) * 15
        w32(ctx, 0x1013b990, ((p >> 16) + ((p >> 15) & 1)) & 0xffffffff)
    return [r.getrandbits(32), v]

def g_21a60(ctx):
    begin(ctx); drv_setup(ctx)
    return [ctx.rng.getrandbits(32)]

def g_29fba(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0, 1, 2, 4, 6])
    w32(ctx, 0x1015dc64, n)
    for i in range(8):
        o = ctx.buf(0x80, 'zero')
        w32(ctx, o + 0x34, r.choice([0, CB, CB + 8, CB + 16, CB + 24]))
        w32(ctx, o + 0x28, r.choice([0, 0, r.getrandbits(32)]))
        w32(ctx, 0x109e8b40 + 4 * i, o)
    return []


# ---- 0x1003xxxx -----------------------------------------------------------------
def vlist(ctx, n, count_word=None):
    r = ctx.rng
    buf = ctx.buf(0x18 + max(n, 1) * 0x58 + 8, 'zero')
    wb(ctx, buf + 4, struct.pack('<h', n if count_word is None else count_word))
    for i in range(n):
        wb(ctx, buf + 0x18 + i * 0x58 + 0xc, struct.pack('<3f', *[flt(r, (3.4e38, -3.4e38, 1e30)) for _ in range(3)]))
    return buf

def g_303ea(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0, 1, 2, 3, 5, 9])
    lst = vlist(ctx, n, n if r.random() > 0.1 else r.choice([-1, -5]))
    outs = [ctx.buf(8, 'rand') for _ in range(9)]
    args = [lst] + outs
    if r.random() < 0.08:
        args[r.randrange(0, 10)] = 0
    return args

def g_30672(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0, 1, 2, 4, 7])
    obj = ctx.buf(0x100, 'zero')
    w32(ctx, obj + 0x2c, vlist(ctx, n))
    blk = ctx.buf(0x24 + 7 * 0xe0 + 0x40, 'rand')
    w32(ctx, obj + 0x98, blk if r.random() > 0.1 else 0)
    return [obj if r.random() > 0.03 else 0]


# ---- 0x1004xxxx ratios and tables -----------------------------------------------
def ratio_args(r):
    b = r.choice([1, 2, 3, 4, 5, 10, 20, 100, 1000, 0, -1, -7])
    a = r.choice([b, b + 1, b * 2, b * 5 // 4, b * 6 // 5, b * 23 // 20, b * 11 // 10, b * 126 // 100, b * 127 // 100, b * 3 // 4, b * 2 // 3, b // 2, b * 1 // 2 + 1,
                  r.randrange(-10, 120), 0, 126, 100, 75, 50, 115, 110, 120])
    return [a & 0xffffffff, b & 0xffffffff]

def g_bc65(ctx):
    begin(ctx); return ratio_args(ctx.rng)

def g_bec6(ctx):
    begin(ctx); return ratio_args(ctx.rng)

def g_bfbd(ctx):
    begin(ctx)
    r = ctx.rng
    return [r.choice([0, 1, 2, 3, 5, 10, r.randrange(0, 60)]), r.getrandbits(32) if r.random() < 0.5 else r.getrandbits(20)]

def g_c0e5(ctx):
    begin(ctx)
    r = ctx.rng
    a = ratio_args(r)
    return [r.choice([0, 1, 2, 3, 5, 10, r.randrange(0, 60)]), r.getrandbits(32) if r.random() < 0.5 else r.getrandbits(20), a[0]]


def g_68e55(ctx):
    begin(ctx)
    r = ctx.rng
    p = ctx.buf(0x100, 'zero')
    for i in range(64):
        wf(ctx, p + 4 * i, flt(r))
    return [p if r.random() > 0.2 else 0, r.choice([0, 1, 2, 3, -1, 5]), r.choice([0, 1, 2, -1, 7, 15, 20, 30])]


# ---- 0x1008xxxx ---------------------------------------------------------------
def rect5(ctx):
    r = ctx.rng
    vals = [r.getrandbits(32)] + [r.choice([0, 0x10000, 0x8000, 0xc000, -0x10000 & 0xffffffff, r.getrandbits(20), r.getrandbits(32), 0x7fffffff, 0x80000000]) for _ in range(4)]
    return ctx.buf(20, struct.pack('<5I', *vals))

def g_ce40(ctx):
    begin(ctx)
    r = ctx.rng
    v = r.choice([0.0, 1.0, 0.8333333, 0.5, 2.0, -1.0, 1.25, r.uniform(-3, 3), 1e30, 1e-30])
    return [rect5(ctx), fbits(v)]

def g_e915(ctx):
    begin(ctx)
    r = ctx.rng
    cam = ctx.buf(0x100, 'zero')
    wf(ctx, cam + 0x44, r.choice([1.0, 1.0, 0.75, 2.0, 0.0, -1.0, 1.3333334, 1e-30]))
    w32(ctx, 0x1014cae0, cam)
    x0, y0 = r.randrange(-50, 300), r.randrange(-50, 200)
    w, h = r.choice([100, 320, 20, 7]), r.choice([100, 200, 20, 1])
    rect = ctx.buf(20, struct.pack('<5i', 0, x0, y0, x0 + w - 1, y0 + h - 1) if r.random() < 0.9 else rnd(ctx, 20))
    cx, cy = x0 + w // 2, y0 + h // 2
    return [rect, (cx + r.randrange(-w, w)) & 0xffffffff if r.random() < 0.8 else r.getrandbits(32), (cy + r.randrange(-h, h)) & 0xffffffff if r.random() < 0.8 else r.getrandbits(32)]


# ---- 0x1009xxxx -----------------------------------------------------------------
def view_setup(ctx, nviews=6):
    r = ctx.rng
    w32(ctx, 0x109e3e14, r.choice([319, 639, 100]))
    w32(ctx, 0x109e3de0, r.choice([199, 479, 100]))
    wb(ctx, 0x109e3e00, struct.pack('<5i', 0, 0, 0, r.choice([319, 100]), r.choice([199, 100])))
    wb(ctx, 0x109e8660, rnd(ctx, 20))
    cam = ctx.buf(0x100, 'zero')
    wf(ctx, cam + 0x44, r.choice([1.0, 0.75, 1.3333334, 2.0, 0.0]))
    w32(ctx, 0x1014cae0, cam)
    st = ctx.buf(0x20, 'zero')
    wb(ctx, st + 9, bytes([r.choice([0, 1, 1, 2])]))
    w32(ctx, 0x1017b8c4, st)
    views = []
    for i in range(nviews):
        v = ctx.buf(0x100, 'rand')
        w32(ctx, v + 0, rect5(ctx))
        w32(ctx, v + 8, r.randrange(0, 4))
        for off in (0x7c, 0x80, 0x84, 0x88):
            w32(ctx, v + off, r.randrange(0, 8))
        w32(ctx, v + 0x18, r.choice([8, 16, 3, 100, 1, 0, -4]))
        w32(ctx, v + 0x20, r.choice([100, 50, 0, 7, -9, 1 << 20]))
        w32(ctx, v + 0x24, r.choice([4, 10, 50]))
        w32(ctx, v + 0x28, r.choice([100, 64, 200]))
        if r.random() < 0.6:
            tab = ctx.buf(0x20, 'zero')
            for k in (4, 8, 0xc):
                w32(ctx, tab + k, rect5(ctx))
            ch = ctx.buf(0x20, 'zero')
            w32(ctx, ch + 4, tab)
            w32(ctx, v + 0x14, ch)
        else:
            w32(ctx, v + 0x14, 0)
        views.append(v)
    for i in range(6):
        w32(ctx, 0x1014c4c8 + 4 * i, views[i % len(views)] if r.random() > 0.05 else 0)
    w32(ctx, 0x1015d1ac, r.randrange(0, 6))
    return views

def g_996c0(ctx):
    begin(ctx)
    views = view_setup(ctx)
    r = ctx.rng
    return [r.choice([0, 1, 2, 3, 4, 5, 4, 1]), r.choice(views) if r.random() > 0.05 else 0]

def g_99928(ctx):
    begin(ctx)
    view_setup(ctx)
    r = ctx.rng
    rec = ctx.buf(0x40, 'rand')
    w32(ctx, 0x1014c06c, rec)
    w32(ctx, 0x1014befc, r.getrandbits(32))
    w32(ctx, 0x1017cc58, r.getrandbits(32))
    return []

def g_9b3a1(ctx):
    begin(ctx)
    view_setup(ctx)
    return [ctx.rng.choice([0, 1, 2, 3, -1, 2, 1])]


# ---- 0x100axxxx ---------------------------------------------------------------
def g_9d7c(ctx):
    begin(ctx)
    r = ctx.rng
    obj = ctx.buf(0x100, 'rand')
    for off in (0x30, 0x34, 0x38):
        wf(ctx, obj + off, flt(r, (1e30, 1e-30, 16777216.0)))
    return [obj] + [fbits(flt(r, (1e30, -1e30, 0.1, 1e-8))) for _ in range(3)]

def g_ab870(ctx):
    begin(ctx)
    r = ctx.rng
    cam = ctx.buf(0x100, 'rand')
    w32(ctx, 0x1014cae0, cam)
    size = ctx.buf(8, struct.pack('<ii', r.choice([640, 320, 1920, 7, 1, 0, -3, 1024]), r.choice([480, 200, 1080, 1, 0, 9, 768])))
    return [size]


# ---- 0x100bxxxx sorted insert -----------------------------------------------------
def sorted_args(ctx, cmp):
    r = ctx.rng
    cap = r.choice([1, 2, 5, 8, 12])
    n = r.randrange(0, cap + 1) if r.random() > 0.15 else min(cap, 3)
    vals = sorted(r.randrange(-20, 40) for _ in range(n)) if r.random() > 0.15 else [r.randrange(-20, 40) for _ in range(n)]
    items = ctx.buf(4 * (cap + 4), struct.pack('<%di' % n, *vals) + b'\xaa' * (4 * (cap + 4 - n)))
    arr = ctx.buf(12, struct.pack('<iiI', cap, n, items))
    value = r.choice(vals) if vals and r.random() < 0.4 else r.randrange(-25, 45)
    return [arr, value & 0xffffffff, cmp]

def g_bdb28(ctx):
    begin(ctx); return sorted_args(ctx, CMPF)

def g_bdd77(ctx):
    begin(ctx); return sorted_args(ctx, CMPI)


# ---- 0x100cxxxx / 0x100dxxxx ----------------------------------------------------
def g_c3725(ctx):
    begin(ctx)
    r = ctx.rng
    obj = ctx.buf(0x300, 'zero')
    wf(ctx, obj + 0x268, flt(r, (1e30, 1e-30)))
    wf(ctx, obj + 0xd4, flt(r))
    w32(ctx, 0x1015dc80, r.choice([0, 1, 2, 3, 7, -5, 100, 0x7fffffff]))
    return [obj]

def g_dacf9(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0, 1, 2, 3, 5, 0x40, 0x41, 0x45])
    size = 0x48 + n * 0x1c + r.choice([0, 0, 3, 27])
    if r.random() < 0.05:
        size = r.choice([0, 0x47, 0x48, 0xffffffff])
    pkt = ctx.buf(0x48 + 0x45 * 0x1c + 64, 'rand')
    w32(ctx, pkt + 4, size)
    wb(ctx, pkt + 8, bytes(r.choice(b'abcdef') for _ in range(r.choice([3, 20, 0x3f])) ) + b'\0' + rnd(ctx, 4))
    for i in range(min(n + 1, 0x45)):
        for k in range(6):
            w32(ctx, pkt + 0x48 + i * 0x1c + 4 * k, r.choice([0, 1, -1, 1 << 24, (1 << 24) + 1, 123456789, 0x7fffffff, 0x80000000, r.randrange(-1000, 1000), r.getrandbits(32)]))
    w32(ctx, 0x1015d17c, r.choice([0, 0, 1, 5, 63, 64, 0x41, -1 & 0xffffffff]))
    return [pkt]

def quant_floats(r):
    return [fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, 3.0, 100.0, -250.0, 1e-6, 1e30, -1e30, float('nan'), r.uniform(-50, 50), r.uniform(-1, 1), 1e-30])) for _ in range(4)]

def outs4(ctx):
    return [ctx.buf(8, 'rand') for _ in range(4)]

def g_db4c2(ctx):
    begin(ctx)
    return quant_floats(ctx.rng) + outs4(ctx)

def g_db659(ctx):
    begin(ctx)
    r = ctx.rng
    pts = [fbits(r.choice([0.0, 1.0, -1.0, 2.0, 5.5, 100.0, r.uniform(-30, 30), r.uniform(-1e5, 1e5), 1e30])) for _ in range(9)]
    return pts + outs4(ctx)

def g_db70f(ctx):
    begin(ctx)
    r = ctx.rng
    v = [fbits(r.choice([0.0, 1.0, -1.0, 2.0, 5.5, 0.25, r.uniform(-30, 30), 1e30, r.uniform(-1, 1)])) for _ in range(6)]
    return v + outs4(ctx)


def S(name, addr, gen, trials=500, **kw):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials)
    d.update(kw)
    return d

def SF(name, addr, gen, trials=500):
    """Float returned in st(0): the original through its trampoline, the C code through the wrapper."""
    return dict(name=name, orig=TRAMP_AT[addr], c='Test_%x' % addr, gen=gen, trials=trials)

SPECS = [
    S('Sim_1000a9bf', 0x1000a9bf, g_a9bf, 800),
    S('Sim_1000b9f1', 0x1000b9f1, g_b9f1, 1200),
    S('Sim_1000bf73', 0x1000bf73, g_bf73, 1500),
    S('Sim_1000cd55', 0x1000cd55, g_cd55, 800),
    S('Sim_1000ce13', 0x1000ce13, g_ce13, 800),
    S('Sim_1001139e', 0x1001139e, g_139e, 600, void=True),
    SF('Sim_10014061', 0x10014061, g_14061, 800),
    SF('Sim_10015266', 0x10015266, g_15266, 800),
    S('Sim_1001be08', 0x1001be08, g_be08, 1000),
    S('Sim_1001b705', 0x1001b705, g_b705, 800),
    S('Sim_1001b880', 0x1001b880, g_b880, 600),
    S('Sim_100219c8', 0x100219c8, g_219c8, 600, void=True),
    S('Sim_10021a10', 0x10021a10, g_219c8, 600, void=True),
    S('Sim_10021a60', 0x10021a60, g_21a60, 400, void=True),
    S('Sim_10029fba', 0x10029fba, g_29fba, 600, void=True),
    S('Sim_100303ea', 0x100303ea, g_303ea, 800, void=True),
    S('Sim_10030672', 0x10030672, g_30672, 600, void=True),
    S('Sim_1004b7d2', 0x1004b7d2, g_4b7d2, 400, void=True),
    SF('Sim_1004bc65', 0x1004bc65, g_bc65, 800),
    S('Sim_1004bec6', 0x1004bec6, g_bec6, 800),
    S('Sim_1004bfbd', 0x1004bfbd, g_bfbd, 600),
    S('Sim_1004c0e5', 0x1004c0e5, g_c0e5, 600),
    SF('Sim_10068e55', 0x10068e55, g_68e55, 500),
    S('Sim_1008ce40', 0x1008ce40, g_ce40, 800),
    S('Sim_1008e915', 0x1008e915, g_e915, 800),
    S('Sim_100996c0', 0x100996c0, g_996c0, 1000, void=True),
    S('Sim_10099928', 0x10099928, g_99928, 400, void=True),
    S('Sim_1009b3a1', 0x1009b3a1, g_9b3a1, 600, void=True),
    S('Sim_100a9d7c', 0x100a9d7c, g_9d7c, 600, void=True),
    S('Camera_SetAspect', 0x100ab870, g_ab870, 500, void=True),
    S('Sim_100bdb28', 0x100bdb28, g_bdb28, 1000),
    S('Sim_100bdd77', 0x100bdd77, g_bdd77, 1000),
    SF('Sim_100c3725', 0x100c3725, g_c3725, 500),
    S('Sim_100dacf9', 0x100dacf9, g_dacf9, 800),
    S('Plane_Quantize', 0x100db4c2, g_db4c2, 1500, void=True),
    S('Plane_FromPoints', 0x100db659, g_db659, 1500, void=True),
    S('Plane_FromPointNormal', 0x100db70f, g_db70f, 1500, void=True),
]
