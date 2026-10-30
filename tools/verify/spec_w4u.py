# Differential tests for round 4, letter u (see difftest.py): heap users, the CD audio module (MCI), the
# DIB window / GDI functions and the full-screen switch.
#
# The checker stubs every Windows import and compares the call log. On top of that this spec
#  * answers the API calls per trial from a hash of (seed, call number): the same answers for the
#    original and the C run, with a different mix in every trial (success, failure, 0 ...);
#  * lets the handlers write what the real function would have written (the MCI status in the parameter
#    block on the stack, the RECT, the DIB bits pointer, the counter value, the device capabilities);
#  * records the contents of the parameter blocks that the stack-based calls pass (play from/to, status
#    item/track, the window message block) in a log buffer in argument memory, compared afterwards;
#  * hooks the (empty) debug-print functions 0x10083490 / 0x10083495 and logs their arguments, as many
#    words as the format string has % conversions, so the text printed would be proven too;
#  * gives the function pointers stored in the refresh/mode objects a stub each (a `ret` at a fixed
#    address with a code hook that logs the arguments and returns a per-trial value).
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX

SOURCES = [
    'sim/win/w4u_cdaudio.c', 'sim/win/w4u_gdi.c',
    'sim/unsorted_1000/w4u_misc.c', 'sim/unsorted_1001/w4u_cd.c', 'sim/unsorted_1002/w4u_heap.c',
    'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1003/w4u_refresh.c', 'sim/unsorted_1004/w4u_heap.c',
    'sim/unsorted_1005/w4u_record.c', 'sim/unsorted_1008/w4u_misc.c', 'sim/unsorted_100a/w4u_misc.c',
    # callees that are already converted
    'sim/win/w4a_heap.c', 'sim/core/timer.c', 'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_1003/w2e_misc.c',
    'sim/unsorted_1004/w2l_values.c', 'sim/unsorted_1003/w3p_misc.c', 'sim/unsorted_1002/w2e_object.c', 'sim/unsorted_1004/fields.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

ST = {}
STUB0 = 0x00F00A00
HEAPVAR = 0x1017b8f8


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def r32(uc, a):
    return struct.unpack('<I', bytes(uc.mem_read(a, 4)))[0]


def u32w(uc, a, v):
    uc.mem_write(a, struct.pack('<I', v & 0xffffffff))


def raw(uc, k):
    """The k-th argument word of the stub call that is running (the stack pointer is at the return address)."""
    return r32(uc, uc.reg_read(UC_X86_REG_ESP) + 4 + 4 * k)


def hsh(n, salt=0):
    x = (ST['seed'] + n * 0x9E3779B1 + salt * 0x85EBCA6B) & 0xffffffff
    x ^= x >> 15
    x = (x * 0x2C1B3C6D) & 0xffffffff
    x ^= x >> 12
    x = (x * 0x297A2D39) & 0xffffffff
    x ^= x >> 15
    return x


def rnd(ctx, n):
    return bytes(ctx.rng.getrandbits(8) for _ in range(n))


def alog(uc, n, *vals):
    """Record of one stack-block call in the api log buffer: 4 dwords per call number."""
    a = ST['alog'] + 16 * (n & 63)
    uc.mem_write(a, struct.pack('<4I', *([v & 0xffffffff for v in vals] + [0] * 4)[:4]))


# ---- stubs and hooks --------------------------------------------------------
def dbg_emit(m, items):
    uc = m.uc
    log = ST['dbg']
    pos = r32(uc, log)
    if pos + len(items) > 250:
        return
    uc.mem_write(log + 4 + 4 * pos, struct.pack('<%dI' % len(items), *[v & 0xffffffff for v in items]))
    u32w(uc, log, pos + len(items))


def install(m):
    if getattr(m, '_w4u', False):
        return
    m._w4u = True
    uc = m.uc
    uc.mem_write(STUB0, b"\xc3" * 0x120)

    def cb(raw_uc, address, size, ud):
        if 'dbg' not in ST:
            return
        esp = raw_uc.reg_read(UC_X86_REG_ESP)
        if address in (0x10083490, 0x10083495):
            fmt = struct.unpack('<I', bytes(raw_uc.mem_read(esp + 4, 4)))[0]
            text = bytes(raw_uc.mem_read(fmt, 96)).split(b'\0')[0]
            nargs = text.count(b'%') - 2 * text.count(b'%%')
            words = struct.unpack('<%dI' % (1 + nargs), bytes(raw_uc.mem_read(esp + 4, 4 + 4 * nargs)))
            dbg_emit(m, [address] + list(words))
        elif STUB0 <= address < STUB0 + 0x120:
            k = (address - STUB0) // 16
            if k < 8:       # method +0x10 of mode object k: 3 arguments, returns a value
                a = struct.unpack('<3I', bytes(raw_uc.mem_read(esp + 4, 12)))
                dbg_emit(m, [0xA000 + k] + list(a))
                raw_uc.reg_write(UC_X86_REG_EAX, ST['m10'][k])   # UC_X86_REG_EAX
            elif k < 16:    # method +0x14 of mode object k-8: no arguments
                dbg_emit(m, [0xB000 + k])
            else:           # method +0x14 of the driver object: 4 arguments
                a = struct.unpack('<4I', bytes(raw_uc.mem_read(esp + 4, 16)))
                dbg_emit(m, [0xC000] + list(a))

    uc.hook_add(UC_HOOK_CODE, cb, None, 0x10083490, 0x10083495)
    uc.hook_add(UC_HOOK_CODE, cb, None, STUB0, STUB0 + 0x11f)


def begin(ctx):
    """Per-trial state: seed, the api/debug log buffers, a heap handle, blocks for HeapAlloc."""
    r = ctx.rng
    install(ctx.m)
    ST.clear()
    ST['seed'] = r.getrandbits(32)
    ST['alog'] = ctx.buf(0x400, 'zero')
    ST['dbg'] = ctx.buf(0x420, 'zero')
    ST['blk'] = [ctx.buf(0x800)]
    ST['mci_fail'] = r.choice([0, 0, 0, 0, 10, 35])
    ST['m10'] = [r.choice([0, 0, 1, 7, r.getrandbits(32)]) for _ in range(8)]
    w32(ctx, HEAPVAR, r.getrandbits(32) | 0x10000)


# ---- API handlers -----------------------------------------------------------
def a_heapalloc(n, a):
    return ST['blk'][n % len(ST['blk'])]


def a_mci(n, a, uc):
    msg, flags = a[1], a[2]
    ptr = raw(uc, 3)
    h = hsh(n)
    ret = 0x111 if h % 100 < ST['mci_fail'] else 0
    if msg == 0x814:
        item = r32(uc, ptr + 8)
        track = r32(uc, ptr + 12) if flags & 0x10 else 0
        if item == 4:
            modes = [0x20d, 0x20e, 0x211, 0x20d, 0x20e, 0x211, 0x212, 0x20c, 0x20f, 0x210, 0x213, 1, 0xffff]
            v = modes[(h >> 4) % len(modes)] if n < 12 else 0x20d
        elif item == 3:
            v = (h >> 8) % 7
        elif flags & 0x10:
            v = h
        else:
            v = (h & 0xffffff00) | ((h >> 8) % 8)
        if ret == 0:
            u32w(uc, ptr + 4, v)
        alog(uc, n, msg, flags, item | (track << 16), v if ret == 0 else 0xdead)
    elif msg == 0x806:
        alog(uc, n, msg, flags, r32(uc, ptr + 4), r32(uc, ptr + 8) if flags & 8 else 0)
    else:
        alog(uc, n, msg, flags)
    return ret


def a_auxnum(n, a):
    return ST['ndev']


def a_auxcaps(n, a, uc):
    dev = a[0]
    ptr = raw(uc, 1)
    h = hsh(n)
    if n == ST['errn']:
        return 0x20
    techs = ST['techs']
    u32w(uc, ptr + 0x28, (techs[dev % len(techs)] if dev < len(techs) else 0) | (h & 0xffff0000))
    alog(uc, n, dev, a[2])
    return 0


def a_send(n, a, uc):
    ptr = raw(uc, 3)
    h = hsh(n)
    block = struct.unpack('<3I', bytes(uc.mem_read(ptr, 12))) if a[1] == 0x4a else (0, 0, 0)
    alog(uc, n, a[1], block[0], block[1], block[2])
    if a[1] == 0x4a:
        # the third dword holds the pointer 0x109e38c0 for a palette block: log what it points to instead
        pass
    return [0, 0, h, h & 0xff, 0x8000, 1][(h >> 5) % 6]


def a_show(n, a):
    return [0, 1, 1, hsh(n) & 0xff][hsh(n, 3) % 4]


def a_qpc(n, a, uc):
    ptr = raw(uc, 0)
    h = hsh(n)
    if h % 10 == 0:
        return 0
    lo = hsh(n, 5) if ST.get('qpc_same_hi') is None else hsh(n, 5)
    hi = 0x12 if (h >> 4) % 3 else hsh(n, 6) & 0xff
    uc.mem_write(ptr, struct.pack('<2I', lo, hi))
    return 1


def a_winrect(n, a, uc):
    uc.mem_write(a[1], struct.pack('<4I', hsh(n, 1), hsh(n, 2), hsh(n, 3), hsh(n, 4)))
    return 1


def a_createdib(n, a, uc):
    h = hsh(n)
    ppv = a[3]
    u32w(uc, ppv, [0, h | 0x100000, h | 0x100000][(h >> 3) % 3])
    alog(uc, n, a[2], a[4], a[5], 0)
    return [0, h | 1, h | 1][(h >> 7) % 3]


def a_setdibits(n, a):
    lines = r32_global(0x109e3d18)
    h = hsh(n)
    return [0, lines, lines, h & 0xff, lines + 1][(h >> 5) % 5]


def a_hash(salt):
    return lambda n, a: hsh(n, salt)


def a_createpalette(n, a):
    return hsh(n, 9) | 1


GLOBALS = {}


def r32_global(addr):
    return GLOBALS[addr]


API = {
    'HeapAlloc': a_heapalloc, 'mciSendCommandA': a_mci, 'auxGetNumDevs': a_auxnum, 'auxGetDevCapsA': a_auxcaps,
    'SendMessageA': a_send, 'ShowWindow': a_show, 'QueryPerformanceCounter': a_qpc, 'GetWindowRect': a_winrect,
    'CreateDIBSection': a_createdib, 'SetDIBitsToDevice': a_setdibits, 'GetLastError': a_hash(11),
    'SetDIBColorTable': a_hash(12), 'SelectObject': a_hash(13), 'CreatePalette': a_createpalette,
    'SelectPalette': a_hash(14), 'SetPaletteEntries': a_hash(15), 'RealizePalette': a_hash(16),
    'HeapFree': a_hash(17), 'FreeLibrary': a_hash(18),
}


# ---- generators: heap users -------------------------------------------------
def g_1000e62e(ctx):
    r = ctx.rng
    begin(ctx)
    obj = ctx.buf(0x200)
    n = r.choice([0, 1, 2, 3, 5, 8])
    w32(ctx, obj + 4, n)
    for i in range(n):
        w32(ctx, obj + 0x54 + 4 * i, r.choice([0, 1, r.getrandbits(32)]))
    w32(ctx, 0x1015d0a4, obj if r.random() < 0.92 else 0)
    return []


def g_none(ctx):
    begin(ctx)
    w32(ctx, 0x1015dc30, ctx.rng.getrandbits(32))
    return []


def g_free(ctx):
    begin(ctx)
    return [ctx.rng.choice([0, ctx.u32(), ctx.buf(16)])]


def g_alloc_only(ctx):
    r = ctx.rng
    begin(ctx)
    ST['blk'] = [0 if r.random() < 0.12 else ctx.buf(0x800)]
    return []


def chain(ctx, k, pool):
    """A list of k nodes {next, value} taken from pool (a list of free addresses); returns the head."""
    head = 0
    for _ in range(k):
        node = pool.pop()
        w32(ctx, node, head, ctx.rng.getrandbits(32))
        head = node
    return head


def g_listfree(ctx):
    r = ctx.rng
    begin(ctx)
    pool = [ctx.buf(8) for _ in range(8)]
    owner = ctx.buf(0x40)
    w32(ctx, owner + 0x1c, chain(ctx, r.choice([0, 0, 1, 2, 3, 5]), pool))
    return [0 if r.random() < 0.08 else owner]


def g_listpush(ctx):
    r = ctx.rng
    begin(ctx)
    ST['blk'] = [0 if r.random() < 0.15 else ctx.buf(16)]
    pool = [ctx.buf(8) for _ in range(3)]
    owner = ctx.buf(0x40)
    w32(ctx, owner + 0x1c, chain(ctx, r.choice([0, 0, 1, 2]), pool))
    w32(ctx, owner + 0x18, r.choice([0, 1, 3, 0xffffffff, r.getrandbits(32)]))
    return [0 if r.random() < 0.08 else owner, 0 if r.random() < 0.12 else r.choice([1, ctx.buf(16), ctx.u32()])]


def g_305f8(ctx):
    r = ctx.rng
    begin(ctx)
    pool = [ctx.buf(8) for _ in range(60)]
    owner = ctx.buf(0x24 + 7 * 0xe0 + 0x40, 'zero')
    for i in range(7):
        for j in range(7):
            if r.random() < 0.35:
                w32(ctx, owner + 0x24 + i * 0xe0 + j * 32 + 0x1c, chain(ctx, r.choice([1, 1, 2, 3]), pool))
    return [0 if r.random() < 0.06 else owner]


def g_3693c(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1015e570, r.choice([0, 1, r.getrandbits(32)]))
    w32(ctx, 0x1015e568, r.getrandbits(32))
    return []


def g_slotenv(ctx, rg):
    """The environment of Sim_10038ad5 (copied from spec_w2e.py): returns (obj, rec factory)."""
    obj = ctx.buf(0x700, 'rand')
    slots = ctx.buf(8 * 0x128, 'rand')
    for i in range(8):
        w32(ctx, slots + i * 0x128 + 4, rg.choice([-1, -1, 0, 1, 2, 3, rg.randrange(0, 20)]))
        w32(ctx, slots + i * 0x128 + 8, rg.choice([-1, 1, 0, 5]))
        w32(ctx, slots + i * 0x128 + 0x20, rg.choice([-2, 0, 3]))
    ents = ctx.buf(0x32 * 0x24 + 0x10, 'rand')
    for i in range(0x32):
        ctx.m.uc.mem_write(ents + i * 0x24 + 2, struct.pack('<H', rg.choice([0, 0, 0, 1, 0xffff, rg.getrandbits(16)])))
    top = ctx.buf(16, 'rand')
    v = rg.choice([0, 1, 7])
    w32(ctx, 0x1017b8c8, v)
    w32(ctx, top + 4, rg.choice([v, v, 5]))
    w32(ctx, obj + 0, top)
    w32(ctx, obj + 0x84, slots)
    w32(ctx, obj + 0x8c, ents)
    w32(ctx, obj + 0x5fb, rg.choice([0, 1, 2, 5, rg.randrange(0, 20)]))
    w32(ctx, 0x109e88ec, *[ctx.buf(0x60, 'rand') for _ in range(8)])
    wb(ctx, 0x10145592, rnd(ctx, 0xd6 * 32))
    return obj


def mkrec(ctx, rg):
    rec = ctx.buf(0x900, 'rand')
    w32(ctx, rec + 4, rg.getrandbits(32))
    w32(ctx, rec + 0x68c, rg.getrandbits(32))
    w32(ctx, rec + 0x10, rg.choice([0, 1, 2, 5, rg.randrange(0, 30)]))
    e108 = rg.choice([0, 1, 2, 3, rg.randrange(0, 0x30)])
    w32(ctx, rec + 0x44, rg.choice([0, 1, 2, 5, 0x20, 0x31, 0x32, 0x40, rg.randrange(0, 0x40)]))
    w32(ctx, rec + 0x114, e108)
    if rg.random() < 0.5:
        w32(ctx, rec + 0x44, e108 + 1)
    w32(ctx, rec + 0x14, rg.choice([-1, 1, 0, 5]))
    w32(ctx, rec + 0x2c, rg.choice([-2, 0, 3]))
    for k in range(0x32):
        ctx.m.uc.mem_write(rec + 0x134 + k * 0x24 + 2, struct.pack('<H', rg.choice([0, 0, 1, 0xffff, rg.getrandbits(16)])))
    ctx.m.uc.mem_write(rec + 0x136 + e108 * 0x24, struct.pack('<H', rg.choice([0, 0, 1, rg.getrandbits(16)])))
    return rec


def g_3913b(ctx):
    r = ctx.rng
    begin(ctx)
    obj = g_slotenv(ctx, r)
    k = r.choice([0, 1, 2, 3, 4])
    recs = [mkrec(ctx, r) for _ in range(k)]
    keys = [r.choice([1, 2, 3, 0x55, r.getrandbits(32)]) for _ in range(k)]
    for i, rec in enumerate(recs):
        w32(ctx, rec, keys[i])
        w32(ctx, rec + 0x690, recs[i - 1] if i > 0 else 0)
        w32(ctx, rec + 0x694, recs[i + 1] if i + 1 < k else 0)
    w32(ctx, 0x109e3760, recs[-1] if k else 0)
    w32(ctx, 0x109e3770, recs[0] if k else 0)
    key = r.choice(keys) if keys and r.random() < 0.8 else r.choice([0, 9, r.getrandbits(32)])
    return [key, obj]


def g_5a138(ctx):
    r = ctx.rng
    begin(ctx)
    rec = ctx.buf(0x100)
    for off in (0x88, 0xc8, 0xd0):
        w32(ctx, rec + off, r.choice([0, 0, r.getrandbits(32)]))
    w32(ctx, 0x1017b910, r.choice([0, r.getrandbits(32) | 1]))
    w32(ctx, 0x1017b900, r.getrandbits(32))
    return [rec]


def g_image(ctx, count):
    r = ctx.rng
    img = ctx.buf(0x12c + 0x48 * 9 + 0x6c + 0x20)
    wb(ctx, img + 0x124, struct.pack('<I', count))      # the dword at record+0x12d
    return img


def g_5b026(ctx):
    r = ctx.rng
    begin(ctx)
    ST['blk'] = [ctx.buf(0x1c0), ctx.buf(0x500)]
    return [g_image(ctx, r.choice([0, 0, 1, 2, 3, 8]))]


def g_5b112(ctx):
    r = ctx.rng
    begin(ctx)
    rec = ctx.buf(0x1c0)
    w32(ctx, rec + 0x131, r.choice([0, r.getrandbits(32)]))
    return [rec]


def g_5b14e(ctx):
    r = ctx.rng
    begin(ctx)
    ST['blk'] = [ctx.buf(0x1c0), ctx.buf(0x500)] if r.random() < 0.9 else [0, 0]
    return [g_image(ctx, r.choice([0, 1, 2, 5, 8])), ctx.buf(8)]


def g_a87f0(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1017d180, r.choice([0, ctx.u32()]))
    return []


# ---- generators: CD audio ---------------------------------------------------
def cd_env(ctx, open_p=0.92):
    r = ctx.rng
    begin(ctx)
    ST['blk'] = [ctx.buf(0x400)] if r.random() < 0.9 else [0]
    w32(ctx, 0x1014c9b0, r.choice([0, 1, 2, 0xffffffff, r.getrandbits(16)]))
    w32(ctx, 0x1017d08c, 1 if r.random() < open_p else 0)
    first = r.choice([1, 1, 1, 2, 0, 5])
    last = first + r.randrange(0, 7)
    tbl = ctx.buf(0x80)
    w32(ctx, 0x1017d080, first, last, tbl)
    w32(ctx, 0x1017d068, r.randrange(0, 9), r.getrandbits(8), r.getrandbits(6), r.getrandbits(5))
    w32(ctx, 0x1017d090, ctx.u32(), ctx.u32())
    w32(ctx, 0x1017d078, ctx.u32())
    return first, last


def g_cd(ctx):
    cd_env(ctx)
    return []


def g_play(ctx):
    cd_env(ctx)
    r = ctx.rng
    return [r.choice([0, 1, ctx.u32(), 0x010203]), r.choice([0, 0, 1, ctx.u32(), 0x020406])]


def g_skip(ctx):
    r = ctx.rng
    first, last = cd_env(ctx)
    rng_arg = 0x1017d080
    if r.random() < 0.2:
        rng_arg = ctx.buf(16)
        w32(ctx, rng_arg, r.choice([0, 1, 2, 3]), r.choice([2, 3, 5, 8]))
    return [rng_arg, r.choice([-1, 1, 1, -1, 2, -2, 0, 0xffffffff, ctx.s32small()])]


def g_playtrack(ctx):
    r = ctx.rng
    first, last = cd_env(ctx)
    return [r.randrange(first - 2, last + 3) & 0xffffffff if r.random() < 0.9 else ctx.u32() & 0xff]


def g_playrange(ctx):
    r = ctx.rng
    cd_env(ctx)
    a = r.choice([0, 5, ctx.u32(), 0x010000])
    b = (a + r.randrange(1, 100)) & 0xffffffff if r.random() < 0.7 else r.choice([0, a, ctx.u32()])
    return [a, b]


def g_out4(ctx):
    cd_env(ctx)
    return [ctx.buf(16)]


def g_loadtracks(ctx):
    r = ctx.rng
    cd_env(ctx)
    rng_arg = ctx.buf(16) if r.random() < 0.7 else 0x1017d080
    return [rng_arg]


def g_freetracks(ctx):
    r = ctx.rng
    cd_env(ctx)
    rng_arg = ctx.buf(16)
    w32(ctx, rng_arg + 8, r.choice([0, ctx.u32(), ctx.u32()]))
    return [rng_arg]


def g_cdflags(ctx):
    r = ctx.rng
    cd_env(ctx)
    w32(ctx, 0x1015d1bc, r.choice([0, 1, 1, 0, 2]))
    w32(ctx, 0x101321a8, r.choice([0xffffffff, 0, 1, 5, ctx.u32()]))
    w32(ctx, 0x101321c0, r.choice([0, 8, 8, 8, 0xf, 0xffffffff, ctx.u32()]))
    w32(ctx, 0x1015ecd0, r.choice([0, 0, 1, 5]))
    return []


def g_aux(ctx):
    r = ctx.rng
    begin(ctx)
    ST['ndev'] = r.choice([0, 1, 2, 3, 4, 0xffffffff, 5])
    ST['techs'] = [r.choice([1, 2, 3, 0, 1, 2, 0x10001, 0x20000]) for _ in range(5)]
    ST['errn'] = r.choice([-1, -1, -1, 1, 2, 3])
    return []


# ---- generators: display ----------------------------------------------------
def g_dibend(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1017b35c, r.choice([0, 0x4242, ctx.u32()]))
    w32(ctx, 0x1017b364, ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32())
    w32(ctx, 0x1017b378, ctx.u32())
    w32(ctx, 0x1015e9a0, ctx.u32())
    w32(ctx, 0x1015e698, ctx.buf(16))
    return []


def g_dibdraw(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1017b35c, ctx.u32())
    w32(ctx, 0x109e3ce8, ctx.u32())
    w32(ctx, 0x1015e9a0, r.choice([0, 0, ctx.u32()]))
    w32(ctx, 0x1015e698, ctx.buf(16))
    return []


def g_dibsetfmt(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1017b35c, ctx.u32())
    w32(ctx, 0x1013b98c, r.randrange(0, 4))
    return [0 if r.random() < 0.1 else ctx.buf(768 + 16)]


def g_1008375a(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1017b7d0, r.choice([0, 0, ctx.u32() | 1]))
    w32(ctx, 0x1017b7cc, ctx.u32())
    w32(ctx, 0x1017b3c0, ctx.u32())
    w32(ctx, 0x1015e698, ctx.buf(16))
    w32(ctx, 0x1015e9a0, ctx.u32())
    return []


def g_bltrect(ctx):
    r = ctx.rng
    begin(ctx)
    lines = r.choice([0, 1, 100, 480, 600, r.randrange(0, 1200)])
    w32(ctx, 0x109e3d18, lines)
    w32(ctx, 0x1015e9a0, ctx.u32())
    w32(ctx, 0x1017b3c0, ctx.u32())
    GLOBALS[0x109e3d18] = lines
    return [r.randrange(-5, 200) & 0xffffffff, r.randrange(-5, 200) & 0xffffffff, r.randrange(0, 700), r.randrange(0, 700)]


def g_setpal(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1017b3c4, r.choice([0, 0, ctx.u32() | 1]))
    w32(ctx, 0x1017b3c0, ctx.u32())
    wb(ctx, 0x1015e6a0, rnd(ctx, 768))
    if r.random() < 0.85:
        start = r.randrange(0, 256)
        count = r.randrange(1, 257 - start)
    else:
        start = r.choice([-1, 0, 255, 256, 300, 0x80000000])
        count = r.choice([0, -1, 1, 256, 257, 0x7fffffff])
    return [start & 0xffffffff, count & 0xffffffff, 0 if r.random() < 0.05 else ctx.buf(768 + 16)]


def g_pal4(ctx):
    r = ctx.rng
    a = g_setpal(ctx)
    w32(ctx, 0x1017b7d8, r.choice([0, 1, 1, 1]))
    return a + [ctx.u32()]


# ---- generators: refresh modes / full screen -------------------------------
def modes_env(ctx, r):
    """7 mode objects with function stubs, the driver object, the current object."""
    begin(ctx)
    objs = []
    for i in range(7):
        o = ctx.buf(0x40, 'rand')
        w32(ctx, o + 0, r.choice([0, 1, 2, 3, 4, 5, 6, 7]))
        w32(ctx, o + 4, r.randrange(0, 3))
        w32(ctx, o + 8, r.choice([0, 1, 1, 5]))
        w32(ctx, o + 0xc, ctx.u32())
        w32(ctx, o + 0x10, STUB0 + 16 * i)
        w32(ctx, o + 0x14, STUB0 + 16 * (8 + i))
        objs.append(o)
    wb(ctx, 0x1013a2a8, struct.pack('<7I', *objs))
    # the mode table 0x1013a2ac (indexed by mode number) is the table above shifted by one entry
    other = [ctx.buf(16, 'rand') for _ in range(3)]
    for o in other:
        w32(ctx, o + 4, r.choice([1, 2, 2, 3]))
    w32(ctx, 0x1013a298, *other)
    drv = ctx.buf(0x40, 'rand')
    w32(ctx, drv + 4, r.choice([0, 1, 1, 2]))
    w32(ctx, drv + 0x14, STUB0 + 16 * 16)
    w32(ctx, 0x1015e688, drv)
    w32(ctx, 0x1015e68c, r.choice(objs))
    w32(ctx, 0x1015e690, r.choice([0] + objs))
    for a in (0x1015e698, 0x109e3d14, 0x109e3d18):
        w32(ctx, a, ctx.u32())
    return objs, other


def g_10037cab(ctx):
    r = ctx.rng
    objs, other = modes_env(ctx, r)
    # mode objects numbered 0..6; the table 0x1013a2ac (indexed by the mode number) leads to the next number,
    # so the walk over unavailable modes ends (the original would loop for ever on a cycle)
    order = list(range(7))
    r.shuffle(objs)
    for k, o in enumerate(objs):
        w32(ctx, o, k)
    w32(ctx, 0x1013a2ac, *(objs[1:] + [objs[6]]))
    w32(ctx, 0x1013a2c4, r.choice([3, 3, 3, 1, 5, 6, 0xffffffff]))
    w32(ctx, 0x1015e68c, r.choice(objs))
    w32(ctx, 0x1015e9cc, r.choice([1, 5, 5, 5, 5, 0, 2, 3, 4, 6, 0xffffffff]))
    w32(ctx, 0x1015e694, r.choice([0, 1]))
    w32(ctx, 0x1015e680, ctx.u32(), r.choice([0x12, 0x13]))
    return []


def g_togglefs(ctx):
    r = ctx.rng
    objs, other = modes_env(ctx, r)
    w32(ctx, 0x1015e694, r.choice([0, 0, 0, 1]))
    w32(ctx, 0x1015e9c0, r.choice([0, 0, 1]))
    w32(ctx, 0x1015e9c4, r.randrange(500, 900))
    w32(ctx, 0x1015e9c8, r.randrange(300, 700))
    w32(ctx, 0x109e3d14, r.randrange(500, 900))
    w32(ctx, 0x109e3d18, r.randrange(300, 700))
    w32(ctx, 0x1017b92c, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b8f0, ctx.u32())
    w32(ctx, 0x109e3780, ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32())
    w32(ctx, 0x1013a2ac, *[r.choice(objs) for _ in range(7)])
    # make the first branch (object 0x1015e688 with +4 == 1) find a usable object often
    return []


def sp(name, addr, gen, trials, void=False):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials, api=API)
    if void:
        d['void'] = True
    return d


SPECS = [
    sp('Sim_1000e62e', 0x1000e62e, g_1000e62e, 400, True),
    sp('Sim_1001f335', 0x1001f335, g_cdflags, 400, True),
    sp('Sim_1001f367', 0x1001f367, g_cdflags, 400, True),
    sp('Sim_1001f399', 0x1001f399, g_cdflags, 400, True),
    sp('Sim_1001f4d3', 0x1001f4d3, g_cdflags, 400, True),
    sp('Sim_10027f06', 0x10027f06, g_none, 300, True),
    sp('Heap_Free_100285a3', 0x100285a3, g_free, 300, True),
    sp('Sim_1002f9e0', 0x1002f9e0, g_alloc_only, 400),
    sp('ListNode_Alloc', 0x10030150, g_alloc_only, 300),
    sp('List_FreeNodes', 0x10030175, g_listfree, 400, True),
    sp('List_PushFront', 0x100301cc, g_listpush, 500),
    sp('Sim_1003058f', 0x1003058f, g_alloc_only, 400),
    sp('Sim_100305f8', 0x100305f8, g_305f8, 400, True),
    sp('Sim_1003693c', 0x1003693c, g_3693c, 300),
    sp('Sim_1003913b', 0x1003913b, g_3913b, 800, True),
    sp('Heap_Free_100461d0', 0x100461d0, g_free, 300, True),
    sp('Sim_1005a138', 0x1005a138, g_5a138, 400, True),
    sp('Sim_1005b026', 0x1005b026, g_5b026, 400),
    sp('Sim_1005b112', 0x1005b112, g_5b112, 300, True),
    sp('Sim_1005b14e', 0x1005b14e, g_5b14e, 400),
    sp('Sim_100a87f0', 0x100a87f0, g_a87f0, 300, True),
    sp('Sim_1008b900', 0x1008b900, g_cdflags, 400, True),
    sp('Aux_FindCdDevice', 0x1009dec0, g_aux, 500),
    sp('Cd_Close', 0x1009e11d, g_cd, 300),
    sp('Cd_Pause', 0x1009e149, g_cd, 300),
    sp('Cd_Resume', 0x1009e169, g_cd, 300),
    sp('Cd_Stop', 0x1009e189, g_cd, 300),
    sp('Cd_Play', 0x1009e1a9, g_play, 400),
    sp('Cd_Shutdown', 0x1009e2b9, g_cd, 400, True),
    sp('Cd_GetStatus', 0x1009e2e3, g_cd, 600),
    sp('Cd_UpdateStatus', 0x1009e389, g_cd, 400),
    sp('Cd_TogglePause', 0x1009e3cb, g_cd, 800, True),
    sp('Sim_1009e441', 0x1009e441, g_cd, 800, True),
    sp('Cd_StopWait', 0x1009e4ae, g_cd, 500, True),
    sp('Cd_PrevTrack', 0x1009e4cb, g_cd, 800, True),
    sp('Cd_NextTrack', 0x1009e4e8, g_cd, 800, True),
    sp('Cd_GetPosition', 0x1009e505, g_out4, 600, True),
    sp('Cd_Skip', 0x1009e680, g_skip, 1000, True),
    sp('Cd_PlayTrack', 0x1009e768, g_playtrack, 1000, True),
    sp('Cd_PlayRange', 0x1009e886, g_playrange, 800, True),
    sp('Cd_LoadTracks', 0x1009e947, g_loadtracks, 600),
    sp('Cd_FreeTracks', 0x1009ea75, g_freetracks, 300, True),
    sp('Cd_ReadPosition', 0x1009eaa2, g_out4, 500, True),
    sp('DisplayDibWindowEnd', 0x1007eda5, g_dibend, 400),
    sp('DisplayDibWindowDraw', 0x1007ee9a, g_dibdraw, 400),
    sp('DisplayDibWindowSetFmt', 0x1007f224, g_dibsetfmt, 400),
    sp('Sim_1008375a', 0x1008375a, g_1008375a, 500),
    sp('Gdi_BitBltRect', 0x10083cd5, g_bltrect, 500),
    sp('Gdi_SetPalette', 0x10083ebb, g_setpal, 600),
    sp('Sim_1008400d', 0x1008400d, g_pal4, 500),
    sp('Sim_10037cab', 0x10037cab, g_10037cab, 1000, True),
    sp('ToggleFullScreen', 0x10037f71, g_togglefs, 1000, True),
]
