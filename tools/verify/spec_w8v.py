# Differential tests for round 8, letter v (see difftest.py and docs/checker.md).
#
# Helpers are replaced in the emulator by stubs: a code hook at the helper's address appends
# ('stub_<address>', arguments) to the checker's call log (compared between the original and the C run like the
# Windows calls), may write the helper's results through its pointer arguments (the same bytes for both runs),
# sets eax to a value chosen by the generator and returns by setting EIP and ESP (code bytes are not patched).
#  * Helpers called through SIM_FN (not converted, converted by another round-8 helper, or patched by a hook
#    file) are stubbed at their own address, which both runs reach.
#  * Converted helpers whose own world would be large are stubbed at their original address (reached by the
#    original code, through its jump stub) AND at the address of their C function in the test image (reached by
#    the C code), under the same log name. Both versions are proven equal by their own specs.
# Stack addresses are logged as 'stack'; texts are logged by content, data buffers (fwrite, _write, records) by
# their bytes, arguments read as 8/16 bits are masked.
# Everything else runs for real on both sides: the C library string, ctype and sscanf routines, fabs, sqrt and
# pow (control word 0x27f, as in the game), the angle table lookup 0x10084766 (its table at 0x103719a0 is filled
# with sines once), the needle below the gauge ring.
# No float tolerance is used anywhere: every function here is compared bit for bit.
import math
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_1004/w8v_files.c', 'sim/unsorted_1004/w8v_shots.c', 'sim/unsorted_1004/w8v_misc.c',
    'sim/unsorted_1005/w8v_hud.c', 'sim/unsorted_1005/w8v_ini.c', 'sim/unsorted_1005/w8v_input.c',
    'sim/unsorted_1005/w8v_misc.c', 'sim/unsorted_1006/w8v_quat.c', 'sim/unsorted_1006/w8v_misc.c',
    # existing converted code the functions above call (and what that calls)
    'sim/core/timer.c', 'sim/core/w2a_timers.c', 'sim/render/w3r_draw.c', 'sim/unsorted_1000/w2e_mono.c',
    'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2i_misc.c', 'sim/unsorted_1000/w2j_slots.c',
    'sim/unsorted_1000/w2l_fields.c', 'sim/unsorted_1000/w2l_objects.c', 'sim/unsorted_1000/w3s_model.c',
    'sim/unsorted_1001/w2e_tables.c', 'sim/unsorted_1001/w2j_misc.c', 'sim/unsorted_1001/w3t_misc.c',
    'sim/unsorted_1001/w4u_cd.c', 'sim/unsorted_1001/w5y_misc.c', 'sim/unsorted_1002/w2g_records.c',
    'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1002/w5y_misc.c',
    'sim/unsorted_1003/records.c', 'sim/unsorted_1003/scramble.c', 'sim/unsorted_1003/setters.c',
    'sim/unsorted_1003/w2b_misc.c', 'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_1003/w2g_misc.c',
    'sim/unsorted_1003/w2k_misc.c', 'sim/unsorted_1003/w3p_net.c', 'sim/unsorted_1003/w3r_misc.c',
    'sim/unsorted_1003/w5c_calls.c', 'sim/unsorted_1004/fields.c', 'sim/unsorted_1004/state.c',
    'sim/unsorted_1004/tables.c', 'sim/unsorted_1004/w2h_misc.c', 'sim/unsorted_1004/w2j_misc.c',
    'sim/unsorted_1004/w2j_strtab.c', 'sim/unsorted_1004/w2k_flags.c', 'sim/unsorted_1004/w2k_hash.c',
    'sim/unsorted_1004/w4w_net.c', 'sim/unsorted_1004/w5b_calls.c', 'sim/unsorted_1004/w5c_calls.c',
    'sim/unsorted_1004/w5z_calls.c', 'sim/unsorted_1004/w6e_stream.c', 'sim/unsorted_1004/w7m_gif.c',
    'sim/unsorted_1005/w3o_calls.c', 'sim/unsorted_1005/w5b_calls.c', 'sim/unsorted_1005/w5y_misc.c',
    'sim/unsorted_1005/w7m_misc.c', 'sim/unsorted_1005/w7n_input.c', 'sim/unsorted_1007/w2e_state.c',
    'sim/unsorted_1007/w3p_net.c', 'sim/unsorted_1008/cell_link.c', 'sim/unsorted_1008/w2g_rect.c',
    'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1008/w3q_place.c',
    'sim/unsorted_1008/w5y_misc.c', 'sim/unsorted_1008/w6f_display.c', 'sim/unsorted_100a/w2k_list.c',
    'sim/unsorted_100a/w3o_misc.c', 'sim/unsorted_100a/w4u_misc.c', 'sim/unsorted_100a/w5a_misc.c',
    'sim/unsorted_100a/w5x_calls.c', 'sim/unsorted_100a/w5z_calls.c', 'sim/unsorted_100a/w7o_misc.c',
    'sim/unsorted_100a/w7p_files.c', 'sim/unsorted_100b/fields.c', 'sim/unsorted_100b/w2e_strings.c',
    'sim/unsorted_100b/w2h_records.c', 'sim/unsorted_100b/w2l_records.c', 'sim/unsorted_100b/w3o_hash.c',
    'sim/unsorted_100b/w3s_array.c', 'sim/unsorted_100b/w4v_wrappers.c', 'sim/unsorted_100b/w5c_calls.c',
    'sim/unsorted_100b/w5z_calls.c', 'sim/unsorted_100b/w6g_misc.c', 'sim/unsorted_100c/w2e_tables.c',
    'sim/unsorted_100c/w2l_objects.c', 'sim/unsorted_100c/w3o_misc.c', 'sim/unsorted_100c/w4v_timers.c',
    'sim/unsorted_100c/w5a_misc.c', 'sim/unsorted_100d/state.c', 'sim/unsorted_100d/w2f_records.c',
    'sim/unsorted_100d/w2j_nodes.c', 'sim/unsorted_100d/w5c_calls.c', 'sim/unsorted_100f/w2e_string.c',
    'sim/unsorted_100f/w2g_crt.c', 'sim/unsorted_100f/w2h_mem.c', 'sim/unsorted_100f/w2j_str.c',
    'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_100f/w6i_mem.c',
    'sim/unsorted_100f/w6i_misc.c', 'sim/unsorted_100f/w6i_stream.c', 'sim/unsorted_100f/w7q_parse.c',
    'sim/unsorted_100f/w7q_stream.c', 'sim/unsorted_100f/w7r_ctype.c', 'sim/unsorted_1010/w2h_misc.c',
    'sim/unsorted_1010/w2i_crt.c', 'sim/unsorted_1010/w2k_crt.c', 'sim/unsorted_1010/w6j_stream.c',
    'sim/unsorted_1010/w6k_handles.c', 'sim/unsorted_1010/w6k_text.c', 'sim/util/field.c', 'sim/util/lookup.c',
    'sim/util/ring.c', 'sim/view/matrix.c', 'sim/view/objlist.c', 'sim/view/viewrect.c', 'sim/view/w2a_points.c',
    'sim/view/w2d_view.c', 'sim/view/w2l_lists.c', 'sim/view/w2l_rects.c', 'sim/win/w4a_heap.c',
    'sim/win/w4u_cdaudio.c', 'sim/win/w4v_dll.c', 'sim/win/w4v_heap.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
STREAM = 0x00A7F000     # the FILE pointer the _fsopen stub hands out (never dereferenced: all stream calls stubbed)
CODE = 0x00AF8000       # (unused) targets of indirect calls

# helpers reached through their own address: address -> number of dword arguments logged
STUBS = {
    0x10105750: 3,      # _write(handle, buffer, count): the data is logged
    0x101049c0: 1,      # _close
    0x10046ac4: 4,      # opens a record file (path, out size, out block, 0)
    0x100febc0: 4,      # fwrite(buffer, size, count, stream): the data is logged
    0x100fe830: 4,      # fread(buffer, size, count, stream): fills the buffer
    0x100feb20: 3,      # fseek
    0x100fd760: 1,      # fclose
    0x100fd820: 3,      # _fsopen(name, mode, share)
    0x100fc780: 2,      # sprintf(buffer, format, ...): formats into the buffer, the text is logged
    0x100ff500: 1,      # time
    0x100282d4: 4, 0x100ed8c3: 1, 0x100e91dc: 5,
    0x100ef727: 7, 0x100e86a1: 7,                       # lines
    0x1008e9af: 3,      # clips a point: writes out
    0x10051c90: 6, 0x100652e8: 4,
    0x100bc280: 2,      # SystemError(code, format, ...): the arguments of the format are logged
    0x100543ee: 1,
    0x100ba7d5: 3,      # finds a resource: upper-cases its two texts in place, writes the size
    0x10059a34: 2, 0x100b9203: 2,
    0x100ff860: 3,      # _assert
    0x100a0def: 1, 0x100a512a: 1, 0x100a4f03: 1, 0x10062b7c: 4, 0x100d2306: 1, 0x100d1f90: 1, 0x10087d83: 1,
    0x100d3427: 1,
}
# converted helpers stubbed on both sides: C name -> (original address, number of dword arguments logged)
DSTUBS = {
    'Sim_100474db': (0x100474db, 1), 'Sim_10109980': (0x10109980, 3), 'FileRec_OpenWrite': (0x100ae344, 1),
    'FileRec_WriteScrambled': (0x100ae4bc, 3), 'FileRec_Close': (0x100ae68f, 1), 'File_OpenRead': (0x100ae2e0, 1),
    'Sim_100ae470': (0x100ae470, 3), 'Sim_1003fcdc': (0x1003fcdc, 2), 'Sim_1007df6c': (0x1007df6c, 4),
    'Sim_1004db5b': (0x1004db5b, 2), 'Sim_1004e719': (0x1004e719, 1),
    'Sim_1004db86': (0x1004db86, 1), 'Sim_1004dcaa': (0x1004dcaa, 1), 'Lzw_Compress': (0x1004e04a, 2),
    'Sim_10047e95': (0x10047e95, 1), 'Sim_1008e266': (0x1008e266, 3), 'Sim_10051a36': (0x10051a36, 6),
    'Stream_ReadLine': (0x100fe770, 3), 'Crt_FTell': (0x100fe970, 1), 'Input_GetDevice': (0x1005388b, 1),
    'Input_AddAnalogSink': (0x10053c40, 1), 'Sim_10053a4f': (0x10053a4f, 2), 'Input_ReadLine': (0x10053b64, 3),
    'Sim_100bb2b0': (0x100bb2b0, 2), 'Sim_1005f9aa': (0x1005f9aa, 1), 'Sim_100ff4e0': (0x100ff4e0, 1),
    'Sim_100d32e5': (0x100d32e5, 1), 'Sim_100d255e': (0x100d255e, 1), 'Sim_10035469': (0x10035469, 0),
    'Sim_100481dd': (0x100481dd, 1),
}
MASK = {(0x1004dcaa, 0): 0xff, (0x100e91dc, 2): 0xffff}
# arguments that point to a text: logged by content
SARGS = {0x100474db: (0,), 0x100ae344: (0,), 0x100ae2e0: (0,), 0x1003fcdc: (1,), 0x1007df6c: (0,),
         0x100fd820: (0, 1), 0x1005388b: (0,), 0x10053a4f: (1,), 0x100bb2b0: (0, 1),
         0x1005f9aa: (0,), 0x100ba7d5: (0, 1), 0x100b9203: (0, 1), 0x100ff860: (0, 1)}
# pointer arguments logged by the bytes they point to: address -> {argument: size}
DEREF = {0x100ae4bc: {1: 1}, 0x1004e719: {0: 0x12}, 0x10047e95: {0: 8},
         0x1008e266: {0: 0x14, 1: 8}, 0x1008e9af: {0: 0x14, 1: 8}, 0x100ff4e0: {0: 4}, 0x10053c40: {0: 8}}
EFFECT = {}         # helper -> function(uc, call number, raw arguments, eax): writes results, may add log items
ST = {}


# ---- helpers ------------------------------------------------------------------------------------
def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def w8(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, bytes(v & 0xff for v in vals))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def ur32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def r32(ctx, addr):
    return ur32(ctx.m.uc, addr)


def ustr(uc, addr, limit=300):
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


def cstr(ctx, data, extra=16):
    return ctx.buf(len(data) + 1 + extra, bytes(data) + b'\0' + rb(ctx, extra))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def rfl(ctx, spread=1000.0):
    """A float bit pattern: ordinary values, zeros, small and big ones, both signs, now and then a quiet NaN or an
    infinity (never a signalling NaN)."""
    r = ctx.rng
    c = r.random()
    if c < 0.5:
        return fbits(r.uniform(-spread, spread))
    if c < 0.65:
        return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 2.0, 100.0, -100.0, 1e-7, -1e-7, 5000.0]))
    if c < 0.9:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-10, 10))
    if c < 0.95:
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])
    v = r.getrandbits(32)
    if (v & 0x7f800000) == 0x7f800000:
        v |= 0x00400000
    return v


def text(ctx, n, alpha=b'abcxyzABC019_ .'):
    return bytes(ctx.rng.choice(alpha) for _ in range(n))


# ---- printf for the formats of these functions (%s %d %c %x with flags, width, l) ------------------------
def fmt(uc, f, words):
    out = b''
    i = k = 0
    while i < len(f):
        c = f[i:i + 1]
        if c != b'%':
            out += c
            i += 1
            continue
        j = i + 1
        flags = b''
        while f[j:j + 1] in (b'-', b'0'):
            flags += f[j:j + 1]
            j += 1
        w = b''
        while f[j:j + 1].isdigit():
            w += f[j:j + 1]
            j += 1
        while f[j:j + 1] == b'l':
            j += 1
        conv = f[j:j + 1]
        if conv == b'%':
            out += b'%'
            i = j + 1
            continue
        v = words[k]
        k += 1
        if conv == b's':
            s = ustr(uc, v)
            if isinstance(s, tuple):
                s = b'?'
        elif conv == b'c':
            s = bytes([v & 0xff])
        elif conv == b'd':
            s = b'%d' % (v - (1 << 32) if v & 0x80000000 else v)
        else:
            s = b'%x' % v
        width = int(w) if w else 0
        if b'-' in flags:
            s = s.ljust(width)
        elif b'0' in flags and conv in (b'd', b'x'):
            neg = s.startswith(b'-')
            s = (b'-' if neg else b'') + s.lstrip(b'-').rjust(width - neg, b'0')
        else:
            s = s.rjust(width)
        out += s
        i = j + 1
    return out, k


def nconv(f):
    """Number of arguments a format takes."""
    return f.replace(b'%%', b'').count(b'%')


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
        name = 'stub_%x' % addr
        n = sum(1 for e in m.api_log if e[0] == name)
        rets = ST['rets'].get(addr) or [0]
        eax = rets[n % len(rets)] & 0xffffffff
        if addr in EFFECT:
            extra, v2 = EFFECT[addr](m.uc, n, raw, eax, esp)    # m.uc notes the pages it writes
            args += extra
            if v2 is not None:
                eax = v2 & 0xffffffff
        m.api_log.append((name, tuple(args)))
        uc.reg_write(UC_X86_REG_EAX, eax)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def begin(ctx):
    """Installs the stubs and the sine table (once per machine), the x87 state of the game (control word 0x27f)
    and the answers of the stubs for this trial. Must be the first thing a generator does."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w8v', False):
        m._w8v = True
        for addr, nargs in STUBS.items():
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr, nargs), None, addr, addr)
        for name, (addr, nargs) in DSTUBS.items():
            cb = _stub_cb(m, addr, nargs)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[name], m.syms[name])
        # the angle table of 0x10084766: sin of 0..90 degrees in 512 steps
        m.uc.mem_write(0x103719a0, struct.pack('<520f', *[math.sin(math.radians(i * 90.0 / 512)) for i in range(520)]))
        m.snap0 = m.dump()
        m._index()          # re-index now, before this trial writes anything
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['seed'] = r.getrandbits(32)
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}
    for name, (a, _) in DSTUBS.items():
        ST['rets'][a] = [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)]
    ST['short'] = {}
    ST['fread_words'] = {}
    EFFECT[0x100fe830] = e_fread        # address -> per-call flags: answer count - 1 instead of count
    ST['lines'] = []
    ST['bytes'] = []
    ST['heap'] = []
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


# ---- effects ------------------------------------------------------------------------------------
def _short(addr, n, count):
    plan = ST['short'].get(addr, [])
    return (count - 1) & 0xffffffff if n < len(plan) and plan[n] else count


def e_write(uc, n, raw, eax, esp):
    return [bytes(uc.mem_read(raw[1], min(raw[2], 0x800))).hex()], None


def e_fwrite(uc, n, raw, eax, esp):
    size = min((raw[1] * raw[2]) & 0xffffffff, 0x1000)
    return [bytes(uc.mem_read(raw[0], size)).hex()], _short(0x100febc0, n, raw[2])


def e_fread(uc, n, raw, eax, esp):
    size = min((raw[1] * raw[2]) & 0xffffffff, 0x1000)
    data = bytearray((mix(ST['seed'], n, i) >> 7) & 0xff for i in range(size))
    for i in range(0, size, 9):     # a few NUL bytes, so the texts in the data end
        data[i] = 0
    for off, val in ST.get('fread_words', {}).get(n, {}).items():
        if off + 4 <= size:
            data[off:off + 4] = struct.pack('<I', val & 0xffffffff)
    uc.mem_write(raw[0], bytes(data))
    return [], _short(0x100fe830, n, raw[2])


def e_sprintf(uc, n, raw, eax, esp):
    f = ustr(uc, raw[1])
    words = list(struct.unpack('<%dI' % nconv(f), bytes(uc.mem_read(esp + 12, 4 * nconv(f))))) if nconv(f) else []
    s, _ = fmt(uc, f, words)
    uc.mem_write(raw[0], s + b'\0')
    return [s], len(s)


def e_syserr(uc, n, raw, eax, esp):
    f = ustr(uc, raw[1])
    k = nconv(f)
    words = list(struct.unpack('<%dI' % k, bytes(uc.mem_read(esp + 12, 4 * k)))) if k else []
    out = [f]
    pos = 0
    for w in words:
        pos = f.index(b'%', pos) + 1
        while f[pos:pos + 1] in b'-0123456789l':
            pos += 1
        if STACK_LO <= w <= STACK_HI:
            out.append('stack')
        elif f[pos:pos + 1] == b's':
            out.append(ustr(uc, w))
        else:
            out.append(w)
    return out, None


def e_10046ac4(uc, n, raw, eax, esp):
    uc.mem_write(raw[1], struct.pack('<I', mix(ST['seed'], n, 1)))
    uc.mem_write(raw[2], struct.pack('<I', ST['block']))
    return [], None


def e_ae470(uc, n, raw, eax, esp):
    """Reads one byte of the scrambled file: the trial's byte list, then the end."""
    if n < len(ST['bytes']):
        uc.mem_write(raw[1], bytes([ST['bytes'][n]]))
        return [], 1
    return [], 0


def e_fread_bytes(uc, n, raw, eax, esp):
    if n < len(ST['bytes']):
        uc.mem_write(raw[0], bytes([ST['bytes'][n]]))
        return [], 1
    return [], 0


def e_clip(uc, n, raw, eax, esp):
    uc.mem_write(raw[2], struct.pack('<2I', *[ST['clip'][(n + k) % len(ST['clip'])] & 0xffffffff for k in range(2)]))
    return [], None


def e_readline(uc, n, raw, eax, esp):
    """fgets / Input_ReadLine: the trial's next line (cut to the size like fgets), NULL at the end."""
    if n >= len(ST['lines']):
        return [], 0
    line = ST['lines'][n][:max(raw[1] - 1, 0)] if raw[1] < 0x80000000 else ST['lines'][n]
    uc.mem_write(raw[0], line + b'\0')
    return [], raw[0]


def e_ba7d5(uc, n, raw, eax, esp):
    """The resource finder (and 0x100b9203) upper-cases its two texts in place; the finder writes the size when asked."""
    for a in raw[:2]:
        s = ustr(uc, a)
        if not isinstance(s, tuple):
            uc.mem_write(a, s.upper())
    if len(raw) > 2 and raw[2]:
        uc.mem_write(raw[2], struct.pack('<I', mix(ST['seed'], n, 3)))
    return [], None


EFFECT.update({0x10105750: e_write, 0x100febc0: e_fwrite, 0x100fe830: e_fread, 0x100fc780: e_sprintf,
               0x100bc280: e_syserr, 0x10046ac4: e_10046ac4, 0x100ae470: e_ae470, 0x1008e9af: e_clip,
               0x1008e266: e_clip, 0x100fe770: e_readline, 0x10053b64: e_readline, 0x100ba7d5: e_ba7d5,
               0x100b9203: e_ba7d5})


# ---- 0x100470c7 .. 0x1004e886 -------------------------------------------------------------------------
def g_rec_write(ctx):
    r = begin(ctx)
    rets(0x100474db, cstr(ctx, b'C:\\HG\\' + text(ctx, r.randrange(0, 12))))
    rets(0x10109980, r.choice([3, 4, 0xffffffff, 0, 7]))
    rets(0x10105750, r.choice([0x17, 0x6c, 0, 0xffffffff]))
    return [cstr(ctx, text(ctx, r.randrange(0, 12))), newbuf(ctx, 0x80)]


def g_712a(ctx):
    r = begin(ctx)
    rets(0x100474db, cstr(ctx, b'C:\\HG\\' + text(ctx, r.randrange(0, 12))))
    rets(0x10046ac4, r.choice([5, 6, 0xffffffff, 0xffffffff, 0]))
    ST['block'] = newbuf(ctx, 0x40)
    w32(ctx, 0x1015ec58, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x1017b8f8, ctx.u32())
    wb(ctx, 0x1013b2c0, rb(ctx, 9) if r.random() < 0.3 else b'mcga.dll\0')
    ST['heap'] = [newbuf(ctx, 0x70)]
    return [cstr(ctx, text(ctx, r.randrange(0, 12))), newbuf(ctx, 8)]


def a_heapalloc(n, args):
    h = ST.get('heap') or [0]
    return h[n % len(h)]


HEAP_API = {'HeapAlloc': a_heapalloc}


def g_tga(ctx):
    r = begin(ctx)
    rets(0x100fd820, r.choice([STREAM, STREAM, STREAM, 0]))
    w = r.choice([0, 1, 2, 3, 5, -1])
    h = r.choice([0, 1, 2, 4, -2])
    stride = 2 * max(w, 0) + r.choice([0, 2, 6])
    pix = newbuf(ctx, max(h, 0) * stride + 16)
    for a in range(0x109e3890, 0x109e38b0, 4):
        w32(ctx, a, r.choice([0xf800, 0x7e0, 0x1f, 0x7c00, 0x3e0, ctx.u32()]) if a < 0x109e38a0 else r.randrange(0, 12))
    ST['short'][0x100febc0] = [r.random() < 0.1 for _ in range(30)]
    return [cstr(ctx, text(ctx, r.randrange(1, 12))), w, h, stride, pix]


def g_deb3(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015f218, r.randrange(0, 700))
    w32(ctx, 0x1015f21c, r.randrange(0, 500))
    w32(ctx, 0x1015f34c, r.choice([0, 2, 4, 16, 256, ctx.u32()]))
    w32(ctx, 0x1015f360, newbuf(ctx, 16))
    rets(0x1004db5b, r.choice([0, 0, 0, 2]))
    rets(0x1004e719, r.choice([0, 0, 0, 2]))
    rets(0x1004db86, r.choice([1, 2, 4, 8, 0, ctx.u32()]))
    rets(0x1004dcaa, r.choice([0, 0, 0, 2]), r.choice([0, 0, 2]))
    rets(0x1004e04a, r.choice([0, 0, 1, 2, 3]))
    def arg():
        return r.choice([0, 5, -1, -7, r.randrange(-50, 700), ctx.u32()])
    return [arg(), arg(), arg(), arg(), r.choice([0x10002955, ctx.u32()])]


def g_copy_from(ctx):
    r = begin(ctx)
    rets(0x100ae344, r.choice([newbuf(ctx, 0x20), newbuf(ctx, 0x20), 0]))
    rets(0x100fd820, r.choice([STREAM, STREAM, 0]))
    ST['bytes'] = [r.getrandbits(8) for _ in range(r.choice([0, 1, 3, 10]))]
    EFFECT[0x100fe830] = e_fread_bytes
    rets(0x100ae4bc, *[r.choice([1, 1, 1, 1, 0, 2]) for _ in range(4)])
    rets(0x100fd760, r.choice([0, 0, 0xffffffff]))
    rets(0x100ae68f, r.choice([0, 0, 1]))
    return [cstr(ctx, text(ctx, r.randrange(1, 12))), cstr(ctx, text(ctx, r.randrange(1, 12)))]


def g_copy_to(ctx):
    r = begin(ctx)
    EFFECT[0x100fe830] = e_fread
    rets(0x100ae2e0, r.choice([newbuf(ctx, 0x20), newbuf(ctx, 0x20), 0]))
    rets(0x100fd820, r.choice([STREAM, STREAM, 0]))
    ST['bytes'] = [r.getrandbits(8) for _ in range(r.choice([0, 1, 3, 10]))]
    ST['short'][0x100febc0] = [r.random() < 0.1 for _ in range(12)]
    rets(0x100fd760, r.choice([0, 0, 0xffffffff]))
    rets(0x100ae68f, r.choice([0, 0, 1]))
    return [cstr(ctx, text(ctx, r.randrange(1, 12))), cstr(ctx, text(ctx, r.randrange(1, 12)))]


def g_8287(ctx):
    r = begin(ctx)
    for i in range(8):
        e = newbuf(ctx, 0x20)
        w32(ctx, e, r.choice([0, 1, 2, 3, ctx.u32()]))
        w32(ctx, e + 4, r.choice([1, 2, 5, 10, r.randrange(1, 100), -3]))
        w32(ctx, e + 8, r.choice([1, 2, 3, 7, 30, -2, r.randrange(1, 1000)]))
        w32(ctx, e + 0xc, r.choice([0, 0, r.randrange(0, 5000), ctx.u32()]))
        w32(ctx, e + 0x10, r.randrange(0, 16))
        w32(ctx, e + 0x18, r.choice([0, 0, ctx.u32() | 1]))
        w32(ctx, 0x109b94e0 + 4 * i, e if r.random() < 0.9 else 0)
    w32(ctx, 0x1015dc78, r.choice([r.randrange(0, 6000), ctx.u32()]))
    rets(0x100282d4, r.choice([0, 0x4321, ctx.u32() | 1]))
    rets(0x100ed8c3, r.choice([1, 3, 8, 20, -1]))
    return [ctx.u32(), r.randrange(0, 8), ctx.u32(), ctx.u32()]


def g_8c84(ctx):
    r = begin(ctx)
    self_id = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_id)
    rec = newbuf(ctx, 0x140)
    wb(ctx, rec + 0x124, text(ctx, r.randrange(0, 12)) + b'\0')
    w32(ctx, 0x109e8b40 + 4 * self_id, rec)
    w32(ctx, 0x10151d50, r.choice([-4, -3, -2, -1, -1, -1, 0, 1, 5, self_id]))
    n = r.choice([0, 1, 5, 0x26, 0x27, 0x28, r.randrange(0, 0x28)])
    w32(ctx, 0x1015f1f4, n)
    wb(ctx, 0x1015dba0, text(ctx, n) + b'\0' + rb(ctx, 0x27 - min(n, 0x27)))
    w32(ctx, 0x109b98c0, ctx.u32())
    c = r.choice([8, 0x1b, 0xd, 0x65, 0x66, 0x61, 0x41, 0x20, 0x1f, 0x2b, 0x5b, 0x7d, 0x7e, 0x80, 0xff, r.getrandbits(8)])
    key = c | r.choice([0, 0, 0x100, 0x200, 0x300, 0x400, 0x500]) | (r.getrandbits(32) & 0xfffff800 if r.random() < 0.1 else 0)
    return [key]


def g_b710(ctx):
    begin(ctx)
    return []


# ---- 0x10050e7e .. 0x100517dc: HUD -------------------------------------------------------------------
def rect(ctx):
    r = ctx.rng
    a = newbuf(ctx, 0x14)
    x, y = r.randrange(-20, 400), r.randrange(-20, 300)
    w32(ctx, a + 4, x, y, x + r.choice([0, 1, 31, 63, 100, -3]), y + r.choice([0, 1, 31, 63, 80, -3]))
    return a


def hud_common(ctx):
    r = ctx.rng
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    ST['clip'] = [r.randrange(-50, 500) for _ in range(5)]
    self_id = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_id)
    rec = newbuf(ctx, 0x40)
    blk = newbuf(ctx, 0x600)
    w32(ctx, rec + 0x28, blk)
    w32(ctx, 0x109e8b40 + 4 * self_id, rec)
    return blk


def angle(ctx):
    r = ctx.rng
    return fbits(r.choice([r.uniform(-720, 720), r.uniform(0, 360), float(r.randrange(-360, 720, 15)), 0.0, 90.0,
                           -90.0, 359.99, 1e-3]))


def g_0e7e(ctx):
    r = begin(ctx)
    hud_common(ctx)
    return [rect(ctx), angle(ctx), r.randrange(0, 24), rect(ctx) if r.random() < 0.6 else 0]


def g_10d9(ctx):
    r = begin(ctx)
    blk = hud_common(ctx)
    w32(ctx, blk + 0x4e8, r.choice([fbits(r.uniform(-50, 200)), fbits(float(r.randrange(0, 200))), rfl(ctx)]))
    g = newbuf(ctx, 0x100)
    w32(ctx, g + 0xec, rect(ctx))
    mx = r.choice([100, 1, 50, 360, 1000, -100, r.randrange(1, 40000)])
    w32(ctx, g + 0x94, mx)
    other = newbuf(ctx, 0x100)
    w32(ctx, other + 0xec, rect(ctx) if r.random() < 0.7 else 0)
    w32(ctx, 0x109e8948, other)
    rets(0x10047e95, r.choice([0, 1, mx // 2, mx - 1, mx, mx + 3, -5, r.randrange(-100, abs(mx) + 50)]))
    return [g]


def g_1372(ctx):
    r = begin(ctx)
    blk = hud_common(ctx)
    w32(ctx, blk + 0x4e8, r.choice([fbits(r.uniform(-50, 200)), rfl(ctx)]))
    g = newbuf(ctx, 0x100)
    w32(ctx, g + 0xec, ctx.u32())
    width = r.choice([1, 2, 10, 33, 100, 0, -4])
    w32(ctx, g + 0x7c, r.randrange(0, 600), r.randrange(0, 400), width, r.randrange(0, 20))
    rets(0x10047e95, r.choice([0, -1, 1, width // 2, width // 2 - 1, width - 1, width, width + 1,
                               r.randrange(-5, abs(width) + 5)]))
    return [g]


def g_1532(ctx):
    r = begin(ctx)
    blk = hud_common(ctx)
    wf(ctx, blk + 0x4e0, r.uniform(-5000, 5000))
    w32(ctx, blk + 0x4ec, r.choice([fbits(r.uniform(-3, 3)), rfl(ctx)]))
    if r.random() < 0.1:
        w32(ctx, blk + 0x4e0, rfl(ctx))
    w32(ctx, 0x1015dc80, r.choice([r.randrange(0, 100000), ctx.u32()]))
    g = newbuf(ctx, 0x100)
    w32(ctx, g + 0xec, ctx.u32())
    w32(ctx, g + 0x9c, r.randrange(0, 600), r.randrange(0, 400), r.choice([0, 1, 50, 100, 768, -3]), r.randrange(0, 20))
    rets(0x10047e95, r.choice([0, 1, -3, 0x2ff, 0x300, 0x301, 0x5ff, r.randrange(-10, 0x700)]))
    return [g]


def g_16c8(ctx):
    r = begin(ctx)
    blk = hud_common(ctx)
    info = newbuf(ctx, 0x60)
    w8(ctx, info + 0x59, r.choice([0, 0, 1, 0x80]))
    w32(ctx, info + 8, r.choice([fbits(r.uniform(-0.01, 0.03)), fbits(r.uniform(-1, 1)), rfl(ctx)]))
    head = newbuf(ctx, 0x60)
    w32(ctx, head + 0x54, info)
    w32(ctx, blk, head)
    g = newbuf(ctx, 0x100)
    return [g]


def g_17dc(ctx):
    r = begin(ctx)
    blk = hud_common(ctx)
    w32(ctx, blk + 0x510, r.choice([0, 5, 100, -1, ctx.u32()]))
    g = newbuf(ctx, 0x100)
    w32(ctx, g + 0x68, r.choice([0, 10, 100, 724, -5, r.randrange(0, 2000)]))
    rets(0x10047e95, r.choice([0, 10, 700, -800, r.randrange(-1000, 2000)]))
    return [g]


# ---- 0x100531e0 .. 0x10053323: HG.INI ----------------------------------------------------------------
def ini_lines(ctx, names):
    r = ctx.rng
    out = []
    for _ in range(r.choice([0, 1, 3, 6, 10])):
        k = r.random()
        nm = r.choice(names)
        if k < 0.25:
            out.append(r.choice([b'', b' ', b'\t']) + b'[' + r.choice([b'', b' ']) + nm + r.choice([b'', b' ']) + b']\n')
        elif k < 0.6:
            out.append(r.choice([b'', b'  ']) + nm + r.choice([b'', b' ']) + b'=' + r.choice([b'', b' ']) +
                       text(ctx, r.randrange(0, 10)) + r.choice([b'', b' ;comment', b';x']) + r.choice([b'\n', b'']))
        elif k < 0.7:
            out.append(b'[' + nm + b'\n')
        elif k < 0.8:
            out.append(text(ctx, r.randrange(0, 0x90)) + b'\n')
        else:
            out.append(b'; ' + text(ctx, r.randrange(0, 20)) + b'\n')
    return out


def g_31e0(ctx):
    r = begin(ctx)
    names = [b'Video', b'video', b'SOUND', b'net', b'Game']
    ST['lines'] = ini_lines(ctx, names)
    rets(0x100fd820, r.choice([STREAM, STREAM, STREAM, 0]))
    rets(0x100fe970, r.choice([0, 0x123, ctx.u32()]))
    w32(ctx, 0x1013f10c, ctx.u32())
    return [cstr(ctx, r.choice([b'', b' ', b'  ']) + r.choice(names) + r.choice([b'', b' ', b'\t']))]


def g_3323(ctx):
    r = begin(ctx)
    names = [b'width', b'Width', b'height', b'mode', b'x']
    ST['lines'] = ini_lines(ctx, names)
    rets(0x100fd820, r.choice([STREAM, STREAM, STREAM, 0]))
    w32(ctx, 0x1013f10c, r.choice([0xffffffff, 0, 0x40, ctx.u32()]))
    wb(ctx, 0x1015f440, rb(ctx, 4))
    wb(ctx, 0x1015f444, text(ctx, r.randrange(0, 8)) + b'\0')
    return [cstr(ctx, r.choice([b'', b' ']) + r.choice(names) + r.choice([b'', b' '])), r.choice([0, 0, 1, 2])]


# ---- 0x10053fe1 .. 0x10054a79: input mapping -----------------------------------------------------------
def g_3fe1(ctx):
    r = begin(ctx)
    n = r.choice([0, 1, 3, 10, 0x31, 0x32, 0x33])
    w32(ctx, 0x1015fde8, n)
    ident = r.choice([1, 7, ctx.u32()])
    for i in range(min(n, 0x34)):
        base = 0x1015fe00 + i * 0x80
        sink = newbuf(ctx, 8)
        w32(ctx, sink, r.choice([ident, ident, 3, ctx.u32()]))
        w32(ctx, base + 0x14, sink if r.random() < 0.8 else 0)
        w32(ctx, base + 8, r.choice([0, 0, ctx.u32()]))
    w32(ctx, 0x1015fdf8, r.randrange(0, 100))
    src = newbuf(ctx, 8)
    w32(ctx, src, cstr(ctx, r.choice([b'torso_pan', b'torso_tilt', b'menu_up', b'menu', b'throttle', b'torso_pant'])),
        ident if r.random() < 0.5 else ctx.u32())
    rets(0x1005388b, r.choice([0, 1, 2, 5, 0xffffffff]))
    rets(0x10053c40, newbuf(ctx, 8))
    dev = r.choice([0, cstr(ctx, b'joystick'), cstr(ctx, b'keyboard')])
    return [src, dev, r.choice([0, 1, 3, 15, ctx.u32()])]


def chord_line(ctx):
    r = ctx.rng
    k = r.random()
    if k < 0.65:
        return (r.choice([b' ', b'', b'\t']) + r.choice([b'+', b'-']) + b' ' + r.choice([b'kbd', b'joy', b'x']) + b' ' +
                r.choice([b'a', b'ctrl', b'b1', text(ctx, r.randrange(1, 70), b'abcdef')]) + b'\n')
    if k < 0.75:
        return b'}' + text(ctx, 3) + b'\n'
    if k < 0.85:
        return b'+ kbd\n'
    if k < 0.9:
        return b'* kbd a\n'
    return b' - ' + text(ctx, r.randrange(1, 80), b'xyz') + b' ' + text(ctx, r.randrange(1, 120), b'pq') + b'\n'


def g_41e9(ctx):
    r = begin(ctx)
    ST['lines'] = [chord_line(ctx) for _ in range(r.choice([0, 1, 3, 8, 9, 12]))]
    rets(0x1005388b, r.randrange(0, 6), r.randrange(0, 6), r.choice([0, 1, 0xffffffff]))
    rets(0x10053a4f, *[r.choice([0xffffffff, 0, 1, 31, 32, 33, 100, 255]) for _ in range(4)])
    w32(ctx, 0x1015fdf8, r.randrange(0, 100))
    count = newbuf(ctx, 4)
    out = newbuf(ctx, 12 * 10)
    return [STREAM, count, out]


def g_48aa(ctx):
    r = begin(ctx)
    n = r.choice([0, 1, 2, 4, 6])
    w32(ctx, 0x10164bc0, n)
    for i in range(n):
        w32(ctx, 0x1015f868 + 4 * i, r.choice([0, 1, 1, ctx.u32()]))
        wb(ctx, 0x1015fc58 + i * 0x28, r.choice([b'joy', b'kbd', b'mouse']) + r.choice([b'', b'1', b'12', b'007']) + b'\0')
    w32(ctx, 0x1015965c, r.choice([1, 1, 2]))
    rets(0x100fd820, *[r.choice([STREAM, STREAM, 0]) for _ in range(5)])
    return []


# 0x10054a79 (gamekey.map reader) is left out: strtok keeps a pointer into that function's stack buffer in
# 0x1017e884, which cannot be equal for the C version. The generator below was used to find that out.
def gamekey_line(ctx):
    r = ctx.rng
    m = ctx.m.uc
    k = r.randrange(0, 0xc4) if r.random() < 0.9 else 0xc4
    name = ustr(m, ur32(m, 0x1013ff00 + k * 5)) if k < 0xc4 else b'nokey'
    keys = []
    for _ in range(r.choice([1, 1, 2, 3])):
        j = r.randrange(0, 0x1b)
        keys.append(r.choice([ustr(m, ur32(m, 0x101402d8 + j * 6)), bytes([r.choice(b'aZ09+')]), bytes([r.randrange(0x80, 0x100)]),
                              b'q']))
    seq = b'+'.join(keys)
    if r.random() < 0.05:
        return name + b'\n'
    if r.random() < 0.05:
        seq = b'+'
    return name + b' ' + seq + b'\n'


def g_4a79(ctx):
    r = begin(ctx)
    ST['lines'] = [gamekey_line(ctx) for _ in range(r.choice([0, 1, 2, 4, 8]))]
    rets(0x100fd820, r.choice([STREAM, STREAM, STREAM, 0]))
    tab = bytearray(0x200)
    for _ in range(r.choice([0, 3, 30])):
        tab[r.randrange(0, 0x200)] = r.getrandbits(8)
    wb(ctx, 0x10164ac8, bytes(tab))
    return []


# ---- 0x100599a0 .. 0x1005f949 ---------------------------------------------------------------------------
def g_99a0(ctx):
    r = begin(ctx)
    name = text(ctx, r.randrange(0, 12), b'abcXY09_') + r.choice([b'', b'.gdf', b'.x.y', b'.'])
    rets(0x100ba7d5, r.choice([0, newbuf(ctx, 16), newbuf(ctx, 16)]))
    return [cstr(ctx, name)]


def g_9e5a(ctx):
    r = begin(ctx)
    rets(0x100fd820, r.choice([STREAM, STREAM, STREAM, 0]))
    n = r.choice([0, 1, 2, 3, -1])
    head = newbuf(ctx, 0x88)
    w32(ctx, head + 0x84, n)
    items = newbuf(ctx, 0x438 * max(n, 1))
    for i in range(max(n, 0)):
        for off in (0x54, 0x78):
            w32(ctx, items + i * 0x438 + off, r.choice([0, items + r.randrange(0, max(n, 1)) * 0x438,
                                                         items + r.randrange(0, 0x900), ctx.u32()]))
    tail = newbuf(ctx, 0x3c)
    k = r.choice([0, 1, 3])
    w32(ctx, tail + 0x24, k)
    data = newbuf(ctx, 0x6c * max(k, 1))
    size = r.choice([0, 1, 7, 40, -3])
    extra = newbuf(ctx, max(size, 1))
    rets(0x100fd760, r.choice([0, 0xffffffff]))
    return [cstr(ctx, text(ctx, r.randrange(1, 12))), head, items, tail, data, size, extra]


def g_a86f(ctx):
    r = begin(ctx)
    rets(0x100fd820, r.choice([STREAM, STREAM, STREAM, 0]))
    ST['fread_words'] = {0: {0x84: r.choice([0, 1, 3, ctx.u32()])}}
    return [cstr(ctx, text(ctx, r.randrange(1, 12))), zbuf(ctx, 0x100), newbuf(ctx, 4)]


def g_af72(ctx):
    r = begin(ctx)
    rets(0x100fd820, r.choice([STREAM, STREAM, STREAM, 0]))
    rec = newbuf(ctx, 0x1b0)
    k = r.choice([0, 1, 3])
    w32(ctx, rec + 0x12d, k)
    w32(ctx, rec + 0x131, newbuf(ctx, 0x48 * max(k, 1)))
    rets(0x100fd760, r.choice([0, 0xffffffff]))
    return [rec, cstr(ctx, text(ctx, r.randrange(0, 10))), cstr(ctx, text(ctx, r.randrange(0, 10)))]


def g_log(ctx):
    r = begin(ctx)
    rets(0x100fd820, r.choice([STREAM, STREAM, 0]))
    w32(ctx, 0x101668f8, r.choice([0, STREAM, ctx.u32()]))
    rets(0x100ff500, ctx.u32())
    rets(0x100ff4e0, cstr(ctx, b'Sun Oct 20 12:00:00 2026\n'))
    return []


# ---- 0x10060b76 .. 0x10061bc5 ---------------------------------------------------------------------------
def quat(ctx):
    r = ctx.rng
    k = r.random()
    if k < 0.6:
        v = [r.uniform(-1, 1) for _ in range(4)]
        n = math.sqrt(sum(x * x for x in v)) or 1.0
        return [x / n * r.choice([1.0, 1.0, 0.999, 3.0]) for x in v]
    if k < 0.8:
        return [r.uniform(-1, 1) * 10 ** r.randint(-5, 2) for _ in range(4)]
    return [r.choice([0.0, -0.0, 1.0, 1e-5, 2e-4]) for _ in range(4)]


def g_0b76(ctx):
    r = begin(ctx)
    q = newbuf(ctx, 16)
    if r.random() < 0.95:
        wf(ctx, q, *quat(ctx))
    else:
        w32(ctx, q, *[rfl(ctx) for _ in range(4)])
    return [q if r.random() < 0.98 else 0, newbuf(ctx, 0x30)]


def g_0d2e(ctx):
    r = begin(ctx)
    m = newbuf(ctx, 0x30)
    x, y, z, w = quat(ctx)
    mat = [1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w),
           2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w),
           2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)]
    if r.random() < 0.3:
        mat = [r.uniform(-2, 2) for _ in range(9)]
    wf(ctx, m, *[v + r.choice([0, 0, 1e-6, -1e-3]) for v in mat])
    if r.random() < 0.05:
        w32(ctx, m + 4 * r.randrange(0, 9), rfl(ctx))
    return [m if r.random() < 0.98 else 0, newbuf(ctx, 16)]


def g_19a0(ctx):
    r = begin(ctx)
    cam = newbuf(ctx, 0x40)
    w32(ctx, cam + 0x18, r.choice([0, 1, ctx.u32()]))
    w32(ctx, 0x1014cae0, cam)
    for a in (0x1014cb04, 0x1014cb08, 0x1014cb1c):
        w32(ctx, a, r.choice([0, 0, 1]))
    w32(ctx, 0x1014c9fc, ctx.u32())
    w32(ctx, 0x1017e650, ctx.u32())
    n = r.choice([0, 1, 3])
    for i in range(n):
        w32(ctx, 0x1036f2a0 + 4 * i, ctx.u32() | 1)
    w32(ctx, 0x1036f2a0 + 4 * n, 0)
    view = newbuf(ctx, 0x40)
    return [r.randrange(0, 5), ctx.u32(), view, r.choice([0, ctx.u32()]), r.choice([0, ctx.u32()])]


def g_1bc5(ctx):
    r = begin(ctx)
    for k in (0x10, 0x11):
        wb(ctx, 0x10185620 + k * 16, text(ctx, r.randrange(0, 12), b'abcXY09') + b'\0')
    w32(ctx, 0x1014ced4, cstr(ctx, r.choice([b'wtb', b'gdf', b'TLT'])))
    rets(0x100ba7d5, r.choice([0, newbuf(ctx, 8)]))
    return [r.choice([0, 1, ctx.u32()])]


SPECS = [
    dict(name='Sim_100470c7', orig=0x100470c7, c='Sim_100470c7', gen=g_rec_write, trials=300),
    dict(name='Sim_1004712a', orig=0x1004712a, c='Sim_1004712a', gen=g_712a, trials=400, api=HEAP_API),
    dict(name='Sim_1004724c', orig=0x1004724c, c='Sim_1004724c', gen=g_rec_write, trials=300),
    dict(name='Screenshot_WriteTga', orig=0x100472af, c='Screenshot_WriteTga', gen=g_tga, trials=400),
    dict(name='Sim_10048287', orig=0x10048287, c='Sim_10048287', gen=g_8287, trials=800, void=True),
    dict(name='FileRec_CopyFromFile', orig=0x10048560, c='FileRec_CopyFromFile', gen=g_copy_from, trials=400),
    dict(name='FileRec_CopyToFile', orig=0x10048613, c='FileRec_CopyToFile', gen=g_copy_to, trials=400),
    dict(name='Chat_HandleKey', orig=0x10048c84, c='Chat_HandleKey', gen=g_8c84, trials=1500),
    dict(name='Gamma_BuildTable', orig=0x1004b710, c='Gamma_BuildTable', gen=g_b710, trials=2, void=True),
    dict(name='Gif_WriteImage', orig=0x1004deb3, c='Gif_WriteImage', gen=g_deb3, trials=600),
    dict(name='Sim_10050e7e', orig=0x10050e7e, c='Sim_10050e7e', gen=g_0e7e, trials=800, void=True),
    dict(name='Sim_100510d9', orig=0x100510d9, c='Sim_100510d9', gen=g_10d9, trials=500, void=True),
    dict(name='Sim_10051372', orig=0x10051372, c='Sim_10051372', gen=g_1372, trials=600, void=True),
    dict(name='Sim_10051532', orig=0x10051532, c='Sim_10051532', gen=g_1532, trials=600, void=True),
    dict(name='Sim_100516c8', orig=0x100516c8, c='Sim_100516c8', gen=g_16c8, trials=500, void=True),
    dict(name='Sim_100517dc', orig=0x100517dc, c='Sim_100517dc', gen=g_17dc, trials=500, void=True),
    dict(name='Ini_FindSection', orig=0x100531e0, c='Ini_FindSection', gen=g_31e0, trials=800),
    dict(name='Ini_GetValue', orig=0x10053323, c='Ini_GetValue', gen=g_3323, trials=800),
    dict(name='Input_AddAnalog', orig=0x10053fe1, c='Input_AddAnalog', gen=g_3fe1, trials=800),
    dict(name='Input_ReadChord', orig=0x100541e9, c='Input_ReadChord', gen=g_41e9, trials=800),
    dict(name='Input_LoadMaps', orig=0x100548aa, c='Input_LoadMaps', gen=g_48aa, trials=500),
    dict(name='Sim_100599a0', orig=0x100599a0, c='Sim_100599a0', gen=g_99a0, trials=400),
    dict(name='Sim_10059e5a', orig=0x10059e5a, c='Sim_10059e5a', gen=g_9e5a, trials=400),
    dict(name='Sim_1005a86f', orig=0x1005a86f, c='Sim_1005a86f', gen=g_a86f, trials=300),
    dict(name='Sim_1005af72', orig=0x1005af72, c='Sim_1005af72', gen=g_af72, trials=300),
    dict(name='Log_Open', orig=0x1005f8d0, c='Log_Open', gen=g_log, trials=200),
    dict(name='Log_Close', orig=0x1005f949, c='Log_Close', gen=g_log, trials=200),
    dict(name='Quat_ToMatrix', orig=0x10060b76, c='Quat_ToMatrix', gen=g_0b76, trials=1500, void=True),
    dict(name='Matrix_ToQuat', orig=0x10060d2e, c='Matrix_ToQuat', gen=g_0d2e, trials=1500, void=True),
    dict(name='Sim_100619a0', orig=0x100619a0, c='Sim_100619a0', gen=g_19a0, trials=400, void=True),
    dict(name='Sim_10061bc5', orig=0x10061bc5, c='Sim_10061bc5', gen=g_1bc5, trials=300, void=True),
]
