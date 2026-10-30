# Differential tests for round 7, letter m (see difftest.py and docs/checker.md).
#
# Helpers of the DLL that are not converted yet (or too deep to run on generated data, like the C library stream
# routines, malloc's heap core and the line drawers) are replaced in the emulator by a stub: a code hook at the
# helper's own address appends ('stub_<address>', arguments) to the checker's call log (compared between the
# original and the C run like the Windows calls), may write results through its arguments (the same bytes for both
# runs), answers with a value chosen by the generator and returns to the caller by setting EIP and ESP (code bytes
# are not patched). Stack addresses are logged as 'stack'; for some stubs the text or the bytes an argument points
# to are logged too. Converted callees run as the original code (original side) and as their C conversion (C side).
# Indirect calls (the plugin's GetProcAddress results, the pixel source of the LZW coder) go to small hooked
# routines at IND + 0x10 * k. Every function here is checked bit for bit (no float tolerance): the x87 control
# word of the game (0x27f, 53-bit precision) is set before each run.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_1003/w7m_collide.c', 'sim/unsorted_1003/w7m_state.c', 'sim/unsorted_1003/w7m_misc.c',
    'sim/unsorted_1004/w7m_gif.c', 'sim/unsorted_1004/w7m_load.c', 'sim/unsorted_1004/w7m_misc.c',
    'sim/unsorted_1005/w7m_misc.c', '../tools/verify/w7m_test.c',
    # existing converted code the functions above call (directly or further down)
    'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_1003/w3s_entry.c', 'sim/unsorted_100f/w6i_mem.c',
    'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_1007/w3p_net.c', 'sim/unsorted_1003/w2e_misc.c',
    'sim/util/field.c', 'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1002/w2j_nodes.c',
    'sim/unsorted_1002/w3q_collide.c', 'sim/unsorted_1002/w3o_ray.c', 'sim/unsorted_1002/w6d_calls.c',
    'sim/unsorted_1003/entry.c', 'sim/unsorted_1003/w3r_mesh.c', 'sim/unsorted_1003/w2f_misc.c',
    'sim/unsorted_1003/w6e_misc.c', 'sim/unsorted_1003/w4v_misc.c', 'sim/unsorted_1003/w5b_calls.c',
    'sim/unsorted_1003/w6e_state.c', 'sim/unsorted_1003/records.c', 'sim/unsorted_1004/w3p_net.c',
    'sim/unsorted_1004/w6e_misc.c', 'sim/unsorted_1004/w3s_records.c', 'sim/unsorted_1004/w5z_calls.c',
    'sim/unsorted_1004/w2k_flags.c', 'sim/unsorted_1004/w6e_stream.c', 'sim/unsorted_1004/w5c_calls.c',
    'sim/unsorted_1004/w2j_misc.c', 'sim/unsorted_1004/w5b_calls.c', 'sim/unsorted_1004/w2j_strtab.c',
    'sim/unsorted_1004/w2k_hash.c', 'sim/unsorted_1006/w6e_misc.c', 'sim/unsorted_1007/w2g_stubs.c',
    'sim/unsorted_1008/w6f_display.c', 'sim/unsorted_1008/w3o_length.c', 'sim/view/objlist.c',
    'sim/unsorted_1009/w3p_box.c', 'sim/unsorted_1009/w6g_segment.c', 'sim/unsorted_1009/w3o_ray.c',
    'sim/unsorted_100a/w5x_calls.c', 'sim/unsorted_100a/w3t_misc.c', 'sim/unsorted_100a/w5z_calls.c',
    'sim/unsorted_100a/w5y_misc.c', 'sim/unsorted_100a/w5c_calls.c', 'sim/unsorted_100a/w6g_nodes.c',
    'sim/unsorted_100b/w6g_misc.c', 'sim/unsorted_100f/w6i_misc.c', 'sim/unsorted_100f/w2k_string.c',
    'sim/win/w4v_heap.c', 'sim/win/w4a_heap.c', 'sim/view/matrix.c', 'sim/view/w3r_matrix.c',
    'sim/unsorted_100f/w2h_mem.c', 'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_1006/w3q_quat.c',
    'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_1000/w2f_records.c', 'sim/unsorted_1000/w2k_misc.c',
    'sim/unsorted_1000/w3o_sphere.c', 'sim/unsorted_1000/w3s_model.c', 'sim/unsorted_1000/w2e_names.c',
    'sim/unsorted_1000/w5x_calls.c', 'sim/unsorted_1001/w5y_misc.c', 'sim/unsorted_1002/w5a_misc.c',
    'sim/unsorted_1002/w2e_tables.c', 'sim/unsorted_1002/w5z_calls.c', 'sim/unsorted_1002/w5y_misc.c',
    'sim/unsorted_1002/w2e_object.c', 'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1002/w2g_records.c',
    'sim/unsorted_1002/w4v_misc.c', 'sim/core/w2a_timers.c', 'sim/core/w2b_timers.c',
    'sim/unsorted_1002/w3s_cell.c', 'sim/unsorted_1003/w3q_grid.c', 'sim/unsorted_1003/w5c_calls.c',
    'sim/unsorted_1003/w2l_records.c', 'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1004/w2i_misc.c',
    'sim/unsorted_1004/w2h_record.c', 'sim/unsorted_1004/w2e_strings.c', 'sim/unsorted_1004/w5y_misc.c',
    'sim/unsorted_1004/w3o_misc.c', 'sim/unsorted_1004/state.c', 'sim/unsorted_1004/w5a_misc.c',
    'sim/unsorted_1004/w2l_values.c', 'sim/unsorted_1004/w4w_net.c', 'sim/unsorted_1004/fields.c',
    'sim/unsorted_1004/tables.c', 'sim/unsorted_1005/w3o_calls.c', 'sim/unsorted_1005/w2g_pool.c',
    'sim/unsorted_1006/w5a_math.c', 'sim/unsorted_1006/w5b_calls.c', 'sim/unsorted_1006/w5c_math.c',
    'sim/unsorted_1007/w5a_misc.c', 'sim/unsorted_1007/node_flag.c', 'sim/unsorted_1007/w2f_misc.c',
    'sim/unsorted_1007/w2e_state.c', 'sim/unsorted_1008/w3o_grid.c', 'sim/view/w2l_lists.c',
    'sim/unsorted_1008/cell_link.c', 'sim/view/w2d_view.c', 'sim/unsorted_1008/w3s_grid.c',
    'sim/unsorted_1008/w5y_misc.c', 'sim/unsorted_1008/w3q_place.c', 'sim/util/ring.c',
    'sim/unsorted_100a/w5a_misc.c', 'sim/unsorted_100a/w5b_calls.c', 'sim/unsorted_100a/w2i_misc.c',
    'sim/unsorted_100a/w2g_misc.c', 'sim/unsorted_100a/w2k_list.c', 'sim/render/w2n_polygon.c',
    'sim/unsorted_100b/w2h_records.c', 'sim/unsorted_100b/w2j_misc.c', 'sim/unsorted_100b/w5z_calls.c',
    'sim/unsorted_100b/w3o_hash.c', 'sim/unsorted_100b/w5c_calls.c', 'sim/unsorted_100b/w3s_array.c',
    'sim/unsorted_100b/fields.c', 'sim/unsorted_100b/w2l_records.c', 'sim/unsorted_100c/w5a_misc.c',
    'sim/unsorted_100d/w2i_table.c', 'sim/unsorted_100d/w5z_calls.c', 'sim/unsorted_100d/w5b_calls.c',
    'sim/unsorted_100d/w5x_calls.c', 'sim/unsorted_100d/w2j_nodes.c', 'sim/unsorted_1010/w2h_misc.c',
    'sim/unsorted_1010/w2i_crt.c', 'sim/unsorted_100d/nodes.c', 'sim/unsorted_100f/w2g_crt.c',
    'sim/unsorted_1000/w2e_mono.c', 'sim/unsorted_100b/w2e_strings.c', 'sim/view/viewrect.c',
    'sim/render/w2n_project.c', 'sim/unsorted_1000/w2l_objects.c', 'sim/unsorted_1000/w2g_fields.c',
    'sim/unsorted_1000/w2l_fields.c', 'sim/unsorted_1001/w2e_names.c', 'sim/unsorted_1001/w2j_misc.c',
    'sim/unsorted_1001/w2h_objects.c', 'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1001/w4u_cd.c',
    'sim/unsorted_1001/w2e_tables.c', 'sim/unsorted_1002/w2i_state.c', 'sim/unsorted_1002/w3o_vectors.c',
    'sim/unsorted_1002/w2l_tables.c', 'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1002/w4u_heap.c',
    'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1003/w5a_misc.c', 'sim/unsorted_1003/w3r_misc.c',
    'sim/unsorted_1003/setters.c', 'sim/unsorted_1008/w4w_display.c', 'sim/unsorted_1008/w4v_fade.c',
    'sim/unsorted_1008/w2g_rect.c', 'sim/unsorted_100a/w4u_misc.c', 'sim/unsorted_100b/w4v_wrappers.c',
    'sim/win/w4v_dll.c', 'sim/unsorted_100c/w2l_objects.c', 'sim/unsorted_100c/w3o_misc.c',
    'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_100d/w2i_select.c', 'sim/core/timer.c', 'sim/win/w4u_cdaudio.c',
    'sim/unsorted_1000/w2j_slots.c', 'sim/unsorted_1005/w2h_tables.c', 'sim/unsorted_100c/w2e_tables.c',
    'sim/unsorted_100c/w4v_timers.c', 'sim/unsorted_100d/w2f_records.c', 'sim/util/lookup.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
IND = 0x00F00100        # indirect-call routines (in the checker's return page, outside compared memory)
OUTF = 0x00AFF000       # st(0) results are stored here (argument memory, compared like all memory)
TRAMP = 0x00AF0000      # machine-code trampolines for the original's st(0) functions
ARENA = 0x00A80000      # blocks the malloc stub hands out (argument memory, compared)
HEAP_HEAD = 0x10159e38  # the C library's small-block region list (made empty, so free goes to HeapFree)

# helper address -> number of argument dwords logged
STUBS = {
    0x100ff030: 2,      # _flsbuf (character, stream)
    0x100fd850: 2,      # fopen (name, mode)
    0x100febc0: 4,      # fwrite (buffer, size, count, stream)
    0x100fef50: 2,      # the heap core of malloc (size, flag)
    0x1006ad98: 4, 0x100b8d6c: 6, 0x1010d180: 1, 0x10015d30: 4, 0x1006abf6: 3, 0x10011093: 1, 0x10011681: 1,
    0x1003be30: 0, 0x100766ca: 1, 0x100ef727: 7, 0x100e86a1: 7, 0x100aa742: 1, 0x10088a2d: 1, 0x100a9ba7: 1,
    0x10060681: 4, 0x100246b3: 12, 0x100d3830: 2,
    0x100ab0d6: 2, 0x1000e737: 12, 0x10038913: 3, 0x100035df: 5, 0x10001e1f: 5, 0x10002775: 5,
}
SARGS = {0x100fd850: (0, 1)}            # stub -> arguments whose text is logged
BARGS = {0x100febc0: (0, 1, 2)}         # stub -> (buffer argument, size argument, count argument): bytes logged
ST = {}


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def cstr(ctx, data, extra=8):
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rb(ctx, extra))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def dbits(d):
    return list(struct.unpack('<2I', struct.pack('<d', d)))


def rfl(ctx, lo=-1000.0, hi=1000.0):
    """An ordinary float (no NaN or infinity): values in a range, special small values, wide magnitudes."""
    r = ctx.rng
    c = r.random()
    if c < 0.6:
        return r.uniform(lo, hi)
    if c < 0.8:
        return r.choice([0.0, -0.0, 1.0, -1.0, 0.5, 2.0, 100.0, 140.0, 1e-7, 1e-8, 1000.0, -1000.0])
    return r.uniform(-1, 1) * 10 ** r.randint(-6, 6)


def read_text(uc, addr, limit=80):
    text = b''
    while len(text) < limit:
        c = bytes(uc.mem_read(addr + len(text), 1))
        if c == b'\0':
            break
        text += c
    return text


# ---- stubs and indirect-call routines ---------------------------------------------------------------------------
def _stub_cb(m, addr):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        nargs = STUBS[addr]
        raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
        name = 'stub_%x' % addr
        k = sum(1 for e in m.api_log if e[0] == name)       # number of earlier calls of this stub in the run
        args = []
        for i, v in enumerate(raw):
            if i in SARGS.get(addr, ()):
                args.append(read_text(uc, v))
            else:
                args.append('stack' if STACK_LO <= v <= STACK_HI else v)
        if addr in BARGS:
            b, s, c = BARGS[addr]
            n = min(raw[s] * raw[c], 0x400)
            args.append(bytes(uc.mem_read(raw[b], n)) if n else b'')
        m.api_log.append((name, tuple(args)))
        fx = ST['effect'].get(addr)
        if fx:
            fx(uc, k, raw)
        rets = ST['rets'].get(addr) or [0]
        v = rets[k % len(rets)]
        if callable(v):
            v = v(raw)
        uc.reg_write(UC_X86_REG_EAX, v & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


IND_ARGS = [0, 3, 1, 6]  # 0: the LZW pixel source, 1: GetFuncDesc(kind, id, text), 2: CheckFunc(0), 3: DirectPlay send


def _ind_cb(m):
    def cb(uc, address, size, ud):
        slot = (address - IND) >> 4
        esp = uc.reg_read(UC_X86_REG_ESP)
        na = IND_ARGS[slot]
        raw = struct.unpack('<%dI' % na, bytes(uc.mem_read(esp + 4, 4 * na))) if na else ()
        name = 'ind%d' % slot
        k = sum(1 for e in m.api_log if e[0] == name)
        m.api_log.append((name, tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)))
        fx = ST['ind_fx'].get(slot)
        if fx:
            fx(uc, k, raw)
        v = ST['ind_ret'].get(slot, lambda k: 0)(k)
        uc.reg_write(UC_X86_REG_EAX, v & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def ind(slot):
    return IND + 0x10 * slot


def begin(ctx):
    """Installs the stubs (once per machine), the x87 state of the game and clears the per-trial answers. Must be
    the first thing a generator does."""
    m = ctx.m
    if not getattr(m, '_w7m', False):
        m._w7m = True
        for addr in STUBS:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr), None, addr, addr)
        m.raw.mem_write(IND, b'\xc3' * 0x10 * len(IND_ARGS))
        m.raw.hook_add(UC_HOOK_CODE, _ind_cb(m), None, IND, IND + 0x10 * len(IND_ARGS) - 1)
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST.clear()
    ST['rets'] = {}
    ST['effect'] = {}
    ST['ind_fx'] = {}
    ST['ind_ret'] = {}
    return ctx.rng


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def tramp_st0(ctx, target, nargs, slot):
    """Machine code that calls target(args as dwords) and stores st(0) as a double at OUTF; returns its address."""
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\xdd\x1d' + struct.pack('<I', OUTF)       # fstp qword [OUTF]
    code += b'\x31\xc0\x5d\xc3'
    a = TRAMP + 0x100 * slot
    ctx.m.uc.mem_write(a, code)
    ctx.m.uc.mem_write(OUTF, bytes(8))
    return a


# ---- shared worlds ----------------------------------------------------------------------------------------------
def file_world(ctx):
    """The stream at [0x1015f344]: buffer pointer and count (negative now and then, so _flsbuf runs)."""
    r = ctx.rng
    f = newbuf(ctx, 0x20)
    w32(ctx, f, zbuf(ctx, 0x400))
    w32(ctx, f + 4, r.choice([0x300, 0x300, 3, 2, 1, 0, 0xffffffff]))
    w32(ctx, 0x1015f344, f)
    rets(0x100ff030, *[r.choice([0xffffffff, 0, 5, 0x41, 0x41, 0x41]) for _ in range(3)])
    return f


def heap_world(ctx):
    """An empty small-block region list (free goes to HeapFree) and malloc answers from the arena."""
    w32(ctx, HEAP_HEAD, HEAP_HEAD)
    w32(ctx, HEAP_HEAD + 0x10, 0, 0)
    w32(ctx, 0x1017e968, ctx.rng.choice([0, 1]))


# ---- 0x1004dd2a, 0x1004e719, 0x1004d980, 0x1004e04a: the GIF writer ---------------------------------------------
def g_putword(ctx):
    r = begin(ctx)
    file_world(ctx)
    return [r.choice([ctx.u32(), r.getrandbits(32), 0xffff, 0xff00, 0x1234])]


def g_e719(ctx):
    r = begin(ctx)
    file_world(ctx)
    return [newbuf(ctx, 0x14)]


def g_gif(ctx):
    r = begin(ctx)
    heap_world(ctx)
    f = file_world(ctx)
    name = cstr(ctx, bytes(r.choice(b'abc.gif') for _ in range(r.randrange(0, 12))))
    rets(0x100fd850, r.choice([f, f, f, 0]))
    rets(0x100febc0, *[r.choice([lambda a: a[2], lambda a: a[2], lambda a: a[2], 0, lambda a: a[2] - 1])
                       for _ in range(2)])
    rets(0x100fef50, r.choice([ARENA, ARENA + 0x40, 0]))
    w32(ctx, 0x1015f360, r.choice([0, 0, ARENA + 0x8000, ctx.u32()]))
    colors = r.choice([0, 0, 1, 2, 3, 4, 5, 16, 17, 255, 256, 257, 1000, 2048, r.randrange(0, 600)])
    bits = r.choice([0, 1, 2, 4, 7, 8, 9, r.randrange(-5, 20)])
    return [name, ctx.u32(), ctx.u32(), colors, bits & 0xffffffff]


def g_lzw(ctx):
    r = begin(ctx)
    heap_world(ctx)
    file_world(ctx)
    wb(ctx, 0x1015f230, rb(ctx, 0x110))
    w32(ctx, 0x1015f340, r.randrange(0, 0x100))
    w32(ctx, 0x1015f348, r.randrange(0, 9))
    for a in (0x1015f35c, 0x1015f358, 0x1015f354):
        w32(ctx, a, r.choice([0, 0, ctx.u32()]))
    ok = r.random() < 0.85
    rets(0x100fef50, *[ARENA + off if (ok or r.random() < 0.6) else 0 for off in (0, 0x1000, 0x5000)])
    rets(0x100febc0, *[r.choice([lambda a: a[2], lambda a: a[2], lambda a: a[2], 0]) for _ in range(3)])
    bits = r.choice([1, 2, 2, 3, 4, 5, 8, 8])
    c = r.random()
    if c < 0.1:
        n, alpha = 4200, 256        # long enough to fill the 12-bit table (restart with a clear code)
        bits = 8
    elif c < 0.2:
        n, alpha = r.randrange(500, 1500), 1 << bits
    else:
        n, alpha = r.choice([0, 1, 2, 3, 10, 50, r.randrange(0, 300)]), 1 << bits
    seq = [r.randrange(0, alpha) if r.random() < 0.8 else 0 for _ in range(n)]     # zeros repeat more often
    if r.random() < 0.05 and seq:
        seq[r.randrange(0, len(seq))] = r.choice([0x1ff, 0x12345, 0xfffffffe])
    seq.append(0xffffffff)
    ST['ind_ret'][0] = lambda k: seq[min(k, len(seq) - 1)]
    return [bits, ind(0)]




# ---- 0x100462c0, 0x100466b6, 0x100468c3: resource records ------------------------------------------------------
def name_tables(ctx):
    """The sorted table list at [0x1017d604] for Sim_100bda9f (as in spec_w6g.py); each hash table: name at +0x18,
    bucket count at +0, bucket array at +0x10, entries of 0x20 bytes (8-byte key, count at +8, next at +0x1c)."""
    r = ctx.rng
    names = sorted(set(bytes(r.choice(b'ABCD') for _ in range(4)) for _ in range(r.choice([1, 2, 3, 4]))))
    tables = []
    for nm in names:
        nb = r.choice([1, 2, 3, 5, 7])
        t = newbuf(ctx, 0x40)
        w32(ctx, t, nb)
        wb(ctx, t + 0x18, nm)
        buckets = zbuf(ctx, 4 * nb)
        w32(ctx, t + 0x10, buckets)
        keys = []
        for _ in range(r.choice([0, 1, 3, 6])):
            key = struct.pack('<2I', r.randrange(0, 50), r.randrange(0, 50))
            e = newbuf(ctx, 0x20)
            wb(ctx, e, key)
            k0, k1 = struct.unpack('<2i', key)
            v = k0 ^ k1
            b = abs(v) % nb * (1 if v >= 0 else -1)
            if b < 0:
                continue
            w32(ctx, e + 0x1c, r32(ctx.m.uc, buckets + 4 * b))
            w32(ctx, buckets + 4 * b, e)
            keys.append(key)
        tables.append((nm, t, keys))
    arr = zbuf(ctx, 4 * len(tables))
    w32(ctx, arr, *[t for _, t, _ in tables])
    lst = zbuf(ctx, 0x10)
    w32(ctx, lst + 4, len(tables))
    w32(ctx, lst + 8, arr)
    w32(ctx, 0x1017d604, lst)
    return tables


def resource_key(ctx, table_slot):
    """The key record: a signed word (-1: the data is freed with HeapFree) and the 8-byte name key at +2; the table
    name pointer at table_slot (Sim_100bb2b0)."""
    r = ctx.rng
    tables = name_tables(ctx)
    nm, t, keys = r.choice(tables)
    w32(ctx, table_slot, cstr(ctx, nm))
    key = r.choice(keys) if keys and r.random() < 0.7 else struct.pack('<2I', r.randrange(0, 50), r.randrange(0, 50))
    word = r.choice([0xffff, 0xffff, 0, 1, 7, 0x8000])
    return ctx.buf(16, struct.pack('<H', word) + key + rb(ctx, 6))


def resource_common(ctx, size, vals):
    r = ctx.rng
    data = ctx.buf(size, struct.pack('<%di' % (size // 4), *vals))
    rets(0x100b8d6c, data if r.random() < 0.9 else 0)
    w32(ctx, 0x1017b8f8, ctx.u32())
    for a in (0x1014cf54, 0x1014cf58, 0x1014cf5c):
        w32(ctx, a, ctx.u32())


def g_462c0(ctx):
    r = begin(ctx)
    key = resource_key(ctx, 0x1014cee8)
    resource_common(ctx, 0x20, [r.choice([0, 1, -1, 100, -100, 0x7fffffff, -0x80000000, 16777217,
                                          r.getrandbits(32) - (1 << 31), r.randrange(-70000, 70000)])
                                for _ in range(8)])
    return [key] + [newbuf(ctx, 4) for _ in range(7)]


def g_466b6(ctx):
    r = begin(ctx)
    key = resource_key(ctx, 0x1014ceec)
    vals = [r.getrandbits(32) - (1 << 31) for _ in range(13)]
    for _ in range(15):
        good = r.random() < 0.8
        vals += [r.randrange(0, 101) if good or r.random() < 0.7 else r.choice([-1, 101, 1000, -50]) for _ in range(4)]
    resource_common(ctx, 4 * len(vals), vals)
    return [key]


def node_world(ctx, key, name):
    """The hash buckets [0x1015dc30] of Sim_10027f33, sometimes with the node for (key, name), and the list heads."""
    r = ctx.rng
    tab = zbuf(ctx, 0x3f1 * 4)
    w32(ctx, 0x1015dc30, tab)
    s = key + sum(b - 256 if b >= 128 else b for b in name[:4])
    if key >= 0 and r.random() < 0.7:
        node = newbuf(ctx, 0x20)
        w16(ctx, node, key)
        w16(ctx, node + 2, r.choice([0, 1, 5]))
        wb(ctx, node + 4, name[:4])
        w32(ctx, node + 8, 0)
        w32(ctx, tab + 4 * (s % 0x3f1), node)
    for a in (0x109f13f0, 0x109f13f4):
        w32(ctx, a, r.choice([0, newbuf(ctx, 0x20)]))


def g_468c3(ctx):
    r = begin(ctx)
    word = r.choice([0xffff, 0xffff, 0, 1, 7, 300])
    key = ctx.buf(8, struct.pack('<H', word) + rb(ctx, 6))
    name = bytes(r.choice(b'ABab') for _ in range(4))
    w32(ctx, 0x1014cef0, cstr(ctx, name))
    node_world(ctx, word if word < 0x8000 else word - 0x10000, name)
    vals = [r.getrandbits(32) - (1 << 31) for _ in range(49)]
    resource_common(ctx, 4 * len(vals), vals)
    return [key, zbuf(ctx, 0x13 * 0x14), zbuf(ctx, 8)]


# ---- 0x1004ee50 -------------------------------------------------------------------------------------------------
def g_ee50(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0xa0)
    w32(ctx, obj, r.choice([1, 1, 5, 5, 9, 9, 0, 2, ctx.u32()]))
    block = newbuf(ctx, 0x600)
    w32(ctx, block, newbuf(ctx, 0x160))
    w16(ctx, block + 0x560, r.choice([0, 0x10, 0x400, 0x410, r.getrandbits(16)]))
    w16(ctx, block + 0x562, r.choice([0, 0x2000, r.getrandbits(16)]))
    w32(ctx, obj + 0x28, block if r.random() < 0.9 else 0)
    other = newbuf(ctx, 0xa0)
    name = r.choice([b'SA', b'SB', b'S', b'X13', b'X63', b'S13', b'A13', b'Z', b'', b'SA63', b'X1', b'X6'])
    w32(ctx, other + 0x90, cstr(ctx, name))
    w32(ctx, obj + 0x48, other)
    w32(ctx, obj + 0x90, r.choice([0, 4, 5, 0x104, 0x3005, ctx.u32()]))
    return [obj]


def g_ee50_null(ctx):
    begin(ctx)
    return [0]


# ---- float helpers 0x10047cf2, 0x10045308, 0x1004409f -----------------------------------------------------------
def matrix_obj(ctx):
    """An object whose matrix (at +0x4c) is up to date (flag 0x8000 at +0x94) and has a position at +0x70."""
    o = newbuf(ctx, 0xa0)
    wf(ctx, o + 0x4c, *[ctx.rng.uniform(-2, 2) for _ in range(12)])
    wf(ctx, o + 0x70, rfl(ctx), rfl(ctx), rfl(ctx))
    w32(ctx, o + 0x94, 0x8000 | ctx.rng.getrandbits(16))
    return o


def ef14_world(ctx):
    """The entry [0x1015ef14] that Sim_10044b2c reads (as in spec_w6e.py)."""
    r = ctx.rng
    e = newbuf(ctx, 0x80)
    wf(ctx, e + 0x58, *[rfl(ctx) for _ in range(8)])
    w32(ctx, e + 0x4c, matrix_obj(ctx) if r.random() < 0.7 else 0)
    w32(ctx, 0x1015ef14, e if r.random() < 0.9 else 0)
    if r.random() < 0.5:
        f = newbuf(ctx, 4)
        wf(ctx, f, rfl(ctx))
        w32(ctx, 0x1015ef1c, f)
    else:
        w32(ctx, 0x1015ef1c, 0)
    w32(ctx, 0x10179cd4, r.choice([0, 1]))


def mover(ctx, addr, now):
    """A record of Sim_10047bcd at addr: time, target, current, rate."""
    r = ctx.rng
    target = rfl(ctx)
    cur = target + r.choice([0.0, 1e-8, -1e-8, r.uniform(-50, 50), r.uniform(-5000, 5000)])
    w32(ctx, addr, (now - r.choice([0, 1, 5, 30, 1000, ctx.u32()])) & 0xffffffff)
    wf(ctx, addr + 4, target, cur, r.choice([1.0, 30.0, 100.0, 0.5, rfl(ctx), -10.0]))


def g_47cf2(ctx):
    r = begin(ctx)
    tramp_st0(ctx, 0x10047cf2, 1, 0)
    obj = newbuf(ctx, 0x14)
    now = ctx.u32()
    w32(ctx, 0x1015dc78, now)
    period = r.choice([360.0, 360.0, 6.2831855, 100.0, 1.0, -360.0, r.uniform(0.5, 1000)])
    target = r.choice([r.uniform(-2 * period, 2 * period), rfl(ctx), 0.0, period, period / 2, -period / 2])
    cur = r.choice([target, target + r.uniform(-period, period), r.uniform(-3 * period, 3 * period), rfl(ctx),
                    target + period / 2, target - period / 2])
    w32(ctx, obj, (now - r.choice([0, 1, 5, 30, 1000, r.randrange(0, 100000), ctx.u32()])) & 0xffffffff)
    wf(ctx, obj + 4, target, cur, r.choice([1.0, 30.0, 100.0, 0.5, 1000.0, rfl(ctx), -10.0]), period)
    return [obj]


def cam_world(ctx):
    ef14_world(ctx)
    now = ctx.u32()
    w32(ctx, 0x1015dc78, now)
    mover(ctx, 0x109b9b90, now)
    mover(ctx, 0x109b9ae0, now)
    wf(ctx, 0x1015fd9c, rfl(ctx))
    wf(ctx, 0x1015fd98, rfl(ctx))


def g_45308(ctx):
    begin(ctx)
    cam_world(ctx)
    ptrs = [newbuf(ctx, 4) for _ in range(6)]
    for p in ptrs:
        wf(ctx, p, rfl(ctx))
    return ptrs


def g_4409f(ctx):
    r = begin(ctx)
    cam_world(ctx)
    w32(ctx, 0x1013aec4, r.choice([10, 10, 3, 1, 0, 0x7fffffff, 0xffffffff]))
    cam = newbuf(ctx, 0x20)
    wf(ctx, cam, *[rfl(ctx) for _ in range(8)])
    w32(ctx, 0x1014cae0, cam)
    for a in (0x1015ed58, 0x1015ee90, 0x1015ed30, 0x1015ed68, 0x1015ed18, 0x1015ed48):
        wf(ctx, a, rfl(ctx))
    for i in range(10):
        w32(ctx, 0x1015ed70 + 0x1c * i, *[fbits(rfl(ctx)) for _ in range(6)])
        w32(ctx, 0x1015ed70 + 0x1c * i + 0x18, r.choice([0, 1, 90, 181, 362, -5, r.randrange(-100000, 100000)]))
    return [r.choice([0, 0, 0, 1, 2, 5, 9, 10, 11, -1, 0x7fffffff, 0x80000000]) & 0xffffffff]


# ---- 0x10051a36, 0x1005346f -------------------------------------------------------------------------------------
def g_51a36(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    wb(ctx, 0x101659e0, rb(ctx, 0x200))
    length = r.choice([0, 1, 2, 3, 4, 5, 7, 8, 13, 16, 31, -1, -4, r.randrange(-10, 60)])
    height = r.choice([0, -1, 1, 5, 40, r.randrange(-50, 200)])
    return [ctx.u32(), r.randrange(-50, 700) & 0xffffffff, r.randrange(-50, 500) & 0xffffffff, length & 0xffffffff,
            height & 0xffffffff, r.choice([0, 1, 2, 3, 7, 100, 255, 0x80])]


SPACEY = bytes([32, 32, 9, 10, 13, 11, 12, 97, 98, 49, 46, 95, 0xa0, 0xff, 0x85])


def g_trim(ctx):
    r = begin(ctx)
    multi = r.random() < 0.25
    w32(ctx, 0x1015965c, 2 if multi else 1)
    if r.random() < 0.05:
        return [0]
    # with [0x1015965c] > 1 the class comes from Crt_IsCType; its GetStringTypeA path (characters above 0x7f) reads
    # the type word that the stub cannot write (on the stack), so those characters are only used with the table
    alpha = SPACEY[:12] if multi else SPACEY
    text = bytes(r.choice(alpha) for _ in range(r.choice([0, 1, 2, 3, 5, 10, r.randrange(0, 40)])))
    return [cstr(ctx, text)]



# ---- 0x10039540, 0x1003ca04, 0x1003c5a0, 0x100381d0, 0x100361d8 --------------------------------------------------
DSERR = [0x8878000a, 0x8878001e, 0x80070057, 0x88780032, 0x80004005, 0x88780046, 0x8007000e, 0x88780064, 0x80004001,
         0x88780078, 0x88780082, 0x80040110, 0x88780096, 0x887800a0, 0x887800aa]


def g_dserr(ctx):
    r = begin(ctx)
    c = r.random()
    if c < 0.6:
        return [r.choice(DSERR)]
    if c < 0.8:
        return [(r.choice(DSERR) + r.choice([-1, 1, 2, -2, 0x10])) & 0xffffffff]
    return [r.choice([0, 1, 0x7fffffff, 0x80000000, 0xffffffff, 0x8878000b, 0x887800ab, 0x88780000, r.getrandbits(32)])]


def g_ca04(ctx):
    r = begin(ctx)
    hdr = newbuf(ctx, 0x10)
    w16(ctx, hdr + 6, r.choice([0, 1, 2, 0xffff, 0x8000]))
    w32(ctx, 0x109e88e0, hdr)
    w32(ctx, 0x109b9e08, r.choice([0, 1, 1, 2]))
    w32(ctx, 0x1015eb98, r.choice([0, 1, 2, 2, 2, 3, 0xffffffff, 0x7fffffff]))
    now = r.choice([r.randrange(0, 1 << 31), r.randrange(0, 100000), 0, 0xb5 * 7, 0x80000000, 0xffffffff])
    w32(ctx, 0x1015dc78, now)
    w32(ctx, 0x109b9df8, r.choice([0, now, now + 1, now - 1, ctx.u32()]))
    w32(ctx, 0x1013abd4, r.choice([0, 3, 100, r.randrange(-1000, 1000)]))
    w32(ctx, 0x1013abc8, r.choice([24, 24, 7, 1, 100, 0x7fffffff]))
    w32(ctx, 0x1013abd0, r.choice([0, 5, 12, 23, 30, r.randrange(-5, 40)]))
    w32(ctx, 0x1015ebb8, r.choice([0, 1, 5, ctx.u32()]))
    w32(ctx, 0x1013abcc, r.choice([4, 7, 1, 3]))
    w32(ctx, 0x1015eba0, *sorted(r.randrange(-2, 26) for _ in range(4)))
    return []


def g_c5a0(ctx):
    begin(ctx)
    return [cstr(ctx, b'hgsim.dll')]


def g_381d0(ctx):
    r = begin(ctx)
    ST['m'] = ctx.m
    if r.random() < 0.05:
        return [0]
    rec = newbuf(ctx, 0x10)
    w32(ctx, rec + 4, r.choice([2, 2, 1, 0, 3]))
    w32(ctx, rec + 8, r.choice([0, 0x80000, 0x10080000, ctx.u32()]))
    w32(ctx, 0x1017b8f0, ctx.u32())
    if r.random() < 0.2:
        w32(ctx, 0x1015e688, 0)
    else:
        p = newbuf(ctx, 4)
        w32(ctx, p, r.choice([0, 0, 1]))
        w32(ctx, 0x1015e688, p)
    for a in (0x109e3780, 0x109e3784, 0x109e3788, 0x109e378c):
        w32(ctx, a, r.randrange(-100, 2000))
    w32(ctx, 0x1017b92c, r.choice([0, 1, 1]))
    rets(0x100766ca, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x109e3d14, r.choice([640, 800, 1024, 0xffffffff]))
    w32(ctx, 0x109e3d18, r.choice([480, 600, 768, 0xffffffff]))
    ST['metrics'] = {0: r.choice([640, 800, 1024, 1280]), 1: r.choice([480, 600, 768, 1024])}
    ST['cursor'] = {1: r.choice([[-3, -2, -1, 0], [0], [-1, 0], [-5, 3]]), 0: r.choice([[2, 1, 0, -1], [-1], [0, -1], [7, -4]])}
    ST['time'] = ctx.u32()
    return [rec]


def a_showcursor(n, args):
    k = sum(1 for e in ST['m'].api_log if e[0] == 'ShowCursor') - 1
    seq = ST['cursor'][args[0] & 1]
    return seq[min(k, len(seq) - 1)] & 0xffffffff


API_381D0 = {
    'ShowCursor': a_showcursor,
    'GetSystemMetrics': lambda n, args: ST['metrics'].get(args[0], 0),
    'timeGetTime': lambda n, args: ST['time'],
}


def fx_desc(uc, k, raw):
    """GetFuncDesc(kind, id, text): writes the kind text, an id and a description."""
    uc.mem_write(raw[0], ST['kind'] + b'\0')
    uc.mem_write(raw[1], struct.pack('<I', ST['id']))
    uc.mem_write(raw[2], ST['text'] + b'\0')


def g_361d8(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015e56c, r.choice([0, 0, 0, 0, 1]))
    idx = r.choice([0, 1, 2, 3])
    w32(ctx, 0x1015e574, idx)
    lst = zbuf(ctx, 0x124 * 5)
    name = r.choice([b'c:\\hg\\drivers\\soft.dll', b'soft.dll', b'.\\d3d.dll', b'a/b/c.dll', b'', b'\\x'])
    ST['module'] = r.choice([0x400000, 0x400000, 0x400000, 0])
    ST['procs'] = {0x1013a0b4: ind(1) if r.random() < 0.9 else 0, 0x1013a0c8: ind(2) if r.random() < 0.7 else 0}
    ST['kind'] = r.choice([b'RASTER', b'RASTER', b'raster', b'Raster', b'RASTER2', b'AUDIO', b''])
    ST['id'] = ctx.u32()
    ST['text'] = bytes(r.choice(b'abc XYZ-1') for _ in range(r.randrange(0, 60)))
    ST['ind_fx'][1] = fx_desc
    d = r.choice([0, 1, 1, 7])
    c = r.choice([0, 1, 1, 0x100])
    ST['ind_ret'][1] = lambda k: d
    ST['ind_ret'][2] = lambda k: c
    return [cstr(ctx, name), lst]


API_361D8 = {
    'LoadLibraryA': lambda n, args: ST['module'],
    'GetProcAddress': lambda n, args: ST['procs'].get(args[1], 0),
}


# ---- collision tree 0x10031acd, 0x10031f95 ----------------------------------------------------------------------
def small(r):
    """Coordinates from a small grid so that equalities between vertices, bounds and the query happen often."""
    return float(r.choice([-2, -1, 0, 1, 2, 3])) if r.random() < 0.8 else r.uniform(-3, 3)


def coll_poly(ctx, nv):
    """A polygon record (0x28 bytes and its vertex index list): count +2, list offset +4, normal +0x18..+0x20."""
    r = ctx.rng
    n = r.choice([3, 3, 4, 4, 5])
    rec = ctx.buf(0x28 + 16)
    wb(ctx, rec + 0x28, bytes([r.randrange(0, nv) for _ in range(n)] + [0]))
    w16(ctx, rec + 2, n)
    w32(ctx, rec + 4, 0x28)
    wf(ctx, rec + 0x18, r.uniform(-1, 1), r.choice([1.0, 0.5, 0.0, -0.5, 1e-3, r.uniform(-1, 1)]), r.uniform(-1, 1))
    return rec


def coll_node(ctx, depth, nv):
    r = ctx.rng
    node = zbuf(ctx, 0x2c + 4 * 6)
    x0, x1 = sorted([small(r), small(r)])
    y0, y1 = sorted([small(r), small(r)])
    z0, z1 = sorted([small(r), small(r)])
    wf(ctx, node, x0 - r.choice([0, 1]), x1 + r.choice([0, 1]), y0 - r.choice([0, 1, 3]), y1 + r.choice([0, 1, 3]),
       z0 - r.choice([0, 1]), z1 + r.choice([0, 1]))
    if depth > 0 and r.random() < 0.6:
        w32(ctx, node + 0x1c, *[coll_node(ctx, depth - 1, nv) if r.random() < 0.75 else 0 for _ in range(4)])
    else:
        n = r.choice([1, 1, 2, 3, 6])
        w32(ctx, node + 0x18, n)
        w32(ctx, node + 0x2c, *[coll_poly(ctx, nv) for _ in range(n)])
    return node


def coll_world(ctx):
    r = ctx.rng
    nv = r.choice([3, 4, 6, 9])
    mesh = ctx.buf(0x18 + nv * 0x58 + 0x20)
    for i in range(nv):
        wf(ctx, mesh + 0x18 + i * 0x58 + 0xc, small(r), small(r), small(r))
    root = coll_node(ctx, r.choice([0, 1, 2, 2, 3]), nv) if r.random() < 0.95 else 0
    if root and r.random() < 0.5:
        wf(ctx, root, -4.0, 4.0)            # a root that holds most queries in x and z
        wf(ctx, root + 0x10, -4.0, 4.0)
    return root, mesh


def coll_query(r):
    q = [small(r), small(r) * r.choice([1, 1, 3]), small(r)]
    if r.random() < 0.04:
        q[r.randrange(0, 3)] = float('nan')
    return [fbits(v) for v in q]


def g_31acd(ctx):
    r = begin(ctx)
    root, mesh = coll_world(ctx)
    return [root, mesh] + coll_query(r)


def g_31f95(ctx):
    r = begin(ctx)
    root, mesh = coll_world(ctx)
    return [root, mesh] + coll_query(r) + [newbuf(ctx, 4)]


# ---- collision records 0x100339e8, 0x10033aeb, 0x10033eb3 -------------------------------------------------------
def model_block(ctx, n):
    r = ctx.rng
    blk = newbuf(ctx, 0x18 + 0x58 * max(n, 0) + 0x40)
    w16(ctx, blk + 4, n)
    w16(ctx, blk + 6, 0)
    w32(ctx, blk + 8, 0x18 + 0x58 * max(n, 0))
    for i in range(max(n, 0)):
        wf(ctx, blk + 0x18 + 0x58 * i, *[r.uniform(-1000, 1000) for _ in range(6)])
    return blk


def game_obj(ctx, kind=None):
    """An object with a model block (+0x2c, +0x30), its matrix holder (+0x28), type bits (+2) and values +0x44..."""
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    w16(ctx, obj + 2, kind if kind is not None else r.choice([0xd0, 0xe0, 0xd5, 0xe3, 0x10, 0, 0xf0]))
    blk = model_block(ctx, r.choice([0, 1, 2, 3, 5, 8]))
    w32(ctx, obj + 0x2c, blk)
    w32(ctx, obj + 0x30, blk if r.random() < 0.9 else 0)
    stamp = ctx.u32()
    w32(ctx, blk + 0x10, stamp)
    w32(ctx, obj + 0x9c, stamp if r.random() < 0.7 else ctx.u32())
    t = newbuf(ctx, 0x100)
    wf(ctx, t + 0x4c, *[r.uniform(-2, 2) for _ in range(12)])
    w32(ctx, obj + 0x28, t)
    wf(ctx, obj + 0x44, rfl(ctx), rfl(ctx), rfl(ctx))
    return obj


def g_339e8(ctx):
    r = begin(ctx)
    rec = newbuf(ctx, 0x44)
    obj = game_obj(ctx)
    return [rec if r.random() < 0.97 else 0, obj if r.random() < 0.97 else 0]


def g_33aeb(ctx):
    r = begin(ctx)
    lists = []
    for _ in range(r.choice([1, 1, 2, 3])):
        head = zbuf(ctx, 0x20)
        w32(ctx, head + 8, 1)
        prev = head
        for _ in range(r.choice([0, 1, 2, 4, 8, 30, 120])):
            o = game_obj(ctx, r.choice([0xd0, 0xe0, 0xd1, 0xe7, 0x10, 0]))
            if r.random() < 0.03:
                w32(ctx, o + 0x30, 0)
            w32(ctx, prev + 0x10, o)
            prev = o
        w32(ctx, prev + 0x10, 0)
        lists.append(head)
    for i in range(9):
        for j in range(9):
            w32(ctx, 0x1017ba4c + i * 0x18c + j * 0x2c, 0)
    if r.random() < 0.5 or len(lists) == 1:
        w32(ctx, 0x1017c8c4, 0)
        w32(ctx, 0x1017c8cc, lists[0])
    else:
        w32(ctx, 0x1017c8c4, 1)
        w32(ctx, 0x1017c8c8, lists[0])
        for h in lists[1:]:
            w32(ctx, 0x1017ba4c + r.randrange(0, 9) * 0x18c + r.randrange(0, 9) * 0x2c, h)
    return []


def g_33eb3(ctx):
    r = begin(ctx)
    n = r.choice([0, 1, 2, 5, 20, 20, 60, 99, 120])
    w32(ctx, 0x109e4cc0, n)
    for i in range(n):
        e = 0x109e4cc4 + 0x44 * i
        w32(ctx, e + 0x10, r.choice([0, 0, 0, 1]))
        wf(ctx, e + 0x20, small(r) * 5, small(r) * 5, small(r) * 5)
    rad = r.choice([0.0, 1.0, 5.0, 10.0, 10.0, 30.0, 100.0, -5.0, r.uniform(0, 60)])
    q = [small(r) * 5, small(r) * 5, small(r) * 5]
    if r.random() < 0.05:
        q[r.randrange(0, 3)] = float('nan')
    return [fbits(q[0]), fbits(q[1]), fbits(q[2]), fbits(rad),
            newbuf(ctx, 4) if r.random() < 0.95 else 0]


# ---- object state 0x10036cc0, 0x10036f25, 0x1003705a, 0x10038dd3 -------------------------------------------------
def obj_tree(ctx, depth):
    """A node with children (+4 first, +8 next) and a record at +0x8c (as in spec_w5x.py), plus position floats."""
    r = ctx.rng
    node = zbuf(ctx, 0x100)
    wf(ctx, node + 0x30, rfl(ctx), rfl(ctx), rfl(ctx))
    wf(ctx, node + 0x70, rfl(ctx), rfl(ctx), rfl(ctx))
    w32(ctx, node + 0x94, r.choice([0, 1, 0x8000, 0x4001]))
    if r.random() < 0.6:
        obj = zbuf(ctx, 0x100)
        w16(ctx, obj, r.choice([0, 0x800, 0x1000, 0x1800, 0x8000, 0xffff]))
        w32(ctx, obj + 0x34, r.choice([4, 0, 1]))
        w32(ctx, node + 0x8c, obj)
    kids = []
    if depth > 0:
        kids = [obj_tree(ctx, depth - 1) for _ in range(r.choice([0, 1, 2]))]
    if kids:
        w32(ctx, node + 4, kids[0])
        for a, b in zip(kids, kids[1:]):
            w32(ctx, a + 8, b)
    return node


def ring_world(ctx):
    r = ctx.rng
    wb(ctx, 0x101cffc0, rb(ctx, 0x200))
    wb(ctx, 0x101cfce0, struct.pack('<128I', *[r.choice([r.randrange(0, 200), r.getrandbits(32),
                                                          r.randrange(-100, 100) & 0xffffffff]) for _ in range(128)]))
    w32(ctx, 0x1017cd38, r.randrange(0, 0x7f))
    w32(ctx, 0x1017cd3c, r.randrange(0, 0x7f))


def state_world(ctx):
    """A state record (0x600 bytes) whose first dword is a game object (+0x48 its node tree)."""
    r = ctx.rng
    w32(ctx, 0x1017c8c4, r.choice([0, 0, 1]))
    w32(ctx, 0x1017c8cc, 0)
    o = newbuf(ctx, 0x180)
    w32(ctx, o + 0x48, obj_tree(ctx, r.choice([0, 1, 2])))
    w32(ctx, o + 0x54, newbuf(ctx, 0x50))
    wf(ctx, o + 0x58, *[rfl(ctx, -400, 400) for _ in range(6)])
    s = newbuf(ctx, 0x600)
    w32(ctx, s, o)
    for off in (0x24, 0x34, 0x74):
        now = ctx.u32()
        w32(ctx, s + off, now - r.choice([0, 1, 30]))
        wf(ctx, s + off + 4, rfl(ctx), rfl(ctx), r.choice([1.0, 30.0, 0.5]))
    wf(ctx, s + 0x520, rfl(ctx))
    wf(ctx, s + 0x4d4, rfl(ctx, -50, 50))
    w32(ctx, s + 0x4f0, r.choice([2, 2, 2, 0, 1, 3, 4, 4, 5]))
    w32(ctx, s + 0x4d8, r.choice([0, 0, 5, ctx.u32()]))
    w32(ctx, 0x1015dc78, ctx.u32())
    w32(ctx, 0x1015dc80, r.choice([0, 1, 18, 40, r.randrange(-50, 1000)]))
    ring_world(ctx)
    return s, o


def g_36cc0(ctx):
    r = begin(ctx)
    s, o = state_world(ctx)
    obj = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x28, s if r.random() < 0.95 else 0)
    return [obj]


def g_36f25(ctx):
    r = begin(ctx)
    s, o = state_world(ctx)
    return [s if r.random() < 0.97 else 0]


def g_3705a(ctx):
    r = begin(ctx)
    s, o = state_world(ctx)
    ident = r.randrange(0, 4)
    w32(ctx, o + 8, ident)
    sub = r.randrange(0, 4)
    w32(ctx, 0x104727a0 + 4 * ident, sub)
    rec = 0x1047383a + ident * 0x4fc3a + sub * 0x13f
    w32(ctx, rec + 2, r.choice([0x100, 0x100, 0x100, 0, 0x101]))
    wb(ctx, rec + 0xee, bytes([r.choice([1, 2, 0, 0x80]), r.randrange(0, 8), r.choice([1, 1, 0, 2])]))
    for k in range(8):
        wf(ctx, 0x109a8658 + k * 0x54, rfl(ctx, -400, 400), rfl(ctx, -400, 400), rfl(ctx, -400, 400))
    c = r.random()
    if c < 0.25:
        # target exactly `speed` away from the node position: the reach test compares equal values
        node = r32(ctx.m.uc, o + 0x48)
        wf(ctx, node + 0x70, 0.0, 0.0, 0.0)
        tick = r32(ctx.m.uc, 0x1015dc80)
        tick = tick - (1 << 32) if tick >= 1 << 31 else tick
        f = struct.unpack('<f', bytes(ctx.m.uc.mem_read(s + 0x4d4, 4)))[0]
        c47 = struct.unpack('<f', bytes(ctx.m.uc.mem_read(0x1011132c, 4)))[0]
        speed = struct.unpack('<f', struct.pack('<f', tick * f / c47))[0]
        k = rb(ctx, 1)[0] & 7
        vec = [0.0, 0.0, 0.0]
        vec[r.randrange(0, 3)] = r.choice([speed, -speed])
        wf(ctx, 0x109a8658 + k * 0x54, *vec)
        wb(ctx, rec + 0xef, bytes([k]))
    elif c < 0.3:
        wf(ctx, s + 0x4d4, float('nan'))
    return [s]


def g_38dd3(ctx):
    r = begin(ctx)
    s, o = state_world(ctx)
    node = r32(ctx.m.uc, o + 0x48)
    wf(ctx, node + 0x70, small(r) * 300, small(r) * 300, small(r) * 300)
    prev = 0
    ents = []
    for _ in range(r.choice([0, 1, 2, 3, 6])):
        e = newbuf(ctx, 0x6a0)
        wf(ctx, e + 0x680, small(r) * 300, small(r) * 300, small(r) * 300 if r.random() < 0.97 else float('nan'))
        w32(ctx, e + 0x124, r.choice([0, 1, 2, 5]))
        ents.append(e)
    for a, b in zip(ents, ents[1:] + [0]):
        w32(ctx, a + 0x690, b)
    w32(ctx, 0x109e3760, ents[0] if ents else 0)
    out = [newbuf(ctx, 4) for _ in range(3)]
    return out + [s, r.choice([-1, -1, 0, 1, 2, 5, 7]) & 0xffffffff, r.choice([0, 0, 1])]



# ---- network packets 0x100410ff, 0x10040191 ---------------------------------------------------------------------
def roster_world(ctx):
    """The ids of the session (Sim_1003dbc0): enabled flag, root [0x1015d004] with count +0x12, key words +0x16 and
    0x1e-byte records +0x126 (word id at +0x1a); the DirectPlay send routine [0x1015eab4] goes to IND slot 3."""
    r = ctx.rng
    w32(ctx, 0x1015ec84, r.choice([1, 1, 1, 0]))
    root = zbuf(ctx, 0x140)
    n = r.choice([0, 1, 3, 8])
    w32(ctx, root + 0x12, n)
    keys = newbuf(ctx, 0x20)
    w32(ctx, root + 0x16, keys + 2)
    recs = zbuf(ctx, 0x1e * 8 + 0x20)
    ids = [r.randrange(0, 10) for _ in range(n)]
    for i, v in enumerate(ids):
        w16(ctx, recs + 0x1e * i + 0x1a, v)
    w32(ctx, root + 0x126, recs)
    w32(ctx, 0x1015d004, root)
    w32(ctx, 0x1015eab4, ind(3))
    w32(ctx, 0x1015ecd4, ctx.u32())
    w16(ctx, 0x1013acac, ctx.u32())
    w32(ctx, 0x1015ecd8, ctx.u32())
    ST['ind_ret'][3] = lambda k: k + 7
    return ids


def g_410ff(ctx):
    r = begin(ctx)
    ids = roster_world(ctx)
    me = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, me)
    for i in range(16):
        if r.random() < 0.15:
            w32(ctx, 0x109e8b40 + 4 * i, 0)
            continue
        o = newbuf(ctx, 0x40)
        w32(ctx, o + 0x14, r.choice([3, 3, 3, 1, 0]))
        w32(ctx, o + 0xc, r.choice([me, r.choice(ids) if ids else 5, 0xffffffff, r.randrange(0, 10), 0x80000000]))
        w32(ctx, 0x109e8b40 + 4 * i, o)
    w32(ctx, 0x1015ecac, newbuf(ctx, 0x10))
    pkt = newbuf(ctx, 0x10)
    wb(ctx, pkt + 2, bytes([r.randrange(0, 16)]))
    w16(ctx, pkt + 3, r.choice([0x4200, 0x4200, r.randrange(0, 16), 0x4100 + r.randrange(0, 16), ctx.u32()]))
    return [pkt]


def g_40191(ctx):
    r = begin(ctx)
    me = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, me)
    w32(ctx, 0x1015dc64, r.choice([8, 8, 4, 0x100, 0]))
    w32(ctx, 0x1017c8c4, 0)
    w32(ctx, 0x1017c8cc, 0)
    for i in range(10):
        if r.random() < 0.1:
            w32(ctx, 0x109e8b40 + 4 * i, 0)
            continue
        o = newbuf(ctx, 0x160)
        w32(ctx, o + 4, r.randrange(0, 8))
        w32(ctx, o + 0x14, r.choice([3, 1, 0]))
        w32(ctx, o + 0xc, r.choice([me, r.randrange(0, 10)]))
        w16(ctx, o + 0x1c, r.choice([0, 0, 0, 2, 4, 1, 8]))
        s = newbuf(ctx, 0x100)
        g = newbuf(ctx, 0x80)
        node = obj_tree(ctx, r.choice([0, 1, 2]))
        rec = r32(ctx.m.uc, node + 0x8c)
        if rec:
            w16(ctx, rec + 0x26, r.randrange(0, 19))
        w32(ctx, g + 0x48, node)
        w32(ctx, s, g)
        ents = newbuf(ctx, 0x3c * 17)
        for k in range(17):
            w32(ctx, ents + 0x3c * k + 0x38, r.choice([0, 1, 3, 100, -7, 0x7fffffff, -0x80000000, r.getrandbits(32)]))
            w16(ctx, ents + 0x3c * k + 0x36, r.choice([0, 0x2000, r.getrandbits(16)]))
        w32(ctx, s + 0x88, ents)
        w32(ctx, o + 0x28, s)
        w32(ctx, 0x109e8b40 + 4 * i, o)
    pkt = newbuf(ctx, 0x20)
    wb(ctx, pkt + 2, bytes([r.choice([r.randrange(0, 10), r.randrange(0, 10), 0xff])]))
    return [pkt]



# ---- 0x100386d5 -------------------------------------------------------------------------------------------------
def name_table(ctx):
    """The table of names at 0x10a39200 (count [0x10a38de0]) used by Sim_1000df9c (as in spec_w6d.py)."""
    rg = ctx.rng
    w32(ctx, 0x10a38de0, rg.choice([0, 1, 2, 3, 5, 8]))
    for i in range(12):
        nm = rg.choice([b'AFXHDWPS', b'abc', b'', b'AFX'])
        used = rg.choice([0, 1, 1, 2])
        ctx.m.uc.mem_write(0x10a39200 + i * 0x14, struct.pack('<I', used) + nm + b'\0' + rb(ctx, 15 - len(nm)))


def fx_celltest(uc, k, raw):
    """A ground test routine of Sim_1002d961: writes a height to its out pointer."""
    uc.mem_write(raw[4], struct.pack('<f', ST['heights'][k % len(ST['heights'])]))


def named_tree(ctx, depth, names):
    """Nodes (child +4, sibling +8) with a name at +0x90, a value at +0, a record at +0x8c and a position at +0x70."""
    r = ctx.rng
    node = zbuf(ctx, 0x100)
    w32(ctx, node, r.choice([0, ctx.u32() | 1, ctx.u32() | 1, ctx.u32() | 1]))
    w32(ctx, node + 0x90, r.choice(names))
    w32(ctx, node + 0x8c, r.choice([0, newbuf(ctx, 0x40)]))
    wf(ctx, node + 0x30, rfl(ctx), rfl(ctx), rfl(ctx))
    wf(ctx, node + 0x70, small(r) * 10, small(r) * 10, small(r) * 10)
    w32(ctx, node + 0x94, r.choice([0, 1, 0x8000]))
    kids = [named_tree(ctx, depth - 1, names) for _ in range(r.choice([0, 1, 2]))] if depth > 0 else []
    if kids:
        w32(ctx, node + 4, kids[0])
        for a, b in zip(kids, kids[1:]):
            w32(ctx, a + 8, b)
    return node


def all_nodes(ctx, node):
    out = []
    while node:
        out.append(node)
        out += all_nodes(ctx, r32(ctx.m.uc, node + 4))
        node = r32(ctx.m.uc, node + 8)
    return out


def g_386d5(ctx):
    r = begin(ctx)
    names = [cstr(ctx, n) for n in (b'WRF', b'WRF', b'xWBZ', b'xWBZ', b'ABC', b'WB', b'aWRFb', b'')]
    root = named_tree(ctx, r.choice([0, 1, 2, 3]), names)
    nodes = all_nodes(ctx, root)
    w32(ctx, 0x1017c8c4, 0)
    g = newbuf(ctx, 0x60)
    w32(ctx, g + 0x48, root)
    me = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, me)
    w32(ctx, g + 4, r.choice([me, me + 1]))
    w32(ctx, g + 0x14, r.choice([3, 1]))
    w32(ctx, g + 0xc, r.choice([me, 7]))
    obj = newbuf(ctx, 0x700)
    w32(ctx, obj, g)
    w32(ctx, obj + 0x490, r.choice([1, 1, 1, 1, 1, 0]))
    tab = newbuf(ctx, 0x128 * 3)
    for i in range(3):
        t = tab + 0x128 * i
        w32(ctx, t + 0x118, r.choice([0, 1, 2]))
        start = r.choice([0, 0, 1, 2])
        w32(ctx, t + 0x108, start)
        w32(ctx, t + 0x38, start + r.choice([0, 1, 2, 3, -1]))
        w32(ctx, t + 0x3c, *[r.randrange(0, 6) for _ in range(6)])
    w32(ctx, obj + 0x84, tab)
    w32(ctx, obj + 0x5fb, r.randrange(0, 3))
    w32(ctx, obj + 0x5ff, ctx.u32())
    w32(ctx, obj + 0x8c, newbuf(ctx, 0x24 * 6))
    # Sim_100ab151: the node and value it hands back
    w32(ctx, 0x101c64f4, r.choice(nodes) if r.random() < 0.9 else r.choice([0, newbuf(ctx, 0x100)]))
    w32(ctx, 0x101c6504, r.choice([0, ctx.u32() | 1, ctx.u32() | 1, ctx.u32() | 1]))
    # Sim_1002d961: a cell with nodes of the ground test types
    cell = zbuf(ctx, 0x40)
    prev = cell
    for _ in range(r.choice([0, 1, 2, 3])):
        n = zbuf(ctx, 0x40)
        w32(ctx, n + 0x34, r.choice([5, 8, 0, 7]))
        w32(ctx, prev + 0x18, n)
        prev = n
    w32(ctx, 0x1017c8cc, cell if r.random() < 0.9 else 0)
    ST['heights'] = [r.choice([small(r) * 10, -5000.0, 1e6, 0.0]) for _ in range(3)]
    for a in (0x100035df, 0x10001e1f, 0x10002775):
        ST['effect'][a] = fx_celltest
        rets(a, *[r.choice([0, 1, 1]) for _ in range(3)])
    # the list at [0x109e3760] (never empty: Sim_10038fac links the new entry behind its head)
    ents = []
    for _ in range(r.choice([1, 1, 2, 3])):
        e = newbuf(ctx, 0x6a0)
        wf(ctx, e + 0x680, small(r) * 10, small(r) * 10, small(r) * 10)
        w32(ctx, e, r.choice([ctx.u32()] + nodes))
        ents.append(e)
    for a, b in zip(ents, ents[1:] + [0]):
        w32(ctx, a + 0x690, b)
    w32(ctx, 0x109e3760, ents[0])
    name_table(ctx)
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    w32(ctx, 0x1015ecb8, newbuf(ctx, 0x10))
    w32(ctx, 0x1015eab4, ind(3))
    ST['ind_ret'][3] = lambda k: k + 7
    return [obj, r.choice([0xffffffff, 0xffffffff, 0, 1, 2]), r.choice([0, 1, 1, 2])]


API_386D5 = {'HeapAlloc': lambda n, args: ARENA + 0x1000 * n}



# ---- 0x1003387d -------------------------------------------------------------------------------------------------
def ground_world(ctx):
    """Sim_1002d961: one cell list ([0x1017c8c4] = 0) with nodes whose ground tests are stubbed."""
    r = ctx.rng
    w32(ctx, 0x1017c8c4, 0)
    cell = zbuf(ctx, 0x40)
    prev = cell
    for _ in range(r.choice([0, 1, 2, 3])):
        n = zbuf(ctx, 0x40)
        w32(ctx, n + 0x34, r.choice([5, 8, 0, 7]))
        w32(ctx, prev + 0x18, n)
        prev = n
    w32(ctx, 0x1017c8cc, cell if r.random() < 0.9 else 0)
    ST['heights'] = [r.choice([small(r) * 10, -5000.0, 1e6, 0.0]) for _ in range(3)]
    for a in (0x100035df, 0x10001e1f, 0x10002775):
        ST['effect'][a] = fx_celltest
        rets(a, *[r.choice([0, 1, 1]) for _ in range(3)])


def g_3387d(ctx):
    r = begin(ctx)
    ground_world(ctx)
    obj = newbuf(ctx, 0x60)
    holder = newbuf(ctx, 0x40)
    w32(ctx, holder + 0x30, r.choice([1, ctx.u32() | 1, 0]) if r.random() < 0.95 else 0)
    w32(ctx, obj + 0x18, holder if r.random() < 0.95 else 0)
    c = [small(r) * 10 for _ in range(3)]
    wf(ctx, obj + 0x20, *c)
    if r.random() < 0.6:        # a box around the centre: the segment goes through it
        lo = [c[i] - r.choice([0.0, 1.0, 5.0, 20.0]) for i in range(3)]
        hi = [c[i] + r.choice([0.0, 1.0, 5.0, 20.0]) for i in range(3)]
    else:
        lo = [r.choice([-40.0, -30.0, -10.0, 0.0, small(r) * 10]) for _ in range(3)]
        hi = [l + r.choice([0.0, 5.0, 20.0, 60.0, 100.0]) for l in lo]
    wf(ctx, obj + 0x2c, lo[0], hi[0], lo[1], hi[1], lo[2], hi[2])
    v = [c[i] + r.choice([0.0, small(r) * 10, r.uniform(-50, 50)]) for i in range(3)]
    vec = ctx.buf(12, struct.pack('<3f', *v))
    back = r.choice([0.0, 1.0, 5.0, -2.0, r.uniform(0, 30)])
    return [obj if r.random() < 0.97 else 0, vec if r.random() < 0.97 else 0, fbits(back),
            newbuf(ctx, 12) if r.random() < 0.97 else 0]


SPECS = [
    dict(name='Stream_PutWord', orig=0x1004dd2a, c='Stream_PutWord', gen=g_putword, trials=500),
    dict(name='Sim_1004e719', orig=0x1004e719, c='Sim_1004e719', gen=g_e719, trials=500),
    dict(name='Gif_Begin', orig=0x1004d980, c='Gif_Begin', gen=g_gif, trials=600),
    dict(name='Lzw_Compress', orig=0x1004e04a, c='Lzw_Compress', gen=g_lzw, trials=400),
    dict(name='Sim_100462c0', orig=0x100462c0, c='Sim_100462c0', gen=g_462c0, trials=500),
    dict(name='Sim_100466b6', orig=0x100466b6, c='Sim_100466b6', gen=g_466b6, trials=400),
    dict(name='Sim_100468c3', orig=0x100468c3, c='Sim_100468c3', gen=g_468c3, trials=400),
    dict(name='Sim_1004ee50', orig=0x1004ee50, c='Sim_1004ee50', gen=g_ee50, trials=600),
    dict(name='Sim_1004ee50_null', orig=0x1004ee50, c='Sim_1004ee50', gen=g_ee50_null, trials=10, void=True),
    dict(name='Sim_10047cf2', orig=TRAMP, c='Test_10047cf2', gen=g_47cf2, trials=1500),
    dict(name='Sim_10045308', orig=0x10045308, c='Sim_10045308', gen=g_45308, trials=500, void=True),
    dict(name='Sim_1004409f', orig=0x1004409f, c='Sim_1004409f', gen=g_4409f, trials=600, void=True),
    dict(name='Sim_10051a36', orig=0x10051a36, c='Sim_10051a36', gen=g_51a36, trials=500, void=True),
    dict(name='Str_TrimSpace', orig=0x1005346f, c='Str_TrimSpace', gen=g_trim, trials=600),
    dict(name='Sound_ErrorName', orig=0x10039540, c='Sound_ErrorName', gen=g_dserr, trials=500),
    dict(name='Sim_1003ca04', orig=0x1003ca04, c='Sim_1003ca04', gen=g_ca04, trials=800, void=True),
    dict(name='Sim_1003c5a0', orig=0x1003c5a0, c='Sim_1003c5a0', gen=g_c5a0, trials=300,
         api={'GetModuleHandleA': lambda n, args: 0x10000000}),
    dict(name='Sim_100381d0', orig=0x100381d0, c='Sim_100381d0', gen=g_381d0, trials=600, void=True, api=API_381D0),
    dict(name='Sim_100361d8', orig=0x100361d8, c='Sim_100361d8', gen=g_361d8, trials=600, void=True, api=API_361D8),
    dict(name='Sim_10031acd', orig=0x10031acd, c='Sim_10031acd', gen=g_31acd, trials=2000),
    dict(name='Sim_10031f95', orig=0x10031f95, c='Sim_10031f95', gen=g_31f95, trials=2000),
    dict(name='Sim_100339e8', orig=0x100339e8, c='Sim_100339e8', gen=g_339e8, trials=500),
    dict(name='Sim_10033aeb', orig=0x10033aeb, c='Sim_10033aeb', gen=g_33aeb, trials=300),
    dict(name='Sim_10033eb3', orig=0x10033eb3, c='Sim_10033eb3', gen=g_33eb3, trials=600),
    dict(name='Sim_10036cc0', orig=0x10036cc0, c='Sim_10036cc0', gen=g_36cc0, trials=500, void=True),
    dict(name='Sim_10036f25', orig=0x10036f25, c='Sim_10036f25', gen=g_36f25, trials=500, void=True),
    dict(name='Sim_1003705a', orig=0x1003705a, c='Sim_1003705a', gen=g_3705a, trials=800, void=True),
    dict(name='Sim_10038dd3', orig=0x10038dd3, c='Sim_10038dd3', gen=g_38dd3, trials=800, void=True),
    dict(name='Sim_100410ff', orig=0x100410ff, c='Sim_100410ff', gen=g_410ff, trials=800, void=True),
    dict(name='Sim_10040191', orig=0x10040191, c='Sim_10040191', gen=g_40191, trials=600, void=True),
    dict(name='Sim_100386d5', orig=0x100386d5, c='Sim_100386d5', gen=g_386d5, trials=1200, api=API_386D5),
    dict(name='Sim_1003387d', orig=0x1003387d, c='Sim_1003387d', gen=g_3387d, trials=1500),
]
