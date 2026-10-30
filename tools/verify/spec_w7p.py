# Differential tests for round 7, letter p (see difftest.py and docs/checker.md).
#
# Helpers that are not converted yet (and the C library stream functions) run as original code in the emulator
# unless a spec activates a stub for them: then an emulator code hook at the helper's own address (the same for
# the original and the C run; code bytes are not patched) appends ('stub_<address>', arguments, extra data) to the
# checker's call log (compared between the two runs like the Windows calls), may write the helper's results
# through its pointer arguments (the same bytes for both runs), answers with a value chosen by the generator and
# returns to the caller by setting EIP/ESP/EAX. Stack addresses are logged as 'stack'; texts and data buffers
# are logged by content (the fwrite stub logs the scrambled bytes, the segment stub the 0x38 segment bytes).
# Float arguments are logged as raw bits: every FPU function here is game logic and must match bit for bit,
# so no spec uses fp=True. Function pointers handed to the functions point at `ret` bytes with a logging hook.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_100a/w7p_files.c', 'sim/unsorted_100b/w7p_misc.c', 'sim/unsorted_100b/w7p_ptrarray.c',
    'sim/unsorted_100c/w7p_states.c', 'sim/unsorted_100c/w7p_fall.c', 'sim/unsorted_100c/w7p_misc.c',
    # existing converted code the functions above call (and what that calls)
    'sim/core/timer.c', 'sim/core/w2a_timers.c', 'sim/render/w2n_polygon.c', 'sim/render/w2n_project.c',
    'sim/unsorted_1000/w2e_mono.c', 'sim/unsorted_1000/w2e_names.c', 'sim/unsorted_1000/w2f_records.c',
    'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2j_slots.c', 'sim/unsorted_1000/w2k_misc.c',
    'sim/unsorted_1000/w2l_fields.c', 'sim/unsorted_1000/w2l_objects.c', 'sim/unsorted_1000/w3o_sphere.c',
    'sim/unsorted_1000/w3s_model.c', 'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1000/w5b_calls.c',
    'sim/unsorted_1001/w2e_names.c', 'sim/unsorted_1001/w2e_tables.c', 'sim/unsorted_1001/w2h_objects.c',
    'sim/unsorted_1001/w2j_misc.c', 'sim/unsorted_1001/w3t_loader.c', 'sim/unsorted_1001/w3t_misc.c',
    'sim/unsorted_1001/w4u_cd.c', 'sim/unsorted_1001/w5c_calls.c', 'sim/unsorted_1001/w5c_math.c',
    'sim/unsorted_1001/w5y_misc.c', 'sim/unsorted_1002/w2e_object.c', 'sim/unsorted_1002/w2e_tables.c',
    'sim/unsorted_1002/w2g_records.c', 'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1002/w2i_misc.c',
    'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_1002/w2k_misc.c', 'sim/unsorted_1002/w2l_tables.c',
    'sim/unsorted_1002/w3o_ray.c', 'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1002/w4v_misc.c',
    'sim/unsorted_1002/w5a_misc.c', 'sim/unsorted_1002/w5y_misc.c', 'sim/unsorted_1002/w5z_calls.c',
    'sim/unsorted_1002/w6d_calls.c', 'sim/unsorted_1003/entry.c', 'sim/unsorted_1003/records.c',
    'sim/unsorted_1003/scramble.c', 'sim/unsorted_1003/setters.c', 'sim/unsorted_1003/w2e_misc.c',
    'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1003/w2l_records.c',
    'sim/unsorted_1003/w3r_misc.c', 'sim/unsorted_1003/w3s_entry.c', 'sim/unsorted_1003/w4u_heap.c',
    'sim/unsorted_1003/w4w_records.c', 'sim/unsorted_1003/w5a_misc.c', 'sim/unsorted_1003/w5c_calls.c',
    'sim/unsorted_1004/tables.c', 'sim/unsorted_1004/w2h_record.c', 'sim/unsorted_1004/w2i_misc.c',
    'sim/unsorted_1004/w3p_net.c', 'sim/unsorted_1004/w4u_heap.c', 'sim/unsorted_1004/w4w_net.c',
    'sim/unsorted_1005/w2g_pool.c', 'sim/unsorted_1005/w2h_tables.c', 'sim/unsorted_1005/w3o_calls.c',
    'sim/unsorted_1005/w4v_misc.c', 'sim/unsorted_1005/w5b_calls.c', 'sim/unsorted_1006/w2e_copy.c',
    'sim/unsorted_1006/w5a_math.c', 'sim/unsorted_1006/w5y_misc.c', 'sim/unsorted_1007/w2e_state.c',
    'sim/unsorted_1007/w2f_misc.c', 'sim/unsorted_1007/w3p_net.c', 'sim/unsorted_1007/w5a_misc.c',
    'sim/unsorted_1008/cell_link.c', 'sim/unsorted_1008/w2g_rect.c', 'sim/unsorted_1008/w2i_rect.c',
    'sim/unsorted_1008/w3o_grid.c', 'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1008/w3s_grid.c',
    'sim/unsorted_1009/w2e_copy.c', 'sim/unsorted_1009/w3o_ray.c', 'sim/unsorted_1009/w3p_box.c',
    'sim/unsorted_1009/w6g_segment.c', 'sim/unsorted_100a/w2i_misc.c', 'sim/unsorted_100a/w2k_list.c',
    'sim/unsorted_100a/w4u_misc.c', 'sim/unsorted_100a/w5b_calls.c', 'sim/unsorted_100a/w5x_calls.c',
    'sim/unsorted_100a/w5y_misc.c', 'sim/unsorted_100a/w5z_calls.c', 'sim/unsorted_100a/w6g_nodes.c',
    'sim/unsorted_100b/fields.c', 'sim/unsorted_100b/w2e_strings.c', 'sim/unsorted_100b/w2h_records.c',
    'sim/unsorted_100b/w2j_misc.c', 'sim/unsorted_100b/w2l_records.c', 'sim/unsorted_100b/w3o_hash.c',
    'sim/unsorted_100b/w3s_array.c', 'sim/unsorted_100b/w4v_wrappers.c', 'sim/unsorted_100b/w4w_buffers.c',
    'sim/unsorted_100b/w5c_calls.c', 'sim/unsorted_100b/w5z_calls.c', 'sim/unsorted_100b/w6g_misc.c',
    'sim/unsorted_100c/entries.c', 'sim/unsorted_100c/w2e_tables.c', 'sim/unsorted_100c/w2i_misc.c',
    'sim/unsorted_100c/w2l_objects.c', 'sim/unsorted_100c/w3o_misc.c', 'sim/unsorted_100c/w3s_entries.c',
    'sim/unsorted_100c/w3t_misc.c', 'sim/unsorted_100c/w4v_timers.c', 'sim/unsorted_100c/w5a_misc.c',
    'sim/unsorted_100c/w5b_calls.c', 'sim/unsorted_100c/w5c_math.c', 'sim/unsorted_100c/w6h_calls.c',
    'sim/unsorted_100d/w2f_records.c', 'sim/unsorted_100d/w2i_select.c', 'sim/unsorted_100d/w2j_nodes.c',
    'sim/unsorted_100d/w4v_misc.c', 'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_100f/w2h_mem.c',
    'sim/unsorted_100f/w2j_str.c', 'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_100f/w6h_crt.c',
    'sim/unsorted_100f/w6i_mem.c', 'sim/unsorted_100f/w6i_misc.c', 'sim/unsorted_1010/w2h_misc.c',
    'sim/unsorted_1010/w2i_crt.c', 'sim/util/field.c', 'sim/util/lookup.c', 'sim/util/record.c', 'sim/util/ring.c',
    'sim/view/matrix.c', 'sim/view/objlist.c', 'sim/view/viewrect.c', 'sim/view/w2d_view.c', 'sim/view/w2l_lists.c',
    'sim/view/w3r_matrix.c', 'sim/win/w4a_heap.c', 'sim/win/w4u_cdaudio.c', 'sim/win/w4v_dll.c', 'sim/win/w4v_heap.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
CODE = 0x00AF8000       # function pointers handed to the functions: a `ret` with a logging hook (16 bytes each)
HEAPBUF = 0x00AC0000    # blocks the HeapAlloc handler hands out (0x200 bytes each)

# helper address -> number of dword arguments logged
STUBS = {
    0x100fef50: 2,      # malloc core (size, flag)
    0x100fd820: 3,      # _fsopen(name, mode, share): texts logged
    0x100febc0: 4,      # fwrite(buffer, size, count, stream): data logged
    0x100fd760: 1,      # fclose
    0x100feb20: 3,      # fseek
    0x100fe970: 1,      # ftell
    0x100fe830: 4,      # fread(buffer, size, count, stream): fills the buffer
    0x100fc780: 2,      # sprintf(buffer, format, ...): formats the text into the buffer, logs it
    0x100fd7e0: 3,      # fprintf(stream, format, text): the text is logged
    0x100ff860: 3,      # _assert
    0x100b469d: 0, 0x100282d4: 4, 0x100ba1c9: 2, 0x1005f949: 0, 0x1008a5e3: 0, 0x100eb070: 2,
    0x100cb4fb: 1, 0x1000b070: 2, 0x1000b587: 2, 0x100cbdb8: 3, 0x100a9ba7: 1,
    0x100c45dc: 3, 0x100c2ae5: 1, 0x100c3550: 2, 0x1002dda9: 3, 0x10033eb3: 5, 0x10002964: 4, 0x10002513: 4,
    0x1003387d: 4, 0x1000a6f0: 2, 0x1000aa9e: 3, 0x100c6cf5: 1,
    0x100035df: 5, 0x10001e1f: 5, 0x10002775: 5,
}
TEXT_ARGS = {0x100fd820: (0, 1), 0x100ba1c9: (1,), 0x100fd7e0: (2,)}
ST = {'active': set(), 'rets': {}, 'effects': {}, 'seed': 0}


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
    w32(ctx, addr, *[fbits(v) for v in vals])


def ur32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def r32(ctx, addr):
    return ur32(ctx.m.uc, addr)


def ustr(uc, addr, limit=300):
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


def cstr(ctx, data, extra=8):
    return ctx.buf(len(data) + 1 + extra, bytes(data) + b'\0' + rb(ctx, extra))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def bitsf(u):
    return struct.unpack('<f', struct.pack('<I', u & 0xffffffff))[0]


def f32(x):
    return bitsf(fbits(x))


def ulps(x, k):
    """The float k steps (in bits) away from the float x (same sign)."""
    return bitsf((fbits(x) + k) & 0xffffffff)


def near_range(ctx, u, o):
    """Puts the unit o at a distance of about 15000 from u (a few float steps either side), with positions whose
    differences are not exact floats, so that the rounding of the squared distance decides the compare."""
    r = ctx.rng
    base = [r.uniform(-3000.0, 3000.0) * r.choice([1.0, 1e-3, 1.0]) for _ in range(3)]
    wf(ctx, u + 0x58, *base)
    pb = [bitsf(r32(ctx, u + k)) for k in (0x58, 0x5c, 0x60)]
    axis = r.choice([0, 1, 2, 3])
    if axis < 3:
        d = [0.0, 0.0, 0.0]
        d[axis] = r.choice([15000.0, -15000.0])
    else:
        d = [15000.0 * 0.6, -15000.0 * 0.8, 0.0]
    for i, k in enumerate((0x58, 0x5c, 0x60)):
        wf(ctx, o + k, ulps(f32(pb[i] + d[i]), r.randrange(-3, 4)) if d[i] else pb[i])


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def rfl(ctx, lo=-1000.0, hi=1000.0, special=()):
    """A float bit pattern: ordinary values, the given thresholds (and their neighbours), zeros, NaN now and then."""
    r = ctx.rng
    c = r.random()
    if c < 0.5:
        return fbits(r.uniform(lo, hi))
    if c < 0.7 and special:
        v = fbits(r.choice(special))
        return (v + r.choice([0, 0, 1, -1])) & 0xffffffff
    if c < 0.8:
        return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, 10.0, -10.0, 180.0, -180.0, 360.0]))
    if c < 0.93:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-6, 6))
    if c < 0.96:
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])
    return fbits(r.uniform(lo, hi))


def float_buf(ctx, n, lo=-1000.0, hi=1000.0):
    return ctx.buf(n, b''.join(struct.pack('<I', rfl(ctx, lo, hi)) for _ in range(n // 4)))


def rname(ctx, maxlen, alpha=b'abcXYZ_09'):
    r = ctx.rng
    return bytes(r.choice(alpha) for _ in range(r.choice([0, 1, 2, 3, 5, r.randrange(0, maxlen + 1)])))


# ---- stubs --------------------------------------------------------------------------------------
def _fmt(uc, fmt, words):
    """A small printf for the formats of these functions (%s %c %d %ld with widths)."""
    out = b''
    i = 0
    k = 0
    while i < len(fmt):
        c = fmt[i:i + 1]
        if c != b'%':
            out += c
            i += 1
            continue
        j = i + 1
        while j < len(fmt) and fmt[j:j + 1] in b'0123456789l-':
            j += 1
        spec = fmt[i + 1:j].replace(b'l', b'')
        conv = fmt[j:j + 1]
        v = words[k]
        k += 1
        if conv == b's':
            s = ustr(uc, v)
        elif conv == b'c':
            s = bytes([v & 0xff])
        elif conv == b'd':
            s = b'%d' % (v - (1 << 32) if v & 0x80000000 else v)
        else:
            s = b'%x' % v
        width = int(spec) if spec.lstrip(b'-').isdigit() else 0
        out += s.rjust(width) if width > 0 else s
        i = j + 1
    return out


def e_sprintf(uc, n, raw, esp):
    words = list(struct.unpack('<8I', bytes(uc.mem_read(esp + 12, 32))))
    text = _fmt(uc, ustr(uc, raw[1]), words)
    uc.mem_write(raw[0], text + b'\0')
    return [text], None


def e_fwrite(uc, n, raw, esp):
    size = min(raw[1] * raw[2], 0x400)
    data = bytes(uc.mem_read(raw[0], size)).hex()
    plan = ST.get('fwrite_short', [])
    val = raw[2] if n >= len(plan) or not plan[n] else (raw[2] - 1) & 0xffffffff
    return [data], val


def e_fread(uc, n, raw, esp):
    size = min(raw[1] * raw[2], 0x400)
    h = mix(ST['seed'], n)
    uc.mem_write(raw[0], bytes((h >> (i % 24)) & 0xff for i in range(size)))
    plan = ST.get('fread_short', [])
    val = raw[2] if n >= len(plan) or not plan[n] else (raw[2] - 1) & 0xffffffff
    return [], val


def e_list(uc, n, raw, esp):
    """0x10033eb3 writes the list pointer through its fifth argument."""
    uc.mem_write(raw[4], struct.pack('<I', ST['list']))
    return [], None


def e_out3(uc, n, raw, esp):
    """0x1003387d writes three dwords through its fourth argument."""
    uc.mem_write(raw[3], struct.pack('<3I', *[mix(ST['seed'], n, k) for k in range(3)]))
    return [], None


def e_hit(uc, n, raw, esp):
    """0x1002dda9 writes the hit pointer through its second argument; the segment's bytes are logged."""
    seg = bytes(uc.mem_read(raw[0], 0x38)).hex()
    uc.mem_write(raw[1], struct.pack('<I', ST['hit']))
    return [seg], None


def e_cell(uc, n, raw, esp):
    """The cell tests write a height through their fifth argument and the hit data into 0x1015dd40.."""
    h = mix(ST['seed'], n, 9)
    out = ST['heights'][h % len(ST['heights'])]
    uc.mem_write(raw[4], struct.pack('<I', out))
    uc.mem_write(0x1015dd40, struct.pack('<3I', *[ST['slopes'][(h >> (4 * k)) % len(ST['slopes'])] for k in range(3)]))
    return [], None


def _stub_cb(m):
    def cb(uc, address, size, ud):
        if address not in ST['active']:
            return
        esp = uc.reg_read(UC_X86_REG_ESP)
        k = STUBS[address]
        raw = list(struct.unpack('<%dI' % k, bytes(uc.mem_read(esp + 4, 4 * k)))) if k else []
        name = 'stub_%x' % address
        n = sum(1 for e in m.api_log if e[0] == name)
        args = []
        for i, v in enumerate(raw):
            if i in TEXT_ARGS.get(address, ()):
                args.append(ustr(uc, v))
            else:
                args.append('stack' if STACK_LO <= v <= STACK_HI else v)
        rets = ST['rets'].get(address) or [0]
        val = rets[n % len(rets)]
        eff = ST['effects'].get(address)
        if eff:
            extra, v2 = eff(m.uc, n, raw, esp)
            args += extra
            if v2 is not None:
                val = v2
        m.api_log.append((name, tuple(args)))
        uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def _code_cb(m):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        k = (address - CODE) // 16
        raw = [ur32(uc, esp + 4 + 4 * i) for i in range(ST['code_nargs'].get(k, 0))]
        n = len(m.api_log)
        m.api_log.append(('code_%d' % k, tuple('stack' if STACK_LO <= a <= STACK_HI else a for a in raw)))
        uc.reg_write(UC_X86_REG_EAX, mix(ST['seed'], n))
    return cb


EFFECTS = {0x100fc780: e_sprintf, 0x100febc0: e_fwrite, 0x100fe830: e_fread, 0x10033eb3: e_list,
           0x1003387d: e_out3, 0x1002dda9: e_hit, 0x100035df: e_cell, 0x10001e1f: e_cell, 0x10002775: e_cell}


def begin(ctx, *stubs):
    """Installs the hooks (once per machine), activates the given stubs, chooses their answers and sets the
    x87 state of the game (control word 0x27f). Must be the first thing a generator does."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w7p', False):
        m._w7p = True
        cb = _stub_cb(m)
        for addr in STUBS:
            m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
        m.raw.hook_add(UC_HOOK_CODE, _code_cb(m), None, CODE, CODE + 16 * 16 - 1)
    ST['active'] = set(stubs)
    ST['seed'] = r.getrandbits(32)
    ST['effects'] = dict(EFFECTS)
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}
    ST['fwrite_short'] = []
    ST['fread_short'] = []
    ST['heap'] = []
    ST['code_nargs'] = {}
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def code(ctx, k, nargs=1):
    """Address of a function pointer target (`ret` with a logging hook) that takes nargs dword arguments."""
    ST['code_nargs'][k] = nargs
    a = CODE + 16 * k
    ctx.m.uc.mem_write(a, b'\xc3')
    return a


def heap_world(ctx):
    """The small-block heap list (0x10159e38) covers nothing, so freed blocks go to HeapFree."""
    w32(ctx, 0x10159e38, 0x10159e38)
    w32(ctx, 0x10159e48, 0, 0)
    w32(ctx, 0x1017e968, ctx.rng.choice([0, 1]))


def a_heapalloc(n, args):
    h = ST.get('heap') or [0]
    return h[n % len(h)]


HEAP_API = {'HeapAlloc': a_heapalloc}


# ---- 0x100ae344 .. 0x100ae68f: file records --------------------------------------------------------
def g_open(ctx):
    r = begin(ctx, 0x100fef50, 0x100fd820)
    heap_world(ctx)
    rec = zbuf(ctx, 0x20) if r.random() < 0.5 else newbuf(ctx, 0x20)
    rets(0x100fef50, r.choice([rec, rec, rec, rec, 0]))
    rets(0x100fd820, r.choice([0x00A7F000, 0x12345678, 0, 0]))
    return [cstr(ctx, rname(ctx, 20, b'abc.\\/09'))]


def g_write(ctx):
    r = begin(ctx, 0x100febc0, 0x100feb20, 0x100fe970)
    rec = newbuf(ctx, 0x10)
    w32(ctx, rec, r.choice([0x00A7F000, ctx.u32()]), r.choice([0, 1, 0x45, 0x46, 0x200, r.getrandbits(32), r.randrange(0, 5000)]),
        r.choice([1, 2, 2, 3, 0, ctx.u32()]))
    count = r.choice([0, 1, 5, 0x1ff, 0x200, 0x201, 0x3ff, 0x400, 0x401, 0x5ff, 0x600, r.randrange(0, 0x900), r.randrange(0, 0x900)])
    src = newbuf(ctx, count + 16)
    ST['fwrite_short'] = [r.random() < 0.12 for _ in range(6)]
    rets(0x100feb20, r.choice([0, 0, 0, 1, 0xffffffff]))
    rets(0x100fe970, r.choice([0, 0x100, r.getrandbits(32)]))
    return [rec, src, count]


def g_close(ctx):
    r = begin(ctx, 0x100fd760)
    heap_world(ctx)
    rec = newbuf(ctx, 0x10)
    rets(0x100fd760, r.choice([0, 0, 0, 1, 0xffffffff]))
    return [rec]


# ---- 0x100b39c3: the entry table 0x101a2100 ---------------------------------------------------------
def node(ctx):
    n = float_buf(ctx, 0xc0, -2.0, 2.0)
    w32(ctx, n + 4, ctx.rng.choice([0, newbuf(ctx, 0x20)]))
    return n


def g_b39c3(ctx):
    r = begin(ctx, 0x100b469d)
    count = r.choice([0, 1, 2, 4, 6])
    keys = [r.randrange(-3, 40) for _ in range(count)]
    idx = [r.randrange(0, 0x30) for _ in range(count)]
    w32(ctx, 0x1017d208, ctx.buf(4 * count + 4, struct.pack('<%di' % count, *keys) + b'\0' * 4))
    w32(ctx, 0x1017d204, ctx.buf(4 * count + 8, struct.pack('<%dI' % count, *idx) + rb(ctx, 8)))
    w32(ctx, 0x101a1168, count)
    w32(ctx, 0x1017d21c, 1)
    w32(ctx, 0x1017d218, r.choice([r.randrange(0, 0x30), 0x401, 0x402]))
    w32(ctx, 0x101c42a4, r.choice([count, count + 1, 0x40]))
    for k in range(0x30):
        w32(ctx, 0x101a212c + k * 0x88, node(ctx) if r.random() < 0.15 else 0)
    w32(ctx, 0x1014d1a8, r.getrandbits(32))
    heap = HEAPBUF + 0x200 * r.randrange(0, 8)
    ST['heap'] = [r.choice([heap, heap, 0])]
    key = r.choice(keys + [r.randrange(-3, 40)]) if keys else r.randrange(-3, 40)
    name = cstr(ctx, rname(ctx, 15))
    m = [r.getrandbits(32) for _ in range(3)]
    ang = [rfl(ctx, -400.0, 400.0) for _ in range(6)]
    parent = r.choice([-2, -1, -1, r.randrange(0, 0x30), r.randrange(0, 0x30)])
    return [key & 0xffffffff, name] + m + ang + [r.getrandbits(32), parent & 0xffffffff, r.getrandbits(32),
                                                 r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)]


# ---- 0x100b8c30: names of a table ------------------------------------------------------------------
def g_b8c30(ctx):
    r = begin(ctx, 0x100282d4)
    w32(ctx, 0x1017e950, 0)
    n = r.choice([0, 1, 2, 3, 5])
    names = [rname(ctx, 9, b'abcABC_1') for _ in range(n + 2)]
    tbl = bytearray(rb(ctx, 0xc + 0xc * (n + 2) + 8))
    struct.pack_into('<h', tbl, 8, r.choice([n, n, n, n, -1, 0]))
    for i, nm in enumerate(names):
        enc = bytes((0x100 - (c if c < 0x80 else c - 0x100)) & 0xff for c in nm) + b'\0'
        enc += rb(ctx, 10 - len(enc))
        tbl[0xc + 0xc * i:0xc + 0xc * i + 10] = enc
    t = ctx.buf(len(tbl), bytes(tbl))
    rets(0x100282d4, r.choice([t, t, t, 0]))
    w32(ctx, 0x1014cf8c, ctx.u32())
    w32(ctx, 0x1014ced8, cstr(ctx, rname(ctx, 6)))
    hashes = zbuf(ctx, 0x3f1 * 4)
    w32(ctx, 0x1015dc30, hashes)
    key = r.choice([r.randrange(0, 50), r.randrange(-5, 0), r.getrandbits(16)])
    if r.random() < 0.5 and key >= 0:
        nm = bytes(ctx.m.uc.mem_read(r32(ctx, 0x1014ced8), 4))
        s = key + sum(c - 0x100 if c >= 0x80 else c for c in nm)
        nd = zbuf(ctx, 0x20)
        w16(ctx, nd, key, r.choice([0, 1, 5]))
        wb(ctx, nd + 4, nm)
        if s >= 0:
            w32(ctx, hashes + 4 * (s % 0x3f1), nd)
        w32(ctx, 0x109f13f4, r.choice([0, newbuf(ctx, 0x20)]))
        w32(ctx, 0x109f13f0, r.choice([0, 1]))
    pick = r.choice(names) if r.random() < 0.7 else rname(ctx, 9, b'abcABC_1')
    pick = bytes(c ^ 0x20 if r.random() < 0.3 and chr(c).isalpha() else c for c in pick)
    return [key & 0xffffffff, cstr(ctx, pick)]


# ---- 0x100b9675: freeing a cache object --------------------------------------------------------------
def g_b9675(ctx):
    r = begin(ctx, 0x100ff860)
    heap_world(ctx)
    obj = newbuf(ctx, 0x20)
    w32(ctx, obj + 0x18, r.choice([0, newbuf(ctx, 0x10), newbuf(ctx, 0x10)]))
    w32(ctx, 0x1017b914, r.choice([0, 0, ctx.u32()]))
    w32(ctx, 0x1017b900, ctx.u32())
    w32(ctx, 0x1017b8f8, ctx.u32())
    fn = r.choice([0, code(ctx, 1)])
    return [r.choice([obj] * 12 + [0]), fn]


# ---- 0x100bae0a: opening along a directory list --------------------------------------------------------
def g_bae0a(ctx):
    r = begin(ctx, 0x100ba1c9, 0x100fc780, 0x100fd820)
    n = r.choice([0, 1, 2, 3])
    items = [cstr(ctx, rname(ctx, 8, b'cd\\:')) for _ in range(n)]
    arr = ctx.buf(16, struct.pack('<iI', 0, 0))
    w32(ctx, arr + 4, r.choice([n, n, n, 0, -1]))
    w32(ctx, arr + 8, ctx.buf(4 * n + 4, struct.pack('<%dI' % n, *items) + b'\0' * 4))
    rec = newbuf(ctx, 0x10)
    w32(ctx, rec + 8, arr)
    w32(ctx, 0x1017d600, r.choice([0, ctx.u32(), ctx.u32(), ctx.u32()]))
    rets(0x100ba1c9, r.choice([rec, rec, rec, 0]))
    w8(ctx, 0x1017d5f8, r.choice([0x61, 0x7a, 0xe0, 0x30]))
    ok = r.randrange(0, 2 * n + 2)
    rets(0x100fd820, *([0] * ok + [0x00A7F000]))
    return [cstr(ctx, rname(ctx, 8)), cstr(ctx, rname(ctx, 3))]


# ---- 0x100bc451: shutdown sequence --------------------------------------------------------------------
def g_bc451(ctx):
    r = begin(ctx, 0x1005f949, 0x1008a5e3, 0x100eb070)
    prev = 0
    for _ in range(r.choice([0, 0, 1, 3])):
        b = zbuf(ctx, 0x10)
        w32(ctx, b, prev)
        prev = b
    w32(ctx, 0x1015f3b0, prev)
    w32(ctx, 0x109b6880, ctx.u32())
    w32(ctx, 0x1017e688, r.choice([0, ctx.u32()]))
    w32(ctx, 0x1017e68c, r.choice([0, ctx.u32()]))
    for i in range(16):
        w32(ctx, 0x1017ee40 + 4 * i, r.choice([0, 0, ctx.u32()]))
    w32(ctx, 0x1015dc30, r.choice([0, 0, zbuf(ctx, 0x3f1 * 4)]))
    w32(ctx, 0x1015ec58, 0)
    w32(ctx, 0x1015eccc, r.choice([0, 1, 2]))
    for a in (0x1015ec88, 0x1015ecc4, 0x1015ec90, 0x1015ec94, 0x1015d084, 0x1015ecc8, 0x1015d008):
        w32(ctx, a, r.choice([0, ctx.u32()]))
    w32(ctx, 0x10164bc0, r.choice([0, 1, 3]))
    for i in range(3):
        w32(ctx, 0x1015f868 + 4 * i, 0)
    w32(ctx, 0x1017d180, 0)
    w32(ctx, 0x109e3d2c, r.choice([0, code(ctx, 2, 0)]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    w32(ctx, 0x1015e534, r.choice([0, ctx.u32()]))
    w32(ctx, 0x109e3e00, r.choice([0, 1]))
    w32(ctx, 0x1017b920, 0)
    w32(ctx, 0x1017b8f8, ctx.u32())
    t = newbuf(ctx, 0x20)
    w32(ctx, t + 0x14, code(ctx, 3, 0))
    w32(ctx, 0x1015e68c, r.choice([0, t]))
    t2 = newbuf(ctx, 0x20)
    w32(ctx, t2 + 0x10, code(ctx, 4, 0))
    w32(ctx, 0x1015e688, r.choice([0, t2]))
    w32(ctx, 0x1015dca0, r.choice([0, 1]))
    w32(ctx, 0x101354d4, r.randrange(0, 0x40), r.choice([0x80, 0x81, 5]), r.randrange(0, 0x40))
    w32(ctx, 0x101354d0, r.choice([0x80, 0x83, 2]))
    return []


# ---- 0x100bd134 .. 0x100bd85b: pointer arrays ---------------------------------------------------------
def ptr_array(ctx, items, cap=None):
    a = newbuf(ctx, 0x10)
    n = len(items)
    w32(ctx, a, cap if cap is not None else n + ctx.rng.randrange(0, 4), n)
    w32(ctx, a + 8, ctx.buf(4 * n + 8, struct.pack('<%dI' % n, *items) + rb(ctx, 8)))
    return a


def cache_obj(ctx):
    r = ctx.rng
    o = newbuf(ctx, 0x10)
    inner = ptr_array(ctx, [r.choice([0, newbuf(ctx, 8)]) for _ in range(r.choice([0, 1, 2]))])
    w32(ctx, o + 4, r.choice([0, inner]))
    return o


def g_bd134(ctx):
    r = begin(ctx, 0x100fd760)
    heap_world(ctx)
    objs = [r.choice([0, cache_obj(ctx), cache_obj(ctx)]) for _ in range(r.choice([0, 1, 2, 3]))]
    w32(ctx, 0x1017e1d0, r.choice([0, ptr_array(ctx, objs)]))
    w32(ctx, 0x1017e1c8, r.choice([0, newbuf(ctx, 0x10)]))
    w32(ctx, 0x1017e1c0, r.choice([0, 0x00A7F000]))
    n = r.choice([0, 1, 3])
    w32(ctx, 0x1017e1bc, n)
    for i in range(1, n + 1):
        w32(ctx, 0x1017dfd0 + 32 * i, ctx.u32())
    return []


def g_bd1f3(ctx):
    begin(ctx)
    heap_world(ctx)
    return [cache_obj(ctx)]


def g_bd560(ctx):
    r = begin(ctx, 0x100fef50)
    heap_world(ctx)
    a = newbuf(ctx, 0x10)
    items = newbuf(ctx, 0x40)
    rets(0x100fef50, r.choice([a] * 12 + [0]), r.choice([items, items, items, 0]))
    return []


def g_bd5fc(ctx):
    r = begin(ctx, 0x100fef50)
    heap_world(ctx)
    n = r.choice([0, 1, 2, 5, 16, 40, -1, -3])
    a = newbuf(ctx, 0x10)
    items = newbuf(ctx, 4 * max(n, 0) + 8)
    rets(0x100fef50, r.choice([a] * 12 + [0]), r.choice([items, items, items, 0]))
    return [n & 0xffffffff]


def g_bd85b(ctx):
    r = begin(ctx)
    heap_world(ctx)
    n = r.choice([0, 1, 2, 3, 5, 8])
    items = [r.choice([0, newbuf(ctx, 8), newbuf(ctx, 8), 7]) for _ in range(n)]
    a = ptr_array(ctx, items)
    if r.random() < 0.1:
        w32(ctx, a + 4, r.choice([-1, n + 1 if n else 0]))
    value = r.choice(items + [0, 7]) if items and r.random() < 0.8 else r.choice([0, 7, ctx.u32()])
    return [a, value, r.choice([0, code(ctx, 5)])]


# ---- units (0x100c....): the pointer table 0x109e8b40, blocks, records -----------------------------
SPEEDS = (1000.0, 5000.0, 6000.0, 999.0, 1001.0, 4999.0, 5001.0)


def ring(ctx):
    for i in range(0x7f):
        w32(ctx, 0x101cffc0 + 4 * i, ctx.rng.getrandbits(31))
    w32(ctx, 0x1017cd38, ctx.rng.randrange(0, 0x7f))


def make_unit(ctx, i, nunits):
    r = ctx.rng
    u = float_buf(ctx, 0x300, -400.0, 400.0)
    w32(ctx, u + 4, i if r.random() < 0.9 else r.randrange(0, nunits))
    w32(ctx, u + 8, r.randrange(0, 4))
    w32(ctx, u + 0x14, r.choice([0, 1, 1, 2, 3]))
    for k in (0x58, 0x5c, 0x60):
        w32(ctx, u + k, rfl(ctx, -30000.0, 30000.0, (20000.0,)))
    w32(ctx, u + 0xd4, rfl(ctx, 0.0, 8000.0, SPEEDS))
    w32(ctx, u + 0x7c, rfl(ctx, -3000.0, 3000.0))
    w32(ctx, u + 0x246, rfl(ctx, 0.0, 300.0))
    blk = float_buf(ctx, 0x600, -50.0, 50.0)
    w32(ctx, blk + 0x50c, r.choice([0, 1]))
    w32(ctx, blk + 0x510, r.choice([-1, 0, 3, 5, 6, 7, 10]))
    w32(ctx, blk + 0x514, r.choice([-1, 0, 1, 5]))
    w32(ctx, blk + 0x4f4, r.choice([0, 0, 1]))
    w32(ctx, blk + 0x4e8, rfl(ctx, 0.0, 60.0, (20.0, 40.0)))
    w32(ctx, blk + 0x534, rfl(ctx, -90.0, 90.0, (10.0, 30.0, -10.0)))
    w32(ctx, blk + 0x54c, rfl(ctx, -20.0, 0.0, (-13.730763244628905, -12.115379333496094, -16.153839111328125)))
    w32(ctx, blk + 0x540, r.choice([fbits(0.0), rfl(ctx, -5.0, 5.0)]))
    w32(ctx, u + 0x28, blk)
    rec = newbuf(ctx, 0x80)
    w8(ctx, rec + 0x1e, r.choice([0, 0, 1, 2]))
    w32(ctx, u + 0x54, rec)
    nd = float_buf(ctx, 0xa0, -1.0, 1.0)
    w32(ctx, u + 0x48, nd)
    w32(ctx, u + 0x1f4, r.choice([0, 0, 1]))
    w32(ctx, u + 0x80, r.choice([0, 0, 1, 5]))
    w32(ctx, u + 0x218, r.choice([0, 1, 2, 3, 4, 1, 2, 3, -1]))
    w32(ctx, u + 0xf0, r.choice([0x200, 0x400, 0x100, 0, 0x200 | r.randrange(0, nunits), r.getrandbits(32)]))
    w16(ctx, u + 0x1aa, r.choice([0x200 | r.randrange(0, nunits), 0x200 | r.randrange(0, nunits), 0x100, 0, 0x8000,
                                  r.getrandbits(16)]))
    w16(ctx, u + 0x1a8, r.getrandbits(16))
    w32(ctx, u + 0x118, r.choice([0x100, 0x105, r.getrandbits(32)]))
    w32(ctx, u + 0x23a, r.choice([0, 1, 2, -1, 7]))
    w32(ctx, u + 0x26c, r.choice([0, 1, 1, 3]))
    w16(ctx, u + 0x25e, r.choice([0, 0, 1]))
    w16(ctx, u + 0x1f8, r.choice([0, 10, 3]))
    w32(ctx, u + 0x264, r.choice([0, 0, 1]))
    wb(ctx, u + 0x270, bytes(r.choice([0, 0, 1]) for _ in range(8)))
    return u


def unit_world(ctx, n=None):
    r = ctx.rng
    n = n or r.randrange(1, 6)
    units = [make_unit(ctx, i, n) for i in range(n)]
    for i, u in enumerate(units):
        w32(ctx, 0x109e8b40 + 4 * i, u)
    w32(ctx, 0x1015dc64, n)
    w32(ctx, 0x1017b8c8, r.randrange(0, n))
    clock = r.randrange(0, 100000)
    w32(ctx, 0x1015dc78, clock)
    for u in units:
        for k in (0x200, 0x204, 0x21c):
            w32(ctx, u + k, r.choice([clock - 1, clock, clock + 1, clock + r.randrange(-2000, 2000), 0]))
        w32(ctx, u + 0x21c, r.choice([r32(ctx, u + 0x21c), clock - 10000, clock - 9999]))
    for g in range(4):
        w32(ctx, 0x109b7d54 + g * 0x6c, r.choice([0, 1, 2]))
    ring(ctx)
    w16(ctx, 0x101654f2, r.getrandbits(16))
    return units


AI_STUBS = (0x100cb4fb, 0x1000b070, 0x1000b587, 0x100cbdb8, 0x100a9ba7)


def entries(ctx):
    """The entry table 0x109e4cc0 / 0x109e4cc4 (0x44 bytes each) and a list for 0x10033eb3."""
    r = ctx.rng
    count = r.choice([0, 1, 2, 3, 6])
    w32(ctx, 0x109e4cc0, count)
    for k in range(count + 2):
        w32(ctx, 0x109e4cc4 + 0x44 * k, *[rfl(ctx, -500.0, 500.0) for _ in range(0x11)])
        w32(ctx, 0x109e4cc4 + 0x44 * k + 0x10, r.choice([0, 0, 1, ctx.u32()]))
    n = r.choice([0, 1, 2, 3, 5])
    lst = ctx.buf(4 * n + 8, struct.pack('<%di' % n, *[r.choice([0, 1, 2, count - 1, r.randrange(-1, 8)]) for _ in range(n)]) + b'\0' * 8)
    ST['list'] = r.choice([lst, lst, lst, 0])
    rets(0x10033eb3, r.choice([n, n, n, 0, n + 1 if n else 0]))


def g_state(ctx):
    r = begin(ctx, *AI_STUBS, 0x100c3550, 0x100fc780, 0x100fd7e0)
    units = unit_world(ctx)
    u = r.choice(units)
    rets(0x100c3550, r.choice([0, 0, 1]))
    if r.random() < 0.3:
        w32(ctx, u + 0x218, 0)
    key = r.choice([0x200 | r.randrange(0, len(units)), 0x200, 0x400, r.getrandbits(16)])
    return [u, key | (r.getrandbits(16) << 16)]


def g_c0dd7(ctx):
    r = begin(ctx, *AI_STUBS)
    units = unit_world(ctx)
    u = r.choice(units)
    w32(ctx, 0x1015dc80, r.choice([0, 1, 2, 16, -3]))
    w32(ctx, u + 0x268, rfl(ctx, 0.0, 8000.0, SPEEDS))
    key = r.randrange(0, len(units)) | r.choice([0x200, 0x100, 0, r.getrandbits(16) & 0xff00])
    return [u, key | (r.getrandbits(16) << 16)]


def g_search(ctx):
    r = begin(ctx, *AI_STUBS, 0x100c45dc, 0x10033eb3, 0x10002964, 0x10002513)
    units = unit_world(ctx)
    entries(ctx)
    u = r.choice(units)
    w32(ctx, u + 0x21c, r.choice([r.getrandbits(32), r.randrange(-50000, 50000)]))
    rets(0x10002964, *[r.choice([-1, 0, 1, 2, 5]) for _ in range(3)])
    rets(0x10002513, *[r.choice([-1, 0, 1, 2, 5]) for _ in range(3)])
    rets(0x100c45dc, r.choice([0, 1, 1, 2]))
    return [u, r.getrandbits(32)]


def g_c2ae5_states(ctx):
    r = begin(ctx, *AI_STUBS, 0x100c2ae5)
    units = unit_world(ctx)
    u = r.choice(units)
    rets(0x100c2ae5, r.choice([0, 0, 1]))
    return [u, r.getrandbits(32)]


def g_c264a(ctx):
    r = begin(ctx, 0x100fc780, 0x100fd7e0)
    units = unit_world(ctx)
    u = r.choice(units)
    w32(ctx, 0x101668f4, r.choice([0, 1, 1]))
    w32(ctx, 0x101668f8, r.choice([0, 0x00A7F000, 0x00A7F000]))
    return [u]


def cell_env(ctx):
    """Grid of 9 x 9 cells with cell records and node chains, plus the offset table of Sim_100881dd (spec_w3o.py)."""
    r = ctx.rng
    w32(ctx, 0x1017c8c4, 1 if r.random() < 0.9 else 0)
    wb(ctx, 0x1017c840, struct.pack('<64i', *[r.choice([-1, 0, 1, 1, 0]) for _ in range(64)]))
    wb(ctx, 0x1017ba4c, bytes(0x18c * 9))
    cells = []
    for k in range(6):
        c = zbuf(ctx, 0x80)
        w32(ctx, c + 8, r.choice([0, 1, 5, 5, 5]))
        first = prev = 0
        for j in range(r.choice([0, 1, 2, 3])):
            n = zbuf(ctx, 0x100)
            w32(ctx, n + 0x34, r.choice([5, 8, 0, 0, 5, 8, 1, 2]))
            if prev:
                w32(ctx, prev + 0x18, n)
            else:
                first = n
            prev = n
        w32(ctx, c + 0x18, first)
        cells.append(c)
    for i in range(9):
        for j in range(9):
            w32(ctx, 0x1017ba4c + i * 0x18c + j * 0x2c, r.choice(cells))
    w32(ctx, 0x1017c8cc, r.choice(cells))
    wb(ctx, 0x1036f2a0, bytes(0x30))


def make_seg(ctx, length=600.0):
    r = ctx.rng
    s = zbuf(ctx, 0x38)
    a = [r.uniform(-240000, 240000), r.uniform(-500, 3000), r.uniform(-240000, 240000)]
    d = [r.uniform(-length, length) for _ in range(3)]
    b = [x + y for x, y in zip(a, d)]
    wf(ctx, s, *a)
    wf(ctx, s + 0xc, *b)
    wf(ctx, s + 0x18, *d)
    st = r.choice([0, 0, 1, 2])
    if st:
        ln = sum(x * x for x in d) ** 0.5 or 1.0
        wf(ctx, s + 0x24, *[x / ln for x in d])
        wf(ctx, s + 0x30, ln)
    w32(ctx, s + 0x34, st)
    return s


def g_c292c(ctx):
    r = begin(ctx, 0x100035df, 0x10001e1f, 0x10002775)
    cell_env(ctx)
    w32(ctx, 0x1014cae0, zbuf(ctx, 0x100))
    ST['heights'] = [rfl(ctx, -500.0, 3000.0, (0.0, 2000.0)) for _ in range(4)]
    if r.random() < 0.4:
        ST['heights'] = [fbits(r.uniform(-20.0, 20.0)) for _ in range(4)]
    ST['slopes'] = [rfl(ctx, -1.0, 1.0, (0.766,)) for _ in range(4)]
    rets(0x100035df, r.choice([0, 1, 1]), r.choice([0, 1]))
    rets(0x10001e1f, r.choice([0, 1, 1]), r.choice([0, 1]))
    rets(0x10002775, r.choice([0, 1, 1]), r.choice([0, 1]))
    w32(ctx, 0x1015dd50, rfl(ctx, -1.0, 1.0, (0.766,)))
    w32(ctx, 0x1015dd40, rfl(ctx), rfl(ctx), rfl(ctx))
    unit = float_buf(ctx, 0x300, -400.0, 400.0)
    w32(ctx, unit + 0x7c, rfl(ctx, -3000.0, 4000.0, (2000.0, -2000.0)))
    if r.random() < 0.4:
        h = bitsf(r.choice(ST['heights']))
        if h == h and abs(h) < 1e6:
            w32(ctx, unit + 0x7c, fbits(ulps(f32(h + r.choice([2000.0, -2000.0])), r.randrange(-3, 4))))
    w32(ctx, unit + 0x246, rfl(ctx, 0.0, 400.0))
    return [make_seg(ctx), unit]


def g_c31a6(ctx):
    r = begin(ctx, 0x100a9ba7)
    units = unit_world(ctx)
    u = r.choice(units)
    nd = r32(ctx, u + 0x48)
    wf(ctx, nd + 0x4c, *[r.uniform(-1, 1) for _ in range(9)] + [r.uniform(-30000, 30000) for _ in range(3)])
    seg = r.choice([zbuf(ctx, 0x38), newbuf(ctx, 0x38)])
    side = r.choice([0, 1, 5, -1, -2, 0x7fffffff, -0x80000000])
    d = r.choice([r.randrange(0, 16), r.randrange(0, 16), r.randrange(-15, 0), 16, 17, 31])
    scale = rfl(ctx, -5000.0, 5000.0, (3000.0,))
    return [u, seg, side & 0xffffffff, (d & 0xffff) | (r.getrandbits(16) << 16), scale, r.choice([0, 1, 1, 7])]


def g_c3550(ctx):
    r = begin(ctx, 0x1002dda9)
    u = float_buf(ctx, 0x300, -3000.0, 3000.0)
    w32(ctx, u + 4, ctx.u32())
    w32(ctx, u + 0xf0, r.choice([0x200, 0x400, 0x600, 0x100, 0, r.getrandbits(32)]) | r.randrange(0, 4))
    hit = newbuf(ctx, 0x40)
    w16(ctx, hit + 2, r.choice([0x100, 0x200, 0x300, 0, r.getrandbits(16)]))
    w16(ctx, hit + 0x24, r.choice([0, 1, 2, 3, r.getrandbits(16)]))
    ST['hit'] = r.choice([hit, hit, hit, 0])
    rets(0x1002dda9, r.choice([0, 1, 1, 1, 5]))
    return [u, rfl(ctx, -3000.0, 3000.0)]


def g_c376e(ctx):
    r = begin(ctx, *AI_STUBS)
    units = unit_world(ctx)
    u = r.choice(units)
    w32(ctx, u + 0x26c, r.choice([1, 1, 1, 0, 3]))
    w16(ctx, u + 0x25e, r.choice([0, 0, 0, 1]))
    w16(ctx, u + 0x1f8, r.choice([0, 0, 0, 10]))
    w32(ctx, r32(ctx, u + 0x54) + 8, r.choice([fbits(0.0), rfl(ctx, -1.0, 1.0)]))
    return [u]


def g_c3d16(ctx):
    r = begin(ctx, *AI_STUBS)
    units = unit_world(ctx)
    u = r.choice(units)
    w16(ctx, u + 0x1aa, r.choice([0x200 | r.randrange(0, len(units))] * 4 + [0, 0x8200, 0x100, 0x1ff, r.getrandbits(16)]))
    if r.random() < 0.5:
        for o in units:
            m = r.randrange(0, 8)
            x = 22.5 + 45.0 * m - 360.0 * (m >= 4)
            w32(ctx, o + 0x68, fbits(r.choice([0.0, 0.0, 0.1, -3.7])))
            v = f32(x + bitsf(r32(ctx, o + 0x68)))
            for k in (0xe8, 0x110):
                w32(ctx, o + k, fbits(ulps(v, r.randrange(-2, 3))))
    return [u]


def g_c4035(ctx):
    r = begin(ctx, 0x10033eb3)
    entries(ctx)
    ring(ctx)
    a = float_buf(ctx, 0x100, -500.0, 500.0)
    b = float_buf(ctx, 0x100, -500.0, 500.0)
    return [r.choice([a] * 12 + [0]), r.choice([b] * 12 + [0]), ctx.u32()]


def g_c4498(ctx):
    r = begin(ctx, 0x1003387d, 0x100cbdb8)
    units = unit_world(ctx)
    u = r.choice(units)
    w32(ctx, 0x1017e5a8, r.choice([0, 1, 3, 5, 0x7f, 0x80, -1]))
    for k in range(6):
        e = 0x109a8640 + k * 0x54
        wb(ctx, e, rb(ctx, 0x54))
        w16(ctx, e + 0x24, r.choice([1, 0x401, 0, 3]))
        w32(ctx, e + 8, r.choice([r32(ctx, u + 4) | 0x200, 0x200, ctx.u32()]))
    w32(ctx, u + 0x118, r.choice([0x100 | r.randrange(0, 6), 0x100 | r.randrange(0, 6), 0x105, 0, r.getrandbits(32)]))
    rets(0x1003387d, r.choice([0, 1, 1, 2]))
    slot = ctx.buf(8, struct.pack('<I', r.choice([0x80000000, r32(ctx, u + 0x118), 0x100 | r.randrange(0, 6), ctx.u32()])))
    return [u, ctx.u32(), slot]


def g_c4a1f(ctx):
    r = begin(ctx, 0x1000a6f0)
    units = unit_world(ctx)
    u = r.choice(units)
    me = r32(ctx, u + 4)
    for o in units:
        w32(ctx, o + 0xf0, r.choice([0x200 | me, 0x200 | me, 0x100 | me, me, r.getrandbits(32)]))
        w16(ctx, o + 0x1aa, r.choice([0x200 | me, 0x200 | (me + 1), 0, r.getrandbits(16)]))
        for k in (0x58, 0x5c, 0x60):
            w32(ctx, o + k, fbits(bitsf(r32(ctx, u + k)) + r.choice([r.uniform(-9000, 9000), r.uniform(-20000, 20000)]))
                if r.random() < 0.9 else rfl(ctx))
    w32(ctx, u + 0x208, r.choice([0, 1, 2]))
    if r.random() < 0.4:
        for o in units:
            if o != u:
                near_range(ctx, u, o)
    w32(ctx, u + 0x20c, *[r.choice([-1, r.randrange(0, len(units))]) for _ in range(3)])
    rets(0x1000a6f0, r.choice([0, 1, 7]))
    return [u]


def g_c4b67(ctx):
    r = begin(ctx, 0x1000aa9e)
    units = unit_world(ctx)
    u = r.choice(units)
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    for o in units:
        for k in (0x58, 0x5c, 0x60):
            w32(ctx, o + k, fbits(bitsf(r32(ctx, u + k)) + r.choice([r.uniform(-9000, 9000), r.uniform(-20000, 20000)]))
                if r.random() < 0.9 else rfl(ctx))
    if r.random() < 0.5:
        for o in units:
            if o != u:
                near_range(ctx, u, o)
        for g in range(4):
            w32(ctx, 0x109b7d54 + g * 0x6c, g)
        for i, o in enumerate(units):
            w32(ctx, o + 4, i)
            w32(ctx, o + 8, r.randrange(0, 4))
            w32(ctx, o + 0x14, r.choice([0, 1]))
        rets(0x1000aa9e, 0, 0, r.choice([0, 1]))
        return [u]
    rets(0x1000aa9e, *[r.choice([0, 0, 1, 7]) for _ in range(3)])
    return [u]


# ---- 0x100c6b30 .. 0x100c6fbd ----------------------------------------------------------------------
def g_octal(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015965c, r.choice([1, 1, 2, 0]))
    if r.random() < 0.3:
        tbl = bytearray(rb(ctx, 0x400))       # entries for the bytes -128..255 around the pointer
        for c in b' 0123456789':
            tbl[0x102 + 2 * c] = (tbl[0x102 + 2 * c] & ~8) | (8 if c == 0x20 else 0)
        w32(ctx, 0x10159450, ctx.buf(len(tbl), bytes(tbl)) + 0x102)
    s = bytes(r.choice(b'  01234567789\0x\t') for _ in range(r.randrange(0, 14)))
    s = r.choice([s, b'   0755 ', b'0000644\0', b'  12', b'777777777777', b'  ', b'17x', b'\xe0 1'])
    n = r.choice([0, 1, 2, 3, 6, 8, 12, len(s), len(s) + 1, -1])
    return [n & 0xffffffff, ctx.buf(len(s) + 4, s + b'\0' + rb(ctx, 3))]


def g_c6e27(ctx):
    r = begin(ctx, 0x100fef50, 0x100fd820, 0x100c6cf5)
    rec = newbuf(ctx, 0x4c0)
    rets(0x100fef50, r.choice([rec, rec, rec, 0]))
    rets(0x100fd820, r.choice([0x00A7F000, 0x00A7F000, 0]))
    return [cstr(ctx, rname(ctx, 12))]


def g_c6fbd(ctx):
    r = begin(ctx, 0x100fe830)
    rec = newbuf(ctx, 0x10)
    n = r.choice([0, 1, 4, 0x200, r.randrange(0, 0x300)])
    ST['fread_short'] = [r.random() < 0.2]
    return [rec, newbuf(ctx, n + 4), n]


def spec(name, orig, gen, trials, **kw):
    return dict(name=name, orig=orig, c=name, gen=gen, trials=trials, **kw)


SPECS = [
    spec('FileRec_OpenWrite', 0x100ae344, g_open, 300),
    spec('FileRec_OpenAppend', 0x100ae3a8, g_open, 300),
    spec('FileRec_OpenUpdate', 0x100ae40c, g_open, 300),
    spec('FileRec_WriteScrambled', 0x100ae4bc, g_write, 500),
    spec('FileRec_Close', 0x100ae68f, g_close, 300),
    spec('Sim_100b39c3', 0x100b39c3, g_b39c3, 500, api=HEAP_API),
    spec('Sim_100b8c30', 0x100b8c30, g_b8c30, 500),
    spec('Sim_100b9675', 0x100b9675, g_b9675, 400, void=True),
    spec('Sim_100bae0a', 0x100bae0a, g_bae0a, 400),
    spec('Sim_100bc451', 0x100bc451, g_bc451, 400, void=True),
    spec('Sim_100bd134', 0x100bd134, g_bd134, 400, void=True),
    spec('Sim_100bd1f3', 0x100bd1f3, g_bd1f3, 300, void=True),
    spec('PtrArray_Create', 0x100bd560, g_bd560, 300),
    spec('PtrArray_CreateSized', 0x100bd5fc, g_bd5fc, 300),
    spec('PtrArray_Remove', 0x100bd85b, g_bd85b, 500),
    spec('Sim_100c0bdd', 0x100c0bdd, g_state, 600),
    spec('Sim_100c0dd7', 0x100c0dd7, g_c0dd7, 600),
    spec('Sim_100c145b', 0x100c145b, g_search, 500),
    spec('Sim_100c1609', 0x100c1609, g_search, 500),
    spec('Sim_100c1c67', 0x100c1c67, g_c2ae5_states, 600),
    spec('Sim_100c1e69', 0x100c1e69, g_c2ae5_states, 600),
    spec('Sim_100c209c', 0x100c209c, g_c2ae5_states, 600),
    spec('Sim_100c264a', 0x100c264a, g_c264a, 600),
    spec('Sim_100c292c', 0x100c292c, g_c292c, 1000),
    spec('Sim_100c31a6', 0x100c31a6, g_c31a6, 600, void=True),
    spec('Sim_100c3550', 0x100c3550, g_c3550, 500),
    spec('Sim_100c376e', 0x100c376e, g_c376e, 600, void=True),
    spec('Sim_100c3d16', 0x100c3d16, g_c3d16, 600),
    spec('Sim_100c4035', 0x100c4035, g_c4035, 500),
    spec('Sim_100c4498', 0x100c4498, g_c4498, 500, void=True),
    spec('Sim_100c4a1f', 0x100c4a1f, g_c4a1f, 1000),
    spec('Sim_100c4b67', 0x100c4b67, g_c4b67, 1000),
    spec('Str_ParseOctal', 0x100c6b30, g_octal, 500),
    spec('Sim_100c6e27', 0x100c6e27, g_c6e27, 300),
    spec('Sim_100c6fbd', 0x100c6fbd, g_c6fbd, 300),
]
