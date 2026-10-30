# Differential tests for round 5, letter x (see difftest.py and docs/checker.md).
#
# These functions call helpers of the DLL that are not converted yet. The helpers are replaced in the
# emulator by a stub: a `ret` written over the helper's first byte plus a code hook that appends
# ('stub_<address>', arguments) to the checker's call log (which is compared between the original and the
# C run, like the Windows calls) and answers with a value chosen by the generator. The stub address is the
# helper's own address, so the original and the C code reach the same stub. Stack addresses are logged as
# 'stack'. Helpers that are only reached through converted code are left alone. The C library square root
# (0x100fca04) is not stubbed: it runs for real in the emulator (Sim_1004c362, Sim_10084b57), with the Windows
# control word 0x27f (53-bit precision) like the real program.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG

SOURCES = [
    'sim/unsorted_1000/w5x_calls.c', 'sim/unsorted_1001/w5x_calls.c', 'sim/unsorted_1002/w5x_calls.c',
    'sim/unsorted_1003/w5x_calls.c', 'sim/unsorted_1004/w5x_math.c', 'sim/unsorted_1005/w5x_calls.c',
    'sim/unsorted_1007/w5x_calls.c', 'sim/unsorted_1008/w5x_calls.c', 'sim/unsorted_1008/w5x_math.c',
    'sim/unsorted_1009/w5x_calls.c', 'sim/unsorted_100a/w5x_calls.c', 'sim/unsorted_100b/w5x_calls.c',
    'sim/unsorted_100c/w5x_calls.c', 'sim/unsorted_100d/w5x_calls.c',
    '../tools/verify/w5x_test.c',
    # existing converted code the functions above call
    'sim/unsorted_1000/w2e_names.c', 'sim/unsorted_1000/w2j_slots.c',
    'sim/unsorted_1000/w2a_fields.c',
    'sim/unsorted_1001/w2j_misc.c', 'sim/unsorted_1001/w4u_cd.c',
    'sim/unsorted_1002/w2j_nodes.c',
    'sim/unsorted_1003/flags.c',
    'sim/unsorted_1004/fields.c', 'sim/unsorted_1004/w2l_values.c',
    'sim/unsorted_1008/cell_link.c',
    'sim/unsorted_100a/w2k_list.c',
    'sim/unsorted_100d/nodes.c',
    'sim/unsorted_100f/w2e_string.c',
    'sim/unsorted_1001/w2e_names.c', 'sim/util/lookup.c', 'sim/util/ring.c', 'sim/unsorted_100d/w2j_nodes.c',
    'sim/util/field.c', 'sim/view/objlist.c', 'sim/view/w2l_lists.c', 'sim/win/w4u_cdaudio.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000
DRIVER = 0x00F00200      # the target of the indirect call of Sim_1005502a (a `ret` with a hook)
TRAMP = 0x00F00800

# helper address -> number of dword arguments
STUBS = {
    0x100bb2b0: 2, 0x100b9203: 2, 0x1000e737: 12, 0x1000eb12: 7, 0x10010700: 2, 0x10016117: 2,
    0x100105cb: 2, 0x1001274b: 1, 0x10017b3e: 2, 0x100be0f0: 2, 0x10039c00: 0, 0x1003a052: 0,
    0x1004724c: 2, 0x1001f790: 3, 0x10027fb8: 1, 0x10031ce0: 3, 0x10031f95: 6, 0x1003462f: 1,
    0x100fd850: 2, 0x100548aa: 0, 0x10054a79: 0, 0x1010d180: 1, 0x1005ae79: 2, 0x1002c1f9: 1,
    0x1002c63a: 0, 0x1008d4f1: 2, 0x100652e8: 4, 0x10090d15: 1, 0x100a96d8: 1, 0x100a5d38: 4,
    0x100a9ba7: 1, 0x10060681: 4, 0x10060d2e: 2, 0x100aaf32: 1, 0x100feb20: 3, 0x100fe970: 1,
    0x100aab03: 2, 0x100fd760: 1, 0x100fefe0: 1, 0x1002c8cb: 4, 0x100ca670: 3, 0x100fef30: 1,
    0x100fdc80: 2, 0x100ba7d5: 3, 0x100d3510: 3,
}
# Sim_10013dbd: the original pushes the low word of its argument with the top half of a pointer; the callee
# 0x100be0f0 reads only the low word (every one of its 19 reads of that argument is 16 bits wide).
MASK = {(0x100be0f0, 1): 0xffff}
ST = {}


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def put(ctx, addr, fmt, *vals):
    out = b''
    for f, v in zip(fmt, vals):
        size = struct.calcsize('<' + f)
        out += (v & ((1 << (8 * size)) - 1)).to_bytes(size, 'little')
    ctx.m.uc.mem_write(addr, out)


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def s32(v):
    v &= 0xffffffff
    return v - (1 << 32) if v >= 1 << 31 else v


def rstr(ctx, maxlen, alpha=b'abcXYZ019 _.'):
    n = ctx.rng.choice([0, 1, 2, 3, 4, 5, 7, 8, 9, ctx.rng.randrange(0, maxlen + 1), ctx.rng.randrange(0, maxlen + 1)])
    return bytes(ctx.rng.choice(alpha) for _ in range(min(n, maxlen)))


def cstr(ctx, data, extra=24):
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rb(ctx, extra))


# ---- stubs --------------------------------------------------------------------------------------
def _make_cb(m, addr, nargs):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        args = []
        for k in range(nargs):
            v = struct.unpack('<I', bytes(uc.mem_read(esp + 4 + 4 * k, 4)))[0]
            if STACK_LO <= v <= STACK_HI:
                v = 'stack'
            elif (addr, k) in MASK:
                v &= MASK[(addr, k)]
            args.append(v)
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % addr, tuple(args)))
        rets = ST['rets'].get(addr) or [0]
        uc.reg_write(UC_X86_REG_EAX, rets[n % len(rets)] & 0xffffffff)
    return cb


def _driver_cb(m):
    def cb(uc, address, size, ud):
        n = len(m.api_log)
        m.api_log.append(('driver', ()))
        rets = ST['driver_rets']
        uc.reg_write(UC_X86_REG_EAX, rets[n % len(rets)] & 0xffffffff)
    return cb


def tramp_double(target, nargs):
    """Calls target(args as dwords) and stores st(0) as a double at OUTF."""
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\xdd\x1d' + struct.pack('<I', OUTF)       # fstp qword [OUTF]
    code += b'\x31\xc0\x5d\xc3'
    return code


def begin(ctx):
    """Installs the stubs (once per machine) and chooses the answers of this trial."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w5x', False):
        m._w5x = True
        for addr, nargs in STUBS.items():
            m.raw.mem_write(addr, b'\xc3')
            m.raw.hook_add(UC_HOOK_CODE, _make_cb(m, addr, nargs), None, addr, addr)
        m.raw.mem_write(DRIVER, b'\xc3')
        m.raw.hook_add(UC_HOOK_CODE, _driver_cb(m), None, DRIVER, DRIVER)
        m.raw.mem_write(TRAMP, tramp_double(0x10084b57, 2))
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}
    ST['driver_rets'] = [r.choice([0, 1, 2, r.getrandbits(32)]) for _ in range(2)]


def rets(addr, *values):
    ST['rets'][addr] = list(values)


# ---- shared worlds ------------------------------------------------------------------------------
def slot_world(ctx):
    """Pointer table 0x109e8b40 with records, the current group and its slot count (as in spec_w2j.py)."""
    rg = ctx.rng
    count = rg.randrange(1, 9)
    group = rg.randrange(0, 3)
    cur = rg.randrange(0, count)
    pc = rg.choice([0.2, 0.7, 1.0, 1.0])
    recs = []
    for i in range(count):
        rec = newbuf(ctx, 0x200)
        g = group if (i == cur or rg.random() < 0.8) else rg.randrange(0, 3)
        put(ctx, rec + 8, 'I', g)
        put(ctx, rec + 0x10, 'I', i + 1 if rg.random() < 0.8 else rg.randrange(0, 12))
        put(ctx, rec + 0x1a6, 'h', 0xc if rg.random() < pc else rg.choice([0, 1, 5, -1]))
        recs.append(rec)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
    put(ctx, recs[cur] + 8, 'I', group)
    w32(ctx, 0x1015dc64, count if rg.random() < 0.9 else rg.randrange(0, count + 1))
    w32(ctx, 0x1017b8c8, cur)
    for g in range(3):
        w32(ctx, 0x109b7d44 + g * 0x6c, rg.choice([0, 1, 2, 3, 4, 5, 6, 7, 3, 4]))
    return recs


def name_table(ctx):
    """The table of names at 0x10a39200 (as in spec_w2e.py); returns the names."""
    rg = ctx.rng
    n = rg.choice([0, 0, 1, 2, 3, 5, 8])
    w32(ctx, 0x10a38de0, n)
    names = []
    for i in range(0, 12):
        nm = rstr(ctx, 6, b'abc')
        used = rg.choice([0, 1, 1, 2, rg.getrandbits(32)])
        ctx.m.uc.mem_write(0x10a39200 + i * 0x14, struct.pack('<I', used) + nm + b'\0' + rb(ctx, 15 - len(nm)))
        names.append(nm)
    return names


def bucket(key, name4):
    total = key + sum(b - 256 if b >= 128 else b for b in name4)
    total = s32(total)
    r = abs(total) % 0x3f1
    return -r if total < 0 else r


def hash_world(ctx, n=None):
    """Hash table of nodes at [0x1015dc30] (as in spec_w2j.py); returns [(node, key, name bytes)]."""
    rg = ctx.rng
    n = rg.choice([0, 1, 3, 6, 10, 14]) if n is None else n
    base = zbuf(ctx, 0x800 + 0x3f1 * 4 + 0x400)
    tab = base + 0x800
    nodes = []
    for _ in range(n):
        key = rg.choice([rg.randrange(0, 16), rg.randrange(0, 0x10000), rg.randrange(0, 0x100)])
        name = rg.choice([rb(ctx, 4), rb(ctx, 4), bytes([1, 2, 3, 4]), bytes([0x80, 0xff, 0x7f, 1]), b'\xff\xff\xff\xff'])
        node = zbuf(ctx, 0x20)
        nodes.append((node, key, name))
    for node, key, name in nodes:
        b = bucket(key, name)
        head = r32(ctx, tab + 4 * b)
        put(ctx, node, 'HhII', key, rg.choice([0, 0, 1, 0xffff, 2]), struct.unpack('<I', name)[0], head)
        w32(ctx, tab + 4 * b, node)
    w32(ctx, 0x1015dc30, tab)
    return nodes


# ---- generators ---------------------------------------------------------------------------------
def g_10006530(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1015cff0, r.choice([0, 0, 1, 0x1234, ctx.u32()]))
    w32(ctx, 0x1014cf0c, ctx.u32())
    return []


def named_args(ctx):
    """Name argument of the Sim_1000df9c users: usually one that is (or is not) in the table."""
    rg = ctx.rng
    names = name_table(ctx)
    nm = rg.choice(names[1:12]) if rg.random() < 0.5 else rstr(ctx, 6, b'abc')
    return cstr(ctx, nm)


def g_e983(ctx):
    begin(ctx)
    name = named_args(ctx)
    w32(ctx, 0x101321b4, ctx.u32())
    return [name, ctx.u32(), ctx.u32(), ctx.u32()]


def g_eaeb(ctx):
    begin(ctx)
    return [ctx.u32() for _ in range(6)]


def g_ec60(ctx):
    begin(ctx)
    r = ctx.rng
    name = named_args(ctx)
    w32(ctx, 0x101321b4, ctx.u32())
    for i in range(16):
        w32(ctx, 0x1012df30 + 4 * i, ctx.u32())
    return [name, ctx.u32(), ctx.u32(), r.choice([0, 1, 2, 5, 9, 15, -1, -3, 16, 17]), ctx.u32()]


def g_mode_world(ctx):
    begin(ctx)
    slot_world(ctx)
    w32(ctx, 0x1015d100, ctx.rng.choice([0, 0, 0, 1, 1, 2, 2, 3, -1, ctx.u32()]))
    return []


def g_10010aec(ctx):
    begin(ctx)
    slot_world(ctx)
    return []


def g_125a3(ctx):
    begin(ctx)
    r = ctx.rng
    p = newbuf(ctx, 32)
    w32(ctx, p, r.choice([8, 8, 5, 2, 0, 1, 7, 9, ctx.u32()]))
    return [p]


def g_13dbd(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    rec = newbuf(ctx, 0x40)
    w32(ctx, obj, r.choice([8, 5, 5, 8, 0, 1, 2, 3, ctx.u32()]))
    w32(ctx, obj + 0x54, rec)
    return [obj, r.choice([0, 1, -1, 0x7fff, 0x8000, 0x12345678, ctx.u32()])]


def g_1f454(ctx):
    begin(ctx)
    r = ctx.rng
    a = r.choice([0, 1, -1, 5, -5, 0x7fffffff, 0x80000000, r.randrange(-300, 300) & 0xffffffff, ctx.u32()])
    w32(ctx, 0x1015dc78, a)
    w32(ctx, 0x109f4228, r.choice([a, a + 1, a - 1, (a + 0x389), 0, ctx.u32()]))
    return []


def g_1f47d(ctx):
    begin(ctx)
    w32(ctx, 0x101321a8, ctx.rng.choice([-1, -1, 0]))      # no CD device open: Sim_1001f399 does nothing
    w32(ctx, 0x1015d1b8, ctx.u32())
    w32(ctx, 0x1017b8f8, ctx.u32())
    return []


def g_2009d(ctx):
    begin(ctx)
    return [ctx.rng.choice([-2, -1, 0, 1, 2, 3, 4, 0x7fffffff, 0x80000000, ctx.u32()]) & 0xffffffff]


def g_27a8f(ctx):
    begin(ctx)
    r = ctx.rng
    p = newbuf(ctx, 0x40)
    w32(ctx, p, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x1017b8f8, ctx.u32())
    w32(ctx, 0x1014cefc, ctx.u32())
    return [0 if r.random() < 0.1 else p]


def g_284df(ctx):
    begin(ctx)
    rg = ctx.rng
    nodes = hash_world(ctx)
    if nodes and rg.random() < 0.7:
        node, key, name = rg.choice(nodes)
        if rg.random() < 0.15:
            key = key + 0x10000 if rg.random() < 0.5 else key ^ 1
    else:
        key = rg.choice([rg.randrange(0, 16), rg.randrange(0, 0x10000), -1, -5, 0x7fffffff, 0x80000000 | rg.getrandbits(8)])
        name = rb(ctx, 4)
    return [key & 0xffffffff, ctx.buf(4, name)]


def g_2d3a5(ctx):
    begin(ctx)
    obj = newbuf(ctx, 0x100)
    return [obj, ctx.u32()]


def g_2d3ca(ctx):
    begin(ctx)
    obj = newbuf(ctx, 0x100)
    return [obj, ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32()]


def g_34777(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0, 1, 0x4f, 0x50, 0x51, 0x9f, 0xa0, 0xa1, 0xf0, 0x150, r.randrange(0, 400)])
    data = bytes(r.choice(b'abc xyz0123') for _ in range(n))
    return [ctx.buf(n + 1 + 8, data + b'\0' + rb(ctx, 8))]


def g_347b9(ctx):
    begin(ctx)
    w32(ctx, 0x1015e500, ctx.u32())
    return []


def g_4c362(ctx):
    begin(ctx)
    r = ctx.rng
    low = r.choice([0, 1, 2, 3, 4, 9, 10, 15, 16, 17, 99, 100, 255, 256, 0x3b6, 0xffff, 0x8000, r.getrandbits(16), r.getrandbits(16)])
    high = r.choice([0, 0, 1, 2, 3, 4, 5, 0xffff, 0xfffe, 0x7fff, r.randrange(0, 40), 0x8000 if r.random() < 0.05 else 6])
    return [(high << 16) | low]


def g_5502a(ctx):
    begin(ctx)
    r = ctx.rng
    idx = r.randrange(0, 4)
    for i in range(4):
        ent = zbuf(ctx, 0x40)
        w32(ctx, ent + 0x1c, DRIVER)
        w32(ctx, 0x1015fd20 + 4 * i, ent)
    w32(ctx, 0x1015fdf4, idx)
    return []


def g_5ae58(ctx):
    begin(ctx)
    return [newbuf(ctx, 0x100), cstr(ctx, rstr(ctx, 20))]


def g_72d16(ctx):
    begin(ctx)
    r = ctx.rng
    return [0 if r.random() < 0.3 else newbuf(ctx, 0x40)]


# a tree of nodes (first child at +4, next sibling at +8) whose +0x8c may point at an object for the list code
def obj_tree(ctx, depth):
    r = ctx.rng
    node = zbuf(ctx, 0x100)
    if r.random() < 0.6:
        obj = zbuf(ctx, 0x100)
        put(ctx, obj, 'H', r.choice([0, 0x800, 0x1000, 0x1800, 0x8000, 0xffff]))
        w32(ctx, obj + 0x34, r.choice([4, 0, 1]))
        w32(ctx, node + 0x8c, obj)
    kids = []
    if depth > 0:
        kids = [obj_tree(ctx, depth - 1) for _ in range(r.choice([0, 1, 2, 3]))]
    if kids:
        w32(ctx, node + 4, kids[0])
        for a, b in zip(kids, kids[1:]):
            w32(ctx, a + 8, b)
    return node


def g_aa351(ctx):
    begin(ctx)
    return [obj_tree(ctx, ctx.rng.choice([0, 1, 2, 3]))]


def records_world(ctx):
    """The 0x34-byte records at 0x101c57e0 (count [0x1017d1c8]) with trees at +0x30."""
    r = ctx.rng
    count = r.choice([0, 1, 2, 3, 5])
    w32(ctx, 0x1017d1c8, count)
    w32(ctx, 0x1017d1c4, r.choice([0, 0, 1, -1, count - 1, count - 2, 3]))
    for i in range(count):
        w32(ctx, 0x101c57e0 + i * 0x34 + 0x30, obj_tree(ctx, r.choice([0, 1, 2])) if r.random() < 0.95 else 0)
    return count


def g_ac58e(ctx):
    begin(ctx)
    count = records_world(ctx)
    return [ctx.rng.choice([-1, 0, 0, 1, 2, 3, 4, count - 1, count, count + 1, 0x7fffffff, 0x80000000]) & 0xffffffff]


def g_72d3a(ctx):
    begin(ctx)
    r = ctx.rng
    count = records_world(ctx)
    e = newbuf(ctx, 0x200)
    w32(ctx, 0x1017b8c8, r.choice([0, 1, 2, 7]))
    put(ctx, e + 4, 'I', r.choice([G_CUR(ctx), G_CUR(ctx), 5, r.getrandbits(32)]))
    put(ctx, e + 0x154, 'I', r.choice([-1, 0, 0, 1, 2, count - 1, count, count + 1, 9]))
    p = zbuf(ctx, 8)
    w32(ctx, p, e)
    return [0 if r.random() < 0.15 else p]


def G_CUR(ctx):
    return r32(ctx, 0x1017b8c8)


def g_788fc(ctx):
    begin(ctx)
    obj = newbuf(ctx, 0x120)
    return [obj, ctx.u32(), ctx.u32()]


def g_10084b57(ctx):
    begin(ctx)
    r = ctx.rng
    def fl():
        c = r.random()
        if c < 0.04: return 0x7fc00000
        if c < 0.07: return r.choice([0x7f800000, 0xff800000])
        if c < 0.12: return 0x80000000
        if c < 0.5:
            return struct.unpack('<I', struct.pack('<f', r.choice([0.0, 1.0, -1.0, 3.0, 4.0, 0.5, 100.0, 12.0, 5.0])))[0]
        if c < 0.55: return r.getrandbits(32)
        return struct.unpack('<I', struct.pack('<f', r.uniform(-500, 500)))[0]
    return [fl(), fl()]


def g_1008b757(ctx):
    begin(ctx)
    rg = ctx.rng
    names = name_table(ctx)
    if rg.random() < 0.5:         # the fixed name the functions pass is in the table
        nm = b'AFXHDWPS'
        slot = rg.randrange(1, 4)
        ctx.m.uc.mem_write(0x10a39200 + slot * 0x14, struct.pack('<I', rg.choice([0, 1, 2])) + nm + b'\0' + rb(ctx, 6))
    w32(ctx, 0x101321b4, ctx.u32())
    for i in range(16):
        w32(ctx, 0x1012df30 + 4 * i, ctx.u32())
    return []


def g_98811(ctx):
    begin(ctx)
    return [ctx.u32()]


def g_989a1(ctx):
    begin(ctx)
    w32(ctx, 0x1015d1b8, newbuf(ctx, 0x40))
    w32(ctx, 0x1014c9fc, ctx.u32())
    return [ctx.u32(), ctx.rng.choice([0, 1, -1, 5, ctx.u32()]) & 0xffffffff]


def g_a6009(ctx):
    begin(ctx)
    return [ctx.u32(), ctx.u32()]


def g_a9ca3(ctx):
    begin(ctx)
    return [newbuf(ctx, 0x100), ctx.u32(), ctx.u32(), ctx.u32()]


def g_aa812(ctx):
    begin(ctx)
    return [newbuf(ctx, 0x100), newbuf(ctx, 0x100)]


def g_aa863(ctx):
    begin(ctx)
    return [newbuf(ctx, 0x100)]


def bin_tree(ctx, depth):
    r = ctx.rng
    node = newbuf(ctx, 0x20)
    l = rt = 0
    if depth > 0:
        l = bin_tree(ctx, depth - 1) if r.random() < 0.7 else 0
        rt = bin_tree(ctx, depth - 1) if r.random() < 0.7 else 0
    w32(ctx, node + 8, l)
    w32(ctx, node + 4, rt)
    return node


def g_aaf9b(ctx):
    begin(ctx)
    r = ctx.rng
    return [0 if r.random() < 0.1 else bin_tree(ctx, r.choice([0, 1, 2, 3]))]


def g_ae64d(ctx):
    begin(ctx)
    r = ctx.rng
    p = newbuf(ctx, 0x20)
    rets(0x100feb20, *[r.choice([0, 0, 1, 5, 0xffffffff]) for _ in range(3)])
    return [p, ctx.u32(), ctx.u32()]


def g_b5b76(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    w32(ctx, obj + 0x8c, r.choice([0, 0, 1, ctx.u32()]))
    return [0 if r.random() < 0.15 else obj]


def g_c6e87(ctx):
    begin(ctx)
    p = newbuf(ctx, 0x20)
    w32(ctx, p, ctx.rng.choice([0, 0, 1, ctx.u32()]))
    return [p]


def g_c81b5(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    put(ctx, obj + 4, 'h', r.choice([0, 0, 1, -1, 0x100, 0x7fff]))
    w32(ctx, 0x109e89d8, ctx.u32())
    return [obj]


def g_cbe68(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1017b8c8, r.randrange(0, 6))
    for i in range(6):
        w32(ctx, 0x109e8b40 + 4 * i, ctx.u32())
    return [ctx.u32()]


def g_d1510(ctx):
    begin(ctx)
    rets(0x100fef30, newbuf(ctx, 0x20))
    return [ctx.u32()]


def free_tree(ctx, depth):
    r = ctx.rng
    node = newbuf(ctx, 0x20)
    s = newbuf(ctx, 16) if r.random() < 0.6 else 0
    w32(ctx, node, s)
    a = b = 0
    if depth > 0:
        a = free_tree(ctx, depth - 1) if r.random() < 0.6 else 0
        b = free_tree(ctx, depth - 1) if r.random() < 0.6 else 0
    w32(ctx, node + 8, a)
    w32(ctx, node + 0xc, b)
    return node


def g_d15fd(ctx):
    begin(ctx)
    r = ctx.rng
    return [0 if r.random() < 0.1 else free_tree(ctx, r.choice([0, 1, 2, 3]))]


def g_d15e0(ctx):
    begin(ctx)
    r = ctx.rng
    node = free_tree(ctx, r.choice([0, 1, 2]))
    if r.random() < 0.7:
        owner = zbuf(ctx, 0x20)
        w32(ctx, node + 0x14, owner)
        nxt = zbuf(ctx, 0x20) if r.random() < 0.6 else 0
        prv = zbuf(ctx, 0x20) if r.random() < 0.6 else 0
        w32(ctx, node + 4, nxt)
        w32(ctx, node + 8, prv)         # replaces the child at +8: the unlink clears it before the tree is freed
    else:
        w32(ctx, node + 0x14, 0)
    return [node]


def g_d1dd0(ctx):
    begin(ctx)
    return []


def g_d37ea(ctx):
    begin(ctx)
    r = ctx.rng
    name = rstr(ctx, 30, b'abcdefXYZ019')
    w32(ctx, 0x1014ced4, ctx.u32())
    return [cstr(ctx, name), r.choice([0, 1, 2, 5, 7, 3])]


def g_d3830(ctx):
    begin(ctx)
    return [ctx.u32(), ctx.u32()]


SPECS = [
    dict(name='Sim_10006530', orig=0x10006530, c='Sim_10006530', gen=g_10006530, trials=300, void=True),
    dict(name='Sim_1000e983', orig=0x1000e983, c='Sim_1000e983', gen=g_e983, trials=400, void=True),
    dict(name='Sim_1000eaeb', orig=0x1000eaeb, c='Sim_1000eaeb', gen=g_eaeb, trials=300, void=True),
    dict(name='Sim_1000ec60', orig=0x1000ec60, c='Sim_1000ec60', gen=g_ec60, trials=400, void=True),
    dict(name='Sim_1000ed52', orig=0x1000ed52, c='Sim_1000ed52', gen=g_eaeb, trials=300, void=True),
    dict(name='Sim_10010aa0', orig=0x10010aa0, c='Sim_10010aa0', gen=g_mode_world, trials=400, void=True),
    dict(name='Sim_10010aec', orig=0x10010aec, c='Sim_10010aec', gen=g_10010aec, trials=300, void=True),
    dict(name='Sim_10010b65', orig=0x10010b65, c='Sim_10010b65', gen=g_mode_world, trials=400, void=True),
    dict(name='Sim_100125a3', orig=0x100125a3, c='Sim_100125a3', gen=g_125a3, trials=300, void=True),
    dict(name='Sim_10013dbd', orig=0x10013dbd, c='Sim_10013dbd', gen=g_13dbd, trials=400, void=True),
    dict(name='Sim_1001f454', orig=0x1001f454, c='Sim_1001f454', gen=g_1f454, trials=400, void=True),
    dict(name='Sim_1001f47d', orig=0x1001f47d, c='Sim_1001f47d', gen=g_1f47d, trials=300, void=True),
    dict(name='Sim_1002009d', orig=0x1002009d, c='Sim_1002009d', gen=g_2009d, trials=300, void=True),
    dict(name='Sim_10027a8f', orig=0x10027a8f, c='Sim_10027a8f', gen=g_27a8f, trials=400, void=True),
    dict(name='Sim_100284df', orig=0x100284df, c='Sim_100284df', gen=g_284df, trials=500, void=True),
    dict(name='Sim_1002d3a5', orig=0x1002d3a5, c='Sim_1002d3a5', gen=g_2d3a5, trials=300),
    dict(name='Sim_1002d3ca', orig=0x1002d3ca, c='Sim_1002d3ca', gen=g_2d3ca, trials=300, void=True),
    dict(name='Sim_10034777', orig=0x10034777, c='Sim_10034777', gen=g_34777, trials=400, void=True),
    dict(name='Sim_100347b9', orig=0x100347b9, c='Sim_100347b9', gen=g_347b9, trials=300, void=True),
    dict(name='Sim_1004c362', orig=0x1004c362, c='Sim_1004c362', gen=g_4c362, trials=500),
    dict(name='Sim_1005502a', orig=0x1005502a, c='Sim_1005502a', gen=g_5502a, trials=300, void=True),
    dict(name='Sim_1005ae58', orig=0x1005ae58, c='Sim_1005ae58', gen=g_5ae58, trials=300, void=True),
    dict(name='Sim_10072d16', orig=0x10072d16, c='Sim_10072d16', gen=g_72d16, trials=300, void=True),
    dict(name='Sim_10072d3a', orig=0x10072d3a, c='Sim_10072d3a', gen=g_72d3a, trials=500, void=True),
    dict(name='Sim_100788fc', orig=0x100788fc, c='Sim_100788fc', gen=g_788fc, trials=300, void=True),
    dict(name='Sim_10084b57', orig=TRAMP, c='Test_10084b57', gen=g_10084b57, trials=500),
    dict(name='Sim_1008b757', orig=0x1008b757, c='Sim_1008b757', gen=g_1008b757, trials=300, void=True),
    dict(name='Sim_1008b773', orig=0x1008b773, c='Sim_1008b773', gen=g_1008b757, trials=300, void=True),
    dict(name='Sim_10098811', orig=0x10098811, c='Sim_10098811', gen=g_98811, trials=300, void=True),
    dict(name='Sim_100989a1', orig=0x100989a1, c='Sim_100989a1', gen=g_989a1, trials=300, void=True),
    dict(name='Sim_100a6009', orig=0x100a6009, c='Sim_100a6009', gen=g_a6009, trials=300, void=True),
    dict(name='Sim_100a9ca3', orig=0x100a9ca3, c='Sim_100a9ca3', gen=g_a9ca3, trials=300, void=True),
    dict(name='Sim_100aa351', orig=0x100aa351, c='Sim_100aa351', gen=g_aa351, trials=500, void=True),
    dict(name='Sim_100aa454', orig=0x100aa454, c='Sim_100aa454', gen=g_aa351, trials=500, void=True),
    dict(name='Sim_100aa812', orig=0x100aa812, c='Sim_100aa812', gen=g_aa812, trials=300, void=True),
    dict(name='Sim_100aa863', orig=0x100aa863, c='Sim_100aa863', gen=g_aa863, trials=300),
    dict(name='Sim_100aaf9b', orig=0x100aaf9b, c='Sim_100aaf9b', gen=g_aaf9b, trials=500, void=True),
    dict(name='Sim_100ac58e', orig=0x100ac58e, c='Sim_100ac58e', gen=g_ac58e, trials=500, void=True),
    dict(name='Sim_100ae64d', orig=0x100ae64d, c='Sim_100ae64d', gen=g_ae64d, trials=400),
    dict(name='Sim_100b5b76', orig=0x100b5b76, c='Sim_100b5b76', gen=g_b5b76, trials=300, void=True),
    dict(name='Sim_100c6e87', orig=0x100c6e87, c='Sim_100c6e87', gen=g_c6e87, trials=300, void=True),
    dict(name='Sim_100c81b5', orig=0x100c81b5, c='Sim_100c81b5', gen=g_c81b5, trials=300, void=True),
    dict(name='Sim_100cbe68', orig=0x100cbe68, c='Sim_100cbe68', gen=g_cbe68, trials=300, void=True),
    dict(name='Sim_100d1510', orig=0x100d1510, c='Sim_100d1510', gen=g_d1510, trials=300),
    dict(name='Sim_100d15e0', orig=0x100d15e0, c='Sim_100d15e0', gen=g_d15e0, trials=500, void=True),
    dict(name='Sim_100d15fd', orig=0x100d15fd, c='Sim_100d15fd', gen=g_d15fd, trials=500, void=True),
    dict(name='Sim_100d1dd0', orig=0x100d1dd0, c='Sim_100d1dd0', gen=g_d1dd0, trials=200, void=True),
    dict(name='Sim_100d37ea', orig=0x100d37ea, c='Sim_100d37ea', gen=g_d37ea, trials=400, void=True),
    dict(name='Sim_100d3830', orig=0x100d3830, c='Sim_100d3830', gen=g_d3830, trials=300, void=True),
]
