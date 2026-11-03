# Differential tests for round 5, letter z (see difftest.py and docs/checker.md).
#
# These functions call original functions that are not converted yet (SIM_FN calls). Each such callee is
# replaced in the emulator by a stub (a `ret` written over its first byte, with a code hook): the same code
# address serves the original and the C run. The stub appends a record (callee id and its arguments) to a log
# in argument memory, which the checker compares after both runs, and answers with a value that depends only
# on the number of calls so far (so both runs get the same answers). Pointers into the stack are logged as a
# marker; a stub may also write results through pointer arguments (out parameters) and log what a pointer
# argument points to. A 16-bit argument whose upper half is garbage in the original is masked in the log.
# The callees that are converted (Sim_1002828e, Sim_1008a16d, ...) run as real code on both sides.
# Windows imports are stubbed by the checker itself (HeapFree, SendMessageA, MessageBoxA).
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW

SOURCES = [
    'sim/unsorted_1000/w5z_calls.c', 'sim/unsorted_1001/w5z_calls.c', 'sim/unsorted_1002/w5z_calls.c',
    'sim/unsorted_1004/w5z_calls.c', 'sim/unsorted_1006/w5z_calls.c', 'sim/unsorted_1007/w5z_calls.c',
    'sim/unsorted_1008/w5z_calls.c', 'sim/unsorted_1009/w5z_calls.c', 'sim/unsorted_100a/w5z_calls.c',
    'sim/unsorted_100b/w5z_calls.c', 'sim/unsorted_100c/w5z_calls.c', 'sim/unsorted_100d/w5z_calls.c',
    # converted callees and what they need
    'sim/win/w4a_heap.c', 'sim/win/w4v_heap.c', 'sim/view/w2l_lists.c', 'sim/view/w2d_view.c', 'sim/view/objlist.c',
    'sim/view/matrix.c', 'sim/view/viewrect.c', 'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_100d/w2j_nodes.c',
    'sim/unsorted_1000/w3s_model.c', 'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1008/w4w_display.c',
    'sim/unsorted_1000/w2j_slots.c', 'sim/unsorted_1001/w2j_misc.c', 'sim/unsorted_1003/w2e_mono.c',
    'sim/unsorted_1005/w3s_groups.c', 'sim/unsorted_100b/w4v_wrappers.c', 'sim/unsorted_1000/w2e_mono.c',
    'sim/unsorted_1007/node_flag.c', 'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_1001/w3s_group.c',
    'sim/util/lookup.c', 'sim/unsorted_1004/tables.c', 'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_100c/w4v_timers.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
LOGMAX = 60
RECW = 16
IND = 0x00F00300        # target of an indirect call (a stub of its own)
ST = {}


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def put(ctx, addr, fmt, *vals):
    out = b''
    for f, v in zip(fmt, vals):
        size = struct.calcsize('<' + f)
        out += (v & ((1 << (8 * size)) - 1)).to_bytes(size, 'little')
    ctx.m.uc.mem_write(addr, out)


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little')


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def s32(v):
    v &= 0xffffffff
    return v - (1 << 32) if v >= 1 << 31 else v


# ---------------------------------------------------------------------------------------------
# stubs for the unconverted callees: address -> (number of arguments, options)
#   mask {i: m}   log argument i & m
#   deref {i: n}  also log n bytes (as dwords) of the memory argument i points to
#   outs (i, ...) write a dword (from ST['outs']) through each of these pointer arguments
#   fn            callable(m, uc, args, n, default) -> eax, may write memory
# ---------------------------------------------------------------------------------------------
def _fe830(m, uc, a, n, d):         # reads at most `count` bytes: the answer never exceeds the count
    return [a[2], a[2] // 2, 0, max(a[2] - 1, 0), a[2]][n % 5]


def _4b(m, uc, a, n, d):            # answers about the size read: count, one less, none, more
    c = a[2]
    return [c, max(c - 1, 0), 0, c + 1, c][n % 5]


def _fc780(m, uc, a, n, d):         # "sprintf": puts the prepared text into the destination
    m.uc.mem_write(a[0], ST['text'] + b'\0')
    return len(ST['text'])


def _100620(m, uc, a, n, d):        # "bsearch": no hit, or the prepared entry
    return ST['found'][n % len(ST['found'])]


STUBS = {
    0x1002feef: (1, {}), 0x1000a6f0: (2, {}), 0x1000ea40: (7, {'mask': {5: 0xffff}}), 0x1005264d: (2, {}),
    0x10016117: (2, {}), 0x10010699: (2, {}), 0x10016d83: (2, {}), 0x10020256: (2, {}), 0x100246b3: (12, {}),
    0x100282d4: (4, {}), 0x10027fb8: (1, {}), 0x1002ce45: (7, {'outs': (4, 5, 6)}), 0x100fd850: (2, {}),
    0x100febc0: (4, {'fn': _4b}), 0x1004ee50: (1, {}), 0x100d3510: (3, {}), 0x1006519d: (3, {}),
    0x100766ca: (1, {}), 0x1002c8cb: (4, {}), 0x1010026a: (0, {}), 0x10098721: (2, {}), 0x1008a94d: (1, {}),
    0x100a7c4c: (1, {}), 0x100ba7d5: (3, {}), 0x100bb2b0: (2, {}), 0x100a9c25: (1, {}), 0x10060681: (4, {}),
    0x1006123b: (5, {}), 0x100a9ba7: (1, {}), 0x100fe830: (4, {'fn': _fe830}), 0x100b469d: (0, {}),
    0x1003b534: (0, {}), 0x100bc280: (2, {}), 0x100b48de: (2, {}), 0x10100620: (5, {'deref': {0: 4}, 'fn': _100620}),
    0x100fc780: (4, {'fn': _fc780}), 0x1005f9aa: (1, {'deref': {0: 16}}), 0x100c4b67: (1, {}), 0x100c4a1f: (1, {}),
    0x100ca670: (3, {}), 0x100d2306: (1, {}), 0x100d255e: (1, {}), 0x100d3427: (1, {}), IND: (1, {}),
}
IDS = {a: i + 1 for i, a in enumerate(sorted(STUBS))}


def _hook(m):
    def make(addr):
        nargs, opt = STUBS[addr]

        def cb(uc, address, size, ud):
            try:
                esp = uc.reg_read(UC_X86_REG_ESP)
                a = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
                log = ST['log']
                n = struct.unpack('<I', bytes(uc.mem_read(log, 4)))[0]
                rec = [IDS[addr]]
                for i, v in enumerate(a):
                    rec.append(0x5afe0000 + i if STACK_LO <= v <= STACK_HI else v & opt.get('mask', {}).get(i, 0xffffffff))
                for i, nb in opt.get('deref', {}).items():
                    try:
                        data = bytes(uc.mem_read(a[i], nb))
                    except Exception:
                        data = b'\xad\xde\xad\xde' * (nb // 4)
                    rec += list(struct.unpack('<%dI' % (nb // 4), data))
                ret = ST['ret'][addr][n % len(ST['ret'][addr])]
                if 'outs' in opt:
                    for k, i in enumerate(opt['outs']):
                        m.uc.mem_write(a[i], struct.pack('<I', ST['outs'][(n * 3 + k) % len(ST['outs'])]))
                if 'fn' in opt:
                    ret = opt['fn'](m, uc, a, n, ret)
                if n < LOGMAX:
                    rec += [0] * (RECW - len(rec))
                    m.uc.mem_write(log + 4 + n * RECW * 4, struct.pack('<%dI' % RECW, *rec))
                m.uc.mem_write(log, struct.pack('<I', n + 1))
                uc.reg_write(UC_X86_REG_EAX, ret & 0xffffffff)
            except Exception as e:
                ST['err'] = repr(e)
                raise
        return cb

    for addr in STUBS:
        m.uc.mem_write(addr, b'\xc3')
        m.uc.hook_add(UC_HOOK_CODE, make(addr), None, addr, addr)


def begin(ctx, ret=None, outs=False):
    """Installs the stubs (once) and prepares a fresh log and the answers of the stubs."""
    m = ctx.m
    if not getattr(m, '_w5z', False):
        _hook(m)
        m._w5z = True
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    r = ctx.rng
    ST['log'] = ctx.buf(4 + LOGMAX * RECW * 4, 'zero')
    ST['ret'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32), 0, 1]) for _ in range(8)] for a in STUBS}
    for a, seq in (ret or {}).items():
        k = r.randrange(len(seq))
        ST['ret'][a] = seq[k:] + seq[:k]          # the first answer matters most: most callees are called once
    ST['outs'] = [r.choice([0, 1, 0xffffffff, r.getrandbits(32)]) for _ in range(9)]
    ST['text'] = b''
    ST['found'] = [0]


def bools(*addrs):
    return {a: [0, 1, 1, 0, 0, 1, 1, 0] for a in addrs}


# ---------------------------------------------------------------------------------------------
# worlds copied from the specs of the converted callees
# ---------------------------------------------------------------------------------------------
def slot_world(ctx):
    """Pointer table 0x109e8b40 with records, the current group and its slot count (spec_w2j.py)."""
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


def bucket(key, name4):
    total = key + sum(b - 256 if b >= 128 else b for b in name4)
    total = s32(total)
    r = abs(total) % 0x3f1
    return -r if total < 0 else r


def hash_world(ctx, n=None):
    """Hash table of nodes with a tail list state; returns [(node, key, name bytes)] (spec_w2j.py)."""
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
        put(ctx, node + 0xc, 'II', rg.getrandbits(32) if rg.random() < 0.2 else 0, 0)
        w32(ctx, tab + 4 * b, node)
    w32(ctx, 0x1015dc30, tab)
    if nodes and rg.random() < 0.6:
        w32(ctx, 0x109f13f0, rg.choice(nodes)[0])
        w32(ctx, 0x109f13f4, rg.choice(nodes)[0])
    else:
        w32(ctx, 0x109f13f0, 0)
        w32(ctx, 0x109f13f4, 0)
    return nodes


def lookup_args(ctx, nodes):
    rg = ctx.rng
    if nodes and rg.random() < 0.7:
        node, key, name = rg.choice(nodes)
        if rg.random() < 0.15:
            key = key + 0x10000 if rg.random() < 0.5 else key ^ 1
    else:
        key = rg.choice([rg.randrange(0, 16), rg.randrange(0, 0x10000), -1, -5, 0x7fffffff, 0x80000000 | rg.getrandbits(8)])
        name = rb(ctx, 4)
    return key & 0xffffffff, ctx.buf(4, name)


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
    wb(ctx, 0x10a3ae20, rb(ctx, 0x60))
    wb(ctx, 0x10130ed0, rb(ctx, 0x60))


def fvec(r, n):
    out = []
    for _ in range(n):
        c = r.random()
        out.append(r.choice([0.0, 1.0, -1.0, 0.5, 2.0]) if c < 0.15 else r.uniform(-100, 100) if c < 0.6 else r.uniform(-1, 1))
    return struct.pack('<%df' % n, *out)


def model_block(ctx, nv, nn):
    r = ctx.rng
    off = 0x18 + nv * 0x58 + r.choice([0, 0, 8])
    b = zbuf(ctx, off + nn * 0x28 + 16)
    wb(ctx, b + 0x18, fvec(r, nv * 22))
    wb(ctx, b + off, fvec(r, nn * 10))
    put(ctx, b + 4, 'HH', nv, nn)
    w32(ctx, b + 8, off)
    return b


# ---------------------------------------------------------------------------------------------
# generators
# ---------------------------------------------------------------------------------------------
def g_none(ctx):
    begin(ctx)
    return []


def g_u32(ctx):
    begin(ctx)
    return [ctx.u32()]


def g_u32x2(ctx):
    begin(ctx)
    return [ctx.u32(), ctx.u32()]


def g_u32x3(ctx):
    begin(ctx)
    return [ctx.u32(), ctx.u32(), ctx.u32()]


def g_u32x4(ctx):
    begin(ctx)
    return [ctx.u32() for _ in range(4)]


def g_10006d75(ctx):
    r = ctx.rng
    begin(ctx, bools(0x100282d4))
    nodes = hash_world(ctx)
    key, name = lookup_args(ctx, nodes)
    w32(ctx, 0x1014ceb8, name)
    ids = []
    for _ in range(r.choice([0, 1, 2, 3, 5, 6])):
        if nodes and r.random() < 0.6:
            ids.append(r.choice(nodes)[1])
        else:
            ids.append(r.choice([0, 1, 2, 5, 0x20, 0x7fffffff, r.randrange(0, 0x10000), 0xfffffffe]))
    ids.append(0xffffffff)
    wb(ctx, 0x1012da40, struct.pack('<%dI' % len(ids), *ids))
    return []


def chain(ctx, n):
    head = 0
    for _ in range(n):
        nb = newbuf(ctx, 0x20)
        put(ctx, nb + 0xc, 'I', head)
        head = nb
    return head


def g_10009369(ctx):
    r = ctx.rng
    begin(ctx)
    obj = newbuf(ctx, 0x100)
    w32(ctx, obj + 0x2c, chain(ctx, r.choice([0, 0, 1, 2, 3, 5])))
    w32(ctx, 0x1017b8f8, r.choice([0x00c10000, r.getrandbits(32)]))
    return [obj]


def list_obj(ctx, size=0x200):
    """A node of the object lists: link fields that are 0 or point at spare buffers."""
    r = ctx.rng
    spares = [newbuf(ctx, 0x100) for _ in range(3)]
    obj = zbuf(ctx, size)
    for off in (4, 8, 0xc, 0x10, 0x14, 0x18, 0x1c, 0x20):
        w32(ctx, obj + off, r.choice([0, 0, spares[0], spares[1], spares[2]]))
    put(ctx, obj, 'H', r.choice([0, 0, 0x800, 0x1000, r.getrandbits(16)]))
    return obj


def g_10009741(ctx):
    r = ctx.rng
    begin(ctx)
    obj = list_obj(ctx)
    w32(ctx, obj + 0x2c, chain(ctx, r.choice([0, 0, 1, 2, 3])))
    w32(ctx, 0x1017b8f8, r.choice([0x00c10000, r.getrandbits(32)]))
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    w32(ctx, 0x1017c8cc, obj if r.random() < 0.15 else r.choice([0, 0x1234]))
    return [0 if r.random() < 0.12 else obj]


def g_1000ed25(ctx):
    begin(ctx)
    return [ctx.u32() for _ in range(7)]


def g_100100a2(ctx):
    begin(ctx)
    w32(ctx, 0x1012ff58, ctx.rng.choice([0, 1, 0x1234, ctx.u32()]))
    w32(ctx, 0x1013bb40, ctx.u32())
    return [ctx.u32(), ctx.u32()]


def g_10010acd(ctx):
    begin(ctx)
    slot_world(ctx)
    return []


def g_10010b0b(ctx):
    begin(ctx)
    slot_world(ctx)
    w32(ctx, 0x1015d100, ctx.rng.choice([0, 0, 0, 1, 1, 2, 2, 3, 0xffffffff, ctx.u32()]))
    return []


def g_10010f70(ctx):
    r = ctx.rng
    begin(ctx)
    mono(ctx)
    nrec = r.choice([3, 6, 10])
    recs = []
    for i in range(nrec):
        b = ctx.buf(0x40, rb(ctx, 0x40))
        put(ctx, b, 'I', r.choice([1, 1, 1, 0, 2]))
        put(ctx, b + 4, 'i', r.choice([r.randrange(0, 6), r.randrange(0, 6), -1]))
        put(ctx, b + 0x10, 'I', r.choice([0, 1, 2, 7, 8, r.randrange(0, 9)]))
        put(ctx, b + 0x1c, 'H', r.choice([0, 0, 0, 2, 4, 6, 8, 0x10, 0xfff1]))
        recs.append(b)
        w32(ctx, 0x109e8b40 + 4 * i, b)
    w32(ctx, 0x1015dc64, nrec)
    for g in range(16):
        gb = 0x109b7d40 + g * 0x6c
        w32(ctx, gb, r.randrange(0, 6))             # a current entry is known: the search of 0x10012843 is not needed
        w32(ctx, gb + 4, r.choice([0, 1, 2, 3, 4, 4]))
        w32(ctx, gb + 8, r.choice([0, 0, 1]))
        for j in range(5):
            w32(ctx, gb + 0x1c + 4 * j, r.randrange(0, nrec))
    w32(ctx, 0x104737e0, r.choice([0, 1, 2, 3, 4, 5, 0xffffffff]))
    return []


def g_1002cdb0(ctx):
    begin(ctx)
    return [ctx.u32() for _ in range(4)]


def g_10028534(ctx):
    begin(ctx)
    w32(ctx, 0x109f13f0, ctx.rng.choice([0, 0, 1, 0x1234, ctx.u32()]))
    return []


def text_name(ctx, n=12):
    return ctx.buf(n, 'text')


def g_1004db2e(ctx):
    begin(ctx)
    w32(ctx, 0x1015f344, ctx.rng.choice([0, 1, ctx.u32()]))
    return [text_name(ctx)]


def g_1004db5b(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1015f344, r.choice([0, 1, ctx.u32()]))
    return [newbuf(ctx, 0x100), r.choice([0, 1, 2, 10, 0x40, 0xff, 0xffffffff, ctx.u32()])]


def g_1004f158(ctx):
    r = ctx.rng
    begin(ctx)
    obj = newbuf(ctx, 0x100)
    blk = newbuf(ctx, 0x700)
    w32(ctx, obj + 0x28, blk)
    w32(ctx, 0x1015f3a8, r.choice([0, 0, 1, 0x1234]))
    return [obj]


def g_1006535f(ctx):
    begin(ctx)
    return [ctx.u32(), ctx.u32(), ctx.u32(), newbuf(ctx, 0x20)]


def g_10074c3d(ctx):
    r = ctx.rng
    begin(ctx, {0x100766ca: [0, 1, 2, 0xffffffff, 0, 1, r.getrandbits(32), 1]})
    n = r.choice([0, 1, 3, 6])
    head = 0
    keys = []
    for _ in range(n):
        node = zbuf(ctx, 0x20)
        k = r.choice([1, 2, 3, 0x10, r.getrandbits(32)])
        put(ctx, node, 'I', k)
        put(ctx, node + 8, 'I', r.choice([0, 1, 7]))
        put(ctx, node + 0x14, 'I', head)
        head = node
        keys.append(k)
    w32(ctx, 0x1015dc14, head)
    w32(ctx, 0x1015dbcc, r.choice([0, 0, 0, 1]))
    return [r.choice(keys) if keys and r.random() < 0.7 else r.choice([0, 5, ctx.u32()])]


def g_100788ba(ctx):
    r = ctx.rng
    begin(ctx)
    obj = newbuf(ctx, 0x100)
    put(ctx, obj + 4, 'H', r.choice([0, 0, 1, 0x8000, 0xff00, r.getrandbits(16)]))
    put(ctx, obj + 0x30, 'I', r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x109e89d8, ctx.u32())
    return [obj]


def g_1008663f(ctx):
    r = ctx.rng
    begin(ctx, {0x1010026a: [0, 1, 2, 1, 0, 0xffffffff, 1, 0]})
    w32(ctx, 0x1015dca0, r.choice([0, 1, 1, ctx.u32()]))
    w32(ctx, 0x1017b8f0, r.choice([0, 0x10001, ctx.u32()]))
    return []


def g_10098a5d(ctx):
    begin(ctx)
    w32(ctx, 0x1015d1b8, newbuf(ctx, 0x40))
    return [ctx.u32(), ctx.u32()]


def g_100a773b(ctx):
    begin(ctx)
    w32(ctx, 0x1014cf8c, ctx.u32())
    return []


def g_100a7758(ctx):
    begin(ctx, bools(0x100282d4))
    nodes = hash_world(ctx)
    key, name = lookup_args(ctx, nodes)
    w32(ctx, 0x1014cf8c, ctx.u32())
    return [key, name]


def g_100a77a7(ctx):
    begin(ctx, bools(0x100ba7d5))
    return [ctx.u32(), ctx.u32()]


def g_100a9ccf(ctx):
    begin(ctx)
    return [newbuf(ctx, 0x40), ctx.u32(), ctx.u32(), ctx.u32()]


def g_100aa0da(ctx):
    begin(ctx)
    obj = newbuf(ctx, 0x200)
    return [obj, ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32()]


def g_100aa307(ctx):
    r = ctx.rng
    begin(ctx)
    obj = newbuf(ctx, 0x200)
    blk = model_block(ctx, r.choice([0, 1, 2, 3, 6]), r.choice([0, 1, 2, 5]))
    t = zbuf(ctx, 0x4c + 48 + 16)
    wb(ctx, t + 0x4c, fvec(r, 12))
    w32(ctx, obj + 0x28, t)
    w32(ctx, obj + 0x30, blk)
    w32(ctx, obj + 0x9c, ctx.u32())
    return [obj]


def obj_tree(ctx, depth, with_node):
    """An object with a chain of children (+4, next at +8) and, when asked, a list node at +0x8c."""
    r = ctx.rng
    obj = zbuf(ctx, 0x100)
    if with_node:
        node = list_obj(ctx, 0x40)
        w32(ctx, obj + 0x8c, node if r.random() < 0.85 else 0)
    first = 0
    if depth > 0:
        for _ in range(r.choice([0, 1, 2, 3])):
            c = obj_tree(ctx, depth - 1, with_node)
            put(ctx, c + 8, 'I', first)
            first = c
    w32(ctx, obj + 4, first)
    return obj


def g_100aa406(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1017c8cc, r.choice([0, 1, 0x1234]))
    return [obj_tree(ctx, r.choice([0, 1, 2, 3]), True)]


def g_100aa541(ctx):
    r = ctx.rng
    begin(ctx)
    obj = obj_tree(ctx, r.choice([0, 1, 2, 3]), True)
    return [obj, ctx.u32()]


def g_100ae470(ctx):
    r = ctx.rng
    begin(ctx)
    f = newbuf(ctx, 8)
    buf = newbuf(ctx, 0x300)
    return [f, buf, r.choice([0, 1, 2, 7, 0x46, 0x47, 0x80, r.randrange(0, 0x200)])]


def g_100b388a(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1019a07c, r.choice([0, 1, 1, ctx.u32()]))
    w32(ctx, 0x1017d228, ctx.u32())
    w32(ctx, 0x1017d218, r.choice([0, 1, 2, 3]))
    for i in range(3):
        rec = 0x101a2100 + i * 0x88
        node = zbuf(ctx, 0x20)      # a timer node without a callback: the list walk skips it
        put(ctx, node + 0x10, 'i', r.choice([0, 5, -3]))
        w32(ctx, rec + 0x84, r.choice([0, node, node]))
        w32(ctx, rec + 0x18, r.choice([0, 0x800, 1, 0xfffff7ff]))
    w32(ctx, 0x1017b8f8, r.choice([0x00c10000, ctx.u32()]))
    return []


def g_100b3f94(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1017d224, r.choice([0, 1, 2, 3, 0xffffffff, 0xfffffffe, 9, 20, r.randrange(0, 30)]))
    wb(ctx, 0x101c4220, rb(ctx, 0x100))
    w32(ctx, 0x101507f8, ctx.u32())
    return [ctx.u32()]


def g_100b418e(ctx):
    r = ctx.rng
    begin(ctx)
    n = r.choice([0, 1, 2, 3, 5, 0xffffffff, 0x80000000])
    w32(ctx, 0x1017d218, n)
    wb(ctx, 0x101a2100, rb(ctx, 0x88 * 6))
    return []


def g_100b787f(ctx):
    r = ctx.rng
    begin(ctx)
    tab = 0 if r.random() < 0.15 else ctx.buf(8, rb(ctx, 8))
    entry = newbuf(ctx, 8)
    ST['found'] = [r.choice([0, entry, entry, 0]) for _ in range(4)]
    return [tab, ctx.u32()]


def g_100bc4f7(ctx):
    r = ctx.rng
    begin(ctx)
    mono(ctx)
    alpha = b'abAB01 \n\n'
    ST['text'] = bytes(r.choice(alpha) for _ in range(r.choice([0, 1, 5, 20, 79, 80, 120])))
    w32(ctx, 0x1015d080, r.choice([0, 1, 40, 78, 79, 80, 81, r.randrange(0, 90)]))
    w32(ctx, 0x1012dcac, r.choice([3, 4, 20, 22, 23, 24, 0, r.randrange(0, 30)]))
    w32(ctx, 0x1012dc78, ctx.u32())
    return [ctx.u32(), ctx.u32()]


def g_100c4d63(ctx):
    begin(ctx)
    return [0 if ctx.rng.random() < 0.15 else newbuf(ctx, 0x40)]


def g_100cbdb8(ctx):
    begin(ctx)
    return [ctx.u32(), ctx.u32(), ctx.rng.choice([0, 1, 2, 0x100, ctx.u32()])]


def g_100cbdea(ctx):
    r = ctx.rng
    begin(ctx)
    wb(ctx, 0x109e8b40, rb(ctx, 0x40))
    w32(ctx, 0x1017b8c8, r.randrange(0, 16))
    return [ctx.u32()]


def tree(ctx, depth):
    r = ctx.rng
    if depth == 0 or r.random() < 0.25:
        return 0
    n = zbuf(ctx, 0x20)
    put(ctx, n + 8, 'II', tree(ctx, depth - 1), tree(ctx, depth - 1))
    return n


def g_100d18e8(ctx):
    r = ctx.rng
    begin(ctx)
    return [0 if r.random() < 0.1 else tree(ctx, r.choice([1, 2, 3, 5, 6]))]


def g_100d33ec(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1017e650, r.choice([0, 1, 1, 0x1234]))
    w32(ctx, 0x1017e65c, r.choice([0, 1, 2, 0xffffffff, 0xfffffffd, 0x80000000, 0x7fffffff, ctx.u32()]))
    w32(ctx, 0x10152424, ctx.u32())
    w32(ctx, 0x10152420, ctx.u32())
    return []


def g_100d36d6(ctx):
    begin(ctx)
    w32(ctx, 0x1017e654, ctx.u32())
    return [ctx.u32(), ctx.u32(), ctx.u32()]


def g_100d379f(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1017e660, r.choice([0, 1, 1, 1, 0x1234]))
    w32(ctx, 0x1017e640, ctx.u32())
    w32(ctx, 0x1017e644, r.choice([0, newbuf(ctx, 0x20)]))
    tabl = zbuf(ctx, 0x40)
    w32(ctx, tabl + 0x18, IND)
    w32(ctx, 0x1015e688, tabl)
    w32(ctx, 0x1017b8f8, r.choice([0x00c10000, ctx.u32()]))
    w32(ctx, 0x1017e664, ctx.u32())
    w32(ctx, 0x1014ced4, ctx.u32())
    w32(ctx, 0x1017e650, r.choice([0, 1, 1]))
    w32(ctx, 0x1017e65c, r.choice([0, 1, 0xffffffff, ctx.u32()]))
    w32(ctx, 0x10152424, ctx.u32())
    return []


def spec(name, addr, gen, trials=300, **kw):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials)
    d.update(kw)
    return d


SPECS = [
    spec('Sim_10006d75', 0x10006d75, g_10006d75, 400, void=True),
    spec('Sim_10009369', 0x10009369, g_10009369, 300, void=True),
    spec('Sim_10009741', 0x10009741, g_10009741, 400, void=True),
    spec('Sim_1000c5f8', 0x1000c5f8, g_u32, 300),
    spec('Sim_1000ed25', 0x1000ed25, g_1000ed25, 300),
    spec('Sim_100100a2', 0x100100a2, g_100100a2, 300),
    spec('Sim_10010acd', 0x10010acd, g_10010acd, 400),
    spec('Sim_10010b0b', 0x10010b0b, g_10010b0b, 400),
    spec('Sim_10010f70', 0x10010f70, g_10010f70, 400, void=True),
    spec('Sim_1001661f', 0x1001661f, g_u32, 300),
    spec('Sim_100203b8', 0x100203b8, g_u32, 300),
    spec('Sim_10024616', 0x10024616, lambda c: (begin(c), [c.u32() for _ in range(8)])[1], 300),
    spec('Sim_1002851b', 0x1002851b, g_u32x2, 300),
    spec('Sim_10028534', 0x10028534, g_10028534, 300),
    spec('Sim_1002cdb0', 0x1002cdb0, g_1002cdb0, 300),
    spec('Sim_1004db2e', 0x1004db2e, g_1004db2e, 300),
    spec('Sim_1004db5b', 0x1004db5b, g_1004db5b, 300),
    spec('Sim_1004f158', 0x1004f158, g_1004f158, 300, void=True),
    spec('Sim_10061b62', 0x10061b62, g_none, 300),
    spec('Sim_1006535f', 0x1006535f, g_1006535f, 300),
    spec('Sim_10074c3d', 0x10074c3d, g_10074c3d, 500),
    spec('Sim_100788ba', 0x100788ba, g_100788ba, 400, void=True),
    # deleted: only the original SimMain reaches it (DECISIONS.md, "The last original code")
    # spec('Sim_1008663f', 0x1008663f, g_1008663f, 300, void=True),
    spec('Sim_10098710', 0x10098710, g_none, 300),
    spec('Sim_10098a5d', 0x10098a5d, g_10098a5d, 300, void=True),
    spec('Sim_100a773b', 0x100a773b, g_100a773b, 300),
    spec('Sim_100a7758', 0x100a7758, g_100a7758, 600),
    spec('Sim_100a77a7', 0x100a77a7, g_100a77a7, 300),
    spec('Sim_100a9ccf', 0x100a9ccf, g_100a9ccf, 300),
    spec('Sim_100aa0da', 0x100aa0da, g_100aa0da, 300, void=True),
    spec('Sim_100aa307', 0x100aa307, g_100aa307, 500, void=True, fp=True),
    spec('Sim_100aa406', 0x100aa406, g_100aa406, 500, void=True),
    spec('Sim_100aa541', 0x100aa541, g_100aa541, 500, void=True),
    spec('Sim_100ae470', 0x100ae470, g_100ae470, 400),
    spec('Sim_100b388a', 0x100b388a, g_100b388a, 400, void=True),
    spec('Sim_100b3f94', 0x100b3f94, g_100b3f94, 400, void=True),
    spec('Sim_100b418e', 0x100b418e, g_100b418e, 300, void=True),
    spec('Sim_100b787f', 0x100b787f, g_100b787f, 400),
    spec('Sim_100bc4f7', 0x100bc4f7, g_100bc4f7, 400),
    spec('Sim_100c4d63', 0x100c4d63, g_100c4d63, 300),
    spec('Sim_100cbdb8', 0x100cbdb8, g_100cbdb8, 300),
    spec('Sim_100cbdea', 0x100cbdea, g_100cbdea, 300),
    spec('Tree_CountNodes', 0x100d18e8, g_100d18e8, 400),
    spec('Sim_100d25ce', 0x100d25ce, g_u32, 300),
    spec('Sim_100d33ec', 0x100d33ec, g_100d33ec, 400, void=True),
    spec('Sim_100d36d6', 0x100d36d6, g_100d36d6, 300),
    spec('Sim_100d379f', 0x100d379f, g_100d379f, 500, void=True),
]
