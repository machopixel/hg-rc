# Differential tests for round 7, letter n (see difftest.py and docs/checker.md).
#
# Helpers that are not converted yet (or that another hook file patches) are replaced in the emulator by a stub: a
# code hook at the helper's own address appends ('stub_<address>', argument words) to the checker's call log
# (compared between the original and the C run like the Windows calls; stack addresses shown as 'stack', texts
# logged by content where noted), may write results through pointer arguments (the same bytes for both runs), sets
# eax to a value that depends only on the trial and the position in the log, and returns to the caller (cdecl) by
# setting EIP and ESP. Code bytes are not patched. Indirect calls (driver blocks, check functions, the display
# driver) go to 16-byte slots at CODE that are stubbed the same way ('code_<slot>'). Converted callees run as the
# original code (original side) and as their C conversion (C side); the C library sqrt (0x100fca04) runs for real.
# The x87 control word is the game's 0x27f (53-bit precision). No spec uses the float tolerance: the FPU functions
# here must match bit for bit. Not generated: signalling NaNs (the emulator's x87 does not quiet them on fld as the
# processor does) and NaN box values for Sim_10068e77 (the converted rotation / addition helpers it calls can give
# a NaN of the other sign, seen once in 600 trials).
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_1005/w7n_input.c', 'sim/unsorted_1005/w7n_misc.c', 'sim/unsorted_1006/w7n_misc.c',
    'sim/unsorted_1007/w7n_misc.c', 'sim/unsorted_1008/w7n_misc.c',
    # converted code the functions above call (and what that calls)
    'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_1003/bitmap.c',
    'sim/win/w4u_gdi.c', 'sim/unsorted_1010/w6i_crt.c', 'sim/unsorted_100f/w2e_string.c',
    'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1001/w5y_misc.c', 'sim/unsorted_1001/w4u_cd.c',
    'sim/unsorted_1002/w2g_records.c', 'sim/unsorted_1002/w2i_setup.c', 'sim/unsorted_1004/state.c',
    'sim/unsorted_1005/w5y_misc.c', 'sim/unsorted_1005/w2j_misc.c', 'sim/unsorted_1005/w2k_misc.c',
    'sim/unsorted_1005/w2e_lists.c', 'sim/unsorted_1006/w5z_calls.c', 'sim/unsorted_1006/w6e_misc.c',
    'sim/unsorted_1006/w3p_box.c', 'sim/unsorted_1006/w3o_add.c', 'sim/unsorted_1006/w3t_misc.c',
    'sim/unsorted_1007/w3o_load.c', 'sim/unsorted_1007/w6e_misc.c', 'sim/unsorted_1007/w2f_misc.c',
    'sim/unsorted_1007/w2e_state.c', 'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_1008/w4w_display.c',
    'sim/unsorted_1008/w5b_calls.c', 'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1008/w3r_grid.c',
    'sim/unsorted_1008/w5x_calls.c', 'sim/view/objlist.c', 'sim/view/w2l_rects.c', 'sim/unsorted_100a/w5x_calls.c',
    'sim/util/field.c', 'sim/unsorted_100d/w2i_select.c', 'sim/unsorted_100d/w2h_misc.c',
    'sim/unsorted_1010/w6k_text.c', 'sim/unsorted_100f/w2j_str.c', 'sim/unsorted_100f/w6i_misc.c',
    'sim/unsorted_100f/w2g_crt.c', 'sim/unsorted_100f/w6i_stream.c', 'sim/win/w4u_cdaudio.c',
    'sim/unsorted_100f/w6i_mem.c', 'sim/unsorted_1003/w2e_mono.c', 'sim/view/viewrect.c',
    'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2e_names.c', 'sim/unsorted_1000/w5x_calls.c',
    'sim/unsorted_1000/w2a_fields.c', 'sim/unsorted_1000/w2l_fields.c', 'sim/unsorted_1001/w2j_misc.c',
    'sim/unsorted_1001/w2e_tables.c', 'sim/unsorted_1002/w5y_misc.c', 'sim/unsorted_1002/w5z_calls.c',
    'sim/core/w2a_timers.c', 'sim/unsorted_1003/w2e_entries.c', 'sim/unsorted_1003/slots.c',
    'sim/unsorted_1004/w5y_misc.c', 'sim/unsorted_1004/w2e_strings.c', 'sim/unsorted_1004/w3o_misc.c',
    'sim/unsorted_1004/w2k_flags.c', 'sim/unsorted_1004/w2l_values.c', 'sim/unsorted_1006/w5a_math.c',
    'sim/unsorted_1006/w5b_calls.c', 'sim/unsorted_1006/w5c_math.c', 'sim/unsorted_1007/w2j_misc.c',
    'sim/unsorted_1008/w2l_grid.c', 'sim/unsorted_1008/cell_link.c', 'sim/view/w2l_lists.c',
    'sim/unsorted_1008/w2g_rect.c', 'sim/util/ring.c', 'sim/unsorted_100a/w5a_misc.c', 'sim/unsorted_100a/w2k_list.c',
    'sim/util/lookup.c', 'sim/unsorted_100d/state.c', 'sim/unsorted_1010/w2i_crt.c', 'sim/core/timer.c',
    'sim/unsorted_100f/w2h_mem.c', 'sim/unsorted_1000/w2j_slots.c', 'sim/unsorted_1001/w2e_names.c',
    'sim/unsorted_1002/w3o_vectors.c', 'sim/unsorted_1004/fields.c', 'sim/unsorted_100a/w5b_calls.c',
    'sim/unsorted_100c/w2e_tables.c', 'sim/unsorted_1010/w2h_misc.c', 'sim/unsorted_1002/w2h_misc.c',
    'sim/unsorted_1008/w3s_grid.c', 'sim/unsorted_100c/w2i_misc.c', 'sim/view/w2d_view.c', 'sim/view/matrix.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
CODE = 0x00AF8000       # targets of indirect calls: 16-byte slots with a logging hook

# helper address -> number of argument words logged (a spec may change it per trial in ST['nargs'])
STUBS = {
    0x100bc280: 3,      # system error report (patched by hooks_sim.c), varargs
    0x1005573d: 3,      # input error report
    0x100fc780: 3,      # sprintf (varargs): writes a text made from its arguments
    0x100ff280: 1,      # character mapping
    0x1005ab44: 6,
    0x100fd820: 3,      # fopen core: answers a stream made by the generator
    0x10105a60: 1,      # stream refill: end of file
    0x100fd760: 1,      # fclose
    0x10074cea: 2, 0x10075667: 1, 0x100782a9: 1, 0x100527fa: 5,
    0x100381d0: 1, 0x100766ca: 1, 0x100e46d0: 2,
    0x100ef727: 7, 0x100e86a1: 7,       # line drawing (reached through Sim_100a2b8c)
    0x100a9ba7: 1,                      # object update before its angles are read (Sim_100a9ca3)
    0x10060681: 4,                      # angles of an object: writes three dwords through its pointer arguments
    0x100bb2b0: 2,                      # release (Sim_10075124)
    0x10106e50: 0,                      # heap walk of the C library (Heap_Check)
    0x1000e737: 12,                     # sound request (Sim_1000ebfb)
    0x100d3510: 3,                      # (Sim_10061b62)
}
SARGS = {0x100fc780: (1,)}      # helper -> arguments that point to a text: logged by content
EFFECT = {}         # helper -> function(uc, call number, raw arguments) that writes the helper's results
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def ur32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def ustr(uc, addr, limit=200):
    out = b''
    while len(out) < limit:
        c = bytes(uc.mem_read(addr + len(out), 1))
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
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rb(ctx, extra))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def rfl(ctx, lo=-1000.0, hi=1000.0):
    """A float as its bits: ordinary values, zeros, small and big magnitudes, both signs, quiet NaNs and
    infinities now and then (no signalling NaN: the emulator's x87 does not quiet one on fld as the processor
    does)."""
    r = ctx.rng
    c = r.random()
    if c < 0.5:
        return fbits(r.uniform(lo, hi))
    if c < 0.65:
        return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 2.0, 100.0, -100.0, 1e-7, 5000.0]))
    if c < 0.88:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-12, 12))
    if c < 0.92:
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000, 0x7fc12345])
    v = r.getrandbits(32)
    if (v & 0x7f800000) == 0x7f800000:
        v |= 0x00400000
    return v


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


# ---- stubs --------------------------------------------------------------------------------------------
def _log_args(uc, m, tag, raw, sargs=()):
    args = []
    for k, v in enumerate(raw):
        if k in sargs:
            args.append(ustr(uc, v))
        else:
            args.append('stack' if STACK_LO <= v <= STACK_HI else v)
    m.api_log.append((tag, tuple(args)))


def _ret(uc, value):
    esp = uc.reg_read(UC_X86_REG_ESP)
    uc.reg_write(UC_X86_REG_EAX, value & 0xffffffff)
    uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
    uc.reg_write(UC_X86_REG_ESP, esp + 4)


def _stub_cb(m, addr):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        nargs = ST.get('nargs', {}).get(addr, STUBS[addr])
        raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
        n = len(m.api_log)
        _log_args(uc, m, 'stub_%x' % addr, raw, SARGS.get(addr, ()))
        if addr in EFFECT:
            EFFECT[addr](uc, n, raw)
        if addr in ST.get('fn', {}):
            _ret(uc, ST['fn'][addr](raw))
            return
        rets = ST['rets'].get(addr) or [0]
        _ret(uc, rets[n % len(rets)])
    return cb


def _code_cb(m):
    def cb(uc, address, size, ud):
        k = (address - CODE) // 16
        esp = uc.reg_read(UC_X86_REG_ESP)
        nargs = ST['code'].get(k, {}).get('nargs', 2)
        raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
        n = len(m.api_log)
        _log_args(uc, m, 'code_%d' % k, raw)
        c = ST['code'].get(k, {})
        if 'eff' in c:
            c['eff'](uc, n, raw)
        rets = c.get('rets') or [mix(ST['seed'], k, n)]
        _ret(uc, rets[n % len(rets)])
    return cb


def begin(ctx):
    """Installs the stubs (once per machine), the x87 state of the game (control word 0x27f) and the answers of the
    stubs for this trial. Must be the first thing a generator does."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w7n', False):
        m._w7n = True
        for addr in STUBS:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr), None, addr, addr)
        m.raw.hook_add(UC_HOOK_CODE, _code_cb(m), None, CODE, CODE + 16 * 64 - 1)
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['seed'] = r.getrandbits(32)
    ST['nargs'] = {}
    ST['code'] = {}
    ST['fn'] = {0x100ff280: lambda raw: (raw[0] | 0x20) if 0x41 <= (raw[0] & 0xff) <= 0x5a and raw[0] < 0x100
                else raw[0]}       # the C library tolower for ASCII
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def code(ctx, k, nargs, rets=None, eff=None):
    """Address of indirect-call slot k (a `ret`, stubbed by the logging hook)."""
    a = CODE + 16 * k
    ctx.m.uc.mem_write(a, b'\xc3')
    ST['code'][k] = dict(nargs=nargs)
    if rets is not None:
        ST['code'][k]['rets'] = list(rets)
    if eff is not None:
        ST['code'][k]['eff'] = eff
    return a


def e_sprintf(uc, n, raw):
    """sprintf writes a short text made from its format (by content) and the next two argument words."""
    fmt = ustr(uc, raw[1], 12)
    text = b'<' + fmt.replace(b'%', b'') + b'|%x|%x>' % (raw[2] if len(raw) > 2 else 0, mix(ST['seed'], n) & 0xfff)
    uc.mem_write(raw[0], text[:0x2f] + b'\0')


EFFECT[0x100fc780] = e_sprintf


# ---- input devices (0x1005366e .. 0x10053c40) ------------------------------------------------------------
DEVNAMES = [b'keyboard', b'joystick1', b'joystick2', b'mouse', b'Keyboard', b'JOYSTICK1', b'pad']


def ctype_mode(ctx):
    w32(ctx, 0x1015965c, ctx.rng.choice([1, 1, 2]))
    w32(ctx, 0x1017e93c, 2)


def device_names(ctx):
    r = ctx.rng
    ST['devnames'] = [r.choice(DEVNAMES) for _ in range(4)]


def e_devname(uc, n, raw):
    """A driver's name(index, record) writes one of the trial's device names into the record."""
    names = ST['devnames']
    nm = names[mix(ST['seed'], n, raw[0]) % len(names)]
    uc.mem_write(raw[1], nm + b'\0')


def drivers(ctx):
    """The three driver blocks of 0x1013f150 (some missing) with stubbed functions."""
    r = ctx.rng
    device_names(ctx)
    for d in range(3):
        if r.random() < 0.2:
            w32(ctx, 0x1013f150 + 4 * d, 0)
            continue
        blk = newbuf(ctx, 0x20)
        w32(ctx, blk,
            code(ctx, 4 * d, 0, [r.choice([0, 1, 2, 3, 4, 0xffffffff])]),
            code(ctx, 4 * d + 1, 2, [r.choice([0, 0, 0, 1]) for _ in range(3)], e_devname),
            code(ctx, 4 * d + 2, 1, [r.choice([0, 0, 1, 0xffffffff])]),
            code(ctx, 4 * d + 3, 1))
        w32(ctx, 0x1013f150 + 4 * d, blk)


def device_table(ctx, count):
    r = ctx.rng
    w32(ctx, 0x10164bc0, count)
    for i in range(6):
        nm = r.choice(DEVNAMES) if i < count else rb(ctx, 4)
        wb(ctx, 0x1015fc58 + 0x28 * i, nm + b'\0')
        w32(ctx, 0x1015f868 + 4 * i, r.choice([0, 1, 1, ctx.u32()]))
        w32(ctx, 0x1015fd20 + 4 * i, ctx.u32())


def g_366e(ctx):
    r = begin(ctx)
    drivers(ctx)
    device_table(ctx, r.choice([0, 0, 1, 2, 3, 4, 5]))
    w32(ctx, 0x1013f144, ctx.u32())
    return [cstr(ctx, r.choice(DEVNAMES))]


def g_388b(ctx):
    r = begin(ctx)
    drivers(ctx)
    device_table(ctx, r.choice([0, 1, 2, 3, 4, 5]))
    w32(ctx, 0x1013f144, ctx.u32())
    w32(ctx, 0x1015fdf4, ctx.u32())
    return [cstr(ctx, r.choice(DEVNAMES + [b'keyboard', b'KEYBOARD']))]


CHANNELS = [b'x_axis', b'y_axis', b'button1', b'Button1', b'hat', b'', b'throttle']


def g_channel(off_list, off_count):
    def gen(ctx):
        r = begin(ctx)
        ctype_mode(ctx)
        ST['nargs'][0x100bc280] = 5
        w32(ctx, 0x1015fdf8, ctx.u32())
        for dev in range(5):
            rec = 0x1015f880 + dev * 0x74
            if r.random() < 0.15:
                w32(ctx, rec + off_list, 0)
                w32(ctx, rec + off_count, ctx.u32())
                continue
            n = r.choice([0, 1, 2, 4, 6])
            lst = newbuf(ctx, 4 * 8)
            for i in range(8):
                w32(ctx, lst + 4 * i, cstr(ctx, r.choice(CHANNELS)) if r.random() < 0.85 else 0)
            w32(ctx, rec + off_list, lst)
            w32(ctx, rec + off_count, r.choice([n, n, -1]))
        c = r.random()
        if c < 0.25:
            name = r.choice([b'0', b'1', b'7', b'12', b'99x', b'3 '])
        else:
            name = r.choice(CHANNELS + [b'nothing', b'X_AXIS'])
        return [r.choice([-1, 0, 1, 2, 3, 4, r.randrange(0, 5)]) & 0xffffffff, cstr(ctx, name)]
    return gen


def stream(ctx, text):
    """A C library stream holding text (pointer +0, count +4); refilling it gives end of file."""
    buf = cstr(ctx, text)
    f = newbuf(ctx, 0x20)
    w32(ctx, f, buf, len(text))
    return f


LINES = [b'\n', b'   \n', b'# comment\n', b'\t\n', b'axis x 1\n', b'  key a # trailing\n', b'#\n', b'b\n',
         b'word', b'  ', b'x#y\n']


def g_3b64(ctx):
    r = begin(ctx)
    ctype_mode(ctx)
    w32(ctx, 0x1015fdf8, ctx.u32())
    rets(0x10105a60, 0xffffffff)
    text = b''.join(r.choice(LINES) for _ in range(r.choice([0, 1, 2, 3, 5])))
    n = r.choice([0, -1, 2, 3, 5, 16, 64, 64])     # (n = 1 reads empty lines for ever, in both versions)
    return [newbuf(ctx, 80), n, stream(ctx, text)]


SINKS = [b'torso_pan_delta', b'torso_pan', b'throttle', b'eyepoint_pan_delta', b'zoom_factor', b'menu_item',
         b'pilot_tilt', b'nothing_delta', b'x']


def g_3c40(ctx):
    r = begin(ctx)
    ST['nargs'][0x100bc280] = 4
    count = r.choice([0, 1, 3, 0x31, 0x32, 0x33])
    w32(ctx, 0x1015fdec, count)
    w32(ctx, 0x1015fdf8, ctx.u32())
    ids = []
    for i in range(min(count, 0x34)):
        k = r.choice([r.randrange(0, 20), ctx.u32()])
        ids.append(k)
        wb(ctx, 0x10161700 + i * 0x32, struct.pack('<I', k) + rb(ctx, 0x2e))
    src = newbuf(ctx, 0x20)
    lo = r.choice([0, 1, -100, r.randrange(-0x8000, 0x8000)])
    hi = r.choice([lo + 1, lo + 255, lo - 3, r.randrange(-0x8000, 0x8000), lo])
    if hi == lo:        # a zero range divides by zero (both versions fault)
        hi = lo + 2
    w32(ctx, src, cstr(ctx, r.choice(SINKS)), r.choice(ids + [ctx.u32()]) if ids and r.random() < 0.5 else ctx.u32(),
        ctx.u32(), r.choice([hi, r.randrange(-0x8000, 0x8000), ctx.u32()]), lo, hi, ctx.u32(), ctx.u32())
    return [src]


# ---- 0x10057186 (registry) ----------------------------------------------------------------------------
def a_regopen(n, a, uc):
    h = mix(ST['seed'], n, 1)
    if h % 100 < ST['fail']:
        return 2
    esp = uc.reg_read(UC_X86_REG_ESP)
    uc.mem_write(ur32(uc, esp + 20), struct.pack('<I', 0x1000 + h % 50))
    return 0


def a_regquery(n, a, uc):
    h = mix(ST['seed'], n, 2)
    if h % 100 < ST['fail']:
        return 2
    esp = uc.reg_read(UC_X86_REG_ESP)
    uc.mem_write(ur32(uc, esp + 16), struct.pack('<I', 1 + h % 3))
    text = b'OEM%d_%x' % (n, h & 0xffff)
    uc.mem_write(ur32(uc, esp + 20), text + b'\0')
    uc.mem_write(ur32(uc, esp + 24), struct.pack('<I', len(text) + 1))
    return 0


API_REG = {'RegOpenKeyExA': a_regopen, 'RegQueryValueExA': a_regquery}


def g_57186(ctx):
    r = begin(ctx)
    ST['fail'] = r.choice([0, 0, 10, 30, 60])
    return [r.choice([0, 1, 2, 15, ctx.u32()]), cstr(ctx, r.choice([b'msjstick.drv', b'', b'joy'])), newbuf(ctx, 0x48),
            r.choice([0, 1, 4, 0x20, 0x40])]


# ---- 0x10057c1a (key queue) ---------------------------------------------------------------------------
def a_keystate(n, a):
    return mix(ST['seed'], n, 3) & r_mask()


def r_mask():
    return ST['ksmask']


def g_57c1a(ctx):
    r = begin(ctx)
    ST['ksmask'] = r.choice([0, 0xffff, 0x1000, 0xffffefff, 0xffffffff])
    key = r.randrange(0, 0x100)
    for i in range(0x100):
        w16(ctx, 0x10143a00 + 2 * i, r.choice([0, 0, r.randrange(0x20, 0x7f), r.getrandbits(16)]))
    del ST['fn'][0x100ff280]
    rets(0x100ff280, r.choice([0, 0x61, r.getrandbits(32)]))
    w32(ctx, 0x1016686c, r.choice([0, 1, 0x3f, r.randrange(0, 0x40)]))
    w32(ctx, 0x10166868, r.choice([0, 1, 2, 0x3f, r.randrange(0, 0x40)]))
    return [key, ctx.u32()]


API_KEY = {'GetKeyState': a_keystate, 'MapVirtualKeyA': lambda n, a: mix(ST['seed'], n, 4) & ST['mvkmask']}


def g_57c1a_k(ctx):
    out = g_57c1a(ctx)
    ST['mvkmask'] = ctx.rng.choice([0, 0xff, 0x7f, 0x80, 0xffffffff])
    return out


# ---- 0x1005a91a / 0x1005f4e0 ----------------------------------------------------------------------------
def e_angles(uc, n, raw):
    """0x10060681 writes three floats (angles in degrees) through its pointer arguments."""
    for k in (1, 2, 3):
        h = mix(ST['seed'], n, k)
        uc.mem_write(raw[k], struct.pack('<f', ((h % 80001) - 40000) / 100.0) if h % 7 else bytes(4))


def sine_table(ctx):
    """The quarter-wave sine table 0x103719a0 (513 floats, 90/512 degrees apart) that 0x10084766 reads; the game
    fills it at start-up, the image has zeros."""
    import math
    wb(ctx, 0x103719a0, struct.pack('<513f', *[math.sin(math.radians(i * 90.0 / 512)) for i in range(513)]))


EFFECT[0x10060681] = e_angles


def g_5a91a(ctx):
    r = begin(ctx)
    cnt = r.choice([0, 1, 2, 3, 6])
    w32(ctx, 0x101c42a0, cnt)
    a, b = newbuf(ctx, 0x100), newbuf(ctx, 0x100)
    w32(ctx, 0x1017d1f8, a)
    w32(ctx, 0x1017d1f4, b)
    off = r.choice([0, 1, 5, 0xffffff00])
    w32(ctx, 0x1017e6a8, off)
    value = r.choice([0, 1, 7, 100, ctx.u32()])
    for i in range(cnt):
        w32(ctx, a + 4 * i, r.choice([(value + off) & 0xffffffff, (value + 1 + off) & 0xffffffff, 0xfffffffe, ctx.u32()]))
        w32(ctx, b + 4 * i, r.choice([-1, 0, 1, 2, 5]))
    w32(ctx, 0x1015ea80, r.choice([0, 1, 2, 5, r.randrange(0, 12)]))
    w32(ctx, 0x1015ea84, 1)
    tab = newbuf(ctx, 0x200)
    for e in range(5):
        nm = r.choice([b'gun', b'Gun', b'gear', b'leg', b'', b'arm_l', b'G'])
        wb(ctx, tab + 0x7c + 0x20 * e, nm + b'\0')
    w32(ctx, tab + 0x15c, rfl(ctx), rfl(ctx), rfl(ctx))
    obj = newbuf(ctx, 0x100)
    w32(ctx, obj + 0x88, tab)
    return [obj, r.randrange(0, 4), value]


# 0x1005f4e0 (lang.ini reader) is not converted: strtok keeps a pointer into that function's stack buffer in
# 0x1017e884, which cannot be equal for the C version. The generator below was used to find that out.
INI_LINES = [b'dll = hgsim.dll\n', b'dll =  42abc\n', b'dll=x\n', b'sim file = game.sim\n', b'  sim file =\tAbc\n',
             b'; nothing\n', b'\n', b'dll = Q', b'sim file = 9z\r\n', b'other = 1\n']


def g_5f4e0(ctx):
    r = begin(ctx)
    ctype_mode(ctx)
    rets(0x10105a60, 0xffffffff)
    text = b''.join(r.choice(INI_LINES) for _ in range(r.choice([0, 1, 2, 3, 4])))
    rets(0x100fd820, stream(ctx, text) if r.random() < 0.9 else 0)
    w32(ctx, 0x10ac3860, ctx.u32())
    return [newbuf(ctx, 0x40), newbuf(ctx, 0x40)]


# ---- 0x10063415 / 0x10067356 / 0x10068e77 ---------------------------------------------------------------
def g_63415(ctx):
    r = begin(ctx)
    cur = r.choice([0, 1, 5, ctx.u32()])
    w32(ctx, 0x10144f4c, cur)
    w32(ctx, 0x109e3e1c, r.choice([0, 1, 2, 640, 480, -5, ctx.u32()]))
    w32(ctx, 0x10144f30, r.choice([0, 100, -1, ctx.u32()]))
    w32(ctx, 0x10144f34, r.choice([0, 1, 100, 240, -1, ctx.u32()]))
    w32(ctx, 0x1015dc80, r.choice([0, 1, 2, 33, 100, -3, ctx.u32()]))
    w32(ctx, 0x10144f38, r.choice([fbits(0.5), fbits(1.0), fbits(r.uniform(-10, 10)), fbits(0.1), rfl(ctx)]))
    w32(ctx, 0x10144f84, r.choice([0, 1, -1, 5]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    for i in range(4):
        w32(ctx, 0x10144f28 + 4 * i, ctx.u32())
    w32(ctx, 0x10144f44, r.choice([4, 0xc, 0x14, r.randrange(0, 0x100)]))     # colour: index of the word table 0x101659e0
    return [r.choice([0, 0, 1, ctx.u32()]), r.choice([cur, cur, ctx.u32()]), ctx.u32(), ctx.u32(),
            r.choice([0, 1, 2, 3, ctx.u32()])]


def sphere_node(ctx, depth, centre):
    """An object of the sphere tree: centre +0x60, radius +0x6c, the children's sphere +0x44..+0x50, children list
    [+0x28]+4 (next +8, object +0x8c), flags word +0."""
    r = ctx.rng
    obj = newbuf(ctx, 0x80)
    cx, cy, cz = [c + r.uniform(-200, 200) for c in centre]
    wf(ctx, obj + 0x60, cx, cy, cz, r.uniform(0, 300))
    w16(ctx, obj, r.choice([0, 0, 0x800, 0x1000, r.getrandbits(16)]))
    if r.random() < 0.08:
        w32(ctx, obj + 0x60 + 4 * r.randrange(0, 4), rfl(ctx))
    kids = []
    if depth > 0 and r.random() < 0.7:
        kids = [sphere_node(ctx, depth - 1, (cx, cy, cz)) for _ in range(r.choice([1, 1, 2, 3]))]
    wf(ctx, obj + 0x44, cx + r.uniform(-50, 50), cy + r.uniform(-50, 50), cz + r.uniform(-50, 50), r.uniform(0, 600))
    if kids or r.random() < 0.3:
        lst = newbuf(ctx, 0x10)
        prev = lst - 4      # the first node pointer is at list +4 (prev + 8)
        for k in kids:
            node = newbuf(ctx, 0x90)
            w32(ctx, node + 0x8c, k)
            w32(ctx, prev + 8, node)
            prev = node
        w32(ctx, prev + 8, 0)
        w32(ctx, obj + 0x28, lst)
    else:
        w32(ctx, obj + 0x28, 0)
    return obj


def g_67356(ctx):
    r = begin(ctx)
    centre = (r.uniform(-3000, 3000), r.uniform(-3000, 3000), r.uniform(-300, 300))
    obj = sphere_node(ctx, r.choice([0, 1, 2, 3]), centre)
    pt = [c + r.uniform(-400, 400) for c in centre]
    outs = [newbuf(ctx, 4) for _ in range(5)]
    return [obj, r.choice([fbits(r.uniform(0, 100)), fbits(0.0), rfl(ctx)]), outs[0], outs[1],
            fbits(pt[0]), fbits(pt[1]), fbits(pt[2]), outs[2], outs[3], outs[4]]


def box_obj(ctx):
    """A pointer to an object pointer: object +0x48 = transform object (position +0x70..), box at +0x284."""
    r = ctx.rng
    tr = newbuf(ctx, 0x100)
    wf(ctx, tr + 0x70, r.uniform(-500, 500), r.uniform(-500, 500), r.uniform(-50, 50))
    obj = newbuf(ctx, 0x2a0)
    w32(ctx, obj + 0x48, tr)
    lo = [r.uniform(-60, 0) for _ in range(3)]
    hi = [r.uniform(0, 60) for _ in range(3)]
    wf(ctx, obj + 0x284, lo[0], hi[0], lo[1], hi[1], lo[2], hi[2])
    if r.random() < 0.05:   # odd box values (no NaN: see the report)
        w32(ctx, obj + 0x284 + 4 * r.randrange(0, 6), r.choice([0, 0x80000000, fbits(1e-30), fbits(-5000.0)]))
    pp = newbuf(ctx, 4)
    w32(ctx, pp, obj)
    return pp


def g_68e77(ctx):
    r = begin(ctx)
    sine_table(ctx)
    a = box_obj(ctx)
    b = box_obj(ctx)
    if r.random() < 0.4:    # b near a
        ta = r32(ctx, r32(ctx, a) + 0x48)
        tb = r32(ctx, r32(ctx, b) + 0x48)
        wb(ctx, tb + 0x70, bytes(ctx.m.uc.mem_read(ta + 0x70, 12)))
    outs = [newbuf(ctx, 4) if r.random() < 0.9 else 0 for _ in range(3)]
    return [a, b] + outs


# ---- 0x10072a6b .. 0x1007e570 ---------------------------------------------------------------------------
def g_72a6b(ctx):
    r = begin(ctx)
    ST['nargs'][0x100fc780] = 2       # sprintf(buffer, format) without further arguments
    flags = zbuf(ctx, 0x40)
    for k in (0x27, 0x28, 0x2c):
        wb(ctx, flags + k, [r.choice([0, 0, 1, 0x80])])
    base = newbuf(ctx, 0x80)
    w32(ctx, base + 0x54, flags)
    wf(ctx, base + 0x58, r.uniform(-1000, 1000), r.uniform(-1000, 1000), r.uniform(-1000, 1000))
    pt = newbuf(ctx, 0x10)
    w32(ctx, 0x1014cae0, pt)
    obj = newbuf(ctx, 0x600)
    w32(ctx, obj, base)
    w32(ctx, obj + 0x4f0, r.choice([5, 5, 4, 2, 2, 2, 3, 3, 0, 1, 7]))
    now = r.choice([0, 100, 5000, ctx.u32()])
    w32(ctx, 0x1015dc78, now)
    w32(ctx, obj + 0x4d8, r.choice([now - 1, now, now + 1, ctx.u32()]))
    w32(ctx, obj + 0x504, r.choice([0, 0, 1]))
    w16(ctx, obj + 0x560, r.choice([0, 4, 8, 0xc, r.getrandbits(16)]))
    for a in (0x10179d08, 0x10179d04, 0x10179ccc, 0x10179cbc):
        w32(ctx, a, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x10179cd8, r.choice([-1, -1, 1, 1, 0, 2]))
    w32(ctx, 0x101c464c, cstr(ctx, b'Text %d'))
    w32(ctx, 0x109b98c0, cstr(ctx, b'label'))
    # the weapon records of Sim_100d5de5 / Sim_100d5f40 / Sim_100d624d (8 x 0x128 bytes at +0x84)
    w32(ctx, obj + 0x4f8, r.choice([0, 0, 1, 3, -1]))
    w32(ctx, obj + 0x4fc, r.choice([-1, 0, 1, 5, 7]))
    recs = newbuf(ctx, 8 * 0x128)
    w32(ctx, obj + 0x84, recs)
    for i in range(8):
        w32(ctx, recs + i * 0x128, r.choice([0, -1, 1]), r.choice([-1, 0, 1, 2, 5]), r.choice([-1, 0, 1]),
            r.randrange(0, 8))
        w32(ctx, recs + i * 0x128 + 0x20, r.choice([0, 1, 5, -1]))
    w32(ctx, base + 4, r.choice([0, 1, 2]))
    w32(ctx, 0x1017b8c8, r.choice([0, 1, 2]))
    w32(ctx, 0x109e8b40 + 4 * r32(ctx, 0x1017b8c8), newbuf(ctx, 0x40))
    w32(ctx, r32(ctx, 0x109e8b40 + 4 * r32(ctx, 0x1017b8c8)) + 0x28, obj)
    # the names of Sim_1000df9c and the message slots of Sim_1007df6c
    w32(ctx, 0x10a38de0, r.choice([0, 1, 3]))
    for i in range(6):
        wb(ctx, 0x10a39200 + 0x14 * i, struct.pack('<I', r.choice([0, 1, 2])) + r.choice([b'AFXHDWPS', b'abc', b'']) + b'\0')
    for i in range(2):
        w32(ctx, 0x10149d28 + 0x3c * i, newbuf(ctx, 0x100), 0, 0, r.choice([0, 1]), r.choice([0, 0x32, 0x40]))
    return [obj if r.random() > 0.03 else 0]


def g_7520b(ctx):
    r = begin(ctx)
    cur = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, cur)
    rec = newbuf(ctx, 0x40)
    blk = newbuf(ctx, 0x700)
    w32(ctx, rec + 0x28, blk)
    w32(ctx, blk + 0x613, r.choice([0, 1]))         # layout set (Sim_1002ba28 indexes static tables with it)
    w32(ctx, 0x101c64f0, r.choice([0, 1]))
    w32(ctx, 0x109e3e14, r.choice([639, 799, 319]))   # screen size used by the rectangle scaling of Sim_1002b92d
    w32(ctx, 0x109e3de0, r.choice([479, 599, 199]))
    w32(ctx, 0x109e8b40 + 4 * cur, rec)
    slot = [0]

    def check_fn():
        if r.random() < 0.25:
            return 0
        slot[0] += 1
        return code(ctx, slot[0], 2, [r.choice([1, 1, 1, 0, 3, 0xffffffff])])
    for i in range(14):
        if r.random() < 0.4:
            w32(ctx, 0x10149958 + 4 * i, 0)
            continue
        e = newbuf(ctx, 0x90)
        w32(ctx, e, newbuf(ctx, 8) if r.random() < 0.9 else 0)
        w32(ctx, e + 0x18, newbuf(ctx, 8) if r.random() < 0.9 else 0)
        if r.random() < 0.85:
            chk = newbuf(ctx, 0x28 + 0x14 * 4)
            w32(ctx, chk + 0x14, check_fn())
            n = r.choice([0, 1, 2, 3, 4])
            w32(ctx, chk + 0xc, n)
            for k in range(4):
                sub = 0
                if r.random() < 0.8:
                    sub = newbuf(ctx, 0x20)
                    w32(ctx, sub + 0x14, check_fn())
                w32(ctx, chk + 0x28 + 0x14 * k, sub)
            w32(ctx, e + 0x80, chk)
        else:
            w32(ctx, e + 0x80, 0)
        w32(ctx, 0x10149958 + 4 * i, e)
    return []


def g_75585(ctx):
    r = begin(ctx)
    w32(ctx, 0x10149948, r.randrange(0, 4))
    w32(ctx, 0x109e3dac, r.choice([0, 1]))
    w32(ctx, 0x1015dbcc, ctx.u32())
    for i in range(14):
        obj = 0
        if r.random() < 0.5:        # an object Sim_10075063 can load (Sim_10076667 / Sim_10076624 use +8, +0xc)
            obj = newbuf(ctx, 0x90)
            w32(ctx, obj + 8, newbuf(ctx, 0x40))
            w32(ctx, obj + 4, r.choice([0, 1, 2, 3]))
        w32(ctx, 0x10149958 + 4 * i, obj)
    els = [newbuf(ctx, 0x20) for _ in range(r.choice([0, 1, 2, 3, 5]))]
    for i, e in enumerate(els):
        # the slot number: Sim_10075063 reads an uninitialised local when it is outside 1..13 (see the report)
        w32(ctx, e, r.choice([1, 2, 5, 0xd]))
        st = r.choice([0, 1, 2])
        w32(ctx, e + 4, st, r.choice([st, 0, 1, 1, 2]))
        blk = zbuf(ctx, 0x60)
        w32(ctx, blk + 4, r.choice([0, 1, 2, 3, 6]))
        w32(ctx, e + 0xc, blk if r.random() < 0.97 else 0)
        w32(ctx, e + 0x14, els[i + 1] if i + 1 < len(els) else 0)
    w32(ctx, 0x1015dc14, els[0] if els else 0)
    return []


def rect(ctx):
    r = ctx.rng
    return ctx.buf(20, struct.pack('<5i', *[r.randrange(-500, 500) for _ in range(5)]))


def g_78950(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x100)
    w16(ctx, obj + 4, r.choice([0, 1, 1, 1, 0x8000]))
    w32(ctx, obj + 0x30, r.choice([0, 1, 2, 3, 3, 4, 5]))
    w32(ctx, obj + 0xc, r.randrange(0, 8))
    w32(ctx, obj + 0xec, newbuf(ctx, 20))
    w32(ctx, obj + 0xf8, r.choice([0, 1, 1, 2]))
    w32(ctx, 0x1015dc80, r.choice([0, 1, 16, 33]))
    if r.random() < 0.9:
        n = newbuf(ctx, 12)
        w32(ctx, n, r.choice([0, 1, 1, 2]), r.choice([0, 1]), r.randrange(0, 200))
        o = newbuf(ctx, 16)
        w32(ctx, o, r.choice([100, 1000, 0x10000]), rect(ctx), rect(ctx), r.choice([rect(ctx), newbuf(ctx, 20)]))
        pair = ctx.buf(8, struct.pack('<2I', n, o))
        w32(ctx, obj + 0xf4, pair)
    else:
        w32(ctx, obj + 0xf4, 0)
    w32(ctx, 0x109e89d8, ctx.u32())
    return [obj]


GROUP_NAMES = [b'alpha', b'Alpha', b'beta', b'gamma', b'', b'BETA']


def groups(ctx):
    """Groups of records (0x10473800 + 0x4fc3a * i, count at +0) with their name tables at 0x1017ee40."""
    r = ctx.rng
    ng = r.choice([0, 1, 2, 3])
    w32(ctx, 0x104737e0, r.choice([ng, ng, ng, -1]))
    for i in range(4):
        if i >= ng or r.random() < 0.15:
            w32(ctx, 0x1017ee40 + 4 * i, 0)
            continue
        n = r.choice([0, 1, 3, 6, 9])
        names = zbuf(ctx, 0x70 + 0x97 * 12)
        for j in range(12):
            wb(ctx, names + 0x70 + 0x97 * j, r.choice(GROUP_NAMES) + b'\0')
        w32(ctx, 0x1017ee40 + 4 * i, names)
        base = 0x10473800 + 0x4fc3a * i
        for j in range(12):
            rec = base + 0x13f * j
            wb(ctx, rec + 0x3a, [r.choice([0, 1, 7, 7, 2, 3])])
            w32(ctx, rec + 0x3c, r.choice([0, 1, 2, 4, 0xffffffff, ctx.u32()]))
            wb(ctx, rec + 0x128, [r.choice([0, 1, 5, 0x27, 0x28, 0x30])])
        w32(ctx, base, r.choice([n, n, -1, 12]))     # record 0's first dword is the count


def g_7950a(ctx):
    r = begin(ctx)
    groups(ctx)
    return [cstr(ctx, r.choice(GROUP_NAMES + [b'delta']))]


def g_79664(ctx):
    r = begin(ctx)
    groups(ctx)
    return [cstr(ctx, r.choice(GROUP_NAMES + [b'delta'])), r.choice([0, 1, 2, 4, 6, 0xffffffff, ctx.u32()]), ctx.u32()]


def g_79b68(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015dc78, r.choice([0, 0xb4, 0xb5, -0xb5, -1, ctx.u32()]))
    w32(ctx, 0x1017e5a8, r.choice([0, 1, 5, 0x7f, 0x100, -1]))
    for k in range(8):
        e = 0x109a8654 + 0x54 * k
        w32(ctx, e, ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32())
    obj = newbuf(ctx, 0x140)
    wb(ctx, obj + 0x128, [r.choice([0, 1, 5, 0x80]), r.choice([0, 1, 3, 7, 0x80, 0xff]), r.choice([1, 1, 0, 2, 0x81])])
    return [obj, ctx.u32()]


ENTRY_NAMES = [b'map.prj', b'MAP.PRJ', b'units', b'a', b'b', b'zz', b'Units', b'x' * 15]


def name_hash(nm):
    h = 0
    for c in nm:
        h = (h + (c | 0x20)) & 0xffffffff
        lo = h & 0xffff
        h = (h & 0xffff0000) | (((lo << 1) | (lo >> 15)) & 0xffff)
    return h % 0x65


def g_7e570(ctx):
    r = begin(ctx)
    count = r.choice([0, 1, 3, 10, 0xc7, 0xc8])
    w32(ctx, 0x1017afb0, count)
    for k in range(0x65):
        w32(ctx, 0x1017ad18 + 4 * k, 0)
    for i in range(min(count, 12)):
        e = 0x10179d78 + 0x14 * i
        nm = r.choice(ENTRY_NAMES)
        wb(ctx, e, nm + b'\0')
        k = r.randrange(0, 0x65) if r.random() < 0.3 else name_hash(nm)
        w32(ctx, e + 0x10, r32(ctx, 0x1017ad18 + 4 * k))
        w32(ctx, 0x1017ad18 + 4 * k, e)
    return [cstr(ctx, r.choice(ENTRY_NAMES)), r.choice([0, 1, 1, ctx.u32()])]


# ---- 0x10083530 .. 0x10088804 ---------------------------------------------------------------------------
def a_heapalloc(n, a):
    return ST['heap']


def g_83530(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017b3c0, r.choice([0, 0, 0, 0, 1]))
    drv = newbuf(ctx, 0x20)
    w32(ctx, drv, r.choice([2, 2, 0, 1]))
    w32(ctx, drv + 0x10, code(ctx, 1, 0))
    w32(ctx, 0x1015e688, drv)
    w32(ctx, 0x1015e9bc, r.choice([1, 2, 3]))
    w32(ctx, 0x1013a2a0, ctx.u32())
    w32(ctx, 0x1017b8f0, ctx.u32())
    w32(ctx, 0x1017b8f8, ctx.u32())
    w32(ctx, 0x1017b7d8, r.choice([0, 1]))
    w32(ctx, 0x1015e698, newbuf(ctx, 8))
    w, h = r.choice([1, 2, 8, 16, 64]), r.choice([1, 2, 8, 16, 40])
    ST['heap'] = newbuf(ctx, w * h) if r.random() < 0.85 else 0
    return [newbuf(ctx, 0x14), w, h]


def vec_ptrs(ctx):
    r = ctx.rng
    c = r.random()
    if c < 0.6:
        vals = [fbits(r.uniform(-100, 100)) for _ in range(3)]
    elif c < 0.8:
        vals = [fbits(r.uniform(-1, 1) * 10 ** r.randint(-8, 8)) for _ in range(3)]
    else:
        vals = [rfl(ctx) for _ in range(3)]
    return [ctx.buf(4, struct.pack('<I', v)) for v in vals]


def g_84cee(ctx):
    begin(ctx)
    return vec_ptrs(ctx)


def g_84f10(ctx):
    r = begin(ctx)
    rets(0x10106e50, r.choice([0, 0xffffffff, 1]))
    ST['hv'] = r.choice([0, 1])
    ST['gle'] = r.choice([0x78, 0x78, 5])
    return [cstr(ctx, r.choice([b'main', b'', b'loop 3']))]


API_HEAP = {'HeapValidate': lambda n, a: ST['hv'], 'GetLastError': lambda n, a: ST['gle']}


def a_showcursor(n, a):
    """ShowCursor answers a display counter that reaches the wanted sign after a few calls (n counts the Windows
    calls of this run)."""
    return ST['cursor'] + (n + 1) * (-1 if a[0] == 0 else 1)


def g_86c32(ctx):
    r = begin(ctx)
    ST['cursor'] = r.choice([-3, -1, 0, 1, 2])
    for a in (0x1017b930, 0x1017b92c, 0x1015ecd0, 0x1017b920):
        w32(ctx, a, r.choice([0, 0, 1]))
    w16(ctx, 0x1015fd4c, r.choice([0, 1, 0x8000]))
    w32(ctx, 0x1015e9bc, r.choice([0, 1, 2]))
    w32(ctx, 0x1017b93c, ctx.u32())
    rets(0x100766ca, *[r.choice([0, 0, 1]) for _ in range(3)])
    return []


def g_88804(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0xb0)
    w16(ctx, obj, r.choice([0, 0, 0x1000, r.getrandbits(16)]))
    cx = r.choice([r.uniform(-260000, 260000), r.uniform(-300000, 300000), -254008.0, 0.0, 55562.0 * 4.5 - 254008.0])
    cz = r.choice([r.uniform(-250000, 250000), r.uniform(-300000, 300000), -246044.5])
    if r.random() < 0.05:
        cx = struct.unpack('<f', struct.pack('<I', rfl(ctx)))[0]
    if r.random() < 0.7:
        tr = newbuf(ctx, 0x80)
        wf(ctx, tr + 0x70, cx, r.uniform(-100, 100), cz)
        w32(ctx, obj + 0x28, tr)
    else:
        w32(ctx, obj + 0x28, 0)
        wf(ctx, obj + 0x44, cx)
        wf(ctx, obj + 0x4c, cz)
    wb(ctx, obj + 0xa0, r.choice([b'TL1MBO', b'tl1mBO', b'TL1Mxx', b'TL2MBO', b'TL1MBo']) + b'\0')
    return [obj]


SPECS = [
    dict(name='Input_AddDevice', orig=0x1005366e, c='Input_AddDevice', gen=g_366e, trials=600),
    dict(name='Input_GetDevice', orig=0x1005388b, c='Input_GetDevice', gen=g_388b, trials=600),
    dict(name='Sim_1005393a', orig=0x1005393a, c='Sim_1005393a', gen=g_channel(0x64, 0x58), trials=600),
    dict(name='Sim_10053a4f', orig=0x10053a4f, c='Sim_10053a4f', gen=g_channel(0x6c, 0x5c), trials=600),
    dict(name='Input_ReadLine', orig=0x10053b64, c='Input_ReadLine', gen=g_3b64, trials=600),
    dict(name='Input_AddAnalogSink', orig=0x10053c40, c='Input_AddAnalogSink', gen=g_3c40, trials=500),
    # superseded (C runtime replacement, the rest, DECISIONS.md): no joystick name, the registry is not read
    # dict(name='Joy_GetOemName', orig=0x10057186, c='Joy_GetOemName', gen=g_57186, trials=500, api=API_REG),
    dict(name='Input_QueueKey', orig=0x10057c1a, c='Input_QueueKey', gen=g_57c1a_k, trials=600, void=True,
         api=API_KEY),
    dict(name='Sim_1005a91a', orig=0x1005a91a, c='Sim_1005a91a', gen=g_5a91a, trials=500, void=True),
    dict(name='Sim_10063415', orig=0x10063415, c='Sim_10063415', gen=g_63415, trials=600),
    dict(name='Sim_10067356', orig=0x10067356, c='Sim_10067356', gen=g_67356, trials=1000),
    dict(name='Sim_10068e77', orig=0x10068e77, c='Sim_10068e77', gen=g_68e77, trials=1000),
    dict(name='Sim_10072a6b', orig=0x10072a6b, c='Sim_10072a6b', gen=g_72a6b, trials=800, void=True),
    dict(name='Sim_1007520b', orig=0x1007520b, c='Sim_1007520b', gen=g_7520b, trials=500, void=True),
    dict(name='Sim_10075585', orig=0x10075585, c='Sim_10075585', gen=g_75585, trials=600, void=True),
    dict(name='Sim_10078950', orig=0x10078950, c='Sim_10078950', gen=g_78950, trials=600, void=True),
    dict(name='Sim_1007950a', orig=0x1007950a, c='Sim_1007950a', gen=g_7950a, trials=500),
    dict(name='Sim_10079664', orig=0x10079664, c='Sim_10079664', gen=g_79664, trials=400, void=True),
    dict(name='Sim_10079b68', orig=0x10079b68, c='Sim_10079b68', gen=g_79b68, trials=500),
    dict(name='ProjFile_FindEntry', orig=0x1007e570, c='ProjFile_FindEntry', gen=g_7e570, trials=600),
    # superseded: the loading screen draws through Vulkan (DECISIONS.md, "The loading screen through Vulkan"; checked natively against GDI by tools/verify/picture_native.c)
    # dict(name='Sim_10083530', orig=0x10083530, c='Sim_10083530', gen=g_83530, trials=400,
    #      api={'HeapAlloc': a_heapalloc}),
    dict(name='Sim_10084cee', orig=0x10084cee, c='Sim_10084cee', gen=g_84cee, trials=1000, void=True),
    dict(name='Heap_ReportCheck', orig=0x10084f10, c='Heap_ReportCheck', gen=g_84f10, trials=300, api=API_HEAP),
    dict(name='Sim_10086c32', orig=0x10086c32, c='Sim_10086c32', gen=g_86c32, trials=500, void=True,
         api={'ShowCursor': a_showcursor}),
    dict(name='Sim_10088804', orig=0x10088804, c='Sim_10088804', gen=g_88804, trials=1000),
]
