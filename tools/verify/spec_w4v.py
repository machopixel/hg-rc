# Differential tests for round 4, letter v: functions that call Windows through the import slots (see difftest.py
# and docs/checker.md).
#
# Windows imports are stubbed by the checker: every call is logged (name and arguments) and the two logs must be
# identical. The answers come from the `API` handlers below; they depend only on the call number or on state the
# argument generator prepared, so the original and the C run see the same answers. A heap block returned by the
# HeapAlloc handler is a buffer of the argument memory filled with random bytes (the flag 9 'zero fill' of the
# real HeapAlloc is the caller's request and is only checked through the call log).
#
# Indirect calls (callbacks of the timer list, the display driver method, the force feedback library) go to small
# stubs in the emulator: a `ret` at STUB_BASE + 16 * k with a code hook that appends the arguments to a log buffer in
# argument memory (compared after every run) and answers from a sequence chosen by the generator, indexed by the
# number of calls already logged, so both runs see the same answers.
#
# Floating point: Sim_100570fb (called by the joystick function) and Camera_SetAspect (called by Sim_10034c11)
# use doubles; the C is built with SSE2 and the control word is set to 0x27f (53-bit mantissa, like the Windows
# default) before every trial, which is what the x87 unit uses in the original. Results are compared bit for bit.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW

SOURCES = [
    # the functions of this letter
    'sim/win/w4v_heap.c', 'sim/win/w4v_gdi.c', 'sim/win/w4v_dll.c',
    'sim/unsorted_1000/w4v_misc.c', 'sim/unsorted_1002/w4v_misc.c', 'sim/unsorted_1003/w4v_misc.c',
    'sim/unsorted_1004/w4v_misc.c', 'sim/unsorted_1005/w4v_misc.c', 'sim/unsorted_1005/w4v_joystick.c',
    'sim/unsorted_1007/w4v_misc.c', 'sim/unsorted_1008/w4v_fade.c', 'sim/unsorted_100a/w4v_misc.c',
    'sim/unsorted_100b/w4v_wrappers.c', 'sim/unsorted_100c/w4v_timers.c', 'sim/unsorted_100d/w4v_misc.c',
    # callees that are already converted
    'sim/win/w4a_heap.c', 'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_100f/w2j_str.c',
    'sim/unsorted_100f/w2g_crt.c', 'sim/view/objlist.c', 'sim/unsorted_1007/w2j_misc.c', 'sim/util/field.c',
    'sim/unsorted_100a/w2g_misc.c', 'sim/unsorted_100a/w3t_misc.c', 'sim/unsorted_100d/w2i_table.c',
    'sim/unsorted_1004/w2l_values.c', 'sim/unsorted_1004/fields.c', 'sim/unsorted_1005/w3s_scale.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STUB_BASE = 0x00F00100
NSTUB = 16
STACK_LO, STACK_HI = 0x700000, 0x800000
TRAMP = 0x00AF0000
LOGMAX = 80
RECW = 8
ST = {}

HEAP = 0x1017b8f8
TICK = 0x1015dc78

S_FORCE, S_DRIVER, S_CB0, S_CB1, S_CB2 = 0, 1, 2, 3, 4
NARGS = {S_FORCE: 0, S_DRIVER: 1}       # the callbacks take four arguments; only the real ones are logged


def stub(k):
    return STUB_BASE + 16 * k


LEAK = []     # (address, previous bytes) of the data the generators wrote; put back at the start of the next trial


def _put(ctx, addr, data):
    LEAK.append((addr, bytes(ctx.m.uc.mem_read(addr, len(data)))))
    ctx.m.uc.mem_write(addr, data)


def w32(ctx, addr, *vals):
    _put(ctx, addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def wbytes(ctx, addr, data):
    _put(ctx, addr, bytes(data))


def _hook(m):
    uc = m.uc

    def cb(uc, address, size, ud):
        if (address - STUB_BASE) & 15:
            return
        k = (address - STUB_BASE) >> 4
        esp = uc.reg_read(UC_X86_REG_ESP)
        a = list(struct.unpack('<5I', bytes(uc.mem_read(esp + 4, 20))))
        a = [0x5ACC if STACK_LO <= v <= STACK_HI else v for v in a]
        log = ST['log']
        n = r32(uc, log)
        nargs = NARGS.get(k, 4)
        rec = [k + 1] + a[:nargs] + [0] * (7 - nargs)
        if k == S_DRIVER:
            try:
                rec[5:8] = struct.unpack('<3I', bytes(uc.mem_read(a[0], 12)))
            except Exception:
                rec[5] = 0xBADBAD
        if n < LOGMAX:
            uc.mem_write(log + 4 + n * RECW * 4, struct.pack('<%dI' % RECW, *rec))
        uc.mem_write(log, struct.pack('<I', n + 1))
        seq = ST['ret'].get(k) or [0]
        uc.reg_write(UC_X86_REG_EAX, seq[n % len(seq)] & 0xffffffff)

    uc.hook_add(UC_HOOK_CODE, cb, None, STUB_BASE, STUB_BASE + 16 * NSTUB - 1)
    uc.mem_write(STUB_BASE, b'\xc3' * (16 * NSTUB))


def begin(ctx):
    """Installs the stubs (once), prepares a fresh log, the FPU control word and the game heap handle."""
    from unicorn.x86_const import UC_X86_REG_MXCSR
    m = ctx.m
    # the checker restores only the pages a run wrote, so data the previous generator wrote would leak into this trial
    for addr, old in reversed(LEAK):
        if not (0xA00000 <= addr < 0xB00000):
            m.uc.mem_write(addr, old)
    LEAK.clear()
    if not getattr(m, '_w4v', False):
        _hook(m)
        m._w4v = True
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    try:
        m.uc.reg_write(UC_X86_REG_MXCSR, 0x1f80)
    except Exception:
        pass
    ST['log'] = ctx.buf(4 + LOGMAX * RECW * 4, 'zero')
    ST['ret'] = {}
    ST['halloc'] = lambda n, a: 0
    ST['hfree'] = 1
    w32(ctx, HEAP, ctx.rng.choice([0x00c10000, 0x00c20000, ctx.rng.getrandbits(32) | 1]))
    # tick of the callback list
    w32(ctx, TICK, ctx.rng.choice([0, 1, 100, 1000, ctx.rng.randrange(0, 5000), ctx.rng.getrandbits(32)]))


def seq(r, values=(0, 1, 2, 5, 0x80000000, 0xffffffff)):
    return [r.choice(values) for _ in range(r.randrange(1, 5))]


def pick_alloc(ctx, size, fail=0.2):
    """HeapAlloc answers: a block of at least `size` bytes of random content, or NULL (fail probability)."""
    blk = ctx.buf(max(size, 0) + 32)
    ptr = 0 if ctx.rng.random() < fail else blk
    ST['halloc'] = lambda n, a: ptr
    return blk


def u32(ctx):
    return ctx.u32()


# ---------------------------------------------------------------------------------------------------------
# heap wrappers
def g_alloc_raw(ctx):
    begin(ctx)
    size = ctx.u32()
    pick_alloc(ctx, 64, 0.3)
    return [size]


def g_free_raw(ctx):
    begin(ctx)
    r = ctx.rng
    ST['hfree'] = r.choice([0, 1, 1, 0x55, 0xffffffff])
    return [r.choice([0, ctx.buf(16), r.getrandbits(32)])]


def g_none(ctx):
    begin(ctx)
    w32(ctx, 0x10179d64, ctx.rng.getrandbits(32))
    return []


def _stretch_setup(ctx):
    begin(ctx)
    r = ctx.rng
    h = r.choice([0, 1, 480, 600, r.randrange(0, 2000), 0xffffffff])
    w32(ctx, 0x109e3d14, r.choice([0, 640, 800, r.randrange(0, 2000)]))
    w32(ctx, 0x109e3d18, h)
    w32(ctx, 0x1017b3c0, r.getrandbits(32))
    w32(ctx, 0x1015e9a0, r.choice([0, ctx.buf(16), r.getrandbits(32)]))
    w32(ctx, 0x103729c4, r.getrandbits(32))
    ST['sd'] = r.choice([h, h, h, r.getrandbits(32), 0, (h + 1) & 0xffffffff])


def g_stretch(ctx):
    _stretch_setup(ctx)
    r = ctx.rng
    return [r.choice([0, 3, 100, r.randrange(-50, 700), r.getrandbits(32)]) for _ in range(4)]


def g_libs(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0, 1, 2, 3, 4, 0xffffffff, r.randrange(0, 5)])
    w32(ctx, 0x1017e1bc, n)
    for i in range(1, 6):
        wbytes(ctx, 0x1017dfb8 + 32 * i, bytes(r.choice(b'abcXYZ019 _.\\/') for _ in range(23)) + b'\0')
        w32(ctx, 0x1017dfb8 + 32 * i + 0x18, r.getrandbits(32), r.getrandbits(32))
    fails = set(r.sample(range(0, 12), r.randrange(0, 6)))     # LoadLibraryA fails on these call numbers
    ST['ll'] = lambda nn, a: 0 if nn in fails else 0x60000000 + 0x1000 * nn
    ST['gp'] = lambda nn, a: 0x61000000 + 0x10 * nn
    return []


def g_force_noff(ctx):
    """Force feedback without a device (platform/force.h): the original with FFORCE.DLL missing."""
    out = g_force(ctx)
    w32(ctx, 0x1015ef44, 0)
    return out


def g_force(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1015ef3c, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    w32(ctx, 0x1015ef44, r.choice([0, stub(S_FORCE), stub(S_FORCE)]))
    ST['ret'][S_FORCE] = [r.choice([0, 1, 0xffffffff, 0x80004005, 0x8004050f, 0x7fffffff, r.getrandbits(32)])]
    return []


# ---------------------------------------------------------------------------------------------------------
# 0x100092dd, 0x1000e520
def g_list_remove(ctx):
    begin(ctx)
    r = ctx.rng
    nodes = [ctx.buf(32) for _ in range(r.randrange(0, 6))]
    for i, nd in enumerate(nodes):
        w32(ctx, nd + 0xc, nodes[i + 1] if i + 1 < len(nodes) else 0)
    obj = ctx.buf(0x40)
    head = nodes[0] if nodes else 0
    if r.random() < 0.1:
        head = 0
    cur = r.choice(nodes + [0, ctx.buf(32), nodes[0] if nodes else 0]) if r.random() < 0.9 else 0
    w32(ctx, obj + 0x2c, head)
    w32(ctx, obj + 0x30, cur)
    ST['hfree'] = r.choice([0, 1])
    return [obj]


def g_1000e520(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1015d0a4, 0 if r.random() < 0.8 else r.getrandbits(32) | 1)
    pick_alloc(ctx, 0x374, 0.2)
    return [r.choice([0, 1, 5, 11, 12, 13, 0xffffffff, r.getrandbits(32), r.randrange(0, 20)])]


def g_1002a29f(ctx):
    begin(ctx)
    r = ctx.rng
    idx = r.choice([0, 1, 2, 5, 17, 100, 299, r.randrange(0, 300), -1, -2])
    w32(ctx, 0x1017b8c8, r.choice([idx, idx, r.randrange(0, 300), 0xffffffff]))
    pick_alloc(ctx, 0x2fc, 0.25)
    return [idx]


# ---------------------------------------------------------------------------------------------------------
# 0x10034b30, 0x10034c11
def _screen(ctx):
    r = ctx.rng
    w32(ctx, 0x1017b918, r.choice([640, 800, 1, 0, 3, 0x7fffffff, 0x80000000, 0xffffffff, r.randrange(0, 2000), r.getrandbits(32)]))
    w32(ctx, 0x1017b91c, r.choice([480, 600, 1, 0, 5, 0xffffffff, r.randrange(0, 2000), r.getrandbits(32)]))


def g_10034b30(ctx):
    begin(ctx)
    _screen(ctx)
    pick_alloc(ctx, 0x18, 0.25)
    return []


def g_10034c11(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1017b918, r.choice([640, 800, 320, r.randrange(1, 2000)]))
    w32(ctx, 0x1017b91c, r.choice([480, 600, 240, r.randrange(1, 2000)]))
    w32(ctx, 0x1014cae0, ctx.buf(0x60))        # Camera_SetAspect stores into this object
    w32(ctx, 0x1014d1b8, *[r.randrange(0, 700) for _ in range(6)])
    pick_alloc(ctx, 0x18, 0.15)
    return []


# ---------------------------------------------------------------------------------------------------------
# 0x1003730b, 0x10072d88, 0x100a9017: owner records
def g_owner(ctx):
    begin(ctx)
    r = ctx.rng
    owner = ctx.buf(0x40)
    pick_alloc(ctx, 0x1e9d, 0.2)
    return [r.choice([0, 1, r.getrandbits(32)]), owner]


# 0x10038fac
def g_10038fac(ctx):
    begin(ctx)
    r = ctx.rng
    keys = [ctx.buf(0x90) for _ in range(4)]
    nodes = [ctx.buf(0x6a0) for _ in range(r.randrange(1, 4))]
    for i, nd in enumerate(nodes):
        w32(ctx, nd, r.choice(keys[:3]) if r.random() < 0.7 else r.getrandbits(32))
        w32(ctx, nd + 0x690, nodes[i + 1] if i + 1 < len(nodes) else 0)
    w32(ctx, 0x109e3760, nodes[0] if r.random() < 0.97 else 0)
    key = r.choice(keys)
    rec = ctx.buf(0x700)
    table = ctx.buf(0x24 * 40 + 16)
    w32(ctx, rec + 0x8c, table)
    w32(ctx, rec + 0x5ff, r.getrandbits(32))
    src = ctx.buf(0x200)
    start = r.choice([0, 0, 1, 3, 7])
    w32(ctx, src + 0x108, start)
    w32(ctx, src + 0x38, start + r.choice([0, 1, 2, 5, 9]) if r.random() < 0.9 else r.randrange(-2, 3))
    for i in range(40):
        w32(ctx, src + 0x3c + 4 * i, r.randrange(0, 40))
    pick_alloc(ctx, 0x698, 0.03)
    return [r.getrandbits(32), key, rec, src]


# 0x10048112 (the original leaves the caller's eax in the top half of some results: a trampoline clears eax first)
def g_10048112(ctx):
    begin(ctx)
    r = ctx.rng
    code = bytes([0x31, 0xC0, 0xE9]) + struct.pack('<I', (0x10048112 - (TRAMP + 7)) & 0xffffffff)
    ctx.m.uc.mem_write(TRAMP, code)
    w32(ctx, 0x1015f1c8, r.choice([0, 0, 1, 3, 6, 6, 7, 8, 0xffffffff, 0xfffffffe, r.randrange(0, 10)]))
    d = ctx.buf(8)
    wbytes(ctx, d, struct.pack('<h', r.choice([1, 2, 5, 100, 0x7fff, 0, -1, -32768, r.randrange(-50, 50)])))
    pick_alloc(ctx, 0x1c, 0.15)
    return [r.getrandbits(32), r.getrandbits(32), r.choice([1, 1, 1, 1, 0, 2, 0xffffffff, r.getrandbits(32)]), d]


# 0x10053132, 0x10056d04
def g_10053132(ctx):
    begin(ctx)
    r = ctx.rng
    nodes = [ctx.buf(16) for _ in range(r.randrange(0, 6))]
    for i, nd in enumerate(nodes):
        w32(ctx, nd, nodes[i + 1] if i + 1 < len(nodes) else 0)
    w32(ctx, 0x1015f3b0, nodes[0] if nodes else 0)
    w32(ctx, 0x109b6880, r.getrandbits(32))
    ST['hfree'] = r.choice([0, 1])
    return []


def g_10056d04(ctx):
    begin(ctx)
    r = ctx.rng

    def blk():
        return r.choice([0, 0, r.getrandbits(32) | 0x10000, ctx.buf(16)])

    obj = ctx.buf(0x80)
    w32(ctx, obj + 0x60, blk())
    w32(ctx, obj + 0x64, blk())
    for off in (0x68, 0x6c):
        cell = r.choice([0, ctx.buf(8)])
        if cell:
            w32(ctx, cell, blk())
        w32(ctx, obj + off, cell)
    w32(ctx, obj + 0x70, blk())
    ST['hfree'] = r.choice([0, 1, 7])
    return [obj if r.random() > 0.15 else 0]


# 0x10056dfa
NICE = [0.5, 1.0, 2.0, 0.001, 1000.0, -1.0, 65536.0, 0.00001, -0.75, 0.25, 3.0]


def _dbl(r):
    d = r.choice(NICE + [r.uniform(-3, 3), r.uniform(-1e4, 1e4)])
    return struct.unpack('<2I', struct.pack('<d', d))


def g_10056dfa(ctx):
    begin(ctx)
    r = ctx.rng
    cfg = ctx.buf(0x88)
    flags = r.choice([0, 0x40, 4, 8, 0x10, 0x20, 0x3c, 0x7c, 0x7c, r.getrandbits(8), 0x44, 0x4c, 0x1c])
    w32(ctx, cfg, r.choice([0, 1, 2, 15, r.getrandbits(32)]), flags)
    for i in range(2, 0xe):
        if i < 8:
            val = r.choice([0, 1000, 0x7fff, 32768, r.randrange(0, 70000)])       # centres
        else:
            val = r.choice([0, 5, 100, 1000, r.randrange(0, 20000), 0xfffffffd])  # dead zones
        w32(ctx, cfg + 4 * i, val)
    for i in range(0xe, 0x1a, 2):
        w32(ctx, cfg + 4 * i, *_dbl(r))
    for i in range(0x1a, 0x1e):
        w32(ctx, cfg + 4 * i, r.randrange(0, 6))
    for i in range(0x1e, 0x22):
        w32(ctx, cfg + 4 * i, r.getrandbits(32))
    axes = ctx.buf(32)
    buttons = ctx.buf(40)
    on = 1 if r.random() < 0.9 else 0
    w32(ctx, 0x1017b920, on)
    ST['joy_flags'] = flags
    ST['joy_ret'] = r.choice([0, 0, 0, 0, 0, 1, 0xa9])
    cx = [r.choice([0, 500, 1000, 0x7fff, 32768, r.randrange(0, 70000), r.getrandbits(32)]) for _ in range(6)]
    pov = r.choice([0, 9000, 18000, 27000, 0xffff, 0xffffffff, 4500, r.getrandbits(32)])
    ST['joy_data'] = struct.pack('<13I', 0x34, 0, *cx, r.getrandbits(32), r.randrange(0, 32), pov, 0, 0)
    api_cfg = [cfg, axes, buttons]
    if r.random() < 0.06:
        api_cfg[1] = 0
    if r.random() < 0.06:
        api_cfg[2] = 0
    return api_cfg


def _joy(n, args, uc):
    esp = uc.reg_read(UC_X86_REG_ESP)
    ptr = r32(uc, esp + 8)
    cur = bytes(uc.mem_read(ptr, 0x34))
    want = struct.pack('<13I', 0x34, ST['joy_flags'], *([0] * 11))
    if cur != want:                       # the caller must pass a cleared JOYINFOEX with dwSize and dwFlags set
        return 0x99
    uc.mem_write(ptr, ST['joy_data'])
    return ST['joy_ret']


# ---------------------------------------------------------------------------------------------------------
# 0x1008c550, 0x1008c6f5, 0x1008c803
def g_1008c550(ctx):
    begin(ctx)
    r = ctx.rng
    cnt = r.choice([0, 1, 2, 10, 40, 256, -1, -5, r.randrange(0, 60)])
    src = ctx.buf(max(cnt, 0) * 3 + 16)
    rec = ctx.buf(0x20)
    pick_alloc(ctx, max(cnt, 0) * 3, 0.2)
    return [rec, src, r.getrandbits(32), cnt]


def g_1008c6f5(ctx):
    begin(ctx)
    r = ctx.rng
    cnt = r.choice([0, 1, 2, 10, 40, 100, 255, 256, 257, 300, -1, r.randrange(0, 60)])
    first = r.choice([0, 0, 1, 7, 100, r.randrange(0, 20)])
    n = (first + max(cnt, 0)) * 3 + 16
    a = ctx.buf(n)
    b = ctx.buf(n)
    rec = ctx.buf(0x20)
    pick_alloc(ctx, max(cnt, 0) * 12, 0.2)
    return [rec, a, b, first, cnt, r.choice([0, 1, 5, 64, r.getrandbits(32)])]


def g_1008c803(ctx):
    begin(ctx)
    r = ctx.rng
    steps = r.choice([1, 2, 3, 5, 10, 30, 64, 0x7fffffff])
    cur = r.choice([steps - 1, steps - 1, steps - 1, 0, r.randrange(-3, steps + 3)])
    cnt = r.choice([0, 1, 5, 20, 60, r.randrange(0, 30)])
    first = r.randrange(0, 40)
    rec = ctx.buf(0x28)
    colours = ctx.buf(first + cnt * 3 + 32)
    deltas = ctx.buf(cnt * 3 + 16)
    accum = ctx.buf(cnt * 6 + 16)
    zero_ok = cnt == 0 or cur >= steps
    w32(ctx, rec, colours if not zero_ok or r.random() < 0.85 else 0)
    w32(ctx, rec + 4, deltas if not zero_ok or r.random() < 0.85 else 0)
    w32(ctx, rec + 8, accum if not zero_ok or r.random() < 0.85 else 0)
    w32(ctx, rec + 0xc, steps)
    w32(ctx, rec + 0x10, cur)
    wbytes(ctx, rec + 0x14, bytes([first]) + struct.pack('<I', cnt))
    table = ctx.buf(0x40)
    w32(ctx, table + 0x18, stub(S_DRIVER))
    w32(ctx, 0x1015e688, table)
    ST['hfree'] = r.choice([0, 1])
    return [rec]


# ---------------------------------------------------------------------------------------------------------
# the callback list (0x100cc6ea.., wrappers 0x100b41dc..)
def mknodes(ctx, n, null_ok=True):
    r = ctx.rng
    nodes = [ctx.buf(0x18) for _ in range(n)]
    for i, nd in enumerate(nodes):
        cbk = r.choice([stub(S_CB0), stub(S_CB1), stub(S_CB2), stub(S_CB0)] + ([0] if null_ok else []))
        t = int.from_bytes(bytes(ctx.m.uc.mem_read(TICK, 4)), 'little')
        w32(ctx, nd, cbk, r.choice([0, 0, ctx.buf(8), r.getrandbits(32)]), r.choice([0, 1, 5, 100, r.randrange(0, 300)]),
            r.getrandbits(32), (t + r.randrange(-4, 5)) & 0xffffffff, nodes[i + 1] if i + 1 < n else 0)
    return nodes


def cbseq(ctx):
    r = ctx.rng
    for k in (S_CB0, S_CB1, S_CB2):
        ST['ret'][k] = seq(r, (0, 1, 1, 1, 2, 0xffffffff))


def g_100cc6ea(ctx, listaddr=None):
    begin(ctx)
    r = ctx.rng
    cbseq(ctx)
    nodes = mknodes(ctx, r.randrange(0, 4))
    lst = ctx.buf(8)
    w32(ctx, lst, nodes[0] if nodes else 0)
    pick_alloc(ctx, 0x18, 0.15)
    cb = r.choice([stub(S_CB0), stub(S_CB1), stub(S_CB2), 0])
    return [lst, cb, r.choice([0, 1, 7, 100, r.getrandbits(32)]), r.getrandbits(32)]


def g_100cc79a(ctx):
    begin(ctx)
    r = ctx.rng
    cbseq(ctx)
    nodes = mknodes(ctx, r.randrange(0, 5), null_ok=False)
    lst = ctx.buf(8)
    w32(ctx, lst, nodes[0] if nodes else 0)
    other = mknodes(ctx, 1, null_ok=False)[0]
    node = r.choice(nodes + [other, 0]) if nodes else r.choice([other, 0])
    ST['hfree'] = r.choice([0, 1])
    return [lst, node]


def g_100cc830(ctx):
    begin(ctx)
    r = ctx.rng
    cbseq(ctx)
    nodes = mknodes(ctx, r.randrange(0, 5), null_ok=False)
    lst = ctx.buf(8)
    w32(ctx, lst, nodes[0] if nodes else 0)
    return [lst]


def g_100cc8c5(ctx):
    begin(ctx)
    r = ctx.rng
    cbseq(ctx)
    nodes = mknodes(ctx, r.randrange(0, 6))
    lst = ctx.buf(8)
    w32(ctx, lst, nodes[0] if nodes else 0)
    w32(ctx, 0x1017e5b8, r.getrandbits(32))
    return [lst]


RECS = 0x101a2100


def _records(ctx, idx, nodes_per=None):
    """Prepares the 0x88-byte record `idx` of the table at 0x101a2100: flags at +0x18 and the list head at +0x84."""
    r = ctx.rng
    nodes = mknodes(ctx, r.randrange(0, 4) if nodes_per is None else nodes_per, null_ok=True)
    w32(ctx, RECS + idx * 0x88 + 0x84, nodes[0] if nodes else 0)
    w32(ctx, RECS + idx * 0x88 + 0x18, r.choice([0, 0x800, 0x1, 0xfffff7ff, r.getrandbits(32)]))
    return nodes


def g_100b41dc(ctx):
    begin(ctx)
    r = ctx.rng
    cbseq(ctx)
    idx = r.randrange(0, 8)
    _records(ctx, idx)
    pick_alloc(ctx, 0x18, 0.15)
    cb = r.choice([stub(S_CB0), stub(S_CB1), stub(S_CB2), 0])
    return [idx, cb, r.choice([0, 1, 7, 100, r.getrandbits(32)]), r.getrandbits(32)]


def g_100b4213(ctx):
    begin(ctx)
    r = ctx.rng
    cbseq(ctx)
    n = r.choice([0, 1, 2, 4, 7, r.randrange(0, 8), 0xffffffff])
    w32(ctx, 0x1017d218, n)
    for i in range(8):
        _records(ctx, i)
    return []


def g_100b427b(ctx):
    begin(ctx)
    r = ctx.rng
    cbseq(ctx)
    idx = r.randrange(0, 8)
    _records(ctx, idx)
    mknodes_fix(ctx, idx)       # nodes with a NULL callback would jump to address 0 (a fault in both versions)
    nodes = list_nodes(ctx, idx)
    other = mknodes(ctx, 1, null_ok=False)[0]
    ST['hfree'] = r.choice([0, 1])
    return [idx, r.choice(nodes + [other, 0])]


def mknodes_fix(ctx, idx):
    r = ctx.rng
    for n in list_nodes(ctx, idx):
        if int.from_bytes(bytes(ctx.m.uc.mem_read(n, 4)), 'little') == 0:
            w32(ctx, n, stub(r.choice([S_CB0, S_CB1, S_CB2])))
    return []


def list_nodes(ctx, idx):
    out = []
    n = int.from_bytes(bytes(ctx.m.uc.mem_read(RECS + idx * 0x88 + 0x84, 4)), 'little')
    while n and len(out) < 20:
        out.append(n)
        n = int.from_bytes(bytes(ctx.m.uc.mem_read(n + 0x14, 4)), 'little')
    return out


def g_100b42aa(ctx):
    begin(ctx)
    r = ctx.rng
    cbseq(ctx)
    idx = r.randrange(0, 8)
    _records(ctx, idx)
    mknodes_fix(ctx, idx)
    return [idx]


# ---------------------------------------------------------------------------------------------------------
# 0x100da45c, 0x100db351
def g_100da45c(ctx):
    begin(ctx)
    r = ctx.rng
    b60, b48 = ctx.buf(0x60 + 16), ctx.buf(0x48 + 16)
    ST['halloc'] = lambda n, a: b60 if a[2] == 0x60 else b48
    w32(ctx, 0x1017e6bc, r.getrandbits(32))
    return []


def g_100db351(ctx):
    begin(ctx)
    r = ctx.rng

    def p():
        return r.choice([0, 0, r.getrandbits(32) | 0x10000])

    w32(ctx, 0x1017e688, p())
    w32(ctx, 0x1017e68c, p())
    w32(ctx, 0x1017ee40, *[p() for _ in range(16)])
    ST['hfree'] = r.choice([0, 1])
    return []


API = {
    'HeapAlloc': lambda n, a: ST['halloc'](n, a),
    'HeapFree': lambda n, a: ST['hfree'],
    'StretchDIBits': lambda n, a: ST['sd'],
    'LoadLibraryA': lambda n, a: ST['ll'](n, a),
    'GetProcAddress': lambda n, a: ST['gp'](n, a),
    'joyGetPosEx': _joy,
}

SPECS = [
    dict(name='Heap_AllocRaw', orig=0x100b659a, c='Heap_AllocRaw', gen=g_alloc_raw, trials=300, api=API),
    dict(name='Heap_FreeRaw', orig=0x100a7800, c='Heap_FreeRaw', gen=g_free_raw, trials=300, api=API),
    dict(name='Sim_100a77ef', orig=0x100a77ef, c='Sim_100a77ef', gen=g_alloc_raw, trials=300, api=API),
    # deleted: dead (SDL3 platform/w28 version or unused; DECISIONS.md "Windows calls")
    # dict(name='Mouse_Unclip', orig=0x1007d948, c='Mouse_Unclip', gen=g_none, trials=100, api=API),
    # superseded: the loading screen draws through Vulkan (DECISIONS.md, "The loading screen through Vulkan"; checked natively against GDI by tools/verify/picture_native.c)
    # dict(name='Sim_10083de1', orig=0x10083de1, c='Sim_10083de1', gen=g_stretch, trials=400, api=API),
    # deleted: no caller and no pointer in the DLL (DECISIONS.md, "Windows calls: the screen")
    # dict(name='Sim_10083e4e', orig=0x10083e4e, c='Sim_10083e4e', gen=g_stretch, trials=400, api=API),
    # superseded: the unpackers are ours, no DLL is loaded (DECISIONS.md "Windows calls: the decompressors";
    # checked natively by tools/verify/unpack_native.c against pkware.dll / zlib.dll on every packed entry)
    # dict(name='Sim_100bcc50', orig=0x100bcc50, c='Sim_100bcc50', gen=g_libs, trials=400, api=API),
    # superseded: FFORCE.DLL is platform/force.h now (DECISIONS.md "Force feedback kept for rumble")
    # dict(name='Force_ExitSystem', orig=0x10045e19, c='Force_ExitSystem', gen=g_force, trials=400, api=API),
    dict(name='Force_ExitSystem_noff', orig=0x10045e19, c='Force_ExitSystem', gen=g_force_noff, trials=400, api=API),
    dict(name='Sim_100092dd', orig=0x100092dd, c='Sim_100092dd', gen=g_list_remove, trials=500, void=True, api=API),
    dict(name='Sim_1000e520', orig=0x1000e520, c='Sim_1000e520', gen=g_1000e520, trials=400, api=API),
    dict(name='Sim_1002a29f', orig=0x1002a29f, c='Sim_1002a29f', gen=g_1002a29f, trials=400, api=API),
    dict(name='Sim_10034b30', orig=0x10034b30, c='Sim_10034b30', gen=g_10034b30, trials=500, api=API),
    dict(name='Sim_10034c11', orig=0x10034c11, c='Sim_10034c11', gen=g_10034c11, trials=500, api=API),
    dict(name='Sim_1003730b', orig=0x1003730b, c='Sim_1003730b', gen=g_owner, trials=400, api=API),
    dict(name='Sim_10072d88', orig=0x10072d88, c='Sim_10072d88', gen=g_owner, trials=400, api=API),
    dict(name='Sim_100a9017', orig=0x100a9017, c='Sim_100a9017', gen=g_owner, trials=400, api=API),
    dict(name='Sim_10038fac', orig=0x10038fac, c='Sim_10038fac', gen=g_10038fac, trials=600, void=True, api=API),
    dict(name='Sim_10048112', orig=TRAMP, c='Sim_10048112', gen=g_10048112, trials=600, api=API),
    dict(name='Sim_10053132', orig=0x10053132, c='Sim_10053132', gen=g_10053132, trials=300, void=True, api=API),
    # dict(name='Sim_10056d04', orig=0x10056d04, c='Sim_10056d04', gen=g_10056d04, trials=400, api=API),   # deleted: no joystick (DECISIONS.md "dropped features")
    # dict(name='Sim_10056dfa', orig=0x10056dfa, c='Sim_10056dfa', gen=g_10056dfa, trials=1000, api=API),   # deleted: no joystick (DECISIONS.md "dropped features")
    dict(name='Sim_1008c550', orig=0x1008c550, c='Sim_1008c550', gen=g_1008c550, trials=400, api=API),
    dict(name='Sim_1008c6f5', orig=0x1008c6f5, c='Sim_1008c6f5', gen=g_1008c6f5, trials=500, api=API),
    dict(name='Sim_1008c803', orig=0x1008c803, c='Sim_1008c803', gen=g_1008c803, trials=800, void=True, api=API),
    dict(name='Sim_100cc6ea', orig=0x100cc6ea, c='Sim_100cc6ea', gen=g_100cc6ea, trials=600, api=API),
    dict(name='Sim_100cc79a', orig=0x100cc79a, c='Sim_100cc79a', gen=g_100cc79a, trials=600, void=True, api=API),
    dict(name='Sim_100cc830', orig=0x100cc830, c='Sim_100cc830', gen=g_100cc830, trials=500, void=True, api=API),
    dict(name='Sim_100cc8c5', orig=0x100cc8c5, c='Sim_100cc8c5', gen=g_100cc8c5, trials=800, void=True, api=API),
    dict(name='Sim_100b41dc', orig=0x100b41dc, c='Sim_100b41dc', gen=g_100b41dc, trials=500, api=API),
    dict(name='Sim_100b4213', orig=0x100b4213, c='Sim_100b4213', gen=g_100b4213, trials=500, void=True, api=API),
    dict(name='Sim_100b427b', orig=0x100b427b, c='Sim_100b427b', gen=g_100b427b, trials=500, void=True, api=API),
    dict(name='Sim_100b42aa', orig=0x100b42aa, c='Sim_100b42aa', gen=g_100b42aa, trials=500, void=True, api=API),
    dict(name='Sim_100da45c', orig=0x100da45c, c='Sim_100da45c', gen=g_100da45c, trials=200, void=True, api=API),
    dict(name='Sim_100db351', orig=0x100db351, c='Sim_100db351', gen=g_100db351, trials=300, void=True, api=API),
]
