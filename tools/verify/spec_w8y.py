# Differential tests for round 8, letter y (see difftest.py and docs/checker.md).
#
# Converted callees run as real code on both sides (the original callee in the original run, its C conversion in
# the C run); both are verified already. Callees that are not converted (or that another hook file patches, or
# that another round-8 helper works on) are called through their original address by both versions; the ones
# that matter are replaced, for the trials that ask for it, by a stub: an emulator code hook at the callee's own
# address (the same for both runs; code bytes are not patched) that appends ('stub_<address>', arguments, extra
# data) to the checker's call log (compared between the two runs like the Windows calls; stack addresses are
# logged as 'stack', texts by content), may write results through its pointer arguments (the same bytes for both
# runs, computed from what it is given and the run's own state), answers with a value chosen by the generator and
# returns to the caller (EIP/ESP/EAX set). The archive and stream stubs (_read/_lseek/fread/fseek) serve the bytes
# of a file image built by the generator with a position kept per run. Allocations (malloc core 0x100fef50,
# HeapAlloc, HeapReAlloc, the archive allocator) are handed out from a pool in argument memory, in call order per
# run, with chosen ones failing. Function pointers handed to or stored for the functions point at `ret` bytes with
# a logging hook. Every FPU spec here is checked bit for bit (no float tolerance).
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_EDX, UC_X86_REG_FPCW,
                               UC_X86_REG_FPSW, UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_100a/w8y_archive.c', 'sim/unsorted_100a/w8y_misc.c', 'sim/unsorted_100a/w8y_objects.c',
    'sim/unsorted_100a/w8y_scene.c', 'sim/unsorted_100b/w8y_boxes.c', 'sim/unsorted_100b/w8y_cache.c',
    'sim/unsorted_100b/w8y_entries.c', 'sim/unsorted_100b/w8y_lang.c', 'sim/unsorted_100b/w8y_pack.c',
    # existing converted code the functions above call (and what that calls)
    'sim/core/timer.c', 'sim/core/w2a_timers.c', 'sim/render/w2n_polygon.c', 'sim/render/w2n_project.c',
    'sim/render/w3p_view.c', 'sim/render/w3q_vertex.c', 'sim/render/w3r_draw.c', 'sim/render/w7o_draw.c',
    'sim/unsorted_1000/w2a_fields.c', 'sim/unsorted_1000/w2e_mono.c', 'sim/unsorted_1000/w2e_names.c',
    'sim/unsorted_1000/w2f_records.c', 'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2j_misc.c',
    'sim/unsorted_1000/w2j_slots.c', 'sim/unsorted_1000/w2k_misc.c', 'sim/unsorted_1000/w2l_fields.c',
    'sim/unsorted_1000/w2l_objects.c', 'sim/unsorted_1000/w3o_sphere.c', 'sim/unsorted_1000/w3s_model.c',
    'sim/unsorted_1000/w4w_blocks.c', 'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1000/w5x_calls.c',
    'sim/unsorted_1000/w5y_misc.c', 'sim/unsorted_1001/w2e_names.c', 'sim/unsorted_1001/w2e_tables.c',
    'sim/unsorted_1001/w2h_objects.c', 'sim/unsorted_1001/w2h_tables.c', 'sim/unsorted_1001/w2j_misc.c',
    'sim/unsorted_1001/w3t_loader.c', 'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1001/w4u_cd.c',
    'sim/unsorted_1001/w5c_calls.c', 'sim/unsorted_1001/w5y_misc.c', 'sim/unsorted_1002/w2e_object.c',
    'sim/unsorted_1002/w2e_tables.c', 'sim/unsorted_1002/w2f_misc.c', 'sim/unsorted_1002/w2g_records.c',
    'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1002/w2j_nodes.c',
    'sim/unsorted_1002/w2l_tables.c', 'sim/unsorted_1002/w3o_ray.c', 'sim/unsorted_1002/w3o_vectors.c',
    'sim/unsorted_1002/w3q_collide.c', 'sim/unsorted_1002/w3s_cell.c', 'sim/unsorted_1002/w4u_heap.c',
    'sim/unsorted_1002/w4v_misc.c', 'sim/unsorted_1002/w5a_misc.c', 'sim/unsorted_1002/w5b_calls.c',
    'sim/unsorted_1002/w5y_misc.c', 'sim/unsorted_1002/w5z_calls.c', 'sim/unsorted_1002/w6d_calls.c',
    'sim/unsorted_1002/w7l_misc.c', 'sim/unsorted_1003/entry.c', 'sim/unsorted_1003/records.c',
    'sim/unsorted_1003/setters.c', 'sim/unsorted_1003/w2b_misc.c', 'sim/unsorted_1003/w2e_misc.c',
    'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1003/w2l_records.c',
    'sim/unsorted_1003/w3q_grid.c', 'sim/unsorted_1003/w3r_misc.c', 'sim/unsorted_1003/w3s_entry.c',
    'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1003/w4w_records.c', 'sim/unsorted_1003/w5a_misc.c',
    'sim/unsorted_1003/w5c_calls.c', 'sim/unsorted_1004/fields.c', 'sim/unsorted_1004/state.c',
    'sim/unsorted_1004/tables.c', 'sim/unsorted_1004/w2e_strings.c', 'sim/unsorted_1004/w2h_record.c',
    'sim/unsorted_1004/w2i_misc.c', 'sim/unsorted_1004/w2k_flags.c', 'sim/unsorted_1004/w3o_misc.c',
    'sim/unsorted_1004/w3p_net.c', 'sim/unsorted_1004/w3s_records.c', 'sim/unsorted_1004/w4u_heap.c',
    'sim/unsorted_1004/w4w_net.c', 'sim/unsorted_1004/w5c_calls.c', 'sim/unsorted_1004/w5y_misc.c',
    'sim/unsorted_1005/w2g_pool.c', 'sim/unsorted_1005/w2h_tables.c', 'sim/unsorted_1005/w3o_calls.c',
    'sim/unsorted_1005/w4v_misc.c', 'sim/unsorted_1006/w2e_copy.c', 'sim/unsorted_1006/w3q_quat.c',
    'sim/unsorted_1006/w5a_math.c', 'sim/unsorted_1006/w5b_calls.c', 'sim/unsorted_1006/w5c_math.c',
    'sim/unsorted_1006/w5y_misc.c', 'sim/unsorted_1006/w6e_misc.c', 'sim/unsorted_1007/w2e_state.c',
    'sim/unsorted_1007/w2f_misc.c', 'sim/unsorted_1007/w2g_stubs.c', 'sim/unsorted_1007/w3p_net.c',
    'sim/unsorted_1007/w5a_misc.c', 'sim/unsorted_1008/cell_link.c', 'sim/unsorted_1008/w2g_rect.c',
    'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_1008/w3o_grid.c', 'sim/unsorted_1008/w3o_length.c',
    'sim/unsorted_1008/w3s_grid.c', 'sim/unsorted_1009/w3o_ray.c', 'sim/unsorted_1009/w3p_box.c',
    'sim/unsorted_100a/w2e_search.c', 'sim/unsorted_100a/w2f_links.c', 'sim/unsorted_100a/w2h_misc.c',
    'sim/unsorted_100a/w2i_misc.c', 'sim/unsorted_100a/w2j_lists.c', 'sim/unsorted_100a/w2k_list.c',
    'sim/unsorted_100a/w2l_small.c', 'sim/unsorted_100a/w3o_misc.c', 'sim/unsorted_100a/w3t_misc.c',
    'sim/unsorted_100a/w4u_misc.c', 'sim/unsorted_100a/w5a_misc.c', 'sim/unsorted_100a/w5b_calls.c',
    'sim/unsorted_100a/w5c_calls.c', 'sim/unsorted_100a/w5x_calls.c', 'sim/unsorted_100a/w5y_misc.c',
    'sim/unsorted_100a/w5z_calls.c', 'sim/unsorted_100a/w6g_nodes.c', 'sim/unsorted_100a/w6g_records.c',
    'sim/unsorted_100a/w7o_misc.c', 'sim/unsorted_100a/w7o_nodes.c', 'sim/unsorted_100b/fields.c',
    'sim/unsorted_100b/w2e_strings.c', 'sim/unsorted_100b/w2f_tables.c', 'sim/unsorted_100b/w2h_records.c',
    'sim/unsorted_100b/w2j_misc.c', 'sim/unsorted_100b/w2l_records.c', 'sim/unsorted_100b/w3o_hash.c',
    'sim/unsorted_100b/w3p_misc.c', 'sim/unsorted_100b/w3s_array.c', 'sim/unsorted_100b/w4v_wrappers.c',
    'sim/unsorted_100b/w4w_buffers.c', 'sim/unsorted_100b/w5c_calls.c', 'sim/unsorted_100b/w5z_calls.c',
    'sim/unsorted_100b/w6g_misc.c', 'sim/unsorted_100b/w7p_misc.c', 'sim/unsorted_100b/w7p_ptrarray.c',
    'sim/unsorted_100c/w2e_tables.c', 'sim/unsorted_100c/w2h_misc.c', 'sim/unsorted_100c/w2i_misc.c',
    'sim/unsorted_100c/w2l_objects.c', 'sim/unsorted_100c/w3o_misc.c', 'sim/unsorted_100c/w3s_entries.c',
    'sim/unsorted_100c/w4v_timers.c', 'sim/unsorted_100c/w5a_misc.c', 'sim/unsorted_100c/w5y_misc.c',
    'sim/unsorted_100c/w7p_misc.c', 'sim/unsorted_100d/state.c', 'sim/unsorted_100d/w2f_records.c',
    'sim/unsorted_100d/w2i_select.c', 'sim/unsorted_100d/w2j_nodes.c', 'sim/unsorted_100d/w4v_misc.c',
    'sim/unsorted_100d/w5c_calls.c', 'sim/unsorted_100e/w5b_calls.c', 'sim/unsorted_100e/w6h_calls.c',
    'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_100f/w2g_crt.c', 'sim/unsorted_100f/w2h_mem.c',
    'sim/unsorted_100f/w2j_str.c', 'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_100f/w6h_crt.c',
    'sim/unsorted_100f/w6i_mem.c', 'sim/unsorted_100f/w6i_misc.c', 'sim/unsorted_100f/w6i_stream.c',
    'sim/unsorted_100f/w7q_sort.c', 'sim/unsorted_100f/w7q_stream.c', 'sim/unsorted_1010/w2f_ld12.c',
    'sim/unsorted_1010/w2f_mantissa.c', 'sim/unsorted_1010/w2g_crt.c', 'sim/unsorted_1010/w2h_misc.c',
    'sim/unsorted_1010/w2i_crt.c', 'sim/unsorted_1010/w2k_crt.c', 'sim/unsorted_1010/w6i_crt.c',
    'sim/unsorted_1010/w6j_heap.c', 'sim/unsorted_1010/w6j_stream.c', 'sim/unsorted_1010/w6k_handles.c',
    'sim/unsorted_1010/w6k_misc.c', 'sim/unsorted_1010/w6k_sbh.c', 'sim/unsorted_1010/w6k_text.c',
    'sim/unsorted_1010/w7r_heap.c', 'sim/unsorted_1010/w7s_locale.c', 'sim/util/field.c', 'sim/util/lookup.c',
    'sim/util/record.c', 'sim/util/ring.c', 'sim/view/matrix.c', 'sim/view/objlist.c', 'sim/view/viewrect.c',
    'sim/view/w2a_points.c', 'sim/view/w2d_view.c', 'sim/view/w2l_lists.c', 'sim/view/w2l_rects.c',
    'sim/view/w3r_matrix.c', 'sim/win/w4a_heap.c', 'sim/win/w4u_cdaudio.c', 'sim/win/w4v_dll.c',
    'sim/win/w4v_heap.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
STACK_TOP = 0x00800000
HEAP, HEAP_END = 0x00AC0000, 0x00AF0000     # allocation pool (argument memory, compared like all of it)
CODE = 0x00AF8000                           # function pointers: a `ret` with a logging hook (16 bytes each)
NCODE = 32

# stubbed callee -> number of argument words logged
STUBS = {
    0x100a4933: 1, 0x100a4700: 2, 0x100a539e: 1, 0x100a65e7: 2, 0x100a9ba7: 1,
    0x10105b50: 3, 0x101049c0: 1, 0x10105fe0: 3,     # _read, _close, _lseek
    0x101099a0: 3,                                  # _sopen(name, flags, share): the fourth word is not logged
    0x100feb20: 3, 0x100fe830: 4,                   # fseek, fread
    0x100bc280: 4,                                  # SystemError
    0x100fef50: 2,                                  # malloc core (size, flag)
    0x10060220: 8, 0x10060b76: 2, 0x10060d2e: 2,    # matrix builders (Sim_100aa016, inside Sim_100a9f6e)
    0x100b8d6c: 6, 0x10089e91: 1, 0x100e2aa3: 5,    # resource load, box finish, box maker (inside Sim_100e2a31)
    0x100eb070: 2,
    0x100fd820: 3, 0x100fd760: 1, 0x100fdb30: 1,    # _fsopen(name, mode, share), fclose, remove
    0x10105a60: 1,                                  # _filbuf: end of the stream
    0x100b7430: 4, 0x100b6f6a: 1, 0x10048613: 2,
    0x10104fd0: 6,                                  # string type of a character (inside Crt_IsCType)
    0x10100fd0: 3, 0x100ff860: 3, 0x100b9203: 2,    # _output (inside Crt_FPrintf), _assert, cache room
    0x100fc780: 2, 0x100b9dde: 0, 0x100b8f30: 0,    # sprintf, language letter, vendor test
    0x100bcd61: 1,                                  # pack directory (inside Sim_100bcbcf)
    0x1002bca0: 0, 0x10011093: 1, 0x10011681: 1, 0x100d4590: 1, 0x100d6833: 1, 0x100cb4fb: 1,   # unit helpers
    0x100aa742: 1, 0x10088a2d: 1, 0x10060681: 4, 0x100246b3: 12,
    0x10029b1f: 1, 0x100089c6: 1, 0x100aa64a: 1,
    0x100b43ce: 1, 0x100b48de: 2, 0x100897c7: 1, 0x10028d9f: 3,   # entries
    0x100fe970: 1,                                  # round 25: ftell
    0x100fd7e0: 2,                                  # fprintf (stream, format): the formatted text is logged
}
# C runtime replacement (DECISIONS.md): the C calls these converted functions by name, no longer through the original's
# address, so their stubs answer the C symbol too
C_ALIASES = {'Crt_SOpen': 0x101099a0, 'Crt_FPrintf': 0x100fd7e0}
TEXT_ARGS = {0x101099a0: (0,), 0x100bc280: (1, 2), 0x100fd820: (0, 1), 0x100fdb30: (0,), 0x10048613: (0, 1),
             0x100b6f6a: (0,), 0x100ff860: (0, 1), 0x100b9203: (1,), 0x100bcd61: (0,), 0x100fc780: (1,)}
ST = {'active': set()}
RUN = {}
M = [None]


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
        try:
            c = bytes(uc.mem_read(addr + len(out), 1))
        except Exception:
            return out + b'<unmapped>'
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


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def rfl(ctx, scale=1000.0):
    """A float bit pattern: ordinary values, zeros, small and big ones, both signs (no NaN, no infinity)."""
    r = ctx.rng
    c = r.random()
    if c < 0.55:
        return fbits(r.uniform(-scale, scale))
    if c < 0.75:
        return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, 2.0, 16.0, 100.0, 256.0, -3.0, 5.0, 1e-7, 1e-30]))
    if c < 0.9:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-12, 12))
    return r.getrandbits(32) & 0xbf7fffff


def sane_floats(ctx, addr, n, scale=1000.0):
    w32(ctx, addr, *[rfl(ctx, scale) for _ in range(n)])


def rname(ctx, maxlen, alpha=b'abcXYZ_09'):
    r = ctx.rng
    return bytes(r.choice(alpha) for _ in range(r.choice([0, 1, 2, 3, 5, r.randrange(0, maxlen + 1)])))


# ---- per-run state: allocation pool and file position (reset when the checker starts a run) -------------
def run_state(m):
    if RUN.get('log') is not m.api_log:
        RUN.clear()
        RUN.update(log=m.api_log, heap=HEAP, nalloc=0, pos=0, spos=0, nread=0, nseek=0)
    return RUN


def alloc(m, size, zero=False):
    rs = run_state(m)
    k = rs['nalloc']
    rs['nalloc'] += 1
    if k in ST.get('fail', ()) or size > 0x10000:
        return 0
    a = rs['heap']
    n = (size + 0x1f) & ~0xf
    if a + n > HEAP_END:
        return 0
    rs['heap'] += n
    if zero:
        m.uc.mem_write(a, bytes(size))
    return a


def a_heapalloc(n, args, uc):
    return alloc(M[0], args[2], zero=bool(args[1] & 8))


def a_heaprealloc(n, args, uc):
    """HeapReAlloc(heap, flags, block, size): a new pool block with the old contents (up to the old pool size)."""
    q = alloc(M[0], args[3])
    if q and args[2] and HEAP <= args[2] < HEAP_END:
        uc.mem_write(q, bytes(uc.mem_read(args[2], min(args[3], HEAP_END - args[2], 0x2000))))
    return q


HEAP_API = {'HeapAlloc': a_heapalloc, 'HeapReAlloc': a_heaprealloc}


# ---- stubs --------------------------------------------------------------------------------------
def e_malloc(uc, n, raw, esp):
    return [], alloc(M[0], raw[0])


def file_read(uc, buf, size, plan_key):
    rs = run_state(M[0])
    k = rs['nread']
    rs['nread'] += 1
    data = ST.get('file', b'')
    pos = rs['pos']
    want = size
    short = ST.get(plan_key, ())
    if k in short:
        want = max(0, size - 1 - (k % 3))
    chunk = data[pos:pos + want] if pos < len(data) else b''
    if chunk:
        uc.mem_write(buf, chunk)
    rs['pos'] = pos + len(chunk)
    return chunk, len(chunk)


def e_read(uc, n, raw, esp):
    chunk, got = file_read(uc, raw[1], min(raw[2], 0x10000), 'read_short')
    return [], got


def e_lseek(uc, n, raw, esp):
    rs = run_state(M[0])
    k = rs['nseek']
    rs['nseek'] += 1
    if k in ST.get('seek_fail', ()):
        return [], 0xffffffff
    off = raw[1] - (raw[1] >> 31 << 32)
    if raw[2] == 0:
        rs['pos'] = raw[1]
    elif raw[2] == 1:
        rs['pos'] = (rs['pos'] + off) & 0xffffffff
    elif raw[2] == 2:
        rs['pos'] = (len(ST.get('file', b'')) + off) & 0xffffffff
    return [], rs['pos']


def e_fread(uc, n, raw, esp):
    size = min((raw[1] * raw[2]) & 0xffffffff, 0x10000)
    chunk, got = file_read(uc, raw[0], size, 'read_short')
    return [], got // raw[1] if raw[1] else 0


def e_fseek(uc, n, raw, esp):
    rs = run_state(M[0])
    k = rs['nseek']
    rs['nseek'] += 1
    if k in ST.get('seek_fail', ()):
        return [], 0xffffffff
    if raw[2] == 0:
        rs['pos'] = raw[1]
    return [], 0


EFFECTS = {0x100fef50: e_malloc, 0x10105b50: e_read, 0x10105fe0: e_lseek, 0x100fe830: e_fread,
           0x100feb20: e_fseek}


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
        name = 'code_%d' % k
        n = sum(1 for e in m.api_log if e[0] == name)
        m.api_log.append((name, tuple('stack' if STACK_LO <= a <= STACK_HI else a for a in raw)))
        eff = ST['code_eff'].get(k)
        if eff:
            val = eff(m.uc, n, raw)
        else:
            rets = ST['code_rets'].get(k)
            val = rets[n % len(rets)] if rets else mix(ST['seed'], n, k)
        uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
        uc.reg_write(UC_X86_REG_EDX, mix(ST['seed'], n, k, 7))
    return cb


def begin(ctx, *stubs):
    """Installs the hooks (once per machine), activates the given stubs, chooses their answers and sets the
    x87 state of the game (control word 0x27f). Must be the first thing a generator does."""
    m = ctx.m
    r = ctx.rng
    M[0] = m
    if not getattr(m, '_w8y', False):
        m._w8y = True
        cb = _stub_cb(m)
        for addr in STUBS:
            m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
        for cname, addr in C_ALIASES.items():
            if cname in m.syms:
                m.raw.hook_add(UC_HOOK_CODE, (lambda a: lambda uc, address, size, ud: cb(uc, a, size, ud))(addr),
                               None, m.syms[cname], m.syms[cname])
        m.raw.hook_add(UC_HOOK_CODE, _code_cb(m), None, CODE, CODE + 16 * NCODE - 1)
    ST['active'] = set(stubs)
    ST['seed'] = r.getrandbits(32)
    ST['effects'] = dict(EFFECTS)
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}
    ST['code_nargs'] = {}
    ST['code_rets'] = {}
    ST['code_eff'] = {}
    ST['fail'] = ()
    ST['file'] = b''
    ST['read_short'] = ()
    ST['seek_fail'] = ()
    RUN.clear()
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def code(ctx, k, nargs=1, rets=None, eff=None):
    """Address of a function pointer target (`ret` with a logging hook) that takes nargs dword arguments."""
    ST['code_nargs'][k] = nargs
    if rets is not None:
        ST['code_rets'][k] = list(rets)
    if eff is not None:
        ST['code_eff'][k] = eff
    a = CODE + 16 * k
    ctx.m.uc.mem_write(a, b'\xc3')
    return a


def heap_world(ctx):
    """The small-block heap list (0x10159e38) covers nothing, so freed blocks go to HeapFree; the new-handler flag
    0x1017e968 is clear and the small-block limit 0x1015be5c is 0 (allocations go to HeapAlloc)."""
    w32(ctx, 0x10159e38, 0x10159e38)
    w32(ctx, 0x10159e48, 0, 0)
    w32(ctx, 0x1017e968, 0)
    w32(ctx, 0x1015be5c, 0)


def fails(ctx, p=0.08, n=12):
    ST['fail'] = {k for k in range(n) if ctx.rng.random() < p}


# ---- 0x100a4f03 .. 0x100a526b: the deferred scene list ----------------------------------------------------
def camera(ctx, scale=3000.0):
    cam = ctx.buf(0x140, b''.join(struct.pack('<I', rfl(ctx, scale)) for _ in range(0x50)))
    w32(ctx, 0x1014cae0, cam)
    return cam


def model_block(ctx, nverts, npolys):
    r = ctx.rng
    poff = 0x18 + 0x58 * nverts
    blk = newbuf(ctx, poff + 0x28 * npolys + 0x40)
    w16(ctx, blk + 4, nverts, npolys)
    w32(ctx, blk + 8, poff)
    for i in range(nverts):
        sane_floats(ctx, blk + 0x18 + i * 0x58, 0x16, 500)
    for i in range(npolys):
        sane_floats(ctx, blk + poff + i * 0x28, 0xa, 2)
    w32(ctx, blk + 0xc, 0)
    return blk


def scene_obj(ctx):
    """flags word +0 (0x1000 = skip), kind word +2, LOD chain +0x2c (mostly none), node +0x28, stamps +0x9c."""
    r = ctx.rng
    obj = newbuf(ctx, 0xc0)
    w16(ctx, obj, r.choice([0, 0, 0x1000, 0x10, 0x1010, r.getrandbits(16)]))
    w16(ctx, obj + 2, r.choice([0x70, 0x20, 0x25, 0x10, 0xa0, r.getrandbits(16)]))
    w32(ctx, obj + 0x2c, 0)
    if r.random() < 0.25:
        node = zbuf(ctx, 0xa0)
        sane_floats(ctx, node + 0x4c, 12, 2)
        w32(ctx, obj + 0x28, r.choice([node, node, 0]))
        chain = []
        for k in range(r.choice([1, 1, 2, 3])):
            blk = model_block(ctx, r.choice([0, 1, 2]), r.choice([0, 1, 2]))
            wf(ctx, blk, r.choice([0.0, 50.0 * (k + 1), r.uniform(0, 500)]))
            w32(ctx, blk + 0x10, 7)
            if chain:
                w32(ctx, chain[-1] + 0xc, blk)
            chain.append(blk)
        w32(ctx, obj + 0x2c, chain[0])
        w32(ctx, obj + 0x9c, r.choice([7, 7, 7, 8]))
    return obj


def node_tree(ctx, depth):
    r = ctx.rng
    node = zbuf(ctx, 0xa0)
    w32(ctx, node + 0x8c, scene_obj(ctx) if r.random() < 0.75 else 0)
    kids = [node_tree(ctx, depth - 1) for _ in range(r.choice([0, 1, 1, 2, 3]))] if depth > 0 else []
    if kids:
        w32(ctx, node + 4, kids[0])
        for a, b in zip(kids, kids[1:]):
            w32(ctx, a + 8, b)
    return node


def e_driver_test(uc, n, raw):
    """The display driver's object test: the answer, and now and then the count of deferred entries grows."""
    h = mix(ST['seed'], n, 3)
    if h % 3 == 0:
        cnt = ur32(uc, 0x1017d138)
        uc.mem_write(0x1017d138, struct.pack('<I', (cnt + 1 + (h >> 8) % 2) & 0xffffffff))
    return ST['test'][n % len(ST['test'])]


def e_4933(uc, n, raw, esp):
    h = mix(ST['seed'], n, 5)
    cnt = ur32(uc, 0x1017d138)
    uc.mem_write(0x1017d138, struct.pack('<I', (cnt + h % 3) & 0xffffffff))
    return [], ST['stop'][n % len(ST['stop'])]


def scene_world(ctx):
    r = ctx.rng
    cam = camera(ctx)
    wf(ctx, cam + 0xb8, r.choice([1.0, 0.5, 2.0]))
    w32(ctx, 0x101668d0, 0)
    w32(ctx, 0x109afdcc, r.choice([0, 1]))
    for a in (0x109b16c0, 0x109b16c4, 0x109b16c8):
        w32(ctx, a, rfl(ctx, 1000))
    lst = newbuf(ctx, 8 * 40)
    w32(ctx, 0x1017d168, lst)
    w32(ctx, 0x1017d170, ctx.u32())
    w32(ctx, 0x1014d1a4, ctx.u32())
    w32(ctx, 0x101c9ea4, r.choice([1, 1, 1, 1, 0]))
    w32(ctx, 0x1017d138, r.choice([0, 3, 0xffffffff]))
    lim = r.choice([5, 40, 0, 2, 0x7fffffff])
    w32(ctx, 0x1017d140, lim)
    w32(ctx, 0x1017d13c, r.choice([0, 0, 1, lim, lim - 1, -5, 0x80000000]))
    w32(ctx, 0x101c9ea0, r.choice([0, 1, 5, 10]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    for a in (0x101cc2b4, 0x101c7a9c, 0x1017d134, 0x1017d130, 0x101cc2ac):
        w32(ctx, a, ctx.u32())
    w32(ctx, 0x101c7aa0, fbits(r.choice([0.0, 10.0, 100.0, 300.0, 1000.0, r.uniform(0, 2000)])))
    ST['test'] = [r.choice([0, 0, 1, 5]) for _ in range(4)]
    w32(ctx, 0x1014cb40, code(ctx, 0, 1, eff=e_driver_test))
    rets(0x100a65e7, 0)


SCENE_STUBS = (0x100a4700, 0x100a539e, 0x100a65e7, 0x100a9ba7)


def g_tree(ctx):
    r = begin(ctx, *SCENE_STUBS)
    scene_world(ctx)
    return [r.choice([node_tree(ctx, r.choice([0, 1, 2, 3]))] * 12 + [0])]


def g_a4f03(ctx):
    r = begin(ctx, *SCENE_STUBS, 0x100a4933)
    ST['effects'][0x100a4933] = e_4933
    scene_world(ctx)
    lst = newbuf(ctx, 0x40)
    prev = lst
    for _ in range(r.choice([0, 1, 2, 4, 6])):
        o = newbuf(ctx, 0x40)
        w32(ctx, prev + 0x20, o)
        prev = o
    w32(ctx, prev + 0x20, 0)
    w32(ctx, lst + 4, r.choice([0, 5, 6]), r.choice([0, 5, 6, 7]))
    ST['stop'] = [r.choice([0, 0, 0, 1]) for _ in range(5)]
    return [r.choice([lst] * 12 + [0])]


# ---- 0x100a648c: direction value -----------------------------------------------------------------------
def g_a648c(ctx):
    r = begin(ctx)
    w32(ctx, 0x109afdcc, r.choice([0, 0, 1]))
    vec = [r.uniform(-2000, 2000) for _ in range(3)]
    wf(ctx, 0x109b16c0, *vec)
    near = r.random()
    pts = []
    for i, k in enumerate((1, 2, 0)):          # px from +4, py from +8, pz from +0
        v = vec[k]
        if near < 0.3:
            pts.append(fbits(v + r.choice([0.0, 0.0, 1e-8 * max(1.0, abs(v)), -5e-8, 1e-7, 2e-7])))
        else:
            pts.append(rfl(ctx, 3000))
    dirs = [rfl(ctx, 2) for _ in range(3)]
    if r.random() < 0.2:
        dirs = [fbits(r.uniform(-1e6, 1e6)) for _ in range(3)]
    return pts + dirs


# ---- 0x100a76a0 .. 0x100a8405: the archive slots ------------------------------------------------------------
def archive_image(ctx, n_entries, items_per):
    """A file image: 'PROJ' header (byte +8 0xfe now and then), the directory size at offset 0x10, the directory
    (count word +0xc, entries of 0x18 bytes from +0xe: name +0, offset +4, size +8, word +0x14) from offset 0xc,
    and per entry an item table (from its offset: 0x16 bytes, then items of 8 bytes: start, end)."""
    r = ctx.rng
    names = [bytes(r.choice(b'ABCD') for _ in range(4)) for _ in range(n_entries)]
    dsize = 0xe + 0x18 * n_entries
    body = bytearray(rb(ctx, 0x40))
    body[0:4] = r.choice([b'PROJ'] * 8 + [b'PROX', b'proj'])
    body[8] = r.choice([0, 0, 0xfe, 0x12])
    struct.pack_into('<I', body, 0x10, r.choice([dsize - 8, dsize - 8, dsize - 8, 0xffffffff, 0x2000]))
    dir_off = 0xc
    while len(body) < dir_off + dsize + 8:
        body.append(0)
    struct.pack_into('<H', body, dir_off + 0xc, n_entries)
    off = (dir_off + dsize + 0x40 + 15) & ~15
    tables = []
    for i, nm in enumerate(names):
        tsize = 0x16 + 8 * items_per + 8
        e = dir_off + 0xe + 0x18 * i
        body[e:e + 4] = nm
        struct.pack_into('<III', body, e + 4, r.choice([off] * 15 + [0]), tsize, 0)
        struct.pack_into('<H', body, e + 0x14, r.randrange(0, 0x40))
        tables.append((off, tsize))
        off += tsize + 0x20
    data = bytearray(body) + bytes(max(0, off - len(body)))
    for (o, tsize) in tables:
        t = bytearray(rb(ctx, tsize))
        for k in range(items_per + 1):
            s = r.randrange(0x40, 0x200)
            struct.pack_into('<II', t, 0x16 + 8 * k, s, s + r.choice([0, 4, 16, 0x40, 0x100]))
        data[o:o + tsize] = t
    data += rb(ctx, 0x400)
    return bytes(data), names


def e_arch_alloc(uc, n, raw):
    return alloc(M[0], raw[0])


def archive_world(ctx, slot_used=None):
    """Archive slot 0 (0x101c65e0, 0x14a bytes): handle, used word, directory block from the image, item tables."""
    r = ctx.rng
    data, names = archive_image(ctx, r.choice([1, 2, 3, 5]), r.choice([1, 2, 4]))
    ST['file'] = data
    w32(ctx, 0x1017d15c, code(ctx, 1, 1, eff=e_arch_alloc))
    w32(ctx, 0x1017d160, code(ctx, 2, 1, rets=[0]))
    slot = 0x101c65e0
    wb(ctx, slot, rb(ctx, 0x14a))
    w32(ctx, slot, r.choice([3, 4, 0x55]))
    w16(ctx, slot + 4, r.choice([0, 0, 1]) if slot_used is None else slot_used)
    dsize = struct.unpack_from('<I', data, 0x10)[0] + 8
    dsize = min(dsize, 0x400)
    dir_ = ctx.buf(dsize + 0x40, data[0xc:0xc + dsize] + rb(ctx, 0x40))
    w32(ctx, slot + 6, dir_)
    n = struct.unpack_from('<H', data, 0xc + 0xc)[0]
    for i in range(n):
        e = 0xc + 0xe + 0x18 * i
        o, sz = struct.unpack_from('<II', data, e + 4)
        loaded = 0
        if r.random() < 0.6:
            loaded = ctx.buf(sz + 0x20, data[o:o + sz] + rb(ctx, 0x20))
        w32(ctx, slot + 0x2a + 8 * i, loaded)
    return names


ARCH_STUBS = (0x10105b50, 0x101049c0, 0x10105fe0, 0x101099a0, 0x100feb20, 0x100fe830, 0x100bc280, 0x100fef50)


def key_for(ctx, names, hit=0.85):
    r = ctx.rng
    k = r.choice(names) if names and r.random() < hit else bytes(r.choice(b'ABCD') for _ in range(4))
    return cstr(ctx, k + r.choice([b'', b'', b'X', b'.dat']))


def g_a79db(ctx):
    r = begin(ctx, *ARCH_STUBS)
    archive_world(ctx)
    rets(0x101099a0, r.choice([3, 3, 3, 0xffffffff]))
    ST['read_short'] = {k for k in range(4) if r.random() < 0.1}
    ST['seek_fail'] = {k for k in range(4) if r.random() < 0.05}
    mode = r.choice([0, 0, 2, 2, 1, 0x102, 0x200, 0xff])
    return [cstr(ctx, rname(ctx, 40, b'abc.\\/_') + b'x' * r.choice([0, 0, 40])), mode]


def g_arch_item(ctx):
    r = begin(ctx, *ARCH_STUBS)
    names = archive_world(ctx, 1)
    ST['read_short'] = {k for k in range(6) if r.random() < 0.1}
    ST['seek_fail'] = {k for k in range(6) if r.random() < 0.1}
    sub = r.choice([0, 1, 2, 3]) | r.choice([0, 0, 0x10000, 0xabcd0000])
    out_a = r.choice([zbuf(ctx, 8), zbuf(ctx, 8), 0])
    out_b = r.choice([zbuf(ctx, 8), zbuf(ctx, 8), 0])
    return [0, key_for(ctx, names, 0.97), sub, out_a, out_b]


def g_a8238(ctx):
    r = begin(ctx, *ARCH_STUBS)
    names = archive_world(ctx, 1)
    ST['read_short'] = {k for k in range(6) if r.random() < 0.1}
    ST['seek_fail'] = {k for k in range(6) if r.random() < 0.1}
    sub = r.choice([0, 1, 2, 3]) | r.choice([0, 0, 0x10000, 0xabcd0000])
    return [0, key_for(ctx, names), sub, zbuf(ctx, 0x200)]


def g_a836e(ctx):
    r = begin(ctx, *ARCH_STUBS)
    names = archive_world(ctx, 1)
    ST['read_short'] = {k for k in range(6) if r.random() < 0.1}
    ST['seek_fail'] = {k for k in range(6) if r.random() < 0.1}
    rec = newbuf(ctx, 0x100)
    for i in range(8):
        w32(ctx, rec + 4 + 12 * i + 4, r.randrange(0, 0x300), r.choice([0, 1, 16, 0x40, 0x100, 0xffffffff]))
    w32(ctx, 0x1017d158, rec)
    sub = r.choice([0, 1, 2, 3, 0xf000, 0xf001, 0xf003, 0xf007, 0xffff]) | r.choice([0, 0, 0x10000])
    return [0, key_for(ctx, names), sub, zbuf(ctx, 0x200)]


def g_a8405(ctx):
    r = begin(ctx, *ARCH_STUBS)
    names = archive_world(ctx, 1)
    ST['read_short'] = {k for k in range(6) if r.random() < 0.1}
    sub = r.choice([0, 1, 2, 3]) | r.choice([0, 0, 0x10000, 0xabcd0000])
    return [0, key_for(ctx, names), sub, zbuf(ctx, 8)]


def g_a76a0(ctx):
    r = begin(ctx, *ARCH_STUBS)
    heap_world(ctx)
    archive_world(ctx)
    rets(0x101099a0, r.choice([3, 3, 3, 0xffffffff]))
    ST['read_short'] = {k for k in range(8) if r.random() < 0.05}
    fails(ctx, 0.05)
    w32(ctx, 0x1017d154, r.choice([0, 0, cstr(ctx, b'heavyg.prj')]))
    wb(ctx, 0x1015f050, r.choice([b'\0', b'C:\\HG\0']))
    w32(ctx, 0x1017b8f8, 0x00AB0000)
    return []


# ---- 0x100aa016: three matrices applied to a node --------------------------------------------------------
def e_matrix(uc, n, raw, esp):
    """0x10060220 writes 12 floats (from what it is given) through its first argument."""
    h = [mix(ST['seed'], n, k, *raw[1:]) for k in range(12)]
    vals = [((x % 2001) - 1000) / 997.0 for x in h]
    uc.mem_write(raw[0], struct.pack('<12f', *vals))
    return [], None


def g_aa016(ctx):
    r = begin(ctx, 0x10060220, 0x10060b76, 0x10060d2e)
    ST['effects'][0x10060220] = e_matrix
    node = newbuf(ctx, 0xa0)
    sane_floats(ctx, node + 0xc, 0x20, 2.0)
    w32(ctx, node + 0x94, r.getrandbits(32) & ~0x4000 if r.random() < 0.5 else r.getrandbits(32))
    w32(ctx, node + 0x98, r.choice([1, 2, 3, 0, r.getrandbits(32)]))
    w32(ctx, 0x1014d1a8, ctx.u32())
    xyz = [rfl(ctx, 400) for _ in range(3)]
    if r.random() < 0.1:
        # quiet NaNs and infinities only: the emulator does not quiet a signalling NaN on fld (real x87s do,
        # and so does the C, see neg_bits)
        xyz[r.randrange(3)] = r.choice([0x7fc00000, 0xffc12345, 0x7f800000, 0xff800000, 0x80000000])
    return [node, rfl(ctx, 400), rfl(ctx, 400), rfl(ctx, 400), r.choice([0, 1, 2, ctx.u32()])] + xyz


# ---- 0x100b5502 .. 0x100b58c5: the debug unit boxes -------------------------------------------------------
def e_resource(uc, n, raw, esp):
    """0x100b8d6c(key, ext, ext4, 2, &size, 0): logs the key name, sets the key word and the size."""
    name = ustr(uc, raw[0] + 2)
    uc.mem_write(raw[0], struct.pack('<h', ST['key'][n % len(ST['key'])]))
    uc.mem_write(raw[4], struct.pack('<I', ST['size']))
    return [name], ST['data'][n % len(ST['data'])]


def e_boxmaker(uc, n, raw, esp):
    """0x100e2aa3(data, &count, &result, d, &unused): a box (or none) through the third argument."""
    cnt = ur32(uc, raw[1])
    uc.mem_write(raw[1], struct.pack('<I', (cnt + 1) & 0xffffffff))
    uc.mem_write(raw[2], struct.pack('<I', ST['boxes'][n % len(ST['boxes'])]))
    return [cnt], None


def box_obj(ctx):
    r = ctx.rng
    o = newbuf(ctx, 0xc0)
    w16(ctx, o, r.choice([0x800, 0x800, 0x0, 0x8800, r.getrandbits(16)]))
    for off in (4, 8, 0xc, 0x10, 0x14, 0x18, 0x1c, 0x20):
        w32(ctx, o + off, r.choice([0, 0, 0, newbuf(ctx, 0x40)]))
    sane_floats(ctx, o + 0x38, 0x14, 500)
    w32(ctx, o + 0x2c, r.choice([0, 0, model_block(ctx, r.choice([0, 1, 2]), r.choice([0, 1]))]))
    w32(ctx, o + 0x28, r.choice([0, 0, zbuf(ctx, 0xa0)]))
    w32(ctx, o + 0x34, r.choice([0, 4]))
    return o


def box_world(ctx):
    r = ctx.rng
    heap_world(ctx)
    w32(ctx, 0x1017c8cc, 0)
    w32(ctx, 0x1017c8c4, 0)
    w32(ctx, 0x1017e678, 0)
    w32(ctx, 0x1015dc30, zbuf(ctx, 0x3f1 * 4))
    w32(ctx, 0x109f13f4, 0)
    w32(ctx, 0x109f13f0, 0)
    w32(ctx, 0x1017b8f8, 0x00AB0000)
    ST['boxes'] = [r.choice([0, box_obj(ctx), box_obj(ctx), box_obj(ctx)]) for _ in range(4)]
    ST['key'] = [r.choice([-1, -1, 3, 0x7fff, -2])]
    ST['size'] = r.choice([0, 1, 5, 100, 0xffffffff])
    data = newbuf(ctx, 0x40)
    ST['data'] = [r.choice([data, data, data, 0])]
    rets(0x100e2aa3, r.choice([0, 0, 0, 1]))
    ST['effects'][0x100b8d6c] = e_resource
    ST['effects'][0x100e2aa3] = e_boxmaker
    ST['effects'][0x10060220] = e_matrix
    for a in (0x1015264c, 0x10152650, 0x10152654, 0x1017e850):
        w32(ctx, a, ctx.u32())


BOX_STUBS = (0x100b8d6c, 0x10089e91, 0x100e2aa3, 0x10060220)


def g_b5502(ctx):
    r = begin(ctx, *BOX_STUBS)
    box_world(ctx)
    w32(ctx, 0x101a1164, r.choice([0, 0, 1]))
    n = r.choice([0, 1, 2, 3, 5])
    w32(ctx, 0x1017d220, r.choice([n, n, n, -1]))
    for i in range(6):
        e = 0x101a1180 + 0x7c * i
        sane_floats(ctx, e, 0x1f, 300)
        if r.random() < 0.2:
            w32(ctx, e + 0xc, r32(ctx, e))
        if r.random() < 0.1:
            w32(ctx, e, 0x7fc00000)
        w32(ctx, 0x101a10e0 + 4 * i, r.choice([0, box_obj(ctx)]))
    return []


def box_tree(ctx, depth):
    r = ctx.rng
    nd = newbuf(ctx, 0x30)
    sane_floats(ctx, nd, 6, 300)
    if r.random() < 0.15:
        w32(ctx, nd + 4, r32(ctx, nd))
    for k in range(4):
        w32(ctx, nd + 0x1c + 4 * k, box_tree(ctx, depth - 1) if depth > 0 and r.random() < 0.4 else 0)
    return nd


def g_b581d(ctx):
    r = begin(ctx, *BOX_STUBS)
    box_world(ctx)
    return [r.choice([box_tree(ctx, r.choice([0, 1, 2]))] * 8 + [0])]


def g_b58c5(ctx):
    r = begin(ctx, *BOX_STUBS)
    box_world(ctx)
    return [r.choice([box_tree(ctx, r.choice([0, 1, 2, 3]))] * 10 + [0]), newbuf(ctx, 0x40),
            r.choice([0, 1, 5, 100, 0xffffffff, 0x80000000])]


# ---- 0x100b69d0 .. 0x100b6e5e: the language file ------------------------------------------------------------
LANG_WORDS = [b'hello', b'Elbonian', b'STARS', b'linenum', b'XtdElbon', b'abc def', b'Z', b'\xc9t\xe9']


def lang_lines(ctx):
    r = ctx.rng
    words = LANG_WORDS
    out = []
    for _ in range(r.choice([0, 1, 2, 3, 5, 8])):
        kind = r.random()
        if kind < 0.6:
            out.append(b':ENGLISH:\n')
            out += [r.choice(words) + b'\n' for _ in range(r.choice([0, 1, 1, 2]))]
            out.append(r.choice([b'\n', b' \t\n', b'\r\n', b'']))
            out.append(b':TRANSLATED:\n')
            out += [r.choice(words) + b'\n' for _ in range(r.choice([0, 1, 1, 2]))]
            out.append(r.choice([b'\n', b' \n']))
        elif kind < 0.75:
            out.append(b':OPTIONS:\n')
            out += [b' '.join(r.choice(words) for _ in range(r.randrange(1, 4))) + b'\n'
                    for _ in range(r.choice([0, 1, 2]))]
            out.append(b'\n')
        elif kind < 0.85:
            out.append(r.choice([b':ENG', b':TRANSLATEDx', b':english:', b'#x', b'']) + b'\n')
        elif kind < 0.92:
            out.append(b'x' * r.choice([254, 255, 256, 300]) + b'\n')
        else:
            out.append(b':ENGLISH:\n' + b'q' * r.choice([300, 600]) + b'\n\n')
    return b''.join(out)


def stream(ctx, text):
    """A FILE with the whole text in its buffer: +0 next byte, +4 bytes left; _filbuf (stubbed) ends it."""
    buf = ctx.buf(len(text) + 16, text + b'\0' * 16)
    f = newbuf(ctx, 0x20)
    w32(ctx, f, buf, len(text))
    return f


LANG_STUBS = (0x100fef50, 0x100fd820, 0x100fd760, 0x100fdb30, 0x10105a60, 0x100b7430, 0x100b6f6a, 0x10048613,
              0x10104fd0)


def e_strtype(uc, n, raw, esp):
    """0x10104fd0(1, bytes, n, &type, 0, 0): the type word of the character (as GetStringTypeA would write it)."""
    ch = bytes(uc.mem_read(raw[1], max(1, min(raw[2], 2))))
    uc.mem_write(raw[3], struct.pack('<H', ST['ctype'][ch[0] % len(ST['ctype'])]))
    return [ch], 1


def e_finish(uc, n, raw, esp):
    return [], raw[0] if ST['finish'][n % len(ST['finish'])] else 0


def lang_world(ctx):
    r = ctx.rng
    heap_world(ctx)
    fails(ctx, 0.03, 40)
    rets(0x10105a60, 0xffffffff)
    w32(ctx, 0x1015965c, r.choice([1, 1, 1, 2]))
    ST['finish'] = [r.choice([1, 1, 1, 0]) for _ in range(3)]
    ST['effects'][0x100b6f6a] = e_finish
    ST['ctype'] = [r.choice([0, 1, 2, 0x101]) for _ in range(5)]
    ST['effects'][0x10104fd0] = e_strtype


def g_b6e5e(ctx):
    r = begin(ctx, *LANG_STUBS)
    lang_world(ctx)
    return [stream(ctx, lang_lines(ctx)), ctx.buf(4, struct.pack('<i', r.randrange(-5, 50)))]


def g_b6ba0(ctx):
    r = begin(ctx, *LANG_STUBS)
    lang_world(ctx)
    eng = r.choice([0, 0, cstr(ctx, b'old')])
    return [stream(ctx, lang_lines(ctx)), ctx.buf(4, struct.pack('<I', eng)),
            ctx.buf(4, struct.pack('<I', r.choice([0, 7]))), ctx.buf(4, struct.pack('<I', r.choice([0, 0, 1, 6]))),
            ctx.buf(4, struct.pack('<i', r.randrange(-5, 50)))]


def g_b69d0(ctx):
    r = begin(ctx, *LANG_STUBS)
    lang_world(ctx)
    text = lang_lines(ctx)
    if r.random() < 0.12:
        # many blocks now and then, so that the table grows past 100
        text = b''.join(b':ENGLISH:\nw%d\n\n:TRANSLATED:\nt%d\n\n' % (k, k) for k in range(r.choice([100, 101, 150, 201])))
    # round 27: the original reads its two unset text pointers [ebp-4] / [ebp-0x20] when :TRANSLATED: comes before
    # any :ENGLISH:; the C starts them at 0, so the original's words are seeded with 0 and that case is kept
    if r.random() < 0.15:
        text = b':TRANSLATED:\n' + r.choice(LANG_WORDS) + b'\n\n' + text
    ebp = STACK_TOP - 0x100 - 4 - 4 - 4
    w32(ctx, ebp - 4, 0)
    w32(ctx, ebp - 0x20, 0)
    rets(0x100fd820, r.choice([stream(ctx, text)] * 8 + [0]))
    rets(0x10048613, r.choice([0, 1]))
    name = r.choice([0, cstr(ctx, b'lang.txt'), cstr(ctx, b'Lang.ini'), cstr(ctx, b'x.txtz')])
    return [name]


def g_b8938(ctx):
    r = begin(ctx, 0x100eb070)
    # [0x1017d5cc], the original loading animation's timer, is never set now (its timeKillEvent branch is deleted)
    w32(ctx, 0x1017d5cc, 0)
    for a in (0x1017d5d0, 0x1017d5d4):
        w32(ctx, a, r.choice([0, ctx.u32()]))
    for a in (0x10199448, 0x1017b8f8, 0x109e3e20, 0x109e3e24, 0x10199440, 0x10199444, 0x1017d5d8, 0x1017d5b8):
        w32(ctx, a, ctx.u32())
    w32(ctx, 0x1017b920, r.choice([0, 1, 1]))
    t = newbuf(ctx, 0x40)
    w32(ctx, t + 0x20, code(ctx, 3, 0, rets=[r.choice([0, 0, 1, 0xffffffff])]))
    w32(ctx, 0x1015e688, t)
    t2 = newbuf(ctx, 0x40)
    w32(ctx, t2 + 0x1c, code(ctx, 4, 4, rets=[0]))
    w32(ctx, 0x1015e68c, t2)
    return []


# ---- 0x100b8ff9 .. 0x100baff4: the resource cache --------------------------------------------------------------
def _fmt(uc, fmt, words):
    """A small printf for the formats of these functions (%s %c %d %x %ld with widths)."""
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
        spec_ = fmt[i + 1:j].replace(b'l', b'')
        conv = fmt[j:j + 1]
        if conv == b'%':
            out += b'%'
            i = j + 1
            continue
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
        width = int(spec_) if spec_.lstrip(b'-').isdigit() else 0
        out += s.rjust(width) if width > 0 else s
        i = j + 1
    return out


def e_output(uc, n, raw, esp):
    """0x10100fd0(stream, format, arguments): the formatted text is logged."""
    words = list(struct.unpack('<8I', bytes(uc.mem_read(raw[2], 32))))
    text = _fmt(uc, ustr(uc, raw[1]), words)
    return [text], len(text)


def e_fprintf(uc, n, raw, esp):
    """fprintf(stream, format, ...): the formatted text is logged (the arguments follow the format)."""
    words = list(struct.unpack('<8I', bytes(uc.mem_read(esp + 12, 32))))
    text = _fmt(uc, ustr(uc, raw[1]), words)
    return [text], len(text)


def e_sprintf(uc, n, raw, esp):
    words = list(struct.unpack('<8I', bytes(uc.mem_read(esp + 12, 32))))
    text = _fmt(uc, ustr(uc, raw[1]), words)
    uc.mem_write(raw[0], text + b'\0')
    return [text], len(text)


TYPE_NAMES = [b'3DI', b'MLT', b'TEXT', b'WAV', b'WTB', b'XEL', b'GDF', b'TLT', b'BWD', b'SDF']


def cache_item(ctx, key):
    r = ctx.rng
    e = newbuf(ctx, 0x30)
    wb(ctx, e, key)
    w32(ctx, e + 8, r.choice([0, 0, 1, 2, -1]))
    w32(ctx, e + 0xc, r.choice([0, 16, 100, 0x1000]))
    w32(ctx, e + 0x18, r.choice([0, newbuf(ctx, 0x20)]))
    return e


def cache_type(ctx, name, hsize=None):
    """A cache type with its hash chains (consistent: each item in the chain of its own key)."""
    r = ctx.rng
    t = newbuf(ctx, 0x40)
    hsize = hsize or r.choice([1, 2, 4, 7, 16])
    w32(ctx, t, hsize, r.choice([0x100000, 0x200, 0]), r.randrange(0, 20), r.randrange(0, 0x10000))
    table = zbuf(ctx, 4 * hsize)
    w32(ctx, t + 0x10, table)
    w32(ctx, t + 0x14, ptr_array(ctx, []))
    wb(ctx, t + 0x18, name.ljust(8, b'\0')[:8])
    w32(ctx, t + 0x20, r.choice([0, 0, code(ctx, 5, 1, rets=[0])]))
    chains = {}
    for _ in range(r.choice([0, 1, 3, 6, 12])):
        key = bytes(r.choice(b'abcdXYZ\0') for _ in range(8))
        k0, k1 = struct.unpack('<II', key)
        x = k0 ^ k1
        x = x - (1 << 32) if x & 0x80000000 else x
        b = abs(x) % hsize * (1 if x >= 0 else -1)     # C remainder (truncating)
        if b < 0:
            continue                    # a negative remainder would index before the table: not generated
        chains.setdefault(b, []).append(cache_item(ctx, key))
    for b, items in chains.items():
        w32(ctx, table + 4 * b, items[0])
        for x, y in zip(items, items[1:]):
            w32(ctx, x + 0x1c, y)
            w32(ctx, y + 0x20, x)
        w32(ctx, items[0] + 0x20, 0)
        w32(ctx, items[-1] + 0x1c, 0)
    return t


def ptr_array(ctx, items, cap=None):
    a = newbuf(ctx, 0x10)
    n = len(items)
    w32(ctx, a, cap if cap is not None else n + ctx.rng.randrange(0, 4), n)
    w32(ctx, a + 8, ctx.buf(4 * n + 16, struct.pack('<%dI' % n, *items) + rb(ctx, 16)))
    return a


def cache_world(ctx):
    r = ctx.rng
    heap_world(ctx)
    names = sorted(set(r.choice(TYPE_NAMES) for _ in range(r.choice([0, 1, 3, 5]))), key=lambda s: s.ljust(4, b'\0')[:4])
    types = [cache_type(ctx, nm) for nm in names]
    ST['types'] = dict(zip(names, types))
    w32(ctx, 0x1017d604, ptr_array(ctx, types))
    for a in (0x1017b904, 0x1017b908, 0x1017b90c, 0x1017b8fc, 0x1017b910, 0x1017b914):
        w32(ctx, a, r.choice([0, ctx.u32(), ctx.u32()]))
    w32(ctx, 0x1017b8f8, 0x00AB0000)
    w32(ctx, 0x1017b900, 0x00AB1000)
    w32(ctx, 0x1017d60c, code(ctx, 6, 0))
    return names


def fd_table(ctx):
    """The C library's handle table: 0x20 handles, handle 3 open (flag 1), not a device."""
    blk = zbuf(ctx, 0x100)
    for h in range(0x20):
        w8(ctx, blk + 8 * h + 4, 1)
    w32(ctx, 0x10ac385c, 0x20)
    w32(ctx, 0x10ac3720, blk)


def out_stream(ctx):
    f = zbuf(ctx, 0x20)
    w32(ctx, f + 0xc, 2)
    w32(ctx, f + 0x10, 3)
    return f


def e_fclose(uc, n, raw, esp):
    """fclose(stream). strtok's position 0x1017e884 is left pointing into the caller's line buffer on the stack,
    whose address depends on the frame layout (the original's or the C compiler's): like the stack addresses in
    the call log, such a pointer is replaced by a marker here (at the end of the reading), and the text it points
    at is logged instead, so the position in the line is still compared."""
    p = ur32(uc, 0x1017e884)
    if STACK_LO <= p <= STACK_HI:
        uc.mem_write(0x1017e884, struct.pack('<I', 0x00DEAD00))
        return [ustr(uc, p)], None
    return [], None


CACHE_STUBS = (0x100fef50, 0x100fd820, 0x100fd760, 0x10100fd0, 0x100ff860, 0x100b9203, 0x100fc780, 0x100b9dde,
               0x100b8f30, 0x100bcd61, 0x10105a60, 0x100fd7e0)


def cache_begin(ctx):
    r = begin(ctx, *CACHE_STUBS)
    ST['effects'][0x10100fd0] = e_output
    ST['effects'][0x100fd7e0] = e_fprintf
    ST['effects'][0x100fc780] = e_sprintf
    ST['effects'][0x100fd760] = e_fclose
    rets(0x10105a60, 0xffffffff)
    fd_table(ctx)
    return r


def type_arg(ctx, names):
    r = ctx.rng
    nm = r.choice(names + [b'WTB', b'TEXT', b'NOPE', b'wtb', b'3di', b'']) if names else r.choice(TYPE_NAMES + [b'wtb'])
    nm = bytes(c ^ 0x20 if chr(c).isalpha() and r.random() < 0.3 else c for c in nm)
    return cstr(ctx, nm)


def g_b8ff9(ctx):
    r = cache_begin(ctx)
    cache_world(ctx)
    rets(0x100fd820, r.choice([out_stream(ctx)] * 6 + [0]))
    return [cstr(ctx, b'cache.log')]


def g_b979f(ctx):
    r = cache_begin(ctx)
    names = cache_world(ctx)
    fails(ctx, 0.01)
    return [r.choice([type_arg(ctx, names)] * 40 + [0])]


def g_b9b3f(ctx):
    r = cache_begin(ctx)
    names = cache_world(ctx)
    fails(ctx, 0.005)
    key = ctx.buf(16, bytes(r.choice(b'abcdXYZ\0') for _ in range(8)) + rb(ctx, 8))
    return [ctx.u32(), key, type_arg(ctx, names), r.choice([0, 16, 0x1000, 0xfffff, 0x100000, ctx.u32()])]


def text_stream(ctx, lines):
    return stream(ctx, b''.join(lines))


def where_lines(ctx):
    r = ctx.rng
    out = []
    for _ in range(r.choice([0, 1, 2, 4])):
        if r.random() < 0.15:
            out.append(b'#comment\n')
            continue
        t = r.choice(TYPE_NAMES + [b'wav', b'Wtb x'])
        dirs = b';'.join(r.choice([b'c:\\hg', b' d:\\x ', b'data', b'']) for _ in range(r.choice([0, 1, 2, 3])))
        out.append(t + r.choice([b'=', b' = ', b'']) + dirs + r.choice([b'\n', b'  \n', b'']))
    return out


def cache_lines(ctx):
    r = ctx.rng
    out = []
    for _ in range(r.choice([0, 1, 2, 4])):
        if r.random() < 0.1:
            out.append(b'# x\n')
            continue
        t = r.choice(TYPE_NAMES + [b'wav', b'tex'])
        parts = [t, r.choice([b'4', b'16', b'-1', b'1024', b'0x10']), r.choice([b'1000', b'-1', b'65536'])]
        parts = parts[:r.choice([1, 2, 3, 3, 3])]
        out.append(r.choice([b' ', b'\t', b'  ']).join(parts) + b'\n')
    return out


def g_b9fa5(ctx):
    r = cache_begin(ctx)
    heap_world(ctx)
    fails(ctx, 0.03, 30)     # round 25: more allocation failures, for the assert paths
    rets(0x100fd820, r.choice([text_stream(ctx, where_lines(ctx))] * 8 + [0]))
    return []


def g_ba304(ctx):
    r = cache_begin(ctx)
    heap_world(ctx)
    fails(ctx, 0.03, 30)     # round 25: more allocation failures, for the assert paths
    rets(0x100fd820, r.choice([text_stream(ctx, cache_lines(ctx))] * 8 + [0]))
    return [cstr(ctx, b'cache.ini')]


def g_b9eab(ctx):
    r = cache_begin(ctx)
    heap_world(ctx)
    rets(0x100b8f30, r.choice([0, 1, 1]))
    vendor = r.choice([b'GenuineIntel', b'AuthenticAMD', b'genuineintel', b'GenuineIntex'])
    wb(ctx, 0x10199194, vendor[0:4])
    wb(ctx, 0x1019917c, vendor[4:8])
    wb(ctx, 0x10199190, vendor[8:12])
    rets(0x100b9dde, r.choice([0x45, 0x58, 0x12345646, 0xffffff47]))
    rets(0x100bcd61, r.choice([0, 0, 1]))
    w32(ctx, 0x1017e1bc, r.choice([0, 0, 1]))
    wb(ctx, 0x1017dfd8, b'unpack.dll\0')
    rets(0x100fd820, r.choice([0, text_stream(ctx, where_lines(ctx))]), r.choice([0, text_stream(ctx, cache_lines(ctx))]),
         r.choice([0, text_stream(ctx, cache_lines(ctx))]))
    pack = r.choice([0, cstr(ctx, b'data\\hg.pak', 8), cstr(ctx, b'x', 8)])
    return [pack, cstr(ctx, b'cache.ini')]


# ---- 0x100bc610 .. 0x100bcd61: the packed resource file ----------------------------------------------------
def e_unpack(uc, n, raw):
    """An unpacker (out, &size, packed, packed size): writes size bytes made from the packed ones, answers."""
    size = min(ur32(uc, raw[1]), 0x200)
    src = bytes(uc.mem_read(raw[2], min(raw[3], 0x200))) if raw[3] else b''
    out = bytes((src[i % len(src)] if src else 0) ^ (i & 0xff) for i in range(size))
    if out:
        uc.mem_write(raw[0], out)
    return ST['unpack'][n % len(ST['unpack'])]


def pack_world(ctx):
    """File image of items (header 0x16 bytes + packed bytes), type lists 0x1017e1d0, groups 0x1017e1c8."""
    r = ctx.rng
    data = bytearray(rb(ctx, 0x10))
    items = []
    for k in range(r.choice([1, 2, 4, 6])):
        name = bytes(r.choice(b'ABCD12') for _ in range(r.choice([3, 5, 8])))
        ext = r.choice([b'WTB', b'XEL', b'TEX', b'WTBE'])
        method = r.choice([0, 0, 1, 2])
        packed = r.choice([0, 4, 16, 0x40, 0x100])
        size = r.choice([packed, packed * 2 + 1, 0, 0x80])
        pos = len(data)
        data += name.ljust(8, b'\0') + ext.ljust(4, b'\0') + struct.pack('<HII', method, size, packed)
        data += rb(ctx, packed)
        items.append((name, ext, pos, packed))
    ST['file'] = bytes(data) + rb(ctx, 0x40)
    # type lists: per extension (4 bytes) a record { name, pointer array of items { name (8), +8 position } }
    byext = {}
    for name, ext, pos, packed in items:
        byext.setdefault(ext[:4].ljust(4, b'\0'), []).append((name, pos))
    recs = []
    for ext, lst in sorted(byext.items()):
        its = []
        for name, pos in sorted(lst):
            it = newbuf(ctx, 0x10)
            wb(ctx, it, name.ljust(8, b'\0')[:8])
            w32(ctx, it + 8, pos)
            its.append(it)
        rec = newbuf(ctx, 0x10)
        wb(ctx, rec, ext)
        w32(ctx, rec + 4, ptr_array(ctx, its))
        recs.append(rec)
    w32(ctx, 0x1017e1d0, ptr_array(ctx, recs))
    # groups: name (4), first position, count
    groups = []
    k = 0
    while k < len(items):
        n = r.choice([1, 2, 3])
        groups.append((items[k][1][:4], items[k][2], min(n, len(items) - k)))
        k += n
    g = newbuf(ctx, 12 * len(groups) + 12)
    for i, (nm, pos, n) in enumerate(groups):
        wb(ctx, g + 12 * i, nm.ljust(4, b'\0'))
        w32(ctx, g + 12 * i + 4, pos, n)
    w32(ctx, 0x1017e1c8, g)
    w32(ctx, 0x1017e1c4, len(groups))
    w32(ctx, 0x1017e1c0, r.choice([0x00A7F000, 0x00A7F000, 0x00A7F000, 0]))
    w8(ctx, 0x1017e1b8, r.choice([0x45, 0x47, 0]))
    for m in range(4):
        w32(ctx, 0x1017dfd4 + 32 * m, code(ctx, 8 + m, 4, eff=e_unpack))
    ST['unpack'] = [r.choice([0, 0, 0, 1]) for _ in range(3)]
    w32(ctx, 0x1017b958, r.choice([0, newbuf(ctx, 0x200)]))
    w32(ctx, 0x1017b95c, r.choice([0, 0x10, 0x200, 0x7fffffff]))
    for a in (0x1017b914, 0x1017b8f8, 0x1017b900):
        w32(ctx, a, r.choice([0, ctx.u32()]))
    return items


PACK_STUBS = (0x100fef50, 0x100feb20, 0x100fe830, 0x100ff860, 0x100fc780, 0x100fd820, 0x100fd760)


def pack_begin(ctx):
    r = begin(ctx, *PACK_STUBS)
    heap_world(ctx)
    ST['effects'][0x100fc780] = e_sprintf
    fails(ctx, 0.002)
    return r


def g_bc70d(ctx):
    r = pack_begin(ctx)
    items = pack_world(ctx)
    ST['read_short'] = {k for k in range(3) if r.random() < 0.05}
    fails(ctx, 0.15, 6)     # round 25: allocation failures, for the assert paths
    pos = r.choice([it[2] for it in items])
    return [pos, zbuf(ctx, 8), zbuf(ctx, 8), r.choice([0, zbuf(ctx, 16)]), r.choice([0, zbuf(ctx, 8)])]


def g_bc610(ctx):
    r = pack_begin(ctx)
    items = pack_world(ctx)
    w32(ctx, 0x1017e1cc, r.choice([0, code(ctx, 12, 4, rets=[0])]))
    t = r.choice([it[1][:4] for it in items] + [b'nope'])
    t = bytes(c ^ 0x20 if chr(c).isalpha() and r.random() < 0.3 else c for c in t)
    return [cstr(ctx, t)]


def g_bca49(ctx):
    r = pack_begin(ctx)
    items = pack_world(ctx)
    it = r.choice(items)
    name = it[0] if r.random() < 0.8 else b'ZZZ'
    ext = it[1][:3] if r.random() < 0.85 else b'QQ'
    return [cstr(ctx, name), cstr(ctx, ext), r.choice([0, zbuf(ctx, 8)])]


def g_baff4(ctx):
    r = cache_begin(ctx)
    names = cache_world(ctx)
    items = pack_world(ctx)
    ST['effects'][0x100fe830] = e_fread
    ST['effects'][0x100feb20] = e_fseek
    ST['active'] |= {0x100feb20, 0x100fe830}
    w32(ctx, 0x1017d5fc, r.choice([0, 1, 1]))
    rets(0x100b9dde, r.choice([0x45, 0x58, 0x12345646]))
    it = r.choice(items)
    name = it[0] if r.random() < 0.8 else b'ZZZ'
    ext = r.choice([it[1][:3], it[1][:3].lower(), r.choice(TYPE_NAMES), b'tex'])
    namebuf = cstr(ctx, name)
    t = ST['types'].get(ext.upper())
    if t and r.random() < 0.5:
        # the item is in the cache: put it first in the chain of its key (the 8 bytes of the name buffer)
        key = bytes(ctx.m.uc.mem_read(namebuf, 8))
        k0, k1 = struct.unpack('<II', key)
        x = k0 ^ k1
        x = x - (1 << 32) if x & 0x80000000 else x
        hsize = r32(ctx, t)
        b = abs(x) % hsize * (1 if x >= 0 else -1)
        if b >= 0:
            e = cache_item(ctx, key)
            slot = r32(ctx, t + 0x10) + 4 * b
            head = r32(ctx, slot)
            w32(ctx, e + 0x1c, head)
            w32(ctx, e + 0x20, 0)
            if head:
                w32(ctx, head + 0x20, e)
            w32(ctx, slot, e)
    return [namebuf, cstr(ctx, ext), r.choice([0, zbuf(ctx, 8)])]


def g_ba7d5(ctx):
    """Round 25: as g_baff4, with the two file opens (_fsopen), ftell and the search path (0x100bae0a)."""
    r = cache_begin(ctx)
    names = cache_world(ctx)
    items = pack_world(ctx)
    ST['effects'][0x100fe830] = e_fread
    ST['effects'][0x100feb20] = e_fseek
    ST['active'] |= {0x100feb20, 0x100fe830}
    w32(ctx, 0x1017d5fc, r.choice([0, 1, 1]))
    fails(ctx, 0.08, 12)    # allocation failures, for the three assert paths
    ST['active'] |= {0x100fe970}
    data = bytes(r.getrandbits(8) for _ in range(r.choice([0, 4, 40])))
    ST['w31data'] = data
    rets(0x100fd820, *[r.choice([0, 0, stream(ctx, data)]) for _ in range(r.choice([1, 2, 3, 5]))])
    rets(0x100fe970, len(data))
    for k in range(4, 9):
        w32(ctx, 0x1017b904 + 4 * (k - 4), r.choice([0, 0x1234 + k]))
    w32(ctx, 0x1017b910, r.choice([0, 0x2222]))
    w32(ctx, 0x1017b8fc, 0x3333)
    rets(0x100b9dde, r.choice([0x45, 0x58, 0x12345646]))
    it = r.choice(items)
    name = it[0] if r.random() < 0.8 else b'ZZZ'
    ext = r.choice([it[1][:3], it[1][:3].lower(), r.choice(TYPE_NAMES), b'tex'])
    namebuf = cstr(ctx, name)
    t = ST['types'].get(ext.upper())
    if t and r.random() < 0.5:
        # the item is in the cache: put it first in the chain of its key (the 8 bytes of the name buffer)
        key = bytes(ctx.m.uc.mem_read(namebuf, 8))
        k0, k1 = struct.unpack('<II', key)
        x = k0 ^ k1
        x = x - (1 << 32) if x & 0x80000000 else x
        hsize = r32(ctx, t)
        b = abs(x) % hsize * (1 if x >= 0 else -1)
        if b >= 0:
            e = cache_item(ctx, key)
            slot = r32(ctx, t + 0x10) + 4 * b
            head = r32(ctx, slot)
            w32(ctx, e + 0x1c, head)
            w32(ctx, e + 0x20, 0)
            if head:
                w32(ctx, head + 0x20, e)
            w32(ctx, slot, e)
    if r.random() < 0.6:
        # a search path for ext ([0x1017d600], Cache_FindName's table: count +4, items +8; an item is the 8-byte
        # name then +8 the directory list: count +4, strings +8)
        dirs = [cstr(ctx, d) for d in r.choice([[b'data'], [b'c:/hg', b'd'], []])]
        dl = ptr_array(ctx, dirs)
        item = ctx.buf(0x10, ext.upper()[:7].ljust(8, bytes(1)) + struct.pack('<I', dl) + bytes(4))
        w32(ctx, 0x1017d600, ptr_array(ctx, [item]))
    else:
        w32(ctx, 0x1017d600, 0)
    return [namebuf, cstr(ctx, ext), r.choice([0, zbuf(ctx, 8)])]


def g_ba7d5_patch(ctx):
    """Round 31: Sim_100ba7d5 with 1.2's Patch folder on (g_w8yPatch). 1.2 (0x100a2bc9) is 1.0 with two opens put
    first, "Patch\\name.ext<letter>" and "Patch\\name.ext", each read like 1.0's file of the same kind. The C runs
    against the 1.0 original given the file the C finds in Patch as its own file of that kind:
      A: neither Patch file: the C's opens are two failures, then the original's;
      B: Patch\\name.ext<letter>: both get it at their first open;
      C: only Patch\\name.ext: both get a failure, then it.
    log_map drops (A) or renames (B, C) the C's Patch paths and its 1.2 assert lines (0x44f, 0x479)."""
    args = g_ba7d5(ctx)
    r = ctx.rng
    opens = (ST['rets'][0x100fd820] * 12)[:12]       # long enough that the C's two extra opens do not wrap
    case = r.choice('AABC')
    ST['w31case'] = case
    if case == 'A':
        rets(0x100fd820, *opens)
        ST['w31c'] = [0, 0] + opens
    else:
        s = stream(ctx, ST['w31data'])
        rets(0x100fd820, *([s] if case == 'B' else [0, s]))
        ST['w31c'] = list(ST['rets'][0x100fd820])
    return args


def w31_pre_c(m):
    ST['rets'][0x100fd820] = ST['w31c']
    m.uc.mem_write(m.syms['g_w8yPatch'], struct.pack('<I', 1))


W31_FMT = '%s\\%s.%s'


W31_PRE = 'Patch' + chr(92)


def _w31_forms(v):
    """(the Patch format, the Patch prefix, 1.0's format) in v's type"""
    if isinstance(v, bytes):
        return W31_FMT.encode(), W31_PRE.encode(), b'%s.%s'
    return W31_FMT, W31_PRE, '%s.%s'


def _w31_patchy(e):
    """an entry of the C's Patch tries: a text that is a Patch path (the search path 0x100bae0a uses the same
    format, so the format alone does not tell)"""
    for v in e[1]:
        if isinstance(v, (bytes, str)) and v.startswith(_w31_forms(v)[1]):
            return True
    return False


def _w31_text(v, patchy):
    if patchy and isinstance(v, (bytes, str)):
        fmt, pre, fmt10 = _w31_forms(v)
        if v == fmt:
            return fmt10
        return v[len(pre):] if v.startswith(pre) else v
    return v


def w31_log_map(log_o, log_c):
    out = []
    for e in log_c:
        patchy = _w31_patchy(e)
        if ST['w31case'] == 'A' and patchy:
            continue
        a = tuple(_w31_text(v, patchy) for v in e[1])
        if e[0] == 'stub_100ff860':
            a = tuple({0x44f: 0x442, 0x479: 0x46c}.get(v, v) if isinstance(v, int) else v for v in a)
        out.append((e[0], a))
    return log_o, out


def g_bcd61(ctx):
    """A directory image: file count, type count, types (name, capacity), groups, unpacker names, items."""
    r = pack_begin(ctx)
    fd_table(ctx)
    types = sorted(set(r.choice([b'WTB', b'XEL', b'TEX', b'3DI']) for _ in range(r.choice([1, 2, 3]))))
    files = [(bytes(r.choice(b'ABC12') for _ in range(r.choice([3, 8]))), r.choice(types)) for _ in range(r.choice([0, 1, 3, 5]))]
    d = struct.pack('<II', len(files), len(types))
    for t in types:
        d += t.ljust(4, b'\0') + struct.pack('<I', r.choice([8, 8, 1, len(files) + 1]))
    ng = r.choice([0, 1, 2])
    d += struct.pack('<I', ng)
    for _ in range(ng):
        d += r.choice(types).ljust(4, b'\0') + struct.pack('<II', r.randrange(0, 0x100), r.randrange(0, 4))
    nu = r.choice([0, 1, 2])
    d += struct.pack('<I', nu)
    for k in range(nu):
        d += (b'unpack%d.dll' % k).ljust(16, b'\0')
    for name, t in files:
        d += name.ljust(8, b'\0')[:8] + t.ljust(4, b'\0') + struct.pack('<II', r.randrange(0, 0x1000), 0)
    ST['file'] = d + rb(ctx, 0x20)
    f = newbuf(ctx, 0x20)
    w32(ctx, f + 0x10, r.choice([3] * 20 + [0x40]))
    rets(0x100fd820, r.choice([f] * 20 + [0]))
    ST['active'] |= {0x10105fe0}
    return [cstr(ctx, b'data\\hg.dir')]


# ---- 0x100a89f0, 0x100a8d09: control records of units --------------------------------------------------------
def unit_body(ctx):
    """A unit: +4 / +0xc ids, +0x14 kind, +0x48 scene node (with object +0x8c), +0x54 state block, +0x58.. floats,
    +0xf0 target word."""
    r = ctx.rng
    u = newbuf(ctx, 0x180)
    sane_floats(ctx, u + 0x58, 0x18, 3000)
    w32(ctx, u + 4, r.choice([0, 1, 2, 3]))
    w32(ctx, u + 0xc, r.choice([0, 1, 2, 3]))
    w32(ctx, u + 0x14, r.choice([0, 1, 2, 3, 3, 4]))
    w32(ctx, u + 0xf0, r.choice([0, 0x1000, 0x1205, 0x200, ctx.u32()]))
    node = zbuf(ctx, 0xa0)
    sane_floats(ctx, node + 0xc, 0x20, 2)
    w32(ctx, node, r.choice([0, 0, newbuf(ctx, 0xa0)]))
    w32(ctx, node + 0x94, r.choice([0, 1, 0x4000, ctx.u32()]))
    obj = zbuf(ctx, 0xa0)
    w16(ctx, obj, r.choice([0, 0x1000, 0x800, 0x1800]))
    w16(ctx, obj + 2, r.choice([0x70, 0x10, 0x20]))
    w32(ctx, obj + 0x6c, ctx.u32())
    w32(ctx, node + 0x8c, obj)
    w32(ctx, obj + 0x28, node)
    w32(ctx, u + 0x48, node)
    st = newbuf(ctx, 0x60)
    w32(ctx, st, ctx.u32())
    w32(ctx, st + 4, fbits(r.uniform(-720, 720)) if r.random() < 0.9 else fbits(r.uniform(-50000, 50000)))
    w32(ctx, u + 0x54, st)
    return u


def control(ctx):
    r = ctx.rng
    c = newbuf(ctx, 0x580)
    w32(ctx, c, unit_body(ctx))
    sane_floats(ctx, c + 8, 2, 400)
    w32(ctx, c + 0xc, fbits(r.uniform(-720, 720)))
    w32(ctx, c + 0x4f0, r.choice([0, 1, 2, 2, 2, 3, 4, 5, 0xffffffff]))
    w32(ctx, c + 0x4d8, r.choice([0, 0, 100, r.randrange(0, 200000)]))
    w32(ctx, c + 0x534, fbits(r.choice([360.0, 361.0, 359.0, 30.0, 0.0, 1000.0, -5.0, r.uniform(0, 720)])))
    w16(ctx, c + 0x560, r.getrandbits(16))
    w32(ctx, c + 0x520, rfl(ctx, 100))
    return c


UNIT_STUBS = (0x1002bca0, 0x10011093, 0x10011681, 0x100d4590, 0x100d6833, 0x100cb4fb, 0x100aa742, 0x10088a2d,
              0x100a9ba7, 0x10060681, 0x100246b3)


def unit_world(ctx):
    r = ctx.rng
    w32(ctx, 0x1017b8c8, r.choice([0, 1, 2, 3]))
    w32(ctx, 0x1015dc78, r.randrange(0, 200000))
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    w32(ctx, 0x1017c8cc, 0)
    w32(ctx, 0x1017d1c4, r.choice([-1, -1, 0, 1]))
    for k in range(3):
        idx = r.randrange(0, 4)
        w32(ctx, 0x101c56c0 + 4 * k, idx)
        ent = 0x101c57e0 + 0x34 * idx
        node = zbuf(ctx, 0xa0)
        obj = zbuf(ctx, 0xa0)
        w16(ctx, obj, r.choice([0, 0x1000]))
        w16(ctx, obj + 2, r.choice([0x70, 0x10]))
        w32(ctx, node + 0x8c, obj)
        w32(ctx, obj + 0x28, node)
        w32(ctx, ent + 0x30, node)
    w32(ctx, 0x1017d1d0, 0, 0)
    w32(ctx, 0x1017cd38, r.randrange(0, 0x7f))
    w32(ctx, 0x101cffc0, *[r.getrandbits(31) for _ in range(0x7f)])
    rets(0x100cb4fb, r.choice([0, 1]), r.choice([0, 1]))


def g_a89f0(ctx):
    r = begin(ctx, *UNIT_STUBS)
    unit_world(ctx)
    obj = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x28, r.choice([control(ctx)] * 12 + [0]))
    return [obj]


def g_a8d09(ctx):
    r = begin(ctx, *UNIT_STUBS)
    unit_world(ctx)
    return [control(ctx)]


# ---- 0x100ab054: node tree search -------------------------------------------------------------------------
def key_tree(ctx, depth, key):
    r = ctx.rng
    node = zbuf(ctx, 0xa0)
    if r.random() < 0.8:
        o = zbuf(ctx, 0xa0)
        w16(ctx, o + 0x26, key if r.random() < 0.25 else r.choice([0, key + 1, 0xffff]))
        w16(ctx, o + 2, r.choice([0x70, 0x10, 0x20]))
        w32(ctx, node + 0x90, r.choice([0, newbuf(ctx, 8)]))
        w32(ctx, node + 0x8c, o)
    kids = [key_tree(ctx, depth - 1, key) for _ in range(r.choice([0, 1, 2, 3]))] if depth > 0 else []
    if kids:
        w32(ctx, node + 4, kids[0])
        for a, b in zip(kids, kids[1:]):
            w32(ctx, a + 8, b)
    for k in kids:
        w32(ctx, k, node)
    return node


def g_ab054(ctx):
    r = begin(ctx, 0x10029b1f, 0x10088a2d, 0x100aa742, 0x10060b76, 0x10060d2e)
    key = r.choice([1, 5, 0x123])
    w32(ctx, 0x101352e4, ctx.u32())
    for i in range(0x80):
        w32(ctx, 0x109f09e0 + 0x14 * i, 1)
    w32(ctx, 0x1015ea80, 0)
    w32(ctx, 0x1017c8c4, 0)
    w32(ctx, 0x1017c8cc, 0)
    w32(ctx, 0x101352ec, 0x00585858)
    w32(ctx, 0x101352f0, 0x00595959)
    keyarg = key if r.random() < 0.9 else key | 0x10000
    return [r.choice([key_tree(ctx, r.choice([1, 2, 3]), key)] * 10 + [0]), keyarg]


# ---- 0x100abcb3: the 64 squares -------------------------------------------------------------------------------
def g_abcb3(ctx):
    r = begin(ctx, 0x100089c6, 0x100aa64a, 0x10089e91)
    heap_world(ctx)
    # the first allocation of a square (its record) fails now and then: the building stops there
    ST['fail'] = {3 * r.randrange(0, 64)} if r.random() < 0.3 else set()
    w32(ctx, 0x1017b8f8, 0x00AB0000)
    w32(ctx, 0x1015d09c, ctx.u32())
    w32(ctx, 0x1014d1a8, ctx.u32())
    w32(ctx, 0x1017d1c8, r.choice([0, 0, 5]))
    w32(ctx, 0x1017c8cc, 0)
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    return []


# ---- 0x100b45b4, 0x100b469d: the entry table 0x101a2100 ---------------------------------------------------------
def entry_world(ctx):
    r = ctx.rng
    n = r.choice([0, 1, 3, 6, 10])
    w32(ctx, 0x1017d218, n)
    for i in range(n + 2):
        e = 0x101a2100 + 0x88 * i
        wb(ctx, e, rb(ctx, 0x88))
        w32(ctx, e + 0x10, ctx.u32())
        w32(ctx, e + 0x14, r.choice([-1, -1, r.randrange(0, n + 1)]))
        w32(ctx, e + 0x18, r.choice([0, 0x200, 0x800, 0xa00, 0x400, 0x600, r.getrandbits(16)]))
        w32(ctx, e + 0x24, r.choice([-1, r.randrange(0, 8)]))
        w32(ctx, e + 0x28, r.choice([0, box_obj(ctx)]))
        nd = r.choice([0, zbuf(ctx, 0xa0), zbuf(ctx, 0xa0)])
        if nd and r.random() < 0.3:
            w32(ctx, nd + 4, zbuf(ctx, 0xa0))
        w32(ctx, e + 0x2c, nd)
    for k in range(8):
        w16(ctx, 0x109e9ba0 + 0x50 * k, r.choice([0, 0x8400, 0x8000, 0x400, r.getrandbits(16)]))
    w32(ctx, 0x1017d228, r.choice([0, 1]))
    w32(ctx, 0x1019a07c, 0)
    w32(ctx, 0x1017c8cc, 0)
    w32(ctx, 0x1017c8c4, 0)
    w32(ctx, 0x1017e678, 0)
    rets(0x100b43ce, *[r.choice([-1, 0, 1, r.randrange(0, n + 1)]) for _ in range(3)])
    rets(0x100b48de, *[r.choice([0, 1]) for _ in range(3)])


ENTRY_STUBS = (0x100b43ce, 0x100b48de, 0x100897c7, 0x10028d9f)


def g_b45b4(ctx):
    r = begin(ctx, *ENTRY_STUBS)
    entry_world(ctx)
    obj = newbuf(ctx, 0x10)
    w32(ctx, obj + 8, r.choice([-1, 0, 1, 2, 3]))
    return [obj]


def g_b469d(ctx):
    begin(ctx, *ENTRY_STUBS)
    entry_world(ctx)
    return []


def spec(name, orig, gen, trials, **kw):
    return dict(name=name, orig=orig, c=name, gen=gen, trials=trials, **kw)


W8Y_LIGHTS = {'g_w33Light1Cam': 12, 'g_w33Light2Cam': 12}
SPECS = [
    spec('Sim_100a4f03', 0x100a4f03, g_a4f03, 400, void=True),
    # round 42: the object walk reaches Sim_1005874a (the only writer of these), which also turns 1.2's two coloured lights
    # into the camera's frame (C-only, checked against 1.2 in spec_w3p_light_v12): 1.0 has no such variables
    spec('Sim_100a4ff0', 0x100a4ff0, g_tree, 500, void=True, ignore_syms=W8Y_LIGHTS),
    spec('Sim_100a512a', 0x100a512a, g_tree, 500, void=True, ignore_syms=W8Y_LIGHTS),
    spec('Sim_100a526b', 0x100a526b, g_tree, 600, void=True),
    spec('Sim_100a648c', 0x100a648c, g_a648c, 1500),
    spec('Sim_100a76a0', 0x100a76a0, g_a76a0, 400, api=HEAP_API),
    spec('Sim_100a79db', 0x100a79db, g_a79db, 800),
    spec('Sim_100a80d2', 0x100a80d2, g_arch_item, 600),
    spec('Sim_100a8238', 0x100a8238, g_a8238, 600),
    spec('Sim_100a836e', 0x100a836e, g_a836e, 600),
    spec('Sim_100a8405', 0x100a8405, g_a8405, 500),
    spec('Sim_100aa016', 0x100aa016, g_aa016, 500, void=True),
    spec('Sim_100b5502', 0x100b5502, g_b5502, 600, api=HEAP_API),
    spec('Sim_100b581d', 0x100b581d, g_b581d, 500, void=True, api=HEAP_API),
    spec('Sim_100b58c5', 0x100b58c5, g_b58c5, 500, void=True, api=HEAP_API),
    spec('Sim_100b69d0', 0x100b69d0, g_b69d0, 800, api=HEAP_API),
    spec('Sim_100b6ba0', 0x100b6ba0, g_b6ba0, 800, api=HEAP_API),
    spec('Sim_100b6e5e', 0x100b6e5e, g_b6e5e, 800, api=HEAP_API),
    spec('Sim_100b8938', 0x100b8938, g_b8938, 300, void=True),
    spec('Sim_100b8ff9', 0x100b8ff9, g_b8ff9, 500, void=True, api=HEAP_API),
    spec('Sim_100b979f', 0x100b979f, g_b979f, 800, api=HEAP_API),
    spec('Sim_100b9b3f', 0x100b9b3f, g_b9b3f, 800, api=HEAP_API),
    spec('Sim_100b9eab', 0x100b9eab, g_b9eab, 600, void=True, api=HEAP_API),
    spec('Sim_100b9fa5', 0x100b9fa5, g_b9fa5, 4000, void=True, api=HEAP_API),
    spec('Sim_100ba304', 0x100ba304, g_ba304, 4000, void=True, api=HEAP_API),
    spec('Sim_100baff4', 0x100baff4, g_baff4, 800, api=HEAP_API),
    spec('Sim_100ba7d5', 0x100ba7d5, g_ba7d5, 4000, api=HEAP_API),
    dict(name='Sim_100ba7d5_patch', orig=0x100ba7d5, c='Sim_100ba7d5', gen=g_ba7d5_patch, trials=4000,
         api=HEAP_API, pre_c=w31_pre_c, log_map=w31_log_map, ignore_syms={'g_w8yPatch': 4}),
    spec('Sim_100bc610', 0x100bc610, g_bc610, 600, api=HEAP_API),
    spec('Sim_100bc70d', 0x100bc70d, g_bc70d, 4000, api=HEAP_API),
    spec('Sim_100bca49', 0x100bca49, g_bca49, 800, api=HEAP_API),
    spec('Sim_100bcd61', 0x100bcd61, g_bcd61, 600, api=HEAP_API),
    spec('Sim_100a89f0', 0x100a89f0, g_a89f0, 600, void=True),
    spec('Sim_100a8d09', 0x100a8d09, g_a8d09, 1000, void=True),
    spec('Sim_100ab054', 0x100ab054, g_ab054, 600, void=True),
    spec('Sim_100abcb3', 0x100abcb3, g_abcb3, 120, void=True, api=HEAP_API),
    spec('Sim_100b45b4', 0x100b45b4, g_b45b4, 600, void=True, api=HEAP_API),
    spec('Sim_100b469d', 0x100b469d, g_b469d, 600, void=True, api=HEAP_API),
]


# ---- round 25: branch coverage, as w10_common.py (W10_COV=ADDR:size) -------------------------------------
import os
_W8Y_COV = os.environ.get('W10_COV')
_W8Y_SEEN = set()
_COV_RESULT = []


def _w8y_cov_install(m):
    if not _W8Y_COV or getattr(m, '_w8y_cov', False):
        return
    m._w8y_cov = True
    a, n = _W8Y_COV.split(':')
    a, n = int(a, 16), int(n, 0)

    def cb(uc, address, size, ud):
        _W8Y_SEEN.add(address)
    m.raw.hook_add(UC_HOOK_CODE, cb, None, a, a + n - 1)

    def result():
        import capstone
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        ins = [i.address for i in md.disasm(bytes(m.raw.mem_read(a, n)), a)]
        missed = [hex(x) for x in ins if x not in _W8Y_SEEN]
        return len(ins) - len(missed), len(ins), missed
    _COV_RESULT.append(result)


def _cov_summary():
    return _COV_RESULT[0]() if _COV_RESULT else None


_w8y_begin_plain = begin


def begin(ctx, *stubs):
    _w8y_cov_install(ctx.m)
    return _w8y_begin_plain(ctx, *stubs)


# round 25 negative check: W25_NEG=1 builds the C with W25C_MUTATE (Sim_100ba7d5 files a pack item one byte longer)
if os.environ.get('W25_NEG'):
    CFLAGS = CFLAGS + ['-DW25C_MUTATE']
# round 31 negative check: W31_NEG=1 builds the C with W31B_MUTATE (Sim_100ba7d5 passes over Patch\name.ext<letter>)
if os.environ.get('W31_NEG'):
    CFLAGS = CFLAGS + ['-DW31B_MUTATE']
# round 27 negative check: W27_NEG=1 builds the C with W27L_MUTATE (Sim_100b69d0's table starts with room for 99
# pairs, not 100)
if os.environ.get('W27_NEG'):
    CFLAGS = CFLAGS + ['-DW27L_MUTATE']
