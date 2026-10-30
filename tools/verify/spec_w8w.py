# Differential tests for round 8, letter w (see difftest.py and docs/checker.md).
#
# Helpers of the DLL are replaced in the emulator by stubs: a code hook at the helper's address appends
# ('stub_<address>', arguments) to the checker's call log (compared between the original and the C run like the
# Windows calls), may write results through pointer arguments (the same bytes for both runs), sets eax to a value
# chosen by the generator and returns to the caller by setting EIP and ESP (code bytes are not patched).
#  * Helpers that are not converted (called with SIM_FN by the C code) are stubbed at their own address.
#  * Converted helpers are stubbed at their original address (reached by the original through its jump stub) AND
#    at the address of their C function in the test image (reached by the C code), under the same log name.
#  * Helpers that return a value in st(0) return through a small routine `fld qword [value]; ret`, so the caller
#    finds the value (a double chosen by the generator) on the x87 stack exactly as after the real helper.
# Stack addresses are logged as 'stack'; texts are logged by content, records by their bytes (LOG functions).
# Everything else runs for real on both sides: the C library fabs 0x100fcb70 and sqrt 0x100fca04, the converted
# string routines, and the functions of this round that call each other.
# All floating point is checked bit for bit: no float tolerance is used. Float inputs are never signalling NaNs
# (the x87 quiets those on load, the game never makes them) and never values that make the original loop forever.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_1006/w8w_event.c',
    'sim/unsorted_1006/w8w_hud.c',
    'sim/unsorted_1006/w8w_misc.c',
    'sim/unsorted_1007/w8w_misc.c',
    'sim/unsorted_1008/w8w_math.c',
    'sim/unsorted_1008/w8w_nodes.c',
    '../tools/verify/w8w_test.c',
    # existing converted code the functions above call (and what that calls)
    'sim/core/timer.c', 'sim/core/w2a_timers.c', 'sim/core/w2b_timers.c', 'sim/render/w2n_polygon.c',
    'sim/render/w2n_project.c', 'sim/render/w3p_view.c', 'sim/render/w3q_vertex.c', 'sim/render/w7o_draw.c',
    'sim/unsorted_1000/w2e_mono.c', 'sim/unsorted_1000/w2e_names.c', 'sim/unsorted_1000/w2f_records.c',
    'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2j_slots.c', 'sim/unsorted_1000/w2k_misc.c',
    'sim/unsorted_1000/w2l_fields.c', 'sim/unsorted_1000/w2l_objects.c', 'sim/unsorted_1000/w3o_sphere.c',
    'sim/unsorted_1000/w3s_model.c', 'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1000/w5b_calls.c',
    'sim/unsorted_1000/w5x_calls.c', 'sim/unsorted_1000/w5z_calls.c', 'sim/unsorted_1001/w2e_names.c',
    'sim/unsorted_1001/w2e_tables.c', 'sim/unsorted_1001/w2h_objects.c', 'sim/unsorted_1001/w2h_tables.c',
    'sim/unsorted_1001/w2j_misc.c', 'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1001/w4u_cd.c',
    'sim/unsorted_1001/w5c_math.c', 'sim/unsorted_1001/w5y_misc.c', 'sim/unsorted_1002/w2e_object.c',
    'sim/unsorted_1002/w2e_tables.c', 'sim/unsorted_1002/w2g_records.c', 'sim/unsorted_1002/w2h_misc.c',
    'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1002/w2i_setup.c', 'sim/unsorted_1002/w2i_state.c',
    'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_1002/w2k_misc.c', 'sim/unsorted_1002/w2l_tables.c',
    'sim/unsorted_1002/w3o_ray.c', 'sim/unsorted_1002/w3o_vectors.c', 'sim/unsorted_1002/w3s_cell.c',
    'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1002/w4v_misc.c', 'sim/unsorted_1002/w5a_misc.c',
    'sim/unsorted_1002/w5y_misc.c', 'sim/unsorted_1002/w5z_calls.c', 'sim/unsorted_1002/w6d_calls.c',
    'sim/unsorted_1003/entry.c', 'sim/unsorted_1003/records.c', 'sim/unsorted_1003/setters.c',
    'sim/unsorted_1003/w2e_misc.c', 'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_1003/w2f_misc.c',
    'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1003/w2l_records.c', 'sim/unsorted_1003/w3r_misc.c',
    'sim/unsorted_1003/w3s_entry.c', 'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1003/w4v_misc.c',
    'sim/unsorted_1003/w5a_misc.c', 'sim/unsorted_1003/w5c_calls.c', 'sim/unsorted_1003/w6e_misc.c',
    'sim/unsorted_1003/w7m_state.c', 'sim/unsorted_1004/fields.c', 'sim/unsorted_1004/state.c',
    'sim/unsorted_1004/tables.c', 'sim/unsorted_1004/w2e_strings.c', 'sim/unsorted_1004/w2h_misc.c',
    'sim/unsorted_1004/w2h_record.c', 'sim/unsorted_1004/w2i_misc.c', 'sim/unsorted_1004/w2j_misc.c',
    'sim/unsorted_1004/w2k_flags.c', 'sim/unsorted_1004/w2l_values.c', 'sim/unsorted_1004/w3o_misc.c',
    'sim/unsorted_1004/w3p_net.c', 'sim/unsorted_1004/w3s_records.c', 'sim/unsorted_1004/w4w_net.c',
    'sim/unsorted_1004/w5a_misc.c', 'sim/unsorted_1004/w5y_misc.c', 'sim/unsorted_1004/w6e_misc.c',
    'sim/unsorted_1005/w2g_pool.c', 'sim/unsorted_1005/w2h_tables.c', 'sim/unsorted_1005/w2j_misc.c',
    'sim/unsorted_1005/w2k_misc.c', 'sim/unsorted_1005/w3o_calls.c', 'sim/unsorted_1006/w2f_records.c',
    'sim/unsorted_1006/w3o_add.c', 'sim/unsorted_1006/w3p_box.c', 'sim/unsorted_1006/w3q_quat.c',
    'sim/unsorted_1006/w3t_misc.c', 'sim/unsorted_1006/w5a_math.c', 'sim/unsorted_1006/w5b_calls.c',
    'sim/unsorted_1006/w5c_math.c', 'sim/unsorted_1006/w5z_calls.c', 'sim/unsorted_1006/w6e_misc.c',
    'sim/unsorted_1006/w7n_misc.c', 'sim/unsorted_1007/node_flag.c', 'sim/unsorted_1007/w2e_state.c',
    'sim/unsorted_1007/w2f_misc.c', 'sim/unsorted_1007/w2g_stubs.c', 'sim/unsorted_1007/w2j_misc.c',
    'sim/unsorted_1007/w2l_fields.c', 'sim/unsorted_1007/w3o_load.c', 'sim/unsorted_1007/w3p_net.c',
    'sim/unsorted_1007/w5a_misc.c', 'sim/unsorted_1007/w6e_misc.c', 'sim/unsorted_1007/w7n_misc.c',
    'sim/unsorted_1008/cell_link.c', 'sim/unsorted_1008/w2g_rect.c', 'sim/unsorted_1008/w2i_rect.c',
    'sim/unsorted_1008/w3o_grid.c', 'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1008/w3q_place.c',
    'sim/unsorted_1008/w3s_grid.c', 'sim/unsorted_1008/w4v_fade.c', 'sim/unsorted_1008/w4w_display.c',
    'sim/unsorted_1008/w5b_calls.c', 'sim/unsorted_1008/w5y_misc.c', 'sim/unsorted_1008/w6f_cells.c',
    'sim/unsorted_1008/w6f_display.c', 'sim/unsorted_1008/w7o_cells.c', 'sim/unsorted_1009/w3o_ray.c',
    'sim/unsorted_100a/w2f_links.c', 'sim/unsorted_100a/w2g_misc.c', 'sim/unsorted_100a/w2h_misc.c',
    'sim/unsorted_100a/w2i_misc.c', 'sim/unsorted_100a/w2k_list.c', 'sim/unsorted_100a/w2l_small.c',
    'sim/unsorted_100a/w3t_misc.c', 'sim/unsorted_100a/w4u_misc.c', 'sim/unsorted_100a/w5a_misc.c',
    'sim/unsorted_100a/w5b_calls.c', 'sim/unsorted_100a/w5c_calls.c', 'sim/unsorted_100a/w5x_calls.c',
    'sim/unsorted_100a/w5y_misc.c', 'sim/unsorted_100a/w5z_calls.c', 'sim/unsorted_100a/w6g_nodes.c',
    'sim/unsorted_100a/w7o_nodes.c', 'sim/unsorted_100b/fields.c', 'sim/unsorted_100b/w2e_strings.c',
    'sim/unsorted_100b/w2h_records.c', 'sim/unsorted_100b/w2j_misc.c', 'sim/unsorted_100b/w2l_records.c',
    'sim/unsorted_100b/w3o_hash.c', 'sim/unsorted_100b/w3s_array.c', 'sim/unsorted_100b/w4v_wrappers.c',
    'sim/unsorted_100b/w5c_calls.c', 'sim/unsorted_100b/w5z_calls.c', 'sim/unsorted_100b/w6g_misc.c',
    'sim/unsorted_100c/w2e_tables.c', 'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_100c/w2l_objects.c',
    'sim/unsorted_100c/w3o_misc.c', 'sim/unsorted_100c/w4v_timers.c', 'sim/unsorted_100c/w5a_misc.c',
    'sim/unsorted_100c/w5c_math.c', 'sim/unsorted_100c/w5z_calls.c', 'sim/unsorted_100c/w6h_calls.c',
    'sim/unsorted_100d/w2f_records.c', 'sim/unsorted_100d/w2h_misc.c', 'sim/unsorted_100d/w2i_select.c',
    'sim/unsorted_100d/w2i_table.c', 'sim/unsorted_100d/w2j_nodes.c', 'sim/unsorted_100d/w5b_calls.c',
    'sim/unsorted_100d/w5z_calls.c', 'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_100f/w2g_crt.c',
    'sim/unsorted_100f/w2j_str.c', 'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_100f/w6h_crt.c',
    'sim/unsorted_100f/w6i_misc.c', 'sim/unsorted_1010/w2g_crt.c', 'sim/unsorted_1010/w2i_crt.c',
    'sim/util/field.c', 'sim/util/lookup.c', 'sim/util/ring.c', 'sim/view/matrix.c', 'sim/view/objlist.c',
    'sim/view/viewrect.c', 'sim/view/w2d_view.c', 'sim/view/w2l_lists.c', 'sim/view/w2l_rects.c',
    'sim/view/w3r_matrix.c', 'sim/win/w4a_heap.c', 'sim/win/w4u_cdaudio.c', 'sim/win/w4u_gdi.c',
    'sim/win/w4v_dll.c', 'sim/win/w4v_heap.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000       # st(0) results are stored here (by the trampolines and the C test wrapper)
TRAMP = 0x00AF0000      # machine-code trampolines for the original side
FLDRET = 0x00AFE000     # `fld qword [FLDVAL + 16 * k]; ret` routines for stubs that return in st(0)
FLDVAL = 0x00AFE800

# address -> (number of dword arguments logged, C symbol of the converted helper or None, returns in st(0))
STUBS = {}
LOG = {}            # address -> function(uc, raw arguments) -> logged tuple (instead of the plain dwords)
ST = {}


def stub(addr, nargs, c=None, st0=False, log=None):
    STUBS[addr] = (nargs, c, st0)
    if log:
        LOG[addr] = log


# helpers that are not converted (or that another round-8 list converts)
for _a, _n in [(0x100ba7d5, 3), (0x100f0205, 5), (0x100e91dc, 5), (0x100f01ad, 1), (0x100e96a7, 1),
               (0x100e96c6, 5), (0x100f01ea, 1), (0x100d2669, 3), (0x1000c1ea, 4), (0x100eb070, 2),
               (0x1007581a, 1), (0x100ed743, 2), (0x100ecce0, 1), (0x1008de0a, 3), (0x1008e0fe, 3),
               (0x1008dba2, 3), (0x10099510, 0), (0x100fe830, 4), (0x100fd760, 1), (0x100febc0, 4),
               (0x100fd820, 3), (0x10089066, 4), (0x1008950e, 4), (0x100e9cd1, 10), (0x100ea9f5, 5),
               (0x100c2ae5, 1), (0x1006b52f, 5), (0x1006a12d, 2), (0x1006bf33, 3), (0x1006ad98, 4),
               (0x100d6f16, 2), (0x1009cd36, 2), (0x100f1f52, 2), (0x100f1f86, 2), (0x100461e8, 2)]:
    stub(_a, _n)
# converted helpers, stubbed on both sides
for _a, _n, _c in [(0x100bb2b0, 2, 'Sim_100bb2b0'), (0x1006535f, 4, 'Sim_1006535f'),
                   (0x10083ebb, 3, 'Gdi_SetPalette'), (0x10083c45, 0, 'Sim_10083c45'),
                   (0x10089907, 5, 'Sim_10089907'), (0x1002fd4c, 7, 'Sim_1002fd4c'),
                   (0x100a9d47, 4, 'Sim_100a9d47'), (0x100aa0da, 5, 'Sim_100aa0da'), (0x100aa64a, 1, 'Sim_100aa64a'),
                   (0x10067356, 10, 'Sim_10067356'), (0x100881dd, 2, 'Sim_100881dd'),
                   (0x10028b90, 2, 'Sim_10028b90'), (0x1002977e, 4, 'Sim_1002977e'), (0x100a9cfb, 4, 'Sim_100a9cfb'),
                   (0x100cbdb8, 3, 'Sim_100cbdb8'), (0x1007cbb4, 0, 'Sim_1007cbb4'), (0x1000ebfb, 5, 'Sim_1000ebfb'),
                   (0x100cb8ea, 1, 'Sim_100cb8ea'), (0x100386d5, 3, 'Sim_100386d5'), (0x1006ab0c, 1, 'Sim_1006ab0c'),
                   (0x1009c929, 1, 'Sim_1009c929'), (0x1000ed52, 6, 'Sim_1000ed52'),
                   (0x10074fce, 1, 'Sim_10074fce'), (0x1008d4f1, 2, 'Sim_1008d4f1'), (0x10076676, 1, 'Sim_10076676'),
                   (0x1008ca50, 3, 'View_ScaleRect'), (0x1008d2d0, 4, 'Sim_1008d2d0'),
                   (0x1008cdc9, 3, 'Rect_ScalePoint'), (0x1008d0da, 3, 'View_CenterRect'),
                   (0x1008d181, 4, 'Rect_ScaleAroundCenter'), (0x1007e570, 2, 'ProjFile_FindEntry'),
                   (0x100429d5, 0, 'Sim_100429d5'), (0x10078177, 0, 'Sim_10078177')]:
    stub(_a, _n, _c)
# helpers that return a value in st(0)
for _a, _n, _c in [(0x1002d961, 3, 'Sim_1002d961'), (0x100144e9, 1, 'Sim_100144e9'),
                   (0x10084a94, 3, 'Sim_10084a94'), (0x100d6e99, 1, None)]:
    stub(_a, _n, _c, st0=True)

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


def ubytes(uc, addr, n):
    try:
        return bytes(uc.mem_read(addr, n))
    except Exception:
        return ('bad', addr)


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


def ffl(ctx, spread=1000.0):
    """A finite float bit pattern (no NaN or infinity)."""
    while True:
        v = rfl(ctx, spread)
        if (v & 0x7f800000) != 0x7f800000:
            return v


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


# ---- stubs --------------------------------------------------------------------------------------
def _stub_cb(m, addr):
    nargs, _, st0 = STUBS[addr]

    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
        name = 'stub_%x' % addr
        k = sum(1 for e in m.api_log if e[0] == name)       # number of earlier calls of this stub in the run
        if addr in LOG:
            args = LOG[addr](uc, raw)
        else:
            args = tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)
        m.api_log.append((name, args))
        fx = ST['fx'].get(addr)
        if fx:
            fx(m.uc, k, raw)       # tracked writes: the checker restores those pages
        vals = ST['rets'].get(addr) or [0]
        v = vals[k % len(vals)]
        if callable(v):
            v = v(raw)
        if st0:
            slot = sorted(a for a in STUBS if STUBS[a][2]).index(addr)
            uc.mem_write(FLDVAL + 16 * slot, struct.pack('<d', v))
            uc.reg_write(UC_X86_REG_EIP, FLDRET + 16 * slot)       # the routine's ret goes back to the caller
            return
        uc.reg_write(UC_X86_REG_EAX, v & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def begin(ctx):
    """Installs the stubs (once per machine), the x87 state of the game (control word 0x27f) and clears the
    answers of the stubs for this trial. Must be the first thing a generator does."""
    m = ctx.m
    if not getattr(m, '_w8w', False):
        m._w8w = True
        for addr, (nargs, c, st0) in STUBS.items():
            cb = _stub_cb(m, addr)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
            if c is not None and c in m.syms:
                m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[c], m.syms[c])
        for slot, addr in enumerate(sorted(a for a in STUBS if STUBS[a][2])):
            m.uc.mem_write(FLDRET + 16 * slot, b'\xdd\x05' + struct.pack('<I', FLDVAL + 16 * slot) + b'\xc3')
        m.snap0 = m.dump()
        m._index()          # re-index now, before this trial writes anything
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST.clear()
    ST['rets'] = {}
    ST['fx'] = {}
    ST['seed'] = ctx.rng.getrandbits(32)
    return ctx.rng


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def effect(addr, fn):
    ST['fx'][addr] = fn


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


def ptrtab(ctx, count, size=0x200):
    """count records in the pointer table 0x109e8b40."""
    recs = []
    for i in range(count):
        rec = newbuf(ctx, size)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
        recs.append(rec)
    return recs


def a_heapalloc(n, args):
    """HeapAlloc: block n of the trial's pool (n = number of the Windows call in the run); 0 past its end."""
    pool = ST.get('heap', [])
    return pool[n] if n < len(pool) else 0


API = {'HeapAlloc': a_heapalloc}


# ---- 0x100840dc .. 0x10089bad ----------------------------------------------------------------------------------
def table(ctx, addr, n):
    r = ctx.rng
    vals = [fbits(r.uniform(-1, 1)) if r.random() < 0.8 else rfl(ctx) for _ in range(n)]
    w32(ctx, addr, *vals)


def angle(ctx):
    r = ctx.rng
    c = r.random()
    if c < 0.4:
        return fbits(r.uniform(-400, 400))
    if c < 0.6:
        return fbits(r.choice([0.0, -0.0, 90.0, -90.0, 180.0, -180.0, 270.0, 360.0, -360.0, 45.0, 1e-7, 9e-8, -1e-7,
                               89.99, 90.01, 179.99, 180.01, 359.99, 360.01, 720.0]))
    if c < 0.8:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-9, 5))
    if c < 0.85:
        return r.choice([0x7fc00000, 0xffc00000])
    return fbits(r.uniform(-20000, 20000))


def g_4766(ctx):
    begin(ctx)
    table(ctx, 0x103719a0, 0x208)
    tramp_st0(ctx, 0x10084766, 1, 1)
    return [angle(ctx)]


def g_486e(ctx):
    r = begin(ctx)
    table(ctx, 0x103721c0, 0x208)
    tramp_st0(ctx, 0x1008486e, 2, 2)

    def one():
        c = r.random()
        if c < 0.5:
            return fbits(r.uniform(-1000, 1000))
        if c < 0.7:
            return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 1e-7, -1e-7, 9e-8, 2e-7, 5.0, -5.0]))
        if c < 0.9:
            return fbits(r.uniform(-1, 1) * 10 ** r.randint(-9, 9))
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])
    x = one()
    y = one()
    if r.random() < 0.15:
        y = x ^ r.choice([0, 0x80000000])
    if r.random() < 0.1:
        y = (x + r.randrange(-3, 4)) & 0xffffffff
    return [x, y]


def g_4bcf(ctx):
    r = ctx.rng
    begin(ctx)
    pts = [ffl(ctx, 500.0) if r.random() < 0.9 else rfl(ctx) for _ in range(9)]
    if r.random() < 0.1:
        pts[3:6] = pts[0:3]
    outs = [newbuf(ctx, 4) for _ in range(3)]
    return pts + outs


def g_40dc(ctx):
    r = begin(ctx)
    ctx.m.uc.mem_write(0x1015e6a0, rb(ctx, 0x300))
    ctx.m.uc.mem_write(0x109b8c00, rb(ctx, 0x300))
    w32(ctx, 0x1015e698, newbuf(ctx, 4))
    w32(ctx, 0x1015e9a0, ctx.u32())
    target = newbuf(ctx, 0x300) if r.random() < 0.95 else 0
    steps = r.choice([0, 1, 2, 3, 4, 5, 7, 8, 16, 33])
    return [target, steps]


def fread_fx(chunks):
    """fread: writes chunk k (bytes) of the trial into the buffer of the k-th call."""
    def fx(uc, k, raw):
        if k < len(chunks) and chunks[k] is not None:
            uc.mem_write(raw[0], chunks[k][:raw[1] * raw[2]])
    return fx


def g_6e27(ctx):
    r = begin(ctx)
    name = cstr(ctx, r.choice([b'mission.dat', b'x', b'', b'HG_data.bin']))
    rec = newbuf(ctx, 0x80)
    count = r.choice([0, 0, 1, 2, 5])
    head = bytearray(rb(ctx, 0x66))
    head[4:8] = struct.pack('<I', count)
    if r.random() < 0.1:
        head = None
    effect(0x100fe830, fread_fx([bytes(head) if head else None, rb(ctx, 0x1e * max(count, 1))]))
    rets(0x100fe830, 1, count)
    rets(0x100fd820, r.choice([0x00ACE000, 0x00ACE000, 0x00ACE000, 0]))
    ST['heap'] = [newbuf(ctx, 0x1e * max(count, 1)) if r.random() < 0.9 else 0]
    return [name, rec]


def node_tree(ctx, depth, centre_offs):
    """An object (0xb0 bytes) with float fields, maybe a block (+0x28) whose child list (+4, next +8) holds nodes
    with objects (+0x8c) built the same way."""
    r = ctx.rng
    obj = newbuf(ctx, 0xb0)
    w16(ctx, obj, r.choice([0, 0, 0, 0x1000, 0x800, 0x1800, r.getrandbits(16)]))
    for off in (0x44, 0x48, 0x4c, 0x7c, 0x80, 0x84, 0x60, 0x64, 0x68):
        w32(ctx, obj + off, fbits(r.uniform(-300, 300)) if r.random() < 0.95 else rfl(ctx))
    w32(ctx, obj + 0x50, r.choice([fbits(r.uniform(0, 50)), fbits(0.0), fbits(1e-7), fbits(2e-7), rfl(ctx)]))
    w8(ctx, obj + 0xa0, r.choice([0x57, 0x57, 0x77, 0x67, 0, r.getrandbits(8)]))
    c = r.random()
    if depth <= 0 or c < 0.2:
        w32(ctx, obj + 0x28, 0)
        return obj
    block = newbuf(ctx, 0x10)
    w32(ctx, obj + 0x28, block)
    n = r.choice([0, 1, 2, 3])
    prev = block + 4 - 8
    for _ in range(n):
        node = newbuf(ctx, 0x90)
        w32(ctx, node + 0x8c, node_tree(ctx, depth - 1, centre_offs) if r.random() < 0.9 else 0)
        w32(ctx, prev + 8, node)
        prev = node
    w32(ctx, prev + 8, 0)
    return obj


def g_8e37(ctx, target=0x10088e37, slot=3):
    r = begin(ctx)
    tramp_st0(ctx, target, 1, slot)
    return [node_tree(ctx, r.choice([0, 1, 2, 3]), 0x7c) if r.random() < 0.95 else 0]


def g_97c7(ctx):
    r = begin(ctx)
    obj = node_tree(ctx, r.choice([0, 1, 2]), 0x7c)
    w32(ctx, obj + 0x94, r.choice([0, 1, 2, 3, ctx.u32()]))
    rets(0x10089066, ctx.u32())
    rets(0x1008950e, ctx.u32())
    return [obj if r.random() < 0.95 else 0]


def g_9bad(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017c8c4, r.choice([0, 1, 1, 1, 1]))
    w32(ctx, 0x1017c8c0, r.choice([0, 1, 1, 1, 1]))
    for i in range(9):
        for j in range(9):
            if r.random() < 0.3:
                cell = newbuf(ctx, 0x50)
                w32(ctx, 0x1017ba50 + i * 0x18c + j * 0x2c, cell)
            else:
                w32(ctx, 0x1017ba50 + i * 0x18c + j * 0x2c, 0)
    cam = newbuf(ctx, 0x10)
    wf(ctx, cam, r.uniform(-254000, 246000), r.uniform(-100, 100), r.uniform(-246000, 254000))
    if r.random() < 0.1:
        w32(ctx, cam, fbits(r.choice([-254008.0, -198446.0, 246049.0, 0.0])))
    w32(ctx, 0x1014cae0, cam)
    if r.random() < 0.2:
        wf(ctx, 0x1014b8e0, r.uniform(240000, 260000), r.uniform(240000, 260000))
    rets(0x1002d961, *[r.uniform(-50, 50) for _ in range(5)])
    return []


# ---- logging of the helpers' arguments -------------------------------------------------------------------------
def sv(v):
    return 'stack' if isinstance(v, int) and STACK_LO <= v <= STACK_HI else v


def log_with(texts=(), blobs=None, rects=()):
    """Logs the arguments: texts by content, blobs {index: size} by their bytes, rects (pointers to a record
    {surface pointer, x0, y0, x1, y1}) by the four coordinates; the rest as dwords."""
    blobs = blobs or {}

    def log(uc, raw):
        out = []
        for i, v in enumerate(raw):
            if i in texts:
                out.append(ustr(uc, v))
            elif i in blobs:
                out.append(ubytes(uc, v, blobs[i]))
            elif i in rects:
                out.append(ubytes(uc, v + 4, 16))
            else:
                out.append(sv(v))
        return tuple(out)
    return log


def log_surface_record(uc, raw):
    """0x100e9cd1 / 0x100ea9f5: the first argument is a record {pointer to {bits, width, height}, rectangle}."""
    head = ubytes(uc, raw[0], 20)
    if isinstance(head, tuple):
        return (head,) + tuple(sv(v) for v in raw[1:])
    sub = ubytes(uc, struct.unpack('<I', head[:4])[0], 12)
    return (head[4:], sub) + tuple(sv(v) for v in raw[1:])


def log_fwrite(uc, raw):
    n = min(raw[1] * raw[2], 0x16000)
    return (ubytes(uc, raw[0], n), raw[1], raw[2], sv(raw[3]))


def log_floats3(uc, raw):
    return tuple(ubytes(uc, v, 4) for v in raw)


LOG.update({
    0x100ba7d5: log_with(texts=(0, 1)),
    0x100fd820: log_with(texts=(0, 1)),
    0x100febc0: log_fwrite,
    0x10083ebb: log_with(blobs={2: 0x300}),
    0x100e9cd1: log_surface_record,
    0x100ea9f5: log_surface_record,
    0x100d2669: log_floats3,
    0x1009cd36: log_with(blobs={0: 0x38}),
    0x1000ed52: log_with(texts=(3,)),
    0x1000ebfb: log_with(texts=(0,)),
    0x1007e570: log_with(texts=(0,)),
    0x100461e8: log_with(texts=(0,)),
    0x1008de0a: log_with(texts=(0,), blobs={2: 4}),
    0x1008e0fe: log_with(rects=(0,)),
    0x1008dba2: log_with(texts=(1,), rects=(0,)),
    0x1008ca50: log_with(rects=(1,)),
    0x1008d0da: log_with(rects=(1,)),
    0x1008d2d0: log_with(rects=(0,)),
    0x1008d181: log_with(rects=(0,)),
    0x1008cdc9: log_with(rects=(0,), blobs={1: 8}),
})


# ---- effects of the helpers (the same bytes for both runs) -----------------------------------------------------
def fvals(k, n, salt, spread=500.0):
    """n finite floats (bit patterns) chosen by the trial's seed, the call number and a salt."""
    out = []
    for i in range(n):
        x = mix(ST['seed'], k, i, salt)
        out.append(fbits(((x % 2000001) - 1000000) / 1000000.0 * spread) if x % 13 else fbits(0.0))
    return out


def fx_upper(uc, k, raw):
    """0x100ba7d5 upper-cases both texts in place."""
    for a in raw[:2]:
        t = ustr(uc, a)
        if isinstance(t, bytes):
            uc.mem_write(a, t.upper())


def fx_write_floats(index_list, salt):
    def fx(uc, k, raw):
        for i, v in zip(index_list, fvals(k, len(index_list), salt)):
            uc.mem_write(raw[i], struct.pack('<I', v))
    return fx


def fx_rect_out(out_index, salt):
    """Writes the four coordinates of the output record (+4..+0x10)."""
    def fx(uc, k, raw):
        vals = [mix(ST['seed'], k, i, salt) % 640 - 100 for i in range(4)]
        uc.mem_write(raw[out_index] + 4, struct.pack('<4i', *vals))
    return fx


def fx_point_out(uc, k, raw):
    vals = [mix(ST['seed'], k, i, 77) % 100000 - 30000 for i in range(2)]
    uc.mem_write(raw[2], struct.pack('<2i', *vals))


def r_choice(x, options):
    return options[x % len(options)]


# ---- 0x10062386 .. 0x1006d911 ----------------------------------------------------------------------------------
def g_2386(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017692c, r.choice([0, 1, 1, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    seed = ST['seed']
    rets(0x100f01ea, lambda raw: mix(seed, raw[0]))
    return []


def g_2abd(ctx):
    r = begin(ctx)
    head = newbuf(ctx, 0x100)
    w32(ctx, head + 0xf0, r.choice([0x200, 0x2ff, 0x1200, 0x300, 0, ctx.u32()]))
    entries = newbuf(ctx, 0x128 * 4)
    idx = r.randrange(4)
    w32(ctx, entries + idx * 0x128 + 8, r.choice([1, 1, 1, 0, 2]))
    w32(ctx, entries + idx * 0x128 + 4, r.randrange(0x2e))
    obj = newbuf(ctx, 0x500)
    w32(ctx, obj, head)
    w32(ctx, obj + 0x84, entries)
    w32(ctx, obj + 0x4fc, idx)

    def fx_c1ea(uc, k, raw):
        a, b = fvals(k, 2, 5)
        uc.mem_write(raw[1], struct.pack('<I', a))
        uc.mem_write(raw[2], struct.pack('<I', b))
        uc.mem_write(raw[3], struct.pack('<3I', *fvals(k, 3, 6)))
    effect(0x1000c1ea, fx_c1ea)
    effect(0x100d2669, fx_write_floats((0, 1, 2), 7))
    rets(0x100d2669, r.choice([0, 1]))
    return [obj]


def display_setup(ctx):
    r = ctx.rng
    disp = newbuf(ctx, 0x110)
    w32(ctx, 0x109e8940, disp)
    surf = newbuf(ctx, 0x10)
    w32(ctx, disp + 0xec, surf)
    for a in (0x10144ed0, 0x10144ed4, 0x10166904, 0x10176920, 0x10176928, 0x10166908, 0x1016690c):
        w32(ctx, a, r.choice([r.randrange(-50, 400), r.randrange(0, 50), ctx.u32()]))
    w32(ctx, 0x1014cec0, cstr(ctx, r.choice([b'wtb', b'TLT', b'xel'])))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    rets(0x100ba7d5, *[r.choice([0, ctx.u32() | 1, ctx.u32() | 1]) for _ in range(6)])
    rets(0x100bb2b0, *[ctx.u32() for _ in range(3)])
    effect(0x100ba7d5, fx_upper)
    return disp


def g_3f00(ctx):
    r = begin(ctx)
    display_setup(ctx)
    wf(ctx, 0x1017691c, r.choice([0.0, 10.0, -3.5, 100.0]))
    head = newbuf(ctx, 0x100)
    w32(ctx, head + 0xf0, r.choice([0x100, 0x200, 0x300, 0x1100, 0, 0xf00, 0x0ff, ctx.u32()]))
    obj = newbuf(ctx, 0x600)
    w32(ctx, obj, head)
    w32(ctx, obj + 0x50c, r.choice([0, 1, 2, 2]))

    def f():
        c = r.random()
        if c < 0.5:
            return fbits(r.uniform(-100, 100))
        if c < 0.8:
            return fbits(r.choice([3.0, -3.0, 90.0, -90.0, 0.0, -0.0, 1e-7, -1e-7, 2.9, 3.1, -2.9, -3.1, 91.0,
                                   -91.0, 89.0]))
        return rfl(ctx)
    return [obj, ctx.u32(), ctx.u32(), f(), f(), f()]


def g_419e(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017b940, r.choice([0, 1, 1]))
    image = newbuf(ctx, 0xfe10) if r.random() < 0.95 else 0
    rets(0x100f1f52, ctx.u32())
    rets(0x100f1f86, ctx.u32())
    return [ctx.u32(), ctx.u32(), ctx.u32(), r.randrange(-500, 500) & 0xffffffff, r.randrange(-500, 500) & 0xffffffff,
            image, ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32()]


def g_4824(ctx):
    r = begin(ctx)
    pobj = ctx.buf(4, struct.pack('<I', ctx.u32()))
    effect(0x100d6f16, lambda uc, k, raw: uc.mem_write(raw[1], struct.pack('<14I', *fvals(k, 14, 11))))
    effect(0x1009cd36, lambda uc, k, raw: uc.mem_write(raw[0] + 0xc, struct.pack('<3I', *fvals(k, 3, 12, 1e5))))
    effect(0x100d2669, fx_write_floats((0, 1, 2), 13))
    rets(0x100d6e99, r.choice([r.uniform(-100, 100), 0.1 + 1e-12, 1e30, -0.0]))
    rets(0x100d2669, r.choice([0, 1, 2]))
    return [pobj, newbuf(ctx, 4), newbuf(ctx, 4)]


def g_5131(ctx):
    r = begin(ctx)
    display_setup(ctx)
    rec = ptrtab(ctx, 2, 0x40)
    cur = r.randrange(2)
    w32(ctx, 0x1017b8c8, cur)
    blk = newbuf(ctx, 0xc00)
    w32(ctx, rec[cur] + 0x28, blk)
    w32(ctx, blk + 0xbcb, r.choice([4, 4, 0, 0xfb, ctx.u32()]))
    w32(ctx, 0x109e85c0, cstr(ctx, b'radar'))
    effect(0x100d2669, fx_write_floats((0, 1, 2), 14))
    rets(0x100d2669, r.choice([0, 1, 1]))
    return []


def g_519d(ctx, nargs=3):
    r = begin(ctx)
    display_setup(ctx)
    w32(ctx, 0x1017692c, r.choice([0, 1, 1]))
    name = cstr(ctx, r.choice([b'cmpmkr2', b'CRTUP', b'x']))
    args = [ctx.u32(), ctx.u32(), name]
    if nargs == 4:
        args.append(ctx.u32())
    return args


def g_754e(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x560)
    for off in range(0x53c, 0x560, 4):
        w32(ctx, obj + off, ffl(ctx, 100.0))
    px, py, pz = (ctx.buf(4, struct.pack('<I', ffl(ctx, 500.0))) for _ in range(3))
    out = newbuf(ctx, 4)
    nlists = r.choice([0, 1, 2, 3])
    for i in range(nlists):
        lst = newbuf(ctx, 0x40)
        prev = lst
        for _ in range(r.choice([0, 1, 2, 4])):
            node = newbuf(ctx, 0x40)
            w32(ctx, node + 0x34, r.choice([6, 6, 6, 5]))
            w16(ctx, node + 2, r.choice([0, 0, 0x100, 0x50]))
            w32(ctx, prev + 0x18, node)
            prev = node
        w32(ctx, prev + 0x18, 0)
        w32(ctx, 0x1036f2a0 + 4 * i, lst)
    w32(ctx, 0x1036f2a0 + 4 * nlists, 0)
    hits = []
    for _ in range(4):
        hit = newbuf(ctx, 0x60)
        w16(ctx, hit + 2, r.choice([0x50, 0x150, 0x40, 0, 0x5f]))
        wf(ctx, hit + 0x44, *[r.uniform(-100, 100) for _ in range(3)])
        hits.append(hit)
    rets(0x10067356, *[r.choice([0] + hits) for _ in range(6)])
    seed = ST['seed']

    def fx67(uc, k, raw):
        d = mix(seed, k, 1)
        dist = r_choice(d, [fbits(1.0), fbits(5.0), fbits(1e-8), fbits(0.0), fbits(20.0), 0x7fc00000])
        limit = r_choice(d >> 8, [fbits(2.0), fbits(10.0), fbits(1.0), fbits(0.5)])
        uc.mem_write(raw[2], struct.pack('<I', dist))
        uc.mem_write(raw[3], struct.pack('<I', limit))
        for i, v in zip((7, 8, 9), fvals(k, 3, 21, 10.0)):
            uc.mem_write(raw[i], struct.pack('<I', v))
    effect(0x10067356, fx67)
    rets(0x10084a94, *[r.choice([0.0, 1e-30, r.uniform(0, 50), 4.0, 8.0, 32.0 + 1e-9, 1.0 / 3]) for _ in range(3)])
    rets(0x10028b90, r.choice([-1 & 0xffffffff, 0, 3]))
    effect(0x100a9cfb, fx_write_floats((1, 2, 3), 22))
    return [obj, px, py, pz, out]


def g_9c90(ctx):
    r = begin(ctx)
    s = newbuf(ctx, 0xc50)
    obj = newbuf(ctx, 0x240)
    w32(ctx, s, obj)
    w32(ctx, s + 0x50c, r.choice([0, 1, 1, 1, 2]))
    w32(ctx, s + 0xc35, r.choice([0, 1, 1, 1]))
    k0 = r.randrange(8)
    w32(ctx, obj + 0x118, k0 | r.choice([0, 0x100, 0x100, 0x1100, 0x1000]) | (r.getrandbits(8) << 16))
    w32(ctx, obj + 4, r.choice([0, 1, 2]))
    w32(ctx, 0x1017b8c8, r.choice([0, 1]))
    w32(ctx, obj + 8, r.choice([0, 3, 15, 16, 40]))
    blk = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x54, blk)
    w32(ctx, obj + 0x21c, r.choice([0, 1, 1, 2]))
    for k in range(8):
        e = 0x109a8650 + k * 0x54
        wf(ctx, e, r.uniform(-10, 10))
        w16(ctx, e + 0x14, r.choice([0, 0x20, 0x60, 0x40, 0x20]))
        w16(ctx, e + 0x16, r.getrandbits(16))
    wf(ctx, 0x1015e4c4, r.uniform(-10, 10))
    seq = [r.randrange(8) for _ in range(r.randrange(6))] + [k0]      # comes back to the first entry

    def fx_cb(uc, k, raw):
        if raw[1] == 1 or k % 2:
            v = ur32(uc, raw[0] + 0x118)
            uc.mem_write(raw[0] + 0x118, struct.pack('<I', (v & ~0xff) | seq[k % len(seq)]))
    effect(0x100cbdb8, fx_cb)
    rets(0x100c2ae5, r.choice([0, 0, 1]))
    rets(0x100144e9, r.uniform(-5, 5))
    return [s]


def g_9fd2(ctx):
    r = begin(ctx)
    w32(ctx, 0x10179ce0, r.choice([0, 1, 1, 1]))
    s = newbuf(ctx, 0x600)
    slots = newbuf(ctx, 0x11 * 0x3c)
    w32(ctx, s + 0x88, slots)
    nparts = r.choice([0, 1, 3, 6])
    parts = newbuf(ctx, 0x24 * max(nparts, 1))
    w32(ctx, s + 0x8c, parts if r.random() < 0.9 else 0)
    w32(ctx, s + 0x51c, nparts)
    targets = newbuf(ctx, 0x128 * 4)
    w32(ctx, s + 0x84, targets)
    for t in range(4):
        w32(ctx, targets + t * 0x128 + 0x20, r.choice([0, 1, 5, -1 & 0xffffffff]))
    ids = [r.choice([10001, 20000, 9999, 0xffff, 12345]) for _ in range(4)]
    for i in range(0x11):
        e = slots + i * 0x3c
        w16(ctx, e + 0x36, r.choice([0, 0, 0x2000, 0x1000]))
        w16(ctx, e + 0x34, r.choice([0, 1, 2, 4]))
        for j in range(4):
            w16(ctx, e + 0xc + 2 * j, r.choice(ids + [5, 100]))
    for p in range(nparts):
        w16(ctx, parts + p * 0x24 + 8, r.choice(ids + [7]))
        w16(ctx, parts + p * 0x24 + 6, r.randrange(4))
    return [ctx.u32(), s]


def g_abf6(ctx):
    r = begin(ctx)
    s = newbuf(ctx, 0xc50)
    obj = newbuf(ctx, 0x160)
    w32(ctx, s, obj)
    w32(ctx, obj, r.choice([1, 1, 9, 9, 2]))
    blk = newbuf(ctx, 0x10)
    w32(ctx, obj + 0x48, blk)
    child = newbuf(ctx, 0x10)
    w32(ctx, blk + 4, child if r.random() < 0.8 else 0)
    w32(ctx, child + 4, r.choice([0, 1, 1]))
    w32(ctx, s + 0xbdb, r.choice([5, 0, 1]))
    wf(ctx, obj + 0x58, *[r.uniform(-1000, 1000) for _ in range(3)])
    cam = newbuf(ctx, 0x10)
    wf(ctx, cam, *[r.uniform(-1000, 1000) for _ in range(3)])
    w32(ctx, 0x1014cae0, cam)
    w32(ctx, 0x1015ef0c, ctx.u32())
    rets(0x1009c929, r.randrange(3))
    code = r.choice([2, 3, 9, 10, 11, 13, 14, 15, 16, 12, 1, 0, 17, 3])
    return [ctx.u32(), s, code]


def box_node(ctx, depth):
    r = ctx.rng
    node = newbuf(ctx, 0x90)
    wf(ctx, node + 0x30, *[r.uniform(-100, 100) for _ in range(3)])
    if r.random() < 0.05:
        w32(ctx, node + 0x30 + 4 * r.randrange(3), rfl(ctx))
    w32(ctx, node + 4, box_node(ctx, depth - 1) if depth > 0 and r.random() < 0.5 else 0)
    w32(ctx, node + 8, box_node(ctx, depth - 1) if depth > 0 and r.random() < 0.5 else 0)
    return node


def fx_box(uc, k, raw):
    vals = fvals(k, 6, 31, 300.0)
    if mix(ST['seed'], k) % 7 == 0:
        vals[mix(ST['seed'], k, 9) % 6] = 0x7fc00000
    for i in range(6):
        uc.mem_write(raw[1 + i], struct.pack('<I', vals[i]))


def g_d5d0(ctx):
    r = begin(ctx)
    effect(0x1002fd4c, fx_box)
    node = box_node(ctx, r.choice([0, 1, 2, 3])) if r.random() < 0.95 else 0
    outs = [newbuf(ctx, 4) if r.random() < 0.98 else 0 for _ in range(6)]
    return [node] + outs


def g_d911(ctx):
    r = begin(ctx)
    effect(0x1002fd4c, fx_box)
    obj = newbuf(ctx, 0x2a0)
    w32(ctx, obj + 0x48, box_node(ctx, r.choice([0, 1, 2])))
    return [obj]


# ---- 0x10074cea .. 0x1007e7b6 ----------------------------------------------------------------------------------
def rect(ctx, w=None, h=None):
    r = ctx.rng
    x0 = r.randrange(0, 300)
    y0 = r.randrange(0, 200)
    w = w if w is not None else r.randrange(1, 300)
    h = h if h is not None else r.randrange(1, 200)
    return ctx.buf(20, struct.pack('<5I', ctx.u32() | 1, x0, y0, x0 + w - 1, y0 + h - 1))


def g_4cea(ctx):
    r = begin(ctx)
    s = newbuf(ctx, 0x80)
    frame = rect(ctx)
    text = rect(ctx, r.randrange(4, 300), r.randrange(4, 200))   # 4 or more: the original's 16.16 quotients fit
    w32(ctx, s, frame if r.random() < 0.95 else 0)
    w32(ctx, s + 0x18, text if r.random() < 0.95 else 0)
    if r.random() < 0.05:
        w32(ctx, frame, 0)
    w32(ctx, s + 0x10, cstr(ctx, b'pnlback1') if r.random() < 0.7 else 0)
    w32(ctx, s + 0x54, r.choice([0, 1, 5, 100, 0x10000]))
    w32(ctx, s + 4, r.choice([0, 0x10, 0x14, 0xffffffff]))
    w32(ctx, 0x101c64f0, r.choice([0, 1]))
    w32(ctx, 0x1014cb38, r.choice([0, 1]))
    w32(ctx, 0x109e3e14, r.choice([319, 639]))
    w32(ctx, 0x109e3de0, r.choice([199, 479]))
    w32(ctx, 0x1014cec0, cstr(ctx, b'wtb'))
    effect(0x100ba7d5, fx_upper)
    rets(0x100ba7d5, r.choice([0, ctx.u32() | 1]))
    rets(0x100ed743, (r.randrange(1, 400) << 16) | r.randrange(1, 300))
    effect(0x1008d181, fx_rect_out(1, 41))
    effect(0x1008ca50, fx_rect_out(2, 42))
    effect(0x1008cdc9, fx_point_out)
    return [s, r.choice([5, 5, 0, 4])]


def g_5667(ctx):
    r = begin(ctx)
    s = newbuf(ctx, 0x80)
    w32(ctx, s, r.choice([0, ctx.u32() | 1, ctx.u32() | 1]))
    w32(ctx, s + 0x18, r.choice([0, ctx.u32() | 1, ctx.u32() | 1, ctx.u32() | 1]))
    w32(ctx, s + 4, r.choice([0, 4, 0x20, 0x24, ctx.u32()]))
    w32(ctx, s + 0x14, r.choice([0, ctx.u32()]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    rets(0x10076676, r.choice([0, 1, 2]))
    return [s]


def file_setup(ctx, reads=()):
    r = ctx.rng
    rets(0x100fd820, r.choice([0x00ACE000, 0x00ACE000, 0x00ACE000, 0]))
    rets(0x100febc0, lambda raw: raw[2])
    effect(0x100fe830, fread_fx([rb(ctx, n) for n in reads]))


def g_7d50(ctx):
    begin(ctx)
    for a in range(0x1096fc00, 0x1096fc1c, 4):
        w32(ctx, a, ctx.u32())
    for a in (0x101355b8, 0x10144fac, 0x1015d1ac):
        w32(ctx, a, ctx.u32())
    file_setup(ctx)
    return []


def g_7e34(ctx):
    begin(ctx)
    file_setup(ctx, (28, 4, 4, 4))
    return []


def g_c89b(ctx):
    r = begin(ctx)
    k = r.randrange(3)
    w32(ctx, 0x1013bb40, k)
    side = 0x10473800 + k * 0x4fc3a
    n = r.choice([0, 1, 3, 8, 16])
    w32(ctx, side, n)
    ctx.m.uc.mem_write(side + 4, rb(ctx, 0x40))
    for i in range(n):
        m = side + i * 0x13f
        ctx.m.uc.mem_write(m + 0x3a, rb(ctx, 2))
        w8(ctx, m + 0x3a, r.choice([5, 5, 1, 0]))
        w8(ctx, m + 0xae, r.choice([0, 1, 1, 7]))
        ctx.m.uc.mem_write(m + 0xa1, rb(ctx, 8))
        ctx.m.uc.mem_write(m + 0xaf, rb(ctx, 4))
        name = bytes(r.choice(b'abcdefXYZ 01') for _ in range(r.randrange(0, 30)))
        ctx.m.uc.mem_write(m + 0xe7, name + b'\0')
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    rets(0x100429d5, ctx.u32())
    file_setup(ctx)
    return []


def g_dd90(ctx):
    r = begin(ctx)
    for i in range(2):
        e = 0x10149d28 + i * 0x3c
        w32(ctx, e + 0x38, rect(ctx))
        ctx.m.uc.mem_write(e + 0x24, r.choice([b'title1\0', b'tl\0', b'abcdefghij\0']))
        ctx.m.uc.mem_write(e + 4, rb(ctx, 8))
    w32(ctx, 0x101c64f0, r.choice([0, 1, 2]))
    w32(ctx, 0x1014cec0, cstr(ctx, b'wtb'))
    w32(ctx, 0x1014cec4, cstr(ctx, r.choice([b'font', b'FONT', b'fnt', b'Font'])))
    effect(0x100ba7d5, fx_upper)
    rets(0x100ba7d5, *[r.choice([0, ctx.u32() | 1]) for _ in range(4)])
    rets(0x100ecce0, *[r.randrange(-10, 40) & 0xffffffff for _ in range(3)])
    effect(0x1008ca50, fx_rect_out(2, 51))
    effect(0x1008d2d0, fx_rect_out(1, 52))
    effect(0x1008cdc9, fx_point_out)
    return []


def g_e29a(ctx):
    r = begin(ctx)
    name1 = cstr(ctx, r.choice([b'', b'', b'back1']))
    name2 = cstr(ctx, b'font8')
    text = cstr(ctx, b'Hello') if r.random() < 0.95 else 0
    w32(ctx, 0x1014cec0, cstr(ctx, b'wtb'))
    w32(ctx, 0x1014cec4, cstr(ctx, b'fnt'))
    effect(0x100ba7d5, fx_upper)
    rets(0x100ba7d5, *[r.choice([0, ctx.u32() | 1, ctx.u32() | 1]) for _ in range(2)])
    effect(0x1008de0a, fx_rect_out(2, 61))
    effect(0x1008d0da, fx_rect_out(2, 62))
    dx = r.choice([-1, 0, 5, -20, 100])
    dy = r.choice([-1, 0, 7, -3, 50])
    return [name1, name2, text, dx & 0xffffffff, dy & 0xffffffff]


def find_data(k):
    data = bytearray(mix(ST['seed'], k, i) & 0xff for i in range(0x140))
    long_name = b'save%d.sfl' % (mix(ST['seed'], k) % 100)
    data[0x2c:0x2c + len(long_name) + 1] = long_name + b'\0'
    if mix(ST['seed'], k, 999) % 2:
        data[0x130:0x13b] = b'SAVE~1.SFL\0'
    else:
        data[0x130] = 0
    return bytes(data)


def a_findfirst(n, args, uc):
    """FindFirstFileA: writes the 0x140-byte find data through the (stack) pointer argument; finds something or
    answers INVALID_HANDLE_VALUE as chosen per trial (by the number of the Windows call)."""
    esp = uc.reg_read(UC_X86_REG_ESP)
    uc.mem_write(ur32(uc, esp + 8), find_data(n))
    return 0x77 if mix(ST['seed'], n, 3) % 3 else 0xffffffff


def a_findnext(n, args, uc):
    if mix(ST['seed'], n, 4) % 3 == 0:
        return 0
    esp = uc.reg_read(UC_X86_REG_ESP)
    uc.mem_write(ur32(uc, esp + 8), find_data(n))
    return 1


API_FIND = {'FindFirstFileA': a_findfirst, 'FindNextFileA': a_findnext}


def g_e66c(ctx):
    r = begin(ctx)
    rets(0x10099510, r.choice([0, 0x44, 0x45, 0x1234500, 0x80]))
    rets(0x1007e570, ctx.u32())
    return []


def g_e7b6(ctx):
    r = begin(ctx)
    ctx.m.uc.mem_write(0x1017aeb0, r.choice([b'keating\0', b'D:/keating\0', b'\0']))
    rets(0x1007e570, r.choice([0, ctx.u32() | 1]))
    rets(0x100461e8, ctx.u32())
    return [cstr(ctx, r.choice([b'game1', b'', b'a long saved game name']))]


SPECS = [
    dict(name='Sim_10084766', orig=TRAMP + 0x100, c='Test_10084766', gen=g_4766, trials=3000),
    dict(name='Sim_1008486e', orig=TRAMP + 0x200, c='Test_1008486e', gen=g_486e, trials=3000),
    dict(name='Sim_10084bcf', orig=0x10084bcf, c='Sim_10084bcf', gen=g_4bcf, trials=2000),
    dict(name='Sim_100840dc', orig=0x100840dc, c='Sim_100840dc', gen=g_40dc, trials=300),
    dict(name='Sim_10086e27', orig=0x10086e27, c='Sim_10086e27', gen=g_6e27, trials=500, api=API),
    dict(name='Sim_10088e37', orig=TRAMP + 0x300, c='Test_10088e37', gen=g_8e37, trials=1500),
    dict(name='Sim_10089325', orig=TRAMP + 0x400, c='Test_10089325', gen=lambda c: g_8e37(c, 0x10089325, 4),
         trials=1500),
    dict(name='Sim_100897c7', orig=0x100897c7, c='Sim_100897c7', gen=g_97c7, trials=800, void=True),
    dict(name='Sim_10089bad', orig=0x10089bad, c='Sim_10089bad', gen=g_9bad, trials=300, void=True),
    dict(name='Sim_10062386', orig=0x10062386, c='Sim_10062386', gen=g_2386, trials=300, void=True),
    dict(name='Sim_10062abd', orig=0x10062abd, c='Sim_10062abd', gen=g_2abd, trials=500, void=True),
    dict(name='Sim_10063f00', orig=0x10063f00, c='Sim_10063f00', gen=g_3f00, trials=1500, void=True),
    dict(name='Sim_1006419e', orig=0x1006419e, c='Sim_1006419e', gen=g_419e, trials=300, void=True),
    dict(name='Sim_10064824', orig=0x10064824, c='Sim_10064824', gen=g_4824, trials=500),
    dict(name='Sim_10065131', orig=0x10065131, c='Sim_10065131', gen=g_5131, trials=500, void=True),
    dict(name='Sim_1006519d', orig=0x1006519d, c='Sim_1006519d', gen=g_519d, trials=500),
    dict(name='Sim_10065216', orig=0x10065216, c='Sim_10065216', gen=lambda c: g_519d(c, 4), trials=500),
    dict(name='Sim_100652e8', orig=0x100652e8, c='Sim_100652e8', gen=lambda c: g_519d(c, 4), trials=500),
    dict(name='Sim_1006754e', orig=0x1006754e, c='Sim_1006754e', gen=g_754e, trials=1500),
    dict(name='Sim_10069c90', orig=0x10069c90, c='Sim_10069c90', gen=g_9c90, trials=1500, void=True),
    dict(name='Sim_10069fd2', orig=0x10069fd2, c='Sim_10069fd2', gen=g_9fd2, trials=800, void=True),
    dict(name='Sim_1006abf6', orig=0x1006abf6, c='Sim_1006abf6', gen=g_abf6, trials=800, void=True),
    dict(name='Sim_1006d5d0', orig=0x1006d5d0, c='Sim_1006d5d0', gen=g_d5d0, trials=1500),
    dict(name='Sim_1006d911', orig=0x1006d911, c='Sim_1006d911', gen=g_d911, trials=800, void=True),
    dict(name='Sim_10074cea', orig=0x10074cea, c='Sim_10074cea', gen=g_4cea, trials=1000, void=True),
    dict(name='Sim_10075667', orig=0x10075667, c='Sim_10075667', gen=g_5667, trials=500),
    dict(name='Sim_10077d50', orig=0x10077d50, c='Sim_10077d50', gen=g_7d50, trials=300, void=True),
    dict(name='Sim_10077e34', orig=0x10077e34, c='Sim_10077e34', gen=g_7e34, trials=300, void=True),
    dict(name='Sim_1007c89b', orig=0x1007c89b, c='Sim_1007c89b', gen=g_c89b, trials=300),
    dict(name='Sim_1007dd90', orig=0x1007dd90, c='Sim_1007dd90', gen=g_dd90, trials=600, void=True),
    dict(name='Sim_1007e29a', orig=0x1007e29a, c='Sim_1007e29a', gen=g_e29a, trials=600, void=True),
    dict(name='Sim_1007e66c', orig=0x1007e66c, c='Sim_1007e66c', gen=g_e66c, trials=500, void=True, api=API_FIND),
    dict(name='Sim_1007e7b6', orig=0x1007e7b6, c='Sim_1007e7b6', gen=g_e7b6, trials=500),
]
