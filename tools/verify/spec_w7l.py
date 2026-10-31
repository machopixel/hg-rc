# Differential tests for round 7, letter l (see difftest.py and docs/checker.md).
#
# Helpers of the DLL are replaced in the emulator by stubs: a code hook at the helper's address appends
# ('stub_<address>', arguments) to the checker's call log (compared between the original and the C run like the
# Windows calls), may write results through pointer arguments (the same bytes for both runs), sets eax to a value
# chosen by the generator and returns to the caller by setting EIP and ESP (code bytes are not patched).
#  * Helpers that are not converted (called with SIM_FN by the C code) are stubbed at their own address, which
#    both runs reach.
#  * Converted helpers whose own world would be large are stubbed at their original address (reached by the
#    original code, through its jump stub) AND at the address of their C function in the test image (reached by
#    the C code), under the same log name. Both versions are proven equal by their own specs.
# Stack addresses are logged as 'stack'; for pointer arguments to small records the bytes they point to are logged
# (DEREF), texts are logged by content (SARGS); arguments the helper reads as 16 bits are masked (MASK).
# Everything else runs for real on both sides: the C library routines sprintf 0x100fc780, sqrt 0x100fca04,
# fabs 0x100fcb70 and the table lookups 0x10084766 / 0x1008486e (control word 0x27f, as in the game), the mono
# screen writer (screen at 0xb0000 mapped and compared) and the converted segment, string and list helpers.
# All floating point here is exact: no float tolerance is used. Float inputs are never signalling NaNs (the x87
# quiets those on load, the game never makes them).
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_1000/w7l_misc.c',
    'sim/unsorted_1001/w7l_misc.c',
    'sim/unsorted_1002/w7l_misc.c',
    'sim/unsorted_1002/w7l_collide.c',
    'sim/unsorted_1003/w7l_grid.c',
    '../tools/verify/w7l_test.c',
    # existing converted code the functions above call (and what that calls)
    'sim/core/timer.c', 'sim/core/w2a_timers.c', 'sim/render/w2n_polygon.c', 'sim/render/w2n_project.c',
    'sim/unsorted_1000/w2e_mono.c', 'sim/unsorted_1000/w2e_names.c', 'sim/unsorted_1000/w2f_records.c',
    'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2h_access.c', 'sim/unsorted_1000/w2j_slots.c',
    'sim/unsorted_1000/w2k_misc.c', 'sim/unsorted_1000/w2l_fields.c', 'sim/unsorted_1000/w2l_objects.c',
    'sim/unsorted_1000/w3o_sphere.c', 'sim/unsorted_1000/w3q_misc.c', 'sim/unsorted_1000/w3s_model.c',
    'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1000/w5x_calls.c', 'sim/unsorted_1001/w2e_names.c',
    'sim/unsorted_1001/w2e_tables.c', 'sim/unsorted_1001/w2h_objects.c', 'sim/unsorted_1001/w2j_misc.c',
    'sim/unsorted_1001/w2l_misc.c', 'sim/unsorted_1001/w3t_loader.c', 'sim/unsorted_1001/w3t_misc.c',
    'sim/unsorted_1001/w4u_cd.c', 'sim/unsorted_1001/w5b_calls.c', 'sim/unsorted_1001/w5c_calls.c',
    'sim/unsorted_1001/w5y_misc.c', 'sim/unsorted_1001/w6d_calls.c', 'sim/unsorted_1002/w2e_object.c',
    'sim/unsorted_1002/w2e_tables.c', 'sim/unsorted_1002/w2f_misc.c', 'sim/unsorted_1002/w2g_records.c',
    'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1002/w2j_nodes.c',
    'sim/unsorted_1002/w2l_tables.c', 'sim/unsorted_1002/w3o_ray.c', 'sim/unsorted_1002/w3o_vectors.c',
    'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1002/w4v_misc.c', 'sim/unsorted_1002/w5a_misc.c',
    'sim/unsorted_1002/w5y_misc.c', 'sim/unsorted_1002/w5z_calls.c', 'sim/unsorted_1002/w6d_calls.c',
    'sim/unsorted_1003/records.c', 'sim/unsorted_1003/setters.c', 'sim/unsorted_1003/w2b_misc.c',
    'sim/unsorted_1003/w2e_misc.c', 'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_1003/w2g_misc.c',
    'sim/unsorted_1003/w2l_records.c', 'sim/unsorted_1003/w3r_mesh.c', 'sim/unsorted_1003/w3r_misc.c',
    'sim/unsorted_1003/w3t_bounds.c', 'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1003/w4w_records.c',
    'sim/unsorted_1003/w5a_misc.c', 'sim/unsorted_1003/w5c_calls.c', 'sim/unsorted_1004/fields.c',
    'sim/unsorted_1004/tables.c', 'sim/unsorted_1004/w2e_strings.c', 'sim/unsorted_1004/w2h_record.c',
    'sim/unsorted_1004/w2i_misc.c', 'sim/unsorted_1004/w2k_flags.c', 'sim/unsorted_1004/w3o_misc.c',
    'sim/unsorted_1004/w3p_net.c', 'sim/unsorted_1004/w4u_heap.c', 'sim/unsorted_1004/w4w_net.c',
    'sim/unsorted_1004/w5y_misc.c', 'sim/unsorted_1005/w2g_pool.c', 'sim/unsorted_1005/w2h_tables.c',
    'sim/unsorted_1005/w3o_calls.c', 'sim/unsorted_1006/w3q_quat.c', 'sim/unsorted_1006/w5a_math.c',
    'sim/unsorted_1006/w5b_calls.c', 'sim/unsorted_1006/w5c_math.c', 'sim/unsorted_1006/w5y_misc.c',
    'sim/unsorted_1006/w6e_misc.c', 'sim/unsorted_1007/w2e_state.c', 'sim/unsorted_1007/w2f_misc.c',
    'sim/unsorted_1007/w2g_stubs.c', 'sim/unsorted_1007/w3p_net.c', 'sim/unsorted_1007/w5a_misc.c',
    'sim/unsorted_1008/cell_link.c', 'sim/unsorted_1008/w2g_rect.c', 'sim/unsorted_1008/w2i_rect.c',
    'sim/unsorted_1008/w3o_grid.c', 'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1008/w3s_grid.c',
    'sim/unsorted_1009/w2e_copy.c', 'sim/unsorted_1009/w2h_misc.c', 'sim/unsorted_1009/w2l_pool.c',
    'sim/unsorted_1009/w3o_ray.c', 'sim/unsorted_1009/w6g_segment.c', 'sim/unsorted_100a/w2i_misc.c',
    'sim/unsorted_100a/w2k_list.c', 'sim/unsorted_100a/w4u_misc.c', 'sim/unsorted_100a/w5a_misc.c',
    'sim/unsorted_100a/w5b_calls.c', 'sim/unsorted_100a/w5c_calls.c', 'sim/unsorted_100a/w5x_calls.c',
    'sim/unsorted_100a/w5y_misc.c', 'sim/unsorted_100a/w5z_calls.c', 'sim/unsorted_100a/w6g_nodes.c',
    'sim/unsorted_100b/fields.c', 'sim/unsorted_100b/w2e_strings.c', 'sim/unsorted_100b/w2h_records.c',
    'sim/unsorted_100b/w2j_misc.c', 'sim/unsorted_100b/w2l_records.c', 'sim/unsorted_100b/w3o_hash.c',
    'sim/unsorted_100b/w3s_array.c', 'sim/unsorted_100b/w4v_wrappers.c', 'sim/unsorted_100b/w5c_calls.c',
    'sim/unsorted_100b/w5z_calls.c', 'sim/unsorted_100b/w6g_misc.c', 'sim/unsorted_100c/w2e_tables.c',
    'sim/unsorted_100c/w2h_misc.c', 'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_100c/w2l_objects.c',
    'sim/unsorted_100c/w3o_misc.c', 'sim/unsorted_100c/w4v_timers.c', 'sim/unsorted_100c/w5a_misc.c',
    'sim/unsorted_100d/state.c', 'sim/unsorted_100d/w2f_records.c', 'sim/unsorted_100d/w2i_select.c',
    'sim/unsorted_100d/w2j_nodes.c', 'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_100f/w2g_crt.c',
    'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_100f/w6h_crt.c', 'sim/util/field.c', 'sim/util/lookup.c',
    'sim/util/record.c', 'sim/util/ring.c', 'sim/view/matrix.c', 'sim/view/objlist.c', 'sim/view/viewrect.c',
    'sim/view/w2d_view.c', 'sim/view/w2l_lists.c', 'sim/view/w3r_matrix.c', 'sim/win/w4a_heap.c',
    'sim/win/w4u_cdaudio.c', 'sim/win/w4v_dll.c', 'sim/win/w4v_heap.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000       # st(0) results are stored here (by the trampolines and the C test wrapper)
TRAMP = 0x00AF0000      # machine-code trampolines for the original side
CODE = 0x00AF8000       # targets of indirect calls (a `ret`, with a logging hook)

# helpers that are not converted: address -> number of dword arguments logged
STUBS = {
    0x1001457f: 1, 0x1007cbcb: 1, 0x10033eb3: 5, 0x100c398d: 2, 0x100608ef: 4, 0x1001b902: 1, 0x1001f790: 3,
    0x100b45b4: 1, 0x100ab1f2: 0, 0x100bc280: 4, 0x10029b1f: 1, 0x1009cfd4: 6, 0x1002dfbb: 7, 0x1002e91e: 5,
    0x1003218b: 1, 0x10031447: 3,
    # jump stubs that Sim_1001ee7c calls through a pointer
    0x100029eb: 0, 0x10001dfc: 0, 0x10003102: 0,
}
# converted helpers stubbed on both sides: C name -> (original address, number of dword arguments logged)
DSTUBS = {
    'Sim_100bb2b0': (0x100bb2b0, 2), 'Sim_10087c86': (0x10087c86, 0), 'Sim_1000951c': (0x1000951c, 2),
    'Sim_1007df6c': (0x1007df6c, 4), 'Sim_10016a6b': (0x10016a6b, 4), 'Sim_10017852': (0x10017852, 1),
    'Sim_100119a7': (0x100119a7, 1), 'Sim_1001403a': (0x1001403a, 2), 'Sim_100094df': (0x100094df, 3),
    'Sim_100096b2': (0x100096b2, 3), 'Sim_100db770': (0x100db770, 1), 'Sim_100b38c5': (0x100b38c5, 1),
    'Sim_100b399c': (0x100b399c, 1), 'Sim_1001f335': (0x1001f335, 0), 'Sim_1009eb24': (0x1009eb24, 0),
    'Sim_1001f26e': (0x1001f26e, 0), 'Sim_1009e441': (0x1009e441, 0), 'Sim_1007cbb4': (0x1007cbb4, 0),
    'Sim_10052c02': (0x10052c02, 1), 'Sim_10028b90': (0x10028b90, 2), 'Sim_100a9d47': (0x100a9d47, 4),
    'Sim_10028c6f': (0x10028c6f, 1), 'Sim_100aa39f': (0x100aa39f, 1), 'Sim_100aa799': (0x100aa799, 1),
    'Sim_10072ff2': (0x10072ff2, 0), 'Sim_100cc96c': (0x100cc96c, 0), 'Sim_10027cce': (0x10027cce, 1),
    'Sim_10027d77': (0x10027d77, 0), 'Sim_10009408': (0x10009408, 2), 'Sim_100aa8cb': (0x100aa8cb, 1),
    'Sim_100aa5e8': (0x100aa5e8, 2), 'Sim_100aa4f0': (0x100aa4f0, 2), 'Sim_100aa541': (0x100aa541, 2),
    'Sim_100aa406': (0x100aa406, 1), 'Sim_10038d65': (0x10038d65, 1), 'Sim_1002997e': (0x1002997e, 1),
    'Sim_1002990f': (0x1002990f, 1), 'Sim_1009c929': (0x1009c929, 1), 'Sim_10061b78': (0x10061b78, 1),
    'Sim_1006863d': (0x1006863d, 1), 'Sim_1000ebfb': (0x1000ebfb, 5), 'Sim_1002fa3f': (0x1002fa3f, 1),
    'Sim_10052c60': (0x10052c60, 2), 'Sim_10052be7': (0x10052be7, 1), 'Sim_10052aaf': (0x10052aaf, 1),
    'Sim_1000c749': (0x1000c749, 3), 'Sim_1002dc60': (0x1002dc60, 4), 'Sim_100881dd': (0x100881dd, 2),
    'Sim_1002e6de': (0x1002e6de, 3), 'Sim_1003184b': (0x1003184b, 7),
}
MASK = {(0x10016a6b, 1): 0xffff, (0x10016a6b, 2): 0xffff, (0x10016a6b, 3): 0xffff, (0x1001403a, 1): 0xffff,
        (0x10009408, 1): 0xffff}
SARGS = {0x1007df6c: (0,), 0x1000ebfb: (0,), 0x100bc280: (2,)}     # arguments that point to a text: logged by content
DEREF = {0x100608ef: {0: 16, 1: 16}}               # pointer arguments logged by the bytes they point to
EFFECT = {}         # helper -> function(uc, call number, raw arguments, eax) that writes the helper's results
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def w8(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, bytes(v & 0xff for v in vals))


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def ur32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def ustr(uc, addr, limit=200):
    out = b''
    while len(out) < limit:
        try:
            c = bytes(uc.mem_read(addr + len(out), 1))
        except Exception:
            return ('bad', addr)
        if c == b'\0':
            break
        out += c
    return out


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def rfl(ctx, spread=1000.0):
    """A float bit pattern: ordinary values, zeros, small and big ones, both signs, now and then a quiet NaN or an
    infinity (never a signalling NaN)."""
    r = ctx.rng
    c = r.random()
    if c < 0.5:
        return fbits(r.uniform(-spread, spread))
    if c < 0.65:
        return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 2.0, 100.0, -100.0, 1e-7, -1e-7, 5000.0]))
    if c < 0.88:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-10, 10))
    if c < 0.92:
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])
    v = r.getrandbits(32)
    if (v & 0x7f800000) == 0x7f800000:
        v |= 0x00400000         # quiet
    return v


def cstr(ctx, data, extra=24):
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rb(ctx, extra))


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


# ---- stubs --------------------------------------------------------------------------------------
def _log_args(uc, addr, nargs, esp):
    raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
    args = []
    for k, v in enumerate(raw):
        if (addr, k) in MASK:
            v &= MASK[(addr, k)]
        if k in SARGS.get(addr, ()):
            v = ustr(uc, v)
        elif k in DEREF.get(addr, {}):
            try:
                v = bytes(uc.mem_read(v, DEREF[addr][k]))
            except Exception:
                v = ('bad', v)
        args.append('stack' if isinstance(v, int) and STACK_LO <= v <= STACK_HI else v)
    return raw, args


def _stub_cb(m, addr, nargs):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        raw, args = _log_args(uc, addr, nargs, esp)
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % addr, tuple(args)))
        rets = ST['rets'].get(addr) or [0]
        eax = rets[n % len(rets)] & 0xffffffff
        if addr in EFFECT:
            EFFECT[addr](uc, n, raw, eax)
        uc.reg_write(UC_X86_REG_EAX, eax)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def _code_cb(m):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        k = (address - CODE) // 16
        nargs = ST['code_nargs'].get(k, 2)
        raw = struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs))) if nargs else ()
        args = tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)
        n = len(m.api_log)
        m.api_log.append(('code_%d' % k, args))
        rets = ST['code_rets'].get(k) or [mix(ST['seed'], k, n)]
        uc.reg_write(UC_X86_REG_EAX, rets[n % len(rets)] & 0xffffffff)
    return cb


def begin(ctx):
    """Installs the stubs and the mono screen (once per machine), the x87 state of the game (control word 0x27f)
    and the answers of the stubs for this trial. Must be the first thing a generator does."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w7l', False):
        m._w7l = True
        for addr, nargs in STUBS.items():
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr, nargs), None, addr, addr)
        for name, (addr, nargs) in DSTUBS.items():
            cb = _stub_cb(m, addr, nargs)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[name], m.syms[name])
        m.raw.hook_add(UC_HOOK_CODE, _code_cb(m), None, CODE, CODE + 16 * 64 - 1)
        m.uc.mem_map(0xb0000, 0x10000)          # the text screen that Mono_PutText writes to
        m.wr.append((0xb0000, 0x10000))
        m.snap0 = m.dump()
        m._index()          # re-index now, before this trial writes anything
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['seed'] = r.getrandbits(32)
    ST['code_nargs'] = {}
    ST['code_rets'] = {}
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}
    for name, (a, _) in DSTUBS.items():
        ST['rets'][a] = [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)]
    ST['heap'] = []
    ST['alloc_fail'] = ()
    ST['found'] = 0
    ST['cfd4'] = [(0, 0)]
    ST['dist'] = [0]
    ST['count94'] = 0
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def code_target(ctx, k, nargs, *values):
    """Address of a routine (`ret` with a logging hook) for an indirect call."""
    a = CODE + 16 * k
    ctx.m.uc.mem_write(a, b'\xc3')
    ST['code_nargs'][k] = nargs
    if values:
        ST['code_rets'][k] = list(values)
    return a


def tramp_st0(ctx, target, nargs, slot):
    """Machine code that calls target(args as dwords) and stores st(0) as a double at OUTF; returns its address.
    Every function has its own slot: the emulator may keep running an old translation of rewritten code."""
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\xdd\x1d' + struct.pack('<I', OUTF)       # fstp qword [OUTF]
    code += b'\x31\xc0\x5d\xc3'
    a = TRAMP + 0x100 * slot
    ctx.m.uc.mem_write(a, code)
    return a


def tramp_ax(ctx, target, nargs, slot):
    """Machine code that calls target(args as dwords) and returns ax zero-extended (the callers only read ax)."""
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\x0f\xb7\xc0\x5d\xc3'                     # movzx eax, ax
    a = TRAMP + 0x100 * slot
    ctx.m.uc.mem_write(a, code)
    return a


def heap_buffers(ctx, n, size):
    ST['heap'] = [zbuf(ctx, size) if ctx.rng.random() < 0.5 else newbuf(ctx, size) for _ in range(n)]


def a_heapalloc(n, args):
    """HeapAlloc: buffer n of the trial's pool, n being the number of the Windows call in the run (the functions
    tested with it make no other Windows calls); 0 past the pool's end or for a chosen failure."""
    k = n
    if k in ST.get('alloc_fail', ()):
        return 0
    return ST['heap'][k] if k < len(ST['heap']) else 0


API = {'HeapAlloc': a_heapalloc}


def ptrtab(ctx, count, size=0x200):
    """count records in the pointer table 0x109e8b40, count in [0x1015dc64]."""
    recs = []
    for i in range(count):
        rec = newbuf(ctx, size)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
        recs.append(rec)
    w32(ctx, 0x1015dc64, count)
    return recs


# ---- effects of stubbed helpers -----------------------------------------------------------------------
def e_1000951c(uc, n, raw, eax):
    """The routine Sim_1000951c runs over a list counts objects: make the four counters change."""
    for k, a in enumerate((0x1015d00c, 0x1015d010, 0x1015d014, 0x1015d018)):
        v = ur32(uc, a)
        uc.mem_write(a, struct.pack('<I', (v + mix(ST['seed'], n, k) % 2000) & 0xffffffff))


def e_10033eb3(uc, n, raw, eax):
    uc.mem_write(raw[4], struct.pack('<I', ST['found']))


def e_100608ef(uc, n, raw, eax):
    uc.mem_write(raw[3], struct.pack('<4I', *[mix(ST['seed'], n, k) for k in range(4)]))


def e_1009cfd4(uc, n, raw, eax):
    vals = ST['cfd4'][n % len(ST['cfd4'])]
    uc.mem_write(raw[4], struct.pack('<I', vals[0]))
    uc.mem_write(raw[5], struct.pack('<I', vals[1]))


def e_1002dfbb(uc, n, raw, eax):
    if eax == 0:
        return
    uc.mem_write(raw[2], struct.pack('<14I', *[mix(ST['seed'], n, k) for k in range(14)]))
    uc.mem_write(raw[3], struct.pack('<I', ST['dist'][n % len(ST['dist'])]))
    for k in range(4, 7):
        uc.mem_write(raw[k], struct.pack('<I', mix(ST['seed'], n, k + 100)))


def e_1001403a(uc, n, raw, eax):
    uc.mem_write(raw[0] + 0xd8, struct.pack('<I', ST['dist'][n % len(ST['dist'])]))


def e_100094df(uc, n, raw, eax):
    if raw[2]:
        uc.mem_write(raw[2], struct.pack('<I', ST['count94'] & 0xffffffff))


def e_1002dc60(uc, n, raw, eax):
    uc.mem_write(raw[3], struct.pack('<I', mix(ST['seed'], n)))


EFFECT.update({0x1000951c: e_1000951c, 0x10033eb3: e_10033eb3, 0x100608ef: e_100608ef, 0x1009cfd4: e_1009cfd4,
               0x1002dfbb: e_1002dfbb, 0x1001403a: e_1001403a, 0x100094df: e_100094df, 0x1002dc60: e_1002dc60})


# ---- generators: 0x100071fb .. 0x1000cfe2 --------------------------------------------------------------
def g_71fb(ctx):
    r = begin(ctx)
    for _ in range(r.choice([0, 1, 5, 20])):
        e = 0x10a51ac0 + r.randrange(0x200) * 0x300 + r.randrange(0x20) * 0x18
        ctx.m.uc.mem_write(e, rb(ctx, 0x18))
        if r.random() < 0.3:
            w32(ctx, e + 0x14, 0)
    for _ in range(4):
        ctx.m.uc.mem_write(0x10ac1b00 + r.randrange(0x200) * 0xe, rb(ctx, 0xe))
    w32(ctx, 0x1014ceb8, ctx.u32())
    w32(ctx, 0x1014cf0c, ctx.u32())
    return []


def g_7c9e(ctx):
    r = begin(ctx)
    w32(ctx, 0x10a3ade8, r.choice([0, 1, 1, 1, ctx.u32()]))
    w32(ctx, 0x1015d038, r.choice([0, 1, 1]))
    w32(ctx, 0x1015d03c, r.choice([0, 0, 1]))
    w32(ctx, 0x1015e54c, r.choice([0, 7, 123, 99999, ctx.u32()]))
    w32(ctx, 0x109b98c0, ctx.u32())
    for k in range(4):
        w32(ctx, 0x1015d00c + 4 * k, ctx.u32())
    n = r.choice([0, 1, 2, 5])
    for i in range(n):
        w32(ctx, 0x1036f2a0 + 4 * i, r.getrandbits(32) | 1)
    w32(ctx, 0x1036f2a0 + 4 * n, 0)
    w32(ctx, 0x1017c8d0, ctx.u32())
    ctx.m.uc.mem_write(0xb0000, rb(ctx, 0x200))
    return []


def model_points(ctx, n, extra=0):
    """A model: word +4 = n, points of 0x58 bytes from +0x18 with floats at +0xc/+0x10/+0x14."""
    r = ctx.rng
    m = newbuf(ctx, 0x18 + 0x58 * max(n, 1) + extra)
    w16(ctx, m + 4, n)
    for i in range(n):
        p = m + 0x18 + 0x58 * i
        if r.random() < 0.9:
            wf(ctx, p + 0xc, r.uniform(-500, 500), r.uniform(-500, 500), r.uniform(-500, 500))
        else:
            w32(ctx, p + 0xc, rfl(ctx), rfl(ctx), rfl(ctx))
    return m


def g_89c6(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x100)
    n = r.choice([1, 2, 3, 5, 8, 12, 30])
    w32(ctx, obj + 0x2c, model_points(ctx, n) if r.random() < 0.95 else 0)
    name = r.choice([b'w', b'W', b'wolf', b'ENBP', b'xxENBPyy', b'ENB', b'tank', b''])
    ctx.m.uc.mem_write(obj + 0xa0, name + b'\0')
    return [obj]


def g_ae6e(ctx):
    r = begin(ctx)
    recs = ptrtab(ctx, 4)
    for rec in recs:
        blk = newbuf(ctx, 0x600)
        w32(ctx, rec + 0x28, blk)
        w32(ctx, blk + 0x53c, r.choice([fbits(r.uniform(-50, 50)), fbits(r.uniform(-1000, 1000)), rfl(ctx)]))
    head = newbuf(ctx, 0x100)
    w32(ctx, head + 0xf0, r.choice([0x200 | r.randrange(4), 0x200 | r.randrange(4), r.randrange(4), ctx.u32() & ~0xfc]))
    w32(ctx, head + 0xd8, r.choice([fbits(r.uniform(-200, 200)), fbits(r.choice([100.0, 99.0, 0.0, -1.0])), rfl(ctx)]))
    w32(ctx, head, r.choice([1, 1, 0, 2]))
    obj = newbuf(ctx, 0x600)
    w32(ctx, obj, head)
    k = r.choice([0, 1, 2, 3])
    w32(ctx, obj + 0x4fc, k)
    w32(ctx, obj + 0x5fb, r.choice([k, k, 5]))
    rets(0x1000c749, r.choice([0, 1]), r.choice([0, 1]))
    a = newbuf(ctx, 8)
    b = newbuf(ctx, 8)
    return [obj, a, b]


def g_c894(ctx):
    r = begin(ctx)
    for i in range(0x2e):
        e = 0x10145508 + i * 0xd6
        w32(ctx, e + 0x86, r.choice([0x40, 0x41, 0, 1, ctx.u32()]))
        w32(ctx, e + 0x38, r.choice([r.randrange(-50, 500), 0, 1, ctx.u32()]))
        w32(ctx, e + 0x3c, r.choice([fbits(r.uniform(0, 3000)), fbits(r.uniform(-10, 10)), rfl(ctx)]))
        w32(ctx, e + 0x10, r.choice([0x45, 0x45, 0, 7, ctx.u32()]))
    w32(ctx, 0x1015d0a0, r.choice([0, 3, ctx.u32()]))
    return []


def g_cfe2(ctx):
    r = begin(ctx)
    recs = ptrtab(ctx, 6)
    for rec in recs:
        w32(ctx, rec + 0x5c, r.choice([fbits(r.uniform(-500, 500)), rfl(ctx)]))
    src = newbuf(ctx, 0x80)
    w32(ctx, src + 0x74, r.choice([fbits(r.uniform(-500, 500)), rfl(ctx)]))
    block = newbuf(ctx, 0x500)
    w32(ctx, block + 0x494, src if r.random() < 0.5 else 0)
    obj = newbuf(ctx, 0x200)
    w32(ctx, obj + 0x28, block)
    w32(ctx, obj + 0x48, src)
    w16(ctx, obj + 0x1aa, r.choice([0x200, 0x200, 0x300, 0x100, 0]) | r.randrange(6) | r.choice([0, 0x1000, 0xf000]))
    w32(ctx, obj + 0xd4, r.choice([fbits(r.uniform(-500, 500)), rfl(ctx)]))
    tramp_st0(ctx, 0x1000cfe2, 1, 1)
    return [obj]


# ---- generators: 0x1001150d .. 0x1001ee7c --------------------------------------------------------------
def g_150d(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x240)
    w32(ctx, obj + 0x14, r.choice([2, 3, 3, 0, 1, ctx.u32()]))
    cur = r.randrange(8)
    w32(ctx, 0x1017b8c8, cur)
    w32(ctx, obj + 0xc, r.choice([cur, cur, cur + 1]))
    w32(ctx, obj + 0x170, newbuf(ctx, 8))
    return [obj]


def g_25d9(ctx):
    r = begin(ctx)
    t = newbuf(ctx, 0x10)
    obj = newbuf(ctx, 0x80)
    w32(ctx, obj + 0x54, t)
    w32(ctx, t + 8, r.choice([fbits(r.uniform(-2, 2)), fbits(0.0), rfl(ctx)]))
    w32(ctx, obj + 0x5c, r.choice([fbits(r.uniform(-1000, 1000)), fbits(0.0), rfl(ctx)]))
    w32(ctx, obj + 0x64, r.choice([fbits(r.uniform(-12, 12)), fbits(r.choice([10.0, -10.0, 0.0, 9.5, 1e-8])), rfl(ctx)]))
    v = ctx.buf(4, struct.pack('<I', r.choice([fbits(r.uniform(-1000, 1000)), fbits(r.uniform(-5, 5)), rfl(ctx)])))
    return [obj, v]


def g_2970(ctx):
    r = begin(ctx)
    w32(ctx, 0x10a3ade8, r.choice([0, 1, 1, 1]))
    recs = ptrtab(ctx, r.choice([0, 1, 3, 6]), 0x200)
    for rec in recs:
        w32(ctx, rec + 4, r.choice([r.randrange(0, 25), r.randrange(-6, 30), ctx.u32()]))
        w32(ctx, rec + 8, r.choice([r.randrange(0, 10), ctx.u32()]))
        txt = bytes(r.choice(b'abcXYZ 0123') for _ in range(r.randrange(0, 20)))
        ctx.m.uc.mem_write(rec + 0x124, txt + b'\0')
        blk = newbuf(ctx, 0x500)
        w32(ctx, rec + 0x28, blk)
    w32(ctx, 0x1015ef14, r.choice(recs) if recs and r.random() < 0.7 else ctx.u32())
    ctx.m.uc.mem_write(0xb0000, rb(ctx, 0x400))
    return []


def g_5018(ctx):
    r = begin(ctx)
    ptrtab(ctx, r.choice([1, 3, 6, 10]), 0x20)
    obj = newbuf(ctx, 0x200)
    sel = r.choice([0x2100, 0x2101, 0x2200, 0x2201, 0x2202, 0x4200, 0x4201, 0x4202, 0x4300 | 0x200, 0x200, 0x2500,
                    0x100, 0x2400, 0x0123, 0x6201, r.getrandbits(16), 0x0f00 | r.getrandbits(8)])
    if r.random() < 0.2:
        sel |= r.choice([0x8000, 0x0300, 0x0c00])
    key = r.choice([-1, -1, 0, 1, 2, 5, 0x123, -2, r.getrandbits(16)])
    w32(ctx, 0x1017b8c8, r.choice([0, 1, 2, 0xffff, 0x10123, ctx.u32()]))
    w32(ctx, 0x1015d110, r.choice([0, 1]))
    w16(ctx, obj + 0x1d6, r.choice([0xffff, 0x12, 0x312, r.getrandbits(16)]))
    w16(ctx, obj + 4, r.choice([0xffff, 0x12, 0x312, r.getrandbits(16)]))
    for a in (0x10052c60, 0x10052be7):
        rets(a, *[r.choice([0, 0, 1, 2, 3]) for _ in range(5)])
    rets(0x10052aaf, *[r.choice([0, 1, 2, 0xffff, 0xffffffff, 0x1ffff, r.getrandbits(32)]) for _ in range(4)])
    rets(0x1007cbcb, r.choice([0, 3, 0xffff, 0x1ffff, 0x12345, 0x300]))
    tramp_ax(ctx, 0x10015018, 3, 2)
    return [obj, sel | (r.getrandbits(16) << 16), (key & 0xffff) | (r.getrandbits(16) << 16)]


def g_5f5b(ctx):
    r = begin(ctx)
    ptrtab(ctx, r.choice([0, 1, 3, 6]), 0x20)
    a = newbuf(ctx, 0x100)
    w32(ctx, a + 0xf0, r.choice([0xffffffff, 0x200 | r.randrange(6), r.randrange(6), 0x1234200 | r.randrange(256)]))
    obj = newbuf(ctx, 0x200)
    w16(ctx, obj + 0x1a6, r.choice([2, 3, 0, 1, 0xc]))
    w16(ctx, obj + 0x1aa, r.choice([0x200, 0x205, 0, 0x8201, r.getrandbits(16)]))
    ST['found'] = r.choice([0, 1, 1, ctx.u32()])
    rets(0x10033eb3, r.choice([0, 1, 1, 1]))
    rets(0x10052be7, *[r.choice([0, 1, 1, 2]) for _ in range(5)])
    ST['dist'] = [r.choice([fbits(r.uniform(0, 5000)), fbits(3.4e38), 0x7f7fc99e, fbits(0.0), rfl(ctx)])
                  for _ in range(7)]
    tramp_ax(ctx, 0x10015f5b, 2, 3)
    return [a, obj]


def g_78f4(ctx):
    r = begin(ctx)
    t = newbuf(ctx, 0x10)
    obj = newbuf(ctx, 0x240)
    w32(ctx, obj + 0x54, t)
    w32(ctx, t + 8, r.choice([fbits(0.0), fbits(1e-8), fbits(r.uniform(-3, 3)), rfl(ctx)]))
    w32(ctx, t + 4, r.choice([fbits(r.uniform(-50, 50)), fbits(r.choice([45.0, 40.0, -40.0, 41.0])), rfl(ctx)]))
    w32(ctx, obj, r.choice([1, 1, 1, 0, 2]))
    now = r.choice([0, 100, 5000, 0x7fffffe0, r.getrandbits(32)])
    w32(ctx, 0x1015dc78, now)
    w32(ctx, obj + 0x200, now + r.choice([-100, -1, 0, 1, 0x1f, 0x20, 0x21, 0x40, 0x41, 1000]))
    w32(ctx, obj + 0x218, r.choice([0, 1, ctx.u32()]))
    return [obj]


def g_96d5(ctx):
    r = begin(ctx)
    a = newbuf(ctx, 0x80)
    pa = [r.uniform(-3000, 3000) for _ in range(3)]
    wf(ctx, a + 0x58, *pa)
    b = newbuf(ctx, 0x40)
    c = newbuf(ctx, 0x40)
    w32(ctx, c + 8, r.choice([1, 1, 1, 0, 2]))
    for _ in range(r.choice([0, 1, 4, 10])):
        e = 0x1096fc80 + r.randrange(0x200) * 0x156
        ctx.m.uc.mem_write(e, rb(ctx, 0x60))
        src = newbuf(ctx, 0x80)
        spread = r.choice([10.0, 30000.0, 60000.0])
        wf(ctx, src + 0x70, *[x + r.uniform(-spread, spread) for x in pa])
        w32(ctx, e + 0x3c, src if r.random() < 0.9 else 0)
        w32(ctx, e + 0x48, r.choice([1, 1, 0]))
        w32(ctx, e, r.choice([3, 9, 0xa, 0xc, 3, 2, 0]))
        wf(ctx, e + 0x18, r.uniform(-1, 1), r.uniform(-1, 1), r.uniform(-1, 1))
    rets(0x10052be7, *[r.choice([0, 1, 2]) for _ in range(5)])
    return [a, b, c]


def g_9da0(ctx):
    r = begin(ctx)
    mode = r.choice([0, 0, 1, 1, 2])
    heap_buffers(ctx, 1, 0x50)
    ST['alloc_fail'] = (0,) if r.random() < 0.1 else ()
    slots = newbuf(ctx, 0x10)
    w32(ctx, 0x1017e5b8, slots)
    nums = [r.choice([0, 1, 7, 16, -3, 100000]) for _ in range(r.choice([0, 1, 3, 16, 18]))]
    body = b','.join(str(v).encode() for v in nums)
    text = str(r.choice([0, 5, 42, -1])).encode()
    if r.random() < 0.8:
        text += b';' + body
    tbuf = cstr(ctx, text)
    rets(0x100b38c5, r.choice([0xffffffff, 0, 3, 3]))
    rets(0x100b399c, newbuf(ctx, 0x20))
    if mode == 1:
        rec = newbuf(ctx, 0x50)
        cnt = r.choice([1, 2, 3, 16])
        w32(ctx, rec + 0xc, cnt)
        p2 = newbuf(ctx, 8)
        p1 = newbuf(ctx, 8)
        w32(ctx, p1, p2 if r.random() < 0.95 else 0)
        w32(ctx, p2, r.choice([ctx.u32() | 1, 0]))
        w32(ctx, rec, p1 if r.random() < 0.95 else 0)
        w32(ctx, rec + 8, r.choice([0xffffffff, 0, 1, 3, 5]))
        ST['count94'] = r.choice([0, 1, 4, 6])
        w32(ctx, slots + 4, rec if r.random() < 0.95 else 0)
    n = r.choice([0, 1, 7, 100, -5, r.randrange(-100000, 100000)])
    div = r.choice([0, 1, 2, 3, -1, 7])
    return [mode, tbuf, n & 0xffffffff, div & 0xffffffff]


def g_bb9d(ctx):
    r = begin(ctx)
    count = r.choice([0, 1, 2, 3, 5, -1])
    keys = newbuf(ctx, 4 + 0x14 * max(count, 1))
    w32(ctx, keys, count)
    times = sorted(r.sample(range(0, 200), max(count, 1)))
    for i in range(max(count, 0)):
        w32(ctx, keys + 4 + 0x14 * i, times[i])
    period = r.choice([200, 201, 300, 50, 7])
    t = r.choice([r.choice(times), r.randrange(0, 400), r.randrange(-400, 400), 0])
    out = newbuf(ctx, 16) if r.random() < 0.95 else 0
    return [t & 0xffffffff, period, keys, out]


def g_cedf(ctx):
    r = begin(ctx)
    ids = [r.getrandbits(32) for _ in range(3)]
    w32(ctx, 0x1015d190, ids[0])
    w32(ctx, 0x1015d194, ids[1])
    w32(ctx, 0x1015d19c, ids[2])
    n = r.choice([0, 1, 2, 4])
    nent = r.choice([3, 4, 6])
    obj = newbuf(ctx, 0x1c + 0xc * nent)
    hdr = newbuf(ctx, 0x10)
    w32(ctx, obj + 0x14, hdr)
    w32(ctx, hdr + 0xc, r.choice([nent, nent, 2, 0]))
    order = list(range(nent))
    r.shuffle(order)
    for i in range(nent):
        e = obj + 0x1c + 0xc * i
        nameblk = newbuf(ctx, 8)
        which = order[i]
        w32(ctx, nameblk + 4, ids[which] if which < 3 and r.random() < 0.95 else ctx.u32())
        w32(ctx, e, nameblk)
        cnt = n if r.random() < 0.9 else n + 1
        keys = newbuf(ctx, 8 + 0x14 * max(cnt, 1))
        w32(ctx, keys, cnt)
        for k in range(cnt):
            wf(ctx, keys + 8 + 0x14 * k, *[r.uniform(-1, 1) for _ in range(4)])
        w32(ctx, e + 8, keys)
    slot = newbuf(ctx, 4)
    w32(ctx, slot, obj if r.random() < 0.95 else 0)
    rets(0x1001b902, slot if r.random() < 0.95 else 0)
    return [ctx.u32()]


def g_ee7c(ctx):
    r = begin(ctx)
    for a in (0x101321b0, 0x101321b4, 0x101321b8, 0x101321bc, 0x101321c0):
        w32(ctx, a, r.choice([0, 1, 2, ctx.u32()]))
    index = r.choice([0, 1, 2, 3, 4, 5, -1, 0x80000000])
    value = r.choice([0, 1, 2, ctx.u32()])
    if r.random() < 0.3 and 0 <= index < 4:
        value = r32(ctx, (0x101321b4, 0x101321b8, 0x101321bc, 0x101321b0)[index])
    rets(0x1009eb24, r.choice([0, 1]))
    return [index & 0xffffffff, value]


# ---- generators: 0x100200d1 .. 0x1003129f --------------------------------------------------------------
def g_00d1(ctx):
    r = begin(ctx)
    for n in range(4):
        w32(ctx, 0x101336b0 + 4 * n, newbuf(ctx, 0x18 * 12))
    for a in range(5):
        w32(ctx, 0x101336f8 + 8 * a, r.choice([-1, 0, 1, 2, 3]))
        w32(ctx, 0x101336fc + 8 * a, r.choice([0, 0, 1, 2, ctx.u32() & 0xff]))
    a = r.choice([-2, -1, 0, 0, 1, 2, 3, 4, 4, 5])
    b = r.choice([-1, 0, 0, 1, 5, 9, 10])
    return [a & 0xffffffff, b & 0xffffffff]


def g_597b(ctx):
    r = begin(ctx)
    index = r.randrange(0, 8)
    for i in range(8):
        w16(ctx, 0x109e9ba0 + i * 0x50, r.choice([0, 4, 0x40, 0x44, 0x8040, r.getrandbits(16)]))
    cur = newbuf(ctx, 0x30)
    w16(ctx, cur + 2, r.choice([0x200, 0x200, 0, 0x8200]))
    w16(ctx, cur + 0x24, r.choice([index, index, index + 1, 0x10000 - 1]))
    w32(ctx, 0x1017e678, cur if r.random() < 0.8 else 0)
    recs = ptrtab(ctx, 4, 0x20)
    for rec in recs:
        w32(ctx, rec + 8, r.choice([0, 0, 1]))
    w32(ctx, 0x10134898, r.choice([-1, 0, 1, 2, 3]))
    w32(ctx, 0x1017b8c8, r.choice([0, 1, 2, 3]))
    ctx.m.uc.mem_write(0x101653ed, rb(ctx, 6))
    ctx.m.uc.mem_write(0x10165406, rb(ctx, 6))
    rets(0x10052c02, r.choice([0, 1, 2, 3]), r.choice([0, 1, 2, 3]))
    return [index]


def g_5c8c(ctx):
    r = begin(ctx)
    for _ in range(r.choice([0, 3, 10, 40])):
        e = 0x109a5a40 + r.randrange(0x100) * 0x2c
        w32(ctx, e + 0x18, r.choice([0, 0, 0, 1]))
        w32(ctx, e + 0x24, r.choice([0xb, 0xb, 0xb, 3]))
        w32(ctx, e, newbuf(ctx, 0x40) if r.random() < 0.9 else 0)
    w32(ctx, 0x10147e14, ctx.u32())
    rets(0x10028b90, *[r.choice([0xffffffff, 0, 3, 5]) for _ in range(4)])
    return [ctx.u32(), ctx.u32(), ctx.u32(), r.choice([0, 0, 1, 2, 5, 0x200, -1 & 0xffffffff])]


def g_7654(ctx):
    r = begin(ctx)
    for a in (0x109f1448, 0x109f144c, 0x109f144e, 0x109f1454, 0x109f1456):
        w16(ctx, a, r.choice([0, 1, 5, 100, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]))
    w32(ctx, 0x109f1458, ctx.u32())
    for k in range(10):
        w32(ctx, 0x10134f80 + 4 * k, ctx.u32())
    for a in (0x10072ff2, 0x100cc96c, 0x100ab1f2):
        rets(a, r.choice([0, 1, 0x30, 0x1000, ctx.u32()]))
    return []


def g_7fb8(ctx):
    r = begin(ctx)
    node = newbuf(ctx, 0x10)
    ident = r.choice([0, 5, 0x3f0, 0x3f1, 0xffff, r.getrandbits(16)])
    w16(ctx, node, ident)
    name = bytes(r.choice([0x41, 0x5a, 0x30, 0x80, 0xff, 0x7f, r.getrandbits(8)]) for _ in range(4))
    ctx.m.uc.mem_write(node + 4, name)
    s = ident + sum(b - 256 if b >= 128 else b for b in name)
    bucket = abs(s) % 0x3f1 * (1 if s >= 0 else -1)
    buf = newbuf(ctx, 0x3f1 * 4 + 0x1000)
    table = buf + 0x800
    w32(ctx, 0x1015dc30, table)
    case = r.choice(['direct', 'chain', 'chain', 'missing', 'empty'])
    w32(ctx, node + 8, r.choice([0, newbuf(ctx, 0x10)]))
    if case == 'direct':
        w32(ctx, table + 4 * bucket, node)
    elif case == 'empty':
        w32(ctx, table + 4 * bucket, 0)
    else:
        chain = [newbuf(ctx, 0x10) for _ in range(r.choice([1, 2, 4]))]
        for a, b in zip(chain, chain[1:] + [node if case == 'chain' else 0]):
            w32(ctx, a + 8, b)
        w32(ctx, table + 4 * bucket, chain[0])
    w32(ctx, 0x109f13f0, r.choice([node, 0, ctx.u32()]))
    w32(ctx, 0x1015f1e8, r.choice([0, 1]))
    w32(ctx, 0x109f13e0, ctx.u32())
    w32(ctx, 0x1017b8f8, ctx.u32())
    w32(ctx, 0x109b98c0, ctx.u32())
    return [node if r.random() < 0.97 else 0]


def g_8fb0(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0xa0)
    text = r.choice([None, b'xxWRFyy', b'WBZ', b'abc', b'', b'WR', b'aWBZ'])
    w32(ctx, obj + 0x90, cstr(ctx, text) if text is not None else 0)
    done = code_target(ctx, 0, 1) if r.random() < 0.7 else 0
    rets(0x1002997e, r.choice([0xffffffff, 0, 3]))
    return [obj if r.random() < 0.97 else 0, done]


def g_8d9f(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0xa0)
    m = newbuf(ctx, 0x30)
    w16(ctx, m + 2, r.choice([0x70, 0x75, 0x10, 0, r.getrandbits(16)]))
    w16(ctx, m + 0x26, r.choice([0, 1]))
    w32(ctx, obj + 0x8c, m if r.random() < 0.95 else 0)
    text = r.choice([None, b'POF_x', b'WRF', b'abc', b'aWBZ'])
    w32(ctx, obj + 0x90, cstr(ctx, text) if text is not None else 0)
    free = r.choice([None, 0, 5, 0x7f, r.randrange(0x80)])
    for i in range(0x80):
        w32(ctx, 0x109f09e0 + i * 0x14, 0 if i == free else r.choice([1, 1, ctx.u32() | 1]))
    w32(ctx, 0x1015dc78, ctx.u32())
    done = code_target(ctx, 0, 1) if r.random() < 0.7 else 0
    rets(0x10028b90, r.choice([0xffffffff, 0, 2, 7]))
    rets(0x1002997e, r.choice([0xffffffff, 0, 3]))
    return [obj if r.random() < 0.97 else 0, done, r.choice([0, 1, ctx.u32()])]


def g_c76d(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x580)
    w16(ctx, obj + 0x560, r.choice([0, 0x80, 0x40, 0xc0, r.getrandbits(16)]))
    arr = newbuf(ctx, 0x128 * 4)
    idx = r.randrange(4)
    w32(ctx, obj + 0x4fc, idx)
    w32(ctx, arr + idx * 0x128 + 8, r.choice([1, 1, 0]))
    w32(ctx, obj + 0x84, arr)
    w32(ctx, obj + 0x4f0, r.choice([2, 2, 0, 3]))
    w32(ctx, 0x1015dd14, r.choice([0, 5, ctx.u32()]))
    w32(ctx, 0x1015dd24, r.choice([0, 3, 4, 5, -1]))
    w32(ctx, 0x1015ef0c, r.choice([0, 1, 1]))
    w32(ctx, 0x1015dd1c, r.choice([0, 1]))
    w32(ctx, 0x1015dd20, r.choice([0, 1]))
    w32(ctx, 0x1015dd18, ctx.u32())
    rets(0x1009c929, r.randrange(5))
    return [obj]


def make_seg(ctx, length=None):
    """A 0x38-byte segment: start +0, end +0xc, vector +0x18, unit +0x24, length +0x30, state +0x34."""
    r = ctx.rng
    seg = newbuf(ctx, 0x38)
    start = [r.uniform(-200, 200) for _ in range(3)]
    d = [r.uniform(-1, 1) for _ in range(3)]
    ln = length if length is not None else r.choice([0.0, 1e-8, 10.0, 300.0, 1000.0, r.uniform(0, 2000)])
    end = [s + ln * v for s, v in zip(start, d)]
    wf(ctx, seg, *start)
    wf(ctx, seg + 0xc, *end)
    wf(ctx, seg + 0x18, *[e - s for s, e in zip(start, end)])
    wf(ctx, seg + 0x24, *d)
    wf(ctx, seg + 0x30, ln)
    w32(ctx, seg + 0x34, r.choice([0, 0, 1, 1, 2]))
    return seg


def g_cf23(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x100)
    box = newbuf(ctx, 0x18)
    wf(ctx, box, *[r.uniform(-300, 300) for _ in range(6)])
    w32(ctx, obj + 0x98, box if r.random() < 0.95 else 0)
    seg = make_seg(ctx)
    rets(0x1009cfd4, *[r.choice([0, 0, 0, 0, 1]) for _ in range(4)])
    ST['cfd4'] = [(r.choice([fbits(r.uniform(-50, 300)), fbits(0.0), fbits(-0.0), rfl(ctx)]),
                   r.choice([fbits(r.uniform(-50, 3000)), fbits(1000.0), rfl(ctx)])) for _ in range(5)]
    return [obj, seg]


def face_world(ctx, nfaces):
    """A model with 4 points (+0x18) and nfaces faces of 0x28 bytes at model + [model+8] (count word +6)."""
    r = ctx.rng
    npts = 4
    model = newbuf(ctx, 0x18 + 0x58 * npts + 0x28 * max(nfaces, 1))
    w16(ctx, model + 6, nfaces)
    w32(ctx, model + 8, 0x18 + 0x58 * npts)
    for i in range(npts):
        wf(ctx, model + 0x18 + 0x58 * i + 0xc, *[r.uniform(-100, 100) for _ in range(3)])
    for f in range(max(nfaces, 1)):
        face = model + 0x18 + 0x58 * npts + 0x28 * f
        w32(ctx, face + 4, 0x24)
        w8(ctx, face + 0x24, r.randrange(npts))
        if r.random() < 0.9:
            wf(ctx, face + 0x18, *[r.uniform(-1, 1) for _ in range(3)])
        else:
            w32(ctx, face + 0x18, rfl(ctx), rfl(ctx), rfl(ctx))
    return model


def g_d5e0(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x30, face_world(ctx, r.choice([0, 1, 3, 5])) if r.random() < 0.95 else 0)
    rets(0x1002e91e, *[r.choice([0, 1]) for _ in range(3)])
    return [obj, make_seg(ctx)]


def g_e790(ctx):
    r = begin(ctx)
    model = face_world(ctx, 1)
    rets(0x1002e91e, r.choice([0, 1, 1]))
    return [model + 0x18 + 0x58 * 4, model + 0x18, make_seg(ctx)]


def node_lists(ctx):
    r = ctx.rng
    nl = r.choice([0, 1, 2, 4])
    for i in range(nl):
        lst = newbuf(ctx, 0x40)
        prev = lst
        for _ in range(r.choice([0, 1, 3])):
            node = newbuf(ctx, 0x40)
            w16(ctx, node + 2, r.choice([0, 0x100, r.getrandbits(16)]))
            w16(ctx, node + 0x24, r.choice([0, 3, 7, r.getrandbits(16)]))
            w32(ctx, node + 0x34, r.choice([6, 0, 5, 8]))
            w32(ctx, prev + 0x18, node)
            prev = node
        w32(ctx, prev + 0x18, 0)
        w32(ctx, 0x1036f2a0 + 4 * i, lst)
    w32(ctx, 0x1036f2a0 + 4 * nl, 0)


def g_dda9(ctx, with_id=True):
    r = begin(ctx)
    seg = make_seg(ctx)
    out = newbuf(ctx, 4)
    w32(ctx, 0x1017c8cc, newbuf(ctx, 0x20) if r.random() < 0.9 else 0)
    node_lists(ctx)
    rets(0x1002dfbb, *[r.choice([0, 0, 1, ctx.u32() | 1]) for _ in range(5)])
    ST['dist'] = [r.choice([fbits(r.uniform(0, 5000)), fbits(3.4e38), 0x7f7fc99e, fbits(0.0), rfl(ctx)])
                  for _ in range(7)]
    rets(0x1002dc60, r.choice([0, 1]))
    if with_id:
        return [seg, out, r.choice([-1, 0, 3, 7]) & 0xffffffff]
    return [seg, out]


def g_e569(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x34, r.randrange(0, 11))
    point = code_target(ctx, 1, 4, *[r.choice([0, 0, 0, 1]) for _ in range(5)])
    segt = code_target(ctx, 2, 2, r.choice([0, 1, 2, ctx.u32()]))
    for k in range(11):
        w32(ctx, 0x10138bc0 + 12 * k, r.choice([0, point]))
        w32(ctx, 0x10138bc4 + 12 * k, r.choice([0, 0, segt]))
    seg = make_seg(ctx)
    return [obj, seg, r.choice([fbits(10.0), fbits(r.uniform(0, 1)), rfl(ctx)])]


def g_ff6e(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0xa0)
    w32(ctx, obj + 0x98, r.choice([0, ctx.u32() | 1, ctx.u32() | 1]))
    w32(ctx, obj + 0x34, r.choice([0, 5, 5, 1, 6, ctx.u32()]))
    rets(0x1003218b, ctx.u32())
    return [obj if r.random() < 0.95 else 0]


def g_079c(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x100)
    w8(ctx, obj + 0xa0, r.choice([0x74, 0x54, 0x74, 0x78]))
    npts = 6
    nparts = r.choice([0, 1, 3, 6])
    model = newbuf(ctx, 0x18 + 0x58 * npts + 0x28 * max(nparts, 1))
    w16(ctx, model + 4, npts)
    w16(ctx, model + 6, nparts)
    w32(ctx, model + 8, 0x18 + 0x58 * npts)
    spread = r.choice([100.0, 30000.0, 60000.0])
    for i in range(npts):
        wf(ctx, model + 0x18 + 0x58 * i + 0xc, *[r.uniform(-spread, spread) for _ in range(3)])
    for p in range(max(nparts, 1)):
        part = model + 0x18 + 0x58 * npts + 0x28 * p
        nv = r.choice([1, 2, 3, 4])
        w16(ctx, part + 2, nv)
        w32(ctx, part + 4, 0x10)
        w8(ctx, part + 0x10, *[r.randrange(npts) for _ in range(4)])
    w32(ctx, obj + 0x2c, model if r.random() < 0.95 else 0)
    ST['heap'] = [newbuf(ctx, 0x644)] + [newbuf(ctx, 0x10) for _ in range(8)]
    ST['alloc_fail'] = (0,) if r.random() < 0.05 else ()
    return [obj if r.random() < 0.97 else 0]


def g_129f(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015dd6c, r.choice([0, 0, 0, 1]))
    n = r.choice([0, 1, 2, 5, 10])
    model = model_points(ctx, n)
    obj = newbuf(ctx, 0xa0)
    w32(ctx, obj + 0x2c, model if r.random() < 0.95 else 0)
    box = newbuf(ctx, 0x30)
    if n and r.random() < 0.7:
        p = bytes(ctx.m.uc.mem_read(model + 0x18 + 0xc, 12))
        x, y, z = struct.unpack('<3I', p)
        w32(ctx, box, x, x, y, y, z, z)
    rets(0x1003184b, box if r.random() < 0.9 else 0)
    rets(0x10031447, ctx.u32(), ctx.u32())
    return [obj]


SPECS = [
    dict(name='Sim_100071fb', orig=0x100071fb, c='Sim_100071fb', gen=g_71fb, trials=60, void=True),
    dict(name='Sim_10007c9e', orig=0x10007c9e, c='Sim_10007c9e', gen=g_7c9e, trials=400, void=True),
    dict(name='Sim_100089c6', orig=0x100089c6, c='Sim_100089c6', gen=g_89c6, trials=600, void=True),
    dict(name='Sim_1000ae6e', orig=0x1000ae6e, c='Sim_1000ae6e', gen=g_ae6e, trials=600, void=True),
    dict(name='Sim_1000c894', orig=0x1000c894, c='Sim_1000c894', gen=g_c894, trials=300, void=True),
    dict(name='Sim_1000cfe2', orig=TRAMP + 0x100, c='Test_1000cfe2', gen=g_cfe2, trials=500),
    dict(name='Sim_1001150d', orig=0x1001150d, c='Sim_1001150d', gen=g_150d, trials=300, void=True),
    dict(name='Sim_100125d9', orig=0x100125d9, c='Sim_100125d9', gen=g_25d9, trials=800, void=True),
    dict(name='Sim_10012970', orig=0x10012970, c='Sim_10012970', gen=g_2970, trials=400, void=True),
    dict(name='Sim_10015018', orig=TRAMP + 0x200, c='Test_10015018', gen=g_5018, trials=1500),
    dict(name='Sim_10015f5b', orig=TRAMP + 0x300, c='Test_10015f5b', gen=g_5f5b, trials=800),
    dict(name='Sim_100178f4', orig=0x100178f4, c='Sim_100178f4', gen=g_78f4, trials=600, void=True),
    dict(name='Sim_100196d5', orig=0x100196d5, c='Sim_100196d5', gen=g_96d5, trials=600),
    dict(name='Sim_10019da0', orig=0x10019da0, c='Sim_10019da0', gen=g_9da0, trials=800, api=API),
    dict(name='Sim_1001bb9d', orig=0x1001bb9d, c='Sim_1001bb9d', gen=g_bb9d, trials=800),
    dict(name='Sim_1001cedf', orig=0x1001cedf, c='Sim_1001cedf', gen=g_cedf, trials=500, void=True),
    dict(name='Sim_1001ee7c', orig=0x1001ee7c, c='Sim_1001ee7c', gen=g_ee7c, trials=500, void=True),
    dict(name='Sim_100200d1', orig=0x100200d1, c='Sim_100200d1', gen=g_00d1, trials=600, void=True),
    dict(name='Sim_1002597b', orig=0x1002597b, c='Sim_1002597b', gen=g_597b, trials=600, void=True),
    dict(name='Sim_10025c8c', orig=0x10025c8c, c='Sim_10025c8c', gen=g_5c8c, trials=400, void=True),
    dict(name='Sim_10027654', orig=0x10027654, c='Sim_10027654', gen=g_7654, trials=300),
    dict(name='Sim_10027fb8', orig=0x10027fb8, c='Sim_10027fb8', gen=g_7fb8, trials=800, void=True),
    dict(name='Sim_10028fb0', orig=0x10028fb0, c='Sim_10028fb0', gen=g_8fb0, trials=500, void=True),
    dict(name='Sim_10028d9f', orig=0x10028d9f, c='Sim_10028d9f', gen=g_8d9f, trials=600, void=True),
    dict(name='Sim_1002c76d', orig=0x1002c76d, c='Sim_1002c76d', gen=g_c76d, trials=600, void=True),
    dict(name='Sim_1002cf23', orig=0x1002cf23, c='Sim_1002cf23', gen=g_cf23, trials=800),
    dict(name='Sim_1002e790', orig=0x1002e790, c='Sim_1002e790', gen=g_e790, trials=1000),
    dict(name='Sim_1002d5e0', orig=0x1002d5e0, c='Sim_1002d5e0', gen=g_d5e0, trials=600),
    dict(name='Sim_1002dda9', orig=0x1002dda9, c='Sim_1002dda9', gen=g_dda9, trials=800),
    dict(name='Sim_1002e362', orig=0x1002e362, c='Sim_1002e362', gen=lambda c: g_dda9(c, False), trials=800),
    dict(name='Sim_1002e569', orig=0x1002e569, c='Sim_1002e569', gen=g_e569, trials=800),
    dict(name='Sim_1002ff6e', orig=0x1002ff6e, c='Sim_1002ff6e', gen=g_ff6e, trials=300),
    dict(name='Sim_1003079c', orig=0x1003079c, c='Sim_1003079c', gen=g_079c, trials=500, void=True, api=API),
    dict(name='Sim_1003129f', orig=0x1003129f, c='Sim_1003129f', gen=g_129f, trials=500, void=True),
]

# round 31: the texture name table is ours now (w31_map.py: the mapping step around the C run)
exec(open('tools/verify/w31_map.py').read())
SOURCES = SOURCES + [x for x in W31_SOURCES if x not in SOURCES]
SPECS = w31_wrap(SPECS, {'Sim_100071fb'})
