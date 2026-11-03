# Differential tests for round 6, letter k (see difftest.py and docs/checker.md).
#
# C runtime code of HGSIM.DLL at 0x10106660..0x1010d340. Helpers that are not converted are replaced by emulator
# hooks on their first instruction (the code bytes are not patched): the hook appends ('stub_<address>', arguments)
# to the checker's call log (compared between the original and the C run, stack addresses shown as 'stack'), sets
# eax to an answer chosen by the trial and returns to the caller. 0x10103940, 0x1010d390 and 0x1010b7c0 are not
# stubbed: they run for real on both sides. The function pointers the code calls through data (the user32
# functions of Sim_10107840, the callback of Sim_101083f0) point at small `ret`/`ret N` stubs in the checker's
# return page with logging hooks.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX

SOURCES = [
    'sim/unsorted_1010/w6k_sbh.c', 'sim/unsorted_1010/w6k_stream.c', 'sim/unsorted_1010/w6k_handles.c',
    'sim/unsorted_1010/w6k_text.c', 'sim/unsorted_1010/w6k_misc.c',
    # existing converted code the functions above call
    'sim/unsorted_1010/w2f_mantissa.c', 'sim/unsorted_1010/w2f_ld12.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
R0 = 0x10159e38                 # the static heap region
LAST = 0x10159e3c
CUR = 0x1015be58
FREEPAGES = 0x1017e970
HANDLES = 0x10ac3720
NHANDLES = 0x10ac385c

# helper address -> number of dword arguments (stubbed by a hook that returns to the caller)
STUBS = {0x100fef30: 1, 0x1010afa0: 7, 0x10108430: 1, 0x101009b0: 1, 0x101099a0: 4, 0x10109df0: 0, 0x100ff280: 1}
# function pointers called through data: address in the return page -> (name, dword arguments, bytes popped)
CB = {0x00F00C00: ('MessageBoxA', 4, 16), 0x00F00C10: ('GetActiveWindow', 0, 0),
      0x00F00C20: ('GetLastActivePopup', 1, 4), 0x00F00C30: ('callback', 0, 0)}
CB_MB, CB_GAW, CB_GLAP, CB_CALLBACK = 0x00F00C00, 0x00F00C10, 0x00F00C20, 0x00F00C30
ST = {'fn': {}, 'seed': 0}


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def abuf(ctx, n, align=0x1000):
    """A zeroed buffer whose start is aligned."""
    a = ctx.buf(n + align, bytes(n + align))
    return (a + align - 1) & ~(align - 1)


def cstr(ctx, data, extra=24, zero_tail=False):
    tail = bytes(extra) if zero_tail else rb(ctx, extra)
    return ctx.buf(len(data) + 1 + extra, bytes(data) + b'\0' + tail)


def mix(*vals):
    x = 0x9e3779b9
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def count_calls(m, name):
    return sum(1 for e in m.api_log if e[0] == name) - 1


def read_cstr(uc, addr, maxn=200):
    out = b''
    while len(out) < maxn:
        c = bytes(uc.mem_read(addr + len(out), 1))
        if c == b'\0':
            break
        out += c
    return out


# ---- hooks ---------------------------------------------------------------------------------------
def install(m):
    if getattr(m, '_w6k', False):
        return
    m._w6k = True

    def stub_cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        k = STUBS[address]
        raw = list(struct.unpack('<%dI' % k, bytes(uc.mem_read(esp + 4, 4 * k)))) if k else []
        args = tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)
        name = 'stub_%x' % address
        m.api_log.append((name, args))
        n = count_calls(m, name)
        f = ST['fn'].get(address)
        val = f(n, raw, uc) if f else mix(ST['seed'], address, n)
        uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
        # return to the caller (cdecl: the caller removes the arguments); code bytes are not patched
        uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)

    for a in STUBS:
        m.raw.hook_add(UC_HOOK_CODE, stub_cb, None, a, a)
    # C runtime replacement: the C calls _sopen by name now; its stub answers that symbol too
    if 0x101099a0 in STUBS and 'Crt_SOpen' in m.syms:
        m.raw.hook_add(UC_HOOK_CODE, lambda uc, address, size, ud: stub_cb(uc, 0x101099a0, size, ud), None,
                       m.syms['Crt_SOpen'], m.syms['Crt_SOpen'])

    def cb_cb(uc, address, size, ud):
        name, k, _ = CB[address]
        esp = uc.reg_read(UC_X86_REG_ESP)
        raw = list(struct.unpack('<%dI' % k, bytes(uc.mem_read(esp + 4, 4 * k)))) if k else []
        args = tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)
        m.api_log.append((name, args))
        n = count_calls(m, name)
        f = ST['fn'].get(address)
        uc.reg_write(UC_X86_REG_EAX, (f(n, raw, uc) if f else mix(ST['seed'], address, n)) & 0xffffffff)

    for a, (name, k, pop) in CB.items():
        m.raw.mem_write(a, b'\xc2' + struct.pack('<H', pop) if pop else b'\xc3')   # our own page, not DLL code
        m.raw.hook_add(UC_HOOK_CODE, cb_cb, None, a, a)


def begin(ctx):
    """Installs the hooks (once per machine) and forgets the answers of the previous trial."""
    install(ctx.m)
    ST.clear()
    ST['fn'] = {}
    ST['seed'] = ctx.rng.getrandbits(32)
    ST['m'] = ctx.m


def malloc_pool(ctx, size, p_zero=0.15):
    """Answers of the 0x100fef30 stub: fresh zeroed blocks of `size` bytes, now and then 0."""
    blocks = [0 if ctx.rng.random() < p_zero else zbuf(ctx, size) for _ in range(3)]
    ST['fn'][0x100fef30] = lambda n, args, uc: blocks[n % 3]


def api_seq(name):
    """An api handler answering the k-th call to `name` in this run with ST[name][k % len] (1 when not set)."""
    def h(n, args):
        vals = ST.get(name)
        if not vals:
            return 1
        return vals[count_calls(ST['m'], name) % len(vals)]
    return h


# ---- small-block heap --------------------------------------------------------------------------
def ring(ctx, regions):
    """Links the regions (R0 among them or not) as a ring through +0 (next) and +4 (previous)."""
    k = len(regions)
    for i, r in enumerate(regions):
        w32(ctx, r, regions[(i + 1) % k])
        w32(ctx, r + 4, regions[(i - 1) % k])


def g_new_region(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, R0 + 0x10, r.choice([0xffffffff, 0xffffffff, 0x12340000, 0]))
    w32(ctx, R0, r.choice([0, R0, R0, zbuf(ctx, 0x20)]))
    w32(ctx, LAST, r.choice([0, R0, R0, zbuf(ctx, 0x20)]) or 0)
    if r32(ctx.m.uc, LAST) == 0 and r.random() < 0.8:
        w32(ctx, LAST, zbuf(ctx, 0x20))     # the C and the original both write through it for a new header
    hdr = zbuf(ctx, 0x2020) if r.random() < 0.85 else 0
    ST['HeapAlloc'] = [hdr]
    pages = abuf(ctx, 0x10000) if r.random() < 0.85 else 0
    ST['VirtualAlloc'] = [pages, r.choice([1, 1, 1, 0, pages])]
    ST['VirtualFree'] = [r.choice([0, 1])]
    ST['HeapFree'] = [r.choice([0, 1])]
    return []


def g_release(ctx):
    begin(ctx)
    r = ctx.rng
    reg = R0 if r.random() < 0.3 else newbuf(ctx, 0x40)
    w32(ctx, reg + 0x10, ctx.u32())
    if reg != R0 or r.random() < 0.8:
        w32(ctx, reg, r.choice([R0, zbuf(ctx, 0x20)]))
        w32(ctx, reg + 4, r.choice([R0, zbuf(ctx, 0x20)]))
    w32(ctx, CUR, r.choice([reg, reg, R0, 0]))
    ST['VirtualFree'] = [r.choice([0, 1, 7])]
    ST['HeapFree'] = [r.choice([0, 1, 9])]
    return [reg]


def region_table(ctx, reg, choices):
    """Fills the 1024 entries of a region with values from `choices` (first dword) and 0xf1 or so."""
    r = ctx.rng
    data = b''
    for _ in range(0x400):
        data += struct.pack('<II', r.choice(choices), r.choice([0xf1, 0xf1, 0x20]))
    wb(ctx, reg + 0x18, data)


def decommit_world(ctx):
    """1..3 regions in a ring with entries 0xf0 (empty pages), -1 and others; returns them."""
    r = ctx.rng
    nreg = r.choice([1, 2, 3])
    regions = []
    use_r0 = r.random() < 0.6
    for i in range(nreg):
        reg = R0 if (i == 0 and use_r0) else zbuf(ctx, 0x2020)
        regions.append(reg)
        w32(ctx, reg + 0x10, r.choice([0x01230000, 0x7f000000, 0x00400000]))   # only passed to VirtualFree
        mode = r.random()
        if mode < 0.3:
            choices = [0xffffffff] * 30 + [0xf0]            # nearly all decommitted: the region may be released
        elif mode < 0.6:
            choices = [0xffffffff, 0xf0, 0x40, 0xf0]
        else:
            choices = [0xffffffff, 0xffffffff, 0x10, 0xef, 0xf0]
        region_table(ctx, reg, choices)
        if r.random() < 0.3:
            # whole region decommitted but a few pages
            wb(ctx, reg + 0x18, struct.pack('<II', 0xffffffff, 0xf1) * 0x400)
            for _ in range(r.choice([1, 2, 3])):
                w32(ctx, reg + 0x18 + 8 * r.randrange(0, 0x400), 0xf0)
        w32(ctx, reg + 0xc, r.choice([0, reg + 0x18 + 8 * r.randrange(0, 0x400), 0xffffffff, reg + 0x20]))
    r.shuffle(regions)
    if R0 not in regions:
        # the static region is always in the ring of the real heap (LAST is its +4)
        w32(ctx, R0 + 0x10, r.choice([0x01230000, 0x7f000000]))
        region_table(ctx, R0, [0xffffffff, 0x40])
        regions.insert(r.randrange(0, len(regions) + 1), R0)
    ring(ctx, regions)
    w32(ctx, CUR, r.choice(regions + [0]))
    w32(ctx, FREEPAGES, r.choice([0, 5, 0x1f, 0x20, 0x40]))
    ST['VirtualFree'] = [r.choice([0, 1, 1, 1]) for _ in range(5)]
    ST['HeapFree'] = [1]
    return regions


def g_decommit(ctx):
    begin(ctx)
    r = ctx.rng
    decommit_world(ctx)
    return [r.choice([0x10, 1, 2, 5, 0, 0xffffffff, 100, 3000])]


def g_free_block(ctx):
    begin(ctx)
    r = ctx.rng
    regions = decommit_world(ctx)
    reg = r.choice(regions)
    base = r32(ctx.m.uc, reg + 0x10)
    idx = r.choice([0, 1, 2, r.randrange(0, 16), r.randrange(0, 0x400)])
    p = base + idx * 0x1000 + r.randrange(0, 0x1000)
    have = r.choice([0xf0 - 5, 0xf0 - 1, 0x80, r.randrange(0, 0xf0)])
    w32(ctx, reg + 0x18 + 8 * idx, have)
    mb = newbuf(ctx, 4)
    wb(ctx, mb, [r.choice([5, 1, 0x70, 0xf0 - have if 0 < 0xf0 - have < 256 else 3, r.randrange(256)])])
    w32(ctx, FREEPAGES, r.choice([0x1f, 0x1f, 0, 5, 0x20]))
    return [reg, p, mb]


def page_map(ctx):
    """A page map of 0xf0 bytes: allocated blocks (size byte then size-1 zeros) and free bytes (0).
    Returns (bytes, [block start positions], [free run (pos, length)])."""
    r = ctx.rng
    m = bytearray(0xf0)
    pos = 0
    starts, runs = [], []
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
            starts.append(pos)
            pos += n
    return bytes(m), starts, runs


def free_run_at(mp, pos):
    k = 0
    while pos + k < 0xf0 and mp[pos + k] == 0:
        k += 1
    return k


def g_page_alloc(ctx):
    begin(ctx)
    r = ctx.rng
    page = abuf(ctx, 0x1000)
    mp, starts, runs = page_map(ctx)
    wb(ctx, page + 8, mp + b'\xff' + rb(ctx, 0x1000 - 0xf9))
    cands = [pos for pos, _ in runs] + starts + [r.randrange(0, 0xf0)]
    pos = r.choice(cands)
    if r.random() < 0.05:
        pos = 0xf0
    cnt = free_run_at(mp, pos) if r.random() < 0.8 else r.randrange(0, 0x30)
    w32(ctx, page, page + 8 + pos)
    w32(ctx, page + 4, cnt)
    total = sum(n for _, n in runs)
    avail = r.choice([total, total, total + 5, max(0, total - 10), r.randrange(0, 0xf0)])
    n = r.choice([1, 2, 3, 4, 8, 16, cnt, cnt + 1, r.randrange(1, 0x40)]) or 1
    return [page, avail, n]


def heap_world(ctx):
    """A consistent set of regions and pages (as the heap keeps them), then often one thing broken."""
    r = ctx.rng
    nreg = r.choice([1, 1, 2, 3])
    regions = []
    use_r0 = r.random() < 0.7
    pages_of = {}
    for i in range(nreg):
        reg = R0 if (i == 0 and use_r0) else zbuf(ctx, 0x2020)
        regions.append(reg)
        if r.random() < 0.15:
            w32(ctx, reg + 0x10, 0xffffffff)
            continue
        npg = r.choice([1, 2, 3])
        base = abuf(ctx, npg * 0x1000)
        w32(ctx, reg + 0x10, base)
        tab = b''
        pl = []
        for k in range(npg):
            pg = base + k * 0x1000
            mp, starts, runs = page_map(ctx)
            wb(ctx, pg + 8, mp + b'\xff')
            pos = r.choice([p for p, _ in runs] + starts) if (runs or starts) else 0
            w32(ctx, pg, pg + 8 + pos)
            w32(ctx, pg + 4, r.choice([0, free_run_at(mp, pos)]))
            # the entry counts the free bytes (the zeros inside allocated blocks are not free)
            tab += struct.pack('<II', sum(n for _, n in runs), r.choice([0xf1, 0xf1, 0x100]))
            pl.append(pg)
        tab += struct.pack('<II', 0xffffffff, 0xf1) * (0x400 - npg)
        wb(ctx, reg + 0x18, tab)
        w32(ctx, reg + 0xc, reg + 0x18 + 8 * npg)
        pages_of[reg] = pl
    r.shuffle(regions)
    if R0 not in regions:
        regions.insert(0, R0)
        w32(ctx, R0 + 0x10, 0xffffffff)
    ring(ctx, regions)
    w32(ctx, CUR, r.choice(regions + [0, 0x1234]))
    # break something
    if r.random() < 0.65 and pages_of:
        reg = r.choice(list(pages_of))
        pg = r.choice(pages_of[reg])
        k = (pg - r32(ctx.m.uc, reg + 0x10)) // 0x1000
        what = r.randrange(0, 8)
        if what == 0:
            wb(ctx, pg + 8 + r.randrange(0, 0xf0), [r.choice([0, 1, 2, 5, 0x30])])
        elif what == 1:
            w32(ctx, reg + 0x18 + 8 * k, r.randrange(0, 0xf1))
        elif what == 2:
            w32(ctx, pg, r.choice([pg + 8 + 0xf0, pg + 8 + r.randrange(0, 0xf0), pg + 0x200]))
        elif what == 3:
            wb(ctx, pg + 8 + 0xf0, [r.randrange(0, 0xff)])
        elif what == 4:
            w32(ctx, reg + 0xc, r.choice([0, reg + 0x18, reg + 0x18 + 8 * 0x3ff]))
        elif what == 5:
            w32(ctx, reg + 0x1c + 8 * k, r.choice([0, 1, 3, 8]))
        elif what == 6:
            w32(ctx, pg + 4, r.choice([5, 20, 0xf0]))
        else:
            w32(ctx, reg + 0x18 + 8 * r.randrange(k + 1, 0x400), r.choice([0, 0xf0]))
    return regions


def g_heap_check(ctx):
    begin(ctx)
    heap_world(ctx)
    return []


# ---- streams -----------------------------------------------------------------------------------
def g_getbuf(ctx):
    begin(ctx)
    malloc_pool(ctx, 0x40, 0.4)
    w32(ctx, 0x1017e964, ctx.u32())
    return [newbuf(ctx, 0x20)]


def g_ungetc(ctx):
    begin(ctx)
    r = ctx.rng
    malloc_pool(ctx, 0x40, 0.4)
    s = newbuf(ctx, 0x20)
    flags = r.choice([1, 0x80, 0x82, 0x41, 0x11, 0x51, 0x81, 0, 2, 0x40]) | (r.getrandbits(32) & r.choice([0, 0xffffff00, 0x0c]))
    w32(ctx, s + 0xc, flags)
    b = newbuf(ctx, 0x40)
    off = r.choice([0, 0, 1, 5, 0x20])
    w32(ctx, s + 8, 0 if r.random() < 0.2 else b)
    w32(ctx, s, b + off)
    w32(ctx, s + 4, r.choice([0, 0, 1, 7]))
    c = r.choice([0xffffffff, r.randrange(256), r.getrandbits(32), r.randrange(256)])
    if off and r.random() < 0.5:
        wb(ctx, b + off - 1, [c & 0xff])
    return [c, s]


def g_putch(ctx):
    begin(ctx)
    r = ctx.rng
    out = zbuf(ctx, 16)
    w32(ctx, 0x1015c494, r.choice([0xfffffffe, 0xfffffffe, 0xffffffff, 0x88]))
    ST['CreateFileA'] = [r.choice([0xffffffff, 0x99, 0x44])]
    ok = r.choice([0, 1, 1])

    def wc(n, args, uc):
        # the character itself (the pointer is a stack address): kept where the checker compares it
        esp = uc.reg_read(UC_X86_REG_ESP)
        p = r32(uc, esp + 8)
        uc.mem_write(out + 4 * n, bytes(uc.mem_read(p, 1)))
        return ok
    ST['WriteConsoleA'] = wc
    return [r.choice([r.randrange(256), ctx.u32()])]


def g_wctomb(ctx):
    begin(ctx)
    r = ctx.rng
    out = zbuf(ctx, 16)
    s = 0 if r.random() < 0.1 else newbuf(ctx, 8)
    w32(ctx, 0x1017e950, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x1017e960, r.choice([1252, 932, ctx.u32()]))
    w32(ctx, 0x1015965c, r.choice([1, 2, 5]))
    ret = r.choice([0, 1, 2, 1])
    used = r.choice([0, 0, 1, 0x100])
    nbytes = r.choice([1, 2])
    data = rb(ctx, 2)

    def h(n, args, uc):
        esp = uc.reg_read(UC_X86_REG_ESP)
        wide = r32(uc, esp + 4 + 8)
        uc.mem_write(out, bytes(uc.mem_read(wide, 2)))
        if args[4] != 'stack' and args[4]:
            uc.mem_write(args[4], data[:nbytes])
        ud = r32(uc, esp + 4 + 28)
        if used:
            uc.mem_write(ud, struct.pack('<I', used))
        return ret
    ST['WideCharToMultiByte'] = h
    wc = r.choice([r.randrange(0, 0x100), r.randrange(0, 0x10000), ctx.u32(), 0x41, 0xff, 0x100])
    return [s, wc]


# ---- handle table ------------------------------------------------------------------------------
def handle_world(ctx, nblk=None):
    """Blocks of 32 slots (dword, flag byte, byte) for the first nblk pointers at 0x10ac3720."""
    r = ctx.rng
    nblk = r.choice([0, 1, 1, 2, 3, 64]) if nblk is None else nblk
    full = r.random() < 0.5
    blocks = []
    for i in range(64):
        if i < nblk:
            data = b''
            for k in range(32):
                fl = r.choice([1, 1, 1, 0x81, 0x41, 0, 0x0a]) if not full else r.choice([1, 0x81, 0x41, 0xc1])
                data += struct.pack('<IBBH', r.choice([0xffffffff, 0x100 + k, 0, ctx.u32()]), fl, r.randrange(256), r.randrange(65536))
            b = ctx.buf(0x100, data)
            blocks.append(b)
            w32(ctx, HANDLES + 4 * i, b)
        else:
            w32(ctx, HANDLES + 4 * i, 0)
    if full and nblk and r.random() < 0.5:
        b = r.choice(blocks)
        wb(ctx, b + 8 * r.randrange(32) + 4, [r.choice([0, 0x80, 0x0a])])
    w32(ctx, NHANDLES, r.choice([nblk * 32, nblk * 32, r.randrange(0, 0x900)]))
    return nblk


def g_alloc_handle(ctx):
    begin(ctx)
    malloc_pool(ctx, 0x100, 0.3)
    handle_world(ctx)
    return []


def g_set_handle(ctx):
    begin(ctx)
    r = ctx.rng
    nblk = handle_world(ctx, r.choice([1, 2, 3]))
    w32(ctx, 0x1017e900, r.choice([1, 1, 0, 2]))
    fh = r.choice([0, 1, 2, 3, r.randrange(0, nblk * 32), r.randrange(0, nblk * 32 + 10), ctx.u32()])
    if r.random() < 0.5 and fh < nblk * 32:
        w32(ctx, r32(ctx.m.uc, HANDLES + 4 * (fh >> 5)) + 8 * (fh & 31), 0xffffffff)
    return [fh, ctx.u32()]


def g_free_handle(ctx):
    begin(ctx)
    r = ctx.rng
    nblk = handle_world(ctx, r.choice([1, 2, 3]))
    w32(ctx, 0x1017e900, r.choice([1, 1, 0, 2]))
    fh = r.choice([0, 1, 2, 3, r.randrange(0, nblk * 32), r.randrange(0, nblk * 32 + 10), ctx.u32()])
    return [fh]


def g_three(ctx):
    begin(ctx)
    return [ctx.u32(), ctx.u32(), ctx.u32()]


# ---- text --------------------------------------------------------------------------------------
def compare_stub(n, args, uc):
    """0x1010afa0 as a case-insensitive compare of the two counted texts: 1, 2 or 3; now and then 0."""
    if mix(ST['seed'], n) % 7 == 0:
        return 0
    a = bytes(uc.mem_read(args[2], args[3] & 0xff)) if args[2] else b''
    b = bytes(uc.mem_read(args[4], args[5] & 0xff)) if args[4] else b''
    a, b = a.split(b'\0')[0].lower(), b.split(b'\0')[0].lower()
    return 2 if a == b else (1 if a < b else 3)


def rname(ctx, alpha=b'abAB_='):
    return bytes(ctx.rng.choice(alpha) for _ in range(ctx.rng.choice([0, 1, 2, 3, 4, 6])))


def g_strnicoll(ctx):
    begin(ctx)
    r = ctx.rng
    ST['fn'][0x1010afa0] = compare_stub
    w32(ctx, 0x1017ebc8, ctx.u32())
    w32(ctx, 0x1017ebc4, ctx.u32())
    a = rname(ctx)
    b = a if r.random() < 0.4 else rname(ctx)
    return [cstr(ctx, a), cstr(ctx, b), r.choice([0, 1, 2, 3, 5, 10, ctx.u32() & 0xff])]


def g_findenv(ctx):
    begin(ctx)
    r = ctx.rng
    ST['fn'][0x1010afa0] = compare_stub
    names = [rname(ctx, b'abAB') or b'x' for _ in range(r.choice([0, 1, 2, 4, 6]))]
    ptrs = []
    for nm in names:
        sep = r.choice([b'=', b'=', b'', b'x='])
        ptrs.append(cstr(ctx, nm + sep + rname(ctx)))
    lst = ctx.buf(4 * (len(ptrs) + 1) + 8, struct.pack('<%dI' % (len(ptrs) + 1), *(ptrs + [0])) + rb(ctx, 8))
    w32(ctx, 0x1017e8b0, lst)
    name = r.choice(names) if names and r.random() < 0.6 else rname(ctx, b'abAB')
    if r.random() < 0.3:
        name = name.upper()
    ln = len(name) if r.random() < 0.8 else r.randrange(0, 5)
    return [cstr(ctx, name + b'=' + rname(ctx)), ln]


def g_strdup(ctx):
    begin(ctx)
    r = ctx.rng
    malloc_pool(ctx, 0x60, 0.2)
    s = 0 if r.random() < 0.1 else cstr(ctx, bytes(r.choice(b'abcXYZ019 .') for _ in range(r.randrange(0, 70))))
    return [s]


def g_strnicmp(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1017e950, r.choice([0, 0, 1, ctx.u32()]))
    # now and then the converter answers with bits above the low byte: the same bits for every call (so equal
    # characters stay equal and the bits travel on into the next characters) or different ones
    garbage = r.choice([0, 0, 1, 2])
    high = 0x100 << r.randrange(0, 20)

    def lower(n, args, uc):
        x = args[0]
        c = x & 0xff
        v = (x & 0xffffff00) | (c + 0x20 if 0x41 <= c <= 0x5a else c)
        if garbage == 1:
            v |= high
        elif garbage == 2 and mix(ST['seed'], n) % 5 == 0:
            v ^= 0x100 << (mix(ST['seed'], n, 1) % 20)
        return v
    ST['fn'][0x100ff280] = lower
    alpha = b'aAbBzZ[`\x80\xe1\xc1@'
    a = bytes(r.choice(alpha) for _ in range(r.randrange(0, 8)))
    if r.random() < 0.5:
        b = bytes(c ^ 0x20 if (c | 0x20) in b'abz' and r.random() < 0.5 else c for c in a)
        if r.random() < 0.5 and b:
            b = b[:-1] + bytes([r.choice(alpha)])
    else:
        b = bytes(r.choice(alpha) for _ in range(r.randrange(0, 8)))
    return [cstr(ctx, a), cstr(ctx, b), r.choice([0, 1, 2, 4, 8, 100, 0xffffffff])]


def g_strnset(ctx):
    begin(ctx)
    r = ctx.rng
    s = cstr(ctx, bytes(r.choice(b'abc') for _ in range(r.randrange(0, 20))))
    return [s, ctx.u32(), r.choice([0, 1, 3, 10, 30, 0xffffffff])]


def g_cmdline(ctx):
    begin(ctx)
    r = ctx.rng
    tab = bytearray(256)
    if r.random() < 0.5:
        for c in range(0x81, 0xa0):
            tab[c] = 4
    for _ in range(r.choice([0, 3, 10])):
        tab[r.randrange(256)] |= r.choice([4, 1, 8])
    wb(ctx, 0x1017eac1, tab)
    pieces = [b' ', b'\t', b'"', b'\\', b'\\\\', b'a', b'bc', b'xyz', b'\x81A', b'\x85', b'""', b'\\"', b'  ']
    name = r.choice([b'"prog name.exe"', b'prog', b'"pro\x81g"', b'"', b'', b'p\x81', b'"abc', b'x\t'])
    rest = b''.join(r.choice(pieces) for _ in range(r.randrange(0, 14)))
    line = name + rest
    if r.random() < 0.1 and line:
        line = line[:-1] + b'\x81'
    cmd = cstr(ctx, line, 32, zero_tail=True)
    n = len(line) + 40
    argv = 0 if r.random() < 0.3 else newbuf(ctx, 4 * n)
    args = 0 if r.random() < 0.3 else newbuf(ctx, 2 * n)
    return [cmd, argv, args, newbuf(ctx, 4), newbuf(ctx, 4)]


def g_round_digits(ctx):
    begin(ctx)
    r = ctx.rng
    digits = bytes(r.choice(b'0123456789999') for _ in range(r.randrange(0, 20)))
    if r.random() < 0.2:
        digits = b'9' * r.randrange(0, 12)
    if r.random() < 0.1:
        digits += bytes([r.choice([0x80, 0xff, 0x35, 0x34])])
    ndig = r.choice([-2, -1, 0, 1, 2, 3, 5, 8, 12, 18, 22])
    st = newbuf(ctx, 0x10)
    w32(ctx, st + 0xc, cstr(ctx, digits))
    w32(ctx, st + 4, r.choice([0, 3, -5, ctx.u32()]))
    buf = newbuf(ctx, max(ndig, 0) + 8)
    return [buf, ndig & 0xffffffff, st]


# ---- other -------------------------------------------------------------------------------------
def g_msgbox(ctx):
    begin(ctx)
    r = ctx.rng
    loaded = r.random() < 0.5
    if loaded:
        w32(ctx, 0x1017e9a8, CB_MB)
        w32(ctx, 0x1017e9ac, r.choice([0, CB_GAW]))
        w32(ctx, 0x1017e9b0, r.choice([0, CB_GLAP]))
    else:
        w32(ctx, 0x1017e9a8, 0)
        w32(ctx, 0x1017e9ac, ctx.u32())
        w32(ctx, 0x1017e9b0, ctx.u32())
    ST['LoadLibraryA'] = [r.choice([0, 0x77000000, 0x77000000])]
    ST['GetProcAddress'] = [r.choice([0, CB_MB, CB_MB, CB_MB]), r.choice([0, CB_GAW]), r.choice([0, CB_GLAP])]
    win = r.choice([0, 0, 0x1234])
    popup = r.choice([0, 0x5678])
    ST['fn'][CB_GAW] = lambda n, args, uc: win
    ST['fn'][CB_GLAP] = lambda n, args, uc: popup
    return [cstr(ctx, b'text'), cstr(ctx, b'caption'), ctx.u32()]


def g_codepage(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1017e960, ctx.u32())
    w32(ctx, 0x1017ebdc, ctx.u32())
    ST['GetACP'] = [1252]
    ST['GetOEMCP'] = [437]
    return [r.choice([0xfffffffe, 0xfffffffd, 0xfffffffc, 0xffffffff, 0, 1252, ctx.u32()])]


def g_nmsg(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1017e8fc, r.choice([0, 1, 2, ctx.u32()]))
    w32(ctx, 0x1017e900, r.choice([0, 1, 2]))
    w32(ctx, 0x1017ebe8, r.choice([0, CB_CALLBACK]))
    return []


def g_atoflt(ctx):
    begin(ctx)
    r = ctx.rng
    sign = r.choice([b'', b'', b'-', b'+', b' ', b'  -'])
    mant = r.choice([b'0', b'1', b'123', b'3.25', b'.5', b'1.', b'99999999999999999999',
                     bytes(r.choice(b'0123456789') for _ in range(r.randrange(1, 25)))])
    exp = r.choice([b'', b'', b'e5', b'E-3', b'e+40', b'e-50', b'e400', b'e-400', b'd7', b'e'])
    text = r.choice([sign + mant + exp, sign + mant + exp, b'abc', b'', b'1e38', b'3.4028236e38', b'1.17e-38', b'1e-45'])
    out = newbuf(ctx, 8)
    return [out, cstr(ctx, text)]


def g_amsg2(ctx):
    begin(ctx)
    return []


def g_once(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1017ece0, r.choice([0, 0, 1, ctx.u32()]))
    return []


def g_cvtdate(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1015c028, r.choice([0, 3600, -3600, 18000, -36000, r.randrange(-90000, 90000), ctx.u32()]))
    big = r.random() < 0.15

    def val(lo, hi):
        return ctx.u32() if big and r.random() < 0.3 else r.randrange(lo, hi)
    return [r.choice([0, 1, 1, 2]), r.choice([0, 1, 1, 2]), val(0, 200), r.randrange(0, 13), r.randrange(1, 6),
            r.randrange(0, 7), val(1, 32), val(0, 24), val(0, 60), val(0, 60), val(0, 1000)]


def g_fpclass(ctx):
    begin(ctx)
    r = ctx.rng
    sign = r.choice([0, 0x80000000])
    kind = r.randrange(0, 7)
    lo = r.getrandbits(32)
    if kind == 0:       # infinity
        hi, lo = 0x7ff00000, 0
    elif kind == 1:     # NaN (quiet or signalling) or odd 0x7ff patterns
        hi, lo = r.choice([0x7ff80000, 0x7ff00001, 0x7ff7ffff, 0x7fffffff]), r.choice([0, lo])
    elif kind == 2:     # denormal
        hi, lo = r.choice([0, 1, 0xfffff, r.getrandbits(20)]), r.choice([0, lo, 1])
    elif kind == 3:     # zero
        hi, lo = 0, 0
    else:
        hi = r.randrange(1, 0x7ff) << 20 | r.getrandbits(20)
    return [lo, (hi & 0x7fffffff) | sign]


API = {name: api_seq(name) for name in ['HeapAlloc', 'HeapFree', 'VirtualAlloc', 'VirtualFree', 'CreateFileA',
                                              'LoadLibraryA', 'GetProcAddress', 'GetACP', 'GetOEMCP']}


def _dyn(name):
    def h(n, args, uc):
        return ST[name](n, args, uc) if name in ST else 1
    return h


API['WriteConsoleA'] = _dyn('WriteConsoleA')
API['WideCharToMultiByte'] = _dyn('WideCharToMultiByte')

SPECS = [
    # deleted: not hooked, no C caller (DECISIONS.md, "The last original code", step 3)
    # dict(name='Sim_10106660', orig=0x10106660, c='Sim_10106660', gen=g_new_region, trials=300, api=API),
    dict(name='Sim_101067d0', orig=0x101067d0, c='Sim_101067d0', gen=g_release, trials=300, api=API),
    dict(name='Sim_10106830', orig=0x10106830, c='Sim_10106830', gen=g_decommit, trials=400, api=API, void=True),
    dict(name='Sim_10106960', orig=0x10106960, c='Sim_10106960', gen=g_free_block, trials=400, api=API, void=True),
    dict(name='Sim_10106c00', orig=0x10106c00, c='Sim_10106c00', gen=g_page_alloc, trials=1500, api=API),
    dict(name='Sim_10106e50', orig=0x10106e50, c='Sim_10106e50', gen=g_heap_check, trials=800, api=API),
    dict(name='Sim_10107020', orig=0x10107020, c='Sim_10107020', gen=g_getbuf, trials=300, api=API, void=True),
    dict(name='Sim_101095f0', orig=0x101095f0, c='Sim_101095f0', gen=g_ungetc, trials=600, api=API),
    dict(name='Sim_1010d2f0', orig=0x1010d2f0, c='Sim_1010d2f0', gen=g_putch, trials=300, api=API),
    dict(name='Sim_10109050', orig=0x10109050, c='Sim_10109050', gen=g_wctomb, trials=400, api=API),
    dict(name='Sim_10109680', orig=0x10109680, c='Sim_10109680', gen=g_alloc_handle, trials=400, api=API),
    dict(name='Sim_10109740', orig=0x10109740, c='Sim_10109740', gen=g_set_handle, trials=400, api=API),
    dict(name='Sim_101097f0', orig=0x101097f0, c='Sim_101097f0', gen=g_free_handle, trials=400, api=API),
    dict(name='Sim_10109980', orig=0x10109980, c='Sim_10109980', gen=g_three, trials=300, api=API),
    dict(name='Sim_101078d0', orig=0x101078d0, c='Sim_101078d0', gen=g_strnicoll, trials=400, api=API),
    dict(name='Sim_1010b4b0', orig=0x1010b4b0, c='Sim_1010b4b0', gen=g_findenv, trials=600, api=API),
    dict(name='Sim_1010c9f0', orig=0x1010c9f0, c='Sim_1010c9f0', gen=g_strdup, trials=300, api=API),
    dict(name='Sim_1010d240', orig=0x1010d240, c='Sim_1010d240', gen=g_strnicmp, trials=1000, api=API),
    dict(name='Crt_StrNSet', orig=0x1010d340, c='Crt_StrNSet', gen=g_strnset, trials=400, api=API),
    dict(name='Sim_10107b20', orig=0x10107b20, c='Sim_10107b20', gen=g_cmdline, trials=1500, api=API, void=True),
    dict(name='Sim_10108e60', orig=0x10108e60, c='Sim_10108e60', gen=g_round_digits, trials=800, api=API, void=True),
    dict(name='Sim_10107840', orig=0x10107840, c='Sim_10107840', gen=g_msgbox, trials=400, api=API),
    dict(name='Sim_10107f20', orig=0x10107f20, c='Sim_10107f20', gen=g_codepage, trials=300, api=API),
    dict(name='Sim_101083f0', orig=0x101083f0, c='Sim_101083f0', gen=g_nmsg, trials=300, api=API, void=True),
    dict(name='Sim_10108e20', orig=0x10108e20, c='Sim_10108e20', gen=g_atoflt, trials=600, api=API),
    dict(name='Sim_10109040', orig=0x10109040, c='Sim_10109040', gen=g_amsg2, trials=300, api=API, void=True),
    dict(name='Sim_10109dd0', orig=0x10109dd0, c='Sim_10109dd0', gen=g_once, trials=300, api=API),
    dict(name='Sim_1010a310', orig=0x1010a310, c='Sim_1010a310', gen=g_cvtdate, trials=1000, api=API, void=True),
    dict(name='Sim_1010a9c0', orig=0x1010a9c0, c='Sim_1010a9c0', gen=g_fpclass, trials=600, api=API),
]
