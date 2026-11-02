# Differential tests for round 8, letter z (see difftest.py and docs/checker.md).
#
# Helpers are replaced in the emulator by a code hook at their first byte (the code bytes are not patched): the
# hook appends ('stub_<address>', arguments, extra data) to the checker's call log, which must be identical for
# the original and the C run (stack addresses are logged as 'stack'; word arguments only by their low 16 bits,
# since the original pushes them with garbage in the top half), writes the helper's out data where it has some,
# answers with a value that depends only on the trial and the call position and returns to the caller. Helpers
# that answer in st(0) return through a small `fld [value]; ret` snippet. Converted callees are stubbed at both
# their original address and their C symbol, so each function is checked on its own logic; the stubs of this
# letter's own functions (used by their callers here) are only active in the specs of those callers. Function
# pointers (object routines, the driver tables, the plugin slot) point at `ret` bytes with logging hooks.
# Run for real on both sides: fabs (0x100fcb70 / OpenLibm hg_fabs) with 0x10103ad0 and Sim_10103940, the float
# truncation 0x100fcb08, Sim_1004b804, Mem_Fill, Mem_Move, Str_Copy, Str_CopyN.
#
# Sim_100c9572 answers in st(0): both versions are called through the same kind of machine-code
# trampoline, which stores st(0) as all 80 bits at OUTF (compared like all memory). No float tolerance is used:
# every function here must match bit for bit.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW,
                               UC_X86_REG_FPSW, UC_X86_REG_FPTAG)

SOURCES = [
    # this letter
    'sim/unsorted_100f/w8z_crt.c', 'sim/unsorted_100c/w8z_units.c', 'sim/unsorted_100c/w8z_targets.c', 'sim/unsorted_1000/w24_aim.c',
    'sim/unsorted_100c/w8z_misc.c', 'sim/unsorted_100d/w8z_project.c', 'sim/unsorted_100d/w8z_effects.c',
    'sim/unsorted_100d/w8z_palette.c', 'sim/unsorted_100d/w8z_lists.c', 'sim/unsorted_100e/w8z_load.c',
    # existing converted code the functions above call, and what those files need to link
    'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_100f/w7q_stream.c', 'sim/unsorted_100f/w6i_mem.c',
    'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_1000/w3o_sphere.c', 'sim/unsorted_1000/w5b_calls.c',
    'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1001/w5y_misc.c', 'sim/unsorted_1001/w5c_math.c',
    'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1001/w2e_tables.c', 'sim/unsorted_1001/w6d_calls.c',
    'sim/unsorted_1001/w5c_calls.c', 'sim/unsorted_1002/w5x_calls.c', 'sim/unsorted_1002/w7l_collide.c',
    'sim/unsorted_1003/entry.c', 'sim/unsorted_1004/state.c', 'sim/unsorted_1004/w2j_misc.c',
    'sim/unsorted_1004/w2l_values.c', 'sim/unsorted_1005/w2h_tables.c', 'sim/unsorted_1004/tables.c',
    'sim/unsorted_1007/w2e_state.c', 'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1008/w7n_misc.c',
    'sim/unsorted_1008/w4v_fade.c', 'sim/util/ring.c', 'sim/unsorted_1009/w3o_ray.c', 'sim/util/field.c',
    'sim/unsorted_100b/w4w_buffers.c', 'sim/unsorted_100b/w2j_misc.c', 'sim/unsorted_100b/w6g_misc.c',
    'sim/unsorted_100c/w5c_calls.c', 'sim/unsorted_100c/w5c_math.c', 'sim/unsorted_100c/w3p_misc.c',
    'sim/unsorted_100c/w7p_states.c', 'sim/unsorted_100c/w5a_misc.c', 'sim/unsorted_100c/w3t_misc.c',
    'sim/unsorted_100c/w5b_calls.c', 'sim/unsorted_100c/w2f_objects.c', 'sim/unsorted_100c/w7p_misc.c',
    'sim/unsorted_100c/entries.c', 'sim/unsorted_100c/w2l_objects.c', 'sim/unsorted_100c/w5z_calls.c',
    'sim/unsorted_100c/w5x_calls.c', 'sim/util/record.c', 'sim/unsorted_100c/w2e_tables.c',
    'sim/unsorted_100c/w7q_relative.c', 'sim/unsorted_100d/w5c_math.c', 'sim/unsorted_100d/w6h_calls.c',
    'sim/unsorted_100d/w4v_misc.c', 'sim/unsorted_100d/w2e_names.c', 'sim/unsorted_100d/state.c',
    'sim/unsorted_100e/w3p_misc.c', 'sim/unsorted_1010/w6j_double.c', 'sim/unsorted_1010/w6j_stream.c',
    'sim/unsorted_1010/w7s_stdio.c', 'sim/unsorted_1010/w6k_text.c', 'sim/unsorted_100c/w6h_calls.c',
    'sim/win/w4u_cdaudio.c', 'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_1003/bitmap.c', 'sim/win/w4u_gdi.c',
    'sim/win/w4v_heap.c', 'sim/win/w4a_heap.c', 'sim/unsorted_1010/w6i_crt.c', 'sim/unsorted_1004/w4u_heap.c',
    'sim/view/w3r_matrix.c', 'sim/unsorted_100f/w2h_mem.c', 'sim/unsorted_1003/w2e_mono.c',
    'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2e_names.c', 'sim/unsorted_1000/w2l_fields.c',
    'sim/unsorted_1001/w5b_calls.c', 'sim/unsorted_1001/w2j_misc.c', 'sim/unsorted_1001/w3t_loader.c',
    'sim/unsorted_1001/w4u_cd.c', 'sim/unsorted_1002/w5y_misc.c', 'sim/unsorted_1002/w2j_nodes.c',
    'sim/unsorted_1002/w2g_records.c', 'sim/core/w2a_timers.c', 'sim/unsorted_1002/w3o_ray.c',
    'sim/unsorted_1002/w2k_misc.c', 'sim/unsorted_1002/w5a_misc.c', 'sim/unsorted_1003/w3s_entry.c',
    'sim/unsorted_1003/w5c_calls.c', 'sim/unsorted_1004/fields.c', 'sim/unsorted_1005/w5b_calls.c',
    'sim/unsorted_1005/w3o_calls.c', 'sim/unsorted_1005/w2j_misc.c', 'sim/unsorted_1005/w2k_misc.c',
    'sim/unsorted_1005/w3q_misc.c', 'sim/unsorted_1006/w5y_misc.c', 'sim/view/objlist.c',
    'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_1008/w4w_display.c', 'sim/unsorted_1008/w5b_calls.c',
    'sim/unsorted_1008/w3r_grid.c', 'sim/unsorted_1008/w3o_grid.c', 'sim/unsorted_1008/w5x_calls.c',
    'sim/unsorted_1009/w2g_objects.c', 'sim/unsorted_1009/w3p_box.c', 'sim/unsorted_1009/w3q_misc.c',
    'sim/unsorted_1009/w6g_segment.c', 'sim/unsorted_1009/w2e_copy.c', 'sim/unsorted_1009/w2l_pool.c',
    'sim/unsorted_1009/w2h_misc.c', 'sim/unsorted_100a/w5z_calls.c', 'sim/unsorted_100a/w5x_calls.c',
    'sim/unsorted_100a/w3t_misc.c', 'sim/unsorted_100a/w5b_calls.c', 'sim/unsorted_100a/w5y_misc.c',
    'sim/unsorted_100a/w5c_calls.c', 'sim/unsorted_100b/w2h_records.c', 'sim/unsorted_100b/w2f_tables.c',
    'sim/unsorted_100b/fields.c', 'sim/unsorted_100b/w5z_calls.c', 'sim/unsorted_100b/w3o_hash.c',
    'sim/unsorted_100b/w5c_calls.c', 'sim/unsorted_100b/w3s_array.c', 'sim/unsorted_100b/w2l_records.c',
    'sim/unsorted_100c/w7p_fall.c', 'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_100c/w3s_entries.c',
    'sim/unsorted_100c/w3o_misc.c', 'sim/unsorted_100d/w2f_records.c', 'sim/unsorted_100d/w3q_misc.c',
    'sim/unsorted_100d/w2d_state.c', 'sim/unsorted_100d/w2j_nodes.c', 'sim/unsorted_100f/w6i_stream.c',
    'sim/unsorted_1010/w2h_misc.c', 'sim/unsorted_1010/w6k_stream.c', 'sim/unsorted_1010/w2k_crt.c',
    'sim/unsorted_1010/w7s_locale.c', 'sim/unsorted_100d/nodes.c', 'sim/unsorted_100f/w2j_str.c',
    'sim/view/matrix.c', 'sim/unsorted_1000/w2e_mono.c', 'sim/unsorted_100b/w2e_strings.c',
    'sim/unsorted_1006/w3q_quat.c', 'sim/view/viewrect.c', 'sim/unsorted_1000/w2l_objects.c',
    'sim/unsorted_1000/w3s_model.c', 'sim/unsorted_1000/w5x_calls.c', 'sim/unsorted_1000/w2a_fields.c',
    'sim/unsorted_1000/w2j_slots.c', 'sim/unsorted_1001/w2e_names.c', 'sim/unsorted_1001/w2h_objects.c',
    'sim/util/lookup.c', 'sim/unsorted_1001/w2l_misc.c', 'sim/unsorted_1002/w2e_tables.c',
    'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1002/w2l_tables.c', 'sim/unsorted_1002/w2h_misc.c',
    'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1003/w5a_misc.c',
    'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1003/w3r_misc.c', 'sim/unsorted_1003/setters.c',
    'sim/unsorted_1003/slots.c', 'sim/unsorted_1004/w3p_net.c', 'sim/unsorted_1008/w2l_grid.c',
    'sim/view/w2l_lists.c', 'sim/unsorted_1008/cell_link.c', 'sim/view/w2d_view.c', 'sim/unsorted_1008/w3s_grid.c',
    'sim/unsorted_1008/w2g_rect.c', 'sim/unsorted_100a/w4u_misc.c', 'sim/unsorted_100a/w2k_list.c',
    'sim/unsorted_100b/w4v_wrappers.c', 'sim/win/w4v_dll.c', 'sim/unsorted_100d/w2g_misc.c',
    'sim/unsorted_1010/w6k_misc.c', 'sim/unsorted_1010/w2i_crt.c', 'sim/unsorted_1010/w2g_crt.c',
    'sim/core/timer.c', 'sim/unsorted_1007/w3p_net.c', 'sim/unsorted_1003/w2e_misc.c',
    'sim/unsorted_1003/records.c', 'sim/unsorted_1004/w2i_misc.c', 'sim/unsorted_1004/w2h_record.c',
    'sim/unsorted_1007/w5a_misc.c', 'sim/unsorted_1007/w2f_misc.c', 'sim/unsorted_100c/w4v_timers.c',
    'sim/unsorted_1010/w2f_mantissa.c', 'sim/unsorted_1002/w2e_object.c', 'sim/unsorted_100d/w2i_select.c',
    'sim/unsorted_1010/w2f_ld12.c',
]
# The list above predates rounds 32 / 33 (W31_TexMode, the particle systems, the network code): what the files need
# to link now is added by w10_common's closure(), run in its own namespace so that nothing else of w10_common
# changes this spec.
_w10 = {}
exec(open('tools/verify/w10_common.py').read(), _w10)
SOURCES = _w10['closure'](SOURCES)
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000          # trampoline results (st(0) as 10 bytes)
TRAMP = 0x00F00800         # trampolines, 0x40 bytes each
SNIP = 0x00F00C00          # fld snippets
FVAL = 0x00F00C80
DVAL = 0x00F00C88
CB = 0x00F00D00            # callback `ret` bytes, 0x10 apart

ST = {'seed': 0}


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<H', v & 0xffff))


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


def f2b(f):
    try:
        return struct.unpack('<I', struct.pack('<f', f))[0]
    except OverflowError:
        return 0xff800000 if f < 0 else 0x7f800000


def b2f(b):
    return struct.unpack('<f', struct.pack('<I', b & 0xffffffff))[0]


def fb(h):
    """Float bits from a hash: mostly ordinary values, some zeros, ones, tiny/huge values and few NaN/inf."""
    k = h % 100
    h2 = mix(h, 77)
    if k < 2:
        return pick(h2, [0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])
    if k < 12:
        return pick(h2, [0, 0x80000000, 0x3f800000, 0xbf800000, 1, 0x80000001, 0x00800000, 0x7f7fffff])
    if k < 70:
        return f2b(((h2 % 2000001) - 1000000) / 100.0)
    if k < 85:
        return f2b(((h2 % 20001) - 10000) / 7.0)
    return h2 & 0xbfffffff


def fbits(ctx, special=True):
    r = ctx.rng
    k = r.random()
    if special and k < 0.03:
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])
    if k < 0.12:
        return r.choice([0, 0x80000000, 1, 0x80000001, 0x00800000, 0x7f7fffff, 0x3f800000, 0xbf800000])
    if k < 0.75:
        return f2b(r.uniform(-5000.0, 5000.0))
    return r.getrandbits(32) & 0xbfffffff


def near(ctx, addr, dbl=False):
    """Float bits near the constant at addr (float, or a double read as float)."""
    r = ctx.rng
    if dbl:
        c = struct.unpack('<d', bytes(ctx.m.uc.mem_read(addr, 8)))[0]
    else:
        c = b2f(r32(ctx.m.uc, addr))
    k = r.random()
    if k < 0.15:
        return f2b(c)
    if k < 0.75:
        return f2b(c * r.choice([0.25, 0.5, 0.9, 0.999, 1.001, 1.1, 2.0, 4.0, -1.0, -0.5]))
    return fbits(ctx)


# ---- stubs --------------------------------------------------------------------------------------------------
class Stub:
    def __init__(self, nargs, pol, csym=None, kind='int', mine=False, words=()):
        self.nargs, self.pol, self.csym, self.kind, self.mine, self.words = nargs, pol, csym, kind, mine, words


def pv(tag, options):
    """Answer from options by (trial, call number)."""
    return lambda uc, n, a: (pick(mix(ST['seed'], tag, n), ST.get('v%x' % tag) or options), [])


def pf(tag):
    """Float answer (bits) by (trial, call number); a list in ST['f<tag>'] overrides."""
    def pol(uc, n, a):
        lst = ST.get('f%x' % tag)
        return (lst[n % len(lst)] if lst else fb(mix(ST['seed'], tag, n))), []
    return pol


def cstring(uc, addr, maxn=0x30):
    out = b''
    while len(out) < maxn:
        c = bytes(uc.mem_read(addr + len(out), 1))
        if c == b'\0':
            break
        out += c
    return list(out)


def p_str(tag, options, which=(0,)):
    def pol(uc, n, a):
        extra = []
        for i in which:
            extra.append(tuple(cstring(uc, a[i])) if a[i] else ())
        return pick(mix(ST['seed'], tag, n), ST.get('v%x' % tag) or options), extra
    return pol


def p_outs(tag, outs, ret=None, retf=False, login=0):
    """Writes float words through the argument pointers `outs` (first logging `login` words read through each)."""
    def pol(uc, n, a):
        extra = []
        for k, i in enumerate(outs):
            if login:
                extra += list(struct.unpack('<%dI' % login, bytes(uc.mem_read(a[i], 4 * login))))
            lst = ST.get('o%x' % tag)
            v = lst[(n * len(outs) + k) % len(lst)] if lst else fb(mix(ST['seed'], tag, n, k))
            uc.mem_write(a[i], struct.pack('<I', v & 0xffffffff))
        if retf:
            return fb(mix(ST['seed'], tag, n, 99)), extra
        return (pick(mix(ST['seed'], tag, n, 98), ST.get('v%x' % tag) or ret or [0])), extra
    return pol


def p_bytes(tag, argi, size, ret=(0,), logsize=0, logi=None):
    """Writes `size` bytes from the hash through argument argi; logs `logsize` bytes read through logi first."""
    def pol(uc, n, a):
        extra = []
        if logsize:
            extra = list(bytes(uc.mem_read(a[logi], logsize)))
        data = bytes(mix(ST['seed'], tag, n, k) & 0xff for k in range(size))
        uc.mem_write(a[argi], data)
        return pick(mix(ST['seed'], tag, n), ST.get('v%x' % tag) or list(ret)), extra
    return pol


def p_rand(uc, n, a):
    m = a[0]
    h = mix(ST['seed'], 0x929, n)
    if 0 < m < 0x80000000:
        return h % m, []
    return pick(h, [0, 1, 0xffffffff]), []


def p_log(tag, ptr_i, nwords, ret=(0,)):
    def pol(uc, n, a):
        extra = list(struct.unpack('<%dI' % nwords, bytes(uc.mem_read(a[ptr_i], 4 * nwords))))
        return pick(mix(ST['seed'], tag, n), ST.get('v%x' % tag) or list(ret)), extra
    return pol


def p_output(uc, n, a):
    # _output(stream, format, args): logs the record fields, the format and three argument words; then puts
    # k bytes in the buffer like the engine does and sometimes leaves no byte (cnt 0) to reach the flush path
    s = a[0]
    ptr, cnt, base, flag = struct.unpack('<4I', bytes(uc.mem_read(s, 16)))
    extra = [ptr, cnt, base, flag] + cstring(uc, a[1]) + list(struct.unpack('<3I', bytes(uc.mem_read(a[2], 12))))
    h = mix(ST['seed'], 0xfd0, n)
    k = h % 9
    uc.mem_write(ptr, bytes(0x41 + (mix(h, i) % 26) for i in range(k)))
    newcnt = pick(h >> 8, [(cnt - k) & 0xffffffff, (cnt - k) & 0xffffffff, 0, 1, 0x80000000])
    uc.mem_write(s, struct.pack('<2I', (ptr + k) & 0xffffffff, newcnt))
    return pick(h >> 4, [k, k, 0xffffffff, 0]), extra


def p_flsbuf(uc, n, a):
    ptr, cnt, base, flag = struct.unpack('<4I', bytes(uc.mem_read(a[1], 16)))
    return 0, [ptr, cnt, base, flag]


def p_fread(uc, n, a):
    h = mix(ST['seed'], 0x830, n)
    uc.mem_write(a[0], bytes(mix(h, i) & 0xff for i in range(0x200)))
    return pick(h, ST.get('vread') or [0x200]), []


def p_tar(uc, n, a):
    # Tar_ReadHeader(header, out, gnu): logs 16 header bytes, sets the size dword 0x1017e4ec
    extra = list(bytes(uc.mem_read(a[0], 16)))
    sizes = ST.get('sizes') or [0]
    uc.mem_write(0x1017e4ec, struct.pack('<I', sizes[n % len(sizes)] & 0xffffffff))
    return 0, extra


def p_timer(uc, n, a):
    step = ST.get('tstep', 0x40)
    return min(n * step, 0x10f + 5), []


def p_eda16(uc, n, a):
    first = list(struct.unpack('<5I', bytes(uc.mem_read(a[0], 20))))
    second = list(struct.unpack('<5I', bytes(uc.mem_read(first[0], 20))))
    return mix(ST['seed'], 0xda16, n), [logged(first[0])] + first[1:] + second


def p_err(uc, n, a):
    # the error report is variadic: 0xb has (format, name), 0x4b one word
    esp = uc.reg_read(UC_X86_REG_ESP)
    if a[0] == 0xb:
        fmt, nm = r32(uc, esp + 8), r32(uc, esp + 12)
        return 0, [logged(fmt), logged(nm)] + cstring(uc, nm)
    return 0, [logged(r32(uc, esp + 8))]


def p_logf(tag, ptr_i, nwords):
    def pol(uc, n, a):
        extra = list(struct.unpack('<%dI' % nwords, bytes(uc.mem_read(a[ptr_i] + 0x30, 4 * nwords))))
        lst = ST.get('f%x' % tag)
        return (lst[n % len(lst)] if lst else fb(mix(ST['seed'], tag, n))), extra
    return pol


def p_tw(uc, n, a):
    # Sim_100c4e1c / Sim_100c45dc as callees: log the slot dword, answer from the list
    return pick(mix(ST['seed'], 0x4e1c, n, a[0]), ST.get('vstep') or [0, 1, 2]), [r32(uc, a[2])]


STUBS = {
    # ---- C runtime
    0x10100fd0: Stub(3, p_output),
    0x100ff030: Stub(2, p_flsbuf),
    0x101049c0: Stub(1, pv(0x49c0, [0, 0, 0xffffffff, 3])),
    0x100fe290: Stub(1, pv(0xe290, [0, 0xffffffff, 7]), 'Crt_FlushStream'),
    0x10104aa0: Stub(1, pv(0x4aa0, [0]), 'Sim_10104aa0'),
    0x100fefe0: Stub(1, pv(0xefe0, [0]), 'Heap_FreeBlock'),
    0x10104db0: Stub(0, pv(0x4db0, [0, 0x00A80000, 0x00A80000]), 'Sim_10104db0'),
    0x10104be0: Stub(4, p_str(0x4be0, [0, 0x00A80000, 7], (0, 1)), 'Sim_10104be0'),
    0x10102e30: Stub(4, pv(0x2e30, [0]), kind='dret8'),
    0x10102f00: Stub(7, pv(0x2f00, [0]), kind='dret14'),
    0x100fe830: Stub(4, p_fread),
    0x100feb20: Stub(3, pv(0xeb20, [0, 0xffffffff])),
    0x100c6c39: Stub(3, p_tar, 'Tar_ReadHeader'),
    0x100fe970: Stub(1, pv(0xe970, [0, 0x200, 0x12345, 0xffffffff]), 'Crt_FTell'),
    0x1010c9f0: Stub(1, p_str(0xc9f0, [0x00A81000, 0x00A81000, 0x00A81000, 0]), 'Sim_1010c9f0'),
    0x100fcb30: Stub(4, pv(0xcb30, [0]), 'hg_fmod', kind='double'),     # the C calls OpenLibm (C runtime replacement)
    0x100fca04: Stub(2, pv(0xca04, [0]), 'hg_sqrt', kind='double'),
    0x100fd9a0: Stub(2, pv(0xd9a0, [0, 4, 0x84]), 'Crt_IsCType'),
    0x100fd1b0: Stub(1, p_str(0xd1b0, [0, 7, 0x12345, 0xffff8000]), 'Crt_AToI'),
    # ---- unit helpers
    0x100c2ae5: Stub(1, pv(0x2ae5, [0, 0, 1])),
    0x1001412d: Stub(2, pf(0x412d), kind='float'),
    0x1000a6f0: Stub(2, pv(0xa6f0, [0, 0, 0, 1, 5])),
    0x1001403a: Stub(2, pv(0x403a, [0]), 'Sim_1001403a', words=(1,)),
    0x100c05cb: Stub(1, pf(0x05cb), 'Sim_100c05cb', kind='float'),
    0x100144e9: Stub(1, pf(0x44e9), 'Sim_100144e9', kind='float'),
    0x10015266: Stub(1, pf(0x5266), 'Sim_10015266', kind='float'),
    0x100c0379: Stub(2, pv(0x0379, [0, 1, 1]), 'Sim_100c0379'),
    0x1000ae00: Stub(2, pv(0xae00, [0, 0, 1]), 'Sim_1000ae00'),
    0x100c3725: Stub(1, pf(0x3725), 'Sim_100c3725', kind='float'),
    0x100c38fe: Stub(1, pv(0x38fe, [0, 1, 2]), 'Sim_100c38fe', words=(0,)),
    0x1001570f: Stub(1, pv(0x570f, [0]), 'Sim_1001570f'),
    0x100157ca: Stub(1, pv(0x57ca, [0]), 'Sim_100157ca'),
    0x100159cf: Stub(2, pv(0x59cf, [0]), 'Sim_100159cf', words=(1,)),
    0x100c00c7: Stub(4, pv(0x00c7, [0]), 'Sim_100c00c7', words=(2,)),
    0x100c4035: Stub(3, pv(0x4035, [0, 3, 0xffffffff, 0x7fffffff]), 'Sim_100c4035'),
    0x10033c42: Stub(1, pv(0x3c42, [0x00A82000, 0]), 'Sim_10033c42'),
    0x100c4d63: Stub(1, pv(0x4d63, [0, 0, 0, 1]), 'Sim_100c4d63'),
    0x100c48c9: Stub(1, pv(0x48c9, [0]), 'Sim_100c48c9'),
    0x100c4898: Stub(1, pv(0x4898, [0]), 'Sim_100c4898'),
    0x100c48fa: Stub(2, pv(0x48fa, [0]), 'Sim_100c48fa'),
    0x100c459f: Stub(2, pv(0x459f, [0, 1, 1, 1]), 'Sim_100c459f'),
    0x100c4498: Stub(3, pv(0x4498, [0]), 'Sim_100c4498'),
    0x1009c929: Stub(1, p_rand, 'Sim_1009c929'),
    0x100ca417: Stub(2, pv(0xa417, [0]), 'Sim_100ca417'),
    0x10084cee: Stub(3, p_outs(0x4cee, (0, 1, 2), login=1), 'Sim_10084cee'),
    0x100ca350: Stub(4, pv(0xa350, [0xffffffff, 0xffff, 5, 0x1234ffff, 0x80]), 'Sim_100ca350'),
    0x100c45dc: Stub(3, p_tw, 'Sim_100c45dc', mine=True),
    0x100c4e1c: Stub(3, p_tw, 'Sim_100c4e1c', mine=True),
    # ---- targets
    0x100cada2: Stub(3, pv(0xada2, [0xffffffff, 0xfffffffe, 0, 1, 1, 0xfffffff7])),
    0x100cb299: Stub(3, pv(0xb299, [0xffffffff, 0xfffffffe, 0, 1, 1]), 'Sim_100cb299'),
    0x100cab7b: Stub(3, pv(0xab7b, [0xffffffff, 0xfffffffe, 0, 1, 1]), 'Sim_100cab7b'),
    0x10052be7: Stub(1, pv(0x2be7, [0, 1, 1]), 'Sim_10052be7'),
    0x1000ebfb: Stub(5, p_str(0xebfb, [0], (0,)), 'Sim_1000ebfb'),
    0x100cbdb8: Stub(3, pv(0xbdb8, [0]), 'Sim_100cbdb8'),
    0x100cb4fb: Stub(1, pv(0xb4fb, [0, 1]), 'Sim_100cb4fb', mine=True),
    0x1008486e: Stub(2, pf(0x486e), kind='float'),
    0x10084766: Stub(1, pf(0x4766), kind='float'),
    0x10064824: Stub(3, pv(0x4824, [0, 1, 1])),
    0x100d2669: Stub(3, p_outs(0x2669, (0, 1, 2), ret=[0, 1, 1], login=1), 'Sim_100d2669', mine=True),
    0x100c3892: Stub(2, pv(0x3892, [0, 1, 1]), 'Sim_100c3892'),
    0x100c34b7: Stub(2, pf(0x34b7), 'Sim_100c34b7', kind='float', words=(1,)),
    0x100c31a6: Stub(6, p_bytes(0x31a6, 1, 0x38), 'Sim_100c31a6', words=(3,)),
    0x1002dda9: Stub(3, p_bytes(0x2dda, 1, 4, ret=(0, 0, 1), logsize=0x38, logi=0), 'Sim_1002dda9'),
    0x100cba68: Stub(0, pv(0xba68, [0, 0x00A83000, 0x00A83000]), 'Sim_100cba68'),
    0x100a9cfb: Stub(4, p_outs(0x9cfb, (1, 2, 3)), 'Sim_100a9cfb'),
    0x100d6ea7: Stub(5, p_outs(0x6ea7, (1, 2, 3)), 'Sim_100d6ea7'),
    0x10084c95: Stub(3, p_outs(0x4c95, (0, 1, 2), login=1), 'Sim_10084c95'),
    0x100cbc6b: Stub(7, p_outs(0xbc6b, (3, 4, 5, 6)), 'Sim_100cbc6b', mine=True),
    # ---- misc
    0x100652e8: Stub(4, p_str(0x52e8, [0], (2,))),
    0x100619a0: Stub(5, p_log(0x19a0, 2, 7)),
    0x100c81b5: Stub(1, pv(0x81b5, [0]), 'Sim_100c81b5'),
    0x10044992: Stub(2, p_bytes(0x4992, 1, 0x1c, ret=(1,)), 'Sim_10044992'),
    0x100787f4: Stub(1, p_bytes(0x87f4, 0, 0x68), 'Sim_100787f4'),
    0x1000946e: Stub(4, p_outs(0x946e, (1, 2, 3), retf=True, login=1), 'Sim_1000946e', kind='float'),
    0x100ba7d5: Stub(3, p_str(0xa7d5, [0, 0x00A84000], (0, 1))),
    0x100f2f01: Stub(6, p_str(0x2f01, [0], (4,))),
    0x100f1e10: Stub(6, p_str(0x1e10, [0], (4,))),
    0x100ecea6: Stub(6, p_str(0xcea6, [0], (4,))),
    0x100bb2b0: Stub(2, p_str(0xb2b0, [0, 1], (0, 1)), 'Sim_100bb2b0'),
    # ---- 0x100d
    0x100e85cb: Stub(3, pv(0x85cb, [0, 1, 15, 31, 0xffffffff, 0xfffffff1, 0x7fffffff, 200])),
    0x100e86a1: Stub(7, pv(0x86a1, [0])),
    0x100ef727: Stub(7, pv(0xf727, [0])),
    0x10047af2: Stub(1, pv(0x7af2, [0]), 'Sim_10047af2'),
    0x100d6f16: Stub(2, p_bytes(0x6f16, 1, 0x44), 'Sim_100d6f16'),
    0x1009cbde: Stub(1, p_logf(0xcbde, 0, 2), 'Sim_1009cbde', kind='float'),
    0x100d9064: Stub(7, pv(0x9064, [0]), 'Sim_100d9064'),
    0x100d965e: Stub(7, pv(0x965e, [0])),
    0x100f01ea: Stub(1, pv(0x01ea, [0, 5, 0x12345, 0xffff])),
    0x10062386: Stub(0, pv(0x2386, [0])),
    0x100d3000: Stub(1, pv(0x3000, [0]), 'Sim_100d3000', mine=True),
    0x1008c6f5: Stub(6, pv(0xc6f5, [0, 1]), 'Sim_1008c6f5'),
    0x100e4579: Stub(1, pv(0x4579, [3, 0x10005, 0x8000])),
    0x100e4622: Stub(1, pv(0x4622, [0])),
    0x100e45e7: Stub(1, p_timer),
    0x100e469b: Stub(1, pv(0x469b, [0])),
    0x100eda16: Stub(4, p_eda16),
    0x100eb070: Stub(2, pv(0xb070, [0])),
    0x100da45c: Stub(0, pv(0xa45c, [0]), 'Sim_100da45c'),
    0x100b9203: Stub(2, p_str(0x9203, [0], (0, 1))),
    0x100da4f7: Stub(2, p_log(0xa4f7, 0, 14, ret=(0, 1, 1)), 'Sim_100da4f7'),
    # ---- 0x100e
    0x10052530: Stub(0, pv(0x2530, [0]), 'Sim_10052530'),
    0x100e2756: Stub(3, pv(0x2756, [0]), 'Sim_100e2756'),
    0x100db77e: Stub(1, pv(0xb77e, [0]), 'Sim_100db77e'),
    0x100278c0: Stub(2, p_bytes(0x78c0, 1, 0x20, ret=(0, 0x00A85000, 0x00A85000), logsize=15, logi=0)),
    0x100b35b0: Stub(0, pv(0x35b0, [0, 1, 1]), 'Sim_100b35b0'),
    0x100dcc12: Stub(1, pv(0xcc12, [0, 1, 1, 7])),
    0x10027a8f: Stub(1, pv(0x7a8f, [0]), 'Sim_10027a8f'),
    0x100db351: Stub(0, pv(0xb351, [0]), 'Sim_100db351'),
    0x100b372b: Stub(0, pv(0xb372, [0]), 'Sim_100b372b'),
    0x100b487e: Stub(0, pv(0xb487, [0, 1, 1]), 'Sim_100b487e'),
    0x100bc280: Stub(1, p_err),
}
# callbacks: `ret` bytes with a logging hook (number of argument words, answer)
CALLBACKS = {CB + 0x10 * i: n for i, n in enumerate([2, 4, 1, 2, 0, 2])}
CB_OBJ, CB_DRV14, CB_DRV18, CB_DRV1C, CB_DRV2_18, CB_SLOT = sorted(CALLBACKS)


def install(m):
    if getattr(m, '_w8z', False):
        return
    m._w8z = True

    def make(addr, st):
        name = 'stub_%x' % addr

        def cb(uc, address, size, ud):
            if st.mine and addr not in ST.get('active', ()):
                return
            esp = uc.reg_read(UC_X86_REG_ESP)
            raw = list(struct.unpack('<%dI' % st.nargs, bytes(uc.mem_read(esp + 4, 4 * st.nargs)))) if st.nargs else []
            n = sum(1 for e in m.api_log if e[0] == name)
            val, extra = st.pol(uc, n, raw)
            args = tuple((v & 0xffff) if i in st.words else logged(v) for i, v in enumerate(raw))
            m.api_log.append((name, args, tuple(extra)))
            if st.kind == 'int':
                uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
                uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
                uc.reg_write(UC_X86_REG_ESP, esp + 4)
            elif st.kind == 'float':
                uc.mem_write(FVAL, struct.pack('<I', val & 0xffffffff))
                uc.reg_write(UC_X86_REG_EIP, SNIP)
            elif st.kind == 'double':
                h = mix(ST['seed'], addr, n)
                lst = ST.get('d%x' % (addr & 0xffff))
                d = lst[n % len(lst)] if lst else struct.unpack('<d', struct.pack('<f', b2f(fb(h))))[0] * \
                    pick(h, [1.0, 1.0, 1.0 + 2.0 ** -40, 3.0])
                uc.mem_write(DVAL, struct.pack('<d', d))
                uc.reg_write(UC_X86_REG_EIP, SNIP + 0x10)
            elif st.kind in ('dret8', 'dret14'):
                # the math error helpers put the saved control word (their last argument) back
                uc.reg_write(UC_X86_REG_FPCW, raw[-1] & 0xffff)
                uc.reg_write(UC_X86_REG_EIP, SNIP + (0x20 if st.kind == 'dret8' else 0x28))
        return cb

    for addr, st in STUBS.items():
        m.raw.hook_add(UC_HOOK_CODE, make(addr, st), None, addr, addr)
        if st.csym and (st.csym in m.syms or not st.csym.startswith("hg_")):   # OpenLibm: only when linked
            c = m.syms[st.csym]
            m.raw.hook_add(UC_HOOK_CODE, make(addr, st), None, c, c)

    def make_cb(addr, nargs):
        name = 'cb_%x' % addr

        def cb(uc, address, size, ud):
            esp = uc.reg_read(UC_X86_REG_ESP)
            raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
            m.api_log.append((name, tuple(logged(v) for v in raw), ()))
            uc.reg_write(UC_X86_REG_EAX, mix(ST['seed'], addr) & 0xffffffff)
        return cb

    for addr, nargs in CALLBACKS.items():
        m.raw.mem_write(addr, b'\xc3')
        m.raw.hook_add(UC_HOOK_CODE, make_cb(addr, nargs), None, addr, addr)
    snips = b'\xd9\x05' + struct.pack('<I', FVAL) + b'\xc3'
    m.raw.mem_write(SNIP, snips)
    m.raw.mem_write(SNIP + 0x10, b'\xdd\x05' + struct.pack('<I', DVAL) + b'\xc3')
    m.raw.mem_write(SNIP + 0x20, b'\xdd\x44\x24\x08\xc3')
    m.raw.mem_write(SNIP + 0x28, b'\xdd\x44\x24\x14\xc3')
    # st(0) trampolines: (original, C) pairs; the C one is registered as a symbol for the spec's `c`
    for i, target, nargs, csym in [(1, 0x100c9572, 3, 'Sim_100c9572')]:     # slot 0 was the exact fabs copy (gone)
        m.raw.mem_write(TRAMP + 0x80 * i, tramp_st0(target, nargs))
        m.raw.mem_write(TRAMP + 0x80 * i + 0x40, tramp_st0(m.syms[csym], nargs))
        m.syms['Tramp_' + csym] = TRAMP + 0x80 * i + 0x40


def tramp_st0(target, nargs):
    code = b'\x55\x89\xe5'
    for k in reversed(range(nargs)):
        code += b'\xff\x75' + bytes([8 + 4 * k])                 # push dword [ebp+8+4k]
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'    # mov eax, target; call eax
    code += b'\x83\xc4' + bytes([4 * nargs])                     # add esp, 4n
    code += b'\xdb\x3d' + struct.pack('<I', OUTF)                # fstp tbyte [OUTF]
    code += b'\x31\xc0\x5d\xc3'                                  # xor eax, eax; pop ebp; ret
    return code


def begin(ctx, active=()):
    m = ctx.m
    install(m)
    ST.clear()
    ST['seed'] = ctx.rng.getrandbits(32)
    ST['active'] = set(active)
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    return ctx.rng


def api_pick(tag, options):
    return lambda n, args: pick(mix(ST['seed'], tag, n), ST.get('a%x' % tag) or options)


# ---- C runtime --------------------------------------------------------------------------------------------
def text(ctx, alphabet, n):
    return bytes(ctx.rng.choice(alphabet) for _ in range(n))


def g_sprintf(ctx):
    r = begin(ctx)
    buf = newbuf(ctx, 0x40)
    f = text(ctx, b'ab%d %s', r.randrange(0, 12))
    return [buf, ctx.buf(len(f) + 4, f + b'\0' + rb(ctx, 3)), ctx.u32(), ctx.u32(), ctx.u32()]


def dbl_words(ctx):
    r = ctx.rng
    k = r.random()
    if k < 0.1:
        lo, hi = 0, r.choice([0x7ff00000, 0xfff00000])
    elif k < 0.2:
        lo, hi = r.choice([0, 1, r.getrandbits(32)]), r.choice([0x7ff80000, 0xfff80000, 0x7fffffff, 0xffffffff])
    elif k < 0.3:
        lo, hi = r.choice([1, r.getrandbits(32)]), r.choice([0x7ff00000, 0xfff00000, 0x7ff7ffff, 0xfff40000])
    elif k < 0.4:
        lo, hi = r.getrandbits(32), r.choice([0, 0x80000000, 0x000fffff, 0x800fffff, 0x00100000])
    elif k < 0.8:
        lo, hi = struct.unpack('<2I', struct.pack('<d', r.uniform(-1e6, 1e6)))
    else:
        lo, hi = r.getrandbits(32), r.getrandbits(32)
    return [lo, hi]


def g_fabs(ctx):
    begin(ctx)
    return dbl_words(ctx)


def g_fclose(ctx):
    r = begin(ctx)
    s = newbuf(ctx, 0x20)
    w32(ctx, s + 0xc, r.choice([0, 1, 2, 0x80, 0x83, 0x40, 0x42, 0x41, 0x102, 0x1088, r.getrandbits(16)]))
    w32(ctx, s + 0x10, r.choice([0, 3, 0xffffffff]))
    w32(ctx, s + 0x1c, r.choice([0, 0, 0x00A86000]))
    return [s]


def g_fsopen(ctx):
    r = begin(ctx)
    name = text(ctx, b'abc.x', r.randrange(1, 9))
    mode = r.choice([b'r', b'rb', b'w+', b'a'])
    return [ctx.buf(len(name) + 1, name + b'\0'), ctx.buf(len(mode) + 1, mode + b'\0'), r.choice([0x10, 0x20, 0x40])]


# ---- units --------------------------------------------------------------------------------------------------
def unit(ctx, who=None):
    """An entity (0x300 bytes) with a record (+0x54) and a block (+0x28); registered in PTRTAB when who given."""
    r = ctx.rng
    u = newbuf(ctx, 0x300)
    rec = newbuf(ctx, 0x60)
    w32(ctx, rec + 8, fbits(ctx))
    w32(ctx, u + 0x54, rec)
    blk = newbuf(ctx, 0x600)
    w32(ctx, blk + 0x50c, r.choice([0, 1, 1, 2]))
    w32(ctx, blk + 0x53c, fbits(ctx))
    w32(ctx, u + 0x28, blk)
    w32(ctx, u, r.choice([0, 1, 1, 2]))
    w32(ctx, u + 4, r.randrange(0, 8))
    w32(ctx, u + 8, r.choice([0, 1, 5, 15, 16, 31, 32, 33, r.getrandbits(32)]))
    for off in (0xd4, 0xd8, 0x58, 0x5c, 0x60):
        w32(ctx, u + off, fbits(ctx))
    w16(ctx, u + 0x1a8, r.choice([0, 1, 2, 0x101, 0x203, 0x102, r.getrandbits(16)]))
    w16(ctx, u + 0x1aa, r.choice([0, 1, 2, 0x101, 0x203, 0x205, r.getrandbits(16)]))
    if r.random() < 0.3:
        wb(ctx, u + 0x1a8, bytes(ctx.m.uc.mem_read(u + 0x1aa, 2)))
    t = r.choice([0, 100, 5000, 0x7ffffff0])
    w32(ctx, 0x1015dc78, t)
    w32(ctx, u + 0x204, t + r.choice([-1, 0, 1, 2000, -2000]))
    w32(ctx, u + 0x218, r.choice([0, 1, 1, 2, 2, 3, 4, 0x11, 0x21, 0x12, 0x22, 0x31, 0x101, r.getrandbits(32)]))
    w32(ctx, u + 0x21c, r.choice([0, 0, 1, 7, 8, 9, 0xffffffff]))
    w32(ctx, u + 0x23a, r.choice([0, 5, 0xffffffff, 0x80000000, 3]))
    w32(ctx, u + 0x278, r.choice([0, 100, 4000, 4500, 5000, 0x7fffffff, 0xffffff00, r.getrandbits(32)]))
    if who is not None:
        w32(ctx, 0x109e8b40 + 4 * who, u)
    return u


def unit_world(ctx):
    """Units in PTRTAB 0..7 (index = low byte of the key words)."""
    us = [unit(ctx, i) for i in range(8)]
    return us


def set_consts_near(ctx, u):
    r = ctx.rng
    w32(ctx, u + 0xd4, r.choice([near(ctx, 0x1012bb10), near(ctx, 0x1012bb2c), near(ctx, 0x1012bb4c),
                                 near(ctx, 0x1012bb58, True), fbits(ctx)]))
    w32(ctx, u + 0xd8, near(ctx, 0x1012bb48))


def g_unit_key(ctx):
    r = begin(ctx)
    us = unit_world(ctx)
    u = r.choice(us)
    set_consts_near(ctx, u)
    ST['v38fe'] = [r32(ctx.m.uc, u + 0x24a), r.getrandbits(32)]
    key = r.choice([0, 1, 0x101, 0x200, 0xffff, r.getrandbits(16)]) | (r.getrandbits(16) << 16)
    rec = r32(ctx.m.uc, u + 0x54)
    # floats near the thresholds the functions compare with
    w32(ctx, rec + 8, r.choice([near(ctx, 0x1012bb18), fbits(ctx)]))
    ST['f5266'] = [r.choice([near(ctx, 0x1012bb38, True), fbits(ctx)]) for _ in range(3)]
    ST['f44e9'] = [r.choice([near(ctx, 0x1012bb60), fbits(ctx)]) for _ in range(3)]
    ST['f3725'] = [r.choice([f2b(b2f(near(ctx, 0x1012bb20)) * b2f(r32(ctx.m.uc, 0x1012bb40))), fbits(ctx)])]
    return [u, key]


def g_unit(ctx):
    return g_unit_key(ctx)[:1]


def g_unit_slot(ctx, mine=()):
    r = begin(ctx, mine)
    us = unit_world(ctx)
    u = r.choice(us)
    set_consts_near(ctx, u)
    ST['f44e9'] = [r.choice([near(ctx, 0x1012bb60), fbits(ctx)]) for _ in range(3)]
    slot = newbuf(ctx, 4)
    t = r32(ctx.m.uc, 0x1015dc78)
    w32(ctx, slot, r.choice([0, 0, 0, 5, -5, 0x7fffffff, 0x80000000, 0x80000001, r.getrandbits(32), t, -t, t + 1,
                             -t - 1, t - 1, 1 - t]))
    return [u, ctx.buf(16, 'rand'), slot]


def g_4e1c(ctx):
    return g_unit_slot(ctx)


def g_45dc(ctx):
    return g_unit_slot(ctx)


def g_12d7(ctx):
    r = begin(ctx, (0x100c45dc,))
    us = unit_world(ctx)
    u = r.choice(us)
    w32(ctx, u + 0x218, r.choice([0, 1, 1, 2, 2, 3, 4, 0xffffffff]))
    w32(ctx, 0x109e4cc0, r.choice([0, 1, 5, 0xffffffff]))
    return [u]


def g_18c8(ctx):
    r = begin(ctx, (0x100c45dc, 0x100c4e1c))
    us = unit_world(ctx)
    u = r.choice(us)
    w32(ctx, u + 0x218, r.choice([0x11, 0x21, 0x31, 0x01, 0x12, 0x22, 0x32, 0x02, 0x03, 0xf1, 0xff12,
                                  r.getrandbits(32)]))
    return [u]


# ---- targets ------------------------------------------------------------------------------------------------
def g_bc6b(ctx):
    r = begin(ctx)
    outs = [newbuf(ctx, 4) for _ in range(4)]
    c54 = b2f(r32(ctx.m.uc, 0x1012bc54))
    ST['f4766'] = [r.choice([f2b(c54 * r.choice([0.5, 1.0, 2.0, -1.0, -2.0])), fbits(ctx), 0, 0x80000000])
                   for _ in range(2)]
    return [fbits(ctx), fbits(ctx), fbits(ctx)] + outs


def target_tables(ctx, u):
    r = ctx.rng
    for i in range(8):
        t = 0x109a8640 + 0x54 * i
        wb(ctx, t, rb(ctx, 0x54))
        w32(ctx, t + 0x10, r.choice([r32(ctx.m.uc, u + 0xd4), fbits(ctx)]))
        w16(ctx, t + 0x24, r.choice([0, 0x20, 0x21, r.getrandbits(16)]))
        e = 0x109e9ba0 + 0x50 * i
        wb(ctx, e, rb(ctx, 0x50))
        w32(ctx, e + 0x14, r.choice([0, 100, -100, 5000, r.getrandbits(32)]))


def g_b4fb(ctx):
    r = begin(ctx)
    us = unit_world(ctx)
    u = r.choice(us)
    idx = r.choice([0, 1, 2, 3, 4, 5, 6, 7])
    kind = r.choice([0x100, 0x200, 0x400, 0x100, 0x200, 0x400, 0, 0x300, 0x800])
    w32(ctx, u + 0xf0, kind | idx | r.choice([0, 0, 0x1000, 0xff0000]))
    w32(ctx, 0x1017b8c8, r.choice([r32(ctx.m.uc, u + 4), 9]))
    target_tables(ctx, u)
    o = us[idx]
    for k in range(3):
        w32(ctx, o + 0x20c + 4 * k, r.choice([0xffffffff, 0, 7]))
    w16(ctx, o + 0x1e, r.choice([0, 0xffff, r.getrandbits(16)]))
    w32(ctx, r32(ctx.m.uc, o + 0x28) + 0x53c, r.choice([fbits(ctx), r32(ctx.m.uc, u + 0xd4)]))
    return [u]


def g_baf8(ctx):
    r = begin(ctx, (0x100cb4fb,))
    us = unit_world(ctx)
    cur = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, cur)
    u = us[cur]
    w32(ctx, u + 0xf0, r.choice([0x201, 0x402, 0x105, r.getrandbits(16)]))
    w32(ctx, u + 0x118, r.choice([0, 0x100, r.getrandbits(32)]))
    if r.random() < 0.9:
        rec = newbuf(ctx, 0x30)
        w16(ctx, rec + 2, r.choice([0, 0x100, 0x200, 0x300, r.getrandbits(16)]))
        w16(ctx, rec + 0x24, r.choice([1, 2, 5, r.getrandbits(16)]))
        if r.random() < 0.2:
            w32(ctx, u + 0xf0, (r32(ctx.m.uc, rec + 0x24) & 0xffff) | 0x200)
        w32(ctx, 0x1017e678, rec)
    else:
        w32(ctx, 0x1017e678, 0)
    return []


def g_398d(ctx):
    r = begin(ctx, (0x100d2669,))
    us = unit_world(ctx)
    u = r.choice(us)
    pp = newbuf(ctx, 0x40)
    w32(ctx, pp, u)
    w32(ctx, pp + 0x1c, fbits(ctx))
    w32(ctx, u + 0x26c, r.choice([4, 4, 6, 7, 0, 2, r.getrandbits(32)]))
    w32(ctx, u + 0x14, r.choice([2, 3, 3, 1, 0]))
    w32(ctx, 0x1017b8c8, r.choice([r32(ctx.m.uc, u + 0xc), 9]))
    sel = r.choice([0x201, 0x202, 0x207, 0x200, 0x101, 0x401, r.getrandbits(12)])
    w32(ctx, u + 0xf0, sel)
    w16(ctx, u + 0x1aa, r.choice([sel, 0x203, 0x103, r.getrandbits(16)]))
    for o in us:
        w32(ctx, o + 0x1fc, r.choice([0xffffffff, 4, 0, 0xb]))
        w32(ctx, o + 0x26c, r.choice([0, 2, 6, r.getrandbits(32)]))
    rec = newbuf(ctx, 0x10)
    t = r.choice([0, 1, 2, 3])
    w32(ctx, rec + 4, t)
    w32(ctx, 0x1014558e + 0xd6 * t, r.choice([0, 1, 1, 3, r.getrandbits(32)]))
    ST['dcb30'] = [r.choice([r.uniform(-20, 20), r.uniform(-5, 5), 3.0, -3.0, 15.0, -15.0, 1e-30, 400.0])
                   for _ in range(2)]
    ST['f34b7'] = [f2b(r.uniform(-40, 40) * b2f(r32(ctx.m.uc, 0x1012bbb8)))]
    return [rec, pp]


def g_9572(ctx):
    r = begin(ctx, (0x100cbc6b,))
    obj = newbuf(ctx, 0x60)
    blk = newbuf(ctx, 0x600)
    w32(ctx, obj + 0x28, blk)
    recs = newbuf(ctx, 0x128 * 2)
    w32(ctx, blk + 0x84, recs)
    w32(ctx, blk + 0x4fc, r.choice([0, 1]))
    for i in range(2):
        rec = recs + 0x128 * i
        w32(ctx, rec + 4, r.choice([0, 1, 2]))
        w32(ctx, rec + 0x11c, r.choice([0, 1, 2]))
    for k in range(3):
        w32(ctx, blk + 0x498 + 4 * k, r.choice([0, newbuf(ctx, 0x20), newbuf(ctx, 0x20)]))
    for t in range(3):
        ty = 0x10145508 + 0xd6 * t
        w32(ctx, ty + 0x10, r.choice([0x45, 0x45, 0, 0x44]))
        w32(ctx, ty + 0x3c, f2b(r.uniform(10, 900)))
        w32(ctx, ty + 0x38, r.choice([1, 10, 100, 0, -5, r.getrandbits(32)]))
    us = unit_world(ctx)
    who = r.randrange(0, 8)
    ST['obc6b'] = [fbits(ctx) for _ in range(4)]
    ST['o9cfb'] = [f2b(r.uniform(-500, 500)) for _ in range(6)]
    ST['o6ea7'] = [f2b(r.uniform(-1, 1)) for _ in range(3)]
    ST['o4c95'] = [f2b(r.uniform(-1, 1)) for _ in range(3)]
    ST['dca04'] = [r.choice([r.uniform(0, 1e6), r.uniform(0, 100), 0.0, 1e-300])]
    kind = r.choice([0x200, 0x200, 0x100, 0x400])
    return [r.choice([obj] * 9 + [0]), kind, who]


# ---- misc ---------------------------------------------------------------------------------------------------
def g_6cf5(ctx):
    r = begin(ctx)
    idx = newbuf(ctx, 0x4c0)
    n = r.choice([0, 1, 50, 97, 98, 99, 100, 101, 0x80000000])
    w32(ctx, idx + 0x4b4, n)
    w32(ctx, idx, newbuf(ctx, 0x20))
    w32(ctx, 0x10151ce4, newbuf(ctx, 0x200))
    ST['vread'] = [0x200] * r.choice([1, 2, 3, 5]) + [r.choice([0, 0x1ff, 5])]
    ST['sizes'] = [r.choice([0, 1, 0x200, 0x201, 0x3ff, 0x400, 0xffffffff, 0x80000000, r.getrandbits(20)])
                   for _ in range(4)]
    ST['vc9f0'] = r.choice([[0x00A81000], [0x00A81000, 0x00A81000, 0], [0]])
    return [idx]


def g_7deb(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x100)
    w16(ctx, obj + 4, r.choice([0, 1, 1, 1]))
    w16(ctx, obj + 6, r.choice([0, 1, 1, 2, 3, 0xffff]))
    rec = newbuf(ctx, 0x20)
    w32(ctx, obj + 0xec, rec)
    w32(ctx, 0x10151cf8, r.choice([0, 1, 1, 2]))
    w32(ctx, 0x109e89d8, r.choice([0, 1, 2, 2]))
    w32(ctx, 0x1017e590, r.choice([0, 1]))
    cur = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, cur)
    us = unit_world(ctx)
    blk = r32(ctx.m.uc, us[cur] + 0x28)
    ent = newbuf(ctx, 0x100)
    w32(ctx, blk, ent)
    idx = r.randrange(0, 8)
    w32(ctx, ent + 0xf0, r.choice([0x100, 0x200, 0x400, 0x400, 0, 0x1100]) | idx)
    w16(ctx, 0x109a8664 + idx * 0x54, r.choice([0, 0x20]))
    w16(ctx, 0x109e9ba0 + idx * 0x50, r.choice([0, 4, 0, 1]))
    cam = newbuf(ctx, 0x40)
    w32(ctx, 0x1014cae0, cam)
    aim = 0x00A83000
    wb(ctx, aim, rb(ctx, 0x100))
    return [obj]


def g_fd28(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x120)
    w16(ctx, obj + 4, r.choice([0, 1, 1, 1]))
    w32(ctx, 0x10179cc4, r.choice([0, 1, 1]))
    w32(ctx, obj + 0x114, CB_OBJ)
    w32(ctx, obj + 0xf0, newbuf(ctx, 8))
    w32(ctx, 0x109b9900, ctx.buf(8, b'FONT1\0\0\0'))
    w32(ctx, 0x1014cec4, ctx.buf(4, b'FNT\0'))
    w32(ctx, 0x101c4c54, ctx.u32())
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    wb(ctx, obj + 0x10, text(ctx, b'abc', 6) + b'\0')
    return [obj]


# ---- 0x100d -------------------------------------------------------------------------------------------------
def camera(ctx):
    r = ctx.rng
    for k in range(9):
        w32(ctx, 0x10185bfc + 4 * k, f2b(r.choice([r.uniform(-1, 1), 0.0, 1.0, -1.0])))
    for a in (0x10185c44, 0x10185c48, 0x10185c4c):
        w32(ctx, a, f2b(r.uniform(-100, 100)))
    w32(ctx, 0x10185c50, f2b(r.choice([0.0, 1.0, 0.5])))
    w32(ctx, 0x10185bd0, 0)
    w32(ctx, 0x10185bec, r.choice([319, 639]))
    w32(ctx, 0x10185bf0, r.choice([0, 10]))
    w32(ctx, 0x10185be0, r.choice([199, 479]))
    w32(ctx, 0x10185bd4, r.choice([160, 320]))
    w32(ctx, 0x10185bf4, r.choice([100, 240]))


def g_2669(ctx):
    r = begin(ctx)
    camera(ctx)
    if r.random() < 0.4:
        # points that land on the edges of the screen rectangle: plain rows, camera at 0
        for k in range(9):
            w32(ctx, 0x10185bfc + 4 * k, f2b(1.0 if k in (0, 4, 8) else 0.0))
        for a in (0x10185c44, 0x10185c48, 0x10185c4c):
            w32(ctx, a, 0)
        u = ctx.m.uc
        bd0, bec, bf0, be0, bd4, bf4 = [struct.unpack('<i', bytes(u.mem_read(a, 4)))[0]
                                        for a in (0x10185bd0, 0x10185bec, 0x10185bf0, 0x10185be0, 0x10185bd4,
                                                  0x10185bf4)]
        z = r.choice([1.0, 2.0, 4.0, 0.5, 8.0])
        qx = r.choice([bd0, bec]) - bd4 + r.choice([-1, 0, 0, 1])
        qy = be0 - bf0 - bf4 - r.choice([bf0, be0]) + r.choice([-1, 0, 0, 1])
        vals = [f2b(qx * z + r.choice([0.0, 0.25 * z, -0.25 * z])), f2b(qy * z + r.choice([0.0, 0.25 * z])),
                f2b(z)]
        return [ctx.buf(4, struct.pack('<I', v)) for v in vals]
    ps = []
    for k in range(3):
        v = r.choice([f2b(r.uniform(-300, 300)), f2b(r.uniform(-2, 2)), fbits(ctx)])
        ps.append(ctx.buf(4, struct.pack('<I', v)))
    return ps


def g_8e43(ctx):
    r = begin(ctx, (0x100d2669,))
    obj = r.choice([newbuf(ctx, 0x600)] * 9 + [0])
    if obj:
        w32(ctx, obj + 0x552, newbuf(ctx, 0x10))
        rec = newbuf(ctx, 0x40)
        w32(ctx, obj + 0x57a, rec)
        tab = newbuf(ctx, 0x100)
        for k in range(0x40):
            w32(ctx, tab + 4 * k, r.choice([0, 1, 15, 200, 255, r.randrange(0, 256)]))
        w32(ctx, rec + 0x20, tab)
        for off in (0x12, 0x16, 0x1a, 0x3c, 0x40, 0x44, 0x56e, 0x572, 0x576):
            w32(ctx, obj + off, fbits(ctx))
        for off in (0x14, 0x18, 0x1c):
            w32(ctx, rec + off, fbits(ctx))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    cam = newbuf(ctx, 0x60)
    w32(ctx, cam + 0x40, f2b(r.choice([1.0, 100.0, 0.5, 0.0, -3.0])))
    w32(ctx, 0x1014cae0, cam)
    ST['o2669'] = [f2b(r.uniform(-50, 700)) for _ in range(6)]
    return [obj]


def g_6d6a(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x100)
    w32(ctx, obj + 0x28, newbuf(ctx, 0x600))
    w32(ctx, obj + 4, r.randrange(0, 4))
    w32(ctx, 0x1017b8c8, r.randrange(0, 4))
    hit = newbuf(ctx, 8)
    w16(ctx, hit + 2, r.choice([0, 0x100, 0x200, 0x300, 0x400, r.getrandbits(16)]))
    ST['v2dda'] = [0, 1, 1, 1]
    ST['h'] = hit
    c = b2f(r32(ctx.m.uc, 0x1012bd38))
    ST['fcbde'] = [r.choice([f2b(c), f2b(c * 2), f2b(c * 0.5), f2b(c + 1e6), fbits(ctx), 0x4f000000, 0x5f000000])]
    return [obj]


def p_2dda_hit(uc, n, a):
    """Sim_1002dda9 for Sim_100d6d6a: logs the segment, writes the find (or 0) through out."""
    extra = list(bytes(uc.mem_read(a[0], 0x44)))
    h = mix(ST['seed'], 0x2dd1, n)
    uc.mem_write(a[1], struct.pack('<I', pick(h, [ST.get('h', 0), ST.get('h', 0), 0])))
    return pick(h >> 4, [0, 1, 1, 1]), extra


def g_82f6(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015dc80, r.choice([0, 1, 2, 3, -1]))
    w32(ctx, 0x1017e684, r.choice([0, 1, 1]))
    w32(ctx, 0x1015245c, r.choice([1, 2, 1, 2, 0, 3]))
    cam = newbuf(ctx, 0x20)
    w32(ctx, 0x1014cae0, cam)
    c60 = r32(ctx.m.uc, 0x1012bd60)
    for i in range(9):
        e = 0x10180a00 + 0x80c * i
        w32(ctx, e, r.choice([0, 1, 1]))
        w32(ctx, e + 0xf8, f2b(r.choice([0.0, 0.5, 1.0, 2.5, -1.0, r.uniform(0, 3)])))
        w32(ctx, e + 0xf4, f2b(r.choice([0.0, 0.5, 0.9, r.uniform(0, 2), -0.5])))
        for off in (0xe4, 0xe8, 0xec):
            w32(ctx, e + off, r.choice([c60, c60, f2b(r.uniform(-20, 20)), f2b(r.choice([1.0, 2.0, 7.0]))]))
        for off in (0xc, 0x10, 0x14, 0xac, 0xb0):
            w32(ctx, e + off, fbits(ctx))
    return []


def palette_world(ctx):
    r = ctx.rng
    drv = newbuf(ctx, 0x20)
    w32(ctx, drv + 0x14, CB_DRV14)
    w32(ctx, drv + 0x18, CB_DRV18)
    w32(ctx, drv + 0x1c, CB_DRV1C)
    w32(ctx, 0x1015e688, drv)
    drv2 = newbuf(ctx, 0x20)
    w32(ctx, drv2 + 0x18, CB_DRV2_18)
    w32(ctx, 0x1015e68c, drv2)
    w32(ctx, 0x109e3d44, CB_SLOT)
    w32(ctx, 0x1014ced4, ctx.buf(4, b'PAL\0'))
    for i in range(0x18):
        wb(ctx, 0x10185620 + 16 * i, text(ctx, b'PALET0123', 8) + b'\0' * 8)
    wb(ctx, 0x00A84000, rb(ctx, 0x300))


def g_3000(ctx):
    r = begin(ctx)
    palette_world(ctx)
    w32(ctx, 0x1017b940, r.choice([0, 1, 1, 1]))
    ST['va7d5'] = r.choice([[0x00A84000], [0]])
    return [ctx.u32()]


def g_3427(ctx):
    r = begin(ctx, (0x100d3000,))
    palette_world(ctx)
    ST['va7d5'] = r.choice([[0x00A84000], [0]])
    return [r.choice([0, 1, 5, 0x10, 0xffffffff, 0x0efe7a9e])]


def g_3510(ctx):
    r = begin(ctx)
    palette_world(ctx)
    w32(ctx, 0x1017e65c, r.choice([0, 1, 5, 0xffffffff]))
    w32(ctx, 0x1017e620, r.choice([0, 0x00A84000]))
    w32(ctx, 0x10152420, r.choice([0, 3, 0xffffffff, 0x10]))
    w32(ctx, 0x1015dc80, r.choice([0, 1, 2, 3, 7, 0xffffffff]))
    ST['va7d5'] = [r.choice([0, 0x00A84000, 0x00A84100]) for _ in range(2)]
    return [r.choice([0, 1, 5, 0x10]), r.choice([0, 1, 100, 1000, -100, 0x7fffffff, -0x80000000]) & 0xffffffff,
            r.choice([0, 1, 2, 3])]


def g_3877(ctx):
    r = begin(ctx, (0x100d3000,))
    palette_world(ctx)
    w32(ctx, 0x1017b940, r.choice([0, 0, 0, 1]))
    w32(ctx, 0x1017b920, r.choice([0, 1]))
    w32(ctx, 0x109e3ce8, r.choice([0x100, 0x300, 1000, 0x80000000, r.getrandbits(32)]))
    ST['va7d5'] = r.choice([[0x00A84000], [0x00A84000], [0]])
    ST['tstep'] = r.choice([0x20, 0x50, 0x90, 0x110])
    ST['a5658'] = [r.choice([0, 0x00A87000, 0x00A87000])]
    return [r.choice([0, 1, 1, 2])]


def h_alloc(n, args):
    lst = ST.get('a5658')
    return lst[n % len(lst)] if lst else 0x00A88000 + 0x100 * n


HEAP_API = {'HeapAlloc': h_alloc, 'HeapFree': api_pick(0x5634, [1, 1, 0])}


def g_list(ctx, tag):
    r = begin(ctx)
    w32(ctx, 0x1017e6bc, r.choice([0, 1, 1]))
    w32(ctx, 0x1017e6c0, r.choice([0, 1, 1]))
    res = 0x00A84000
    size = 0x60 if tag == 0 else 0x48
    n = r.choice([0, 1, 2, 3, 5])
    data = bytearray(rb(ctx, 0x2c + size * 6))
    data[0:4] = struct.pack('<I', r.choice([tag, tag, tag, 1 - tag, 7]))
    data[0x28:0x2c] = struct.pack('<I', n)
    at = 0 if tag == 0 else 0x20
    for i in range(6):
        p = 0x2c + size * i + at
        nm = text(ctx, b'abcXY', r.choice([0, 3, 8, 15]))
        data[p:p + len(nm) + 1] = nm + b'\0'
    wb(ctx, res, data)
    ST['va7d5'] = r.choice([[res], [res], [0]])
    for a in (0x1017e6c4, 0x1017e6c8):
        w32(ctx, a, r.choice([0, 5]))
    wb(ctx, 0x00A88000, bytes(0x1000))
    return [ctx.buf(8, b'LIST\0\0\0\0')]


def g_load(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015965c, r.choice([1, 1, 2]))
    tab = ctx.buf(0x300, bytes(r.choice([0, 4, 4, 0x84, 1]) if i % 2 == 0 else 0 for i in range(0x300)))
    w32(ctx, 0x10159450, tab + 0x100)
    s = r.choice([b'12abc', b'abc', b'\xe9xyz', b'7', b'', b'longername12345']) + rb(ctx, 2)
    return [ctx.buf(len(s) + 1, s + b'\0')]


def spec(name, orig, gen, trials, **kw):
    return dict(name=name, orig=orig, c=name, gen=gen, trials=trials, **kw)


def p_2dda_dispatch(uc, n, a):
    return (p_2dda_hit if ST.get('h') else STUBS_2DDA_DEFAULT)(uc, n, a)


STUBS_2DDA_DEFAULT = STUBS[0x1002dda9].pol
STUBS[0x1002dda9].pol = p_2dda_dispatch

SPECS = [
    # superseded (C runtime replacement, formatting, DECISIONS.md): SDL3 formatter, checked natively against the original
    # spec('Crt_SPrintf', 0x100fc780, g_sprintf, 400),
    spec('Crt_FClose', 0x100fd760, g_fclose, 400),
    spec('Crt_FSOpen', 0x100fd820, g_fsopen, 300),
    spec('Sim_100c0670', 0x100c0670, g_unit_key, 1000, void=True),
    spec('Sim_100c07c5', 0x100c07c5, g_unit_key, 1000, void=True),
    spec('Sim_100c0965', 0x100c0965, g_unit_key, 800),
    spec('Sim_100c0a2d', 0x100c0a2d, g_unit_key, 600),
    spec('Sim_100c0b1d', 0x100c0b1d, g_unit_key, 500),
    spec('Sim_100c1049', 0x100c1049, g_unit_key, 600),
    spec('Sim_100c1169', 0x100c1169, g_unit_key, 800),
    spec('Sim_100c12d7', 0x100c12d7, g_12d7, 600),
    spec('Sim_100c18c8', 0x100c18c8, g_18c8, 1200),
    spec('Sim_100c2279', 0x100c2279, g_unit_key, 1200),
    spec('Sim_100c45dc', 0x100c45dc, g_45dc, 600),
    spec('Sim_100c4e1c', 0x100c4e1c, g_4e1c, 1000),
    spec('Sim_100cbc6b', 0x100cbc6b, g_bc6b, 1500, void=True),
    spec('Sim_100cb4fb', 0x100cb4fb, g_b4fb, 1500,
         ignore_syms={'g_w24AimDz': 4, 'g_w24AimDy': 4, 'g_w24TargetDy': 4, 'g_w24TargetAngle': 4, 'g_w24TargetSet': 4}),
    spec('Sim_100cbaf8', 0x100cbaf8, g_baf8, 800, void=True),
    spec('Sim_100c398d', 0x100c398d, g_398d, 2000, void=True),
    dict(name='Sim_100c9572', orig=TRAMP + 0x80, c='Tramp_Sim_100c9572', gen=g_9572, trials=1500),
    spec('Sim_100c6cf5', 0x100c6cf5, g_6cf5, 600, void=True),
    spec('Sim_100c7deb', 0x100c7deb, g_7deb, 1200, void=True),
    spec('Sim_100cfd28', 0x100cfd28, g_fd28, 400, void=True),
    spec('Sim_100d2669', 0x100d2669, g_2669, 2000),
    spec('Sim_100d8e43', 0x100d8e43, g_8e43, 800),
    spec('Sim_100d943f', 0x100d943f, g_8e43, 800),
    spec('Sim_100d6d6a', 0x100d6d6a, g_6d6a, 800),
    spec('Sim_100d82f6', 0x100d82f6, g_82f6, 600, void=True),
    spec('Sim_100d3000', 0x100d3000, g_3000, 300, void=True),
    spec('Sim_100d3427', 0x100d3427, g_3427, 300, void=True),
    spec('Sim_100d3510', 0x100d3510, g_3510, 1000, api=HEAP_API),
    spec('Sim_100d3877', 0x100d3877, g_3877, 500, void=True, api=HEAP_API),
    spec('Sim_100da805', 0x100da805, lambda ctx: g_list(ctx, 0), 600, api=HEAP_API),
    spec('Sim_100daa08', 0x100daa08, lambda ctx: g_list(ctx, 1), 600, api=HEAP_API),
    spec('Sim_100e122f', 0x100e122f, g_load, 400),
]


# C runtime replacement (DECISIONS.md), recorded exception: the original's fabs (0x100fcb70) reports a NaN or infinite
# argument to the C runtime's maths error routine 0x10102e30 (its only other caller is the runtime's shared x87 error
# path 0x1010a59d); OpenLibm's fabs, which the C calls now, reports nothing (the value is the same). Those calls are
# dropped from both call logs; every other call is still compared.
def _no_math_error(log_o, log_c):
    keep = lambda log: [e for e in log if e[0] != 'stub_10102e30']
    return keep(log_o), keep(log_c)


for _s in SPECS:
    _s.setdefault('log_map', _no_math_error)
