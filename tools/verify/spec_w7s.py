# Differential tests for round 7, letter s (see difftest.py and docs/checker.md): C runtime code of HGSIM.DLL at
# 0x10104970..0x1010d180 (src/sim/unsorted_1010/w7s_*.c).
#
# Helpers are replaced by emulator hooks on their first instruction (code bytes are not patched) only in the
# trials that ask for it (ST['active']): the hook appends ('stub_<address>', arguments) to the checker's call log
# (compared between the original and the C run; stack addresses shown as 'stack', text arguments of 0x10107840
# and 0x1010b2a0 logged as their bytes), sets eax to an answer chosen by the trial and returns to the caller.
# Converted helpers that are stubbed (Heap_Malloc 0x100fef30, Heap_FreeBlock 0x100fefe0, Sim_10106660,
# Sim_10107840) get the same hook on the original address AND on their C symbol, so both runs see the same answer.
# Every other helper runs for real on both sides (the original for the original, the converted C for the C).
# Windows functions that fill structures on the stack (GetStartupInfoA, GetCPInfo, GetModuleFileNameA,
# WideCharToMultiByte) are answered by handlers that write the same bytes for both runs through the pointer
# argument read from the stack.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW,
                               UC_X86_REG_FPSW, UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_1010/w7s_stdio.c', 'sim/unsorted_1010/w7s_heap.c', 'sim/unsorted_1010/w7s_locale.c',
    'sim/unsorted_1010/w7s_env.c', 'sim/unsorted_1010/w7s_math.c', 'sim/unsorted_1010/w7s_misc.c',
    # existing converted code the functions above call, and what those files need
    'sim/unsorted_1010/w6j_stream.c', 'sim/unsorted_1010/w6k_stream.c', 'sim/unsorted_1010/w6k_sbh.c',
    'sim/unsorted_1010/w6k_text.c', 'sim/unsorted_1010/w6k_misc.c', 'sim/unsorted_1010/w2h_misc.c',
    'sim/unsorted_1010/w2i_crt.c', 'sim/unsorted_1010/w2g_crt.c', 'sim/unsorted_1010/w2k_crt.c',
    'sim/unsorted_1010/w2f_mantissa.c', 'sim/unsorted_1010/w2f_ld12.c',
    'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_100f/w6i_mem.c',
    'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_100f/w2h_mem.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
MALLOC, FREE = 0x100fef30, 0x100fefe0
# helper address -> number of dword arguments
STUBS = {MALLOC: 1, FREE: 1, 0x101099a0: 4, 0x10100580: 2, 0x10105b50: 3, 0x101009b0: 1, 0x1010b2a0: 2,
         0x10106660: 0, 0x10107840: 3, 0x10105a60: 1}
SYM_STUBS = {'Heap_Malloc': MALLOC, 'Heap_FreeBlock': FREE, 'Sim_10106660': 0x10106660, 'Sim_10107840': 0x10107840,
             'Crt_SOpen': 0x101099a0}   # C runtime replacement: the C calls _sopen by name now
TEXT_ARGS = {0x10107840: (0, 1), 0x1010b2a0: (0,)}
ST = {'active': set(), 'fn': {}, 'api': {}, 'seed': 0}

R0 = 0x10159e38                 # the static small-block heap region
CUR = 0x1015be58
HANDLES = 0x10ac3720
NHANDLES = 0x10ac385c


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def abuf(ctx, n, align=0x1000):
    a = ctx.buf(n + align, bytes(n + align))
    return (a + align - 1) & ~(align - 1)


def cstr(ctx, data, extra=16):
    return ctx.buf(len(data) + 1 + extra, bytes(data) + b'\0' + rb(ctx, extra))


def wstr(ctx, chars, extra=8):
    return ctx.buf(2 * len(chars) + 2 + extra, struct.pack('<%dH' % len(chars), *chars) + b'\0\0' + rb(ctx, extra))


def mix(*vals):
    x = 0x9e3779b9
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def count_calls(m, name):
    return sum(1 for e in m.api_log if e[0] == name) - 1


def read_cstr(uc, addr, maxn=300):
    try:
        out = b''
        while len(out) < maxn:
            c = bytes(uc.mem_read(addr + len(out), 1))
            if c == b'\0':
                break
            out += c
        return out
    except Exception:
        return 'unreadable'


def stack_arg(uc, i):
    """Raw dword argument i of a Windows stub (the hook runs at the stub's first instruction)."""
    return r32(uc, uc.reg_read(UC_X86_REG_ESP) + 4 + 4 * i)


# ---- hooks ---------------------------------------------------------------------------------------
def install(m):
    if getattr(m, '_w7s', False):
        return
    m._w7s = True

    def make(key):
        def cb(uc, address, size, ud):
            if key not in ST['active']:
                return
            esp = uc.reg_read(UC_X86_REG_ESP)
            k = STUBS[key]
            raw = list(struct.unpack('<%dI' % k, bytes(uc.mem_read(esp + 4, 4 * k)))) if k else []
            args = []
            for i, v in enumerate(raw):
                if i in TEXT_ARGS.get(key, ()):
                    args.append(read_cstr(uc, v) if v else 0)
                else:
                    args.append('stack' if STACK_LO <= v <= STACK_HI else v)
            name = 'stub_%x' % key
            m.api_log.append((name, tuple(args)))
            n = count_calls(m, name)
            f = ST['fn'].get(key)
            val = f(n, raw, uc) if f else mix(ST['seed'], key, n)
            uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
            # return to the caller (cdecl: the caller removes the arguments)
            uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
            uc.reg_write(UC_X86_REG_ESP, esp + 4)
        return cb

    for a in STUBS:
        m.raw.hook_add(UC_HOOK_CODE, make(a), None, a, a)
    for sym, a in SYM_STUBS.items():
        addr = m.syms[sym]
        m.raw.hook_add(UC_HOOK_CODE, make(a), None, addr, addr)


def begin(ctx, *active):
    install(ctx.m)
    ST.clear()
    ST['active'] = set(active)
    ST['fn'] = {}
    ST['api'] = {}
    ST['seed'] = ctx.rng.getrandbits(32)
    ST['m'] = ctx.m
    ctx.m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    ctx.m.uc.reg_write(UC_X86_REG_FPSW, 0)
    ctx.m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)


def answers(addr, vals):
    ST['fn'][addr] = lambda n, raw, uc: vals[n % len(vals)]


def malloc_pool(ctx, size, p_zero=0.15, k=4):
    blocks = [0 if ctx.rng.random() < p_zero else zbuf(ctx, size) for _ in range(k)]
    answers(MALLOC, blocks)
    return blocks


def _api(name):
    def h(n, args, uc):
        v = ST.get('api', {}).get(name)
        if v is None:
            return 1
        k = count_calls(ST['m'], name)
        if callable(v):
            return v(k, args, uc)
        return v[k % len(v)]
    return h


API = {name: _api(name) for name in [
    'GetStringTypeA', 'GetStringTypeW', 'MultiByteToWideChar', 'WideCharToMultiByte', 'LCMapStringA',
    'LCMapStringW', 'CompareStringA', 'CompareStringW', 'GetCPInfo', 'GetLastError', 'GetStartupInfoA',
    'GetFileType', 'GetStdHandle', 'SetHandleCount', 'VirtualAlloc', 'GetModuleFileNameA',
    'GetEnvironmentStrings', 'GetEnvironmentStringsW', 'FreeEnvironmentStringsA', 'FreeEnvironmentStringsW',
    'WriteFile', 'GetACP', 'GetOEMCP']}


def seq(r, choices, k=4):
    return [r.choice(choices) for _ in range(k)]


# ---- streams -------------------------------------------------------------------------------------
def g_skipws(ctx):
    begin(ctx, 0x10105a60)
    r = ctx.rng
    w32(ctx, 0x1015965c, r.choice([1, 1, 1, 2]))
    text = bytes(r.choice(b'  \t\n\x0b\x0cab1\xa0\x85') for _ in range(r.randrange(0, 12)))
    data = cstr(ctx, text)
    s = newbuf(ctx, 0x20)
    w32(ctx, s, data)
    w32(ctx, s + 4, r.choice([len(text), len(text), r.randrange(0, 6), 0]))
    fills = [r.choice([0x20, 0x09, 0x41, 0xffffffff, 0x0a, 0x85]) for _ in range(r.randrange(0, 4))] + \
        [r.choice([0x41, 0xffffffff, 0x30])]
    answers(0x10105a60, fills)
    return [newbuf(ctx, 4), s]


MODES = [b'r', b'w', b'a', b'r+', b'w+b', b'a+t', b'rb', b'rt', b'wc', b'wn', b'rS', b'rR', b'rT', b'rD', b'r+b',
         b'rbt', b'r++', b'rcn', b'rSR', b'rTT', b'rDD', b'x', b'', b'r x', b'wb+c', b'a+bDTRn', b'rq+', b'w\x80+']


def g_openfile(ctx):
    begin(ctx, 0x101099a0)
    r = ctx.rng
    mode = r.choice(MODES)
    if r.random() < 0.3:
        mode = bytes([r.choice(b'rwa')]) + bytes(r.choice(b'+btcnSRTDx ') for _ in range(r.randrange(0, 6)))
    w32(ctx, 0x1017ec20, r.choice([0, 0x4000, ctx.u32()]))
    answers(0x101099a0, [r.choice([3, 0, 0xffffffff, 0x80000000, 0x7fffffff, 17])])
    return [cstr(ctx, b'file.txt'), cstr(ctx, mode), ctx.u32(), newbuf(ctx, 0x20)]


def g_getstream(ctx):
    begin(ctx, MALLOC)
    r = ctx.rng
    n = r.choice([0, 1, 3, 8, 20, 0xffffffff])
    cnt = n if n < 100 else 4
    ptrs = []
    for _ in range(cnt + 2):
        if r.random() < 0.2:
            ptrs.append(0)
        else:
            s = newbuf(ctx, 0x20)
            if r.random() < 0.7:
                wb(ctx, s + 0xc, [r.choice([0x01, 0x02, 0x80, 0x83, 0x10, 0x40, 0x00])])
            ptrs.append(s)
    tab = ctx.buf(4 * len(ptrs), struct.pack('<%dI' % len(ptrs), *ptrs))
    w32(ctx, 0x10ac4ba0, n)
    w32(ctx, 0x10ac3860, tab)
    malloc_pool(ctx, 0x20, 0.3)
    return []


def g_filbuf(ctx):
    begin(ctx, 0x10105b50, MALLOC)
    r = ctx.rng
    s = newbuf(ctx, 0x20)
    flags = r.choice([0x01, 0x80, 0x81, 0x82, 0x02, 0x41, 0x10, 0x11, 0x08, 0x09, 0x109, 0x401, 0x409, 0x04, 0x05, 0])
    flags |= ctx.u32() & r.choice([0, 0, 0xffff0000, 0x2020])
    w32(ctx, s + 0xc, flags)
    buf = newbuf(ctx, 0x40)
    w32(ctx, s + 8, buf if r.random() < 0.9 else 0)
    w32(ctx, s, buf + r.randrange(0, 8))
    w32(ctx, s + 0x18, r.choice([0x200, 0x200, 0x1000, 2]))
    nblk = 2
    blocks = []
    for i in range(64):
        if i < nblk:
            data = b''.join(struct.pack('<IBBH', ctx.u32(), r.choice([0x82, 0x80, 0x02, 0x83, 0]), 0, 0)
                            for _ in range(32))
            b = ctx.buf(0x100, data)
            blocks.append(b)
            w32(ctx, HANDLES + 4 * i, b)
        else:
            w32(ctx, HANDLES + 4 * i, 0)
    wb(ctx, 0x10159dc8 + 4, [r.choice([0x82, 0x80, 0x02, 0])])
    w32(ctx, s + 0x10, r.choice([0xffffffff, 0, 1, 2, r.randrange(0, 64)]))
    answers(0x10105b50, [r.choice([0, 0xffffffff, 1, 5, 0x200, 0x40])])
    malloc_pool(ctx, 0x1000, 0.3, 2)
    return [s]


def startup_reserved(ctx):
    r = ctx.rng
    n = r.choice([0, 1, 3, 32, 40, 70, 0x900, 0xffffffff])
    real = n if n < 0x900 else 0x800
    if n == 0xffffffff:
        real = 0
    flags = bytes(r.choice([0x01, 0x09, 0x81, 0x41, 0x00, 0x08]) for _ in range(real))
    handles = struct.pack('<%dI' % real, *[r.choice([0xffffffff, 0x100 + i, ctx.u32()]) for i in range(real)])
    return ctx.buf(4 + len(flags) + len(handles) + 8, struct.pack('<I', n) + flags + handles + bytes(8))


def g_ioinit(ctx):
    begin(ctx, MALLOC, 0x101009b0)
    r = ctx.rng
    blocks = [zbuf(ctx, 0x100) for _ in range(r.choice([1, 2, 3, 70]))]
    if r.random() < 0.3:
        blocks.insert(r.randrange(1, len(blocks) + 1), 0)
    if r.random() < 0.02:
        blocks[0] = 0
    answers(MALLOC, blocks + [0])
    info = bytearray(rb(ctx, 0x44))
    struct.pack_into('<HI', info, 0x32, r.choice([0, 8, 0x100, 0x100]), r.choice([0, startup_reserved(ctx),
                                                                                 startup_reserved(ctx)]))
    info = bytes(info)

    def gsi(k, args, uc):
        uc.mem_write(stack_arg(uc, 0), info)
        return 0
    ST['api']['GetStartupInfoA'] = gsi
    ST['api']['GetFileType'] = seq(r, [0, 1, 2, 3, 0x102], 6)
    ST['api']['GetStdHandle'] = seq(r, [0xffffffff, 0x44, 0x48], 3)
    return []


def g_osfhandle(ctx):
    begin(ctx)
    r = ctx.rng
    nblk = r.choice([0, 1, 2, 3])
    for i in range(64):
        if i < nblk:
            data = b''.join(struct.pack('<IBBH', ctx.u32(), r.choice([1, 0, 0x81, 0x40]), 0, 0) for _ in range(32))
            w32(ctx, HANDLES + 4 * i, ctx.buf(0x100, data))
        else:
            w32(ctx, HANDLES + 4 * i, 0)
    w32(ctx, NHANDLES, r.choice([nblk * 32, nblk * 32, nblk * 32, 0, max(nblk * 32 - 5, 0)]))
    return [r.choice([0, 1, 31, 32, 33, r.randrange(0, 100), ctx.u32()])]


def g_dosmaperr(ctx):
    begin(ctx)
    r = ctx.rng
    tab = [r32(ctx.m.uc, 0x101599e0 + 8 * i) for i in range(45)]
    return [r.choice([r.choice(tab), r.randrange(0x10, 0x28), r.randrange(0xb8, 0xd0), ctx.u32(), r.randrange(0, 300)])]


# ---- small-block heap ----------------------------------------------------------------------------
def ring(ctx, regions):
    k = len(regions)
    for i, reg in enumerate(regions):
        w32(ctx, reg, regions[(i + 1) % k])
        w32(ctx, reg + 4, regions[(i - 1) % k])


def page_map(ctx):
    r = ctx.rng
    m = bytearray(0xf0)
    pos = 0
    runs = []
    pfree = r.choice([0.2, 0.4, 0.6])
    while pos < 0xf0:
        rem = 0xf0 - pos
        if r.random() < pfree:
            n = min(rem, r.choice([1, 2, 3, 5, 8, 16, 40]))
            runs.append((pos, n))
            pos += n
        else:
            n = min(rem, r.choice([1, 2, 3, 4, 6, 10, 20]))
            m[pos] = n
            pos += n
    return bytes(m), runs


def g_sbh_alloc(ctx):
    begin(ctx, 0x10106660)
    r = ctx.rng
    nreg = r.choice([1, 1, 2, 3])
    regions = []
    use_r0 = r.random() < 0.7
    for i in range(nreg):
        reg = R0 if (i == 0 and use_r0) else zbuf(ctx, 0x2020)
        regions.append(reg)
        if r.random() < 0.15:
            w32(ctx, reg + 0x10, 0xffffffff)
            w32(ctx, reg + 0xc, 0)
            continue
        npg = r.choice([1, 2, 3])
        nmapped = npg + 22
        base = abuf(ctx, nmapped * 0x1000)
        w32(ctx, reg + 0x10, base)
        tab = b''
        for k in range(npg):
            pg = base + k * 0x1000
            mp, runs = page_map(ctx)
            wb(ctx, pg + 8, mp + b'\xff')
            pos = r.choice([p for p, _ in runs]) if runs else 0
            w32(ctx, pg, pg + 8 + pos)
            w32(ctx, pg + 4, r.choice([0, 1, 0x30]))
            tab += struct.pack('<II', r.choice([sum(n for _, n in runs), r.randrange(0, 0xf1)]),
                               r.choice([0xf1, 0xf1, 0x100, r.randrange(0, 0x20)]))
        nfree = r.choice([0, 1, 3, 16, 20])
        tab += struct.pack('<II', 0xffffffff, 0xf1) * nfree
        rest = 0x400 - npg - nfree
        tab += struct.pack('<II', r.choice([0xf0, 0x10, 0xffffffff]), 0xf1) * min(rest, 1)
        epg = base + (npg + nfree) * 0x1000      # that page as a committed empty one
        w32(ctx, epg, epg + 8, r.choice([0xf0, 0x10]))
        wb(ctx, epg + 0xf8, [0xff])
        tab += struct.pack('<II', 0xffffffff, 0xf1) * (rest - min(rest, 1))
        wb(ctx, reg + 0x18, tab)
        w32(ctx, reg + 8, reg + 0x18 + 8 * r.randrange(0, npg + 1))
        w32(ctx, reg + 0xc, r.choice([reg + 0x18 + 8 * npg, reg + 0x18 + 8 * npg, 0, reg + 0x18 + 8 * (npg + nfree)]))
    r.shuffle(regions)
    if R0 not in regions:
        regions.insert(0, R0)
        w32(ctx, R0 + 0x10, 0xffffffff)
    ring(ctx, regions)
    w32(ctx, CUR, r.choice(regions))
    # a new region from Sim_10106660
    if r.random() < 0.8:
        nr = zbuf(ctx, 0x2020)
        w32(ctx, nr + 0x10, abuf(ctx, 0x1000))
        w32(ctx, nr + 0x18, 0xf0)
    else:
        nr = 0
    answers(0x10106660, [nr])

    def valloc(k, args, uc):
        return args[0] if mix(ST['seed'], 7) % 5 else 0x1234000
    ST['api']['VirtualAlloc'] = valloc
    return [r.choice([1, 1, 2, 3, 4, 8, 0x10, 0x20, 0x40, 0xf0, 0xf1])]


# ---- locale wrappers -----------------------------------------------------------------------------
def g_gettype(ctx):
    begin(ctx, 0x10100580, FREE)
    r = ctx.rng
    w32(ctx, 0x1017e93c, r.choice([0, 0, 1, 1, 2, 2, 5]))
    w32(ctx, 0x1017e950, r.choice([0, 0x409]))
    w32(ctx, 0x1017e960, r.choice([0, 1252]))
    ST['api']['GetStringTypeA'] = seq(r, [0, 1, 1])
    ST['api']['GetStringTypeW'] = seq(r, [0, 1, 1])
    ST['api']['MultiByteToWideChar'] = seq(r, [0, 3, 5, 5])
    answers(0x10100580, [r.choice([0, zbuf(ctx, 0x40), zbuf(ctx, 0x40)])])
    return [r.choice([1, 2, 4]), cstr(ctx, b'abc'), r.choice([3, 0xffffffff, 0]), newbuf(ctx, 16),
            r.choice([0, 932]), r.choice([0, 0x411])]


def g_lcmap(ctx, upper_only=False):
    begin(ctx, MALLOC, FREE)
    r = ctx.rng
    w32(ctx, 0x1017e944, r.choice([0, 0, 1, 1, 2, 5]))
    w32(ctx, 0x1017e960, r.choice([0, 1252]))
    ST['api']['LCMapStringA'] = seq(r, [0, 4, 7, 7])
    ST['api']['LCMapStringW'] = seq(r, [0, 4, 7, 7, 30])
    ST['api']['MultiByteToWideChar'] = seq(r, [0, 4, 6, 6])
    ST['api']['WideCharToMultiByte'] = seq(r, [0, 3, 6, 6])
    malloc_pool(ctx, 0x80, 0.2)
    text = bytes(r.choice(b'abcXYZ\x81') for _ in range(r.randrange(0, 12)))
    flags = r.choice([0x100, 0x200, 0x400, 0x401, 0x1000])
    return [r.choice([0, 0x409]), flags, cstr(ctx, text), r.choice([0xffffffff, 0, 3, len(text), 40]),
            r.choice([0, newbuf(ctx, 0x40)]), r.choice([0, 0, 3, 7, 0x40]), r.choice([0, 0, 932])]


def g_strupr(ctx):
    begin(ctx, MALLOC, FREE)
    r = ctx.rng
    w32(ctx, 0x1017e950, r.choice([0, 0, 0x409]))
    w32(ctx, 0x1017e944, r.choice([2, 2, 1, 0]))
    text = bytes(r.choice(b'abcxyzABC09 {}`\xe1\x81') for _ in range(r.randrange(0, 20)))

    def lcmap_a(k, args, uc):
        if mix(ST['seed'], k) % 6 == 0:
            return 0
        src = read_cstr(uc, args[2])
        if args[4] and args[4] != 'stack':
            out = bytes(c - 32 if 0x61 <= c <= 0x7a else c for c in src) + b'\0'
            uc.mem_write(args[4], out[:max(args[5], 0)] if args[5] < 0x1000 else out)
        return len(src) + 1
    ST['api']['LCMapStringA'] = lcmap_a
    ST['api']['LCMapStringW'] = seq(r, [0, 5])
    ST['api']['MultiByteToWideChar'] = seq(r, [0, 5])
    ST['api']['WideCharToMultiByte'] = seq(r, [0, 5])
    malloc_pool(ctx, 0x40, 0.15)
    return [cstr(ctx, text)]


def cpinfo_bytes(r):
    maxc = r.choice([1, 2, 2, 2, 0, 3])
    pairs = r.choice([[], [(0x81, 0x9f), (0xe0, 0xfc)], [(0x81, 0xfe)], [(0xa1, 0xa1)], [(0x90, 0x80), (0x81, 0x82)],
                      [(0x81, 0x00)], [(0xfe, 0xff)]])
    lead = b''.join(bytes(p) for p in pairs)
    lead = (lead + bytes(12))[:12]
    return struct.pack('<I', maxc) + b'?\0' + lead + bytes(2)


def cpinfo_handler(r):
    data = cpinfo_bytes(r)
    rets = seq(r, [1, 1, 1, 0, 2], 2)

    def h(k, args, uc):
        uc.mem_write(stack_arg(uc, 1), data)
        return rets[k % 2]
    return h


def g_setmbcp(ctx, fixed=None):
    begin(ctx)
    r = ctx.rng
    known = [r32(ctx.m.uc, 0x1015be68 + 0x30 * i) for i in range(5)]
    cur = r.choice([0, 1252, r.choice(known)])
    w32(ctx, 0x1017ebc4, cur)
    w32(ctx, 0x1017ebdc, r.choice([0, 1]))
    w32(ctx, 0x1017e960, r.choice([1252, 932]))
    ST['api']['GetCPInfo'] = cpinfo_handler(r)
    acp = r.choice(known + [1252, 0, 437, cur])
    ST['api']['GetACP'] = [acp]
    ST['api']['GetOEMCP'] = [r.choice(known + [437, 0])]
    if fixed is not None:
        return []
    return [r.choice(known + [0xfffffffd, 0xfffffffe, 0xfffffffc, 0, 1252, 437, cur, ctx.u32()])]


def g_setmbcp3(ctx):
    return g_setmbcp(ctx, fixed=True)


def g_mbtowc(ctx):
    begin(ctx)
    r = ctx.rng
    tab = bytearray(rb(ctx, 0x204))
    for c in range(256):
        hi = tab[2 * c + 3] & 0x7f
        if 0x81 <= c <= 0x9f or c in (0xe0, 0xfc, 0x41) or r.random() < 0.05:
            hi |= 0x80
        tab[2 * c + 3] = hi
    t = ctx.buf(len(tab), bytes(tab))
    w32(ctx, 0x10159450, t + 2)
    w32(ctx, 0x1017e950, r.choice([0, 0x411, 0x411]))
    w32(ctx, 0x1017e960, r.choice([932, 0]))
    w32(ctx, 0x1015965c, r.choice([1, 2, 2, 3, 0, 0xffffffff]))
    ST['api']['MultiByteToWideChar'] = seq(r, [0, 1, 2])
    s = bytes(r.choice(b'\x81\x82\x9f\xe0A a\x00\xff') for _ in range(r.randrange(0, 4)))
    sp = 0 if r.random() < 0.05 else cstr(ctx, s)
    return [r.choice([0, newbuf(ctx, 4), newbuf(ctx, 4)]), sp, r.choice([0, 1, 2, 3, 0xffffffff, 0x80000000])]


def g_isspace(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1015965c, r.choice([1, 1, 1, 2, 0]))
    w32(ctx, 0x1017e950, r.choice([0, 0x409]))
    ST['api']['GetStringTypeA'] = seq(r, [0, 1])
    return [r.choice([r.randrange(0, 256), r.randrange(0, 256), 0xffffffff, 0x20, 9, 0x0d])]


def g_cmpstr(ctx):
    begin(ctx, MALLOC, FREE)
    r = ctx.rng
    w32(ctx, 0x1017ed14, r.choice([0, 0, 1, 1, 1, 2, 7]))
    w32(ctx, 0x1017e960, r.choice([0, 932]))
    ST['api']['CompareStringA'] = seq(r, [0, 1, 2, 3])
    ST['api']['CompareStringW'] = seq(r, [0, 1, 2, 3])
    ST['api']['MultiByteToWideChar'] = seq(r, [0, 3, 4, 4, 4])
    ST['api']['GetCPInfo'] = cpinfo_handler(r)
    malloc_pool(ctx, 0x40, 0.15)
    a = bytes(r.choice(b'ab\x81\x85\xe0\x00') for _ in range(r.randrange(0, 5)))
    b = bytes(r.choice(b'ab\x81\x85\xe0\x00') for _ in range(r.randrange(0, 5)))
    return [r.choice([0, 0x411]), r.choice([0, 1]), cstr(ctx, a), r.choice([0xffffffff, 0, 0, 1, 2, 5]),
            cstr(ctx, b), r.choice([0xffffffff, 0, 0, 1, 2, 5]), r.choice([0, 932])]


def g_wcstombs(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1017e950, r.choice([0, 0x411, 0x411]))
    w32(ctx, 0x1017e960, r.choice([932, 0]))
    mb = r.choice([1, 1, 2, 2, 3, 4])
    w32(ctx, 0x1015965c, mb)
    chars = [r.choice([0x41, 0x62, 0xe9, 0x100, 0x3042, 0x7f, 0xff]) for _ in range(r.randrange(0, 8))]
    pw = wstr(ctx, chars)
    s = 0 if r.random() < 0.2 else newbuf(ctx, 0x40)
    used = r.choice([0, 0, 0, 1])
    rets = seq(r, [0, 0, 1, 2, 3, 5], 6)
    data = rb(ctx, 8)
    if r.random() < 0.5:
        data = bytes([data[0] | 1, 0]) + data[2:]

    def wctmb(k, args, uc):
        ret = rets[k % len(rets)]
        cb = stack_arg(uc, 5)
        if args[4] == 'stack':
            ret = min(ret, cb)
            uc.mem_write(stack_arg(uc, 4), data[:ret])
        elif args[4] and cb:
            ret = min(ret, cb, 0x40)
            uc.mem_write(args[4], data[:ret])
        up = stack_arg(uc, 7)
        if up:
            uc.mem_write(up, struct.pack('<I', used if k % 3 == 2 else 0))
        return ret
    ST['api']['WideCharToMultiByte'] = wctmb
    ST['api']['GetLastError'] = seq(r, [0x7a, 0x7a, 0])
    return [s, pw, r.choice([0, 1, 2, 3, 5, 8, 0x40])]


# ---- environment, command line, messages ---------------------------------------------------------
def g_wtomb_env(ctx):
    begin(ctx, MALLOC, 0x1010b2a0)
    r = ctx.rng
    n = r.choice([0, 1, 2, 4])
    ptrs = [wstr(ctx, [0x41 + i, 0x3d, 0x31]) for i in range(n)] + [0]
    w32(ctx, 0x1017e8b8, ctx.buf(4 * len(ptrs), struct.pack('<%dI' % len(ptrs), *ptrs)))
    rets = seq(r, [0, 4, 4, 4, 4], 8)

    def wctmb(k, args, uc):
        ret = rets[k % len(rets)]
        if args[4] and args[4] != 'stack' and ret:
            uc.mem_write(args[4], (b'A=%d' % k)[:3] + b'\0')
        return ret
    ST['api']['WideCharToMultiByte'] = wctmb
    malloc_pool(ctx, 0x10, 0.1)
    return []


def env_block(ctx):
    r = ctx.rng
    items = [r.choice([b'A=1', b'=C:=C:\\', b'PATH=x;y', b'=', b'B=', b'LONGNAME=' + b'v' * 20])
             for _ in range(r.randrange(0, 6))]
    return ctx.buf(sum(len(i) + 1 for i in items) + 1 + 8, b''.join(i + b'\0' for i in items) + b'\0' + bytes(8))


def g_setenvp(ctx):
    begin(ctx, MALLOC, FREE, 0x101009b0)
    r = ctx.rng
    w32(ctx, 0x1017e8f4, env_block(ctx))
    blocks = [zbuf(ctx, 0x40) for _ in range(8)]
    if r.random() < 0.05:
        blocks[r.randrange(0, 8)] = 0
    answers(MALLOC, blocks)
    return []


def g_setargv(ctx):
    begin(ctx, MALLOC, 0x101009b0)
    r = ctx.rng
    line = r.choice([b'', b'prog', b'"my prog.exe" a b', b'x "a b" c\\"d', b'a  b\tc', b'"unterminated'])
    w32(ctx, 0x10ac4ba4, cstr(ctx, line))
    name = r.choice([b'', b'C:\\GAME\\HG.EXE', b'"q" r'])

    def gmfn(k, args, uc):
        uc.mem_write(stack_arg(uc, 1), name + b'\0')
        return len(name)
    ST['api']['GetModuleFileNameA'] = gmfn
    answers(MALLOC, [0 if r.random() < 0.03 else zbuf(ctx, 0x200)])
    return []


def g_envstrings(ctx):
    begin(ctx, MALLOC, FREE)
    r = ctx.rng
    w32(ctx, 0x1017ebe4, r.choice([0, 0, 1, 2, 3]))
    items = [r.choice([b'A=1', b'PATH=x', b'B=2', b'=C:=C:\\']) for _ in range(r.randrange(0, 4))]
    ablock = b''.join(i + b'\0' for i in items) + b'\0'
    if not items:
        ablock = b'\0\0'
    wblock = b''.join(struct.pack('<%dH' % len(i), *i) + b'\0\0' for i in items) + b'\0\0'
    if not items:
        wblock = b'\0\0\0\0'
    wp = ctx.buf(len(wblock) + 8, wblock + bytes(8))
    ap = ctx.buf(len(ablock) + 8, ablock + bytes(8))
    ST['api']['GetEnvironmentStringsW'] = [r.choice([0, wp, wp])]
    ST['api']['GetEnvironmentStrings'] = [r.choice([0, ap, ap])]
    size = len(ablock)
    rets = [r.choice([0, size, size]), r.choice([0, size, size])]

    def wctmb(k, args, uc):
        ret = rets[k % 2]
        if args[4] and args[4] != 'stack' and ret:
            uc.mem_write(args[4], ablock[:ret])
        return ret
    ST['api']['WideCharToMultiByte'] = wctmb
    malloc_pool(ctx, 0x80, 0.15)
    return []


def g_nmsg(ctx):
    begin(ctx, 0x10107840)
    r = ctx.rng
    codes = [r32(ctx.m.uc, 0x1015bf58 + 8 * i) for i in range(18)]
    code = r.choice(codes + codes + [0xfc, 0xff, 3, ctx.u32()] + ([0x400] if r.random() < 0.05 else []))
    w32(ctx, 0x1017e8fc, r.choice([0, 0, 1, 2]))
    w32(ctx, 0x1017e900, r.choice([0, 1]))
    if r.random() < 0.5:
        t = newbuf(ctx, 0x20)
        w32(ctx, t + 0x10, r.choice([0xffffffff, 0x99]))
        w32(ctx, HANDLES, t)
    name = r.choice([b'C:\\HG\\HG.EXE', b'', b'C:\\' + b'd' * 70 + b'\\GAME.EXE', b'x' * 59, b'x' * 58, b'y' * 60])
    ret = r.choice([len(name), len(name), 0])

    def gmfn(k, args, uc):
        uc.mem_write(stack_arg(uc, 1), name + b'\0')
        return ret
    ST['api']['GetModuleFileNameA'] = gmfn
    ST['api']['GetStdHandle'] = [r.choice([0x44, 0xffffffff])]
    return [code]


def g_copyenv(ctx):
    begin(ctx, MALLOC, 0x101009b0)
    r = ctx.rng
    n = r.choice([0, 1, 2, 5])
    ptrs = [cstr(ctx, r.choice([b'A=1', b'', b'LONG=' + b'z' * 30])) for _ in range(n)] + [0]
    lst = 0 if r.random() < 0.05 else ctx.buf(4 * len(ptrs), struct.pack('<%dI' % len(ptrs), *ptrs))
    blocks = [zbuf(ctx, 0x40) for _ in range(7)]
    if r.random() < 0.2:
        blocks[r.randrange(1, 7) if r.random() < 0.85 else 0] = 0
    answers(MALLOC, blocks)
    return [lst]


# ---- math and time -------------------------------------------------------------------------------
INF, NINF = 0x7ff0000000000000, 0xfff0000000000000


def dbits(v):
    return struct.unpack('<Q', struct.pack('<d', v))[0]


def rand_double(r):
    k = r.random()
    if k < 0.5:
        return dbits(r.choice([0.0, -0.0, 1.0, -1.0, 2.0, -2.0, 3.0, -3.0, 0.5, -0.5, 4.0, -5.0, 1e300, -1e300,
                                5e-324, -5e-324, 2.0 ** 53, 2.0 ** 53 + 2, -(2.0 ** 53), 2.0 ** 52 + 1,
                                -(2.0 ** 52 + 1), 1.0000000000000002, 0.9999999999999999, -0.9999999999999999,
                                -1.0000000000000002, 7.0, -7.0, 2.0 ** 1023, 1e15 + 1, 2.0 ** 30 + 1,
                                -(2.0 ** 30 + 1), 6.0, -6.0]))
    if k < 0.7:
        return r.choice([0x7ff8000000000000, 0xfff8000000000000, 0x7ff0000000000001, 0xfff4000000000000,
                         0x7fffffffffffffff, 0x0000000000000001, 0x8000000000000001, 0x000fffffffffffff])
    return r.getrandbits(64)


def g_pow(ctx):
    begin(ctx)
    r = ctx.rng
    # the x87 precision / rounding control the caller may have set (Direct3D sets single precision)
    ctx.m.uc.reg_write(UC_X86_REG_FPCW, r.choice([0x27f, 0x27f, 0x07f, 0x37f, 0x67f, 0xa7f, 0xe7f]))
    x = rand_double(r)
    y = rand_double(r)
    k = r.random()
    if k < 0.3:
        y = r.choice([INF, NINF])
    elif k < 0.6:
        x = r.choice([INF, NINF])
    elif k < 0.7:
        x, y = r.choice([INF, NINF]), r.choice([INF, NINF])
    # 0x10159988 is the constant +infinity (nothing writes it); other values but signalling NaNs are tried too
    if r.random() < 0.15:
        w32(ctx, 0x10159988, *struct.unpack('<II', struct.pack('<Q', r.choice(
            [0x7ff8000000000000, 0xfff8000000000001, 0x0000000000000001, 0x8000000000000001, 0x3ff0000000000000,
             r.getrandbits(64) & 0xfff7ffffffffffff | 0x7ff8000000000000, r.getrandbits(52)]))))
    if r.random() < 0.1:
        w32(ctx, 0x101599a8, ctx.u32(), ctx.u32())
    res = newbuf(ctx, 8)
    return [x & 0xffffffff, x >> 32, y & 0xffffffff, y >> 32, res]


def g_controlfp(ctx):
    begin(ctx)
    r = ctx.rng
    ctx.m.uc.reg_write(UC_X86_REG_FPCW, r.choice([0x27f, 0x37f, 0x7f, 0xe7f, 0x1272]))
    return [ctx.u32(), r.choice([0, 0xffffffff, 0x80000, 0x30000, 0x300, 0x8031f, ctx.u32()])]


def systemtime(ctx, addr, day_in_month):
    r = ctx.rng
    vals = [0 if day_in_month else r.choice([1998, 0x7cf, 1]), r.choice([3, 4, 10, 11, 0, 13]), r.randrange(0, 7),
            r.choice([1, 2, 5, 15, 31]) if not day_in_month else r.choice([1, 2, 5]), r.choice([0, 1, 2, 3, 23]),
            r.choice([0, 30, 59]), r.choice([0, 59]), r.choice([0, 999])]
    wb(ctx, addr, struct.pack('<8H', *vals))


def g_isindst(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1015c024, r.choice([0, 1, 1, 1]))
    year = r.choice([70, 96, 97, 100, 104, 137, r.randrange(70, 139)])
    tm = newbuf(ctx, 0x24)
    w32(ctx, tm + 0x14, year)
    w32(ctx, tm + 8, r.choice([0, 1, 2, 3, 12, 23, ctx.u32()]))
    w32(ctx, tm + 4, r.choice([0, 30, 59, ctx.u32()]))
    w32(ctx, tm, r.choice([0, 59, ctx.u32()]))
    w32(ctx, 0x1015c028, r.choice([0, 3600, -3600, 18000]))
    w32(ctx, 0x1017ec28, r.choice([0, 1, 1]))
    systemtime(ctx, 0x1017ecc8, r.random() < 0.7)
    systemtime(ctx, 0x1017ec74, r.random() < 0.7)
    same = r.random() < 0.5
    if same:
        w32(ctx, 0x1015c0b8, year)
        w32(ctx, 0x1015c0c8, year)
        s, e = r.randrange(0, 366), r.randrange(0, 366)
        w32(ctx, 0x1015c0bc, s)
        w32(ctx, 0x1015c0cc, e)
        w32(ctx, 0x1015c0c0, r.choice([0, 7200000, 3600000, 86400000]))
        w32(ctx, 0x1015c0d0, r.choice([0, 7200000, 3600000]))
        yday = r.choice([s, e, s - 1, s + 1, e - 1, e + 1, r.randrange(0, 366)])
    else:
        w32(ctx, 0x1015c0b8, r.choice([year, year + 1]))
        w32(ctx, 0x1015c0c8, r.choice([year + 1, year]))
        yday = r.choice([r.randrange(0, 366), r.randrange(85, 100), r.randrange(295, 310)])
    w32(ctx, tm + 0x1c, yday)
    return [tm]


SPECS = [
    dict(name='Sim_10104970', orig=0x10104970, c='Sim_10104970', gen=g_skipws, trials=600, api=API),
    dict(name='Sim_10104be0', orig=0x10104be0, c='Sim_10104be0', gen=g_openfile, trials=800, api=API),
    dict(name='Sim_10104db0', orig=0x10104db0, c='Sim_10104db0', gen=g_getstream, trials=500, api=API),
    dict(name='Sim_10104fd0', orig=0x10104fd0, c='Sim_10104fd0', gen=g_gettype, trials=600, api=API),
    dict(name='Sim_10105100', orig=0x10105100, c='Sim_10105100', gen=g_dosmaperr, trials=400, api=API, void=True),
    dict(name='Sim_101053c0', orig=0x101053c0, c='Sim_101053c0', gen=g_lcmap, trials=1200, api=API),
    dict(name='Sim_10105a60', orig=0x10105a60, c='Sim_10105a60', gen=g_filbuf, trials=800, api=API),
    dict(name='Sim_10105db0', orig=0x10105db0, c='Sim_10105db0', gen=g_ioinit, trials=600, api=API),
    dict(name='Sim_101062b0', orig=0x101062b0, c='Sim_101062b0', gen=g_pow, trials=2000, api=API),
    dict(name='Sim_101069c0', orig=0x101069c0, c='Sim_101069c0', gen=g_sbh_alloc, trials=1500, api=API),
    dict(name='Sim_10107910', orig=0x10107910, c='Sim_10107910', gen=g_wtomb_env, trials=500, api=API),
    dict(name='Sim_10107990', orig=0x10107990, c='Sim_10107990', gen=g_setenvp, trials=500, api=API, void=True),
    dict(name='Sim_10107a80', orig=0x10107a80, c='Sim_10107a80', gen=g_setargv, trials=400, api=API),
    dict(name='Sim_10107d30', orig=0x10107d30, c='Sim_10107d30', gen=g_setmbcp, trials=1000, api=API),
    dict(name='Sim_10108010', orig=0x10108010, c='Sim_10108010', gen=g_setmbcp3, trials=400, api=API),
    dict(name='Sim_10108220', orig=0x10108220, c='Sim_10108220', gen=g_envstrings, trials=600, api=API),
    dict(name='Sim_10108430', orig=0x10108430, c='Sim_10108430', gen=g_nmsg, trials=800, api=API, void=True),
    dict(name='Sim_101086c0', orig=0x101086c0, c='Sim_101086c0', gen=g_controlfp, trials=300, api=API),
    dict(name='Sim_10109200', orig=0x10109200, c='Sim_10109200', gen=g_mbtowc, trials=1000, api=API),
    dict(name='Sim_101093f0', orig=0x101093f0, c='Sim_101093f0', gen=g_isspace, trials=400, api=API),
    dict(name='Sim_10109890', orig=0x10109890, c='Sim_10109890', gen=g_osfhandle, trials=400, api=API),
    dict(name='Sim_1010a0a0', orig=0x1010a0a0, c='Sim_1010a0a0', gen=g_isindst, trials=1500, api=API),
    dict(name='Sim_1010afa0', orig=0x1010afa0, c='Sim_1010afa0', gen=g_cmpstr, trials=1200, api=API),
    dict(name='Sim_1010b530', orig=0x1010b530, c='Sim_1010b530', gen=g_copyenv, trials=400, api=API),
    dict(name='Sim_1010c4b0', orig=0x1010c4b0, c='Sim_1010c4b0', gen=g_wcstombs, trials=1500, api=API),
    dict(name='Crt_StrUpper', orig=0x1010d180, c='Crt_StrUpper', gen=g_strupr, trials=600, api=API),
]
