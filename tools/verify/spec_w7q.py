# Differential tests for round 7, letter q (see difftest.py and docs/checker.md).
#
# Helpers that are not converted (and a few big converted ones whose work does not matter here) are replaced in
# the emulator by a code hook at their first byte (the code bytes are not patched): the hook appends
# ('stub_<address>', arguments, extra words) to the checker's call log, which is compared between the original
# and the C run like the Windows calls (stack addresses are logged as 'stack', and where a stub reads or writes
# through a stack pointer it logs the bytes it read), answers in eax with a value chosen from the trial's seed
# and the call number, and returns to the caller (EIP = return address, ESP + 4; all cdecl). Function pointers
# the code calls (qsort's compare function, the new-handler) point at `ret` bytes in the checker's return page
# with a logging hook of the same kind. Windows imports are stubbed by the checker; handlers below give varied
# answers and write the out structures through the raw stack pointers.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW,
                               UC_X86_REG_FPSW, UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_100f/w7q_parse.c', 'sim/unsorted_100f/w7q_sort.c', 'sim/unsorted_100f/w7q_stream.c',
    'sim/unsorted_100f/w7q_find.c', '../tools/verify/w7q_test.c',
    # existing converted code the functions above call, and what those files need
    'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_100f/w6i_mem.c', 'sim/unsorted_100f/w6i_stream.c',
    'sim/unsorted_100f/w2h_mem.c', 'sim/unsorted_1010/w6j_stream.c', 'sim/unsorted_1010/w6j_heap.c',
    'sim/unsorted_1010/w2k_crt.c', 'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_1010/w2h_misc.c',
    # game code of this letter
    'sim/unsorted_100c/w7q_objects.c', 'sim/unsorted_100c/w7q_relative.c', 'sim/unsorted_100d/w7q_tree.c',
    'sim/unsorted_100d/w7q_records.c', 'sim/unsorted_100d/w7q_names.c',
    # existing converted code they call, and what those files need to link
    'sim/view/objlist.c', 'sim/view/w2l_rects.c', 'sim/util/lookup.c', 'sim/util/field.c',
    'sim/unsorted_100c/w5z_calls.c', 'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1004/tables.c',
    'sim/unsorted_100b/w2f_tables.c', 'sim/unsorted_100b/w2j_misc.c', 'sim/unsorted_100b/fields.c',
    'sim/unsorted_1003/w6e_state.c', 'sim/unsorted_1002/w5a_misc.c', 'sim/unsorted_100d/w2i_select.c',
    'sim/unsorted_1000/w2h_reset.c', 'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_1010/w6k_text.c',
    'sim/unsorted_100f/w2j_str.c', 'sim/unsorted_100f/w6i_misc.c', 'sim/render/w3q_texture.c',
    'sim/unsorted_1000/w2f_records.c', 'sim/unsorted_1000/w2g_fields.c', 'sim/util/record.c',
    'sim/unsorted_1002/w5x_calls.c',
    'sim/unsorted_1003/w2e_mono.c', 'sim/view/viewrect.c', 'sim/unsorted_1000/w2e_names.c',
    'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1002/w2l_tables.c',
    'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1003/w5a_misc.c',
    'sim/unsorted_1003/setters.c', 'sim/unsorted_1006/w5b_calls.c', 'sim/unsorted_1007/w5a_misc.c',
    'sim/util/ring.c', 'sim/unsorted_100d/w5x_calls.c', 'sim/unsorted_1010/w2i_crt.c', 'sim/unsorted_100d/nodes.c',
    'sim/unsorted_1001/w2e_names.c', 'sim/win/w4a_heap.c', 'sim/unsorted_1002/w3o_vectors.c',
    'sim/unsorted_1003/w2e_misc.c', 'sim/unsorted_1007/w2e_state.c', 'sim/unsorted_100a/w5b_calls.c',
    'sim/unsorted_100d/w2j_nodes.c', 'sim/unsorted_1002/w2e_object.c', 'sim/core/w2a_timers.c',
    'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1008/w3s_grid.c',
    'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_1008/cell_link.c', 'sim/view/w2d_view.c',
    'sim/view/w2l_lists.c', 'sim/core/timer.c', 'sim/view/matrix.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000
TRAMP_Q = 0x00F00800     # calls 0x100fd1c0 and stores edx:eax at OUTF
CMP = 0x00F00A00         # qsort compare function
NEWH = 0x00F00A10        # new-handler called by Sim_10106590 through the dword 0x1017e96c
VISIT = 0x00F00A20       # visit function handed to Sim_100dafa8

ST = {'seed': 0, 'fn': {}}


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def pick(h, options):
    return options[h % len(options)]


def logged(v):
    return 'stack' if STACK_LO <= v <= STACK_HI else v


# ---- stubs -------------------------------------------------------------------------------------------
def p_output(uc, n, a):
    # (stream, format, argument pointer): log the three words the argument pointer points at
    return pick(mix(ST['seed'], 1, n), [0, 5, 0xffffffff, 0x20, n]), list(struct.unpack('<3I', bytes(uc.mem_read(a[2], 12))))


def p_writeitems(uc, n, a):
    count = a[2]
    return pick(mix(ST['seed'], 2, n), [count, count, 0, count - 1, 0xffffffff]), []


def p_write(uc, n, a):
    count = a[2]
    return pick(mix(ST['seed'], 3, n), [count, count, count, 0, 0xffffffff, count - 1]), []


def p_seek(uc, n, a):
    seq = ST.get('seek') or [0]
    return seq[n % len(seq)], []


def p_mapstring(uc, n, a):
    # (locale, flags, source, count, out, out size, code page): log the source bytes, write three out bytes
    src = bytes(uc.mem_read(a[2], 4))
    h = mix(ST['seed'], 4, n)
    uc.mem_write(a[4], struct.pack('<I', h)[:3])
    return pick(h >> 8, [0, 1, 2, 2, 3]), list(src)


def p_ctype(uc, n, a):
    # 0x10104fd0 (info type, source, count, out words, code page, locale): log the source bytes, write a word
    src = bytes(uc.mem_read(a[1], 2))
    h = mix(ST['seed'], 5, n)
    uc.mem_write(a[3], struct.pack('<H', h & 0xffff))
    return pick(h >> 16, [0, 1, 1]), list(src)


def p_value(tag, options):
    return lambda uc, n, a: (pick(mix(ST['seed'], tag, n), options), [])


def p_cmp(uc, n, a):
    w = ST.get('width', 4)
    k = min(w, 4)
    x = int.from_bytes(bytes(uc.mem_read(a[0], k)), 'little', signed=True)
    y = int.from_bytes(bytes(uc.mem_read(a[1], k)), 'little', signed=True)
    return (x > y) - (x < y), []


def p_newh(uc, n, a):
    return (1 if n < ST.get('newh_yes', 0) else 0), []


def cstring(uc, addr, maxn=0x40):
    out = b''
    while len(out) < maxn:
        c = bytes(uc.mem_read(addr + len(out), 1))
        if c == b'\0':
            break
        out += c
    return list(out)


def p_draw(uc, n, a):
    # 0x100c7deb(object): log the record at 0x1018548c and the record [object+0xec] as the callee sees them
    g = list(struct.unpack('<5I', bytes(uc.mem_read(0x1018548c, 20))))
    o = list(struct.unpack('<5I', bytes(uc.mem_read(r32(uc, a[0] + 0xec), 20))))
    return mix(ST['seed'], 10, n), g + o


def p_relative(uc, n, a):
    # 0x100cbc6b(dx, dy, dz, four float out pointers): the three float words are logged as arguments
    outs = ST.get('rel') or [0, 0, 0, 0]
    for k in range(4):
        uc.mem_write(a[3 + k], struct.pack('<I', outs[k] & 0xffffffff))
    return mix(ST['seed'], 11, n), []


def p_name(tag, options):
    # a callee whose first argument is a string: log its bytes
    return lambda uc, n, a: (pick(mix(ST['seed'], tag, n), options), cstring(uc, a[0]))


def p_namerec(uc, n, a):
    # 0x100278c0(name record, out): log the 15 bytes of the record
    return pick(mix(ST['seed'], 12, n), ST.get('objs') or [0]), list(bytes(uc.mem_read(a[0], 15)))


# original address -> (argument words, policy, C symbol that is stubbed the same way or None)
STUBS = {
    0x10100fd0: (3, p_output, None),
    0x100febc0: (4, p_writeitems, None),
    0x10105750: (3, p_write, None),
    0x10105fe0: (3, p_seek, None),
    0x101053c0: (7, p_mapstring, None),
    0x10104fd0: (6, p_ctype, None),
    0x101069c0: (1, p_value(6, [0, 0, 0x00A80000, 0x00A80040]), None),
    0x10105970: (7, p_value(7, [0, 0xffffffff, 0x12345678, 0x30000000]), None),
    0x10109890: (1, p_value(8, [0x44, 0xffffffff, 0x1000]), None),
    0x100fef30: (1, p_value(9, [0, 0x00A81000]), None),
    CMP: (2, p_cmp, None),
    NEWH: (1, p_newh, None),
    VISIT: (1, p_value(13, [0, 1, 1, 2, 3, 0xffffffff]), None),
    # game code
    0x100c7deb: (1, p_draw, None),
    0x100cbc6b: (7, p_relative, None),
    0x100eccf3: (2, p_value(14, [0, 1, 7, 0x10, 0xffffffff]), None),
    0x100ca670: (3, p_value(15, [0, 1]), None),
    0x100c398d: (2, p_value(16, [0, 1]), None),
    0x100bc280: (2, p_value(17, [0]), None),
    0x100b9203: (2, p_value(18, [0, 1]), None),
    0x100599a0: (1, p_name(19, [0x00AE0000, 0x00AE0000, 0x00AE0100]), None),
    0x100278c0: (2, p_namerec, None),
    # converted callees whose own work does not matter here (sound, texture loading, game state): both the
    # original and the C version are stubbed, the log shows the call and its arguments
    0x1000ebfb: (5, p_name(20, [0, 1, 5]), 'Sim_1000ebfb'),
    0x10006ba7: (3, p_name(21, [0, 1, 0xffffffff]), 'Sim_10006ba7'),
    0x1003cbb3: (2, p_value(22, [0]), 'Sim_1003cbb3'),
    0x100203cf: (2, p_value(23, [0]), 'Sim_100203cf'),
}
CALLBACKS = (CMP, NEWH, VISIT)


def install(m):
    if getattr(m, '_w7q', False):
        return
    m._w7q = True

    def make(addr, nargs, pol):
        def cb(uc, address, size, ud):
            esp = uc.reg_read(UC_X86_REG_ESP)
            raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
            name = 'stub_%x' % addr
            n = sum(1 for e in m.api_log if e[0] == name)
            val, extra = pol(uc, n, raw)
            m.api_log.append((name, tuple(logged(v) for v in raw), tuple(extra)))
            uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
            if addr not in CALLBACKS:       # callbacks are real `ret` bytes in the return page
                uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
                uc.reg_write(UC_X86_REG_ESP, esp + 4)
        return cb

    for addr, (nargs, pol, csym) in STUBS.items():
        if addr in CALLBACKS:
            m.raw.mem_write(addr, b'\xc3')
        m.raw.hook_add(UC_HOOK_CODE, make(addr, nargs, pol), None, addr, addr)
        if csym:
            c = m.syms[csym]
            m.raw.hook_add(UC_HOOK_CODE, make(addr, nargs, pol), None, c, c)
    m.raw.mem_write(TRAMP_Q, tramp_qword(0x100fd1c0))



def tramp_qword(target):
    code = b'\x55\x89\xe5\xff\x75\x08'
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4\x04'
    code += b'\xa3' + struct.pack('<I', OUTF)                   # mov [OUTF], eax
    code += b'\x89\x15' + struct.pack('<I', OUTF + 4)           # mov [OUTF+4], edx
    code += b'\x31\xc0\x5d\xc3'
    return code


def begin(ctx):
    m = ctx.m
    install(m)
    ST.clear()
    ST['seed'] = ctx.rng.getrandbits(32)
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    return ctx.rng


def api_pick(tag, options):
    return lambda n, args: pick(mix(ST['seed'], tag, n), options)


# ---- shared set-ups ------------------------------------------------------------------------------------
def ctype_table(ctx):
    """A character class table (words for -128..255, pointer at [0x10159450] to the entry of 0) and the
    multibyte switch 0x1015965c."""
    r = ctx.rng
    words = []
    for c in range(-128, 256):
        b = c & 0xff
        w = 0
        if c >= 0:
            if b in b' \t\n\r\v\f':
                w |= 8
            if 0x30 <= b <= 0x39:
                w |= 4
            if 0x41 <= b <= 0x5a:
                w |= 0x101
            if 0x61 <= b <= 0x7a:
                w |= 0x102
            if b >= 0x80 or r.random() < 0.05:
                w = r.getrandbits(16) if r.random() < 0.5 else w
        else:
            w = r.getrandbits(16)
        words.append(w)
    tab = ctx.buf(len(words) * 2 + 0x400, struct.pack('<%dH' % len(words), *words) + rb(ctx, 0x400))
    w32(ctx, 0x10159450, tab + 128 * 2)
    w32(ctx, 0x1015965c, r.choice([1, 1, 1, 0, 2, 2, 0xffffffff]))
    w32(ctx, 0x1017e950, r.choice([0, 0, 0x409, 0x411]))


def text(ctx, alphabet, n):
    r = ctx.rng
    return bytes(r.choice(alphabet) for _ in range(n))


def handle_table(ctx):
    """Handle table: count at 0x10ac385c, blocks of 32 eight-byte entries at 0x10ac3720 (flag byte at +4)."""
    r = ctx.rng
    w32(ctx, 0x10ac385c, r.choice([0, 1, 5, 32, 40, 64, 64, 64, 64]))
    for g in range(2):
        blk = ctx.buf(32 * 8, bytes(r.choice([0, 1, 1, 0x40, 0x41, 0x41, 0x81, 0x84, 0x85, 0xc5, 0xff,
                                              r.getrandbits(8)]) if i % 8 == 4
                                    else r.getrandbits(8) for i in range(32 * 8)))
        w32(ctx, 0x10ac3720 + 4 * g, blk)


def stream(ctx, flags=None, fd=None):
    """A stream record (0x20 bytes) whose pointers point into a fresh buffer."""
    r = ctx.rng
    data = ctx.buf(0x80, text(ctx, b'ab\n\n\r x', 0x80))
    f = newbuf(ctx, 0x20)
    w32(ctx, f, data + r.randrange(0, 0x60))
    w32(ctx, f + 4, r.choice([0, 0, 1, 2, 5, 0x10, 0x1f, 0xffffffff, 0x80000000]))
    w32(ctx, f + 8, r.choice([data, data, data, data + 0x70]))
    if flags is None:
        flags = r.choice([0, 1, 2, 3, 0x80, 0x82, 0x81, 0x102, 0x108, 0x10a, 0x109, 0x18a, 0x4102, 0x4002,
                          0x2109, 0x408, 0x8, 0x9, 0x2001, 0x1102, r.getrandbits(16)])
    w32(ctx, f + 0xc, flags)
    w32(ctx, f + 0x10, fd if fd is not None else r.choice([0, 1, 2, 3, 31, 32, 33, 40, 63]))
    w32(ctx, f + 0x18, r.choice([0x200, 0x1000, 0x10, 0]))
    return f


# ---- generators: parsing ----------------------------------------------------------------------------------
def g_atoi64(ctx):
    r = begin(ctx)
    ctype_table(ctx)
    s = text(ctx, b'  \t+-0123456789', r.randrange(0, 4)) + text(ctx, b'0123456789', r.randrange(0, 25)) + \
        text(ctx, b'x9 -\xc3\x80', r.randrange(0, 3))
    return [ctx.buf(len(s) + 8, s + b'\0' + rb(ctx, 7))]


def g_strtoxl(ctx):
    r = begin(ctx)
    ctype_table(ctx)
    k = r.random()
    if k < 0.3:
        body = text(ctx, b'0123456789', r.randrange(0, 14))
    elif k < 0.6:
        body = r.choice([b'0x', b'0X', b'0', b'']) + text(ctx, b'0123456789abcdefABCDEFxz', r.randrange(0, 12))
    else:
        body = text(ctx, b'0123456789aZz.\xc1\xe1\xff\x80 ', r.randrange(0, 14))
    s = text(ctx, b' \t', r.randrange(0, 3)) + r.choice([b'', b'', b'-', b'+']) + body
    p = ctx.buf(len(s) + 8, s + b'\0' + rb(ctx, 7))
    end = r.choice([0, newbuf(ctx, 4)])
    base = r.choice([0, 0, 10, 10, 16, 16, 8, 2, 36, 1, -1, 37, 3, 7, 35, r.getrandbits(32)])
    flags = r.choice([0, 1, 0, 1, 2, 4, 8, r.getrandbits(32)])
    return [p, end, base, flags]


def g_toupper(ctx):
    r = begin(ctx)
    ctype_table(ctx)
    c = r.choice([r.randrange(0, 256), r.randrange(0x61, 0x7b), r.randrange(0x41, 0x5b), r.randrange(-128, 0),
                  r.randrange(0x100, 0x10000), -1, r.getrandbits(32) & 0x8000ffff, r.randrange(0x80, 0x100)])
    return [c & 0xffffffff]


# ---- generators: sorting and allocation ----------------------------------------------------------------
def g_qsort(ctx):
    r = begin(ctx)
    width = r.choice([1, 2, 4, 4, 4, 8, 12, 5])
    ST['width'] = width
    count = r.choice([0, 1, 2, 3, 8, 9, 10, r.randrange(0, 40), r.randrange(0, 80)])
    k = r.random()
    data = bytearray(rb(ctx, count * width + 8))
    if k < 0.3:     # few distinct keys
        for i in range(count):
            data[i * width] = r.randrange(0, 4)
            if width > 1:
                data[i * width + 1:i * width + min(width, 4)] = bytes(min(width, 4) - 1)
    elif k < 0.5:   # already sorted or reversed
        vals = sorted(r.getrandbits(16) for _ in range(count))
        if r.random() < 0.5:
            vals.reverse()
        for i, v in enumerate(vals):
            data[i * width:i * width + min(width, 4)] = v.to_bytes(4, 'little')[:min(width, 4)]
    base = ctx.buf(len(data), bytes(data))
    return [base, count, r.choice([width, width, width, width, 0]), CMP]


def g_malloc(ctx):
    r = begin(ctx)
    ST['newh_yes'] = r.choice([0, 1, 2, 3])
    w32(ctx, 0x1017e96c, r.choice([0, NEWH, NEWH]))
    w32(ctx, 0x1015be5c, r.choice([0, 0x1e0, 0x3f0, 0xffffffff]))
    w32(ctx, 0x10ac3714, ctx.u32())
    size = r.choice([0, 1, 15, 16, 17, 0x100, 0x1e0, 0x1e1, 0xffffffe0, 0xffffffe1, 0xffffffff, 0xffffffd1,
                     r.getrandbits(12), r.getrandbits(32)])
    return [size, r.choice([0, 1, 1, 0x80000000])]


HEAP_API = {'HeapAlloc': api_pick(20, [0, 0, 0x00A90000, 0x00A90010])}


# ---- generators: streams -------------------------------------------------------------------------------
def std_streams(ctx):
    """The standard streams and their buffer slots, as Sim_10104ae0 reads them."""
    r = ctx.rng
    handle_table(ctx)
    for f in (0x10159b68, 0x10159b88):
        ctx.m.uc.mem_write(f, rb(ctx, 0x20))
        w32(ctx, f + 0x10, r.choice([0, 1, 2, 3, 33]))
        w32(ctx, f + 0xc, r.choice([0, 0, 2, 0x100, 0x1000, 0x1002, ctx.u32()]))
    for s in range(2):
        w32(ctx, 0x1017e930 + 4 * s, r.choice([0, 0x00A12340, 0x00A12340]))


def g_fprintf(ctx):
    r = begin(ctx)
    std_streams(ctx)
    f = r.choice([0x10159b68, 0x10159b88, stream(ctx), stream(ctx)])
    return [f, newbuf(ctx, 16), ctx.u32(), ctx.u32(), ctx.u32()]


def g_printf(ctx):
    begin(ctx)
    std_streams(ctx)
    return [newbuf(ctx, 16), ctx.u32(), ctx.u32(), ctx.u32()]


def g_fputs(ctx):
    r = begin(ctx)
    std_streams(ctx)
    f = r.choice([0x10159b68, 0x10159b88, stream(ctx), stream(ctx)])
    s = text(ctx, b'abc \n', r.randrange(0, 30))
    return [ctx.buf(len(s) + 4, s + b'\0' + rb(ctx, 3)), f]


def g_fflush(ctx):
    r = begin(ctx)
    handle_table(ctx)
    n = r.choice([0, 1, 2, 3])
    tab = newbuf(ctx, 4 * max(n, 1))
    for i in range(n):
        w32(ctx, tab + 4 * i, r.choice([0, stream(ctx), stream(ctx)]))
    w32(ctx, 0x10ac3860, tab)
    w32(ctx, 0x10ac4ba0, n)
    return [r.choice([0, stream(ctx), stream(ctx), stream(ctx)])]


FLUSH_API = {'FlushFileBuffers': api_pick(21, [0, 1, 1]), 'GetLastError': api_pick(22, [0, 5, 0x6d])}


def g_flush(ctx):
    r = begin(ctx)
    return [stream(ctx, r.choice([None, 2, 0x82, 0x102, 0x10a, 0x182, 0x88a, 0x6]))]


def g_ftell(ctx):
    r = begin(ctx)
    handle_table(ctx)
    f = stream(ctx, r.choice([None, None, 1, 2, 0x80, 0x109, 0x10a, 0x189, 0x2109, 0x2189, 0x409, 0x8,
                              0x9, 0x88, 0x100]))
    pos = r.choice([0, 0, 5, 0x100, 0x1234, 0xffffffff, 0x80000000, r.getrandbits(31)])
    ST['seek'] = [pos, r.choice([pos, pos, pos + 1, 0xffffffff]), r.getrandbits(32)]
    return [f]


# ---- generators: directory search -----------------------------------------------------------------------
def find_data(n, args, uc, tag):
    h = mix(ST['seed'], tag, n)
    esp = uc.reg_read(UC_X86_REG_ESP)
    dst = r32(uc, esp + 8)
    words = [pick(h, [0x80, 0x10, 0x20, 0x21, h & 0xff])]
    words += [mix(h, k) for k in range(1, 11)]
    name = bytes(0x61 + mix(h, 99, k) % 26 for k in range(mix(h, 98) % 40)) + b'\0'
    data = struct.pack('<11I', *words) + name
    data += bytes(mix(h, 97, k) & 0xff for k in range(0x140 - len(data)))
    uc.mem_write(dst, data)


def h_findfirst(n, args, uc):
    find_data(n, args, uc, 30)
    return pick(mix(ST['seed'], 31, n), [0xffffffff, 0x1234, 0x1234, 0x80000000, 0])


def h_findnext(n, args, uc):
    find_data(n, args, uc, 32)
    return pick(mix(ST['seed'], 33, n), [0, 1, 1, 2])


def h_local_ft(n, args, uc):
    esp = uc.reg_read(UC_X86_REG_ESP)
    src, dst = r32(uc, esp + 4), r32(uc, esp + 8)
    lo, hi = struct.unpack('<2I', bytes(uc.mem_read(src, 8)))
    uc.mem_write(dst, struct.pack('<2I', mix(lo, 1), mix(hi, 2)))
    return pick(mix(ST['seed'], n, 5), [1, 1, 1, 0])


def h_system_t(n, args, uc):
    esp = uc.reg_read(UC_X86_REG_ESP)
    src, dst = r32(uc, esp + 4), r32(uc, esp + 8)
    lo, hi = struct.unpack('<2I', bytes(uc.mem_read(src, 8)))
    uc.mem_write(dst, struct.pack('<8H', *[mix(lo, hi, k) & 0xffff for k in range(8)]))
    return pick(mix(ST['seed'], n, 6), [1, 1, 1, 0])


FIND_API = {'FindFirstFileA': h_findfirst, 'FindNextFileA': h_findnext,
            'GetLastError': api_pick(34, [2, 3, 8, 18, 0, 1, 4, 19, 5, 0x12, 0x80000000]),
            'FileTimeToLocalFileTime': h_local_ft, 'FileTimeToSystemTime': h_system_t}


def g_findfirst(ctx):
    begin(ctx)
    return [ctx.buf(16, 'text'), newbuf(ctx, 0x118)]


def g_findnext(ctx):
    begin(ctx)
    return [ctx.u32(), newbuf(ctx, 0x118)]


# ---- game code -----------------------------------------------------------------------------------------
def fbits(ctx, special=True):
    """A float word: mostly ordinary values, sometimes zero, denormals, huge values or one NaN/infinity."""
    r = ctx.rng
    k = r.random()
    if special and k < 0.04:
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000, 0x7fa00001])
    if k < 0.15:
        return r.choice([0, 0x80000000, 1, 0x80000001, 0x007fffff, 0x00800000, 0x7f7fffff, 0xff7fffff])
    if k < 0.3:
        return struct.unpack('<I', struct.pack('<f', r.choice([1.0, -1.0, 0.5, 1e-38, 3.0e38])))[0]
    if k < 0.75:
        return struct.unpack('<I', struct.pack('<f', r.uniform(-5000.0, 5000.0)))[0]
    return r.getrandbits(32) & 0xbfffffff     # exponent below 0x80: never NaN / infinity


def entity(ctx, who):
    """Entity `who` of the pointer table 0x109e8b40 (0x120 bytes, position floats at +0x58..+0x60)."""
    r = ctx.rng
    e = newbuf(ctx, 0x120)
    for off in (0x58, 0x5c, 0x60):
        w32(ctx, e + off, fbits(ctx))
    blk = newbuf(ctx, 0x510)
    w32(ctx, blk + 0x50c, r.choice([0, 1, 1, 2]))
    w32(ctx, e + 0x28, blk)
    w32(ctx, e + 8, r.choice([0, 1, 2, 3, 7, 15, 16, 31, 33, r.getrandbits(32)]))
    w32(ctx, 0x109e8b40 + 4 * who, e)
    return e


def answers(ctx, near=None):
    """The four float answers of the 0x100cbc6b stub (the second one around `near` when given)."""
    r = ctx.rng
    outs = [fbits(ctx, False) for _ in range(4)]
    if near is not None and r.random() < 0.7:
        outs[1] = struct.unpack('<I', struct.pack('<f', near * r.choice([0.5, 1.0, 1.0, 2.0, 2.0, 4.0, -1.0])))[0]
    if near is not None and r.random() < 0.05:
        outs[1] = 0x7fc00000
    ST['rel'] = outs


def target_entry(ctx, i, who=None, ent=None):
    """Entry i of the table 0x109a8640 (0x54 bytes)."""
    r = ctx.rng
    t = 0x109a8640 + 0x54 * i
    wb(ctx, t, rb(ctx, 0x54))
    w32(ctx, t, r.choice([0, 1, 1, 1, 1, 5]))
    if r.random() < 0.5:
        obj = newbuf(ctx, 0x80)
        for off in (0x70, 0x74, 0x78):
            w32(ctx, obj + off, fbits(ctx))
        w32(ctx, t + 4, obj)
    else:
        w32(ctx, t + 4, 0)
    for off in (0x10, 0x18, 0x1c, 0x20):
        w32(ctx, t + off, fbits(ctx))
    ctx.m.uc.mem_write(t + 0x24, struct.pack('<H', r.choice([0, 1, 0x20, 0x40, 0x41, 0x61, 0x21, r.getrandbits(16)])))
    if who is not None:
        w32(ctx, t + 8, r.choice([who | 0x200, who | 0x200, who, ctx.u32()]))
    if ent is not None:
        w32(ctx, t + 0xc, r.choice([r32(ctx.m.uc, ent + 8)] * 3 + [ctx.u32()]))


def g_ca996(ctx):
    r = begin(ctx)
    who = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, who)
    e = entity(ctx, who)
    i = r.randrange(0, 8)
    w32(ctx, e + 0x118, (r.getrandbits(24) << 8) | i)
    target_entry(ctx, i)
    t = 0x109a8640 + 0x54 * i
    near = struct.unpack('<f', struct.pack('<I', r32(ctx.m.uc, t + 0x10)))[0]
    if near != near or abs(near) > 1e30:
        near = 1.0
    ST['rel'] = [fbits(ctx, False) for _ in range(4)]
    if r.random() < 0.8:
        ST['rel'][2] = struct.unpack('<I', struct.pack('<f', near * r.choice([0.5, 1.0, 2.0, -1.0])))[0]
    return []


def g_cab7b(ctx):
    r = begin(ctx)
    who = r.randrange(0, 4)
    e = entity(ctx, who)
    count = r.choice([0, 1, 4] + [8] * 7)
    w32(ctx, 0x1017e5a8, count)
    i = r.choice([-1, count, count + 1] + [r.randrange(0, 8)] * 12)
    if 0 <= i < 8:
        target_entry(ctx, i, who, e)
    answers(ctx)
    flags = r.choice([0] * 8 + [0x100, 0x10000, 0x10100, 2, 4, 8, 1, 0x200, ctx.u32()])
    return [who, i & 0xffffffff, flags]


def g_cb299(ctx):
    r = begin(ctx)
    who = r.randrange(0, 4)
    e = entity(ctx, who)
    w32(ctx, 0x1017b8c8, r.choice([who, who, (who + 1) & 3]))
    w32(ctx, 0x1017e5b0, r.choice([0, 1]))
    count = r.choice([0, 1, 4] + [8] * 7)
    w32(ctx, 0x1015dc68, count)
    i = r.choice([-1, count, count + 1] + [r.randrange(0, 8)] * 12)
    w32(ctx, 0x1017d218, r.choice([0, 3, 6, 7, 7, 7]))
    for k in range(4):
        w32(ctx, 0x109b8400 + 4 * k, r.choice([0, 1, 2]))
    for k in range(8):
        # +0x28 (a block, position floats at +0x44) is what Sim_100b53bb tests, +0x2c (an object, position
        # through Sim_100a9cfb) what Sim_100b5386 tests
        ent = 0x101a2100 + 0x88 * k
        blk = newbuf(ctx, 0x50)
        for off in (0x44, 0x48, 0x4c):
            w32(ctx, blk + off, fbits(ctx))
        obj = newbuf(ctx, 0x80)
        for off in (0x70, 0x74, 0x78):
            w32(ctx, obj + off, fbits(ctx))
        w32(ctx, ent + 0x28, r.choice([0] + [blk] * 7))
        w32(ctx, ent + 0x2c, r.choice([0] + [obj] * 7))
    if 0 <= i < 8:
        rec = 0x109e9ba0 + 0x50 * i
        wb(ctx, rec, rb(ctx, 0x50))
        ctx.m.uc.mem_write(rec, struct.pack('<H', r.choice([0, 0, 4, 0x40, 0x400, 0x800, 0x1000, 0x1400, 0x1040,
                                                            r.getrandbits(16) & ~4])))
        w32(ctx, rec + 8, r.choice([-1, 0, 1, 2, 3, 4, 5, 6, 7, 8]))
        w32(ctx, rec + 0x10, r.choice([-1, 0, 1, 2, 3]))
    answers(ctx, 175000.0)
    flags = r.choice([0] * 8 + [0x10000, 0x20000, 0x40000, 0x60000, 1, 2, 0x20, 0x100, ctx.u32() & 0x7ff00])
    return [who, i & 0xffffffff, flags]


def g_c81ed(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x100)
    ctx.m.uc.mem_write(obj + 4, struct.pack('<h', r.choice([0, 1, 1, 1, -1, 5])))
    w32(ctx, 0x10151cf8, r.choice([0, 1, 1, 1]))
    w32(ctx, 0x109e89d8, ctx.u32())
    w32(ctx, 0x1015dc80, r.choice([0, 0x1000, 0x4000, 0x10000, 0x20000]))
    wb(ctx, 0x1018548c, rb(ctx, 20))
    w32(ctx, obj + 0xec, newbuf(ctx, 20))
    w32(ctx, obj + 0xf8, r.choice([0, 1, 2, 3, 4, 5]))
    if r.random() < 0.85:
        n = newbuf(ctx, 0xc)
        w32(ctx, n, r.choice([0, 1, 1, 2]))
        w32(ctx, n + 4, r.choice([0, 1, 1]))
        w32(ctx, n + 8, r.choice([0, 0x4000, 0x9000, 0x20000]))
        o = newbuf(ctx, 0x10)
        w32(ctx, o, r.choice([0x8000, 0x10000, 0x20000, 0x7fffffff]))  # the original's divide traps on overflow
        w32(ctx, o + 4, newbuf(ctx, 20))
        w32(ctx, o + 8, newbuf(ctx, 20))
        w32(ctx, o + 0xc, newbuf(ctx, 20))
        pair = ctx.buf(8, struct.pack('<2I', n, o))
        w32(ctx, obj + 0xf4, pair)
    else:
        w32(ctx, obj + 0xf4, 0)
    return [obj]


def g_ccb0d(ctx):
    r = begin(ctx)
    w32(ctx, 0x101668e8, r.choice([0, 1, 1]))
    words = [r.choice([0, 0, 0, r.randrange(1, 0x40)]) for _ in range(0x100)]
    w32(ctx, 0x10157590, ctx.buf(0x200, struct.pack('<256H', *words)))
    base = newbuf(ctx, 0x10 + 0x400)
    for k in range(0x100):
        w32(ctx, base + 0x10 + 4 * k, 4 * r.randrange(0, 0x100))
    s = bytes(r.choice([r.randrange(1, 0x80), r.randrange(0x80, 0x100), 0x81, 0x85, 0xa5, 0xe5, 0xec, 0xef, 0xfa])
              for _ in range(r.randrange(0, 24)))
    return [ctx.buf(len(s) + 8, s + bytes(8)), base]


def g_d191d(ctx):
    r = begin(ctx)
    n = r.choice([0, 1, 2, 5, 12])
    nodes = [newbuf(ctx, 0x10) for _ in range(n)]
    for i, a in enumerate(nodes):
        later = nodes[i + 1:]
        w32(ctx, a + 8, r.choice([0] + later) if later else 0)
        w32(ctx, a + 0xc, r.choice([0] + later) if later else 0)
    return [nodes[0] if nodes else 0]


def g_d1b91(ctx):
    r = begin(ctx)
    for a in (0x10144fa0, 0x101452d0, 0x1015f1d8, 0x1014cb1c):
        w32(ctx, a, ctx.u32())
    opt = r.choice([0x2b, 0x54, 0x58, 0xc4, 0xc5, 0xc5, 0x2a, 0x2c, 0xc6, 0x100, -1, r.randrange(0, 0x100)])
    return [opt & 0xffffffff, r.choice([0, 1, 2, ctx.u32()])]


def records_block(ctx):
    r = ctx.rng
    blk = newbuf(ctx, 0x600)
    recs = newbuf(ctx, 8 * 0x128)
    w32(ctx, blk + 0x84, recs)
    w32(ctx, blk, newbuf(ctx, 0x100))
    for t in range(10):
        ty = 0x10145508 + 0xd6 * t
        w32(ctx, ty + 8, r.choice([0, 4, 4, 0xff]))
        w32(ctx, ty + 0x10, r.choice([0x3d, 0, 1]))
        w32(ctx, ty + 0x2c, ctx.u32())
        w32(ctx, ty + 0x86, r.choice([0, 1, ctx.u32()]))
    for i in range(8):
        rec = recs + 0x128 * i
        w32(ctx, rec, r.choice([0, 0, -1]))
        w32(ctx, rec + 4, r.choice([-1, 0, 1, 2, 3, 4, 5, 6, 9]))
        w32(ctx, rec + 8, r.choice([1, 1, 2, -1, 0]))
        w32(ctx, rec + 0xc, r.choice([0, 0, 1]))
        w32(ctx, rec + 0x20, r.choice([-2, 0, 5]))
    ctx.m.uc.mem_write(blk + 0x560, struct.pack('<2H', r.choice([0, 0x80, 0xff, 0x7f]), r.getrandbits(16)))
    w32(ctx, blk + 0x4fc, r.randrange(0, 8))
    return blk


def g_d650f(ctx):
    r = begin(ctx)
    blk = records_block(ctx)
    w32(ctx, blk + 0x4f8, r.choice([0, 1, 3, 8, 8]))
    return [blk]


def g_d666d(ctx):
    r = begin(ctx)
    blk = records_block(ctx)
    w32(ctx, blk + 0x4f8, r.choice([0, 1, 3, 8, 8, -1]))
    return [r.choice([blk, blk, blk, blk, 0])]


def g_da14c(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017e950, 0)
    w32(ctx, 0x1012da30, r.choice([-2, 0, 0, 0, 0]))
    names = [text(ctx, b'abAB', r.randrange(1, 6)) for _ in range(4)]
    for g, nm in enumerate(names):
        wb(ctx, 0x10a51ac0 + 0x300 * g, nm + b'\0')
    for i in list(range(0, 8)) + list(range(0x100, 0x108)):
        ctx.m.uc.mem_write(0x10ac1b00 + 0xe * i, struct.pack('<h', r.choice([-1, 0, 1, 2, 3])))
    w32(ctx, 0x1017e6cc, r.choice([0, 1, 3, 6, 8, 0xff, 0x100, 0x150]))
    w32(ctx, 0x10152480, r.choice([0x100, 0x101, 0x104, 0x108, 0x1ff, 0x200, 0x250]))
    info = newbuf(ctx, 0x60)
    k = r.random()
    if k < 0.1:
        nm = b''
    elif k < 0.5:
        nm = r.choice(names) + text(ctx, b'aB', r.randrange(0, 3))
        if r.random() < 0.5:
            nm = nm.upper()
    else:
        nm = text(ctx, b'abAB', r.randrange(1, 8))
    wb(ctx, info + 0x20, nm + b'\0')
    w32(ctx, info + 0x40, r.choice([3, 3, 2, 0, 1]))
    first = r.choice([0, 3, 9, 10, 95, 120, -2])
    w32(ctx, info + 0x4c, first)
    w32(ctx, info + 0x50, first + r.choice([-1, 0, 1, 2]))
    w32(ctx, info + 0x54, r.choice([1, 2, 3, -5, 0xb5, 1000, 7]))
    return [r.choice([info, info, info, info, 0]), newbuf(ctx, 0x48)]


def g_dafa8(ctx):
    r = begin(ctx)
    src = newbuf(ctx, 0x20)
    ctx.m.uc.mem_write(src + 8, struct.pack('<h', r.choice([-1, -2, -2, 5, 0])))
    nm = r.choice([b'^', b'^ab', b'abc', b'^xyz012345678']) + text(ctx, b'xy', r.randrange(0, 4))
    wb(ctx, src + 0xa, (nm + bytes(16))[:16])
    k = r.choice([0, 1, 2, 3])
    if r.random() < 0.8:
        lst = ctx.buf(0xc * 4 + 0x10, bytes(r.choice(b'abcdef\0') for _ in range(0xc * 4 + 0x10)))
        w32(ctx, 0x1017e688, lst)
    else:
        w32(ctx, 0x1017e688, 0)
    w32(ctx, 0x1017e690, r.choice([0, 1, 2, 3]))
    w32(ctx, 0x1017f118, r.choice([0, 2, 4]))
    w32(ctx, 0x1017b8f8, ctx.u32())
    w32(ctx, 0x1014cefc, ctx.u32())
    obj = newbuf(ctx, 0x20)
    w32(ctx, obj, r.choice([0, 1]))
    ST['objs'] = [obj, obj, 0]
    return [src, VISIT]


def g_db259(ctx):
    r = begin(ctx)
    src = newbuf(ctx, 0x80)
    s = text(ctx, b'abcXYZ.\\', r.randrange(0, 0x50))
    wb(ctx, src + 0x24, s + b'\0')
    for a in range(0x109f1448, 0x109f1452, 2):
        ctx.m.uc.mem_write(a, struct.pack('<H', r.getrandbits(16)))
    w32(ctx, 0x1017b8f8, ctx.u32())
    wb(ctx, 0x00AE0000, rb(ctx, 0x200))
    return [src]


def spec(name, orig, gen, trials, **kw):
    return dict(name=name, orig=orig, c=name, gen=gen, trials=trials, **kw)


SPECS = [
    dict(name='Crt_AToI64', orig=TRAMP_Q, c='Test_100fd1c0', gen=g_atoi64, trials=800),
    spec('Crt_StrToXL', 0x100fd2b0, g_strtoxl, 2000),
    spec('Crt_ToUpper', 0x100fdb80, g_toupper, 1000),
    spec('Crt_QSort', 0x100fdcc0, g_qsort, 600, void=True),
    spec('Heap_MallocRetry', 0x100fef50, g_malloc, 600, api=HEAP_API),
    # superseded (C runtime replacement, formatting, DECISIONS.md): SDL3 formatter, checked natively against the original
    # spec('Crt_FPrintf', 0x100fd7e0, g_fprintf, 400, api=FLUSH_API),
    # superseded (C runtime replacement, formatting, DECISIONS.md): SDL3 formatter, checked natively against the original
    # spec('Crt_Printf', 0x100fdc80, g_printf, 300, api=FLUSH_API),
    spec('Crt_FPuts', 0x100fe390, g_fputs, 400, api=FLUSH_API),
    spec('Crt_FFlush', 0x100fe240, g_fflush, 800, api=FLUSH_API),
    spec('Crt_FlushStream', 0x100fe290, g_flush, 600),
    spec('Crt_FTell', 0x100fe970, g_ftell, 1500),
    spec('Crt_FindFirst', 0x100fe470, g_findfirst, 500, api=FIND_API),
    spec('Crt_FindNext', 0x100fe5a0, g_findnext, 500, api=FIND_API),
    spec('Sim_100c81ed', 0x100c81ed, g_c81ed, 800, void=True),
    spec('Sim_100c82e8', 0x100c82e8, g_c81ed, 800, void=True),
    spec('Sim_100ca996', 0x100ca996, g_ca996, 1500),
    spec('Sim_100cab7b', 0x100cab7b, g_cab7b, 1500),
    spec('Sim_100cb299', 0x100cb299, g_cb299, 2000),
    spec('Sim_100ccb0d', 0x100ccb0d, g_ccb0d, 800),
    spec('Sim_100d191d', 0x100d191d, g_d191d, 400, void=True),
    spec('Sim_100d1b91', 0x100d1b91, g_d1b91, 600, void=True),
    spec('Sim_100d650f', 0x100d650f, g_d650f, 800, void=True),
    spec('Sim_100d666d', 0x100d666d, g_d666d, 800, void=True),
    spec('Sim_100da14c', 0x100da14c, g_da14c, 1000),
    spec('Sim_100dafa8', 0x100dafa8, g_dafa8, 800, api=HEAP_API),
    spec('Sim_100db259', 0x100db259, g_db259, 400),
]

# round 31: the texture name table is ours now (w31_map.py: the mapping step around the C run)
exec(open('tools/verify/w31_map.py').read())
SOURCES = SOURCES + [x for x in W31_SOURCES if x not in SOURCES]
SPECS = w31_wrap(SPECS, {'Sim_100da14c'})
