# Differential tests for round 5, letter b (see difftest.py and docs/checker.md).
#
# These functions call helpers of the DLL that are not converted yet. Most helpers are replaced in the emulator by a
# stub: a `ret` written over the helper's first byte plus a code hook that appends ('stub_<address>', arguments) to the
# checker's call log (compared between the original and the C run, like the Windows calls) and answers with a value
# chosen by the generator (or, for a few, by a small function that also writes the results the real helper would write).
# The stub address is the helper's own address, so the original and the C code reach the same stub. Stack addresses are
# logged as 'stack'. The C library routines that are pure computation run for real in the emulator, with the Windows
# control word 0x27f (53-bit precision): sprintf 0x100fc780, fmod 0x100fcb30, fabs 0x100fcb70, the function 0x10084766 and
# _strcmpi 0x100ff1e0.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG

SOURCES = [
    'sim/unsorted_1000/w5b_calls.c', 'sim/unsorted_1001/w5b_calls.c', 'sim/unsorted_1002/w5b_calls.c',
    'sim/unsorted_1003/w5b_calls.c', 'sim/unsorted_1004/w5b_calls.c', 'sim/unsorted_1005/w5b_calls.c',
    'sim/unsorted_1006/w5b_calls.c', 'sim/unsorted_1008/w5b_calls.c', 'sim/unsorted_1009/w5b_calls.c',
    'sim/unsorted_100a/w5b_calls.c', 'sim/unsorted_100b/w5b_calls.c', 'sim/unsorted_100c/w5b_calls.c',
    'sim/unsorted_100d/w5b_calls.c', 'sim/unsorted_100e/w5b_calls.c',
    '../tools/verify/w5b_test.c',
    # existing converted code the functions above call
    'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_100f/w2e_string.c',
    'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2j_slots.c',
    'sim/util/lookup.c', 'sim/unsorted_1001/w2j_misc.c',
    'sim/unsorted_1001/w2h_objects.c', 'sim/unsorted_1004/tables.c', 'sim/view/objlist.c',
    'sim/unsorted_1001/w2l_misc.c', 'sim/unsorted_1004/w3p_net.c', 'sim/unsorted_1007/w3p_net.c',
    'sim/unsorted_100a/w2f_links.c', 'sim/unsorted_100a/w2l_small.c',
    'sim/unsorted_1002/w3o_vectors.c', 'sim/unsorted_1002/w3q_collide.c',
    'sim/unsorted_1008/w3s_grid.c', 'sim/unsorted_1002/w2h_misc.c',
    'sim/win/w4a_heap.c', 'sim/win/w4u_gdi.c', 'sim/unsorted_1004/w2l_values.c',
    'sim/view/viewrect.c', 'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_100c/entries.c',
    'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1008/w4v_fade.c',
    # what those call in turn
    'sim/win/w4v_heap.c', 'sim/unsorted_1003/w3q_grid.c', 'sim/unsorted_1003/entry.c', 'sim/unsorted_1003/records.c',
    'sim/unsorted_1004/w2i_misc.c', 'sim/unsorted_1004/w2h_record.c', 'sim/unsorted_1004/fields.c',
    'sim/unsorted_1005/w2h_tables.c', 'sim/unsorted_1007/w2f_misc.c', 'sim/unsorted_1008/w3o_length.c',
    'sim/unsorted_1008/cell_link.c', 'sim/view/w2d_view.c', 'sim/view/w2l_lists.c', 'sim/util/ring.c',
    'sim/util/field.c', 'sim/unsorted_100d/w2f_records.c', 'sim/view/matrix.c', 'sim/unsorted_100d/w2i_select.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000
TRAMP = 0x00F00800
DPSEND = 0x00F00200      # the "dpSend" function pointer target of Sim_100409be (a `ret` with a hook)
CALLBACK = 0x00F00210    # the callback of Sim_100aab03
DRIVER = 0x00F00220      # the driver block function [[0x1015e688] + 0x18] used by Sim_1008c803 and Sim_1008c5e0

ST = {}


# ---- small helpers ------------------------------------------------------------------------------
def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def put(ctx, addr, fmt, *vals):
    out = b''
    for f, v in zip(fmt, vals):
        size = struct.calcsize('<' + f)
        out += (v & ((1 << (8 * size)) - 1)).to_bytes(size, 'little')
    ctx.m.uc.mem_write(addr, out)


def wf(ctx, addr, *v):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(v), *v))


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def s32(v):
    v &= 0xffffffff
    return v - (1 << 32) if v >= 1 << 31 else v


def cstr(ctx, data, extra=24):
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rb(ctx, extra))


def rfl(r, spread=100.0, special=0.12):
    """An ordinary float (no NaN, no infinity)."""
    c = r.random()
    if c < special:
        return r.choice([0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 2.0, 1e-7, -1e-7, 100.0, -100.0, 1e10, -1e10, 3.0e38, 1e-30, 1e5])
    return r.uniform(-spread, spread)


def rfbits(r, spread=100.0, special=0.12):
    return fbits(rfl(r, spread, special))


# ---- stubs --------------------------------------------------------------------------------------
# helper address -> (number of dword arguments, {argument number: mask})
STUBS = {
    0x1000b070: (2, {}), 0x1000b587: (2, {}), 0x10016117: (2, {}), 0x100200d1: (2, {}), 0x10020256: (2, {}),
    0x10017b3e: (2, {}), 0x10016de7: (2, {1: 0xffff}), 0x10016632: (1, {}),
    0x10027fb8: (1, {}), 0x10028d9f: (3, {}), 0x100a77a7: (2, {}), 0x10031acd: (5, {}), 0x100d3830: (2, {}),
    0x100fefe0: (1, {}), 0x100fef30: (1, {}), 0x100bc280: (2, {}), 0x100fd7e0: (3, {}),
    0x10028b90: (2, {}), 0x10062386: (0, {}), 0x1006a12d: (2, {}), 0x10083801: (0, {}), 0x100f01cc: (2, {}),
    0x100eb6e8: (6, {}), 0x100fe830: (4, {}), 0x10105750: (3, {}), 0x100a7f8c: (3, {2: 0xffff}),
    0x10060b76: (2, {}), 0x10060d2e: (2, {}), 0x100aa8cb: (1, {}), 0x1001403a: (2, {1: 0xffff}),
    0x100d3510: (3, {}), 0x100e2aa3: (5, {}), 0x100e2776: (3, {}),
}


def _fread(m, uc, a, n):
    """fread(buf, 1, 1, file): hands out the characters of this trial's line one at a time, then end of file."""
    seq = ST['fread']
    if n < len(seq):
        m.uc.mem_write(a[0], bytes([seq[n]]))
        return 1
    return 0


def _write(m, uc, a, n):
    if n == ST['write_fail']:
        return ST['write_fail_value'] & 0xffffffff
    return a[2]


def _b76(m, uc, a, n):
    """0x10060b76(obj + 0x3c, obj + 0xc): clobbers the dwords obj[0x30..0x38] the caller must put back."""
    m.uc.mem_write(a[1] + 0x24, ST['garbage'])
    return 0


def _e2aa3(m, uc, a, n):
    """0x100e2aa3(a, b, &result, d, &x): stores a result pointer through the third argument."""
    m.uc.mem_write(a[2], struct.pack('<I', ST['e2_result']))
    m.uc.mem_write(a[4], struct.pack('<I', 0x1234))
    return ST['e2_ret']


FNS = {0x100fe830: _fread, 0x10105750: _write, 0x10060b76: _b76, 0x100e2aa3: _e2aa3}


def _make_cb(m, addr, nargs, mask):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        raw = [struct.unpack('<I', bytes(uc.mem_read(esp + 4 + 4 * k, 4)))[0] for k in range(nargs)]
        args = []
        for k, v in enumerate(raw):
            if STACK_LO <= v <= STACK_HI:
                v = 'stack'
            elif k in mask:
                v &= mask[k]
            args.append(v)
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % addr, tuple(args)))
        if addr in FNS:
            val = FNS[addr](m, uc, raw, n)
        else:
            rets = ST['rets'].get(addr) or [0]
            val = rets[n % len(rets)]
        uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
    return cb


def _special_cb(m, kind):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        a = struct.unpack('<6I', bytes(uc.mem_read(esp + 4, 24)))
        n = len(m.api_log)
        if kind == 'dp':
            # dpSend(session, from, to, flag, buf, len): the 7 bytes of the message are part of the log
            try:
                data = bytes(uc.mem_read(a[4], 7))
            except Exception:
                data = b'?'
            m.api_log.append(('dp', (a[0], a[1], a[2], a[3], a[4], a[5], data)))
            uc.reg_write(UC_X86_REG_EAX, ST['dp_ret'] & 0xffffffff)
        elif kind == 'cb':
            m.api_log.append(('callback', (a[0],)))
            uc.reg_write(UC_X86_REG_EAX, 0x5555)
        else:
            m.api_log.append(('driver', (a[0],)))
            uc.reg_write(UC_X86_REG_EAX, 0)
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
    if not getattr(m, '_w5b', False):
        m._w5b = True
        for addr, (nargs, mask) in STUBS.items():
            m.raw.mem_write(addr, b'\xc3')
            m.raw.hook_add(UC_HOOK_CODE, _make_cb(m, addr, nargs, mask), None, addr, addr)
        for addr, kind in ((DPSEND, 'dp'), (CALLBACK, 'cb'), (DRIVER, 'driver')):
            m.raw.mem_write(addr, b'\xc3')
            m.raw.hook_add(UC_HOOK_CODE, _special_cb(m, kind), None, addr, addr)
        m.raw.mem_write(TRAMP, tramp_double(0x10047e1a, 2))
        m.raw.mem_map(0xb0000, 0x10000)         # the text screen that Mono_PutText writes to
        m.wr.append((0xb0000, 0x10000))
        m.snap0 = m.dump()
        m._index()          # the checker re-indexes when the snapshot changes; do it now, before this trial writes anything
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}
    ST['fread'] = b''
    ST['write_fail'] = -1
    ST['write_fail_value'] = 0
    ST['garbage'] = rb(ctx, 12)
    ST['e2_result'] = 0
    ST['e2_ret'] = 0
    ST['dp_ret'] = r.getrandbits(32)
    ST['alloc'] = 0
    ST['sdib'] = 1


def rets(addr, *values):
    ST['rets'][addr] = list(values)


# ---- shared worlds ------------------------------------------------------------------------------
def slot_world(ctx):
    """Pointer table 0x109e8b40 with records, the current group and its slot count (as in spec_w2j.py).
    Each record gets a block at +0x28 whose dword +0x4f0 is 2 most of the time."""
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
        block = newbuf(ctx, 0x600)
        put(ctx, block + 0x4f0, 'I', 2 if rg.random() < 0.7 else rg.choice([0, 1, 3, 4, 5, 6]))
        put(ctx, rec + 0x28, 'I', block)
        recs.append(rec)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
    put(ctx, recs[cur] + 8, 'I', group)
    w32(ctx, 0x1015dc64, count if rg.random() < 0.9 else rg.randrange(0, count + 1))
    w32(ctx, 0x1017b8c8, cur)
    for g in range(3):
        w32(ctx, 0x109b7d44 + g * 0x6c, rg.choice([0, 1, 2, 3, 4, 5, 6, 7, 3, 4]))
    return recs


def obj_tree(ctx, nodes, mode):
    """A tree of nodes (first child at +4, next sibling at +8). mode says what the +0x8c record is."""
    r = ctx.rng
    tree = []
    for i in range(nodes):
        node = zbuf(ctx, 0x200)
        put(ctx, node + 0x94, 'I', r.choice([0, 4, 4, 0x4000, 0x4004, 1]))
        if mode == 'list':        # the record Sim_1008a46e looks at: word0 with 0x1000 sometimes, word2 with 0x70 sometimes
            if r.random() < 0.7:
                rec = zbuf(ctx, 0x100)
                put(ctx, rec, 'HH', r.choice([0, 0x1000, 0x1000, 0x1800, 0x8000, 0xffff]), r.choice([0x70, 0x71, 0x7f, 0x00, 0x30, 0xf0, 0x170, 0xffff]))
                put(ctx, node + 0x8c, 'I', rec)
        elif mode == 'state':     # the record Sim_1002d890 changes
            if r.random() < 0.7:
                rec = zbuf(ctx, 0x100)
                put(ctx, rec, 'H', r.choice([0, 0x800, 0x1000, 0x1800, 0x8000, 0xffff]))
                put(ctx, rec + 0x34, 'I', r.choice([4, 0, 1]))
                put(ctx, node + 0x8c, 'I', rec)
        elif mode == 'free':
            if r.random() < 0.7:
                put(ctx, node + 0x8c, 'I', zbuf(ctx, 0x40))
        elif mode == 'names':
            names = [b'abc', b'ABC', b'abd', b'x', b'', b'Abc ', b'aBC']
            if r.random() < 0.7:
                put(ctx, node + 0x90, 'I', cstr(ctx, r.choice(names)))
        tree.append(node)
    for i in range(1, nodes):
        parent = tree[r.randrange(0, i)]
        put(ctx, tree[i] + 8, 'I', r32(ctx, parent + 4))
        put(ctx, parent + 4, 'I', tree[i])
    return tree


# ---- generators ---------------------------------------------------------------------------------
def g_mono(ctx):
    begin(ctx)
    r = ctx.rng
    ctx.m.uc.mem_write(0xb0000, bytes([0xaa]) * 0x2000)
    w32(ctx, 0x10a3ade8, r.choice([0, 1, 1, 1, 0x100]))
    w32(ctx, 0x1015d048, ctx.u32())
    w32(ctx, 0x1015d050, ctx.u32())
    return []


def g_ae00(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x200)
    put(ctx, obj + 0x1f4, 'I', r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x1017b8c8, r.randrange(0, 8))
    word = r.choice([0x200 | r.randrange(0, 8), 0x200 | r.randrange(0, 8), 0x200 | r.randrange(0, 8), r.getrandbits(16)])
    put(ctx, obj + 0x1aa, 'H', word)
    w32(ctx, 0x101654f2, ctx.u32())
    rets(0x1000b070, *[r.choice([0, 1, r.getrandbits(32)]) for _ in range(3)])
    rets(0x1000b587, *[r.choice([0, 5, r.getrandbits(32)]) for _ in range(3)])
    return [obj, r.choice([rfbits(r), r.getrandbits(32)])]


def g_c7b4(ctx):
    r = ctx.rng
    return [r.choice(list(range(0, 20)) + [0xffffffff, 0x80000000, 0x100, 0x109, 12, 9, 9, 12, 0, 1, ctx.u32()])]


def g_mode_set(ctx):
    begin(ctx)
    r = ctx.rng
    recs = slot_world(ctx)
    count = r32(ctx, 0x1015dc64)
    value = r.choice([1, 2, 3, 4, 5, 6, 7, 8, 0, -1, -3, r.randrange(1, 9), r.randrange(1, 9), 0x7fffffff, ctx.u32()])
    return [value & 0xffffffff, ctx.u32()]


def g_10951(ctx):
    begin(ctx)
    r = ctx.rng
    slot_world(ctx)
    w32(ctx, 0x1015d100, ctx.u32())
    return [r.choice([0, 0, 0, 1, 2, 3, -1, 5, ctx.u32()]) & 0xffffffff]


def g_1393e(ctx):
    begin(ctx)
    r = ctx.rng
    # tables read by Sim_100152fc (kinds 0x100, 0x200, 0x400) for the numbers 0..7
    ctx.m.uc.mem_write(0x109a8660, rb(ctx, 0x2c0))
    ctx.m.uc.mem_write(0x109e9b90, rb(ctx, 0x2c0))
    count = 8
    for i in range(count):
        rec = newbuf(ctx, 0x200)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
    obj = newbuf(ctx, 0x40)
    key = r.randrange(0, 16)
    put(ctx, obj + 8, 'I', key)
    put(ctx, obj + 0x10, 'I', ctx.u32())
    if key < 0x10:
        value = r.choice([0, 1, 2, 3, 0x7fffffff, 0x100000000 - 1])
        w32(ctx, 0x109b7d40 + key * 0x6c, value)
    w32(ctx, 0x1017b8c8, r32(ctx, 0x109b7d40 + key * 0x6c) if r.random() < 0.5 else r.randrange(0, 5))
    kind = r.choice([0x100, 0x200, 0x200, 0x400, 0x300, 0x0, 0x1000, 0x2200, 0xf000])
    word = (kind | r.randrange(0, 8)) if r.random() < 0.85 else r.getrandbits(16)
    rets(0x10020256, r.getrandbits(32), r.getrandbits(32))
    return [obj, word | (r.getrandbits(16) << 16 if r.random() < 0.4 else 0), r.getrandbits(32)]


def g_13999(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x300)
    rec = newbuf(ctx, 0x40)
    block = newbuf(ctx, 0x600)
    put(ctx, obj, 'I', r.choice([8, 5, 5, 8, 0, 1, 2, 3, ctx.u32()]))
    put(ctx, obj + 0x54, 'I', rec)
    put(ctx, obj + 0x28, 'I', block)
    put(ctx, obj + 0x1a6, 'h', r.choice([0, 0, 1, -1, 0x100, r.getrandbits(16)]))
    return [obj, r.getrandbits(32)]


def g_16d83(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1015ec58, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x1015eab4, DPSEND)
    w32(ctx, 0x1015ecbc, newbuf(ctx, 16))
    w32(ctx, 0x1015ecd4, ctx.u32())
    put(ctx, 0x1013acac, 'H', r.getrandbits(16))
    words = ctx.buf(8, struct.pack('<hHHH', r.choice([0, 7, 7, 7, 1, -1, 3, 0x7fff, -32768]), r.getrandbits(16), r.getrandbits(16), 0))
    return [r.choice([0, 1, 5, 7, 0x20, 0xff, ctx.u32()]), 0 if r.random() < 0.15 else words]


def g_27df4(ctx):
    begin(ctx)
    r = ctx.rng
    table = zbuf(ctx, 0x3f1 * 4 + 16)
    for _ in range(r.choice([0, 3, 8, 20])):
        head = 0
        for _ in range(r.randrange(1, 4)):
            node = zbuf(ctx, 0x20)
            put(ctx, node, 'H', r.choice([0, 0x100, 0xefff, 0xf000, 0xf000, 0xf001, 0xffff, r.getrandbits(16)]))
            put(ctx, node + 8, 'I', head)
            head = node
        w32(ctx, table + 4 * r.randrange(0, 0x3f1), head)
    w32(ctx, 0x1015dc30, 0 if r.random() < 0.1 else table)
    return []


def g_28f38(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.randrange(1, 9)
    nodes = [zbuf(ctx, 0x40) for _ in range(n)]
    for i in range(n):
        for off in (4, 8):
            if i + 1 < n and r.random() < 0.55:
                put(ctx, nodes[i] + off, 'I', nodes[r.randrange(i + 1, n)])
    return [0 if r.random() < 0.1 else nodes[0], ctx.u32(), ctx.u32()]


def g_none(ctx):
    begin(ctx)
    w32(ctx, 0x1014ceb4, ctx.u32())
    return []


def g_d349(ctx):
    begin(ctx)
    r = ctx.rng
    mesh, polys = mesh_world(ctx)
    obj = ctx.buf(0x100)
    w32(ctx, obj + 0x30, mesh)
    w32(ctx, obj + 0x2c, mesh if r.random() < 0.7 else ctx.u32())
    w32(ctx, obj + 0x98, 0 if r.random() < 0.5 else r.choice([mesh, ctx.u32()]))
    return [obj] + [fbits(small(r)) for _ in range(3)]


def g_cb21(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1015ebb0, r.choice([1, 1, 1, 0, 2]))
    idx = r.choice([0, 1, 2, 3, 5, 7, 11, -1, 100, r.randrange(0, 20)])
    w32(ctx, 0x1013abb8, idx if r.random() < 0.2 else r.randrange(0, 20))
    w32(ctx, 0x109b9e08, r.choice([0, 1, 1, 2]))
    w32(ctx, 0x1015eb98, r.choice([0, 1, 2, 2, 3]))
    ctx.m.uc.mem_write(0x1013ab90, rb(ctx, 0x100))
    return [idx & 0xffffffff]


def g_47e1a(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x40)
    period = r.choice([r.uniform(0.5, 400.0), 360.0, 360.0, 1.0, 100.0, 6.283185, 2.0, -360.0, 0.0])
    wf(ctx, obj + 0x10, period)
    x = r.choice([r.uniform(-2000, 2000), r.uniform(-400, 400), 0.0, 180.0, -180.0, 270.0, -270.0, 90.0, 360.0, 540.0, period / 2, -period / 2, period, 3 * period / 2, -3 * period / 2, 1e-3, 1e5])
    return [obj, fbits(x)]


def g_4e3d7(ctx):
    begin(ctx)
    r = ctx.rng
    for a in (0x1015f35c, 0x1015f358, 0x1015f354):
        w32(ctx, a, r.choice([0, 0, ctx.u32(), r.randrange(0x00a00000, 0x00a10000)]))
    rets(0x100fef30, r.choice([0, 0x00a20000]), r.choice([0, 0x00a30000]), r.choice([0, 0x00a40000]))
    return []


def g_4e441(ctx):
    begin(ctx)
    r = ctx.rng
    for a in (0x1015f35c, 0x1015f358, 0x1015f354):
        w32(ctx, a, r.choice([0, 0, ctx.u32()]))
    ok = r.random() < 0.4
    seq = [0x00a20000 + 0x1000 * i if (ok or r.random() < 0.7) else 0 for i in range(3)]
    rets(0x100fef30, *seq)
    return []


def g_strtab(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0, 1, 3, 6, 12])
    w32(ctx, 0x1015f3ac, n)
    names = []
    for i in range(12):
        nm = bytes(r.choice(b'abcAB ') for _ in range(r.randrange(0, 8)))
        names.append(nm)
        ctx.m.uc.mem_write(0x109b68e0 + i * 0x74, nm + b'\0' + rb(ctx, 0x74 - len(nm) - 1))
    ctx.m.uc.mem_write(0x109b8420 - 0x10, rb(ctx, 0x20 + 17 * 0x74))
    pick = r.choice(names[:max(n, 1)]) if r.random() < 0.7 else bytes(r.choice(b'abcAB ') for _ in range(r.randrange(0, 8)))
    if r.random() < 0.5:
        pick = pick.swapcase()
    idx = r.choice([0, 1, 2, 5, 15, 16, 17, -1, -2, r.randrange(-3, 20)])
    return [idx & 0xffffffff, cstr(ctx, pick)]


def g_53010(ctx):
    begin(ctx)
    r = ctx.rng
    ST['alloc'] = zbuf(ctx, 0x20) if r.random() < 0.7 else 0
    w32(ctx, 0x1015f3b0, r.choice([0, 0, ctx.u32()]))
    w32(ctx, 0x109b6880, zbuf(ctx, 0x20))
    text = bytes(r.choice(b'abcXYZ019') for _ in range(r.randrange(0, 12)))
    data = struct.pack('<H', r.getrandbits(16)) + (text + bytes(16))[:8] + rb(ctx, 4)
    if r.random() < 0.3:
        data = struct.pack('<H', r.getrandbits(16)) + rb(ctx, 12)
    return [ctx.buf(16, data)]


def g_5f9aa(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x101668f4, r.choice([0, 1, 1, ctx.u32()]))
    w32(ctx, 0x101668f8, r.choice([0, 0x00a50000, ctx.u32(), 3]))
    return [ctx.u32() if r.random() < 0.3 else ctx.buf(16)]


def g_61d67(ctx):
    begin(ctx)
    r = ctx.rng
    tree = obj_tree(ctx, r.randrange(1, 6), 'list')
    rec = zbuf(ctx, 0x100)
    put(ctx, rec + 0x48, 'I', tree[0])
    obj = newbuf(ctx, 0x500)
    put(ctx, obj, 'I', rec)
    put(ctx, obj + 0x48c, 'I', r.choice([0, 1, ctx.u32()]))
    put(ctx, obj + 0x490, 'I', r.choice([0, 1, ctx.u32()]))
    # the effect table 0x109eeb00 + 0x30 * index (index 0..0x7f): +4 is an object or 0
    ctx.m.uc.mem_write(0x109eeb00, bytes(0x30 * 0x80))
    for k in range(0x80):
        if r.random() < 0.35:
            o = newbuf(ctx, 0x100)
            put(ctx, o + 0x8c, 'I', newbuf(ctx, 0x100) if r.random() < 0.6 else 0)
            put(ctx, 0x109eeb00 + 0x30 * k + 4, 'I', o)
    rets(0x10028b90, r.choice([-1, 0, 1, 5, 0x7f, 3]), r.choice([-1, 0, 2, 6, 0x7f]), r.choice([-1, 0, 7, 9, 0x40]))
    return [obj]


def g_62a3a(ctx):
    begin(ctx)
    r = ctx.rng
    for slot in (0x109e893c, 0x109e8940):
        o = zbuf(ctx, 0x100)
        rect = ctx.buf(32, struct.pack('<5i', 0, *[r.choice([0, 1, 100, -100, 639, 479, r.randrange(-2000, 2000)]) for _ in range(4)]))
        put(ctx, o + 0xec, 'I', rect)
        w32(ctx, slot, o)
    for a in (0x10144ec8, 0x10144ed0):
        ctx.m.uc.mem_write(a, struct.pack('<2i', *[r.choice([0, 0x10000, 0x8000, -0x10000, r.randrange(-0x20000, 0x20000)]) for _ in range(2)]))
    w32(ctx, 0x109e3e1c, r.choice([0, 1, 640, 320, -50, r.randrange(-100000, 100000), ctx.u32()]))
    w32(ctx, 0x10144f4c, ctx.u32())
    w32(ctx, 0x10144f38, ctx.u32())
    w32(ctx, 0x1017694c, ctx.u32())
    return []


def angle_int(r):
    return r.choice([0, 0, 1, -1, 45, -45, 90, -90, 180, 360, 359, 720, 1000, -1000, r.randrange(-720, 721), r.randrange(-720, 721), r.randrange(-100000, 100000)])


def g_631ef(ctx):
    begin(ctx)
    r = ctx.rng
    ints = [0, 1, -1, 2, -2, 100, -100, 12345, -12345, 0x7fffffff, -0x80000000, 1000000, r.randrange(-1000, 1000), r.randrange(-100000, 100000)]
    return [ctx.buf(8), ctx.buf(8), r.choice(ints) & 0xffffffff, r.choice(ints) & 0xffffffff, angle_int(r) & 0xffffffff]


def g_68bb2(ctx):
    begin(ctx)
    r = ctx.rng
    angle = r.choice([0.0, 1e-8, -1e-8, 1e-7, 0.5, -0.5, 45.0, 90.0, -90.0, 180.0, 360.0, r.uniform(-720, 720), r.uniform(-720, 720), r.uniform(-30, 30), 1e5])
    outs = [ctx.buf(8) if r.random() > 0.08 else 0 for _ in range(3)]
    return [r.getrandbits(32), rfbits(r), rfbits(r), fbits(angle)] + outs


def g_6c9a0(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x600)
    ident = r.randrange(0, 5)
    w32(ctx, 0x1017b8c8, ident)
    p = newbuf(ctx, 0x20)
    put(ctx, p + 4, 'I', ident if r.random() < 0.7 else r.randrange(0, 5))
    put(ctx, obj, 'I', p)
    put(ctx, obj + 0x4f0, 'I', r.choice([0, 1, 2, 3, 4, 5, 6, ctx.u32()]))
    w32(ctx, 0x1015ebbc, r.choice([0, 0, 1, ctx.u32()]))
    return [obj, r.choice([0, 1, 1, 2, ctx.u32()])]


def g_83c45(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x10179ca0, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x10179ca4, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x1017b92c, r.choice([0, 0, 1]))
    for a in (0x1017b3c0, 0x1015e9a0, 0x109e3d14):
        w32(ctx, a, ctx.u32())
    w32(ctx, 0x109e3d18, r.choice([0, 1, 200, 480, 0x7fffffff, ctx.u32()]))
    w32(ctx, 0x103729c4, r.choice([0, 1, 200, 480, ctx.u32()]))
    ST['sdib'] = r.choice([0, 1, 2, 2])
    rets(0x10083801)
    ctx.m.uc.mem_write(0x1017b92c, struct.pack('<I', r.choice([0, 0, 1])))
    return []


def api_sdib(n, args):
    if ST['sdib'] == 0:
        return 0
    if ST['sdib'] == 1:
        return args[8] if isinstance(args[8], int) else 1
    return (n * 7919 + 13) & 0xff


def g_8404b(ctx):
    begin(ctx)
    r = ctx.rng
    rgb = newbuf(ctx, 0x300)
    w32(ctx, 0x1013b98c, r.randrange(0, 4))
    w32(ctx, 0x1015e698, ctx.buf(16))
    w32(ctx, 0x1015e9a0, ctx.u32())
    w32(ctx, 0x1017b3c4, r.choice([0, 0, 0x1234]))
    w32(ctx, 0x10179ca0, r.choice([0, 0, 1]))
    w32(ctx, 0x10179ca4, 0)
    w32(ctx, 0x1017b92c, r.choice([0, 1]))
    w32(ctx, 0x109e3d18, r.choice([0, 480]))
    w32(ctx, 0x103729c4, r.choice([0, 480]))
    ST['sdib'] = r.choice([0, 1, 2])
    return [rgb]


def g_878bb(ctx):
    begin(ctx)
    r = ctx.rng
    start = r.choice([0, 1, 5, 100, 250, 255, 256, -1, -3, r.randrange(-3, 300)])
    count = r.choice([0, 1, 2, 7, 16, 40, -1, -5, r.randrange(0, 41)])
    base = newbuf(ctx, 0x1400)
    return [start & 0xffffffff, count & 0xffffffff, base + 0x40]


def g_8792c(ctx):
    begin(ctx)
    return [newbuf(ctx, 0x300)]


def g_8e687(ctx):
    begin(ctx)
    r = ctx.rng
    left = r.choice([0, 1, -1, 5, -100, 100, r.randrange(-1000, 1000)])
    top = r.choice([0, 1, -1, 7, -100, 100, r.randrange(-1000, 1000)])
    width = r.choice([0, 1, 2, 3, -1, -2, -3, 10, 100, 639, 1000, 0x7fffffff, r.randrange(-5, 2000)])
    height = r.choice([0, 1, 2, 3, -1, 10, 100, 479, r.randrange(-5, 2000)])
    rect = ctx.buf(0x20, struct.pack('<5i', 0, left, top, s32(left + width), s32(top + height)) + rb(ctx, 12))
    blk = zbuf(ctx, 0x100)
    wf(ctx, blk + 0x44, r.choice([0.0, 1.0, 0.5, 0.25, -0.5, 1.5, 3.7, 1e3, r.uniform(-10, 10)]))
    w32(ctx, 0x1014cae0, blk)
    return [rect, ctx.u32()]


def g_9ff33(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0x200, 0x200, 0x200, 2, 3, 4, 5, 8, 16, 0])
    length = r.choice([0, 1, 2, 5, 8, 15, 40])
    chars = bytes(r.choice(b'abcXYZ019 _.') for _ in range(length))
    if r.random() < 0.6:
        chars += b'\n' + bytes(r.choice(b'abc') for _ in range(r.randrange(0, 4)))
    ST['fread'] = chars
    buf = newbuf(ctx, 0x800)
    return [r.getrandbits(32), buf, n]


def g_a78fd(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0, 1, 2, 0x3fff, 0x4000, 0x4001, 0x8000, 0x8001, 0xc000, 0x10000, r.randrange(0, 0x14000)])
    if r.random() < 0.3:
        ST['write_fail'] = r.randrange(0, 4)
        ST['write_fail_value'] = r.choice([0, 0xffffffff, 5, 0x4000, 0x3fff])
    return [r.choice([0, 1, 2, 3, 7, ctx.u32()]), r.choice([0x00a00000, ctx.u32(), 0x1000]), n]


def g_a8068(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x600)
    count = r.randrange(0, 40)
    put(ctx, obj + 0x4b4, 'I', count)
    w32(ctx, 0x1017d158, obj)
    low = r.choice([0, 1, 0x1234, 0xefff, 0xf000, 0xf000, 0xf001, 0xf000 + count - 1 if count else 0xf002, 0xf000 + count, 0xffff, 0xfffe, r.getrandbits(16), 0xf000 + r.randrange(0, 40)])
    return [ctx.u32(), ctx.u32(), (r.getrandbits(16) << 16 | low) if r.random() < 0.5 else low]


def g_a9c25(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x200)
    put(ctx, obj + 0x94, 'I', r.choice([0, 0x4000, 1, 0x4001, ctx.u32()]))
    return [obj]


def g_a9dc3(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x200)
    src = obj + 0x30 if r.random() < 0.1 else newbuf(ctx, 0x80)
    put(ctx, obj + 0x94, 'I', ctx.u32())
    return [obj, src]


def g_aa39f(ctx):
    begin(ctx)
    tree = obj_tree(ctx, ctx.rng.randrange(1, 8), 'list')
    return [tree[0]]


def g_aa592(ctx):
    begin(ctx)
    r = ctx.rng
    tree = obj_tree(ctx, r.randrange(1, 8), 'state')
    return [tree[0], r.choice([0, 1, 2, 4, 4, 5, -1, ctx.u32()])]


def g_aab03(ctx):
    begin(ctx)
    r = ctx.rng
    tree = obj_tree(ctx, r.randrange(1, 8), 'free')
    w32(ctx, 0x1017b8f8, ctx.u32())
    return [tree[0], 0 if r.random() < 0.25 else CALLBACK]


def g_aab8e(ctx):
    begin(ctx)
    r = ctx.rng
    tree = obj_tree(ctx, r.randrange(1, 8), 'names')
    name = r.choice([b'abc', b'ABC', b'abd', b'x', b'', b'ABD', b'zzz', b'aBc '])
    return [tree[0], cstr(ctx, name)]


def g_b77b3(ctx):
    begin(ctx)
    r = ctx.rng
    if r.random() < 0.1:
        return [0]
    n = r.choice([0, 1, 2, 3, 5, 8])
    entries = newbuf(ctx, max(n, 1) * 8)
    block = ctx.buf(8, struct.pack('<ii', n if r.random() > 0.05 else -r.randrange(1, 4), entries if r.random() > 0.15 else 0))
    return [block]


def obj_world(ctx):
    """An object for the angle/mode functions around 0x100c0558 (see Sim_10015266, Sim_100c48fa, Sim_1000ae00)."""
    r = ctx.rng
    obj = newbuf(ctx, 0x700)
    block = newbuf(ctx, 0x600)
    put(ctx, obj + 0x28, 'I', block)
    put(ctx, block + 0x50c, 'I', r.choice([0, 1, 2]))
    ident = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, ident)
    put(ctx, obj + 4, 'I', ident if r.random() < 0.6 else r.randrange(0, 4))
    for off in (0x68, 0x110, 0xe8):
        wf(ctx, obj + off, rfl(r, 200.0, 0.2))
    rec = newbuf(ctx, 0x100)
    put(ctx, obj + 0x54, 'I', rec if r.random() < 0.9 else 0)
    put(ctx, obj + 0x1f4, 'I', r.choice([0, 0, 1]))
    word = r.choice([0x200 | ident, 0x200 | r.randrange(0, 8), r.getrandbits(16)])
    put(ctx, obj + 0x1aa, 'H', word)
    put(ctx, obj + 0x218, 'I', r.choice([0, 1, 2, 3, ctx.u32()]))
    w32(ctx, 0x101654f2, ctx.u32())
    rets(0x1000b070, *[r.choice([0, 1, r.getrandbits(32)]) for _ in range(3)])
    rets(0x1000b587, *[r.choice([0, 5, r.getrandbits(32)]) for _ in range(3)])
    return obj


def g_c0558(ctx):
    begin(ctx)
    r = ctx.rng
    obj = obj_world(ctx)
    clock = r.choice([0, 100, 5000, ctx.u32()])
    w32(ctx, 0x1015dc78, clock)
    put(ctx, obj + 0x204, 'I', r.choice([clock, clock - 1, clock + 1, 0, 1, ctx.u32()]) & 0xffffffff)
    return [obj]


def g_c0d40(ctx):
    begin(ctx)
    r = ctx.rng
    obj = obj_world(ctx)
    base = r.uniform(-3000, 3000)
    wf(ctx, obj + 0xe0, base)
    wf(ctx, obj + 0x5c, r.choice([base + 2000.0, base + 2000.0 + 0.5, base + 1999.5, base + 2000.0 - 1e-3, rfl(r, 3000.0), base]))
    return [obj, r.choice([0, 1, -1, 0x7fff, -0x8000, r.getrandbits(32)])]


def g_c3892(ctx):
    begin(ctx)
    r = ctx.rng
    obj = newbuf(ctx, 0x40)
    block = newbuf(ctx, 0x600)
    put(ctx, obj + 0x28, 'I', block)
    put(ctx, block + 0x510, 'I', r.choice([0, 5, 6, 6, 7, 9, -1, ctx.u32()]))
    wf(ctx, block + 0x540, r.choice([0.0, 1e-7, -1e-7, 1e-8, 5e-8, 1.0, -1.0, 3.0, -0.5, r.uniform(-100, 100)]))
    value = r.choice([0.0, 0.5, -0.5, 1.5, -1.5, 99.5, -99.5, 1e5, -1e5, 3e9, -3e9, 1e10, 4.5e9, 2147483648.0, -2147483648.0, 2147483647.5, r.uniform(-200, 200), r.uniform(-1000, 1000)])
    wf(ctx, block + 0x4e8, value)
    limit = r.choice([int(value), int(value) + 1, int(value) - 1, 0, 1, -1, 100, -100, ctx.u32() & 0xffffffff]) if abs(value) < 2 ** 31 else r.choice([0, 1, -1, ctx.u32() & 0xffffffff, 0x7fffffff, 0x80000000])
    return [obj, limit & 0xffffffff]


def g_ccaa0(ctx):
    begin(ctx)
    r = ctx.rng
    return [r.choice([0, 1, 59, 60, 61, 3599, 3600, 3601, 86399, 86400, 359999, 360000, -1, -60, -3600, -3601, 0x7fffffff, 0x80000000, r.randrange(0, 100000), r.randrange(-100000, 0), ctx.u32()]) & 0xffffffff]


def g_d3491(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1017e65c, r.choice([0, 1, 1, 2, 3]))
    w32(ctx, 0x10152428, ctx.u32())
    w32(ctx, 0x10152420, ctx.u32())
    w32(ctx, 0x1015242c, ctx.u32())
    w32(ctx, 0x1017e658, r.choice([0, 0, 0x1234, ctx.u32()]))
    w32(ctx, 0x1017e650, ctx.u32())
    w32(ctx, 0x1017e660, r.choice([0, 1, 1]))
    # the table that holds the driver function for the block calls
    drv = zbuf(ctx, 0x40)
    put(ctx, drv + 0x18, 'I', DRIVER)
    w32(ctx, 0x1015e688, drv)
    # record at 0x1017e620 (Sim_1008c803): dst, delta, accum blocks, steps, current step, first, count
    steps = r.choice([1, 2, 3, 5, 100])
    cur = r.choice([0, 0, 1, steps - 1, steps, steps + 1])
    count = r.randrange(0, 40)
    dst = newbuf(ctx, 0x300)
    delta = newbuf(ctx, 0x300)
    accum = newbuf(ctx, 0x600)
    rec = struct.pack('<IIIII', dst, delta, accum, steps, cur) + bytes([r.randrange(0, 20)]) + struct.pack('<I', count) + rb(ctx, 3)
    ctx.m.uc.mem_write(0x1017e620, rec)
    # block at 0x1017e640 (Sim_1008c5e0): +4 colour block, +8 first, +9 count
    ctx.m.uc.mem_write(0x1017e640, struct.pack('<II', 0, newbuf(ctx, 0x300)) + bytes([r.randrange(0, 20)]) + struct.pack('<I', r.randrange(0, 30)) + rb(ctx, 3))
    # the block is freed through the heap when the counter reaches the end
    w32(ctx, 0x1017b8f8, ctx.u32())
    return []


def g_e2a31(ctx):
    begin(ctx)
    r = ctx.rng
    cnt = ctx.buf(4, struct.pack('<i', r.choice([0, 1, -1, 5, 100, r.randrange(-10, 10)])))
    limit = r.choice([0, 1, 2, 6, 101, -1, r.randrange(-10, 10)])
    ST['e2_result'] = r.choice([0, 0, newbuf(ctx, 0x20)])
    ST['e2_ret'] = r.choice([0, 0, 0, 1, 5])
    w32(ctx, 0x1017e838, ctx.u32())
    w32(ctx, 0x1017e850, ctx.u32())
    w32(ctx, 0x1017e858, ctx.u32())
    return [ctx.u32(), cnt, limit & 0xffffffff, ctx.u32()]


def g_e3ba4(ctx):
    a = g_e2a31(ctx)
    return a + [ctx.rng.choice([0, ctx.u32(), ctx.buf(16)])]


def g_e3c0d(ctx):
    begin(ctx)
    r = ctx.rng
    cnt = ctx.buf(4, struct.pack('<i', r.choice([0, 1, -1, 5, 100, r.randrange(-3, 12)])))
    ST['e2_result'] = r.choice([0, 0, newbuf(ctx, 0x20)])
    ST['e2_ret'] = r.choice([0, 0, 0, 1, 5])
    w32(ctx, 0x1017e838, ctx.u32())
    w32(ctx, 0x1017e850, ctx.u32())
    w32(ctx, 0x1017e858, ctx.u32())
    return [ctx.u32(), cnt]


def g_spr(ctx):
    begin(ctx)
    return []


# ---- mesh builders (as in spec_w3q.py) -------------------------------------------------------------
def small(r):
    """Coordinates from a small grid so that equalities between vertices and the query happen often."""
    return float(r.choice([-2, -1, 0, 1, 2, 3])) if r.random() < 0.8 else r.uniform(-3, 3)


def mesh_world(ctx, npolys=None):
    """A mesh buffer: short +6 = polygon count, dword +8 = offset of the 0x28-byte records, vertices at +0x18."""
    r = ctx.rng
    nv = r.choice([3, 4, 6, 9])
    npoly = npolys if npolys is not None else r.choice([1, 2, 3, 4, 5])
    mesh = ctx.buf(0x18 + nv * 0x58 + npoly * 0x28 + 64)
    voff = 0x18
    for i in range(nv):
        wf(ctx, mesh + voff + i * 0x58 + 0xc, small(r), small(r), small(r))
    polys = []
    roff = voff + nv * 0x58
    ctx.m.uc.mem_write(mesh + 6, struct.pack('<h', npoly))
    w32(ctx, mesh + 8, roff)
    lists = mesh + roff + npoly * 0x28
    for p in range(npoly):
        rec = mesh + roff + p * 0x28
        n = r.choice([3, 3, 4, 4, 5, 6])
        idx = [r.randrange(0, nv) for _ in range(n)]
        lo = lists + p * 8
        ctx.m.uc.mem_write(lo, bytes(idx[:8]) + b'\0')
        ctx.m.uc.mem_write(rec + 2, struct.pack('<H', n))
        w32(ctx, rec + 4, lo - rec)
        wf(ctx, rec + 0x18, rfl(r, 2.0, 0.2), r.choice([-1.0, 1.0, 0.5, -0.5, 0.0, 1e-3, rfl(r, 2.0, 0.1)]), rfl(r, 2.0, 0.2))
        polys.append(rec)
    return mesh, polys


SPECS = [
    dict(name='Sim_10008123', orig=0x10008123, c='Sim_10008123', gen=g_mono, trials=300, void=True),
    dict(name='Sim_100082be', orig=0x100082be, c='Sim_100082be', gen=g_mono, trials=300, void=True),
    dict(name='Sim_1000ae00', orig=0x1000ae00, c='Sim_1000ae00', gen=g_ae00, trials=500),
    dict(name='Sim_1000c7b4', orig=0x1000c7b4, c='Sim_1000c7b4', gen=g_c7b4, trials=300),
    dict(name='Sim_100104fd', orig=0x100104fd, c='Sim_100104fd', gen=g_mode_set, trials=500, void=True),
    dict(name='Sim_10010632', orig=0x10010632, c='Sim_10010632', gen=g_mode_set, trials=500, void=True),
    dict(name='Sim_10010767', orig=0x10010767, c='Sim_10010767', gen=g_mode_set, trials=500, void=True),
    dict(name='Sim_10010951', orig=0x10010951, c='Sim_10010951', gen=g_10951, trials=600, void=True),
    dict(name='Sim_1001393e', orig=0x1001393e, c='Sim_1001393e', gen=g_1393e, trials=600),
    dict(name='Sim_10013999', orig=0x10013999, c='Sim_10013999', gen=g_13999, trials=400, void=True),
    dict(name='Sim_10016d83', orig=0x10016d83, c='Sim_10016d83', gen=g_16d83, trials=500, void=True),
    dict(name='Sim_10027df4', orig=0x10027df4, c='Sim_10027df4', gen=g_27df4, trials=300, void=True),
    dict(name='Sim_10028f38', orig=0x10028f38, c='Sim_10028f38', gen=g_28f38, trials=400, void=True),
    dict(name='Sim_1002b600', orig=0x1002b600, c='Sim_1002b600', gen=g_none, trials=100, void=True),
    dict(name='Sim_1002d349', orig=0x1002d349, c='Sim_1002d349', gen=g_d349, trials=600),
    dict(name='Sim_1003cb21', orig=0x1003cb21, c='Sim_1003cb21', gen=g_cb21, trials=500, void=True),
    dict(name='Sim_10047e1a', orig=TRAMP, c='Test_10047e1a', gen=g_47e1a, trials=600, fp=True),
    dict(name='Sim_1004e3d7', orig=0x1004e3d7, c='Sim_1004e3d7', gen=g_4e3d7, trials=300, void=True),
    dict(name='Sim_1004e441', orig=0x1004e441, c='Sim_1004e441', gen=g_4e441, trials=400),
    dict(name='Sim_1005259e', orig=0x1005259e, c='Sim_1005259e', gen=g_strtab, trials=600),
    dict(name='Sim_10053010', orig=0x10053010, c='Sim_10053010', gen=g_53010, trials=400,
         api={'HeapAlloc': lambda n, a: ST['alloc']}),
    dict(name='Sim_1005f9aa', orig=0x1005f9aa, c='Sim_1005f9aa', gen=g_5f9aa, trials=300),
    dict(name='Sim_10061d67', orig=0x10061d67, c='Sim_10061d67', gen=g_61d67, trials=500, void=True),
    dict(name='Sim_10062a3a', orig=0x10062a3a, c='Sim_10062a3a', gen=g_62a3a, trials=400, void=True, fp=True),
    dict(name='Sim_100631ef', orig=0x100631ef, c='Sim_100631ef', gen=g_631ef, trials=600, void=True, fp=True),
    dict(name='Sim_10068bb2', orig=0x10068bb2, c='Sim_10068bb2', gen=g_68bb2, trials=600, void=True, fp=True),
    dict(name='Sim_1006c9a0', orig=0x1006c9a0, c='Sim_1006c9a0', gen=g_6c9a0, trials=400, void=True),
    # superseded: the loading screen draws through Vulkan (DECISIONS.md, "The loading screen through Vulkan"; checked natively against GDI by tools/verify/picture_native.c)
    # dict(name='Sim_10083c45', orig=0x10083c45, c='Sim_10083c45', gen=g_83c45, trials=500,
    #      api={'SetDIBitsToDevice': api_sdib, 'GetLastError': lambda n, a: 0x7e + n}),
    # superseded: the loading screen draws through Vulkan (DECISIONS.md, "The loading screen through Vulkan"; checked natively against GDI by tools/verify/picture_native.c)
    # dict(name='Sim_1008404b', orig=0x1008404b, c='Sim_1008404b', gen=g_8404b, trials=300,
    #      api={'SetDIBitsToDevice': api_sdib, 'GetLastError': lambda n, a: 0x7e + n}),
    dict(name='Sim_100878bb', orig=0x100878bb, c='Sim_100878bb', gen=g_878bb, trials=400),
    dict(name='Sim_1008792c', orig=0x1008792c, c='Sim_1008792c', gen=g_8792c, trials=300),
    dict(name='Sim_1008e687', orig=0x1008e687, c='Sim_1008e687', gen=g_8e687, trials=600, fp=True),
    dict(name='Sim_1009ff33', orig=0x1009ff33, c='Sim_1009ff33', gen=g_9ff33, trials=600),
    dict(name='Sim_100a78fd', orig=0x100a78fd, c='Sim_100a78fd', gen=g_a78fd, trials=500),
    dict(name='Sim_100a8068', orig=0x100a8068, c='Sim_100a8068', gen=g_a8068, trials=500),
    dict(name='Sim_100a9c25', orig=0x100a9c25, c='Sim_100a9c25', gen=g_a9c25, trials=400, void=True),
    dict(name='Sim_100a9dc3', orig=0x100a9dc3, c='Sim_100a9dc3', gen=g_a9dc3, trials=300, void=True),
    dict(name='Sim_100aa39f', orig=0x100aa39f, c='Sim_100aa39f', gen=g_aa39f, trials=400, void=True),
    dict(name='Sim_100aa592', orig=0x100aa592, c='Sim_100aa592', gen=g_aa592, trials=400, void=True),
    dict(name='Sim_100aab03', orig=0x100aab03, c='Sim_100aab03', gen=g_aab03, trials=400, void=True),
    dict(name='Sim_100aab8e', orig=0x100aab8e, c='Sim_100aab8e', gen=g_aab8e, trials=500),
    dict(name='Sim_100b77b3', orig=0x100b77b3, c='Sim_100b77b3', gen=g_b77b3, trials=300, void=True),
    dict(name='Sim_100c0558', orig=0x100c0558, c='Sim_100c0558', gen=g_c0558, trials=500, void=True, fp=True),
    dict(name='Sim_100c0d40', orig=0x100c0d40, c='Sim_100c0d40', gen=g_c0d40, trials=500, fp=True),
    dict(name='Sim_100c3892', orig=0x100c3892, c='Sim_100c3892', gen=g_c3892, trials=600, fp=True),
    dict(name='Sim_100ccaa0', orig=0x100ccaa0, c='Sim_100ccaa0', gen=g_ccaa0, trials=300),
    dict(name='Sim_100d3491', orig=0x100d3491, c='Sim_100d3491', gen=g_d3491, trials=500, void=True),
    dict(name='Sim_100e2a31', orig=0x100e2a31, c='Sim_100e2a31', gen=g_e2a31, trials=400),
    dict(name='Sim_100e3ba4', orig=0x100e3ba4, c='Sim_100e3ba4', gen=g_e3ba4, trials=400),
    dict(name='Sim_100e3c0d', orig=0x100e3c0d, c='Sim_100e3c0d', gen=g_e3c0d, trials=400),
]
