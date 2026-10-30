# Differential tests for round 4, letter w: functions that call Windows through the import slots (see difftest.py).
#
# The checker stubs every import; it logs name and arguments of each call and compares the logs of the original and
# the C version. The handlers below answer the calls. They depend only on the call number, the arguments and memory, so
# the original and the C run see the same answers. Handlers that fill an out struct (cursor position, message,
# volume) or that need to see what a pointer argument pointed to (the WM_COPYDATA block, the text of a message box,
# the library name) read the raw arguments from the stack of the stub (the logged copy shows stack addresses only
# as 'stack'). What they read is folded into the return value, or into a log buffer in argument memory, so a wrong
# string or struct changes memory or the result.
#
# Function pointers kept in the DLL's data (the network library exports, the display driver method) point at
# the import stubs of Windows functions with the right argument count, so the call shows up in the same log.
import json
import struct
from unicorn.x86_const import UC_X86_REG_ESP

SOURCES = json.loads(r'''
["sim/unsorted_1000/w2e_mono.c", "sim/unsorted_1000/w4w_blocks.c", "sim/unsorted_1001/w2h_objects.c",
 "sim/unsorted_1002/w2j_nodes.c", "sim/unsorted_1003/records.c", "sim/unsorted_1003/w2e_mono.c",
 "sim/unsorted_1003/w4w_records.c", "sim/unsorted_1004/tables.c", "sim/unsorted_1004/w2h_record.c",
 "sim/unsorted_1004/w2i_misc.c", "sim/unsorted_1004/w3p_net.c", "sim/unsorted_1004/w4w_net.c",
 "sim/unsorted_1005/w2h_tables.c", "sim/unsorted_1007/w2f_misc.c", "sim/unsorted_1007/w3p_net.c",
 "sim/unsorted_1007/w4w_display.c", "sim/unsorted_1008/w2i_rect.c", "sim/unsorted_1008/w4w_display.c",
 "sim/unsorted_100b/w4w_buffers.c", "sim/unsorted_100d/w2i_select.c", "sim/unsorted_100d/w2j_nodes.c",
 "sim/unsorted_100e/w4w_table.c", "sim/unsorted_100f/w2e_string.c", "sim/util/field.c", "sim/win/w4a_heap.c",
 "sim/win/w4w_win.c"]
''')
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

ARENA = 0x00A80000      # blocks that the HeapAlloc stub hands out (0x800 bytes apart, call number based)
LOG = 0x00AFE000        # handler log in argument memory (compared with all other memory)
HEAPVAR = 0x1017b8f8
ST = {}


TOUCHED = set()


def put(ctx, addr, data):
    """Writes into the emulator. Pages of the DLL's data that a generator writes are remembered: the checker only
    restores the pages that the C run wrote, so a page that only the generator (and the original) wrote would keep
    its content for the next trial and make the original start from different data than the C version."""
    if 0x10000000 <= addr < 0x11000000:
        TOUCHED.update(range(addr >> 12, (addr + len(data) - 1 >> 12) + 1))
    ctx.m.uc.mem_write(addr, data)


def w32(ctx, addr, *vals):
    put(ctx, addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def rbytes(uc, addr, n):
    return bytes(uc.mem_read(addr, n))


def raw(uc, k):
    """Raw argument k (0 based) of the Windows call that the stub is answering."""
    return r32(uc, uc.reg_read(UC_X86_REG_ESP) + 4 + 4 * k)


def hsh(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def bhash(data):
    x = 0x811c9dc5
    for b in data:
        x = ((x ^ b) * 0x01000193) & 0xffffffff
    return x


def logw(uc, v):
    n = r32(uc, LOG)
    if n < 200:
        w32_uc(uc, LOG + 4 + 4 * n, v)
        w32_uc(uc, LOG, n + 1)


def w32_uc(uc, addr, *vals):
    uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def cstr(uc, addr, limit=64):
    out = b''
    while len(out) < limit:
        c = rbytes(uc, addr + len(out), 1)
        if c == b'\0':
            break
        out += c
    return out


def common(ctx, heap=True):
    """State that every function reads: the heap handle, and garbage in the blocks the HeapAlloc stub hands out."""
    rg = ctx.rng
    m = ctx.m
    for pg in TOUCHED:
        if pg in m.pagemap:
            m.raw.mem_write(pg << 12, m.page_snap(pg))
    TOUCHED.clear()
    w32(ctx, HEAPVAR, rg.choice([0, 0x00c10000, rg.getrandbits(32)]))
    ctx.m.uc.mem_write(ARENA, bytes(rg.getrandbits(8) for _ in range(0x1800)))
    ST['fail'] = set()
    return rg


def fails(rg, ncalls, p=0.15):
    """A set of HeapAlloc call numbers (api call index) that return 0."""
    if rg.random() < p:
        return {rg.randrange(0, ncalls)}
    return set()


def h_alloc(n, a):
    return 0 if n in ST.get('fail', ()) else ARENA + 0x800 * (n % 100)


def iat(ctx, slot):
    """The stub address that the checker put into an import slot."""
    return r32(ctx.m.uc, slot)


FLOATS = [0x00000000, 0x80000000, 0x3f800000, 0x40000000, 0x3fc00000, 0xbf800000, 0x40400000, 0x7fc00000,
          0x7f800000, 0x00000001, 0xff800000, 0x41200000]


def g_none(ctx):
    common(ctx)
    return []


# ---- 0x10008cbc, 0x10008f1c --------------------------------------------------------------------------
def g_8cbc(ctx, owner_given=True):
    rg = common(ctx)
    ST['fail'] = fails(rg, 1, 0.1)
    owner = ctx.buf(0x40, 'zero')
    if rg.random() < 0.05:
        owner = 0
    else:
        nodes = []
        for _ in range(rg.choice([0, 0, 1, 2, 3])):
            nodes.append(ctx.buf(0x30, 'zero'))
        for i, nd in enumerate(nodes):
            w32(ctx, nd, rg.choice(FLOATS))
            w32(ctx, nd + 0xc, nodes[i + 1] if i + 1 < len(nodes) else 0)
        w32(ctx, owner + 0x2c, nodes[0] if nodes else 0)
        w32(ctx, owner + 0x30, rg.getrandbits(32))
    end = ctx.buf(8)
    return [owner, rg.choice(FLOATS), rg.choice([0, 1, 2, 3, 5]), rg.choice([0, 0, 1, 2, 4]),
            rg.choice([0, 1, 7, 0x20, 0x41]), end]


def g_8f1c(ctx):
    rg = common(ctx)
    ST['fail'] = fails(rg, 2, 0.25)
    w32(ctx, 0x1015d09c, rg.getrandbits(32))
    return [rg.choice([0, 1, 2, 4]), rg.choice([0, 1, 3]), rg.choice([0, 5, 0x20]), ctx.buf(8)]


# ---- 0x1003184b and the records at 0x109e3760 ---------------------------------------------------------
def g_1848(ctx):
    rg = common(ctx)
    ST['fail'] = fails(rg, 1, 0.15)
    return [ctx.u32() for _ in range(6)] + [rg.choice([-3, 0, 0, 1, 2, 3, 4, 7, 12])]


def g_38ccd(ctx):
    rg = common(ctx)
    w32(ctx, 0x109e3760, rg.getrandbits(32))
    w32(ctx, 0x109e3770, rg.getrandbits(32))
    return []


def g_39202(ctx):
    rg = common(ctx)
    n = rg.choice([0, 0, 1, 2, 3, 5])
    nodes = [ctx.buf(0x698, 'zero') for _ in range(n)]
    for i, nd in enumerate(nodes):
        w32(ctx, nd + 0x690, nodes[i + 1] if i + 1 < n else 0)
    w32(ctx, 0x109e3760, nodes[0] if nodes else 0)
    return []


def g_3c313(ctx):
    rg = common(ctx)
    ST['fl'] = rg.choice([0, 1, 1, 2, 3, 5])
    for a in range(0x1015ea8c, 0x1015eb70, 4):
        w32(ctx, a, rg.getrandbits(32))
    return []


def g_3edda(ctx):
    rg = common(ctx)
    w32(ctx, 0x1015eccc, rg.choice([0, 1, 1, 2, 3]))
    w32(ctx, 0x1015ec58, 0)
    for a in (0x1015ec88, 0x1015ecc4, 0x1015ec90, 0x1015ec94, 0x1015d084, 0x1015ecc8, 0x1015d008):
        w32(ctx, a, 0 if rg.random() < 0.3 else 0x00b00000 + rg.randrange(0x1000))
    for a in (0x1015ece8, 0x1015ecd0, 0x1015dc84):
        w32(ctx, a, rg.getrandbits(32))
    return []



# ---- network library loader (0x10045d70 .. 0x10045ec2) ---------------------------------------------------
def h_getmodule(n, a, uc):
    name = cstr(uc, raw(uc, 0))
    return ST['gmh'] if name else 0xdead


def h_loadlib(n, a, uc):
    name = cstr(uc, raw(uc, 0))
    return ST['ll'] if name else 0xbeef


def h_getproc(n, a, uc):
    # 0 or a stub address chosen by the name pointer, so the order of the three lookups is checked too
    which = {0x1013b050: 0, 0x1013b060: 1, 0x1013b070: 2}.get(raw(uc, 1))
    if which is None:
        return 0x7777
    return ST['gpa'][which]


def h_dbg(n, a, uc):
    logw(uc, bhash(cstr(uc, raw(uc, 0))))
    return 0


def lib_state(ctx, rg):
    ST['gmh'] = rg.choice([0, 0, 0x10, 0x20, 0x21, 0x60000000, 0x5000])
    ST['ll'] = rg.choice([0, 0x10, 0x1f, 0x20, 0x21, 0x70000000])
    stub2 = iat(ctx, 0x10ac5794)         # SetCursorPos, 2 stdcall arguments
    stub1 = iat(ctx, 0x10ac5790)         # ClipCursor, 1 stdcall argument
    stub0 = iat(ctx, 0x10ac5778)         # GetFocus
    ST['gpa'] = [rg.choice([0, stub2, stub2]), rg.choice([0, stub0, stub0]), rg.choice([0, stub1, stub1])]


def g_5d70(ctx):
    rg = common(ctx)
    lib_state(ctx, rg)
    w32(ctx, 0x1015ef40, rg.choice([0, iat(ctx, 0x10ac5794)]))
    return [ctx.u32(), ctx.u32()]


API_LIB = {'GetModuleHandleA': h_getmodule, 'LoadLibraryA': h_loadlib, 'GetProcAddress': h_getproc,
           'OutputDebugStringA': h_dbg,
           'SetCursorPos': lambda n, a: hsh(n, a[0] if a[0] != 'stack' else 1, a[1]) & 0x7fffffff,
           'ClipCursor': lambda n, a: ST['clip'][n % len(ST['clip'])]}


def g_5db9(ctx):
    rg = common(ctx)
    w32(ctx, 0x1015ef3c, rg.choice([0, 1, 1, 7]))
    w32(ctx, 0x1015ef48, rg.choice([0, iat(ctx, 0x10ac5790), iat(ctx, 0x10ac5790)]))
    ST['clip'] = [rg.choice([0, 1, 0x7fffffff, 0xffffffff, 0x80004005, 0x8007000e, 0xfffffffe])]
    # the format arguments of wsprintfA are the first stack words beyond the three real ones: they are compared too
    return []


def g_5e5d(ctx):
    rg = common(ctx)
    lib_state(ctx, rg)
    for a in (0x1015ef40, 0x1015ef44, 0x1015ef48):
        w32(ctx, a, rg.getrandbits(32))
    return [rg.choice([0, 1, 0x20, 0x21, rg.getrandbits(32)])]


def g_5ec2(ctx):
    rg = common(ctx)
    lib_state(ctx, rg)
    for a in (0x1015ef40, 0x1015ef44, 0x1015ef48):
        w32(ctx, a, rg.getrandbits(32))
    return []


# ---- the table of seven records at 0x109b94e0 ---------------------------------------------------------
def bucket(key, name4):
    total = key + sum(b - 256 if b >= 128 else b for b in name4)
    total = sx(total & 0xffffffff)
    r = abs(total) % 0x3f1
    return -r if total < 0 else r


def g_4821c(ctx, whole=False):
    rg = common(ctx)
    zero = ctx.buf(0x4000, 'zero')
    table = zero + 0x2000                       # the hash table of the callee (0x1015dc30), mostly empty
    w32(ctx, 0x1015dc30, table)
    name = bytes(rg.getrandbits(8) for _ in range(4))
    nameptr = ctx.buf(8)
    ctx.m.uc.mem_write(nameptr, name)
    w32(ctx, 0x1014cec0, nameptr)
    base = rg.randrange(0, 0x40)
    w32(ctx, 0x101c64f0, base)
    for a in (0x109f13f0, 0x109f13f4):
        w32(ctx, a, rg.choice([0, 0]))
    for i in range(8):
        if rg.random() < 0.7:
            rec = ctx.buf(0x40)
            flag = rg.choice([0, 1, 1, 5])
            w32(ctx, rec + 0x18, flag)
            off = rg.randrange(0, 0x20)
            w32(ctx, rec + 0x14, off)
            w32(ctx, 0x109b94e0 + 4 * i, rec)
            if flag and rg.random() < 0.8:      # a node that the callee finds and moves to its second list
                key = base + off
                node = ctx.buf(0x20, 'zero')
                b = bucket(key, name)
                head = struct.unpack('<I', ctx.m.uc.mem_read(table + 4 * b, 4))[0]
                w32(ctx, node, key & 0xffff)
                ctx.m.uc.mem_write(node + 2, struct.pack('<H', rg.choice([0, 1, 1, 0xffff])))
                w32(ctx, node + 4, struct.unpack('<I', name)[0])
                w32(ctx, node + 8, head)
                w32(ctx, table + 4 * b, node)
        else:
            w32(ctx, 0x109b94e0 + 4 * i, 0)
    if whole:
        return [rg.choice([-3, -1, 0, 1, 2, 3, 4, 5, 6, 7])]
    return [rg.choice([0, 1, 2, 3, 4, 5, 6, 7])]


def g_481dd(ctx):
    return g_4821c(ctx, True)


# ---- list of 0x18 byte entries ------------------------------------------------------------------------
def g_74b80(ctx):
    rg = common(ctx)
    ST['fail'] = fails(rg, 1, 0.12)
    tail = ctx.buf(0x20)
    w32(ctx, 0x1015dc14, rg.choice([0, tail]))
    w32(ctx, 0x1015dc10, tail)
    return [ctx.u32()]



# ---- cursor, mouse, message pump and other window functions -------------------------------------------------
def sdim(rg):
    return rg.choice([640, 800, 1024, 480, 1, 2, 3, 639, 0, -7, 321, 1279, rg.randrange(1, 2000)])


def window_state(ctx, rg, capture=None):
    w32(ctx, 0x1017b920, rg.choice([0, 1, 1, 1]) if capture is None else capture)
    w32(ctx, 0x1017b8f0, rg.choice([0, 0x300ac, rg.getrandbits(32)]))
    w32(ctx, 0x1017b918, sdim(rg))
    w32(ctx, 0x1017b91c, sdim(rg))
    w32(ctx, 0x10179d64, rg.choice([0, 1]))
    w32(ctx, 0x10179d68, rg.choice([0, 1]))
    w32(ctx, 0x1017b8b8, rg.choice([0, 0, 0, 1]))
    w32(ctx, 0x1017b93c, rg.choice([0, 1]))
    ST['seed'] = rg.getrandbits(32)
    ST['gcp'] = rg.choice([0, 1, 1, 1])
    ST['pz'] = rg.choice([0, 2, 5])


def sx(v):
    return v - (1 << 32) if v & 0x80000000 else v


def h_getcursorpos(n, a, uc):
    ptr = raw(uc, 0)
    w32_uc(uc, ptr, hsh(ST['seed'], n, 1) % 1500 - 100, hsh(ST['seed'], n, 2) % 1100 - 100)
    return ST['gcp']


def h_screentoclient(n, a, uc):
    ptr = raw(uc, 1)
    x, y = struct.unpack('<ii', rbytes(uc, ptr, 8))
    w, h = sx(r32(uc, 0x1017b918)), sx(r32(uc, 0x1017b91c))
    # land inside the client area most of the time, but also outside of it
    k = hsh(ST['seed'], n, 3) % 8
    if k < 4:
        x = hsh(ST['seed'], n, 4) % w if w > 0 else x - 17
        y = hsh(ST['seed'], n, 5) % h if h > 0 else y + 9
    elif k == 4:                                # on and next to the edges of the client area
        x, y = hsh(ST['seed'], n, 6) % 3 - 1, hsh(ST['seed'], n, 7) % 3 - 1
    elif k == 5:
        x, y = w - hsh(ST['seed'], n, 6) % 3, h - hsh(ST['seed'], n, 7) % 3
    elif k == 6:
        x, y = (w if hsh(n) & 1 else hsh(ST['seed'], n, 8) % max(w, 1)), (h if hsh(n) & 2 else hsh(ST['seed'], n, 9) % max(h, 1))
    else:
        x, y = x - 40, y - 300
    w32_uc(uc, ptr, x, y)
    return 1


def h_clienttoscreen(n, a, uc):
    ptr = raw(uc, 1)
    x, y = struct.unpack('<ii', rbytes(uc, ptr, 8))
    w32_uc(uc, ptr, x + 13, y - 9)
    return 1


API_MOUSE = {'GetCursorPos': h_getcursorpos, 'ScreenToClient': h_screentoclient, 'ClientToScreen': h_clienttoscreen,
             'GetAsyncKeyState': lambda n, a: hsh(n, a[0]) & (0xffff8000 if n % 3 else 0xffff0000 | (hsh(n) & 0x7fff)),
             'SetCursorPos': lambda n, a: 1}


def g_7d961(ctx):
    rg = common(ctx)
    window_state(ctx, rg)
    return [ctx.u32(), rg.choice([0, 1, 0, 1, 2, -1, 5])]


def g_7d9d8(ctx):
    rg = common(ctx)
    window_state(ctx, rg)
    pos = rg.choice([0, ctx.buf(8), ctx.buf(8)])
    buttons = ctx.buf(4)
    if rg.random() < 0.1 and not ST.get('keep'):
        buttons = 0
    return [ctx.u32(), pos, buttons]


def g_7dbb1(ctx):
    rg = common(ctx)
    window_state(ctx, rg)
    return [ctx.buf(16), rg.choice([0, 1, 640, rg.getrandbits(32)]), rg.choice([0, 1, 480, rg.getrandbits(32)])]


MSGS = [0x200, 0x201, 0x209, 0x20a, 0x12, 0x12, 0x100, 0x113, 0x1ff, 0xf, 0x0, 0x2, 0x113]


def h_peek(n, a, uc):
    r = hsh(ST['seed'], n, 9)
    if n > 12 or (r & 3) < ST['pz'] % 3:
        return 0
    ptr = raw(uc, 0)
    w32_uc(uc, ptr, 0 if r & 0x10 else 0x20000 + (r & 0xff), MSGS[(r >> 8) % len(MSGS)], r, r ^ 0xff, n, r >> 3, r >> 5)
    return 1


def h_msg_hash(n, a, uc):
    logw(uc, bhash(rbytes(uc, raw(uc, 0), 28)))
    return n + 3


API_PUMP = {'PeekMessageA': h_peek, 'TranslateMessage': h_msg_hash, 'DispatchMessageA': h_msg_hash}


def g_pump(ctx):
    rg = common(ctx)
    window_state(ctx, rg)
    return []


def g_errbox(ctx):
    rg = common(ctx)
    return [ctx.buf(16, 'text'), ctx.u32(), ctx.u32(), ctx.u32()]


def h_wvsprintf(n, a, uc):
    out, fmt, va = raw(uc, 0), raw(uc, 1), raw(uc, 2)
    sig = cstr(uc, fmt, 16) + rbytes(uc, va, 12)
    uc.mem_write(out, sig[:40] + b'\0')
    return len(sig)


def h_msgbox(n, a, uc):
    return bhash(cstr(uc, raw(uc, 1), 64)) & 0xffff


API_ERR = {'wvsprintfA': h_wvsprintf, 'MessageBoxA': h_msgbox, 'GetFocus': lambda n, a: hsh(n, 77),
           'MessageBeep': lambda n, a: 1}


def g_aux(ctx, vol=False):
    rg = common(ctx)
    w32(ctx, 0x1014c9b4, rg.choice([0xffffffff, 0, 1, 7, rg.getrandbits(32)]))
    w32(ctx, 0x101321bc, rg.choice([0, 0xffff, 0x10000, 0xffffffff, 0x8000, 0x7fffffff, 0x80000000, ctx.u32()]))
    ST['aux'] = rg.choice([0, 0, 0, 1, 11])
    if vol:
        return [rg.choice([0, 1, 0x8000, 0xfffe, 0xffff, 0x10000, 0x12345678, 0xffffffff, 0x80000000, ctx.u32()])]
    return []


def g_aux_set(ctx):
    return g_aux(ctx, True)


def h_auxget(n, a, uc):
    w32_uc(uc, raw(uc, 1), hsh(n, 31))
    return ST['aux']


API_AUX = {'auxGetVolume': h_auxget, 'auxSetVolume': lambda n, a: ST['aux']}


def g_time(ctx):
    common(ctx)
    return []


# ---- WM_COPYDATA senders and the drawing functions ------------------------------------------------------
def h_sendmsg(n, a, uc):
    data = rbytes(uc, raw(uc, 3), 12)
    return bhash(data) if ST['sm'] else 0


def h_stretch(n, a):
    return ST['sb']


def display_state(ctx, rg):
    w32(ctx, 0x109e3ce8, rg.choice([0, 0x4b000, 640 * 480, rg.getrandbits(32)]))
    w32(ctx, 0x1015e9a0, rg.choice([0, ctx.buf(16)]))
    w32(ctx, 0x1017b374, rg.choice([0, ctx.buf(16)]))
    w32(ctx, 0x1017b35c, rg.choice([0, 0x10042, rg.getrandbits(32)]))
    w32(ctx, 0x1017b364, rg.getrandbits(32))
    w32(ctx, 0x1017b368, rg.getrandbits(32))
    w32(ctx, 0x109e3d14, rg.choice([0, 640, rg.getrandbits(32)]))
    w32(ctx, 0x109e3d18, rg.choice([0, 480, 1, rg.getrandbits(32)]))
    w32(ctx, 0x1015e698, ctx.buf(16))
    w32(ctx, 0x10372d8c, rg.getrandbits(32))
    ST['sm'] = rg.choice([0, 1, 1])
    ST['sb'] = rg.choice([0, 1, 5])
    ST['sd'] = rg.choice(['zero', 'eq', 'other'])


def g_7ef27(ctx):
    rg = common(ctx)
    display_state(ctx, rg)
    return []


def g_7efb4(ctx):
    rg = common(ctx)
    display_state(ctx, rg)
    return [ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32()]


def g_7f0c8(ctx):
    rg = common(ctx)
    display_state(ctx, rg)
    w32(ctx, 0x1017b378, rg.choice([0, 1, 1, 1]))
    first = rg.choice([0, 1, 100, 255, 256, -1, rg.randrange(0, 256), rg.randrange(0, 256)])
    count = rg.choice([1, 2, 5, 256, 0, -1, 255, 257, rg.randrange(1, 257 - first if 0 <= first < 256 else 3)])
    rgb = 0 if rg.random() < 0.05 else ctx.buf(780)
    put(ctx, 0x1015e6a0, bytes(rg.getrandbits(8) for _ in range(768)))
    put(ctx, 0x109e38c0, bytes(rg.getrandbits(8) for _ in range(0x428)))
    return [first, count, rgb]


def g_83692(ctx):
    rg = common(ctx)
    display_state(ctx, rg)
    for a in (0x1017b3c0, 0x1017b3c4, 0x1017b7c8, 0x1017b7cc, 0x1017b7d0, 0x1017b7d4, 0x1017b7d8, 0x103729dc):
        w32(ctx, a, rg.choice([0, 0x1234, rg.getrandbits(32)]))
    return []


def h_setdib(n, a, uc):
    m = ST['sd']
    return 0 if m == 'zero' else (r32(uc, 0x109e3d18) if m == 'eq' else 0x1f)


def g_83d5b(ctx):
    rg = common(ctx)
    display_state(ctx, rg)
    w32(ctx, 0x1017b3c0, rg.getrandbits(32))
    w32(ctx, 0x103729c4, rg.getrandbits(32))
    return [ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32()]


def g_8c6b6(ctx):
    rg = common(ctx)
    table = ctx.buf(0x40)
    w32(ctx, table + 0x18, iat(ctx, 0x10ac5778))      # the GetFocus stub: plain ret, like a cdecl function
    w32(ctx, 0x1015e688, table)
    rec = ctx.buf(16)
    w32(ctx, rec + 4, rg.choice([0, 0x00b00123]))
    return [rec]


# ---- work buffers and the rest --------------------------------------------------------------------------
def g_35b0(ctx):
    rg = common(ctx)
    ST['fail'] = {rg.randrange(0, 12)} if rg.random() < 0.6 else set()
    for a in (0x1017d214, 0x1017d204, 0x1017d1f4, 0x1017d208, 0x1017d1f8):
        w32(ctx, a, rg.choice([0, 0x00b11111]))
    for a in (0x1017d20c, 0x1017d210, 0x101a1160, 0x101c42a4, 0x101c42a0, 0x101a1168):
        w32(ctx, a, rg.getrandbits(32))
    return []


def g_372b(ctx):
    return g_35b0(ctx)


def g_ba285(ctx):
    rg = common(ctx)
    ws = b' \t\r\n'
    body = bytes(rg.choice(b'abcXYZ019_.') for _ in range(rg.choice([0, 1, 2, 5, 9])))
    tail = bytes(rg.choice(ws) for _ in range(rg.choice([0, 0, 1, 2, 4])))
    head = bytes(rg.choice(ws) for _ in range(rg.choice([0, 0, 1, 3])))
    s = head + body + tail
    buf = ctx.buf(48, bytes(rg.getrandbits(8) | 1 for _ in range(48)))
    ctx.m.uc.mem_write(buf, s + b'\0')
    return [buf]


def g_bd1b6(ctx):
    rg = common(ctx)
    w32(ctx, 0x1017e1bc, rg.choice([0, 1, 2, 5, 9, 0xffffffff]))
    for i in range(12):
        w32(ctx, 0x1017dfd0 + 32 * i, rg.getrandbits(32))
    return []


def g_469b(ctx):
    rg = common(ctx)
    r = rg.random()
    if r < 0.5:
        idx = rg.randrange(0, 0x100)
    elif r < 0.8:
        idx = rg.randrange(0, 0x100) | (rg.getrandbits(8) << 8) & 0x1ff00 | rg.choice([0, 0x80])
    elif r < 0.9:
        idx = rg.choice([0x80, 0x7f, 0xff, 0x100, 0x180, 0x1ff, 0xffffff80, 0xffffffff, 0x10080])
    else:
        idx = ctx.u32()
    for base in (0x10153180, 0x10153280):
        put(ctx, base, bytes(rg.getrandbits(8) | 1 for _ in range(0x100)))
    return [idx]


API_FL = {'HeapAlloc': h_alloc,
          'FreeLibrary': lambda n, a: 1 if n < ST.get('fl', 0) else 0}

SPECS = [
    dict(name='Sim_10008cbc', orig=0x10008cbc, c='Sim_10008cbc', gen=g_8cbc, trials=800, api={'HeapAlloc': h_alloc}),
    dict(name='Sim_10008f1c', orig=0x10008f1c, c='Sim_10008f1c', gen=g_8f1c, trials=500, api={'HeapAlloc': h_alloc}),
    dict(name='Sim_1003184b', orig=0x1003184b, c='Sim_1003184b', gen=g_1848, trials=500, api={'HeapAlloc': h_alloc}),
    dict(name='Sim_10038ccd', orig=0x10038ccd, c='Sim_10038ccd', gen=g_38ccd, trials=300, void=True, api={'HeapAlloc': h_alloc}),
    dict(name='Sim_10039202', orig=0x10039202, c='Sim_10039202', gen=g_39202, trials=400, void=True),
    dict(name='Sim_10039241', orig=0x10039241, c='Sim_10039241', gen=g_38ccd, trials=300, void=True, api={'HeapAlloc': h_alloc}),
    dict(name='Sim_1003924b', orig=0x1003924b, c='Sim_1003924b', gen=g_39202, trials=400, void=True),
    dict(name='Sim_1003c313', orig=0x1003c313, c='Sim_1003c313', gen=g_3c313, trials=300, void=True, api=API_FL),
    dict(name='Sim_1003edda', orig=0x1003edda, c='Sim_1003edda', gen=g_3edda, trials=500, void=True),
    dict(name='Sim_10045d70', orig=0x10045d70, c='Sim_10045d70', gen=g_5d70, trials=600, api=API_LIB),
    dict(name='Sim_10045db9', orig=0x10045db9, c='Sim_10045db9', gen=g_5db9, trials=300, api=API_LIB),
    dict(name='Sim_10045e5d', orig=0x10045e5d, c='Sim_10045e5d', gen=g_5e5d, trials=400, api=API_LIB),
    dict(name='Sim_10045ec2', orig=0x10045ec2, c='Sim_10045ec2', gen=g_5ec2, trials=600, api=API_LIB),
    dict(name='Sim_100481dd', orig=0x100481dd, c='Sim_100481dd', gen=g_481dd, trials=400, void=True),
    dict(name='Sim_1004821c', orig=0x1004821c, c='Sim_1004821c', gen=g_4821c, trials=400, void=True),
    dict(name='Sim_10074b80', orig=0x10074b80, c='Sim_10074b80', gen=g_74b80, trials=400, api={'HeapAlloc': h_alloc}),
    # Win_JoyGetNumDevs (0x10056210): round 27 made it return 0; proven in spec_w27_joystick.py
    dict(name='Win_CenterCursorAxis', orig=0x1007d961, c='Win_CenterCursorAxis', gen=g_7d961, trials=600, api=API_MOUSE),
    dict(name='Win_PollMouse', orig=0x1007d9d8, c='Win_PollMouse', gen=g_7d9d8, trials=1000, api=API_MOUSE),
    dict(name='Win_ClientRectToScreen', orig=0x1007dbb1, c='Win_ClientRectToScreen', gen=g_7dbb1, trials=300, void=True, api=API_MOUSE),
    dict(name='Sim_1007ef27', orig=0x1007ef27, c='Sim_1007ef27', gen=g_7ef27, trials=400, api={'SendMessageA': h_sendmsg}),
    dict(name='Sim_1007efb4', orig=0x1007efb4, c='Sim_1007efb4', gen=g_7efb4, trials=500,
         api={'SendMessageA': h_sendmsg, 'StretchBlt': h_stretch, 'GetLastError': lambda n, a: hsh(n, 5)}),
    dict(name='Sim_1007f0c8', orig=0x1007f0c8, c='Sim_1007f0c8', gen=g_7f0c8, trials=600, api={'SendMessageA': h_sendmsg}),
    dict(name='Sim_10083692', orig=0x10083692, c='Sim_10083692', gen=g_83692, trials=400),
    dict(name='Sim_10083d5b', orig=0x10083d5b, c='Sim_10083d5b', gen=g_83d5b, trials=500,
         api={'SetDIBitsToDevice': h_setdib, 'GetSystemMetrics': lambda n, a: hsh(n, 3) & 0x3f,
              'GetLastError': lambda n, a: hsh(n, 5)}),
    dict(name='Win_PumpMessage', orig=0x10086b99, c='Win_PumpMessage', gen=g_pump, trials=800, void=True, api=API_PUMP),
    dict(name='Sim_1008c6b6', orig=0x1008c6b6, c='Sim_1008c6b6', gen=g_8c6b6, trials=300,
         api={'GetFocus': lambda n, a, uc: hsh(raw(uc, 0), 5)}),
    dict(name='Win_AuxGetVolume', orig=0x1009eb52, c='Win_AuxGetVolume', gen=g_aux, trials=300, api=API_AUX),
    dict(name='Win_AuxSetVolume', orig=0x1009eb85, c='Win_AuxSetVolume', gen=g_aux_set, trials=500, api=API_AUX),
    dict(name='Sim_1009ebeb', orig=0x1009ebeb, c='Sim_1009ebeb', gen=g_aux, trials=300, api=API_AUX),
    dict(name='Sim_100b35b0', orig=0x100b35b0, c='Sim_100b35b0', gen=g_35b0, trials=500, api={'HeapAlloc': h_alloc}),
    dict(name='Sim_100b372b', orig=0x100b372b, c='Sim_100b372b', gen=g_372b, trials=400, void=True),
    dict(name='Win_TimeGetTime', orig=0x100b8fdb, c='Win_TimeGetTime', gen=g_time, trials=300,
         api={'timeGetTime': lambda n, a: hsh(n, 1)}),
    dict(name='Sim_100ba285', orig=0x100ba285, c='Sim_100ba285', gen=g_ba285, trials=800, void=True),
    dict(name='Sim_100bd1b6', orig=0x100bd1b6, c='Sim_100bd1b6', gen=g_bd1b6, trials=300, void=True),
    dict(name='Win_ErrorBox', orig=0x100d9fc5, c='Win_ErrorBox', gen=g_errbox, trials=400, api=API_ERR),
    dict(name='Sim_100e469b', orig=0x100e469b, c='Sim_100e469b', gen=g_469b, trials=500, void=True),
]
