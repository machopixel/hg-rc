# Differential tests for round 8, letter A (see difftest.py and docs/checker.md): C runtime code of HGSIM.DLL
# (src/sim/unsorted_100f/w8A_*.c and src/sim/unsorted_1010/w8A_*.c): low-level file handles, streams, text
# helpers, time, environment, abort and the floating-point error path.
#
# Helpers are replaced by emulator hooks on their first instruction (code bytes are not patched) only in the
# trials that ask for it (ST['active']): the hook appends ('stub_<address>', arguments) to the checker's call log
# (compared between the original and the C run; stack addresses shown as 'stack'), may log the bytes behind a
# pointer argument and write out data, sets eax to an answer chosen by the trial and returns to the caller.
# Converted helpers that are stubbed get the same hook on the original address AND on their C symbol, so both
# runs see the same answer. Every other helper runs for real on both sides (the original for the original, the
# converted C for the C).
# Windows functions that fill or read buffers (ReadFile, WriteFile, GetLocalTime, GetSystemTime,
# GetTimeZoneInformation, RaiseException) are answered by handlers that read the raw pointer arguments from the
# stack: data passed to Windows is logged as bytes, data Windows returns is written with the same bytes for
# both runs.
# Math_DomainError returns in st(0): the original and the C are called through the same kind of machine-code
# trampoline, which stores st(0) as all 80 bits, the x87 stack top before and after popping it and the control
# word at OUTF (compared like all memory). No float tolerance is used anywhere.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW,
                               UC_X86_REG_FPSW, UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_100f/w8A_stream.c', 'sim/unsorted_100f/w8A_misc.c',
    'sim/unsorted_1010/w8A_lowio.c', 'sim/unsorted_1010/w8A_text.c', 'sim/unsorted_1010/w8A_time.c',
    'sim/unsorted_1010/w8A_env.c', 'sim/unsorted_1010/w8A_math.c',
    # existing converted code the functions above call, and what those files need
    'sim/unsorted_1010/w7s_stdio.c', 'sim/unsorted_1010/w7s_heap.c', 'sim/unsorted_1010/w7s_locale.c',
    'sim/unsorted_1010/w7s_env.c', 'sim/unsorted_1010/w7s_math.c', 'sim/unsorted_1010/w7s_misc.c',
    'sim/unsorted_1010/w6j_stream.c', 'sim/unsorted_1010/w6k_stream.c', 'sim/unsorted_1010/w6k_sbh.c',
    'sim/unsorted_1010/w6k_text.c', 'sim/unsorted_1010/w6k_misc.c', 'sim/unsorted_1010/w6k_handles.c',
    'sim/unsorted_1010/w2h_misc.c', 'sim/unsorted_1010/w2i_crt.c', 'sim/unsorted_1010/w2g_crt.c',
    'sim/unsorted_1010/w2k_crt.c', 'sim/unsorted_1010/w2j_misc.c', 'sim/unsorted_1010/w2f_misc.c',
    'sim/unsorted_1010/w2f_mantissa.c', 'sim/unsorted_1010/w2f_ld12.c', 'sim/unsorted_1010/w2f_digits.c',
    'sim/unsorted_1010/w6i_crt.c', 'sim/unsorted_1010/w6j_double.c', 'sim/unsorted_1010/w6j_heap.c',
    'sim/unsorted_1010/w7r_env.c', 'sim/unsorted_1010/w7r_heap.c', 'sim/unsorted_1010/w7r_fmt.c',
    'sim/unsorted_1010/w7r_math.c',
    'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_100f/w6i_mem.c',
    'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_100f/w2h_mem.c', 'sim/unsorted_100f/w6i_exit.c',
    'sim/unsorted_100f/w7r_ctype.c', 'sim/unsorted_100f/w7q_stream.c', 'sim/unsorted_100f/w6i_stream.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000          # trampoline results
TRAMP = 0x00F00800         # trampolines (original, C) for the st(0) function, 0x80 bytes each

# helper address -> number of dword arguments
STUBS = {
    0x100fef30: 1,   # Heap_Malloc
    0x100fefe0: 1,   # Heap_FreeBlock
    0x101003e0: 2,   # Heap_Realloc
    0x1010b530: 1,   # Sim_1010b530 (copy of an environment array)
    0x10107910: 0,   # Sim_10107910 (16-bit environment converted)
    0x10100130: 1,   # Crt_GetEnv
    0x1010c4b0: 3,   # Sim_1010c4b0 (16-bit text to multibyte)
    0x10100fd0: 3,   # the printf formatter
    0x10105970: 7,   # local SYSTEMTIME fields to seconds
    0x10108430: 1,   # run-time message
    0x10107670: 1,   # raise
    0x100ff3d0: 1,   # Proc_ExitQuick
    0x100fd9a0: 2,   # Crt_IsCType
    0x10100d20: 3,   # Fmt_DoubleF
    0x10109dd0: 0,   # Sim_10109dd0 (time zone set-up once)
    0x1010a0a0: 1,   # Sim_1010a0a0 (daylight saving test)
    0x10105b50: 3,   # Crt_Read
    0x10105750: 3,   # Crt_Write
    0x10105a60: 1,   # Sim_10105a60 (refill)
    0x100fe290: 1,   # Crt_FlushStream
    0x100ff030: 2,   # Crt_FlushBuf
    0x100fe970: 1,   # Crt_FTell
    0x10105fe0: 3,   # Crt_LSeek
    0x10107080: 1,   # Sim_10107080 (is a device)
    0x10107020: 1,   # Sim_10107020 (gives the stream a buffer)
    0x1010afa0: 7,   # locale compare (CompareString wrapper) behind the name search of Sim_1010b4b0
}
SYM_STUBS = {'Heap_Malloc': 0x100fef30, 'Heap_FreeBlock': 0x100fefe0, 'Heap_Realloc': 0x101003e0,
             'Sim_1010b530': 0x1010b530, 'Sim_10107910': 0x10107910, 'Crt_GetEnv': 0x10100130,
             'Sim_1010c4b0': 0x1010c4b0, 'Sim_10108430': 0x10108430, 'Proc_ExitQuick': 0x100ff3d0,
             'Crt_IsCType': 0x100fd9a0, 'Fmt_DoubleF': 0x10100d20, 'Sim_10109dd0': 0x10109dd0,
             'Sim_1010a0a0': 0x1010a0a0, 'Crt_Read': 0x10105b50, 'Crt_Write': 0x10105750,
             'Sim_10105a60': 0x10105a60, 'Crt_FlushStream': 0x100fe290, 'Crt_FlushBuf': 0x100ff030,
             'Crt_FTell': 0x100fe970, 'Crt_LSeek': 0x10105fe0, 'Sim_10107080': 0x10107080,
             'Sim_10107020': 0x10107020}
ST = {'active': set(), 'fn': {}, 'api': {}, 'seed': 0}

HANDLES = 0x10ac3720
NHANDLES = 0x10ac385c


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


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def cstr(ctx, data, extra=16):
    return ctx.buf(len(data) + 1 + extra, bytes(data) + b'\0' + rb(ctx, extra))


def mix(*vals):
    x = 0x9e3779b9
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def count_calls(m, name):
    return sum(1 for e in m.api_log if e[0] == name) - 1


def read_cstr(uc, addr, maxn=300):
    try:
        out = b''
        while len(out) < maxn:
            c = bytes(uc.mem_read(addr + len(out), 1))
            if c == b'\0':
                break
            out += c
        return out
    except Exception:
        return 'unreadable'


def read_bytes(uc, addr, n):
    try:
        return bytes(uc.mem_read(addr, n))
    except Exception:
        return 'unreadable'


def stack_arg(uc, i):
    """Raw dword argument i of a stub (the hook runs at the stub's first instruction)."""
    return r32(uc, uc.reg_read(UC_X86_REG_ESP) + 4 + 4 * i)


def log(entry):
    ST['m'].api_log.append(entry)


def mw(addr, data):
    """Memory write from a hook through the tracked object, so the checker puts the page back before the C run."""
    ST['m'].uc.mem_write(addr, data)


# ---- hooks ---------------------------------------------------------------------------------------
def install(m):
    if getattr(m, '_w8A', False):
        return
    m._w8A = True

    def make(key):
        def cb(uc, address, size, ud):
            if key not in ST['active']:
                return
            esp = uc.reg_read(UC_X86_REG_ESP)
            k = STUBS[key]
            raw = list(struct.unpack('<%dI' % k, bytes(uc.mem_read(esp + 4, 4 * k)))) if k else []
            args = tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)
            name = 'stub_%x' % key
            m.api_log.append((name, args))
            n = count_calls(m, name)
            f = ST['fn'].get(key)
            val = f(n, raw, uc) if f else mix(ST['seed'], key, n)
            uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
            # return to the caller (cdecl: the caller removes the arguments)
            uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
            uc.reg_write(UC_X86_REG_ESP, esp + 4)
        return cb

    for a in STUBS:
        m.raw.hook_add(UC_HOOK_CODE, make(a), None, a, a)
    for sym, a in SYM_STUBS.items():
        addr = m.syms[sym]
        m.raw.hook_add(UC_HOOK_CODE, make(a), None, addr, addr)
    def fpu_reset(uc, address, size, ud):
        # the x87 state each run starts from (the original's run changes it and registers are not put back)
        if 'fpsw' in ST:
            uc.reg_write(UC_X86_REG_FPCW, ST['fpcw'])
            uc.reg_write(UC_X86_REG_FPSW, ST['fpsw'])
            uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    for a in (0x101030b0, m.syms['Math_RaiseFpError'], TRAMP, TRAMP + 0x80):
        m.raw.hook_add(UC_HOOK_CODE, fpu_reset, None, a, a)
    for i, (name, orig, nargs) in enumerate((('Math_DomainError', 0x10102e30, 4),)):
        to, tc = TRAMP + 0x100 * i, TRAMP + 0x100 * i + 0x80
        m.raw.mem_write(to, tramp_st0(orig, nargs))
        m.raw.mem_write(tc, tramp_st0(m.syms[name], nargs))
        m.syms['T_' + name] = tc


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


def begin(ctx, *active):
    install(ctx.m)
    ST.clear()
    ST['active'] = set(active)
    ST['fn'] = {}
    ST['api'] = {}
    ST['seed'] = ctx.rng.getrandbits(32)
    ST['m'] = ctx.m
    ctx.m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    ctx.m.uc.reg_write(UC_X86_REG_FPSW, 0)
    ctx.m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    return ctx.rng


def answers(addr, vals):
    ST['fn'][addr] = lambda n, raw, uc: vals[n % len(vals)]


def malloc_pool(ctx, size, p_zero=0.15, k=4):
    blocks = [0 if ctx.rng.random() < p_zero else zbuf(ctx, size) for _ in range(k)]
    answers(0x100fef30, blocks)
    return blocks


def _api(name):
    def h(n, args, uc):
        v = ST.get('api', {}).get(name)
        if v is None:
            return 1
        k = count_calls(ST['m'], name)
        if callable(v):
            return v(k, args, uc)
        return v[k % len(v)]
    return h


API = {name: _api(name) for name in [
    'GetLastError', 'SetFilePointer', 'WriteFile', 'ReadFile', 'CloseHandle', 'SetEndOfFile', 'DeleteFileA',
    'SetStdHandle', 'GetLocalTime', 'GetSystemTime', 'GetTimeZoneInformation', 'SetEnvironmentVariableA',
    'RaiseException']}


def seq(r, choices, k=4):
    return [r.choice(choices) for _ in range(k)]


# ---- handle table --------------------------------------------------------------------------------
def handle_table(ctx, flag_choices, nblk=None, fill=None):
    """Blocks of 32 slots (+0 Windows handle, +4 flag byte, +5 read-ahead byte); returns the block addresses."""
    r = ctx.rng
    if nblk is None:
        nblk = r.choice([1, 1, 2])
    blocks = []
    for i in range(64):
        if i < nblk:
            data = b''.join(struct.pack('<IBBH', fill(i * 32 + j) if fill else r.choice([0x100 + i * 32 + j,
                                                                                        0xffffffff, ctx.u32()]),
                                        r.choice(flag_choices), r.choice([0x0a, 0x0a, 0x41, 0x0d, 0x1a]), 0)
                            for j in range(32))
            b = ctx.buf(0x100, data)
            blocks.append(b)
            w32(ctx, HANDLES + 4 * i, b)
        else:
            w32(ctx, HANDLES + 4 * i, 0)
    w32(ctx, NHANDLES, r.choice([nblk * 32, nblk * 32, nblk * 32, max(nblk * 32 - 7, 0)]))
    return blocks


def pick_fh(ctx, nblk):
    r = ctx.rng
    return r.choice([r.randrange(0, nblk * 32), r.randrange(0, nblk * 32), r.randrange(0, nblk * 32), 0, 1, 2,
                     nblk * 32, 0xffffffff, ctx.u32()])


def set_slot_flag(ctx, blocks, fh, flag):
    if fh < 32 * len(blocks):
        wb(ctx, blocks[fh >> 5] + 8 * (fh & 31) + 4, [flag])


ERRS = [5, 6, 0x6d, 2, 0x13, 0xbc, 0x26, 0, 0x50]


# ---- low-level handle functions -------------------------------------------------------------------
def g_lseek(ctx):
    r = begin(ctx)
    blocks = handle_table(ctx, [0x01, 0x01, 0x03, 0x83, 0x41, 0x00, 0x22])
    fh = pick_fh(ctx, len(blocks))
    ST['api']['SetFilePointer'] = [r.choice([0, 5, 0x1000, 0xffffffff, 0xffffffff, 0x7fffffff])]
    ST['api']['GetLastError'] = [r.choice(ERRS)]
    return [fh, r.choice([0, 1, -1, 0x100, -0x100, ctx.u32()]) & 0xffffffff, r.choice([0, 1, 2, 3])]


def _write_handler(r):
    oks = seq(r, [1, 1, 1, 1, 0], 4)
    kinds = seq(r, ['all', 'all', 'all', 'half', 'zero'], 4)

    def h(k, args, uc):
        p, n, pdone = stack_arg(uc, 1), stack_arg(uc, 2), stack_arg(uc, 3)
        log(('WriteFile_data', read_bytes(uc, p, min(n, 0x2000))))
        ok = oks[k % 4]
        if ok:
            kind = kinds[k % 4]
            done = n if kind == 'all' else (n // 2 if kind == 'half' else 0)
            uc.mem_write(pdone, struct.pack('<I', done))
        return ok
    return h


def g_write(ctx):
    r = begin(ctx)
    blocks = handle_table(ctx, [0x01, 0x81, 0x81, 0x21, 0xa1, 0xc1, 0x41, 0x00])
    fh = pick_fh(ctx, len(blocks))
    if r.random() < 0.7 and fh < 32 * len(blocks):
        set_slot_flag(ctx, blocks, fh, r.choice([0x81, 0x81, 0xc1, 0x01, 0x41, 0xa1]))
    cnt = r.choice([0, 1, 2, 5, 16, 0x100, 0x3ff, 0x400, 0x401, 0x500, 0x900, r.randrange(1, 0xc00)])
    alphabet = r.choice([b'ab\n', b'\n', b'xyz', b'\x1a\nq', b'\n\n\na'])
    data = bytes(r.choice(alphabet) for _ in range(cnt))
    buf = ctx.buf(cnt + 16, data + rb(ctx, 16))
    ST['api']['WriteFile'] = _write_handler(r)
    ST['api']['GetLastError'] = [r.choice(ERRS)]
    ST['api']['SetFilePointer'] = [r.choice([0, 0x200, 0xffffffff])]
    return [fh, buf, cnt]


def _read_handler(r, ctx, chunks):
    oks = seq(r, [1, 1, 1, 1, 1, 0], 6)

    def h(k, args, uc):
        p, n, pdone = stack_arg(uc, 1), stack_arg(uc, 2), stack_arg(uc, 3)
        ok = oks[k % 6]
        if ok:
            data = chunks[k % len(chunks)][:n]
            uc.mem_write(p, data)
            uc.mem_write(pdone, struct.pack('<I', len(data)))
        return ok
    return h


def g_read(ctx):
    r = begin(ctx)
    blocks = handle_table(ctx, [0x01, 0x81, 0x81, 0x89, 0xc1, 0xc9, 0x41, 0x09, 0x03, 0x00])
    fh = pick_fh(ctx, len(blocks))
    if r.random() < 0.7 and fh < 32 * len(blocks):
        set_slot_flag(ctx, blocks, fh, r.choice([0x81, 0x81, 0x89, 0xc1, 0x01, 0x09, 0xc9]))
    cnt = r.choice([0, 1, 2, 3, 8, 16, 40, 0x100, r.randrange(1, 0x80)])
    alphabet = r.choice([b'ab\r\n', b'\r', b'\r\n', b'x\r\x1a\n', b'\n\r\ra', b'abc'])
    chunks = []
    for _ in range(3):
        k = r.choice([0, 1, 2, cnt, cnt, max(cnt - 1, 0), r.randrange(0, cnt + 1)])
        d = bytes(r.choice(alphabet) for _ in range(k))
        if d and r.random() < 0.4:
            d = d[:-1] + b'\r'
        chunks.append(d)
    peek = [bytes([r.choice([0x0a, 0x0a, 0x41, 0x0d])])]
    if r.random() < 0.2:
        peek = [b'']
    ST['api']['ReadFile'] = _read_handler(r, ctx, chunks[:1] + peek + chunks[1:])
    ST['api']['GetLastError'] = [r.choice([5, 0x6d, 6, 0x26, 0])]
    ST['api']['SetFilePointer'] = [r.choice([0, 0x10, 0xffffffff])]
    buf = newbuf(ctx, cnt + 16)
    return [fh, buf, cnt]


def g_close(ctx):
    r = begin(ctx)
    same = r.random() < 0.4
    blocks = handle_table(ctx, [0x01, 0x01, 0x81, 0x00],
                          fill=(lambda i: 0x44 if (same and i in (1, 2)) else r.choice([0x100 + i, 0xffffffff, 0x100 + i])))
    fh = r.choice([0, 1, 2, 1, 2, pick_fh(ctx, len(blocks))])
    w32(ctx, 0x1017e900, r.choice([0, 1, 1]))
    ST['api']['CloseHandle'] = [r.choice([0, 1, 1])]
    ST['api']['GetLastError'] = [r.choice(ERRS)]
    return [fh]


def g_chsize(ctx):
    r = begin(ctx)
    blocks = handle_table(ctx, [0x01, 0x81, 0x41, 0x00])
    fh = pick_fh(ctx, len(blocks))
    if r.random() < 0.8 and fh < 32 * len(blocks):
        set_slot_flag(ctx, blocks, fh, r.choice([0x01, 0x81, 0xa1]))
    end = r.choice([0, 0x10, 0x1000, 0x2345])
    pos = r.choice([0, 5, end])
    ST['api']['SetFilePointer'] = [r.choice([pos, pos, pos, 0xffffffff]), r.choice([end, end, end, 0xffffffff])]
    ST['api']['WriteFile'] = _write_handler(r)
    ST['api']['SetEndOfFile'] = [r.choice([0, 1, 1])]
    ST['api']['GetLastError'] = [r.choice([5, 5, 6, 0x70])]
    size = r.choice([end, end + 1, end + 0x1000, end + 0x2001, end - 1, 0, r.randrange(0, 0x5000)])
    return [fh, size & 0xffffffff]


def g_remove(ctx):
    r = begin(ctx)
    ST['api']['DeleteFileA'] = [r.choice([0, 1])]
    ST['api']['GetLastError'] = [r.choice(ERRS)]
    return [cstr(ctx, b'C:\\HG\\SAVE.TMP')]


# ---- streams -------------------------------------------------------------------------------------
STREAM_FLAGS = [0x01, 0x02, 0x09, 0x0a, 0x81, 0x82, 0x101, 0x102, 0x108, 0x10a, 0x11, 0x19, 0x401, 0x409, 0x40, 0x42,
                0x00, 0x80, 0x03, 0x83, 0x109, 0x0b]


def stream(ctx, flags=None):
    r = ctx.rng
    s = newbuf(ctx, 0x20)
    base = newbuf(ctx, 0x200)
    w32(ctx, s + 8, base)
    w32(ctx, s, base + r.choice([0, 0, 1, 5, 0x40]))
    w32(ctx, s + 4, r.choice([0, 0, 1, 3, 0x10, 0x100, ctx.u32() & 0xff]))
    w32(ctx, s + 0xc, (flags if flags is not None else r.choice(STREAM_FLAGS)) | (ctx.u32() & r.choice([0, 0, 0xffff0000])))
    w32(ctx, s + 0x10, r.choice([3, 4, 0xffffffff, r.randrange(0, 64)]))
    w32(ctx, s + 0x18, r.choice([0x200, 0x1000, 0x10, 1, 0, 0xffffffff]))
    return s


def g_fread(ctx):
    r = begin(ctx, 0x10105b50, 0x10105a60)
    s = stream(ctx)
    size = r.choice([1, 1, 2, 4, 3, 0x10, 0])
    count = r.choice([0, 1, 2, 5, 0x10, 0x100, 0x401, r.randrange(1, 0x600)])
    tot = (size * count) & 0xffffffff
    buf = newbuf(ctx, min(tot, 0x4000) + 16)
    reads = seq(r, [0, 0xffffffff, 1, 7, 0x10, 0x200, 0x1000], 3)

    def rd(n, raw, uc):
        v = reads[n % 3]
        if v not in (0, 0xffffffff):
            v = min(v, raw[2])
            uc.mem_write(raw[1], bytes((n * 7 + i) & 0xff for i in range(v)))
        return v
    ST['fn'][0x10105b50] = rd
    answers(0x10105a60, seq(r, [0x41, 0x0a, 0xffffffff, 0x80, 0x41], 8))
    return [buf, size, count, s]


def g_fwrite(ctx):
    r = begin(ctx, 0x10105750, 0x100fe290, 0x100ff030)
    s = stream(ctx)
    size = r.choice([1, 1, 2, 4, 3, 0x10, 0])
    count = r.choice([0, 1, 2, 5, 0x10, 0x100, 0x401, r.randrange(1, 0x600)])
    tot = (size * count) & 0xffffffff
    buf = newbuf(ctx, min(tot, 0x4000) + 16)
    writes = seq(r, ['all', 'all', 'half', 'fail'], 3)

    def wr(n, raw, uc):
        log(('write_data', read_bytes(uc, raw[1], min(raw[2], 0x4000))))
        kind = writes[n % 3]
        return raw[2] if kind == 'all' else (raw[2] // 2 if kind == 'half' else 0xffffffff)
    ST['fn'][0x10105750] = wr
    answers(0x100fe290, seq(r, [0, 0, 0xffffffff], 2))
    answers(0x100ff030, seq(r, [0x41, 0x41, 0xffffffff, 0], 6))
    return [buf, size, count, s]


def g_fseek(ctx):
    r = begin(ctx, 0x100fe970, 0x100fe290, 0x10105fe0)
    s = stream(ctx)
    answers(0x100fe970, [r.choice([0, 0x100, 0xffffffff, ctx.u32()])])
    answers(0x100fe290, [r.choice([0, 0xffffffff])])
    answers(0x10105fe0, [r.choice([0, 0x200, 0xffffffff, 0xffffffff])])
    return [s, r.choice([0, 1, -1, 0x100, ctx.u32()]) & 0xffffffff, r.choice([0, 1, 2, 1, 3, 0xffffffff])]


def g_flsbuf(ctx):
    r = begin(ctx, 0x10105750, 0x10105fe0, 0x10107080, 0x10107020)
    blocks = handle_table(ctx, [0x01, 0x21, 0x81, 0xa1])
    flags = r.choice([0x02, 0x01, 0x11, 0x12, 0x40, 0x42, 0x80, 0x82, 0x0a, 0x8a, 0x10a, 0x102, 0x00, 0x19,
                      0x13, 0x0b, 0x109, 0x11b])
    s = stream(ctx, flags)
    if r.random() < 0.3:
        s = r.choice([0x10159b68, 0x10159b88])
        base = newbuf(ctx, 0x200)
        w32(ctx, s, base + r.choice([0, 0, 3]), r.choice([0, 1]), base, flags, r.choice([1, 2, 0xffffffff]), 0,
            r.choice([0x200, 0x1000]), 0)
    fh = r32(ctx.m.uc, s + 0x10)
    if r.random() < 0.3:
        w32(ctx, s + 0x10, r.choice([0xffffffff, r.randrange(0, 32)]))
    newbase = newbuf(ctx, 0x200)
    writes = seq(r, ['all', 'all', 'half', 'fail'], 2)

    def wr(n, raw, uc):
        log(('write_data', read_bytes(uc, raw[1], min(raw[2], 0x400))))
        kind = writes[n % 2]
        return raw[2] if kind == 'all' else (raw[2] // 2 if kind == 'half' else 0xffffffff)

    def getbuf(n, raw, uc):        # like the real one: a buffer of 0x1000 and flag 8, or flag 4 when told so
        sp = raw[0]
        f = r32(uc, sp + 0xc)
        if ST['getbuf'] == 'buffer':
            mw(sp + 8, struct.pack('<I', newbase))
            mw(sp, struct.pack('<I', newbase))
            mw(sp + 0x18, struct.pack('<I', 0x1000))
            mw(sp + 0xc, struct.pack('<I', f | 8))
        elif ST['getbuf'] == 'char':
            mw(sp + 0xc, struct.pack('<I', f | 4))
        return 0
    ST['getbuf'] = r.choice(['buffer', 'char', 'none'])
    ST['fn'][0x10105750] = wr
    ST['fn'][0x10107020] = getbuf
    answers(0x10107080, [r.choice([0, 1])])
    answers(0x10105fe0, [r.choice([0, 0xffffffff])])
    return [r.choice([0x41, 0x0a, 0xff, 0xffffff80, 0x141, 0]), s]


def fmt_stub(r):
    plan = seq(r, [0, 1, 3, 10, 0x20], 1)
    ret = r.choice([None, None, 0xffffffff])
    exhaust = r.random() < 0.25     # as if the room had been used up by a long output
    left = r.choice([0, 0xffffffff, 0x80000000])

    def h(n, raw, uc):
        f = raw[0]
        log(('fmt', read_bytes(uc, f, 16), read_cstr(uc, raw[1]), raw[2]))
        for i in range(plan[0]):
            ptr, cnt = r32(uc, f), r32(uc, f + 4)
            cnt = (cnt - 1) & 0xffffffff
            uc.mem_write(f + 4, struct.pack('<I', cnt))
            if cnt < 0x80000000:
                uc.mem_write(ptr, bytes([0x61 + i % 26]))
                uc.mem_write(f, struct.pack('<I', ptr + 1))
            else:
                fl = r32(uc, f + 0xc)
                uc.mem_write(f + 0xc, struct.pack('<I', fl | 0x20))
        if exhaust:
            uc.mem_write(f + 4, struct.pack('<I', left))
        return plan[0] if ret is None else ret
    return h


def g_vsnprintf(ctx):
    r = begin(ctx, 0x10100fd0)
    ST['fn'][0x10100fd0] = fmt_stub(r)
    buf = newbuf(ctx, 0x40)
    return [buf, r.choice([0, 1, 2, 3, 5, 10, 11, 0x20, 0x40, 0x80000000, 0xffffffff]), cstr(ctx, b'%d %s'),
            r.choice([0, ctx.u32(), newbuf(ctx, 8)])]


def g_vsprintf(ctx):
    r = begin(ctx, 0x10100fd0)
    ST['fn'][0x10100fd0] = fmt_stub(r)
    buf = newbuf(ctx, 0x40)
    return [buf, cstr(ctx, b'%x-%u'), r.choice([0, ctx.u32(), newbuf(ctx, 8)])]


# ---- text helpers --------------------------------------------------------------------------------
def g_strnlen(ctx):
    r = begin(ctx)
    n = r.randrange(0, 20)
    s = bytes(r.choice(b'ab\x80\xff') for _ in range(n)) + b'\0' + rb(ctx, 4)
    return [ctx.buf(len(s) + 4, s), r.choice([0, 1, n, n + 1, n - 1 if n else 0, r.randrange(0, 30), 0xffffffff])]


def g_wcsncnt(ctx):
    r = begin(ctx)
    n = r.randrange(0, 20)
    s = struct.pack('<%dH' % n, *[r.choice([0x41, 0x100, 0xffff, 0x8000]) for _ in range(n)]) + b'\0\0' + rb(ctx, 8)
    return [ctx.buf(len(s) + 4, s), r.choice([0, 1, n, n + 1, n + 2, n - 1 if n else 0, r.randrange(0, 30),
                                              0xffffffff])]


def g_forcdecpt(ctx):
    r = begin(ctx, 0x100fd9a0)
    w32(ctx, 0x1017e950, 0)                     # plain ASCII case mapping in Crt_ToLower
    w32(ctx, 0x1015965c, r.choice([1, 1, 2]))
    tab = bytearray(rb(ctx, 0x600))
    for i in range(0x600 // 2):
        c = (i - 0x100) & 0xff
        tab[2 * i] = (tab[2 * i] & ~4) | (4 if (0x30 <= c <= 0x39 or r.random() < 0.03) else 0)
    t = ctx.buf(len(tab), bytes(tab))
    w32(ctx, 0x10159450, t + 0x200)
    ST['fn'][0x100fd9a0] = lambda n, raw, uc: 4 if 0x30 <= (raw[0] & 0xff) <= 0x39 and raw[0] < 0x100 else 0
    wb(ctx, 0x10159660, [r.choice([0x2e, 0x2c])])
    text = r.choice([b'123', b'1e+05', b'E10', b'e5', b'7', b'', b'-12', b'12345678901234567890', b'0\x80',
                     bytes(r.choice(b'0123456789eE+-.\xb0') for _ in range(r.randrange(0, 12)))])
    return [ctx.buf(len(text) + 8, text + b'\0' + rb(ctx, 7))]


def g_cftof2(ctx):
    r = begin(ctx, 0x10100d20)
    wb(ctx, 0x1017e908, [r.choice([0, 2, 0x55])])

    def f(n, raw, uc):
        log(('flag', bytes(uc.mem_read(0x1017e908, 1))))
        return raw[1] + 1
    ST['fn'][0x10100d20] = f
    return [newbuf(ctx, 8), newbuf(ctx, 0x40), r.choice([0, 1, 6, 17, 0xffffffff])]


def g_atodbl(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015965c, 1)
    text = r.choice([b'0', b'1', b'-1.5', b'3.14159', b'1e10', b'1e308', b'1e309', b'1e-320', b'1e-400', b'  42',
                     b'abc', b'', b'+.5e-3', b'99999999999999999999', b'-0', b'1.7976931348623157e308',
                     b'%d.%de%d' % (r.randrange(0, 10 ** 6), r.randrange(0, 10 ** 6), r.randrange(-330, 330))])
    if r.random() < 0.3:
        wb(ctx, 0x10159660, [r.choice([0x2e, 0x2c])])
    return [newbuf(ctx, 8), cstr(ctx, text)]


# ---- time ----------------------------------------------------------------------------------------
def g_localtime(ctx):
    r = begin(ctx, 0x10109dd0, 0x1010a0a0)
    answers(0x1010a0a0, seq(r, [0, 1], 2))
    w32(ctx, 0x1015c020, r.choice([0, 3600, -3600, 18000, -36000, 28800, 0x7fffffff]))
    w32(ctx, 0x1015c024, r.choice([0, 1, 1]))
    w32(ctx, 0x1015c028, r.choice([-3600, 0, 3600, -1800]))
    t = r.choice([0, 1, 0x3f480, 0x3f481, 0x7ffc0b7e, 0x7ffc0b7f, 0x7fffffff, -1, -0x80000000, 3600, 86399,
                  r.randrange(0, 0x3f480), r.randrange(0x7ffc0000, 0x80000000), r.randrange(0, 0x7fffffff),
                  r.randrange(0, 0x7fffffff)])
    return [ctx.buf(4, struct.pack('<i', t))]


def g_tzset(ctx):
    r = begin(ctx, 0x10100130, 0x1010c4b0, 0x100fef30, 0x100fefe0)
    w32(ctx, 0x1015965c, 1)
    tz = r.choice([None, None, b'', b'PST8PDT', b'EST5', b'CET-1CEST', b'GMT0', b'XYZ-10:30:15ABC', b'AB',
                   b'UTC+3:x', b'JST-9:', b'ABC123:45DEF', b'NZST-12NZDT', b'X1Y'])
    p = 0 if tz is None else cstr(ctx, tz)
    answers(0x10100130, [p])
    last = r.choice([0, 0, cstr(ctx, tz if tz else b'PST8PDT'), cstr(ctx, b'EST5EDT')])
    w32(ctx, 0x1017ecdc, last)
    w32(ctx, 0x1015c0b0, newbuf(ctx, 0x40), newbuf(ctx, 0x40))
    malloc_pool(ctx, 0x40, 0.15, 1)
    answers(0x1010c4b0, seq(r, [0, 3, 0xffffffff, 0x40], 2))
    tzi = bytearray(rb(ctx, 0xac))
    struct.pack_into('<i', tzi, 0, r.choice([0, 480, -60, -570]))
    struct.pack_into('<H', tzi, 0x46, r.choice([0, 10, 11]))
    struct.pack_into('<i', tzi, 0x54, r.choice([0, 0, -30]))
    struct.pack_into('<H', tzi, 0x9a, r.choice([0, 3, 4]))
    struct.pack_into('<i', tzi, 0xa8, r.choice([0, -60, -60, 30]))
    wb(ctx, 0x1017ec30, tzi)
    ST['api']['GetTimeZoneInformation'] = [r.choice([0, 1, 2, 0xffffffff])]
    return []


def g_time(ctx):
    r = begin(ctx, 0x10105970)
    utc = struct.pack('<8H', r.choice([1998, 2026]), r.choice([1, 12]), r.randrange(0, 7), r.choice([1, 28]),
                      r.choice([0, 23]), r.choice([0, 59]), r.randrange(0, 60), r.randrange(0, 1000))
    local = struct.pack('<8H', *[r.getrandbits(16) for _ in range(8)])
    if r.random() < 0.5:                      # the cached minute
        cache = bytearray(utc)
        if r.random() < 0.5:
            k = r.choice([0, 2, 6, 8, 10, 4, 12, 14])
            cache[k] ^= 1
        wb(ctx, 0x1017e8e0, cache)
    w32(ctx, 0x1017e8d8, r.choice([0, 1, 0xffffffff, 7]))
    tzi = bytearray(rb(ctx, 0xac))
    struct.pack_into('<H', tzi, 0x9a, r.choice([0, 3]))
    struct.pack_into('<i', tzi, 0xa8, r.choice([0, -60]))
    tzi = bytes(tzi)
    ST['api']['GetLocalTime'] = lambda k, args, uc: uc.mem_write(stack_arg(uc, 0), local) or 0
    ST['api']['GetSystemTime'] = lambda k, args, uc: uc.mem_write(stack_arg(uc, 0), utc) or 0
    tret = r.choice([0, 1, 2, 2, 0xffffffff])

    def gtzi(k, args, uc):
        uc.mem_write(stack_arg(uc, 0), tzi)
        return tret
    ST['api']['GetTimeZoneInformation'] = gtzi
    answers(0x10105970, [ctx.u32()])
    return [r.choice([0, newbuf(ctx, 4)])]


def g_xtoa(ctx):
    r = begin(ctx)
    return [r.choice([0, 1, 9, 10, 35, 36, 0x7fffffff, 0x80000000, 0xffffffff, ctx.u32(), ctx.u32()]),
            newbuf(ctx, 0x30), r.choice([2, 8, 10, 10, 16, 16, 36, r.randrange(2, 37)]), r.choice([0, 0, 1])]


# ---- environment and abort -----------------------------------------------------------------------
def env_array(ctx, names):
    ptrs = [cstr(ctx, nm + b'=v', 4) for nm in names] + [0]
    return ctx.buf(4 * len(ptrs) + 8, struct.pack('<%dI' % len(ptrs), *ptrs) + rb(ctx, 8))


def g_setenv(ctx):
    r = begin(ctx, 0x100fef30, 0x100fefe0, 0x101003e0, 0x1010b530, 0x10107910, 0x1010afa0)
    w32(ctx, 0x1015965c, 1)

    def cmp(n, raw, uc):      # case-blind compare of the first raw[3] bytes: 1 less, 2 equal, 3 greater
        a = read_bytes(uc, raw[2], raw[3]).upper()
        b = read_bytes(uc, raw[4], raw[5]).upper()
        return 2 if a == b else (1 if a < b else 3)
    ST['fn'][0x1010afa0] = cmp
    names = [r.choice([b'PATH', b'TEMP', b'A', b'HG', b'X1']) for _ in range(r.randrange(0, 5))]
    env = r.choice([0, env_array(ctx, names), env_array(ctx, names), env_array(ctx, names)])
    w32(ctx, 0x1017e8b0, env)
    w32(ctx, 0x1017e8b4, r.choice([env, 0x1234, 0x1234]))
    w32(ctx, 0x1017e8b8, r.choice([0, newbuf(ctx, 8)]))
    copy = env_array(ctx, names)
    answers(0x1010b530, [r.choice([0, copy, copy])])
    ST['fn'][0x10107910] = lambda n, raw, uc: (mw(0x1017e8b0, struct.pack('<I', copy)) or 0) \
        if ST['w2m'] == 0 else 0xffffffff
    ST['w2m'] = r.choice([0, 0, 1])
    malloc_pool(ctx, 0x40, 0.15, 3)
    answers(0x101003e0, [r.choice([0, zbuf(ctx, 0x40), zbuf(ctx, 0x40)])])
    nm = r.choice([b'PATH', b'TEMP', b'NEW', b'A', b'', b'X1'])
    val = r.choice([b'', b'c:\\x', b'1'])
    text = r.choice([nm + b'=' + val, nm + b'=' + val, nm, b'=x', nm + b'=='])
    option = 0 if r.random() < 0.03 else cstr(ctx, text)
    return [option, r.choice([0, 1, 1])]


def g_abort(ctx):
    begin(ctx, 0x10108430, 0x10107670, 0x100ff3d0)
    return []


# ---- floating-point error path -------------------------------------------------------------------
def g_raise(ctx):
    r = begin(ctx)
    ST['fpcw'], ST['fpsw'] = 0x27f, r.getrandbits(6) | (r.getrandbits(1) << 7)
    rec = newbuf(ctx, 0x60)
    pcw = ctx.buf(8, struct.pack('<I', r.choice([0x27f, 0x37f, 0x7f, 0x27e, 0xe7f, 0x672, r.getrandbits(16),
                                                 ctx.u32()])) + rb(ctx, 4))
    if r.random() < 0.05:
        pcw = rec + 8        # aliasing: the control word inside the record
    arg1 = newbuf(ctx, 8)
    res = newbuf(ctx, 8)
    change = r.random() < 0.6
    newvals = [ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32()]

    def raise_h(k, args, uc):
        lp = stack_arg(uc, 3)
        recp = r32(uc, lp)
        log(('raise_arg', recp, read_bytes(uc, recp, 0x54)))
        if change:
            for off, v in zip((0, 8, 0x40, 0x44), newvals):
                uc.mem_write(recp + off, struct.pack('<I', v))
        return 0
    ST['api']['RaiseException'] = raise_h
    return [rec, pcw, r.choice([r.getrandbits(5), r.getrandbits(5), 0, ctx.u32()]), ctx.u32(), arg1, res]


def g_domain(ctx):
    r = begin(ctx)
    ST['fpcw'], ST['fpsw'] = r.choice([0x27f, 0x37f, 0x7f]), 0
    w32(ctx, 0x1015c018, r.choice([0, 0, 1]))
    w32(ctx, 0x1015c014, r.choice([0, 1]))
    x = r.choice([0x0, 0x8000000000000000, 0x3ff0000000000000, 0x7ff8000000000000, 0x7ff0000000000001,
                  0x0000000000000001, r.getrandbits(64), r.getrandbits(64)])
    return [r.choice([1, 2, 3, 0xd, 0x10, 0x18, ctx.u32()]), x & 0xffffffff, x >> 32,
            r.choice([0x27f, 0x37f, 0x7f, 0x1272])]


SPECS = [
    dict(name='Crt_LSeek', orig=0x10105fe0, c='Crt_LSeek', gen=g_lseek, trials=600, api=API),
    dict(name='Crt_Write', orig=0x10105750, c='Crt_Write', gen=g_write, trials=1500, api=API),
    dict(name='Crt_Read', orig=0x10105b50, c='Crt_Read', gen=g_read, trials=2000, api=API),
    dict(name='Crt_Close', orig=0x101049c0, c='Crt_Close', gen=g_close, trials=600, api=API),
    dict(name='Crt_ChSize', orig=0x1010c330, c='Crt_ChSize', gen=g_chsize, trials=600, api=API),
    dict(name='Crt_Remove', orig=0x100fdb30, c='Crt_Remove', gen=g_remove, trials=300, api=API),
    dict(name='Crt_FRead', orig=0x100fe830, c='Crt_FRead', gen=g_fread, trials=1200, api=API),
    dict(name='Crt_FWrite', orig=0x100febc0, c='Crt_FWrite', gen=g_fwrite, trials=1200, api=API),
    dict(name='Crt_FSeek', orig=0x100feb20, c='Crt_FSeek', gen=g_fseek, trials=600, api=API),
    dict(name='Crt_FlushBuf', orig=0x100ff030, c='Crt_FlushBuf', gen=g_flsbuf, trials=1500, api=API),
    dict(name='Crt_VSNPrintf', orig=0x100fe3e0, c='Crt_VSNPrintf', gen=g_vsnprintf, trials=500, api=API),
    dict(name='Crt_VSPrintf', orig=0x101006c0, c='Crt_VSPrintf', gen=g_vsprintf, trials=400, api=API),
    dict(name='Crt_StrNLen', orig=0x101055e0, c='Crt_StrNLen', gen=g_strnlen, trials=400, api=API),
    dict(name='Crt_WcsNCount', orig=0x1010c6a0, c='Crt_WcsNCount', gen=g_wcsncnt, trials=400, api=API),
    dict(name='Crt_ForceDecPoint', orig=0x10100a90, c='Crt_ForceDecPoint', gen=g_forcdecpt, trials=600, api=API,
         void=True),
    dict(name='Sim_10100f00', orig=0x10100f00, c='Sim_10100f00', gen=g_cftof2, trials=300, api=API),
    dict(name='Sim_10108da0', orig=0x10108da0, c='Sim_10108da0', gen=g_atodbl, trials=800, api=API),
    dict(name='Crt_LocalTime', orig=0x101071e0, c='Crt_LocalTime', gen=g_localtime, trials=1500, api=API),
    dict(name='Crt_TzSet', orig=0x10109df0, c='Crt_TzSet', gen=g_tzset, trials=1000, api=API),
    dict(name='Crt_Time', orig=0x100ff500, c='Crt_Time', gen=g_time, trials=600, api=API),
    dict(name='Crt_NumToText', orig=0x100ffe20, c='Crt_NumToText', gen=g_xtoa, trials=600, api=API, void=True),
    dict(name='Crt_SetEnv', orig=0x1010b2a0, c='Crt_SetEnv', gen=g_setenv, trials=1500, api=API),
    dict(name='Crt_Abort', orig=0x101073d0, c='Crt_Abort', gen=g_abort, trials=300, api=API, void=True),
    dict(name='Math_RaiseFpError', orig=0x101030b0, c='Math_RaiseFpError', gen=g_raise, trials=1500, api=API,
         void=True),
    dict(name='Math_DomainError', orig=TRAMP, c='T_Math_DomainError', gen=g_domain, trials=800, api=API),
]
