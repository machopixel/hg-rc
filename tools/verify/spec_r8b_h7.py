# Round 8B, helper h7: EXACT (no fp tolerance) differential tests of the float functions of round 6 letter e.
# Generators copied from spec_w6e.py; special float values (NaN, inf, signed zero, denormals) added.
#
# The functions call helpers of the DLL that are not converted yet (through their original address on both sides).
# Helpers that are too deep to run with generated data are replaced in the emulator by a stub: a code hook at the
# helper's address that appends ('stub_<address>', arguments) to the checker's call log (compared between the
# original and the C run, like the Windows calls), answers with a value that depends only on the trial and the
# position in the log, and returns to the caller (EIP/ESP are set; code bytes are not patched). Stack addresses are
# logged as 'stack'; for a few stubs the text an argument points to is logged as well. Some stubs also write results
# (an out pointer, a global) the way the helper would, with the same bytes for both runs. Indirect calls through data
# (a driver table, function pointers in globals, GetProcAddress results) go to small hooked stubs at IND + 0x10 * k.
# Pure helpers (C library fabs, _strcmpi, memmove, sqrt, the sine table, the vector length) run for real.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_1003/w6e_misc.c', 'sim/unsorted_1003/w6e_state.c', 'sim/unsorted_1003/w6e_sound.c',
    'sim/unsorted_1004/w6e_misc.c', 'sim/unsorted_1004/w6e_stream.c', 'sim/unsorted_1005/w6e_misc.c',
    'sim/unsorted_1006/w6e_misc.c', 'sim/unsorted_1007/w6e_misc.c', '../tools/verify/w6e_test.c',
    # existing converted code the functions above call (directly or further down)
    'sim/unsorted_1006/w5b_calls.c', 'sim/unsorted_1004/w5c_calls.c', 'sim/unsorted_1008/w3o_length.c',
    'sim/unsorted_1007/w5a_misc.c', 'sim/unsorted_100d/w5x_calls.c', 'sim/unsorted_1004/w3o_misc.c',
    'sim/util/field.c', 'sim/unsorted_1004/w3s_records.c', 'sim/unsorted_100a/w5a_misc.c', 'sim/unsorted_1004/state.c',
    'sim/unsorted_1005/w2k_misc.c', 'sim/unsorted_1004/w5z_calls.c', 'sim/unsorted_100d/w5z_calls.c',
    'sim/unsorted_100d/w5b_calls.c', 'sim/core/w2b_timers.c', 'sim/core/w2a_timers.c', 'sim/unsorted_1004/w2e_strings.c',
    'sim/util/ring.c', 'sim/unsorted_1005/w5a_misc.c', 'sim/unsorted_1003/setters.c', 'sim/unsorted_1004/tables.c',
    'sim/unsorted_1006/w5a_math.c', 'sim/unsorted_1002/w5y_misc.c', 'sim/unsorted_100a/w5y_misc.c',
    'sim/unsorted_1007/w2e_state.c', 'sim/unsorted_100a/w5x_calls.c', 'sim/unsorted_1004/w5a_misc.c',
    'sim/unsorted_1004/w2k_flags.c', 'sim/unsorted_1002/w5a_misc.c', 'sim/unsorted_100f/w2e_string.c',
    'sim/unsorted_1006/w5c_math.c', 'sim/unsorted_1004/w5y_misc.c', 'sim/unsorted_1000/w5x_calls.c',
    'sim/view/w3r_matrix.c', 'sim/unsorted_1002/w5z_calls.c', 'sim/unsorted_1003/w5a_misc.c',
    'sim/unsorted_1000/w2e_names.c', 'sim/view/viewrect.c', 'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_100d/nodes.c',
    'sim/core/timer.c', 'sim/unsorted_1002/w3s_cell.c', 'sim/unsorted_1004/w2j_misc.c', 'sim/unsorted_1002/w2l_tables.c',
    'sim/unsorted_1002/w2e_tables.c', 'sim/unsorted_1008/w4v_fade.c', 'sim/unsorted_1002/w4u_heap.c',
    'sim/unsorted_1000/w5a_misc.c', 'sim/view/matrix.c', 'sim/unsorted_100a/w5b_calls.c', 'sim/unsorted_1002/w2i_misc.c',
    'sim/unsorted_1004/fields.c', 'sim/unsorted_1008/w4w_display.c', 'sim/unsorted_100f/w2k_string.c',
    'sim/unsorted_1002/w2i_state.c', 'sim/unsorted_1002/w3o_vectors.c', 'sim/unsorted_100a/w2k_list.c',
    'sim/unsorted_1008/w3s_grid.c', 'sim/unsorted_1008/cell_link.c', 'sim/view/w2l_lists.c', 'sim/view/objlist.c',
    'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_1003/w2e_misc.c', 'sim/unsorted_100c/w2i_misc.c',
    'sim/win/w4v_heap.c', 'sim/unsorted_1001/w2e_names.c', 'sim/unsorted_1003/w2e_mono.c', 'sim/win/w4a_heap.c',
    'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1002/w2e_object.c', 'sim/view/w2d_view.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
IND = 0x00F00100        # indirect-call stubs (in the checker's return page, outside compared memory)
OUTF = 0x00AFF000       # float result of the st(0) function (argument memory, compared like all memory)
TRAMP = 0x00AF0000      # machine code for the original's st(0) wrapper
ARENA = 0x00A80000      # blocks the HeapAlloc stub hands out

# helper address -> number of argument dwords logged
STUBS = {
    0x10007890: 0, 0x100386d5: 3, 0x1002597b: 1, 0x100bc280: 2, 0x1004dd2a: 1, 0x100fc780: 2, 0x1005a91a: 3,
    0x1006abf6: 3, 0x100bb2b0: 2, 0x100fd9a0: 2, 0x1003a0e4: 2, 0x10075124: 1,
    # reached from converted callees through SIM_FN
    0x100d3510: 3, 0x100d3427: 1, 0x1001f790: 3, 0x1006a12d: 2, 0x100aa742: 1, 0x10088a2d: 1, 0x100a9ba7: 1,
    0x10060681: 4, 0x1000eb12: 7, 0x100246b3: 12, 0x1004409f: 1, 0x100ef727: 7, 0x100e86a1: 7, 0x1000e737: 12,
    0x100ff030: 2, 0x100febc0: 4, 0x100b7834: 2,
}
STUB_STR = {0x1000eb12: 3}      # stub -> argument whose text is logged too (it points into the caller's stack)
# stubs that are on unless a spec tests the function itself
DEFAULT_OFF = {0x10075124}
IND_ARGS = [0, 4, 3, 1, 3, 1, 0, 0]
ST = {}


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


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


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def cstr(ctx, data, extra=8):
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rb(ctx, extra))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


SPECIAL = [float('inf'), -float('inf'), float('nan'), -float('nan'), 0.0, -0.0, 1e-40, -1e-40, 1.4e-45, -1.4e-45,
           1.1754942e-38, 3.4e38, -3.4e38, 1e30, -1e30, 1e-30, 999.99994, 1000.0, 1000.0001, 140.0, 139.99998,
           140.00002, 1e-7, 1.0000001e-7, 9.999999e-8, 0.5, -0.5, 2.0 ** 24, 2.0 ** 31, -2.0 ** 31]


def special(ctx):
    return ctx.rng.choice(SPECIAL)


def rfl(ctx, lo=-1000.0, hi=1000.0):
    r = ctx.rng
    if r.random() < 0.12:
        return special(ctx)
    c = r.random()
    if c < 0.6:
        return r.uniform(lo, hi)
    if c < 0.8:
        return r.choice([0.0, -0.0, 1.0, -1.0, 0.5, 2.0, 100.0, 140.0, 1e-7, 1e-8, 1000.0])
    return r.uniform(-1, 1) * 10 ** r.randint(-6, 6)


# ---- stubs ---------------------------------------------------------------------------------------------------------
def read_text(uc, addr):
    text = b''
    while len(text) < 64:
        c = bytes(uc.mem_read(addr + len(text), 1))
        if c == b'\0':
            break
        text += c
    return text


def stub_ret(addr, n, raw):
    h = mix(ST.get('seed', 0), addr, n)
    if addr in (0x1004dd2a,):
        return 2 if h % 6 == 0 else 0
    if addr == 0x100ff030:                  # _flsbuf: the character or -1
        return 0xffffffff if h % 3 == 0 else h & 0xff
    if addr == 0x100febc0:                  # fwrite: all items, now and then fewer
        return ST.get('fwrite_count', 0xff) - (1 if h % 4 == 0 else 0)
    if addr == 0x100fd9a0:                  # _isctype(c, 8): white space; other values for the bytes >= 0x80
        c = raw[0] & 0xffffffff
        if c in (9, 10, 11, 12, 13, 32):
            return 8
        return 8 if c >= 0x80 and mix(ST.get('seed', 0), c) % 2 else 0     # the same answer for the same byte
    if addr == 0x100bb2b0:
        return h & 1
    return h


def stub_effect(uc, addr, raw, n):
    h = mix(ST.get('seed', 0), addr, n, 77)
    if addr == 0x1003a0e4:                  # SoundCreate stores the DirectSound object at its first argument
        uc.mem_write(raw[0], struct.pack('<I', 0 if h % 3 == 0 else 0x00c50000 + (h & 0xff0)))
    elif addr == 0x1005a91a:                # changes the global that the caller puts back
        uc.mem_write(0x101c42a0, struct.pack('<I', h))
    elif addr == 0x10060681:                # writes three floats to its out pointers
        for k in range(3):
            uc.mem_write(raw[1 + k], struct.pack('<f', ((mix(h, k) % 20001) - 10000) / 7.0))


def install(m):
    if getattr(m, '_w6e', False):
        return
    m._w6e = True

    def cb(uc, address, size, ud):
        if address not in ST['active']:
            return
        esp = uc.reg_read(UC_X86_REG_ESP)
        k = STUBS[address]
        raw = struct.unpack('<%dI' % k, bytes(uc.mem_read(esp + 4, 4 * k))) if k else ()
        args = tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)
        if address in STUB_STR:
            args += (read_text(uc, raw[STUB_STR[address]]),)
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % address, args))
        stub_effect(uc, address, raw, n)
        uc.reg_write(UC_X86_REG_EAX, stub_ret(address, n, raw) & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)

    for a in STUBS:
        m.raw.hook_add(UC_HOOK_CODE, cb, None, a, a)

    def ind_cb(uc, address, size, ud):
        k = (address - IND) >> 4
        esp = uc.reg_read(UC_X86_REG_ESP)
        na = IND_ARGS[k]
        raw = struct.unpack('<%dI' % na, bytes(uc.mem_read(esp + 4, 4 * na))) if na else ()
        args = tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)
        n = len(m.api_log)
        m.api_log.append(('ind%d' % k, args))
        h = mix(ST.get('seed', 0), k, n, 5)
        if k == 2:                          # the plugin's GetFuncDesc writes the kind text and an id
            uc.mem_write(raw[0], ST['kind'] + b'\0')
            uc.mem_write(raw[1], struct.pack('<I', h))
        val = ST.get('ind_ret', {}).get(k, h)
        uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)

    m.raw.mem_write(IND, b'\xc3' * 0x10 * len(IND_ARGS))
    m.raw.hook_add(UC_HOOK_CODE, ind_cb, None, IND, IND + 0x10 * len(IND_ARGS) - 1)


def begin(ctx, extra=()):
    m = ctx.m
    install(m)
    ST.clear()
    ST['seed'] = ctx.rng.getrandbits(32)
    ST['active'] = (set(STUBS) - DEFAULT_OFF) | set(extra)
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    return ctx.rng


def ind(k):
    return IND + 0x10 * k


# ---- shared worlds -------------------------------------------------------------------------------------------------
def table_world(ctx, count=None, size=0x600):
    """Entries in the pointer table at 0x109e8b40, the count [0x1015dc64] and the current index [0x1017b8c8]."""
    r = ctx.rng
    count = r.randrange(1, 8) if count is None else count
    ents = []
    for i in range(count):
        e = newbuf(ctx, size)
        ents.append(e)
        w32(ctx, 0x109e8b40 + 4 * i, e)
    w32(ctx, 0x1015dc64, count)
    cur = r.randrange(0, count)
    w32(ctx, 0x1017b8c8, cur)
    return ents, cur


def matrix_obj(ctx):
    """An object whose matrix (at +0x4c) is up to date (flag 0x8000 at +0x94) and has a position at +0x70."""
    o = newbuf(ctx, 0xa0)
    wf(ctx, o + 0x4c, *[ctx.rng.uniform(-2, 2) if ctx.rng.random() < 0.95 else special(ctx) for _ in range(12)])
    wf(ctx, o + 0x70, rfl(ctx), rfl(ctx), rfl(ctx))
    w32(ctx, o + 0x94, 0x8000 | ctx.rng.getrandbits(16))
    return o


def file_world(ctx):
    """The stream at [0x1015f344]: buffer pointer and count (negative now and then, so _flsbuf runs)."""
    r = ctx.rng
    f = newbuf(ctx, 0x20)
    w32(ctx, f, zbuf(ctx, 0x400))
    w32(ctx, f + 4, r.choice([0x300, 0x300, 3, 1, 0, 0xffffffff]))
    w32(ctx, 0x1015f344, f)


# ---- generators ----------------------------------------------------------------------------------------------------
def g_35478(ctx):
    r = begin(ctx)
    tbl = zbuf(ctx, 0x40)
    w32(ctx, tbl + 0x18, ind(0))
    w32(ctx, tbl + 0x20, ind(1))
    w32(ctx, 0x1015e68c, tbl)
    w32(ctx, 0x109e3dac, r.choice([0, 1, 1, 7]))
    w32(ctx, 0x1017b920, r.choice([0, 1]))
    wb(ctx, 0x1015e518, rb(ctx, 0x14))
    wb(ctx, 0x109e3e00, rb(ctx, 0x14))
    w32(ctx, 0x1014bef4, ctx.u32())
    # 0x100d3491 / 0x100d33ec state: the fade record at 0x1017e620 is finished (+0x10 >= +0xc), no palette cycle
    w32(ctx, 0x1017e65c, r.choice([0, 0, 1, 2, 0xffffffff]))
    w32(ctx, 0x1017e620 + 0xc, 3)
    w32(ctx, 0x1017e620 + 0x10, 5)
    w32(ctx, 0x1017e660, 0)
    w32(ctx, 0x1017e658, r.choice([0, ctx.u32()]))
    w32(ctx, 0x1017e650, r.choice([0, 1]))
    w32(ctx, 0x10152424, ctx.u32())
    w32(ctx, 0x10152428, ctx.u32())
    return []


KINDS = [b'RASTER', b'raster', b'Raster', b'SOUND', b'', b'RASTERX']


def g_36027(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015e56c, 0 if r.random() < 0.85 else ctx.u32())
    ST['load'] = 0 if r.random() < 0.15 else 0x12340000
    ST['proc'] = {0x1013a078: ind(2) if r.random() < 0.85 else 0, 0x1013a08c: ind(3) if r.random() < 0.85 else 0}
    ST['kind'] = r.choice(KINDS)
    ST['ind_ret'] = {3: r.choice([0, 1, 2, ctx.u32()])}
    wb(ctx, 0x1015e57c, rb(ctx, 0x40))
    path = cstr(ctx, b'plugin' + bytes(r.choice(b'abcdef') for _ in range(r.randrange(0, 20))) + b'.dll')
    return [path, ctx.u32()]


API_36027 = {
    'LoadLibraryA': lambda n, a: ST['load'],
    'GetProcAddress': lambda n, a: ST['proc'].get(a[1], 0),
}


def g_38eea(ctx):
    r = begin(ctx)
    q = newbuf(ctx, 0x80)
    px, py, pz = rfl(ctx), rfl(ctx), rfl(ctx)
    wf(ctx, q + 0x70, px, py, pz)
    p = newbuf(ctx, 0x50)
    w32(ctx, p + 0x48, q)
    obj = newbuf(ctx, 8)
    w32(ctx, obj, p)
    head = 0
    ids = []
    for _ in range(r.choice([0, 1, 2, 3, 5])):
        nd = newbuf(ctx, 0x6a0)
        near = r.random() < 0.3
        d = r.uniform(-500, 500) if near else r.uniform(-3000, 3000)
        if r.random() < 0.25:
            d, near = r.choice([1000.0, 999.99994, 1000.0001, -1000.0, 1e-40, -0.0]), 0
        wf(ctx, nd + 0x680, px + d, py + r.uniform(-300, 300) * near, pz + r.uniform(-300, 300) * near)
        i = r.choice([ctx.u32(), r.randrange(0, 10)])
        ids.append(i)
        w32(ctx, nd + 0x124, i)
        w32(ctx, nd + 0x690, head)
        head = nd
    w32(ctx, 0x109e3760, head)
    c = r.random()
    ident = 0xffffffff if c < 0.5 else (r.choice(ids) if ids and c < 0.8 else ctx.u32())
    return [obj, ident]


def g_39aca(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015d0a4, 0 if r.random() < 0.85 else ctx.u32())
    w32(ctx, 0x1017b8f8, ctx.u32())
    ST['heap'] = 0 if r.random() < 0.1 else ARENA + 0x40 * r.randrange(0, 4)
    wb(ctx, ARENA, rb(ctx, 0x500))
    wb(ctx, 0x109e369c, rb(ctx, 4))
    return [r.choice([0, 1, 5, 0xb, 0xc, 0xffffffff, ctx.u32()]), ctx.u32()]


API_39aca = {'HeapAlloc': lambda n, a: ST['heap']}


def g_3cbb3(ctx):
    r = begin(ctx)
    ents, cur = table_world(ctx)
    for e in ents:
        sub = newbuf(ctx, 0x620)
        w32(ctx, sub + 0x613, r.randrange(0, 4))
        w32(ctx, e + 0x28, sub)
    rec = newbuf(ctx, 0x10)
    w16(ctx, rec + 6, r.choice([0xffff, 0, 1, 2, 0x8000]))
    w32(ctx, 0x109e88e0, rec)
    prev = r.choice([0, 1, 2, 0xffffffff])
    w32(ctx, 0x109b9e08, prev)
    mode = r.choice([0, 1, 1, 2, prev, 0xffffffff])
    return [ctx.u32(), mode]


def node_list(ctx, n):
    head = 0
    for _ in range(n):
        nd = newbuf(ctx, 0x20)
        w32(ctx, nd + 0x14, head)
        head = nd
    return head


def g_3ce98(ctx):
    r = begin(ctx, extra=(0x10075124,))
    ents, cur = table_world(ctx)
    for e in ents:
        o = newbuf(ctx, 0x500)
        w32(ctx, o + 0x4f0, r.choice([0, 1, 4, 5]))
        p = newbuf(ctx, 0x10)
        w32(ctx, p + 4, r.choice([cur, ctx.u32()]))
        w32(ctx, o, p)
        w32(ctx, e + 0x28, o)
    w32(ctx, 0x1015ebbc, r.choice([0, 1]))
    w32(ctx, 0x1017b8c4, newbuf(ctx, 0x10))
    w32(ctx, 0x1015dc14, node_list(ctx, r.choice([0, 1, 3])))
    kind = r.choice([0xd, 0xd, 0x31, 0x40, ctx.u32()])
    w32(ctx, 0x1015dc18, kind)
    obj = 0 if r.random() < 0.1 else newbuf(ctx, 0x20)
    c = r.random()
    value = (kind - 0x31) & 0xffffffff if c < 0.3 else (ctx.u32() if c < 0.5 else r.randrange(0, 4))
    if obj:
        w32(ctx, obj + 0x10, value if r.random() < 0.5 else ctx.u32())
    return [ctx.u32(), ctx.u32(), value, ctx.u32(), ctx.u32(), obj]


def g_4131b(ctx):
    r = begin(ctx)
    ents, cur = table_world(ctx, count=r.randrange(1, 6))
    for i, e in enumerate(ents):
        w32(ctx, e + 4, r.choice([cur, i, ctx.u32()]))
        w32(ctx, e + 0x14, r.choice([3, 3, 0, 1]))
        w32(ctx, e + 0xc, r.choice([cur, ctx.u32()]))
        w16(ctx, e + 0x1c, r.choice([0, 0, 0, 2, 4, 0x8000, ctx.u32()]))
        o = newbuf(ctx, 0x90)
        w32(ctx, o, r.choice([0, 0, 1]))
        w32(ctx, e + 0x48, o)
    if r.random() < 0.2:
        w32(ctx, 0x109e8b40 + 4 * r.randrange(0, len(ents)), 0)
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    msg = newbuf(ctx, 8)
    wb(ctx, msg + 2, bytes([r.randrange(0, len(ents) + 1)]))
    return [msg]


def g_41b2d(ctx):
    r = begin(ctx)
    slot = r.randrange(0, 4)
    old = r.choice([0, 5, 100, 0x7fffffff, 0x80000000, ctx.u32()])
    w32(ctx, 0x1015db80 + 4 * slot, old)
    msg = newbuf(ctx, 0x30)
    w32(ctx, msg + 2, r.choice([old + 1, old - 1, old, ctx.u32(), 0x7fffffff]))
    dens = r.random()
    wb(ctx, msg + 6, bytes(sum(1 << b for b in range(8) if r.random() < dens) for _ in range(0x28)))
    for i in range(0xfe):
        w16(ctx, 0x109e9ba0 + i * 0x50, r.choice([0, 0, 4, 1, 0xffff]))
    return [msg, slot]


def g_41d67(ctx):
    r = begin(ctx)
    s = newbuf(ctx, 0x140)
    wb(ctx, s + 0x1e, bytes([r.choice([0, 1, 0x80])]))
    w32(ctx, s + 0x16, r.choice([0, 1, ctx.u32()]))
    w32(ctx, s + 0xc, r.choice([0, 0, ctx.u32()]))
    wb(ctx, s + 0x136, bytes([r.choice([0, 1, 2, 2])]))
    w32(ctx, 0x1015d004, s)
    w32(ctx, 0x1015ea94, ind(4))
    w32(ctx, 0x1015ead8, ind(5))
    w32(ctx, 0x1017b8c4, newbuf(ctx, 0x10))
    return []


def g_44a69(ctx):
    r = begin(ctx)
    ents, cur = table_world(ctx, count=r.choice([0, 1, 2, 4, 7]) or 1)
    if r.random() < 0.1:
        w32(ctx, 0x1015dc64, 0)
    for e in ents:
        w16(ctx, e + 0x1c, r.choice([0, 0, 0x4000, 0x800, 0x4800, 1]))
    reset = r.choice([0, 0, 1])
    w16(ctx, ents[r.randrange(0, len(ents))] + 0x1c, 0)
    if reset:
        w16(ctx, ents[cur] + 0x1c, 0)
    w32(ctx, 0x1015ef18, r.choice([0, len(ents) - 1, len(ents), 0xffffffff, r.randrange(0, len(ents))]))
    return [r.choice([0, 1, 5]), reset]


def ef14_world(ctx):
    r = ctx.rng
    e = newbuf(ctx, 0x80)
    wf(ctx, e + 0x58, *[rfl(ctx) for _ in range(8)])
    w32(ctx, e + 0x4c, matrix_obj(ctx) if r.random() < 0.7 else 0)
    w32(ctx, 0x1015ef14, e if r.random() < 0.9 else 0)
    if r.random() < 0.5:
        f = newbuf(ctx, 4)
        wf(ctx, f, rfl(ctx))
        w32(ctx, 0x1015ef1c, f)
    else:
        w32(ctx, 0x1015ef1c, 0)
    w32(ctx, 0x10179cd4, r.choice([0, 1]))


def g_44b2c(ctx):
    r = begin(ctx)
    ef14_world(ctx)
    ptrs = [newbuf(ctx, 4) for _ in range(6)]
    if r.random() < 0.15:
        ptrs[r.randrange(0, 6)] = ptrs[r.randrange(0, 6)]
    return ptrs


def g_453a3(ctx):
    r = begin(ctx)
    ef14_world(ctx)
    w32(ctx, 0x1013aed4, r.choice([4, 4, 0, 0xffffffff]))
    cam = newbuf(ctx, 0x20)
    wf(ctx, cam, *[rfl(ctx) for _ in range(8)])
    w32(ctx, 0x1014cae0, cam)
    w32(ctx, 0x1015dc78, ctx.u32())
    w32(ctx, 0x1015ef28, ctx.u32())
    w32(ctx, 0x1015dc80, r.choice([0, 1, 2, 18, 19, 40, r.randrange(-50, 1000)]))
    wf(ctx, 0x1013aee4, rfl(ctx, -5, 5))
    wf(ctx, 0x1015ef24, rfl(ctx))
    w32(ctx, 0x1015ef04, r.choice([0, 1]))
    w32(ctx, 0x1013aecc, fbits(rfl(ctx)))
    w32(ctx, 0x1013aed0, fbits(rfl(ctx)))
    return []


def tramp1(target):
    """push [esp+4]; call target; add esp, 4; fstp dword [OUTF]; xor eax, eax; ret"""
    return (b'\xff\x74\x24\x04' + b'\xb8' + struct.pack('<I', target) + b'\xff\xd0' + b'\x83\xc4\x04' +
            b'\xd9\x1d' + struct.pack('<I', OUTF) + b'\x31\xc0\xc3')


def g_47bcd(ctx):
    r = begin(ctx)
    wb(ctx, TRAMP, tramp1(0x10047bcd))
    wb(ctx, OUTF, bytes(8))
    obj = newbuf(ctx, 0x10)
    now = ctx.u32()
    w32(ctx, 0x1015dc78, now)
    target = rfl(ctx)
    cur = target + r.choice([0.0, 1e-8, -1e-8, r.uniform(-50, 50), r.uniform(-5000, 5000)])
    w32(ctx, obj, (now - r.choice([0, 1, 5, 30, 1000, ctx.u32()])) & 0xffffffff)
    wf(ctx, obj + 4, target, cur, r.choice([1.0, 30.0, 100.0, 0.5, rfl(ctx), -10.0]))
    return [obj]


def code_strings(ctx):
    """The 16 records (code, text, text) at 0x1013b380 and the label [0x109b98c0]; returns the codes."""
    r = ctx.rng
    codes = []
    for i in range(16):
        code = bytes(r.choice(b'abcdwxyz') for _ in range(r.randrange(3, 7)))
        codes.append(code)
        w32(ctx, 0x1013b380 + i * 0xc, cstr(ctx, code))
        w32(ctx, 0x1013b384 + i * 0xc, cstr(ctx, rb(ctx, r.randrange(1, 12)).replace(b'\0', b'q')))
        w32(ctx, 0x1013b388 + i * 0xc, cstr(ctx, rb(ctx, r.randrange(1, 12)).replace(b'\0', b'q')))
    w32(ctx, 0x109b98c0, cstr(ctx, b'label'))
    return codes


def g_488a3(ctx):
    r = begin(ctx)
    codes = code_strings(ctx)
    w32(ctx, 0x1015f1f8, 1 if r.random() < 0.9 else 0)
    wb(ctx, 0x109b9480, rb(ctx, 0x3f))
    k = r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15])
    code = bytes(c ^ 9 for c in codes[k])
    if r.random() < 0.85:
        n = len(code)
        wb(ctx, 0x109b94be - n + 2, code[:-1])
        low = code[-1]
    else:
        low = r.getrandbits(8)
    key = (r.getrandbits(16) << 16) | (0x700 if r.random() < 0.9 else r.getrandbits(8) << 8) | low
    flags = newbuf(ctx, 0x10)
    wb(ctx, flags, bytes(r.choice([0, 0, 1, 0x80]) for _ in range(8)))
    w32(ctx, 0x1017b8c4, flags)
    for i in range(2):
        slot = 0x10149d28 + i * 0x3c
        w32(ctx, slot, zbuf(ctx, 0x110))
        w32(ctx, slot + 0xc, r.choice([0, 1]))
        w32(ctx, slot + 0x10, r.choice([0, 0x10, 0x40]))
        w32(ctx, slot + 0x34, r.randrange(0, 100))
    w32(ctx, 0x1015cfec, r.choice([0, 1, 0xffffffff, 0x7fffffff, 0x80000000, ctx.u32()]))
    w32(ctx, 0x10179cd4, r.choice([0, 1]))
    w32(ctx, 0x1013aee0, r.choice([0xffffffff, 3]))
    return [key]


def g_4dbc5(ctx):
    r = begin(ctx)
    file_world(ctx)
    return [newbuf(ctx, 0x10)]


def g_4e24a(ctx):
    r = begin(ctx)
    file_world(ctx)
    wb(ctx, 0x1015f230, rb(ctx, 0x110))
    idx = r.choice([0, 1, 5, 0x80, 0xfd, 0xfe, 0xfe, 0xff, r.randrange(0, 0x100)])
    w32(ctx, 0x1015f340, idx)
    w32(ctx, 0x1015f348, r.choice([0, 1, 3, 7, 8, 8, r.randrange(0, 9)]))
    ST['fwrite_count'] = 0xff
    bits = r.choice([0, 1, 2, 3, 5, 7, 8, 9, 12, 16, 24, 31, 32, r.randrange(1, 40), 0xffffffff])
    return [r.choice([ctx.u32(), r.getrandbits(32), 0xffffffff, 0x80000000]), bits]


def g_4ff72(ctx):
    r = begin(ctx)
    slot = r.randrange(0, 8)
    obj = newbuf(ctx, 0xa0)
    w32(ctx, obj + 4, slot)
    a = r.choice([10, 10, 1, 2, 3, r.randrange(0, 12)])
    b = r.choice([a, r.randrange(0, 12)])
    w32(ctx, obj + 0x8c, a)
    w32(ctx, obj + 0x90, b)
    wf(ctx, obj + 0x58, rfl(ctx), rfl(ctx), rfl(ctx))
    w32(ctx, 0x109b8b60 + 4 * slot, r.choice([0xffffffff, 0xe, a, b, r.randrange(0, 12)]))
    base = newbuf(ctx, 0x400)
    for off in range(0, 0x400, 0x10):
        if r.random() < 0.3:
            wb(ctx, base + off, b'\0')
    cam = newbuf(ctx, 0x10)
    wf(ctx, cam, rfl(ctx), rfl(ctx), rfl(ctx))
    w32(ctx, 0x1014cae0, cam)
    w32(ctx, 0x1015ef0c, ctx.u32())
    point = newbuf(ctx, 12) if r.random() < 0.5 else 0
    return [obj, base, point]


def g_529a2(ctx):
    r = begin(ctx)
    ents, cur = table_world(ctx, count=r.randrange(1, 6))
    for i, e in enumerate(ents):
        w32(ctx, e + 8, r.randrange(0, 4))
        w32(ctx, e + 0x10, r.randrange(0, 8))
        w32(ctx, e + 0x48, matrix_obj(ctx))
    for g in range(4):
        w32(ctx, 0x109b7d40 + g * 0x6c, r.choice([0, 1, 2, r.randrange(0, len(ents))]) % len(ents))
    wb(ctx, 0x109b8430, rb(ctx, 0x74 * 4 + 0x40))
    for a in range(0x109b8430, 0x109b8430 + 0x74 * 4 + 0x40, 4):
        if r.random() < 0.9:
            wf(ctx, a, rfl(ctx))
    index = r.choice([r.randrange(0, len(ents)), r.randrange(0, len(ents)), len(ents), ctx.u32()])
    return [index, newbuf(ctx, 4), newbuf(ctx, 4), newbuf(ctx, 4)]


def g_5790a(ctx):
    r = begin(ctx)
    p = newbuf(ctx, 0x10)
    w32(ctx, p + 2, newbuf(ctx, 0x200) if r.random() < 0.7 else 0)
    w32(ctx, p + 6, newbuf(ctx, 0x200) if r.random() < 0.7 else 0)
    w32(ctx, 0x10144230, p)
    w32(ctx, 0x101668a4, r.choice([0, 1]))
    w32(ctx, 0x101c4fdc, ctx.u32())
    return [ctx.u32(), newbuf(ctx, 0x80)]


def g_5ada5(ctx):
    r = begin(ctx)
    t = newbuf(ctx, 0x100)
    for j in range(6):
        c = r.choice([0, b'g'[0], b'G'[0], b'x'[0], b'a'[0], 0x80, 0xff])
        wb(ctx, t + 0x7c + 0x20 * j, bytes([c]))
    obj = newbuf(ctx, 0x90)
    w32(ctx, obj + 0x88, t)
    w32(ctx, 0x101c42a0, ctx.u32())
    return [obj]


def g_607d5(ctx):
    r = begin(ctx)
    m = newbuf(ctx, 36)
    wf(ctx, m, *[r.uniform(-3, 3) if r.random() < 0.9 else rfl(ctx) for _ in range(9)])
    return [m]


def ring_world(ctx):
    r = ctx.rng
    wb(ctx, 0x101cffc0, rb(ctx, 0x200))
    wb(ctx, 0x101cfce0, struct.pack('<128I', *[r.choice([r.randrange(0, 200), r.getrandbits(32),
                                                          r.randrange(-100, 100) & 0xffffffff]) for _ in range(128)]))
    w32(ctx, 0x1017cd38, r.randrange(0, 0x7f))
    w32(ctx, 0x1017cd3c, r.randrange(0, 0x7f))


def g_61c32(ctx):
    r = begin(ctx)
    ring_world(ctx)
    e = newbuf(ctx, 0x160)
    wf(ctx, e + 0x58, rfl(ctx), rfl(ctx), rfl(ctx))
    obj = newbuf(ctx, 0x4e0)
    w32(ctx, obj, e)
    w32(ctx, obj + 0x4d8, r.choice([0, 1, 1]))
    return [obj]


def g_61dfc(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x4b0)
    w32(ctx, obj, newbuf(ctx, 0x80))
    for k in (4, 5):
        w32(ctx, obj + 0x498 + 4 * k, newbuf(ctx, 0x80) if r.random() < 0.6 else 0)
    return [obj]


def f8e_world(ctx):
    r = ctx.rng
    ents, cur = table_world(ctx, count=r.randrange(1, 4))
    w16(ctx, ents[cur] + 0x1c, r.choice([0, 0, 0, 2, 1]))
    w32(ctx, 0x1015dc80, r.choice([0x12, 0x13, 0x13, 100, 5]))
    w32(ctx, 0x1015ee98, r.choice([0, 0, 0, 1]))
    w32(ctx, 0x1013aec4, r.choice([0, 0, 3, 9, 10]))


def g_61ecd(ctx):
    r = begin(ctx)
    f8e_world(ctx)
    cur = r32(ctx.m.uc, 0x1017b8c8)
    e = newbuf(ctx, 0x70)
    w32(ctx, e + 4, r.choice([cur, cur, ctx.u32()]))
    wf(ctx, e + 0x58, rfl(ctx), rfl(ctx), rfl(ctx))
    obj = newbuf(ctx, 0x530)
    w32(ctx, obj, e)
    wf(ctx, obj + 0x520, rfl(ctx))
    cam = newbuf(ctx, 0x10)
    wf(ctx, cam, rfl(ctx), rfl(ctx), rfl(ctx))
    w32(ctx, 0x1014cae0, cam)
    w32(ctx, 0x1015ef0c, ctx.u32())
    v = r.choice([140.0, 139.99, 140.01, 0.0, -5.0, 1000.0, rfl(ctx, 0, 300)])
    return [obj, fbits(v)]


def g_61f8e(ctx):
    r = begin(ctx)
    f8e_world(ctx)
    vals = []
    for _ in range(3):
        vals.append(r.choice([0.0, -0.0, 0.0, rfl(ctx), rfl(ctx, -3, 3)]))
    return [fbits(v) for v in vals]


def g_6326b(ctx):
    r = begin(ctx)
    rect = newbuf(ctx, 0x20)
    w32(ctx, rect, *[r.choice([r.randrange(0, 640), r.randrange(-50, 50), ctx.u32()]) for _ in range(4)])
    w32(ctx, rect + 0x1c, r.randrange(0, 0x100))
    w32(ctx, 0x109e3de8, ctx.u32())
    w32(ctx, 0x109e3da8, ctx.u32())
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    return [rect]


def g_6863d(ctx):
    r = begin(ctx)
    cur = r.randrange(0, 5)
    w32(ctx, 0x1017b8c8, cur)
    e = newbuf(ctx, 0x70)
    w32(ctx, e + 4, r.choice([cur, ctx.u32(), ctx.u32()]))
    w32(ctx, e + 0xc, r.choice([cur, ctx.u32()]))
    w32(ctx, e + 0x14, r.choice([3, 3, 0]))
    obj = newbuf(ctx, 0x570)
    w32(ctx, obj, e)
    w32(ctx, obj + 0x4f0, r.choice([4, 0, 1, 5]))
    w32(ctx, 0x1015ecd0, r.choice([0, 1, 1, 1]))
    cam = newbuf(ctx, 0x10)
    w32(ctx, 0x1014cae0, cam)
    return [obj]


def g_68d76(ctx):
    r = begin(ctx)
    vals = [fbits(rfl(ctx)) for _ in range(3)]
    angs = [fbits(r.choice([0.0, -0.0, 0.0001, r.uniform(-400, 400), r.uniform(-4, 4), 90.0, 180.0, 360.0, -90.0,
                             special(ctx), r.uniform(-1e5, 1e5)])) for _ in range(3)]
    outs = [newbuf(ctx, 4) if r.random() < 0.93 else 0 for _ in range(3)]
    return vals + angs + outs


def g_75124(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x20)
    blk = newbuf(ctx, 0x50)
    for off in (0x14, 0x20, 0x48):
        w32(ctx, blk + off, r.choice([0, 1, ctx.u32()]))
    w32(ctx, blk + 4, ctx.u32())
    w32(ctx, obj + 0xc, blk if r.random() < 0.85 else 0)
    w32(ctx, 0x1014cec0, ctx.u32())
    w32(ctx, 0x1014cec4, ctx.u32())
    w32(ctx, 0x1015dbcc, ctx.u32())
    return [obj]


def g_79a80(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015965c, r.choice([1, 1, 2]))
    n = r.randrange(0, 40)
    s = bytes(r.choice(b'ab  \t\n\x0b\x0c\x0dz\x80\xa0\xff') for _ in range(n))
    return [cstr(ctx, s.replace(b'\0', b'a'))]


def S(name, orig, gen, trials, **kw):
    d = dict(name=name, orig=orig, c=kw.pop('c', name), gen=gen, trials=trials)
    d.update(kw)
    return d


SPECS = [
    S('Sim_10038eea', 0x10038eea, g_38eea, 2000),
    S('Sim_10044b2c', 0x10044b2c, g_44b2c, 2000, void=True),
    S('Sim_100453a3', 0x100453a3, g_453a3, 2000, void=True),
    S('Sim_10047bcd', TRAMP, g_47bcd, 3000, c='Test_10047bcd'),
    S('Sim_100529a2', 0x100529a2, g_529a2, 2000),
    S('Sim_100607d5', 0x100607d5, g_607d5, 2000, void=True),
    S('Sim_10061ecd', 0x10061ecd, g_61ecd, 2000, void=True),
    S('Sim_10061f8e', 0x10061f8e, g_61f8e, 3000, void=True),
    S('Sim_10068d76', 0x10068d76, g_68d76, 3000, void=True),
]
