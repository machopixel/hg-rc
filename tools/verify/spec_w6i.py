# Differential tests for round 6, letter i (see difftest.py and docs/checker.md): C runtime code of the DLL
# around 0x100fd000..0x10102000.
#
# Helpers that are not converted yet are replaced in the emulator by stubs: a code hook at the helper's own
# address (the same for the original and the C run) appends (address, arguments, extra words) to a log in
# argument memory (compared like all memory), answers with a value that depends only on the trial's seed,
# the helper and the position in the log, and returns to the caller by setting EIP/ESP (code bytes are not
# patched). Function pointers that the functions call (compare functions, the exit/init tables, the
# address GetProcAddress hands out) point to stubs of the same kind in argument memory. Windows imports are
# stubbed by the checker; handlers below give varied answers and write the out structures.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG

SOURCES = [
    'sim/unsorted_100f/w6i_mem.c', 'sim/unsorted_100f/w6i_stream.c', 'sim/unsorted_100f/w6i_misc.c',
    'sim/unsorted_100f/w6i_exit.c', 'sim/unsorted_1010/w6i_crt.c', 'sim/unsorted_1010/w6i_fdiv.c',
    # existing converted code the functions above call
    'sim/unsorted_100f/w2h_mem.c', 'sim/unsorted_1010/w2h_misc.c', 'sim/unsorted_1010/w2i_crt.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

LOG = 0x00AF8000        # count of dwords, then the logged dwords
LOG_MAX = 0x1800
CODE = 0x00AF2000       # stub addresses for function pointers (a `ret` byte, the hook does the work)
CMP = CODE              # compare(p1, p2): compares the signed dwords at p1 and p2
CB = [CODE + 0x10 * (i + 1) for i in range(4)]      # void (*)(void) callbacks; CB[3] also moves the at-exit start
FEAT = CODE + 0x60      # IsProcessorFeaturePresent (stdcall, 1 argument)
ARENA = 0x00A40000      # a block area for the small block heap world (page aligned, away from ctx.buf)

STATE = {'seed': 0}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def s32(v):
    v &= 0xffffffff
    return v - (1 << 32) if v >= 1 << 31 else v


def rnd(ctx, n):
    return bytes(ctx.rng.getrandbits(8) for _ in range(n))


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def pick(h, options):
    return options[h % len(options)]


# ---- stub answers: (uc, m, h, args) -> (eax, extra dwords for the log) --------------------------------
def p_fold(uc, m, h, args):
    c = args[0] & 0xff
    if 0x41 <= c <= 0x5a:
        c += 0x20
    if h % 9 == 0:
        c = (h >> 8) & 0xff
    hi = (h & 0xffffff00) if h % 4 == 0 else 0
    return hi | c, []


def p_cmp(uc, m, h, args):
    a = s32(r32(uc, args[0]))
    b = s32(r32(uc, args[1]))
    if a > b:
        v = pick(h, [1, 1, 7, 0x7fffffff, 0x100])
    elif a < b:
        v = pick(h, [-1, -1, -9, 0x80000000, 0xffffff00])
    else:
        v = 0
    return v, []


def p_cb(i):
    def p(uc, m, h, args):
        extra = [r32(uc, 0x1017e8cc), bytes(uc.mem_read(0x1017e8c8, 1))[0], r32(uc, 0x1017e8d0), r32(uc, 0x10ac4bc8)]
        if i == 3 and h % 2 == 0:
            m.uc.mem_write(0x10ac4bc8, struct.pack('<I', (r32(uc, 0x10ac4bc8) + 8) & 0xffffffff))
        return h, extra
    return p


def p_va(uc, m, h, args):
    return h, list(struct.unpack('<4I', bytes(uc.mem_read(args[2], 16))))


POLICY = {
    0x100fe240: lambda uc, m, h, a: (pick(h, [0xffffffff, 0, 1, 0xffffffff, 0x12345]), []),
    0x10103ba0: p_va,
    0x10105a60: lambda uc, m, h, a: (pick(h, [0xffffffff, 0x0a, 0x10a, 0x41, 0x61, h, 0xffffff0a, 0, 0x20, 0x7a]), []),
    0x101069c0: lambda uc, m, h, a: (pick(h, [0, 0, 0x00A80000, h]), []),
    0x100ff280: p_fold,
    0x101071e0: lambda uc, m, h, a: (pick(h, [0, STATE.get('tm', 0), STATE.get('tm', 0)]), []),
    0x10106e50: lambda uc, m, h, a: (pick(h, [0xffffffff, 0, 1, 0xfffffffb, 0x80000000, 0x7fffffff]), []),
    0x10105fe0: lambda uc, m, h, a: (pick(h, [0xffffffff, 0, 5, 5, 1000, h]), []),
    0x10100be0: lambda uc, m, h, a: (h, [bytes(uc.mem_read(0x1017e908, 1))[0]]),
    0x100ff030: lambda uc, m, h, a: (pick(h, [0xffffffff, a[0] & 0xff, h, 0, 0xffffffff]), []),
    CMP: p_cmp,
    FEAT: lambda uc, m, h, a: (pick(h, [0, 1, h]), []),
}
for _i in range(4):
    POLICY[CB[_i]] = p_cb(_i)

# address -> (argument words, extra bytes the callee removes from the stack)
STUBS = {
    0x100fe240: (1, 0), 0x10103ba0: (3, 0), 0x10105970: (7, 0), 0x10105a60: (1, 0), 0x100fef50: (2, 0),
    0x101069c0: (1, 0), 0x10106960: (3, 0), 0x100ff280: (1, 0), 0x101071e0: (1, 0), 0x100ffe20: (4, 0),
    0x10106e50: (0, 0), 0x10105fe0: (3, 0), 0x101086c0: (2, 0), 0x10100be0: (4, 0), 0x100ff030: (2, 0),
    CMP: (2, 0), CB[0]: (0, 0), CB[1]: (0, 0), CB[2]: (0, 0), CB[3]: (0, 0), FEAT: (1, 4),
}


def install(m):
    if getattr(m, '_w6i', False):
        return
    m._w6i = True

    def make(addr, nargs, pop):
        def cb(uc, address, size, ud):
            esp = uc.reg_read(UC_X86_REG_ESP)
            args = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
            n = r32(uc, LOG)
            h = mix(STATE['seed'], addr, n)
            pol = POLICY.get(addr)
            val, extra = pol(uc, m, h, args) if pol else (h, [])
            entry = [addr] + args + extra
            if n + len(entry) < LOG_MAX:
                uc.mem_write(LOG + 4 + 4 * n, struct.pack('<%dI' % len(entry), *[v & 0xffffffff for v in entry]))
                uc.mem_write(LOG, struct.pack('<I', n + len(entry)))
            uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
            uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
            uc.reg_write(UC_X86_REG_ESP, esp + 4 + pop)
        return cb

    for addr, (nargs, pop) in STUBS.items():
        m.raw.hook_add(UC_HOOK_CODE, make(addr, nargs, pop), None, addr, addr)


def begin(ctx):
    m = ctx.m
    install(m)
    STATE['seed'] = ctx.rng.getrandbits(32)
    STATE['tm'] = 0
    uc = m.uc
    uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    uc.reg_write(UC_X86_REG_FPSW, 0)
    uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    uc.mem_write(LOG, bytes(8))
    uc.mem_write(CODE, b'\xc3' * 0x80)
    return ctx.rng


# ---- Windows answers ------------------------------------------------------------------------------
def api_pick(options):
    return lambda n, args: pick(mix(STATE['seed'], n, 77), options)


def h_local_ft(n, args, uc):
    esp = uc.reg_read(UC_X86_REG_ESP)
    src, dst = r32(uc, esp + 4), r32(uc, esp + 8)
    lo, hi = struct.unpack('<2I', bytes(uc.mem_read(src, 8)))
    uc.mem_write(dst, struct.pack('<2I', mix(lo, 1), mix(hi, 2)))
    return pick(mix(STATE['seed'], n, 5), [1, 1, 1, 0, 2])


def h_system_t(n, args, uc):
    esp = uc.reg_read(UC_X86_REG_ESP)
    src, dst = r32(uc, esp + 4), r32(uc, esp + 8)
    lo, hi = struct.unpack('<2I', bytes(uc.mem_read(src, 8)))
    words = [mix(lo, hi, k) & (0xffff if mix(lo, k) % 3 else 0x3f) for k in range(8)]
    uc.mem_write(dst, struct.pack('<8H', *words))
    return pick(mix(STATE['seed'], n, 6), [1, 1, 1, 0, 0x100])


# ---- generators -----------------------------------------------------------------------------------
def g_sort(ctx):
    r = begin(ctx)
    width = r.choice([1, 2, 4, 4, 5, 8, 12])
    count = r.choice([0, 1, 2, 3, r.randrange(0, 14), r.randrange(0, 14)])
    vals = [r.choice([0, 1, 2, 3, -1, 0x7fffffff, -0x80000000, r.getrandbits(32)]) for _ in range(count)]
    data = bytearray(rnd(ctx, count * width + 8))
    for i, v in enumerate(vals):
        if width >= 4:
            data[i * width:i * width + 4] = struct.pack('<I', v & 0xffffffff)
    lo = ctx.buf(len(data), bytes(data))
    if count == 0:
        hi = r.choice([lo, lo - width, lo - 1])
    else:
        hi = lo + (count - 1) * width
    return [lo, hi, width, CMP]


def g_copy(ctx):
    r = begin(ctx)
    size = 0x300
    area = ctx.buf(size)
    n = r.choice([0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 31, 32, 33, 64, r.randrange(0, 0x100), r.randrange(0, 0x100)])
    src = area + r.randrange(0, size - n)
    if r.random() < 0.5:
        dst = src + r.randrange(-min(n + 3, src - area), min(n + 4, area + size - n - src))     # overlapping or near
    else:
        dst = area + r.randrange(0, size - n)
    return [dst, src, n]


def g_streams(ctx):
    r = begin(ctx)
    count = r.choice([0, 1, 2, 3, 5, 8, -1])
    table = ctx.buf(4 * 10)
    for i in range(10):
        if r.random() < 0.25:
            w32(ctx, table + 4 * i, 0)
        else:
            f = ctx.buf(0x20)
            w32(ctx, f + 0xc, r.choice([0, 1, 2, 0x80, 0x83, 0x40, 0x102, 0x100, r.getrandbits(32)]))
            w32(ctx, table + 4 * i, f)
    w32(ctx, 0x10ac3860, table)
    w32(ctx, 0x10ac4ba0, count)
    return [r.choice([0, 1, 0, 1, 2, 0xffffffff])]


def g_va(ctx):
    begin(ctx)
    return [ctx.u32() for _ in range(6)]


def g_findclose(ctx):
    begin(ctx)
    return [ctx.u32()]


def g_filetime(ctx):
    r = begin(ctx)
    lo = r.choice([0, 0, r.getrandbits(32)])
    hi = r.choice([0, 0, r.getrandbits(32), 0x01c00000])
    return [ctx.buf(8, struct.pack('<2I', lo, hi))]


def g_readline(ctx):
    r = begin(ctx)
    buf = ctx.buf(0x60)
    n = r.choice([-1, 0, 1, 2, 3, 0x80000000, r.randrange(0, 40), r.randrange(0, 40)])
    text = bytes(r.choice(b'ab\n\n\x00x\xff') for _ in range(48))
    data = ctx.buf(len(text), text)
    f = ctx.buf(0x20)
    w32(ctx, f, data)
    w32(ctx, f + 4, r.choice([0, 1, 2, 5, 40, 0x80000000, 0xffffffff, r.randrange(0, 30)]))
    return [buf, n, f]


def g_malloc(ctx):
    begin(ctx)
    w32(ctx, 0x1017e968, ctx.u32())
    return [ctx.u32()]


def g_alloc(ctx):
    r = begin(ctx)
    limit = r.choice([0, 0x1e0, 0x3f0, 0x1000, r.getrandbits(32)])
    w32(ctx, 0x1015be5c, limit)
    size = r.choice([0, 1, 15, 16, 17, limit, limit + 1, limit - 15, limit + 15, 0xfffffff8, 0xfffffff1, r.getrandbits(32), r.randrange(0, 0x800)])
    return [size & 0xffffffff]


def g_free(ctx):
    r = begin(ctx)
    head = 0x10159e38
    lo, hi = ARENA - r.choice([0, 1, 0x10]), ARENA + 0x4000
    second = ctx.buf(0x20)
    if r.random() < 0.5:     # the head covers the area
        w32(ctx, head + 0x10, lo, hi)
        w32(ctx, head, r.choice([head, second]))
        w32(ctx, second + 0x10, 0, 0)
    else:                    # a second node covers it
        w32(ctx, head + 0x10, 0, 0)
        w32(ctx, head, second)
        w32(ctx, second + 0x10, lo, hi)
    w32(ctx, second, head)
    page = ARENA + 0x1000 * r.randrange(0, 4)
    block = r.choice([0, page + 0x100 + 16 * r.randrange(0, 0xf0), page + 0x100 + 16 * r.randrange(0, 0xf0),
                      page + r.randrange(0, 0x100) & ~0xf, page + 0x108, ARENA + 0x5000, ctx.u32()])
    return [block & 0xffffffff]


def g_stricmp(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017e950, r.choice([0, 0, 1, 0x409]))
    alpha = b'aAbBzZ@[`{\xe0\xc0_'
    base = bytes(r.choice(alpha) for _ in range(r.randrange(0, 10)))

    def variant(s):
        out = bytearray(s)
        for i in range(len(out)):
            if r.random() < 0.3 and 0x41 <= (out[i] | 0x20) - 0x20 <= 0x5a:
                out[i] ^= 0x20
            elif r.random() < 0.05:
                out[i] = r.choice(alpha)
        if r.random() < 0.2:
            out += bytes(r.choice(alpha) for _ in range(r.randrange(1, 3)))
        if r.random() < 0.1 and out:
            out = out[:-1]
        return bytes(out)
    a = variant(base)
    b = variant(base)
    return [ctx.buf(len(a) + 8, a + b'\0' + rnd(ctx, 7)), ctx.buf(len(b) + 8, b + b'\0' + rnd(ctx, 7))]


def fill_table(ctx, start, end, density=0.1):
    r = ctx.rng
    vals = [r.choice(CB) if r.random() < density else 0 for _ in range((end - start) // 4)]
    w32(ctx, start, *vals)


def g_calltable(ctx):
    r = begin(ctx)
    k = r.randrange(0, 10)
    table = ctx.buf(4 * k + 8)
    fill_table(ctx, table, table + 4 * k + 8, 0.6)
    end = r.choice([table + 4 * k, table + 4 * k, table + 4 * k + 2, table - 4, table])
    return [table, end]


def g_init(ctx):
    r = begin(ctx)
    w32(ctx, 0x101593cc, r.choice([0, CB[0], CB[2]]))
    fill_table(ctx, 0x1012d208, 0x1012d410, 0.05)
    fill_table(ctx, 0x1012d000, 0x1012d104, 0.1)
    return []


def exit_world(ctx):
    r = ctx.rng
    w32(ctx, 0x1017e8d0, r.choice([0, 1, 1, 2]))
    w32(ctx, 0x1017e8cc, r.choice([0, 5]))
    k = r.randrange(0, 8)
    lst = ctx.buf(4 * k + 16)
    fill_table(ctx, lst, lst + 4 * k + 16, 0.6)
    start = lst + 8
    w32(ctx, 0x10ac4bc8, r.choice([0, start, start, start]))
    w32(ctx, 0x10ac4bb8, start + 4 * k if r.random() < 0.9 else start)
    fill_table(ctx, 0x1012d514, 0x1012d724, 0.05)
    fill_table(ctx, 0x1012d828, 0x1012d92c, 0.1)


def g_doexit(ctx):
    r = begin(ctx)
    exit_world(ctx)
    return [ctx.u32(), r.choice([0, 1, 0, 2, 0x100]), r.choice([0, 1, 0, 0x100, 0x201])]


def g_exit1(ctx):
    begin(ctx)
    exit_world(ctx)
    return [ctx.u32()]


def g_exit0(ctx):
    begin(ctx)
    exit_world(ctx)
    return []


def g_ctime(ctx):
    r = begin(ctx)
    tm = ctx.buf(0x24, struct.pack('<9I', r.randrange(0, 60), r.randrange(0, 60), r.randrange(0, 24),
                                   r.randrange(1, 32), r.randrange(0, 12), r.randrange(70, 140),
                                   r.randrange(0, 7), r.randrange(0, 366), r.randrange(0, 2)))
    STATE['tm'] = tm
    return [ctx.buf(8)]


def g_itoa(ctx):
    r = begin(ctx)
    return [ctx.u32(), ctx.u32(), r.choice([10, 10, 16, 2, 8, 36, 0, 0xfffffff6, r.getrandbits(32)])]


def g_none(ctx):
    begin(ctx)
    return []


def g_bsearch(ctx):
    r = begin(ctx)
    width = r.choice([4, 8, 12])
    num = r.choice([0, 1, 2, 3, 4, 5, 7, 8, r.randrange(0, 24), r.randrange(0, 24)])
    vals = sorted(r.sample(range(-50, 50), num))
    data = bytearray(rnd(ctx, num * width + 4))
    for i, v in enumerate(vals):
        data[i * width:i * width + 4] = struct.pack('<i', v)
    base = ctx.buf(len(data), bytes(data))
    key = r.choice(vals) if vals and r.random() < 0.6 else r.randrange(-55, 55)
    return [ctx.buf(4, struct.pack('<i', key)), base, num, width, CMP]


def g_filelen(ctx):
    r = begin(ctx)
    w32(ctx, 0x10ac385c, r.choice([0, 1, 32, 40, 64]))
    for i in range(2):
        t = ctx.buf(32 * 8)
        w32(ctx, 0x10ac3720 + 4 * i, t)
    return [r.choice([0, 1, 5, 31, 32, 33, 39, 40, 63, 64, r.randrange(0, 64), 0xffffffff])]


def g_fdiv(ctx):
    r = begin(ctx)
    t = r.choice([1.0, 1.0, 0.0, -0.0, -1.0, -1e-300, 5e-324, 1e300, float('nan'), -float('inf'), r.uniform(-2, 2)])
    wb(ctx, 0x1012bf38, struct.pack('<d', t))
    return []


def g_10100ed0(ctx):
    r = begin(ctx)
    wb(ctx, 0x1017e908, bytes([r.choice([0, 1, 7])]))
    return [ctx.u32() for _ in range(4)]


def g_makeroom(ctx):
    r = begin(ctx)
    area = ctx.buf(0x100)
    s = bytes(r.choice(b'abc012') for _ in range(r.randrange(0, 60)))
    p = area + r.randrange(0, 16)
    wb(ctx, p, s + b'\0')
    return [p, r.choice([0, 1, 2, 3, 4, 5, 8, r.randrange(0, 40)])]


def g_putc(ctx):
    r = begin(ctx)
    f = ctx.buf(0x20)
    out = ctx.buf(0x10)
    w32(ctx, f, r.choice([out, out + 3, f, f + 1]))
    w32(ctx, f + 4, r.choice([0, 1, 2, 5, 0x80000000, 0xffffffff, 0x7fffffff]))
    count = r.choice([ctx.buf(4), f + 4, f + 8])
    return [ctx.u32(), f, count]


FT_API = {'FileTimeToLocalFileTime': h_local_ft, 'FileTimeToSystemTime': h_system_t}
HEAP_API = {'HeapAlloc': api_pick([0, 0x00A90000, 0x12345678]), 'HeapFree': api_pick([0, 1])}
CHECK_API = {'HeapValidate': api_pick([0, 1, 0, 5]), 'GetLastError': api_pick([0x78, 0, 5, 0x78])}
FEAT_API = {'GetModuleHandleA': api_pick([0, 0x7c800000, 0x7c800000]), 'GetProcAddress': api_pick([0, FEAT, FEAT])}


def spec(name, orig, gen, trials, **kw):
    return dict(name=name, orig=orig, c=name, gen=gen, trials=trials, **kw)


SPECS = [
    spec('Sort_SelectionSort', 0x100fde70, g_sort, 800, void=True),
    spec('Mem_Copy', 0x100fdf00, g_copy, 2000),
    spec('Sim_100fe310', 0x100fe310, g_streams, 600),
    # superseded (C runtime replacement, formatting, DECISIONS.md): SDL3 formatter, checked natively against the original
    # spec('Sim_100fe450', 0x100fe450, g_va, 300),
    spec('File_FindClose', 0x100fe6c0, g_findclose, 300, api={'FindClose': api_pick([0, 1, 0x80000000])}),
    spec('Time_FromFileTime', 0x100fe6e0, g_filetime, 600, api=FT_API),
    spec('Stream_ReadLine', 0x100fe770, g_readline, 1000),
    # C runtime replacement (DECISIONS.md, memory): malloc and the block allocation are standard C now (no original
    # to compare with; difftest answers the original's calls with them). free of a block of the DLL's own heap still
    # runs the original's code: checked against the original heap.
    # Recorded exception: only real blocks (0 or inside the arena): the C reads the 16 bytes in front of a block to
    # tell its own blocks from the DLL's, which faults for the generator's arbitrary addresses.
    spec('Heap_FreeBlock', 0x100fefe0, g_free, 800, void=True, api=HEAP_API, heap_original=True,
         skip=lambda a: a[0] != 0 and not ARENA <= a[0] < ARENA + 0x6000),
    spec('Str_CompareNoCase', 0x100ff1e0, g_stricmp, 2000),
    spec('Sim_100ff380', 0x100ff380, g_init, 300, void=True),
    spec('Proc_Exit', 0x100ff3b0, g_exit1, 300, void=True),
    spec('Proc_ExitQuick', 0x100ff3d0, g_exit1, 300, void=True),
    spec('Sim_100ff3f0', 0x100ff3f0, g_exit0, 300, void=True),
    spec('Sim_100ff400', 0x100ff400, g_exit0, 300, void=True),
    spec('Proc_DoExit', 0x100ff410, g_doexit, 800, void=True),
    spec('Crt_CallTable', 0x100ff4c0, g_calltable, 500, void=True),
    spec('Sim_100ff4e0', 0x100ff4e0, g_ctime, 400),
    spec('Text_IntToRadix', 0x100ffde0, g_itoa, 500),
    spec('Heap_Check', 0x101000d0, g_none, 400, api=CHECK_API),
    spec('Search_Binary', 0x10100620, g_bsearch, 1500),
    spec('File_GetLength', 0x10100730, g_filelen, 800),
    spec('Sim_101009f0', 0x101009f0, g_none, 300),
    spec('Fpu_HasFdivBug', 0x10100a10, g_fdiv, 400, fp=True),
    spec('Fpu_CheckFdivBug', 0x10100a60, g_fdiv, 400, fp=True, api=FEAT_API),
    spec('Sim_10100ed0', 0x10100ed0, g_10100ed0, 300),
    spec('Str_MakeRoom', 0x10100fa0, g_makeroom, 600, void=True),
    spec('Stream_PutCharCounted', 0x10101960, g_putc, 800, void=True),
]
