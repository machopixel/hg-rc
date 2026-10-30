# Differential tests for round 7, letter r (see difftest.py and docs/checker.md): C runtime code of HGSIM.DLL
# around 0x100ff000..0x10104000 (case mapping, environment, heap resize / zeroed allocation, fatal exit,
# the E/F/G double layouts of the formatter, the math error exit and frexp).
#
# Library helpers that are not converted (and the converted ones' own unconverted helpers) are replaced in the
# emulator by a code hook at their first byte (code bytes are not patched): the hook appends
# ('stub_<address>', arguments, extra data) to the checker's call log, which must be identical for the original
# and the C run, writes the helper's out data where it has some, answers in eax with a value that depends only on
# the trial and the call position, and returns to the caller (EIP = return address, ESP + 4; cdecl). Stack
# addresses are logged as 'stack'. Function pointers in the DLL's data point at `ret` bytes with logging hooks.
# Everything else (the converted callees, 0x10103ad0, 0x10100be0/0x10100d20/0x10100f00 behind 0x10100ed0 and
# 0x10100f00, the small block heap world for the resize) runs for real on both sides.
#
# Math_ReportError and Math_FrExp return in st(0): the original and the C are both called through the same kind
# of machine-code trampoline, which stores st(0) as all 80 bits, the x87 stack top before and after popping it
# and the control word at OUTF (compared like all memory). No float tolerance is used anywhere.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW,
                               UC_X86_REG_FPSW, UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_100f/w7r_ctype.c', 'sim/unsorted_1010/w7r_env.c', 'sim/unsorted_1010/w7r_heap.c',
    'sim/unsorted_1010/w7r_fmt.c', 'sim/unsorted_1010/w7r_math.c',
    # existing converted code the functions above call
    'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_100f/w6i_mem.c', 'sim/unsorted_100f/w2h_mem.c',
    'sim/unsorted_1010/w2h_misc.c', 'sim/unsorted_1010/w2i_crt.c', 'sim/unsorted_1010/w2j_misc.c',
    'sim/unsorted_1010/w6i_crt.c', 'sim/unsorted_1010/w6j_double.c', 'sim/unsorted_1010/w6j_heap.c',
    'sim/unsorted_1010/w6k_sbh.c', 'sim/unsorted_1010/w6k_text.c', 'sim/unsorted_1010/w6k_misc.c',
    'sim/unsorted_1010/w2f_digits.c', 'sim/unsorted_1010/w2f_mantissa.c', 'sim/unsorted_1010/w2f_ld12.c',
    'sim/unsorted_100f/w2k_string.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000          # trampoline results
ARENA = 0x00A40000         # pages of the small block heap world (page aligned, away from ctx.buf)
TRAMP = 0x00F00800         # trampolines (original, C) for the st(0) functions, 0x80 bytes each
CB_NEWH = 0x00F00C00       # target of the new-handler pointer 0x1017e96c (Sim_10106590)
CB_EXIT = 0x00F00C10       # target of the exit pointer 0x101596fc
CB_MSG = 0x00F00C20        # target of the pointer 0x1017ebe8 (Sim_101083f0)

# helper address -> number of dword arguments
STUBS = {
    0x101053c0: 7,   # locale string mapping (locale, flags, src, srclen, dst, dstsize, codepage)
    0x10104fd0: 6,   # character type query used by Crt_IsCType (1, src, n, out types, 0, 0)
    0x10107910: 0,   # environment set-up used by Crt_GetEnv
    0x1010afa0: 7,   # locale compare used by Sim_101078d0
    0x101069c0: 1,   # small block allocator (paragraphs)
    0x100fef50: 2,   # allocator behind Heap_Malloc (size, mode)
    0x10108430: 1,   # message output (code)
}
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def rb(ctx, n):
    return ctx.rng.randbytes(n)


def logged(v):
    return 'stack' if STACK_LO <= v <= STACK_HI else v


def answer(addr, n):
    vals = ST['rets'].get(addr) or [0]
    return vals[n % len(vals)] & 0xffffffff


def count(m, tag):
    return sum(1 for e in m.api_log if e[0] == tag)


def _stub_cb(m, addr, nargs):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        ret = r32(uc, esp)
        args = [r32(uc, esp + 4 + 4 * k) for k in range(nargs)]
        tag = 'stub_%x' % addr
        n = count(m, tag)
        val = answer(addr, n)
        extra = ()
        if addr == 0x101053c0:       # log the source bytes, write min(answer, 3) bytes of mapped text
            extra = bytes(uc.mem_read(args[2], args[3] & 0xff))
            k = min(val, 3)
            if k:
                m.uc.mem_write(args[4], ST['mapped'][:k])
        elif addr == 0x10104fd0:     # log the source bytes, write the character type word when answering 1
            extra = bytes(uc.mem_read(args[1], args[2] & 0xff))
            if val:
                m.uc.mem_write(args[3], ST['ctype'])
        elif addr == 0x10107910 and val == 0 and ST.get('env_after') is not None:
            m.uc.mem_write(0x1017e8b0, struct.pack('<I', ST['env_after']))   # tracked, so the checker restores it
        m.api_log.append((tag, tuple(logged(a) for a in args), extra))
        uc.reg_write(UC_X86_REG_EAX, val)
        uc.reg_write(UC_X86_REG_EIP, ret)
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def _ptr_cb(m, tag, nargs):
    """Hook at a `ret` byte that a function pointer of the DLL points to."""
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        args = tuple(logged(r32(uc, esp + 4 + 4 * k)) for k in range(nargs))
        n = count(m, tag)
        m.api_log.append((tag, args))
        vals = ST['ptr'].get(tag) or [0]
        uc.reg_write(UC_X86_REG_EAX, vals[n] if n < len(vals) else 0)
    return cb


def tramp_st0(target, nargs):
    """Calls target with the trampoline's nargs dword arguments; stores st(0) (80 bits), the stack top before
    and after popping it and the control word at OUTF; returns 0 in eax."""
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])                       # push dword [ebp+8+4i]
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'         # mov eax, target; call eax
    code += b'\x83\xc4' + bytes([4 * nargs])                          # add esp, 4n
    code += b'\xdf\xe0\x25\x00\x38\x00\x00\xa3' + struct.pack('<I', OUTF + 12)   # fnstsw ax; and eax, 0x3800
    code += b'\xdb\x3d' + struct.pack('<I', OUTF)                     # fstp tword [OUTF]
    code += b'\xdf\xe0\x25\x00\x38\x00\x00\xa3' + struct.pack('<I', OUTF + 16)
    code += b'\xd9\x3d' + struct.pack('<I', OUTF + 20)                # fnstcw [OUTF+20]
    code += b'\x31\xc0\x5d\xc3'                                       # xor eax, eax; pop ebp; ret
    assert len(code) <= 0x80
    return code


TRAMPS = {}   # name -> (original trampoline address, C trampoline symbol)


def begin(ctx):
    """Installs the hooks once per machine and resets the FPU and the answers of this trial."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w7r', False):
        m._w7r = True
        for addr, nargs in STUBS.items():
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr, nargs), None, addr, addr)
        for a, tag, nargs in ((CB_NEWH, 'newhandler', 1), (CB_EXIT, 'exit', 1), (CB_MSG, 'msgcb', 0)):
            m.raw.mem_write(a, b'\xc3')
            m.raw.hook_add(UC_HOOK_CODE, _ptr_cb(m, tag, nargs), None, a, a)
        for i, (name, orig, nargs) in enumerate((('Math_ReportError', 0x10103720, 9), ('Math_FrExp', 0x101039a0, 3))):
            to, tc = TRAMP + 0x100 * i, TRAMP + 0x100 * i + 0x80
            m.raw.mem_write(to, tramp_st0(orig, nargs))
            m.raw.mem_write(tc, tramp_st0(m.syms[name], nargs))
            m.syms['T_' + name] = tc
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['rets'] = {}
    ST['ptr'] = {}
    ST['env_after'] = None
    ST['mapped'] = rb(ctx, 3)
    ST['ctype'] = rb(ctx, 2)
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def mix_api(seq):
    def h(n, args):
        return seq[n % len(seq)]
    return h


# ---- Crt_ToLower --------------------------------------------------------------------------------
def g_tolower(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017e950, r.choice([0, 0, 1, 0x409, 0x411]))
    w32(ctx, 0x1015965c, r.choice([0, 1, 1, 2, 3, 0xffffffff]))
    tab = ctx.buf(0x600, bytes(r.choice([0, 1, 1, 0x80, 0x81, 0x82, 0x40, r.getrandbits(8)]) for _ in range(0x600)))
    w32(ctx, 0x10159450, tab + 0x200)
    rets(0x101053c0, r.choice([0, 1, 1, 2, 2, 3]))
    rets(0x10104fd0, r.choice([0, 1, 1]))
    c = r.choice([r.randrange(0x41, 0x5b), r.randrange(0x41, 0x5b), r.randrange(0, 0x100), r.randrange(-0x100, 0),
                  -1, 0x5b, 0x40, 0x60, 0x7b, r.randrange(0x100, 0x10000), r.randrange(0x8100, 0x9fff),
                  ctx.u32() if r.random() < 0.3 else 0x61])
    return [c & 0xffffffff]


# ---- Crt_GetEnv ---------------------------------------------------------------------------------
def env_table(ctx, names):
    r = ctx.rng
    ptrs = []
    for nm in names:
        text = nm + (b'=' if r.random() < 0.85 else r.choice([b'', b'x', b'_'])) + bytes(r.choice(b'abc01=') for _ in range(r.randrange(0, 6)))
        ptrs.append(ctx.buf(len(text) + 1, text + b'\0'))
    return ctx.buf(4 * len(ptrs) + 4, struct.pack('<%dI' % (len(ptrs) + 1), *ptrs, 0))


def g_getenv(ctx):
    r = begin(ctx)
    pool = [b'PATH', b'Path', b'HOME', b'TMP', b'TEMP', b'A', b'AB', b'ABC', b'X', b'']
    names = [r.choice(pool) for _ in range(r.randrange(0, 7))]
    table = env_table(ctx, names)
    k = r.random()
    w32(ctx, 0x1017e8b0, table if k < 0.7 else 0)
    w32(ctx, 0x1017e8b8, r.choice([0, 1, 1]))
    ST['env_after'] = r.choice([table, table, 0])
    rets(0x10107910, r.choice([0, 0, 1, 0xffffffff]))
    rets(0x1010afa0, *[r.choice([2, 2, 1, 3, 0]) for _ in range(4)])
    w32(ctx, 0x1017ebc8, ctx.u32())
    w32(ctx, 0x1017ebc4, ctx.u32())
    nm = r.choice(pool + [b'PATHX', b'PAT'])
    name = 0 if r.random() < 0.05 else ctx.buf(len(nm) + 1, nm + b'\0')
    return [name]


# ---- Sim_101009b0 -------------------------------------------------------------------------------
def g_amsg(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017e8fc, r.choice([0, 0, 1, 1, 2, 3]))
    w32(ctx, 0x1017e900, r.choice([0, 1, 1, 2]))
    w32(ctx, 0x1017ebe8, r.choice([0, CB_MSG]))
    w32(ctx, 0x101596fc, CB_EXIT)
    ST['ptr']['exit'] = [r.choice([0, 1, 0xff, ctx.u32()])]
    return [r.choice([0, 1, 2, 8, 0x10, 0x18, 0xfc, 0xff, ctx.u32()])]


# ---- heap ---------------------------------------------------------------------------------------
def heap_common(ctx):
    r = ctx.rng
    limit = r.choice([0x1e0, 0x1e0, 0x3f0, 0x100, 0x10, 0, 0xffffffff, r.randrange(0, 0x800)])
    w32(ctx, 0x1015be5c, limit)
    w32(ctx, 0x1017e968, r.choice([0, 0, 1, 2]))
    w32(ctx, 0x1017e96c, r.choice([CB_NEWH, CB_NEWH, 0]))
    w32(ctx, 0x10ac3714, ctx.u32())
    ST['ptr']['newhandler'] = [r.choice([0, 1, 1, 0xffffffff]), r.choice([0, 0, 1])]
    return limit


def sbh_world(ctx):
    """The static region record 0x10159e38 (or a second record linked to it) covers four pages at ARENA: dword +0
    of a page is its search pointer, +4 the free run there, the bytes +8..+0xf8 the paragraph map of the blocks
    at +0x100.."""
    r = ctx.rng
    head = 0x10159e38
    lo, hi = ARENA - r.choice([0, 1, 0x10]), ARENA + 0x4000
    second = ctx.buf(0x60)
    if r.random() < 0.6:
        w32(ctx, head + 0x10, lo, hi)
        node = head
        w32(ctx, head, r.choice([head, second]))
        w32(ctx, second + 0x10, 0, 0)
    else:
        w32(ctx, head + 0x10, 0, 0)
        w32(ctx, head, second)
        w32(ctx, second + 0x10, lo, hi)
        node = second
    w32(ctx, second, head)
    for k in range(4):     # page entries of the record: free paragraph count, flag
        w32(ctx, node + 0x18 + 8 * k, r.choice([0, 1, 0x10, 0xef, 0xf0, r.randrange(0, 0xf0)]),
            r.choice([0, 0xf1, ctx.u32()]))
    w32(ctx, 0x1017e970, r.choice([0, 1, 0x1f, 0x1f, 5]))
    for k in range(4):
        page = ARENA + 0x1000 * k
        data = bytearray(rb(ctx, 0x1000))
        mp = bytearray(0xf0)
        i = 0
        while i < 0xf0:
            if r.random() < 0.45:
                n = r.choice([1, 1, 2, 3, 4, 8, r.randrange(1, 0x20)])
                mp[i] = n
                i += r.choice([1, n, n])
            else:
                i += r.randrange(1, 6)
        data[8:0xf8] = mp
        cur = page + 8 + r.randrange(0, 0xf0)
        data[0:8] = struct.pack('<2I', cur, r.randrange(0, 0x20))
        wb(ctx, page, data)
    return node


def g_realloc(ctx):
    r = begin(ctx)
    limit = heap_common(ctx)
    sbh_world(ctx)
    dst1 = ctx.buf(0x1000)
    dst2 = ctx.buf(0x1000)
    other = ctx.buf(0x40)
    rets(0x101069c0, r.choice([0, dst1, dst1]))
    rets(0x100fef50, r.choice([0, dst2, 0x12345678]))
    ST['api'] = {'HeapAlloc': [r.choice([0, dst2, dst2]), r.choice([0, dst2])],
                 'HeapReAlloc': [r.choice([0, other, 0x00C00000]), r.choice([0, other])],
                 'HeapFree': [r.choice([0, 1])]}
    page = ARENA + 0x1000 * r.randrange(0, 4)
    i = r.randrange(0, 0xf0)
    k = r.random()
    if k < 0.75:
        block = page + 0x100 + 16 * i
    elif k < 0.85:
        block = other
    elif k < 0.9:
        block = 0
    else:
        block = r.choice([page + 0x108, page + 0x40, ARENA + 0x5000, ctx.u32()])
    size = r.choice([0, 1, 15, 16, 17, 0x20, 0x31, r.randrange(1, 0x100), r.randrange(1, 0x100), r.randrange(1, 0x400),
                     limit - 1, limit, limit + 1, 0xffffffe0, 0xffffffe1, 0xffffffff, r.getrandbits(32)])
    return [block & 0xffffffff, size & 0xffffffff]


def g_calloc(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015be5c, r.choice([0x1e0, 0x1e0, 0x3f0, 0x100, 0x10, 0, r.randrange(0, 0x800)]))
    w32(ctx, 0x1017e968, r.choice([0, 0, 1, 2]))
    w32(ctx, 0x1017e96c, r.choice([CB_NEWH, CB_NEWH, 0]))
    w32(ctx, 0x10ac3714, ctx.u32())
    ST['ptr']['newhandler'] = [r.choice([0, 1, 1, 0xffffffff]), r.choice([0, 0, 1])]
    dst1 = ctx.buf(0x800)
    rets(0x101069c0, r.choice([0, dst1, dst1]), r.choice([0, dst1]))
    ST['api'] = {'HeapAlloc': [r.choice([0, 0x00C00000, 0x12345678]), r.choice([0, 0x00C00000])]}
    count = r.choice([0, 1, 2, 3, 7, 16, r.randrange(0, 0x40), 0x10000, 0x80000000, 0xffffffff, r.getrandbits(32)])
    size = r.choice([0, 1, 2, 4, 12, 16, r.randrange(0, 0x40), 0x10000, 0x80000000, 0xffffffff, r.getrandbits(32)])
    return [count, size]


def api_seq(name):
    def h(n, args):
        seq = ST.get('api', {}).get(name) or [1]
        return seq[n % len(seq)]
    return h


HEAP_API = {name: api_seq(name) for name in ('HeapAlloc', 'HeapReAlloc', 'HeapFree', 'VirtualFree')}


# ---- formatter ----------------------------------------------------------------------------------
def rand_double(ctx):
    r = ctx.rng
    k = r.random()
    if k < 0.15:
        v = r.choice([0.0, -0.0, 1.0, -1.0, 0.5, 9.5, 99.999, 1e-5, 0.0001, 123456789.0, 1e21, 1e-300, 5e-324,
                      9.9999999, 0.09999999, 1e100, -2.5e-7])
        return struct.unpack('<2I', struct.pack('<d', v))
    if k < 0.2:
        return (r.choice([0, 1]), r.choice([0x7ff00000, 0xfff00000, 0x7ff80000, 0x7ff00001]))
    if k < 0.75:
        v = r.uniform(-1, 1) * 10.0 ** r.randrange(-12, 25)
        return struct.unpack('<2I', struct.pack('<d', v))
    return (r.getrandbits(32), r.getrandbits(32))


def fmt_reuse(ctx, buf):
    """Leaves a digit record and a digit text like Fmt_DoubleG does, and sets the reuse byte or not."""
    r = ctx.rng
    digits = bytes(r.choice(b'0123456789') for _ in range(r.randrange(1, 18)))
    if r.random() < 0.2:
        digits = b'0'
    dp = ctx.buf(len(digits) + 1, digits + b'\0')
    sign = r.choice([0x2d, 0x20, 0, 0x2d])
    decpt = r.choice([0, 1, 2, 3, -1, -2, -5, 10, 100, 120, -100, r.randrange(-400, 400)])
    rec = ctx.buf(0x20, struct.pack('<4I', sign, decpt & 0xffffffff, r.choice([0, 1]), dp))
    w32(ctx, 0x1017e904, rec)
    w32(ctx, 0x1017e90c, r.choice([decpt - 1, decpt - 1, r.randrange(-5, 20)]) & 0xffffffff)
    text = (b'-' if sign == 0x2d else b'') + digits + b'\0'
    wb(ctx, buf, text)
    wb(ctx, 0x1017e908, [r.choice([0, 0, 1, 1, 2])])


def fmt_args(ctx):
    r = ctx.rng
    lo, hi = rand_double(ctx)
    value = ctx.buf(8, struct.pack('<2I', lo, hi))
    area = ctx.buf(0x200, rb(ctx, 0x200))
    buf = area + 0x20
    fmt_reuse(ctx, buf)
    if r.random() < 0.3:
        wb(ctx, 0x10159660, [r.choice([0x2e, 0x2c])])
    ndig = r.choice([0, 0, 1, 2, 3, 4, 5, 6, 6, 8, 10, 12, 15, 17, 20, 30, r.randrange(0, 40), -1])
    return value, buf, ndig


def g_fmt_e(ctx):
    r = begin(ctx)
    value, buf, ndig = fmt_args(ctx)
    return [value, buf, ndig & 0xffffffff, r.choice([0, 1, 0, 0x100])]


def g_fmt_f(ctx):
    begin(ctx)
    value, buf, ndig = fmt_args(ctx)
    return [value, buf, ndig & 0xffffffff]


def g_fmt_g(ctx):
    r = begin(ctx)
    value, buf, ndig = fmt_args(ctx)
    if r.random() < 0.5:
        ndig = r.choice([1, 2, 3, 4, 5, 6, 6, 7, 10, 15, 17])
    return [value, buf, ndig & 0xffffffff, r.choice([0, 1, 0, 0x100])]


# ---- math ---------------------------------------------------------------------------------------
def special_double(ctx):
    r = ctx.rng
    k = r.random()
    if k < 0.25:
        return r.choice([(0, 0), (0, 0x80000000), (0, 0x7ff00000), (0, 0xfff00000), (0, 0x7ff80000), (1, 0x7ff00000),
                         (0, 0x7ff40000), (0xffffffff, 0xffffffff), (1, 0), (0, 1), (0xffffffff, 0x800fffff),
                         (0, 0x00100000), (0xffffffff, 0x000fffff), (0, 0x3ff00000), (0, 0xbfe00000)])
    if k < 0.45:     # denormals with mantissas of every length
        bits = r.randrange(1, 53)
        m = r.getrandbits(bits) | (1 << (bits - 1))
        return (m & 0xffffffff, (r.getrandbits(1) << 31) | (m >> 32))
    if k < 0.6:
        return (r.getrandbits(32), (r.getrandbits(1) << 31) | (r.choice([0x7ff, 0x7fe, 0x001, 0x3ff, 0x400]) << 20)
                | r.getrandbits(20))
    return (r.getrandbits(32), r.getrandbits(32))


def g_report(ctx):
    r = begin(ctx)
    keys = [r32(ctx.m.uc, 0x101598b0 + 8 * i) for i in range((0x10159988 - 0x101598b0) // 8)]
    op = r.choice(keys + keys + [0, 0xffffffff, ctx.u32()])
    w32(ctx, 0x1017e888, ctx.u32())
    a1, a2, rv = special_double(ctx), special_double(ctx), special_double(ctx)
    cw = r.choice([0x27f, 0x27f, 0x37f, 0x7f, 0x127f, 0x1372, 0xc7f, 0x3f, r.getrandbits(16) | 0x3f, ctx.u32()])
    return [r.choice([1, 2, 3, 4, 5, 6, 0, -1 & 0xffffffff, ctx.u32()]), op, *a1, *a2, *rv, cw]


def g_frexp(ctx):
    begin(ctx)
    lo, hi = special_double(ctx)
    return [lo, hi, ctx.buf(4)]


SPECS = [
    dict(name='Crt_ToLower', orig=0x100ff280, c='Crt_ToLower', gen=g_tolower, trials=2000),
    dict(name='Crt_GetEnv', orig=0x10100130, c='Crt_GetEnv', gen=g_getenv, trials=1500),
    dict(name='Sim_101009b0', orig=0x101009b0, c='Sim_101009b0', gen=g_amsg, trials=400),
    dict(name='Heap_Realloc', orig=0x101003e0, c='Heap_Realloc', gen=g_realloc, trials=3000, api=HEAP_API),
    dict(name='Heap_Calloc', orig=0x10100580, c='Heap_Calloc', gen=g_calloc, trials=1500, api=HEAP_API),
    dict(name='Fmt_DoubleE', orig=0x10100be0, c='Fmt_DoubleE', gen=g_fmt_e, trials=2500),
    dict(name='Fmt_DoubleF', orig=0x10100d20, c='Fmt_DoubleF', gen=g_fmt_f, trials=2500),
    dict(name='Fmt_DoubleG', orig=0x10100e20, c='Fmt_DoubleG', gen=g_fmt_g, trials=2500),
    dict(name='Math_ReportError', orig=TRAMP, c='T_Math_ReportError', gen=g_report, trials=1500),
    dict(name='Math_FrExp', orig=TRAMP + 0x100, c='T_Math_FrExp', gen=g_frexp, trials=3000),
]
