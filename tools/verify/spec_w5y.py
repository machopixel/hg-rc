# Differential tests for round 5, letter y (see difftest.py and docs/checker.md).
#
# The functions call helpers that are not converted yet through their original address; the checker runs the
# original code for those on both sides, so the call is proven by identical arguments, results and memory.
# Windows imports are stubbed by the checker (call log compared). Functions that return a float in st(0) are
# called through a machine-code trampoline (original) or a C wrapper (w5y_test.c) that stores st(0) at OUTF.
import struct
import json
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG

SOURCES = json.loads(r'''
["sim/unsorted_1000/w5y_misc.c", "sim/unsorted_1001/w5y_misc.c", "sim/unsorted_1002/w5y_misc.c", "sim/unsorted_1003/w5y_misc.c", "sim/unsorted_1004/w5y_misc.c", "sim/unsorted_1005/w5y_misc.c", "sim/unsorted_1006/w5y_misc.c", "sim/unsorted_1008/w5y_misc.c", "sim/unsorted_1009/w5y_misc.c", "sim/unsorted_100a/w5y_misc.c", "sim/unsorted_100b/w5y_misc.c", "sim/unsorted_100c/w5y_misc.c", "sim/unsorted_100d/w5y_misc.c", "sim/unsorted_1003/w2e_mono.c", "sim/unsorted_100f/w2e_string.c", "sim/unsorted_1000/w2e_names.c", "sim/unsorted_1000/w2a_fields.c", "sim/unsorted_1000/w2g_fields.c", "sim/unsorted_1000/w2l_fields.c", "sim/unsorted_1000/w2j_slots.c", "sim/unsorted_1001/w2j_misc.c", "sim/unsorted_1001/w2e_tables.c", "sim/unsorted_1001/w4u_cd.c", "sim/view/w2l_lists.c", "sim/unsorted_1001/w3t_misc.c", "../tools/verify/w5y_test.c", "sim/win/w4u_cdaudio.c", "sim/view/objlist.c", "sim/util/field.c", "sim/unsorted_1001/w2e_names.c", "sim/util/lookup.c", "sim/util/ring.c", "sim/unsorted_100c/w2e_tables.c", "sim/unsorted_100d/w2f_records.c"]
''')
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

OUTF = 0x00AFF000       # float result of the st(0) function (inside argument memory, compared like all memory)
TRAMP = 0x00AF0000      # machine code in argument memory (rewritten for every trial)
ARENA = 0x00A80000      # blocks that the HeapAlloc stub hands out


# ---- helpers -------------------------------------------------------------------------------------------
def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def rnd(ctx, n):
    return bytes(ctx.rng.getrandbits(8) for _ in range(n))


def rw(ctx):
    r = ctx.rng
    return r.choice([0, 1, 0xffffffff, 0x80000000, 0x7fffffff, r.getrandbits(32), r.getrandbits(32), r.randrange(0, 64)])


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def rfl(ctx):
    """A float (as a bit pattern): ordinary values, zeros, tiny and huge, both signs."""
    r = ctx.rng
    c = r.random()
    if c < 0.5:
        return fbits(r.uniform(-1000, 1000))
    if c < 0.7:
        return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, 100.0, 3840.0, 1e-7, 1e-8, 2e-7, 180.0, -180.0, 360.0]))
    if c < 0.85:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-12, 12))
    return r.getrandbits(32) & 0xff7fffff if r.random() < 0.7 else fbits(r.uniform(-5, 5))


LOG = 0x00AFE000        # log of the stubbed callees: count, then (address, arguments...) per call
ARENA2 = 0x00A90000     # blocks that the stubbed malloc hands out
CODE = 0x00AF1000       # small routines that data pointers point to (mov eax, imm; ret), rewritten for every trial

# Callees that are too deep to run with generated data (rendering, loaders, COM) are replaced by a stub in the
# emulator for the specs that ask for it (with_stubs): the same stub at the same address for the original and the
# C run; it logs its arguments into LOG and returns a value that depends only on the call number. The stub is also
# used to check the argument order of wrappers with many arguments ("spy" specs): the C code must pass the same
# argument words as the original. address -> number of argument words to log.
STUB_ARGS = {
    0x10039aca: 2, 0x1009e1ef: 0, 0x100d3510: 3, 0x1004e838: 0, 0x1000ae00: 2, 0x100fef30: 1, 0x10099c36: 0,
    0x100999fc: 0, 0x100ca670: 3, 0x1000e737: 12, 0x100246b3: 12, 0x10060220: 8, 0x10105fe0: 3, 0x100a796c: 3,
    0x1001f790: 3, 0x10030c87: 3, 0x100feb20: 3, 0x1005af06: 2, 0x1010d180: 1, 0x100ab0d6: 2, 0x10060d2e: 2,
    0x10090d15: 1, 0x100c6eae: 2, 0x1005259e: 2, 0x100aa742: 1, 0x10088a2d: 1,
    0x100461e8: 2, 0x10010564: 2, 0x10010632: 2, 0x100e46d0: 2,
}
STUB_STR = {0x100461e8: 0}      # stubs that also log the text one of their arguments points to (stack buffers)
WANT = set()        # stubs the next generator asks for (consumed by begin)
ACTIVE = set()      # stubs in force for the trial that is being generated and run


STATE = {}


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def stub_ret(address, n, args):
    """The value a stub returns: depends on the trial's seed, the stub and the position in the log only."""
    h = mix(STATE.get('seed', 0), address, n)
    if address == 0x100fef30:                       # malloc: blocks in argument memory, now and then 0
        return 0 if h % 5 == 3 else ARENA2 + 0x100 * (h % 8)
    if address == 0x100c6eae:                       # a record number or -1
        return [0xffffffff, 0, 1, 2, 3][h % 5]
    if address == 0x100feb20:                       # fseek: 0 or -1
        return [0, 0, 0xffffffff][h % 3]
    return h


def install_stubs(m):
    if getattr(m, '_w5y_stubs', False):
        return
    m._w5y_stubs = True
    raw = m.raw

    def cb(uc, address, size, ud):
        if address not in ACTIVE:
            return
        esp = uc.reg_read(UC_X86_REG_ESP)
        k = STUB_ARGS[address]
        args = struct.unpack('<%dI' % k, bytes(uc.mem_read(esp + 4, 4 * k))) if k else ()
        n = r32(uc, LOG)
        extra = b''
        if address in STUB_STR:
            text = b''
            while len(text) < 120:
                c = bytes(uc.mem_read(args[STUB_STR[address]] + len(text), 1))
                if c == b'\0':
                    break
                text += c
            extra = text + b'\0' * (4 - len(text) % 4)
            args = tuple(0 if i == STUB_STR[address] else a for i, a in enumerate(args))      # the pointer itself means nothing
        if n < 900:
            uc.mem_write(LOG + 4 + 4 * n, struct.pack('<%dI' % (1 + k), address, *args) + extra)
            uc.mem_write(LOG, struct.pack('<I', n + 1 + k + len(extra) // 4))
        uc.reg_write(UC_X86_REG_EAX, stub_ret(address, n, args))
        # return from the stubbed function (cdecl: the caller removes the arguments); code bytes are not patched
        # because the emulator may have translated the original code already
        uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)

    for a in STUB_ARGS:
        raw.hook_add(UC_HOOK_CODE, cb, None, a, a)


def with_stubs(gen, *addrs):
    def g(ctx):
        WANT.clear()
        WANT.update(addrs)
        return gen(ctx)
    return g


def begin(ctx):
    """Maps the text screen once, sets the x87 state like the Windows control word (53 bit precision, empty register
    stack), the heap handle and the stub log. Must be the first thing a generator does."""
    m = ctx.m
    if not getattr(m, 'mono_mapped', False):
        m.uc.mem_map(0xb0000, 0x10000)         # the original writes to physical 0xb0000 (text screen)
        m.wr.append((0xb0000, 0x10000))
        m.mono_mapped = True
        m.snap0 = m.dump()
        m.w5y_fix = True
    elif getattr(m, 'w5y_fix', False):
        # the checker rebuilt its page index after the first trial and forgot which pages that trial dirtied
        m.restore(m.snap0)
        m.w5y_fix = False
    install_stubs(m)
    ACTIVE.clear()
    ACTIVE.update(WANT)
    WANT.clear()
    uc = m.uc
    STATE['seed'] = ctx.rng.getrandbits(32)
    uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    uc.reg_write(UC_X86_REG_FPSW, 0)
    uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    uc.mem_write(LOG, bytes(8))
    uc.mem_write(0x00A7F000, bytes(0x100))          # the table object the cache lookup reads (0x1017d604)
    w32(ctx, 0x1017d604, 0x00A7F000)
    w32(ctx, 0x1017b8f8, ctx.rng.choice([0x00c10000, 0x00c10000, ctx.rng.getrandbits(32)]))
    uc.mem_write(ARENA, bytes(ctx.rng.getrandbits(8) for _ in range(0x1800)))
    return ctx.rng


def mono(ctx):
    """Text screen content and the switch that enables it (begin() has mapped it)."""
    wb(ctx, 0xb0000, rnd(ctx, 0x10000))
    w32(ctx, 0x10a3ade8, ctx.rng.choice([0, 1, 1, 1, 0x100]))


def code_stub(ctx, slot, value):
    """A routine `mov eax, value; ret` in argument memory; returns its address."""
    a = CODE + 0x10 * slot
    wb(ctx, a, b'\xb8' + struct.pack('<I', value & 0xffffffff) + b'\xc3')
    return a


def newbuf(ctx, size, fill='rand'):
    return ctx.buf(size, fill)


def ptr_or_null(ctx, p, pnull=0.1):
    return 0 if ctx.rng.random() < pnull else p


def h_alloc(n, a):
    return 0 if n % 7 == 3 else ARENA + 0x800 * (n % 3)


API = {'HeapAlloc': h_alloc}


def g_none(ctx):
    begin(ctx)
    return []


def slot_world(ctx, players=True):
    """Pointer table 0x109e8b40 with records, the current index, the group slot counts (like spec_w2j)."""
    rg = begin(ctx)
    count = rg.randrange(1, 8)
    group = rg.randrange(0, 3)
    cur = rg.randrange(0, count)
    pc = rg.choice([0.2, 0.7, 1.0, 1.0])
    recs = []
    for i in range(count):
        rec = newbuf(ctx, 0x600)
        g = group if (i == cur or rg.random() < 0.8) else rg.randrange(0, 3)
        w32(ctx, rec + 4, i if rg.random() < 0.9 else rg.randrange(0, 12))
        w32(ctx, rec + 8, g)
        w32(ctx, rec + 0x10, i + 1 if rg.random() < 0.8 else rg.randrange(0, 12))
        w16(ctx, rec + 0x1a6, 0xc if rg.random() < pc else rg.choice([0, 1, 5, 0xffff]))
        if players:
            pl = newbuf(ctx, 0x600)
            w32(ctx, pl, newbuf(ctx, 0x200))
            w32(ctx, pl + 0x4f0, rg.choice([2, 2, 2, 0, 1, 3]))
            w32(ctx, pl + 0x50c, rg.choice([0, 1, 2]))
            w32(ctx, rec + 0x28, pl)
        recs.append(rec)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
    w32(ctx, 0x1015dc64, count if rg.random() < 0.9 else rg.randrange(0, count + 1))
    w32(ctx, 0x1017b8c8, cur)
    for g in range(3):
        w32(ctx, 0x109b7d44 + g * 0x6c, rg.choice([0, 1, 2, 3, 4, 5, 6, 7, 3, 4]))
    return recs, cur


# ---- 0x10008197, 0x100081dd -------------------------------------------------------------------------
def g_showcount(ctx):
    rg = begin(ctx)
    mono(ctx)
    w32(ctx, 0x109f13e0, rg.choice([0, 1, 7, 42, 999, 12345, 99999, 0xffffffff, 0xffffffd6, rg.randrange(0, 100000),
                                    rg.randrange(0, 100000), (-rg.randrange(1, 9999)) & 0xffffffff]))
    return []


def g_clearcount(ctx):
    begin(ctx)
    mono(ctx)
    return []


# ---- 0x100094ba ---------------------------------------------------------------------------------------
def g_94ba(ctx):
    rg = begin(ctx)
    obj = newbuf(ctx, 0xb0)
    n = rg.choice([1, 1, 2, 3, 4])
    rec = newbuf(ctx, 0x18 + 0x58 * 5)
    w16(ctx, rec + 4, n)
    for k in range(5):
        wf(ctx, rec + 0x18 + 0x58 * k + 0xc, *[rg.uniform(-100, 100) for _ in range(3)])
    w32(ctx, obj + 0x2c, rec if rg.random() > 0.1 else 0)
    w32(ctx, obj + 0x30, rg.choice([0, 1, 1, 1, rw(ctx)]))
    wb(ctx, obj + 0xa0, rg.choice([b'w\0', b'W\0', b'x\0', b'\0', b'Q', b'H\0']) + rnd(ctx, 6))
    return [obj]


# ---- 0x1000e93e ---------------------------------------------------------------------------------------
def g_e93e(ctx):
    rg = begin(ctx)
    block = newbuf(ctx, 0x800)
    w32(ctx, 0x1015d0a4, block if rg.random() > 0.08 else 0)
    w32(ctx, block, rg.choice([0, 1, 1, 1]))
    w32(ctx, 0x109e369c, rg.choice([0, 1, 1, 1, 1]))
    w32(ctx, 0x101321b4, rg.choice([0, 1, 100, 10000, 0x7fffffff, rw(ctx)]))
    w32(ctx, 0x1015dc78, rg.randrange(0, 5000))
    w32(ctx, 0x1015dc80, rg.randrange(0, 5000))
    n = rg.choice([0, 1, 3, 6])
    w32(ctx, 0x10a38de0, n)
    for i in range(12):
        nm = b'AFXHDMLG' if (rg.random() < 0.3 and 1 <= i <= n + 1) else bytes(rg.choice(b'abc') for _ in range(6))
        wb(ctx, 0x10a39200 + i * 0x14, struct.pack('<I', rg.choice([0, 1, 1, 2])) + nm + b'\0' + rnd(ctx, 7))
    return []


# ---- 0x10010a73, 0x10010b38 ---------------------------------------------------------------------------
def g_mode(ctx):
    recs, cur = slot_world(ctx)
    ctx.m.uc.mem_write(0x1015d100, struct.pack('<I', ctx.rng.choice([0, 0, 0, 1, 1, 2, 2, 3, 0xffffffff, 7]) & 0xffffffff))
    for i in range(8):
        w32(ctx, 0x1012ff58 + 4 * i, rw(ctx))
    return []


# ---- 0x1001403a ---------------------------------------------------------------------------------------
def obj_world(ctx):
    """An object for the 0x100cb4fb / 0x100ca670 family: the pointer table, tables of the lookup helpers."""
    recs, cur = slot_world(ctx)
    rg = ctx.rng
    obj = rg.choice(recs) if rg.random() < 0.7 else newbuf(ctx, 0x600)
    if obj not in recs:
        w32(ctx, obj + 4, rg.randrange(0, 12))
        w32(ctx, obj + 0x28, recs[0] + 0 if False else newbuf(ctx, 0x600))
    w32(ctx, 0x1015dc68, rg.randrange(0, 8))
    w32(ctx, 0x1017e5a8, rg.randrange(0, 8))
    return obj


def g_1403a(ctx):
    rg = ctx.rng
    obj = obj_world(ctx)
    cmd = (rg.choice([0x100, 0x200, 0x400, 0x300, 0]) | rg.randrange(0, 12))
    return [obj, rg.choice([cmd, cmd, cmd | 0x1000, rg.getrandbits(16)]) | (rg.getrandbits(16) << 16 if rg.random() < 0.5 else 0)]


# ---- 0x100144bb (float in st(0)) ----------------------------------------------------------------------
def tramp_code(target, nargs):
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\xd9\x1d' + struct.pack('<I', OUTF)      # fstp dword [OUTF]
    code += b'\x31\xc0\x5d\xc3'
    return code


def g_144bb(ctx):
    rg = begin(ctx)
    ctx.m.uc.mem_write(TRAMP, tramp_code(0x100144bb, 3))
    return [rw(ctx), rfl(ctx), rg.choice([0, 1, 2, 5, 100, 0xffffffff, 0x40000000, 0x7fffffff, 0x80000000, rw(ctx)])]


# ---- 0x1001f415 / 0x1001f4ba ---------------------------------------------------------------------------
def g_1f415(ctx):
    rg = begin(ctx)
    w32(ctx, 0x1015d0a4, rg.choice([0, 0, 0, 0, newbuf(ctx, 0x400)]))
    w32(ctx, 0x1015dc78, rw(ctx))
    return [rw(ctx)]


def chain(ctx, n, size=0x100):
    """Nodes of a chain whose next pointer is at +0x67 (unaligned); returns the first node."""
    rg = ctx.rng
    nodes = [newbuf(ctx, size) for _ in range(n)]
    for i, nd in enumerate(nodes):
        w32(ctx, nd + 0x67, nodes[i + 1] if i + 1 < n else 0)
        w32(ctx, nd + 0x24, rg.choice([0, 0, newbuf(ctx, 0x40)]))
        w32(ctx, nd + 0x28, rg.choice([0, 0, 0, 1]))
    return nodes[0] if nodes else 0


def g_1f4ba(ctx):
    rg = begin(ctx)
    block = newbuf(ctx, 0x400)
    w32(ctx, 0x1015d0a4, rg.choice([0, block]))
    root = newbuf(ctx, 0x100)
    w32(ctx, root + 0x67, chain(ctx, rg.choice([0, 1, 2, 3])))
    w32(ctx, 0x1015d528, rg.choice([root, root, 0]))
    w32(ctx, 0x1015d1bc, rg.choice([0, 0, 1]))
    w32(ctx, 0x101321a8, rg.choice([0xffffffff, 0, 1, 3]))
    w32(ctx, 0x101321c0, rw(ctx))
    return []


def g_2001f(ctx):
    rg = begin(ctx)
    root = newbuf(ctx, 0x100)
    w32(ctx, root + 0x67, chain(ctx, rg.choice([0, 1, 2, 3, 5])))
    w32(ctx, 0x1015d528, rg.choice([root, root, root, 0]))
    return []


# ---- 0x10020441 -------------------------------------------------------------------------------------
def g_20441(ctx):
    rg = begin(ctx)
    w32(ctx, 0x109b98c0, ctx.buf(16, 'text'))
    w32(ctx, 0x1015d530, rg.choice([0, 0, 0, 1]))
    def text_obj():
        o = newbuf(ctx, 0x100)
        w32(ctx, o + 0x10, rg.choice([0, ctx.buf(24, 'text'), ctx.buf(24, 'text'), ctx.buf(24, bytes(24))]))
        w32(ctx, o + 0x14, rg.choice([0, 0, 1]))
        wb(ctx, o, rg.choice([b'', b'abc\0', b'x\0']) + b'\0')
        return o
    return [rg.choice([0, text_obj(), text_obj(), text_obj()])]


# ---- 0x1002464b, 0x10024684 ----------------------------------------------------------------------------
def world_24(ctx):
    recs, cur = slot_world(ctx)
    rg = ctx.rng
    w32(ctx, 0x1015ef3c, rg.choice([0, 1]))
    return recs


def g_2464b(ctx):
    rg = ctx.rng
    world_24(ctx)
    return [rg.choice([0, 1, 5, 0x10, 0x4d, 0x4000, 0x4001, 0x11, 0xffffffff]),
            rw(ctx), rw(ctx), rw(ctx), rfl(ctx), rfl(ctx), rfl(ctx)]


def g_24684(ctx):
    rg = ctx.rng
    world_24(ctx)
    return [rg.choice([0, 1, 5, 0x10, 0x4d, 0x4000, 0x4001, 0x11, 0xffffffff, 0x2a, 0x3c]), rw(ctx)]


# ---- 0x10025dac -------------------------------------------------------------------------------------
def g_25dac(ctx):
    rg = begin(ctx)
    wb(ctx, 0x101653e0, rnd(ctx, 480))
    STATE['create_fails'] = rg.random() < 0.3
    return []


# ---- 0x100291a2 -------------------------------------------------------------------------------------
def g_291a2(ctx):
    rg = begin(ctx)
    view_world(ctx)
    i = rg.choice([0, 1, 2, 3, 4, 5, 9, 12])
    base = 0x109f09e0 + i * 0x14
    w32(ctx, base + 4, rg.choice([0, 0, tree(ctx), tree(ctx)]))
    w32(ctx, base + 0x10, rg.choice([0, 0, code_stub(ctx, 2, 0)]))
    cur = rg.choice([rg.uniform(-50, 50), rg.uniform(0, 2), 0.0, 1.5, -0.0, 3.0])
    d = rg.choice([cur, cur + 1e-6, cur - 1e-6, rg.uniform(-50, 50), 0.0, rg.uniform(0, 3), 1e-30])
    wf(ctx, base + 0xc, cur)
    return [i, fbits(d) if rg.random() < 0.9 else rfl(ctx)]


# ---- 0x1002d454 -------------------------------------------------------------------------------------
def g_2d454(ctx):
    rg = begin(ctx)
    obj = newbuf(ctx, 0x200)
    grid = newbuf(ctx, 0x800, bytes(0x800))
    wf(ctx, grid, *[rg.uniform(-50, 0) for _ in range(1)], *[rg.uniform(0, 50) for _ in range(1)],
       *[rg.uniform(-50, 0), rg.uniform(0, 50), rg.uniform(-50, 0), rg.uniform(0, 50)])
    w32(ctx, obj + 0x2c, ptr_or_null(ctx, newbuf(ctx, 0x100), 0.1))
    w32(ctx, obj + 0x98, ptr_or_null(ctx, grid, 0.1))
    ray = newbuf(ctx, 0x40, bytes(0x40))
    wf(ctx, ray, *[rg.uniform(-60, 60) for _ in range(6)])
    return [obj, ptr_or_null(ctx, ray, 0.1)]


# ---- 0x100345e0 -------------------------------------------------------------------------------------
def g_scroll(ctx):
    begin(ctx)
    mono(ctx)
    return []


# ---- 0x10034891 -------------------------------------------------------------------------------------
def g_34891(ctx):
    rg = begin(ctx)
    wb(ctx, 0x10139f58, rnd(ctx, 0x60))
    fmt = rg.choice([b'hello', b'%d', b'%s %d', b'x=%d y=%x %c', b'%5.2s|%-4d|', b'%08lX', b'%%%d', b'a%sb', b'%u %u %u',
                     b'%s%s%s%s', b'%s%s%s%s', b'%-60d|%s', b'%s%s%s'])
    txt = bytes(rg.choice(b'abcdefghij') for _ in range(rg.choice([8, 8, 22, 26, 39]))) + b'\0'
    text = ctx.buf(len(txt), txt)
    args = [text if b'%s' in fmt else rw(ctx) for _ in range(4)]
    return [ctx.buf(len(fmt) + 1, fmt + b'\0')] + args


# ---- 0x1003c582 -------------------------------------------------------------------------------------
def g_lib(ctx):
    rg = begin(ctx)
    wb(ctx, 0x1015ea00, rnd(ctx, 0x180))
    return [ctx.buf(24, 'text')]


def h_load(n, args):
    return 0 if n % 5 == 4 else 0x60000000 + 0x1000 * n


def h_getproc(n, args):
    return 0x61000000 + 0x10 * n if n % 11 != 5 else 0


# ---- 0x1003cf35 -------------------------------------------------------------------------------------
def g_3cf35(ctx):
    rg = begin(ctx)
    name = bytes(rg.choice(b'abcXYZ019_.') for _ in range(rg.randrange(0, 12))) + b'\0'
    return [ctx.buf(len(name), name)]


# ---- 0x1003e26a -------------------------------------------------------------------------------------
def g_3e26a(ctx):
    rg = begin(ctx)
    w32(ctx, 0x1015ec84, rg.choice([0, 1, 1, 1, 5]))
    st = newbuf(ctx, 0x200)
    wb(ctx, st + 0x1e, bytes([rg.choice([0, 1, 1, 1, 7])]))
    w32(ctx, st + 0x16, rg.choice([0, 1, 1, 0x1234]))
    w32(ctx, st + 0xc, rw(ctx))
    w16(ctx, st + 0x10, rw(ctx))
    w32(ctx, st + 0x12, rg.choice([0, 1, 2]))
    w32(ctx, st + 0x126, newbuf(ctx, 0x100))
    wb(ctx, st + 0x136, bytes([rg.choice([0, 1, 2, 3])]))
    w32(ctx, 0x1015d004, st)
    w32(ctx, 0x1017b8c4, newbuf(ctx, 0x40))
    for k, slot in enumerate((0x1015ea94, 0x1015ead8)):          # the library functions the callee calls
        w32(ctx, slot, code_stub(ctx, k, rg.choice([0, 5, 0x1000, 0xffffffff])))
    w32(ctx, 0x1015ecd4, rw(ctx))
    return []


# ---- 0x10041a16 / 0x10044069 / 0x1004edb6 -----------------------------------------------------------------
def g_41a16(ctx):
    rg = begin(ctx)
    w32(ctx, 0x1015f1fc, rg.choice([0, 0, 1, rw(ctx)]))
    w32(ctx, 0x1015f1e4, rw(ctx))
    w32(ctx, 0x1015f1d0, rw(ctx))
    return []


def g_44069(ctx):
    rg = begin(ctx)
    w32(ctx, 0x1013aec4, rg.choice([0, 1, 5, 0xffffffff, 0x80000000, rw(ctx)]))
    w32(ctx, 0x1015ed20, rw(ctx))
    w32(ctx, 0x1015ed38, rw(ctx))
    w32(ctx, 0x1015ee98, rw(ctx))
    return []


# ---- 0x10048720 -------------------------------------------------------------------------------------
def g_48720(ctx):
    rg = begin(ctx)
    wb(ctx, 0x109b9470, rnd(ctx, 0x60))
    hi = rg.choice([0x700, 0x700, 0x700, 0x600, 0x800, 0x7f00 & 0xff00, 0xf700, rg.getrandbits(8) << 8])
    v = hi | rg.getrandbits(8)
    return [v | (rg.getrandbits(16) << 16 if rg.random() < 0.5 else 0)]


# ---- 0x10053620 -------------------------------------------------------------------------------------
def g_53620(ctx):
    rg = begin(ctx)
    names = []
    for i in range(0x6d):
        a = struct.unpack('<I', bytes(ctx.m.uc.mem_read(0x1013f160 + i * 32, 4)))[0]
        names.append(bytes(ctx.m.uc.mem_read(a, 24)).split(b'\0')[0] if 0x10000000 <= a < 0x10200000 else b'')
    c = rg.random()
    cand = [n for n in names if n]
    if c < 0.7 and cand:
        s = rg.choice(cand)
        if rg.random() < 0.4:
            s = s.swapcase()
        if rg.random() < 0.1:
            s = s[:-1]
    else:
        s = bytes(rg.choice(b'abcxyz') for _ in range(rg.randrange(0, 8)))
    return [ctx.buf(len(s) + 1, s + b'\0')]


# ---- 0x10057afc -------------------------------------------------------------------------------------
def h_peek(n, args, uc):
    esp = uc.reg_read(UC_X86_REG_ESP)
    msg = r32(uc, esp + 4)
    if n % 3 == 1:
        return 0
    uc.mem_write(msg, struct.pack('<7I', 0x1234 + n, 0x100 + n % 9, 0x41 + n, 0xdead0000 + n, n, n, n))
    return 1


# ---- 0x1005aee5 -------------------------------------------------------------------------------------
def g_5aee5(ctx):
    rg = begin(ctx)
    words = [bytes(rg.choice(b'abcXYZ') for _ in range(rg.randrange(1, 4))) for _ in range(5)]
    nodes = []

    def node(depth):
        n = newbuf(ctx, 0xb0, bytes(0xb0))
        t = rg.choice(words)
        w32(ctx, n + 0x90, ctx.buf(8, t + b'\0' * (8 - len(t))))
        if depth < 3:
            w32(ctx, n + 4, rg.choice([0, node(depth + 1), node(depth + 1)]))
            w32(ctx, n + 8, rg.choice([0, node(depth + 1)]))
        return n
    root = node(0)
    s = rg.choice(words)
    if rg.random() < 0.3:
        s = s.swapcase()
    s = s + b'\0' + bytes(8)
    return [ptr_or_null(ctx, root, 0.1), ctx.buf(len(s), s)]


# ---- 0x10060656 -------------------------------------------------------------------------------------
def g_60656(ctx):
    begin(ctx)
    fl = ctx.buf(0x40)
    return [ptr_or_null(ctx, fl, 0.05)] + [rfl(ctx) for _ in range(6)]


# ---- 0x10061b78 -------------------------------------------------------------------------------------
def g_61b78(ctx):
    rg = begin(ctx)
    return [rg.choice([0, 1, 2, 3, 7, 14, 15, 15, 16, 17, 100, 0xffffffff, 0x80000000, rg.randrange(0, 20)])]


# ---- 0x1008b8de -------------------------------------------------------------------------------------
def g_8b8de(ctx):
    rg = begin(ctx)
    w32(ctx, 0x1015ecd0, rg.choice([0, 0, 1, rw(ctx)]))
    w32(ctx, 0x1015d0a4, rg.choice([0, ctx.buf(0x400)]))
    return []


# ---- 0x1008e211 -------------------------------------------------------------------------------------
def g_8e211(ctx):
    rg = begin(ctx)
    a = rg.choice([rfl(ctx), fbits(rg.uniform(0, 100)), fbits(rg.choice([0.0, 1e-14, 1e-15, 1e-16, 9e-15, 1.1e-14, 1.0, 4.0])),
                   fbits(rg.uniform(0, 1e-13))])
    return [a, rfl(ctx), ctx.buf(16)]


# ---- 0x10098826 -------------------------------------------------------------------------------------
def g_98826(ctx):
    rg = begin(ctx)
    block = newbuf(ctx, 0x100)
    w32(ctx, 0x1015d1b8, block)
    w32(ctx, 0x1015d1b0, rw(ctx))
    return [rw(ctx), rg.choice([0, 1, 2, 3, 5, 10, rw(ctx)])]


# ---- 0x100999bc -------------------------------------------------------------------------------------
def g_999bc(ctx):
    rg = begin(ctx)
    w32(ctx, 0x1015d1c8, rw(ctx))
    w32(ctx, 0x1015d1ac, rg.choice([0, 1, 2, 3, 4, 4, 4, 5, 0xffffffff, 0x80000000]))
    w32(ctx, 0x1015d1cc, rg.choice([0, 1, 2, 3, 4, 0xffffffff, 0x80000000]))
    return []


# ---- 0x100a81fb -------------------------------------------------------------------------------------
def g_a81fb(ctx):
    rg = begin(ctx)
    for i in range(4):
        w32(ctx, 0x101c65e0 + i * 0x14a, rg.choice([0, 1, 2, 3, 0xffffffff, rw(ctx)]))
    return [ctx.buf(0x100), rg.choice([0, 1, 2, 3, 3]), rg.choice([0, 1, 100, 0xffffffff, rw(ctx)]), rw(ctx)]


# ---- 0x100a9b8d -------------------------------------------------------------------------------------
def g_a9b8d(ctx):
    rg = begin(ctx)
    s = bytes(rg.choice(b'abcXYZ019 _.') for _ in range(rg.randrange(0, 20)))
    return [newbuf(ctx, 0xb0), ctx.buf(len(s) + 1, s + b'\0')]


# ---- 0x100aa4a2, 0x100aa4f0 -------------------------------------------------------------------------
def tree(ctx, depth=0):
    """A node with a child chain (+4 first child, +8 next sibling) and a record pointer at +0x8c."""
    rg = ctx.rng
    node = newbuf(ctx, 0x100, bytes(0x100))
    w32(ctx, node, rg.choice([0, 0, 1]))
    w32(ctx, node + 0x94, rg.choice([0, 0, 1, 0x8000, 0x8001, rw(ctx)]))
    nkids = rg.choice([0, 0, 1, 2, 3]) if depth < 3 else 0
    kids = [tree(ctx, depth + 1) for _ in range(nkids)]
    for i, k in enumerate(kids):
        w32(ctx, k + 8, kids[i + 1] if i + 1 < len(kids) else 0)
    w32(ctx, node + 4, kids[0] if kids else 0)
    w32(ctx, node + 8, 0)
    w32(ctx, node + 0x8c, rg.choice([0, 0, newbuf(ctx, 0x80, bytes(0x80))]))
    return node


def view_world(ctx):
    """Globals read by Sim_1008a1ea."""
    rg = ctx.rng
    w32(ctx, 0x1017c8cc, rg.choice([0x1017b988, 0x1017b988, 0x1017b988, 0, rw(ctx)]))
    w32(ctx, 0x1017c8c4, rg.choice([0, 1, 1]))


def g_aa4a2(ctx):
    begin(ctx)
    view_world(ctx)
    return [tree(ctx)]


def g_aa4f0(ctx):
    rg = begin(ctx)
    return [tree(ctx), rg.choice([0, 1, 0x7fff, 0xffff, 0x10000, 0xffffffff, rw(ctx)])]


# ---- 0x100aa799 -------------------------------------------------------------------------------------
def g_aa799(ctx):
    rg = begin(ctx)
    view_world(ctx)
    obj = tree(ctx)
    w32(ctx, obj, rg.choice([0, 0, 1]))
    return [obj]


# ---- 0x100aa87a -------------------------------------------------------------------------------------
def g_aa87a(ctx):
    rg = begin(ctx)
    obj = newbuf(ctx, 0xb0)
    src = newbuf(ctx, 0x60)
    for k in range(3):
        wf(ctx, src + 0x24 + 4 * k, rg.uniform(-50, 50))
    return [obj, src]


# ---- 0x100ab151 -------------------------------------------------------------------------------------
def g_ab151(ctx):
    rg = begin(ctx)
    view_world(ctx)
    a = tree(ctx)
    w32(ctx, 0x101c64f4, rw(ctx))
    w32(ctx, 0x101c6504, rw(ctx))
    out = ctx.buf(16)
    return [ptr_or_null(ctx, a, 0.15), rg.choice([0, 1, 2, 3, 5, 0xffffffff, 0x7fffffff]), out,
            out + 8 if rg.random() < 0.9 else out]


# ---- 0x100b7834 -------------------------------------------------------------------------------------
def g_b7834(ctx):
    rg = begin(ctx)
    n = rg.choice([0, 1, 2, 5, 9])
    names = sorted({bytes(rg.choice(b'abcdXYZ_01') for _ in range(rg.randrange(1, 6))) for _ in range(n)})
    arr = ctx.buf(8 * max(len(names), 1))
    for i, nm in enumerate(names):
        w32(ctx, arr + 8 * i, ctx.buf(8, nm + b'\0' + bytes(7 - len(nm))), rg.getrandbits(32))
    tbl = ctx.buf(8)
    w32(ctx, tbl, len(names), arr)
    if names and rg.random() < 0.6:
        key = rg.choice(names)
    else:
        key = bytes(rg.choice(b'abcdXYZ_01') for _ in range(rg.randrange(1, 6)))
    kp = ctx.buf(8, key + b'\0' + bytes(7 - len(key)))
    return [ptr_or_null(ctx, tbl, 0.12), kp]


# ---- 0x100c03d8 / 0x100c0d9d --------------------------------------------------------------------------
def g_c03d8(ctx):
    rg = ctx.rng
    obj = obj_world(ctx)
    w16(ctx, obj + 0x1aa, rg.choice([0x100, 0x200, 0x400, 0x105, 0x201, 0x1000, 0, rg.getrandbits(16)]))
    w32(ctx, obj + 0x54, newbuf(ctx, 0x40))
    w32(ctx, obj + 0x1f4, rg.choice([0, 0, 1]))
    w32(ctx, obj + 0x68, rfl(ctx))
    w32(ctx, obj + 0xe8, rfl(ctx))
    w32(ctx, obj + 0x110, rfl(ctx))
    return [obj]


def g_c0d9d(ctx):
    rg = ctx.rng
    obj = obj_world(ctx)
    w32(ctx, obj + 0x1f4, rg.choice([0, 0, 1]))
    w32(ctx, obj + 0x68, rfl(ctx))
    w32(ctx, obj + 0xe8, rfl(ctx))
    w32(ctx, obj + 0x110, rfl(ctx))
    cmd = rg.choice([0x100, 0x200, 0x400, 0x300, 0]) | rg.randrange(0, 12)
    return [obj, cmd | (rg.getrandbits(16) << 16 if rg.random() < 0.5 else 0)]


# ---- 0x100c6f39, 0x100c6f82 -------------------------------------------------------------------------
def file_obj(ctx):
    rg = ctx.rng
    f = ctx.buf(0x40)                         # a FILE for the C runtime seek
    w32(ctx, f + 0xc, rg.choice([0x80, 0x01, 0x02, 0x00, 0x82, rw(ctx)]))
    obj = ctx.buf(0x100)
    w32(ctx, obj, f)
    for i in range(6):
        w32(ctx, obj + 4 + 12 * i, rw(ctx), rg.choice([0, 4, 100, 0xffffffff]), rw(ctx))
    return obj


def g_c6f39(ctx):
    rg = begin(ctx)
    return [file_obj(ctx), rg.choice([-1 & 0xffffffff, 0, 1, 2, 3, 5, 5])]


def g_c6f82(ctx):
    rg = begin(ctx)
    obj = file_obj(ctx)
    s = bytes(rg.choice(b'abc') for _ in range(rg.randrange(0, 5)))
    return [obj, ctx.buf(len(s) + 1, s + b'\0')]


# ---- 0x100cbe13, 0x100cbe3c ---------------------------------------------------------------------------
def g_cbe13(ctx):
    rg = ctx.rng
    obj_world(ctx)
    return [rg.choice([0, 1, 2, 3, 5, 0xffffffff, rw(ctx)])]


# ---- 0x100db205 -------------------------------------------------------------------------------------
def g_db205(ctx):
    rg = begin(ctx)
    base = newbuf(ctx, 8 + 16 * 0x11 + 16)
    for i in range(16):
        w32(ctx, 0x109b7d50 + i * 0x6c, rw(ctx))
    return [base]


def S(name, addr, gen, trials=400, **kw):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials, api=API)
    d.update(kw)
    return d


def SPY(name, addr, gen, trials, *stubs, **kw):
    """The same function with its callees replaced by logging stubs: proves the arguments and their order."""
    return S(name + '_args', addr, with_stubs(gen, *stubs), trials, c=name, **kw)


SPECS = [
    S('Cache_ShowItemCount', 0x10008197, g_showcount, 400, void=True),
    S('Cache_ClearItemCount', 0x100081dd, g_clearcount, 200, void=True),
    S('Sim_100094ba', 0x100094ba, g_94ba, 400, void=True),
    S('Sim_1000e93e', 0x1000e93e, g_e93e, 500),
    SPY('Sim_1000e93e', 0x1000e93e, g_e93e, 300, 0x1000e737),
    S('Sim_10010a73', 0x10010a73, g_mode, 400, void=True),
    SPY('Sim_10010a73', 0x10010a73, g_mode, 300, 0x10010564, void=True),
    S('Sim_10010b38', 0x10010b38, g_mode, 400, void=True),
    SPY('Sim_10010b38', 0x10010b38, g_mode, 300, 0x10010632, void=True),
    S('Sim_1001403a', 0x1001403a, g_1403a, 600),
    S('Sim_100144bb', TRAMP, g_144bb, 500, c='Test_100144bb'),
    # superseded: 1.0 drove the CD here; no CD audio (DECISIONS.md "dropped features")
    # S('Sim_1001f415', 0x1001f415, with_stubs(g_1f415, 0x10039aca, 0x1009e1ef), 300),
    S('Sim_1001f4ba', 0x1001f4ba, g_1f4ba, 300, void=True),
    S('Sim_1002001f', 0x1002001f, g_2001f, 400, void=True),
    S('Sim_10020441', 0x10020441, g_20441, 400),
    SPY('Sim_10020441', 0x10020441, g_20441, 200, 0x1001f790),
    S('Sim_1002464b', 0x1002464b, g_2464b, 400, void=True),
    SPY('Sim_1002464b', 0x1002464b, g_2464b, 300, 0x100246b3, void=True),
    S('Sim_10024684', 0x10024684, g_24684, 400, void=True),
    SPY('Sim_10024684', 0x10024684, g_24684, 300, 0x100246b3, void=True),
    S('Sim_10025dac', 0x10025dac, with_stubs(g_25dac, 0x100fef30), 300,
      api=dict(API, CreateFileA=lambda n, a: 0xffffffff if STATE.get('create_fails') else 1)),
    S('Sim_100291a2', 0x100291a2, g_291a2, 500, void=True),
    S('Sim_1002d454', 0x1002d454, g_2d454, 300),
    SPY('Sim_1002d454', 0x1002d454, g_2d454, 200, 0x10030c87),
    S('Mono_ScrollUp', 0x100345e0, g_scroll, 200, void=True),
    S('Sim_10034891', 0x10034891, g_34891, 400),
    S('Lib_Load_1003c582', 0x1003c582, g_lib, 200, api=dict(API, LoadLibraryA=h_load, GetProcAddress=h_getproc)),
    S('Sim_1003cf35', 0x1003cf35, g_3cf35, 300),
    SPY('Sim_1003cf35', 0x1003cf35, g_3cf35, 200, 0x100461e8),
    S('Sim_1003e26a', 0x1003e26a, g_3e26a, 300),
    S('Sim_10041a16', 0x10041a16, g_41a16, 300, void=True),
    S('Sim_10044069', 0x10044069, g_44069, 300, void=True),
    S('Sim_10048720', 0x10048720, g_48720, 400),
    S('Sim_10053620', 0x10053620, g_53620, 500),
    # Sim_10057afc (0x10057afc): round 28 moved it to src/sim/platform/w28_input.c, proven in spec_w28_input.py
    S('Sim_1005aee5', 0x1005aee5, g_5aee5, 300),
    SPY('Sim_1005aee5', 0x1005aee5, g_5aee5, 200, 0x1010d180, 0x1005af06),
    S('Sim_10060656', 0x10060656, g_60656, 300, void=True),
    SPY('Sim_10060656', 0x10060656, g_60656, 300, 0x10060220, void=True),
    S('Sim_10061b78', 0x10061b78, with_stubs(g_61b78, 0x100d3510), 300),
    S('Sim_1008b8de', 0x1008b8de, g_8b8de, 300, void=True),
    SPY('Sim_1008b8de', 0x1008b8de, g_8b8de, 200, 0x100e46d0, void=True),
    S('Sim_1008e211', 0x1008e211, g_8e211, 500),
    S('Sim_10098826', 0x10098826, g_98826, 300, void=True),
    SPY('Sim_10098826', 0x10098826, g_98826, 200, 0x10090d15, void=True),
    S('Sim_100999bc', 0x100999bc, with_stubs(g_999bc, 0x10099c36, 0x100999fc), 300, void=True),
    S('Sim_100a81fb', 0x100a81fb, g_a81fb, 400),
    SPY('Sim_100a81fb', 0x100a81fb, g_a81fb, 300, 0x10105fe0, 0x100a796c),
    S('Sim_100a9b8d', 0x100a9b8d, with_stubs(g_a9b8d, 0x100fef30), 300, void=True),
    S('Sim_100aa4a2', 0x100aa4a2, g_aa4a2, 300, void=True),
    S('Sim_100aa4f0', 0x100aa4f0, g_aa4f0, 300, void=True),
    S('Sim_100aa799', 0x100aa799, with_stubs(g_aa799, 0x100aa742, 0x10088a2d), 300, void=True),
    S('Sim_100aa87a', 0x100aa87a, g_aa87a, 300, void=True),
    SPY('Sim_100aa87a', 0x100aa87a, g_aa87a, 200, 0x10060d2e, void=True),
    S('Sim_100ab151', 0x100ab151, g_ab151, 300, void=True),
    SPY('Sim_100ab151', 0x100ab151, g_ab151, 200, 0x100ab0d6, void=True),
    S('Sim_100b7834', 0x100b7834, g_b7834, 500),
    S('Sim_100c03d8', 0x100c03d8, with_stubs(g_c03d8, 0x1000ae00), 500, void=True),
    S('Sim_100c0d9d', 0x100c0d9d, with_stubs(g_c0d9d, 0x1000ae00), 500),
    S('Sim_100c6f39', 0x100c6f39, g_c6f39, 400),
    SPY('Sim_100c6f39', 0x100c6f39, g_c6f39, 200, 0x100feb20),
    S('Sim_100c6f82', 0x100c6f82, g_c6f82, 400),
    SPY('Sim_100c6f82', 0x100c6f82, g_c6f82, 200, 0x100c6eae),
    S('Sim_100cbe13', 0x100cbe13, with_stubs(g_cbe13, 0x100ca670), 400, void=True),
    S('Sim_100cbe3c', 0x100cbe3c, with_stubs(g_cbe13, 0x100ca670), 400, void=True),
    S('Sim_100db205', 0x100db205, g_db205, 300, void=True),
    SPY('Sim_100db205', 0x100db205, g_db205, 200, 0x1005259e, void=True),
]
