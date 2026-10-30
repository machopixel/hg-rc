# Differential tests for round 6, letter j (see difftest.py and docs/checker.md): C runtime code of HGSIM.DLL
# around 0x10101000..0x10106700.
#
# Library helpers that are not converted are replaced in the emulator by a code hook at their first byte (the
# code bytes are not patched): the hook appends ('stub_<address>', arguments) to the checker's call log, which is
# compared between the original and the C run like the Windows calls, answers in eax with a value chosen per
# trial, and returns to the caller (EIP = return address, ESP + 4; cdecl). Stack addresses are logged as 'stack'.
# 0x10101960 (store one character into a stream) is NOT stubbed: it runs for real on both sides, only the routine
# it calls when the buffer is full (0x100ff030) is stubbed.
# 0x10103870 returns st(0) and 0x10101a50 returns edx:eax: the original is called through a machine-code
# trampoline and the C through a wrapper (w6j_test.c), both store the whole result at OUTF.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW,
                               UC_X86_REG_FPSW, UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_1010/w6j_format.c', 'sim/unsorted_1010/w6j_double.c', 'sim/unsorted_1010/w6j_stream.c',
    'sim/unsorted_1010/w6j_heap.c', '../tools/verify/w6j_test.c',
    # existing converted code the functions above call (Sim_10107080) and what that file needs
    'sim/unsorted_1010/w2k_crt.c', 'sim/unsorted_100f/w2k_string.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000
TRAMP_D = 0x00F00800     # calls 0x10103870 and stores st(0) as a double at OUTF
TRAMP_Q = 0x00F00900     # calls 0x10101a50 and stores edx:eax at OUTF
CALLBACK = 0x00F00A00    # target of the pointer at 0x1017e96c (a `ret` with a logging hook)

# helper address -> number of dword arguments
STUBS = {
    0x100ff030: 2,   # (character, stream) called by 0x10101960 when the buffer is full
    0x100fd9a0: 2,   # (character, mask)
    0x10105a60: 1,   # (stream)
    0x101095f0: 2,   # (character, stream)
    0x100fefe0: 1,   # (block)
    0x100fef30: 1,   # (size)
    0x100fe290: 1,   # (stream)
    0x10109890: 1,   # (file handle)
    0x10106660: 0,
}
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def _stub_cb(m, addr, nargs):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        ret = struct.unpack('<I', bytes(uc.mem_read(esp, 4)))[0]
        args = []
        for k in range(nargs):
            v = struct.unpack('<I', bytes(uc.mem_read(esp + 4 + 4 * k, 4)))[0]
            args.append('stack' if STACK_LO <= v <= STACK_HI else v)
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % addr, tuple(args)))
        rets = ST['rets'].get(addr) or [0]
        uc.reg_write(UC_X86_REG_EAX, rets[n % len(rets)] & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, ret)
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def _callback_cb(m):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        arg = struct.unpack('<I', bytes(uc.mem_read(esp + 4, 4)))[0]
        n = len(m.api_log)
        m.api_log.append(('callback', (arg,)))
        rets = ST['cb_rets']
        uc.reg_write(UC_X86_REG_EAX, rets[n % len(rets)] & 0xffffffff)
    return cb


def tramp_double(target, nargs):
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])                 # push dword [ebp+8+4i]
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'   # mov eax, target; call eax
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\xdd\x1d' + struct.pack('<I', OUTF)               # fstp qword [OUTF]
    code += b'\x31\xc0\x5d\xc3'                                 # xor eax, eax; pop ebp; ret
    return code


def tramp_qword(target):
    code = b'\x55\x89\xe5\xff\x75\x08'
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4\x04'
    code += b'\xa3' + struct.pack('<I', OUTF)                   # mov [OUTF], eax
    code += b'\x89\x15' + struct.pack('<I', OUTF + 4)           # mov [OUTF+4], edx
    code += b'\x31\xc0\x5d\xc3'
    return code


def begin(ctx):
    """Installs the hooks once per machine and chooses the stub answers of this trial."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w6j', False):
        m._w6j = True
        for addr, nargs in STUBS.items():
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr, nargs), None, addr, addr)
        m.raw.mem_write(CALLBACK, b'\xc3')
        m.raw.hook_add(UC_HOOK_CODE, _callback_cb(m), None, CALLBACK, CALLBACK)
        m.raw.mem_write(TRAMP_D, tramp_double(0x10103870, 3))
        m.raw.mem_write(TRAMP_Q, tramp_qword(0x10101a50))
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['rets'] = {a: [r.choice([0, 1, 2, 4, 0xffffffff, 0x80000000, r.getrandbits(32)]) for _ in range(3)]
                  for a in STUBS}
    ST['cb_rets'] = [r.choice([0, 1, 0x100, 0xffffffff, r.getrandbits(32)]) for _ in range(2)]
    ST['api'] = [r.choice([0, 1, 2, 5, 0x6d, r.getrandbits(32)]) for _ in range(4)]


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def api_answer(n, args):
    return ST['api'][n % len(ST['api'])]


API = {name: api_answer for name in ('FlushFileBuffers', 'GetLastError', 'HeapCreate', 'HeapDestroy', 'VirtualFree')}


def pick_count(ctx):
    r = ctx.rng
    return r.choice([0, 1, 2, 3, 5, 9, 17, 40, -1, -2, 0x7fffffff, 0x80000000, 0x80000001, r.getrandbits(32)])


def stream(ctx, cnt=None):
    """A stream record (0x20 bytes) whose next-byte pointer points into a fresh buffer."""
    r = ctx.rng
    data = newbuf(ctx, 0x60)
    f = newbuf(ctx, 0x20)
    w32(ctx, f, data + r.randrange(0, 0x20))
    w32(ctx, f + 4, pick_count(ctx) if cnt is None else cnt)
    w32(ctx, f + 8, data)
    return f


def handle_table(ctx):
    """Handle table: count at 0x10ac385c, blocks of 32 eight-byte entries at 0x10ac3720 (flag byte at +4)."""
    r = ctx.rng
    count = r.choice([0, 1, 5, 32, 40, 64, 64, 64, 64, 0x80000000, 0xffffffff])
    w32(ctx, 0x10ac385c, count)
    for g in range(2):
        blk = ctx.buf(32 * 8, bytes(r.choice([0, 1, 1, 0x40, 0x41, 0x41, 0x81, 0xff, r.getrandbits(8)]) if i % 8 == 4
                                    else r.getrandbits(8) for i in range(32 * 8)))
        w32(ctx, 0x10ac3720 + 4 * g, blk)
    return count


# ---- generators ---------------------------------------------------------------------------------
def g_19b0(ctx):
    begin(ctx)
    r = ctx.rng
    f = stream(ctx, r.choice([-1, 0, 1, 2, 3, 6, 30, 0x80000000]))
    cnt = ctx.buf(4, struct.pack('<i', r.choice([0, 3, 100, -1, -2])))
    ch = r.choice([0x41, 0x20, 0x80, 0xff, 0x1ff, ctx.u32()])
    rets(0x100ff030, *[r.choice([ch & 0xff, 0xffffffff, 0, 0x30]) for _ in range(3)])
    return [ch, r.choice([-5, -1, 0, 1, 2, 3, 4, 8, 15, 0x80000000]), f, cnt]


def g_19f0(ctx):
    begin(ctx)
    r = ctx.rng
    f = stream(ctx, r.choice([-1, 0, 1, 2, 3, 6, 30, 0x80000000]))
    cnt = ctx.buf(4, struct.pack('<i', r.choice([0, 3, 100, -1, -2])))
    s = newbuf(ctx, 24)
    rets(0x100ff030, *[r.choice([0x41, 0xffffffff, 0, 0x30]) for _ in range(3)])
    return [s, r.choice([-5, -1, 0, 1, 2, 3, 4, 8, 15, 20, 0x80000000]), f, cnt]


def cursor(ctx):
    r = ctx.rng
    data = newbuf(ctx, 0x40)
    return ctx.buf(8, struct.pack('<I', data + r.randrange(8, 0x30)) + rb(ctx, 4))


def g_1a30(ctx):
    begin(ctx)
    return [cursor(ctx)]


def g_37d0(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1017e888, ctx.u32())
    return [r.choice([-3, -1, 0, 1, 2, 3, 4, 5, 0x80000000, 0x7fffffff, ctx.u32()])]


def rand_double(ctx):
    r = ctx.rng
    k = r.random()
    lo = r.choice([0, 0, 1, r.getrandbits(32)])
    if k < 0.3:
        hi = r.choice([0x7ff00000, 0xfff00000, 0x7ff80000, 0xfff80000, 0x7ff00001, 0x7ff40000, 0x7fffffff,
                       0xffffffff, 0x7ff7ffff, 0x80000000, 0, 0x3ff00000])
    elif k < 0.5:
        hi = (r.getrandbits(1) << 31) | (0x7ff << 20) | r.getrandbits(20)
    else:
        hi = r.getrandbits(32)
    return lo, hi


def g_3870(ctx):
    begin(ctx)
    r = ctx.rng
    lo, hi = rand_double(ctx)
    e = r.choice([0, 1, -1, 2, 10, -10, 0x401, -0x3fe, -0x3ff, 0x400, 0xc01, 0x7fffffff, 0x80000000,
                  r.randrange(-1100, 1100), r.getrandbits(32)])
    return [lo, hi, e]


def g_3940(ctx):
    begin(ctx)
    return list(rand_double(ctx))


def g_48e0(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1015965c, r.choice([0, 1, 1, 2, 3, 0xffffffff, 0x80000000]))
    tab = newbuf(ctx, 0x240)
    w32(ctx, 0x10159450, tab + 0x20)
    rets(0x100fd9a0, r.choice([0, 4, 1, 0x80000000]))
    return [r.choice([r.randrange(0, 256), r.randrange(0x30, 0x3a), r.randrange(0x41, 0x47), r.randrange(0x61, 0x67),
                      -1, -2, -16])]


def g_4920(ctx):
    begin(ctx)
    return [stream(ctx)]


def g_4950(ctx):
    begin(ctx)
    r = ctx.rng
    return [r.choice([0xffffffff, 0xffffffff, 0, 0x41, 0xff, ctx.u32()]), stream(ctx)]


def g_4aa0(ctx):
    begin(ctx)
    r = ctx.rng
    f = stream(ctx)
    w32(ctx, f + 0xc, r.choice([0, 8, 1, 2, 0x80, 0x83, 0x88, 0x89, 0x8a, 0x408, 0x40b, 0xffffffff, ctx.u32()]))
    return [f]


def g_4ae0(ctx):
    begin(ctx)
    r = ctx.rng
    handle_table(ctx)
    k = r.random()
    if k < 0.4:
        f = 0x10159b68
    elif k < 0.8:
        f = 0x10159b88
    else:
        f = stream(ctx)
    ctx.m.uc.mem_write(f, rb(ctx, 0x20))
    w32(ctx, f + 0x10, r.choice([0, 1, 2, 3, 31, 32, 33, 40, 0xffffffff]))
    w32(ctx, f + 0xc, r.choice([0, 0, 0, 2, 0x100, 4, 8, 0x1000, ctx.u32()]))
    for s in range(2):
        w32(ctx, 0x1017e930 + 4 * s, r.choice([0, 0, 0x00A12340, ctx.u32()]))
    w32(ctx, 0x1017e964, ctx.u32())
    rets(0x100fef30, r.choice([0, 0x00A55000, 0x80000000]))
    return [f]


def g_4b80(ctx):
    begin(ctx)
    r = ctx.rng
    f = stream(ctx)
    w32(ctx, f + 0xc, r.choice([0, 0x1000, 0x1100, 0x1102, 0x100, 0xffffffff, ctx.u32()]))
    w32(ctx, f + 0x18, ctx.u32())
    return [r.choice([0, 0, 1, 2, 0xffffffff]), f]


def g_56f0(ctx):
    begin(ctx)
    r = ctx.rng
    handle_table(ctx)
    ST['api'] = [r.choice([0, 0, 1, 7]), r.choice([0, 5, 0x6d, r.getrandbits(32)])]   # FlushFileBuffers, GetLastError
    w32(ctx, 0x1017e888, ctx.u32())
    w32(ctx, 0x1017e88c, ctx.u32())
    return [r.choice([0, 1, 2, 5, 31, 32, 33, 63, 64, 0x80000000, 0xffffffff, r.randrange(0, 64)])]


def g_5fb0(ctx):
    begin(ctx)
    r = ctx.rng
    for i in range(64):
        w32(ctx, 0x10ac3720 + 4 * i, r.choice([0, 0, 0x00A40000 + 0x100 * i, ctx.u32()]))
    w32(ctx, 0x10ac3820, ctx.u32())
    return []


def g_6590(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x1017e96c, r.choice([0, CALLBACK, CALLBACK]))
    return [ctx.u32()]


def g_65b0(ctx):
    begin(ctx)
    r = ctx.rng
    w32(ctx, 0x10ac3714, ctx.u32())
    rets(0x10106660, r.choice([0, 1, 0x80000000, 0xffffffff]))
    return []


def g_65f0(ctx):
    begin(ctx)
    r = ctx.rng
    head = 0x10159e38
    n = r.choice([0, 1, 2, 3, 5])
    recs = [newbuf(ctx, 0x20) for _ in range(n)]
    chain = [head] + recs + [head]
    for a, b in zip(chain, chain[1:]):
        w32(ctx, a, b)
    for a in [head] + recs:
        w32(ctx, a + 0x10, r.choice([0, 0x00C00000, ctx.u32()]))
    w32(ctx, 0x10ac3714, ctx.u32())
    return []


SPECS = [
    dict(name='Sim_101019b0', orig=0x101019b0, c='Sim_101019b0', gen=g_19b0, trials=500, void=True),
    dict(name='Sim_101019f0', orig=0x101019f0, c='Sim_101019f0', gen=g_19f0, trials=500, void=True),
    dict(name='Sim_10101a30', orig=0x10101a30, c='Sim_10101a30', gen=g_1a30, trials=300),
    dict(name='Sim_10101a50', orig=TRAMP_Q, c='Test_10101a50', gen=g_1a30, trials=300),
    dict(name='Sim_101037d0', orig=0x101037d0, c='Sim_101037d0', gen=g_37d0, trials=300, void=True),
    dict(name='Sim_10103870', orig=TRAMP_D, c='Test_10103870', gen=g_3870, trials=1000),
    dict(name='Sim_10103940', orig=0x10103940, c='Sim_10103940', gen=g_3940, trials=1000),
    dict(name='Sim_101048e0', orig=0x101048e0, c='Sim_101048e0', gen=g_48e0, trials=500),
    dict(name='Sim_10104920', orig=0x10104920, c='Sim_10104920', gen=g_4920, trials=400),
    dict(name='Sim_10104950', orig=0x10104950, c='Sim_10104950', gen=g_4950, trials=300),
    dict(name='Sim_10104aa0', orig=0x10104aa0, c='Sim_10104aa0', gen=g_4aa0, trials=400, void=True),
    dict(name='Sim_10104ae0', orig=0x10104ae0, c='Sim_10104ae0', gen=g_4ae0, trials=800),
    dict(name='Sim_10104b80', orig=0x10104b80, c='Sim_10104b80', gen=g_4b80, trials=400, void=True),
    dict(name='Sim_101056f0', orig=0x101056f0, c='Sim_101056f0', gen=g_56f0, trials=600, api=API),
    dict(name='Sim_10105fb0', orig=0x10105fb0, c='Sim_10105fb0', gen=g_5fb0, trials=300, void=True),
    dict(name='Sim_10106590', orig=0x10106590, c='Sim_10106590', gen=g_6590, trials=300),
    dict(name='Sim_101065b0', orig=0x101065b0, c='Sim_101065b0', gen=g_65b0, trials=300, api=API),
    dict(name='Sim_101065f0', orig=0x101065f0, c='Sim_101065f0', gen=g_65f0, trials=300, void=True, api=API),
]
