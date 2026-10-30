# Differential tests for round 6, letter h (see difftest.py and docs/checker.md).
#
# Helpers that are not converted yet run as original code in the emulator on both sides, except the ones listed
# in STUBS for the specs that ask for them: there an emulator code hook at the helper's address appends
# ('stub_<address>', arguments) to the checker's call log (compared between the original and the C run, like the
# Windows calls), answers with a value chosen by the generator and returns to the caller (EIP/ESP/EAX are set;
# no code bytes are patched). Stack addresses are logged as 'stack'.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_100c/w6h_calls.c', 'sim/unsorted_100d/w6h_calls.c',
    'sim/unsorted_100e/w6h_calls.c',
    # existing converted code the functions above call (and what that calls)
    'sim/unsorted_100f/w2k_string.c',
    'sim/unsorted_1000/w5b_calls.c', 'sim/unsorted_1001/w5c_math.c', 'sim/unsorted_1001/w3t_misc.c',
    'sim/unsorted_1003/w3s_entry.c', 'sim/unsorted_100c/w5c_math.c', 'sim/unsorted_100c/w5a_misc.c',
    'sim/unsorted_100c/w2l_objects.c', 'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_1003/w2e_mono.c',
    'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_100d/w2f_records.c', 'sim/unsorted_1003/entry.c',
    'sim/unsorted_1002/w2k_misc.c', 'sim/util/field.c', 'sim/unsorted_100c/w3o_misc.c', 'sim/unsorted_1004/tables.c',
    'sim/unsorted_1008/w3o_length.c',
    'sim/unsorted_1009/w2g_objects.c', 'sim/unsorted_100d/w2d_state.c', 'sim/unsorted_100a/w5x_calls.c',
    'sim/unsorted_1008/w4v_fade.c', 'sim/util/ring.c', 'sim/win/w4a_heap.c', 'sim/unsorted_100a/w5z_calls.c',
    'sim/unsorted_1006/w5y_misc.c', 'sim/unsorted_1005/w5b_calls.c', 'sim/unsorted_1005/w3q_misc.c',
    'sim/unsorted_100a/w3t_misc.c', 'sim/unsorted_100a/w5b_calls.c', 'sim/unsorted_100d/nodes.c',
    'sim/unsorted_100a/w5y_misc.c', 'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1008/cell_link.c',
    'sim/view/w2l_lists.c', 'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_1000/w3s_model.c', 'sim/view/w2d_view.c',
    'sim/unsorted_100a/w2k_list.c', 'sim/unsorted_1003/slots.c', 'sim/win/w4v_heap.c', 'sim/unsorted_1003/w2g_misc.c',
    'sim/unsorted_100d/w2g_misc.c', 'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_1008/w3s_grid.c',
    'sim/view/matrix.c', 'sim/unsorted_100d/w2j_nodes.c', 'sim/view/objlist.c', 'sim/view/viewrect.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000

# helper address -> number of dword arguments to log
STUBS = {
    0x100fd2b0: 4, 0x100fd820: 3,
    0x100c3550: 2, 0x10033eb3: 5, 0x100c6b30: 2, 0x100cab7b: 3, 0x10002964: 4, 0x10002513: 4,
    0x1000b070: 2, 0x1000b587: 2, 0x100c4d63: 1,
    0x100ba7d5: 3, 0x100bb2b0: 2, 0x100a9ba7: 1, 0x10060681: 4, 0x100a9e1a: 2, 0x100aa742: 1, 0x10088a2d: 1,
    0x100bc280: 2, 0x10078f60: 1,
}
CB = 0x00AF8000         # a callback handed to Sim_100c417e: a `ret` with a hook that logs its arguments
ST = {'active': set(), 'rets': {}}


# ---- helpers --------------------------------------------------------------------------------------
def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def cstr(ctx, data, extra=16):
    return ctx.buf(len(data) + 1 + extra, bytes(data) + b'\0' + rb(ctx, extra))


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


# ---- stubs ----------------------------------------------------------------------------------------
def _stub_cb(m):
    def cb(uc, address, size, ud):
        if address not in ST['active']:
            return
        esp = uc.reg_read(UC_X86_REG_ESP)
        k = STUBS[address]
        raw = struct.unpack('<%dI' % k, bytes(uc.mem_read(esp + 4, 4 * k))) if k else ()
        args = tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % address, args))
        eff = ST.get('effects', {}).get(address)
        if eff:
            eff(uc, raw)
        rets = ST['rets'].get(address) or [0]
        uc.reg_write(UC_X86_REG_EAX, rets[n % len(rets)] & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def _callback_cb(m):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        raw = struct.unpack('<4I', bytes(uc.mem_read(esp + 4, 16)))
        n = len(m.api_log)
        m.api_log.append(('callback', tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)))
        uc.reg_write(UC_X86_REG_EAX, mix(ST['seed'], n, raw[0], raw[1]))
    return cb


def begin(ctx, *stubs):
    """Installs the stub hooks (once per machine), activates the given stubs and chooses their answers."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w6h', False):
        m._w6h = True
        cb = _stub_cb(m)
        for addr in STUBS:
            m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
        m.raw.hook_add(UC_HOOK_CODE, _callback_cb(m), None, CB, CB)
    ST['active'] = set(stubs)
    ST['seed'] = r.getrandbits(32)
    ST['effects'] = {}
    ST['heap_fail'] = False
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)


# ---- C runtime (0x100f....) -----------------------------------------------------------------------
CTYPE_PTR = 0x10159450
MB_CUR_MAX = 0x1015965c


def ctype_world(ctx, table=True):
    """__mb_cur_max-like dword 1 or 2; the class table is the DLL's own or a random one (entry 0 without 4 and 8)."""
    r = ctx.rng
    w32(ctx, MB_CUR_MAX, r.choice([1, 1, 2, 2, 0, 3, -1]))
    if table and r.random() < 0.4:
        data = bytearray(rb(ctx, 0x204))
        data[2] &= ~0x0c & 0xff                          # entry for byte 0
        t = ctx.buf(len(data), bytes(data))
        w32(ctx, CTYPE_PTR, t + 2)


def g_fc640(ctx):
    begin(ctx)
    for i in range(6):
        w32(ctx, 0x10159700 + 4 * i, ctx.u32())
    return []


def g_strstr(ctx):
    begin(ctx)
    r = ctx.rng
    alpha = r.choice([b'ab', b'abc', b'aab\x80\xff'])
    hay = bytes(r.choice(alpha) for _ in range(r.choice([0, 1, 2, 5, 10, 20, r.randrange(0, 40)])))
    if hay and r.random() < 0.5:
        i = r.randrange(0, len(hay))
        needle = hay[i:i + r.choice([1, 2, 3, 4, 6])]
    else:
        needle = bytes(r.choice(alpha) for _ in range(r.choice([0, 1, 1, 2, 2, 3, 4, 5])))
    return [cstr(ctx, hay), cstr(ctx, needle)]


def gst_handler(n, args, uc):
    """Windows GetStringTypeA(locale, type, src, count, out): on success writes count words made from the source bytes."""
    esp = uc.reg_read(UC_X86_REG_ESP)
    raw = struct.unpack('<5I', bytes(uc.mem_read(esp + 4, 20)))
    ok = ST['gst'][n % len(ST['gst'])]
    if ok:
        cnt = raw[3] if raw[3] < 8 else 1
        src = bytes(uc.mem_read(raw[2], cnt))
        words = [mix(ST['seed'], *src, i) & 0xffff for i in range(cnt)]
        uc.mem_write(raw[4], struct.pack('<%dH' % cnt, *words))
    return ok


def g_isctype(ctx):
    begin(ctx)
    r = ctx.rng
    ctype_world(ctx)
    ST['seed'] = r.getrandbits(32)
    ST['gst'] = [r.choice([1, 1, 1, 0, 5]) for _ in range(3)]
    w32(ctx, 0x1017e93c, 2)          # GetStringTypeA works (the other way needs the heap)
    w32(ctx, 0x1017e950, ctx.u32())
    c = r.choice([-1, 0, 1, 0x20, 0x41, 0x7f, 0x80, 0xff, 0x100, 0x101, r.randrange(-1, 256), r.randrange(-1, 256),
                  r.randrange(0, 0x10000), r.randrange(0x8000, 0x10000), -2, -0x100, r.getrandbits(32)])
    mask = r.choice([0xffff, 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x8000, 0xffffffff, ctx.u32()])
    return [c & 0xffffffff, mask]


def g_atol(ctx):
    begin(ctx)
    r = ctx.rng
    ctype_world(ctx)
    lead = bytes(r.choice(b'  \t\n\r\x0b\x0c') for _ in range(r.choice([0, 0, 1, 2, 5])))
    sign = r.choice([b'', b'', b'-', b'+', b'--', b'-+', b' '])
    digits = bytes(r.choice(b'0123456789') for _ in range(r.choice([0, 1, 2, 5, 9, 10, 11, 15, 25])))
    tail = bytes(r.choice(b'0123456789ab \x80\xff.') for _ in range(r.choice([0, 0, 1, 3])))
    s = lead + sign + digits + tail
    if r.random() < 0.1:
        s = rb(ctx, r.randrange(0, 12)).replace(b'\0', b'1')
    return [cstr(ctx, s)]


def g_strtox(ctx):
    begin(ctx, 0x100fd2b0)
    r = ctx.rng
    s = cstr(ctx, b'123 abc')
    end = zbuf(ctx, 8) if r.random() < 0.8 else 0
    return [s, end, r.choice([0, 2, 8, 10, 16, 36, 37, -1, ctx.u32()]) & 0xffffffff]


def g_fd850(ctx):
    begin(ctx, 0x100fd820)
    return [cstr(ctx, b'name.dat'), cstr(ctx, ctx.rng.choice([b'rb', b'r', b'wb']))]


SCAN_CASES = [
    (b'%d', lambda r: str(r.randrange(-100000, 100000)).encode()),
    (b'%d %d', lambda r: b'%d %d' % (r.randrange(-999, 999), r.randrange(0, 99999))),
    (b'%s', lambda r: bytes(r.choice(b'abcXYZ') for _ in range(r.randrange(0, 20)))),
    (b'%x %s', lambda r: b'%x %s' % (r.getrandbits(24), b'word')),
    (b'%c%c', lambda r: bytes(r.choice(b'ab z') for _ in range(r.randrange(0, 4)))),
    (b'%3d%s', lambda r: str(r.randrange(0, 100000)).encode() + b'rest'),
    (b'%[a-c]%d', lambda r: bytes(r.choice(b'abcd') for _ in range(r.randrange(0, 6))) + b'12'),
    (b'%u,%i', lambda r: b'%d,%s' % (r.randrange(0, 1 << 31), r.choice([b'0x1f', b'017', b'-5', b'x']))),
    (b'%f', lambda r: r.choice([b'1.5', b'-2.25', b'3', b'1e3', b'0.1', b'abc', b''])),
    (b'%hd %ld', lambda r: b'%d %d' % (r.randrange(-70000, 70000), r.randrange(-5, 5))),
    (b'name %d', lambda r: r.choice([b'name 4', b'nam 4', b'name', b'name  -7'])),
]


def g_sscanf(ctx):
    begin(ctx)
    r = ctx.rng
    fmt, make = r.choice(SCAN_CASES)
    w32(ctx, MB_CUR_MAX, 1)
    # the floating-point conversion table as the program's start-up sets it (what Sim_100fc640 stores); without
    # it %f ends in the library's "floating point not loaded" error path
    w32(ctx, 0x10159700, 0x10100f30, 0x10100af0, 0x10100b80, 0x10100a90, 0x10100b60, 0x10100f30)
    s = make(r)
    outs = [ctx.buf(64, rb(ctx, 64)) for _ in range(4)]
    return [cstr(ctx, s), cstr(ctx, fmt)] + outs


# ---- 0x100c.... --------------------------------------------------------------------------------
def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def rfl(ctx, lo=-1000.0, hi=1000.0):
    """A float as a bit pattern: ordinary values, zeros, tiny and huge, both signs, now and then a NaN."""
    r = ctx.rng
    c = r.random()
    if c < 0.55:
        return fbits(r.uniform(lo, hi))
    if c < 0.75:
        return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, 10.0, -10.0, 180.0, -180.0, 360.0, 800.0, 1e-7, 2e-7, 5.0]))
    if c < 0.9:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-12, 12))
    if c < 0.93:
        return 0x7fc00000
    return fbits(r.uniform(-5, 5))


def float_buf(ctx, n, lo=-1000.0, hi=1000.0):
    return ctx.buf(n, b''.join(struct.pack('<I', rfl(ctx, lo, hi)) for _ in range(n // 4)))


def ptr_table(ctx, count, size=0x600):
    """The pointer table 0x109e8b40: count records of float-like contents (dword +8 small); returns them."""
    recs = []
    for i in range(count):
        rec = float_buf(ctx, size)
        w32(ctx, rec + 8, ctx.rng.randrange(0, 4))
        recs.append(rec)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
    return recs


def unit_world(ctx):
    """An object for Sim_100c17b7 and its callees (records at 0x1096fc80 for Sim_100c476c)."""
    r = ctx.rng
    ptr_table(ctx, 8, 0x100)
    for k in range(4):                                   # the owners Sim_10052be7 reads for the records' +8
        w32(ctx, 0x109b7d54 + k * 0x6c, k)
    obj = float_buf(ctx, 0x600, -400.0, 400.0)
    w32(ctx, obj + 4, r.randrange(0, 8))
    w32(ctx, 0x1017b8c8, r.choice([r32(ctx.m.uc, obj + 4), r.randrange(0, 8)]))
    blk = float_buf(ctx, 0x600)
    w32(ctx, blk + 0x50c, r.choice([0, 1, ctx.u32()]))
    w32(ctx, obj + 0x28, blk)
    w32(ctx, obj + 0x54, float_buf(ctx, 0x80))
    w32(ctx, obj + 0x1f4, r.choice([0, 0, 1]))
    w16(ctx, obj + 0x1aa, r.choice([0x200 | r32(ctx.m.uc, 0x1017b8c8), 0, r.getrandbits(16)]))
    for _ in range(r.choice([0, 0, 1, 2])):
        i = r.randrange(0, 0x200)
        rec = 0x1096fc80 + i * 0x156
        target = float_buf(ctx, 0x100, -400.0, 400.0)
        for k in (0x58, 0x5c, 0x60):                       # near the object
            x = struct.unpack('<f', struct.pack('<I', r32(ctx.m.uc, obj + k)))[0]
            if x == x:
                w32(ctx, target + 0x70 + k - 0x58, fbits(x + r.uniform(-30.0, 30.0)))
        wb(ctx, rec, rb(ctx, 0x156))
        w32(ctx, rec + 0x50, r.randrange(0, 8))
        w32(ctx, rec + 0x3c, target)
        for k in (0x18, 0x1c, 0x20):
            w32(ctx, rec + k, rfl(ctx, -1, 1))
    return obj


def g_c17b7(ctx):
    begin(ctx, 0x1000b070, 0x1000b587, 0x100c4d63)
    return [unit_world(ctx)]


def g_c25af(ctx):
    begin(ctx, 0x100c3550)
    r = ctx.rng
    obj = float_buf(ctx, 0x200, -2000.0, 2000.0)
    w32(ctx, obj + 0xf0, r.choice([0x200, 0x2ff, 0x1234, 0x100, r.getrandbits(32), 0x200 | r.getrandbits(8)]))
    blk = newbuf(ctx, 0x600)
    w32(ctx, blk + 0x510, r.choice([0, 1, -1, 5, 0x80000000, ctx.u32()]))
    w32(ctx, obj + 0x28, blk)
    ST['rets'][0x100c3550] = [r.choice([0, 0, 1, 0xffffffff])]
    return [obj]


def g_c2851(ctx):
    begin(ctx)
    r = ctx.rng
    count = r.choice([0, 1, 2, 3, 5, 8])
    recs = ptr_table(ctx, count)
    w32(ctx, 0x1015dc64, count if r.random() < 0.9 else r.choice([-1, 0]))
    skip = r.choice([0, 1, 2, 3, ctx.u32()])
    for rec in recs:
        w32(ctx, rec + 4, r.choice([skip, 0, 1, 2, 3]))
        for k in (0x58, 0x5c, 0x60):
            w32(ctx, rec + k, rfl(ctx, -50.0, 50.0))
        w32(ctx, rec + 0x246, rfl(ctx, 0.0, 30.0))
        blk = newbuf(ctx, 0x600)
        w32(ctx, blk + 0x53c, rfl(ctx, 0.0, 30.0))
        w32(ctx, rec + 0x28, blk)
        w32(ctx, rec + 0x48, newbuf(ctx, 0x100))
    block = float_buf(ctx, 0x100, -50.0, 50.0)
    for k in range(0x18, 0x24, 4):
        w32(ctx, block + k, rfl(ctx, -1.0, 1.0))
    w32(ctx, block + 0x30, rfl(ctx, 0.0, 60.0))
    return [block, zbuf(ctx, 8), skip, r.choice([0, 1, 7])]


def g_c3372(ctx):
    begin(ctx)
    obj = float_buf(ctx, 0x100, -400.0, 400.0)
    other = float_buf(ctx, 0x100)
    return [obj, other, rfl(ctx), rfl(ctx), rfl(ctx)]


def g_c3419(ctx):
    begin(ctx)
    obj = float_buf(ctx, 0x100, -400.0, 400.0)
    return [obj] + [rfl(ctx) for _ in range(6)]


def list_effect(lst):
    def eff(uc, raw):
        uc.mem_write(raw[4], struct.pack('<I', lst))
    return eff


def c417e_setup(ctx):
    """Answers of 0x10033eb3: a count (eax) and a list pointer written through its fifth argument."""
    r = ctx.rng
    n = r.choice([0, 1, 2, 3, 5, -1])
    lst = newbuf(ctx, 0x40) if r.random() < 0.85 else 0
    ST['rets'][0x10033eb3] = [n]
    ST['effects'][0x10033eb3] = list_effect(lst)
    count = r.choice([0, 1, 2, 3, 6])                     # the table of 0x44-byte entries Sim_10033d75 searches
    w32(ctx, 0x109e4cc0, count)
    for k in range(count + 2):
        w32(ctx, 0x109e4cc4 + 0x44 * k, *[rfl(ctx, -50.0, 50.0) for _ in range(0x11)])
        w32(ctx, 0x109e4cc4 + 0x44 * k + 0x10, r.choice([0, 0, 1, ctx.u32()]))   # in use when not 0


def g_c417e(ctx):
    begin(ctx, 0x10033eb3)
    r = ctx.rng
    c417e_setup(ctx)
    wb(ctx, CB, b'\xc3')
    rec = float_buf(ctx, 0x100) if r.random() < 0.93 else 0
    other = newbuf(ctx, 0x40) if r.random() < 0.93 else 0
    return [rec, other, ctx.u32(), CB]


def g_c43e1(ctx):
    begin(ctx, 0x10033eb3, 0x10002964, 0x10002513)
    r = ctx.rng
    c417e_setup(ctx)
    ptr_table(ctx, 4, 0x100)
    obj = float_buf(ctx, 0x200)
    w16(ctx, obj + 0x1aa, r.choice([0x200, 0x201, 0x202, 0x203, 0x2ff, 0, 0x1ff, (r.getrandbits(16) & 0xfc03) | 0x200,
                                    (r.getrandbits(16) & 0xfc03) | 0x200, r.getrandbits(16)]))
    return [obj, ctx.u32(), r.choice([1, 1, 1, 2, 2, 2, 0, 3, -1]) & 0xffffffff]


def g_tar(ctx):
    begin(ctx, 0x100c6b30)
    r = ctx.rng
    hdr = bytearray(rb(ctx, 0x200))
    magic = r.choice([b'ustar  \0', b'ustar  \0', b'ustar\x0000', b'ustar \0', b'\0', b'USTAR  \0'])
    hdr[0x101:0x101 + len(magic)] = magic
    return [ctx.buf(0x200, bytes(hdr)), newbuf(ctx, 0x40), newbuf(ctx, 8)]


def g_cb8ea(ctx):
    begin(ctx, 0x100cab7b)
    r = ctx.rng
    obj = newbuf(ctx, 0x200)
    w32(ctx, obj + 0x118, r.choice([0x100, 0x1ff, 0x105, 0x200, 0x1100, r.getrandbits(32), 0x100 | r.getrandbits(8)]))
    ST['rets'][0x100cab7b] = [r.choice([0, 1, -1, -2, 5, 0x80000000])]
    return [obj]


# ---- 0x100d.... / 0x100e.... ---------------------------------------------------------------------
ARENA = 0x00A80000      # blocks that the HeapAlloc stub hands out (0x1000 bytes each)


def heap_alloc(n, args):
    """Windows HeapAlloc: a fresh block in argument memory, or 0 when the trial asks for a failure."""
    return 0 if ST.get('heap_fail') else ARENA + 0x1000 * (n % 32)


HEAP_API = {'HeapAlloc': heap_alloc}


def g_d36f5(ctx):
    begin(ctx, 0x100ba7d5)
    r = ctx.rng
    ST['heap_fail'] = r.random() < 0.05
    w32(ctx, 0x1017e65c, r.choice([0, 0, 0, -1, 1, 5]))
    w32(ctx, 0x1017e660, r.choice([0, 0, 0, 0, 1]))
    w32(ctx, 0x10152420, r.choice([0, 1, 2, 7, r.randrange(0, 64)]))
    w32(ctx, 0x1014ced4, ctx.u32())
    wb(ctx, 0x1017e640, rb(ctx, 0x1c))                    # the record Sim_1008c550 fills (up to 0x1017e65c)
    ST['rets'][0x100ba7d5] = [newbuf(ctx, 0x400) if r.random() < 0.85 else 0]
    return [ctx.u32(), r.choice([0, 1, 5, 0x40, 0x80, 0x100, r.randrange(0, 0x100)])]


def g_d62bd(ctx):
    begin(ctx, 0x100ba7d5, 0x100bb2b0)
    r = ctx.rng
    for i in range(0x2e):
        e = 0x10145508 + i * 0xd6
        wb(ctx, e + 0xb2, r.choice([b'\0', b'\0', b'abc\0', b'\x80x\0', bytes([r.getrandbits(8)]) + b'\0']))
    for i in range(0x4d):
        w32(ctx, 0x10147b88 + i * 0x3c, r.choice([0, 0, 1, -1, 5, 0x80000000, ctx.u32()]))
    w32(ctx, 0x1014ceb4, ctx.u32())
    ST['rets'][0x100ba7d5] = [r.choice([0, 1, ctx.u32()]) for _ in range(5)]
    return []


def g_d6f92(ctx):
    begin(ctx, 0x100a9ba7, 0x10060681)
    r = ctx.rng
    vals = [rfl(ctx, -200.0, 200.0) for _ in range(3)]

    def eff(uc, raw):
        for k in range(3):
            uc.mem_write(raw[1 + k], struct.pack('<I', vals[k]))
    ST['effects'][0x10060681] = eff
    obj = newbuf(ctx, 0x100)
    w32(ctx, obj + 0x4c, float_buf(ctx, 0x100))
    s = float_buf(ctx, 0x600, -100.0, 100.0)
    kind = zbuf(ctx, 8)
    w32(ctx, kind, r.choice([1, 9, 0, 2, 9, 1, ctx.u32()]))
    w32(ctx, s, kind)
    w16(ctx, s + 0x562, r.choice([0, 4, 0x400, 0x404, 1, r.getrandbits(16)]))
    w32(ctx, obj + 0x28, s)
    return [obj, newbuf(ctx, 8), newbuf(ctx, 8), newbuf(ctx, 8)]


def g_d7111(ctx):
    begin(ctx, 0x100a9ba7, 0x100a9e1a)
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    w32(ctx, obj + 0x4c, float_buf(ctx, 0x100))
    s = newbuf(ctx, 0x600)
    w32(ctx, obj + 0x28, s)
    arr = newbuf(ctx, 0x128 * 4)
    w32(ctx, s + 0x84, arr)
    idx = r.randrange(0, 4)
    w32(ctx, s + 0x4fc, idx)
    e = arr + 0x128 * idx
    k = r.randrange(0, 8)
    w32(ctx, e + 0x11c, k)
    w32(ctx, e, r.choice([0, 1, -1, -1, ctx.u32()]))
    for j in range(8):              # never 0: Sim_100d709d would then leave the three values unset (the original reads them anyway)
        w32(ctx, s + 0x498 + 4 * j, float_buf(ctx, 0x100))
    return [obj, newbuf(ctx, 0x100)]


def pool_owner(ctx):
    """A slot pool for SlotPool_Take/Release: depth at +0x754, size at +0x750, free stack at +0x758."""
    r = ctx.rng
    o = zbuf(ctx, 0x900)
    w32(ctx, o + 0x750, r.choice([0x2d, 0x2d, 5, 0]))
    w32(ctx, o + 0x754, r.choice([0, 1, 3, 0x2c, 0x2d, 0x2e, -1, r.randrange(0, 0x2d)]))
    for i in range(0x2d):
        w32(ctx, o + 0x758 + 4 * i, r.randrange(0, 0x2d))
    return o


def mover(ctx):
    r = ctx.rng
    obj = float_buf(ctx, 0x600, -100.0, 100.0)
    node = float_buf(ctx, 0x100)
    w32(ctx, node, r.choice([0, 1]))
    w32(ctx, node + 4, 0)
    w32(ctx, node + 8, 0)
    w32(ctx, node + 0x8c, 0)
    w32(ctx, node + 0x74, r.choice([rfl(ctx), 0, 0x80000000, fbits(1e-7), fbits(2e-7), fbits(1e-8), fbits(-1.0)]))
    w32(ctx, obj + 0x552, node)
    rec = float_buf(ctx, 0x40, -10.0, 10.0)
    w32(ctx, rec, r.choice([0, 1, 5, 0x2c, 0x2d, -1]))
    w32(ctx, rec + 0x20, pool_owner(ctx) if r.random() < 0.9 else 0)
    w32(ctx, obj + 0x57a, rec)
    w32(ctx, 0x1015dc80, r.choice([1, 2, 30, -3, 0, ctx.u32()]))
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    return obj


def g_d8d0a(ctx):
    begin(ctx, 0x100aa742, 0x10088a2d)
    return [mover(ctx)]


def g_d9064(ctx):
    begin(ctx, 0x100aa742, 0x10088a2d)
    r = ctx.rng
    cam = float_buf(ctx, 0x80, -50.0, 50.0)
    w32(ctx, cam + 0x40, r.choice([fbits(400.0), fbits(400.0), fbits(400.0), rfl(ctx, 0.0, 200.0)]))
    w32(ctx, 0x1014cae0, cam)
    # the object pool of Sim_10090581
    top = r.choice([-1, 0, 1, 2, 3, 3, 2])
    w32(ctx, 0x10215ae8, top)
    w32(ctx, 0x1014b9b8, r.choice([1, 1, 1, 1, 1, 0]))
    for t in range(4):
        k = r.randrange(0, 6)
        w32(ctx, 0x10216b80 + 4 * t, k)
        o = 0x10217c20 + k * 0x57e
        wb(ctx, o, rb(ctx, 0x57e))
        node = float_buf(ctx, 0x100)
        w32(ctx, node + 4, 0)
        w32(ctx, node + 8, 0)
        w32(ctx, node, r.choice([0, 1]))
        rec = zbuf(ctx, 0x80)
        w16(ctx, rec + 2, r.choice([0x70, 0x50, 0x170]))
        w32(ctx, node + 0x8c, rec)
        w32(ctx, o + 0x552, node)
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    w32(ctx, 0x1015dc78, ctx.u32())
    args = [rfl(ctx, -60.0, 60.0) for _ in range(3)] + [rfl(ctx, -100.0, 100.0) for _ in range(3)]
    owner = pool_owner(ctx) if r.random() < 0.95 else 0
    if owner and r.random() < 0.6:
        w32(ctx, owner + 0x754, r.randrange(0, 0x2d))
    return args + [owner]


def g_da098(ctx):
    begin(ctx, 0x100bc280)
    r = ctx.rng
    ST['heap_fail'] = r.random() < 0.15
    w32(ctx, 0x1017e688, r.choice([0, 0x00a7f000, 0x12345678]))
    w32(ctx, 0x1017f118, ctx.u32())
    w32(ctx, 0x1017e690, ctx.u32())
    size = r.choice([8, 9, 20, 32, 0x44, r.randrange(8, 0x400), r.randrange(0, 8)])
    data = newbuf(ctx, max(size, 12))
    w32(ctx, data + 4, size)
    return [data]


def g_dac0a(ctx):
    begin(ctx, 0x100bc280, 0x10078f60)
    r = ctx.rng
    ST['heap_fail'] = r.random() < 0.15
    for i in range(16):
        w32(ctx, 0x1017ee40 + 4 * i, r.choice([0, 0, 0x00a7f000 + i]))
        w32(ctx, 0x1017ee80 + 4 * i, ctx.u32())
    size = r.choice([12, 20, 0x97 + 8, 0x200, r.randrange(12, 0x800), r.randrange(0, 12)])
    data = newbuf(ctx, max(size, 12))
    w32(ctx, data + 4, size)
    w32(ctx, data + 8, r.randrange(0, 16))
    return [data]


def g_db0ee(ctx):
    begin(ctx)
    r = ctx.rng
    n = r.choice([0, 1, 2, 3, 5, 8])
    data = newbuf(ctx, 8 + 0x18 * n + 0x20)
    w32(ctx, data + 4, 8 + 0x18 * n + r.choice([0, 0, 1, 0x17]))
    # the record table Sim_1005259e searches (0x74-byte records at 0x109b68e0, count [0x1015f3ac])
    names = [b'alpha', b'BETA', b'gamma', b'x']
    cnt = r.choice([0, 2, 4])
    w32(ctx, 0x1015f3ac, cnt)
    for i in range(cnt):
        wb(ctx, 0x109b68e0 + i * 0x74, names[i] + b'\0' + rb(ctx, 0x74 - len(names[i]) - 1))
    for i in range(n):
        e = data + 8 + i * 0x18
        w32(ctx, e, r.randrange(0, 0x40))
        w32(ctx, e + 4, ctx.u32())
        nm = r.choice(names + [b'none', b'ALPHA'])
        wb(ctx, e + 8, nm[:7] + b'\0')
    w32(ctx, 0x1013bb40, r.choice([0, 1, 2, -1, 9]))
    w32(ctx, 0x1017e6b0, cstr(ctx, r.choice(names)) if r.random() < 0.5 else 0)
    w32(ctx, 0x1017e6b4, cstr(ctx, r.choice(names + [b'zz'])) if r.random() < 0.4 else 0)
    return [data]


def g_e2776(ctx):
    begin(ctx)
    r = ctx.rng

    def v():
        return r.choice([rfl(ctx), rfl(ctx), fbits(1e-7), fbits(-1e-7), fbits(9.9e-8), fbits(1.0000001e-7), 0, 0x80000000,
                         0x7fc00000, 0xff800000, 0x7f800000, 0x00000001])
    return [v(), v(), v()]


SPECS = [
    dict(name='Sim_100fc640', orig=0x100fc640, c='Sim_100fc640', gen=g_fc640, trials=300, void=True),
    dict(name='Crt_StrStr', orig=0x100fc970, c='Crt_StrStr', gen=g_strstr, trials=1500),
    dict(name='Crt_IsCType', orig=0x100fd9a0, c='Crt_IsCType', gen=g_isctype, trials=1200,
         api={'GetStringTypeA': gst_handler}),
    dict(name='Crt_AToL', orig=0x100fd110, c='Crt_AToL', gen=g_atol, trials=1000),
    dict(name='Crt_AToI', orig=0x100fd1b0, c='Crt_AToI', gen=g_atol, trials=500),
    dict(name='Sim_100fd290', orig=0x100fd290, c='Sim_100fd290', gen=g_strtox, trials=300),
    dict(name='Sim_100fd540', orig=0x100fd540, c='Sim_100fd540', gen=g_strtox, trials=300),
    dict(name='Sim_100fd850', orig=0x100fd850, c='Sim_100fd850', gen=g_fd850, trials=300),
    dict(name='Sim_100fd710', orig=0x100fd710, c='Sim_100fd710', gen=g_sscanf, trials=800),
    dict(name='Sim_100c17b7', orig=0x100c17b7, c='Sim_100c17b7', gen=g_c17b7, trials=500, fp=True),
    dict(name='Sim_100c25af', orig=0x100c25af, c='Sim_100c25af', gen=g_c25af, trials=600, fp=True),
    dict(name='Sim_100c2851', orig=0x100c2851, c='Sim_100c2851', gen=g_c2851, trials=800, fp=True),
    dict(name='Sim_100c3372', orig=0x100c3372, c='Sim_100c3372', gen=g_c3372, trials=800, fp=True),
    dict(name='Sim_100c3419', orig=0x100c3419, c='Sim_100c3419', gen=g_c3419, trials=800, fp=True),
    dict(name='Sim_100c417e', orig=0x100c417e, c='Sim_100c417e', gen=g_c417e, trials=600),
    dict(name='Sim_100c43e1', orig=0x100c43e1, c='Sim_100c43e1', gen=g_c43e1, trials=600),
    dict(name='Tar_ReadHeader', orig=0x100c6c39, c='Tar_ReadHeader', gen=g_tar, trials=400, void=True),
    dict(name='Sim_100cb8ea', orig=0x100cb8ea, c='Sim_100cb8ea', gen=g_cb8ea, trials=500),
    dict(name='Sim_100d36f5', orig=0x100d36f5, c='Sim_100d36f5', gen=g_d36f5, trials=500, void=True, api=HEAP_API),
    dict(name='Sim_100d62bd', orig=0x100d62bd, c='Sim_100d62bd', gen=g_d62bd, trials=300, void=True),
    dict(name='Sim_100d6f92', orig=0x100d6f92, c='Sim_100d6f92', gen=g_d6f92, trials=800, void=True, fp=True),
    dict(name='Sim_100d7111', orig=0x100d7111, c='Sim_100d7111', gen=g_d7111, trials=500, void=True),
    dict(name='Sim_100d8d0a', orig=0x100d8d0a, c='Sim_100d8d0a', gen=g_d8d0a, trials=800, fp=True),
    dict(name='Sim_100d9306', orig=0x100d9306, c='Sim_100d9306', gen=g_d8d0a, trials=800, fp=True),
    dict(name='Sim_100d9064', orig=0x100d9064, c='Sim_100d9064', gen=g_d9064, trials=800, void=True, fp=True),
    dict(name='Sim_100da098', orig=0x100da098, c='Sim_100da098', gen=g_da098, trials=500, api=HEAP_API),
    dict(name='Sim_100dac0a', orig=0x100dac0a, c='Sim_100dac0a', gen=g_dac0a, trials=500, api=HEAP_API),
    dict(name='Sim_100db0ee', orig=0x100db0ee, c='Sim_100db0ee', gen=g_db0ee, trials=600, void=True),
    dict(name='Sim_100e2776', orig=0x100e2776, c='Sim_100e2776', gen=g_e2776, trials=800, void=True, fp=True),
]
