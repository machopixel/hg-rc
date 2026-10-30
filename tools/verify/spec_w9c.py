# Differential tests for round 9, letter c (see difftest.py and docs/checker.md).
#
# Helpers are replaced in the emulator by a code hook at their first byte (the code bytes are not patched): the
# hook appends ('stub_<address>', arguments, extra data) to the checker's call log, which must be identical for
# the original and the C run (stack addresses are logged as 'stack', texts by content), answers with a value that
# depends only on the trial and the call position and returns to the caller. Unconverted or patched helpers (the
# C calls them with SIM_FN) are stubbed at their original address; converted helpers whose own world is large
# (allocators, message senders, the resource cache) are stubbed at their original address AND at their C symbol
# under the same log name, so each function here is checked on its own logic (the helpers are proven by their own
# specs). A stub is only active in the trials whose generator asks for it (begin(ctx, addresses...)).
# Run for real on both sides: Mem_Fill, Mem_Move, Str_Length, Str_Copy, Str_CopyN, Str_FindLast, Crt_StrStr,
# Crt_SPrintf (with the DLL's own _output), Crt_StrUpper, Sim_10047aa0, Sim_1003b485,
# Sim_100a9b7f, Sim_100a9d47, Sim_1008d405 and Sim_1003b986 (this letter). The font height 0x100ecce0 (reads the
# font's dword +8) is stubbed: its `push es` / `pop es` leaves the emulator with a broken stack pointer.
# Everything here is integer code: no float tolerance is used.
#
# Coverage: a few addresses inside the originals are counted (COVER) and printed at exit, so the report can say
# which branches the trials reached.
import atexit
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW,
                               UC_X86_REG_FPSW, UC_X86_REG_FPTAG)

MINE = [
    'sim/unsorted_1002/w9c_record.c', 'sim/unsorted_1003/w9c_slots.c', 'sim/unsorted_1003/w9c_plugin.c',
    'sim/unsorted_1003/w9c_surface.c', 'sim/unsorted_100b/w9c_cache.c', 'sim/unsorted_1005/w9c_keys.c',
    'sim/unsorted_1006/w9c_event.c', 'sim/unsorted_1006/w22_mounts.c', 'sim/unsorted_1007/w9c_messages.c', 'sim/unsorted_1009/w9c_tree.c',
    'sim/unsorted_100b/w9c_misc.c', 'sim/unsorted_100c/w9c_misc.c', 'sim/unsorted_100c/w9c_hud.c',
    'sim/unsorted_100d/w9c_hud.c',
]
SOURCES = MINE + [
    # the converted code the functions above call, and what those files need to link (the transitive
    # closure of the undefined symbols, computed with nm over src/sim)
    'sim/unsorted_100f/w8z_crt.c', 'sim/unsorted_100f/w8A_stream.c', 'sim/unsorted_100f/w7q_stream.c',
    'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_1010/w7s_locale.c', 'sim/unsorted_100f/w6i_mem.c',
    'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_1003/w8u_plugin.c', 'sim/unsorted_1003/w6e_misc.c',
    'sim/unsorted_1002/w5a_misc.c', 'sim/unsorted_1002/w5c_calls.c', 'sim/core/w2a_timers.c',
    'sim/unsorted_1003/slots.c', 'sim/unsorted_1004/w3p_calc.c', 'sim/unsorted_1008/w2h_misc.c',
    'sim/unsorted_1008/w7o_cells.c', 'sim/unsorted_100a/w6g_nodes.c', 'sim/unsorted_100a/w2i_misc.c',
    'sim/unsorted_100a/w5y_misc.c', 'sim/util/field.c', 'sim/unsorted_100a/w5z_calls.c',
    'sim/unsorted_100b/w8y_cache.c', 'sim/unsorted_100b/w7p_misc.c', 'sim/unsorted_100b/w3o_hash.c',
    'sim/unsorted_100b/w6g_misc.c', 'sim/unsorted_100c/w7q_objects.c', 'sim/unsorted_100f/w2j_str.c',
    'sim/unsorted_100f/w2g_crt.c', 'sim/unsorted_100f/w6i_stream.c', 'sim/unsorted_100b/w2f_tables.c',
    'sim/unsorted_100f/w7q_find.c', 'sim/unsorted_1010/w8A_lowio.c', 'sim/unsorted_100f/w2k_string.c',
    'sim/win/w4v_heap.c', 'sim/view/matrix.c', 'sim/unsorted_100f/w2h_mem.c', 'sim/unsorted_100b/w7p_ptrarray.c',
    'sim/unsorted_100b/w3s_array.c', 'sim/unsorted_1000/w2f_records.c', 'sim/unsorted_1000/w2k_misc.c',
    'sim/unsorted_1000/w3s_model.c', 'sim/unsorted_1001/w5y_misc.c', 'sim/unsorted_1002/w6d_calls.c',
    'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1002/w2l_tables.c',
    'sim/unsorted_1002/w2g_records.c', 'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1003/w4u_heap.c',
    'sim/unsorted_1003/w5a_misc.c', 'sim/unsorted_1003/w5c_calls.c', 'sim/unsorted_1003/w7m_misc.c',
    'sim/unsorted_1003/w2l_records.c', 'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1003/w4w_records.c',
    'sim/unsorted_1005/w4v_misc.c', 'sim/unsorted_1005/w3o_calls.c', 'sim/unsorted_1006/w2e_copy.c',
    'sim/unsorted_1006/w5y_misc.c', 'sim/unsorted_1006/w5a_math.c', 'sim/view/objlist.c',
    'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_1008/w6f_cells.c', 'sim/view/w2l_lists.c', 'sim/view/w2d_view.c',
    'sim/unsorted_1008/w3s_grid.c', 'sim/view/w2l_rects.c', 'sim/unsorted_100a/w5x_calls.c',
    'sim/unsorted_100a/w5b_calls.c', 'sim/render/w2n_polygon.c', 'sim/util/lookup.c',
    'sim/unsorted_100b/w2h_records.c', 'sim/unsorted_100b/w5z_calls.c', 'sim/unsorted_100b/w8y_pack.c',
    'sim/unsorted_100b/w5c_calls.c', 'sim/unsorted_100b/w4w_buffers.c', 'sim/unsorted_100b/w3p_misc.c',
    'sim/unsorted_100b/fields.c', 'sim/unsorted_100b/w2l_records.c', 'sim/unsorted_100c/w5a_misc.c',
    'sim/unsorted_100c/w3o_misc.c', 'sim/unsorted_100d/w5z_calls.c', 'sim/unsorted_100d/w5b_calls.c',
    'sim/unsorted_100d/w2j_nodes.c', 'sim/unsorted_100d/w4v_misc.c', 'sim/unsorted_1010/w6j_double.c',
    'sim/unsorted_1010/w6j_stream.c', 'sim/unsorted_1010/w7s_stdio.c', 'sim/unsorted_1010/w2h_misc.c',
    'sim/unsorted_1010/w6k_stream.c', 'sim/unsorted_1010/w2k_crt.c', 'sim/unsorted_1010/w6k_misc.c',
    'sim/unsorted_1010/w2i_crt.c', 'sim/unsorted_1010/w6k_text.c', 'sim/unsorted_100f/w6i_misc.c',
    'sim/unsorted_1010/w2g_crt.c', 'sim/core/timer.c', 'sim/unsorted_1010/w6i_crt.c', 'sim/win/w4a_heap.c',
    'sim/unsorted_1000/w2e_mono.c', 'sim/unsorted_100b/w2e_strings.c', 'sim/unsorted_1003/w2e_misc.c',
    'sim/view/viewrect.c', 'sim/render/w2n_project.c', 'sim/unsorted_1000/w5z_calls.c',
    'sim/unsorted_1000/w3o_sphere.c', 'sim/unsorted_1000/w2l_objects.c', 'sim/unsorted_1000/w2g_fields.c',
    'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1000/w2l_fields.c', 'sim/unsorted_1001/w2j_misc.c',
    'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1001/w4u_cd.c', 'sim/unsorted_1001/w2e_tables.c',
    'sim/unsorted_1002/w5y_misc.c', 'sim/unsorted_1002/w2e_tables.c', 'sim/unsorted_1002/w5z_calls.c',
    'sim/unsorted_1002/w4v_misc.c', 'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1003/w3r_misc.c',
    'sim/unsorted_1003/setters.c', 'sim/unsorted_1003/w5b_calls.c', 'sim/unsorted_1003/w6e_state.c',
    'sim/unsorted_1004/w3p_net.c', 'sim/unsorted_1004/w4w_net.c', 'sim/unsorted_1004/tables.c',
    'sim/unsorted_1005/w2g_pool.c', 'sim/unsorted_1007/w5a_misc.c', 'sim/unsorted_1007/w2e_state.c',
    'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1008/cell_link.c', 'sim/unsorted_1008/w4w_display.c',
    'sim/unsorted_1008/w4v_fade.c', 'sim/unsorted_1008/w2g_rect.c', 'sim/util/ring.c',
    'sim/unsorted_100a/w4u_misc.c', 'sim/unsorted_100a/w2k_list.c', 'sim/unsorted_100b/w4v_wrappers.c',
    'sim/unsorted_100b/w2j_misc.c', 'sim/win/w4v_dll.c', 'sim/unsorted_100c/w2l_objects.c',
    'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_1010/w2f_mantissa.c', 'sim/unsorted_1010/w6k_handles.c',
    'sim/unsorted_1010/w2f_misc.c', 'sim/win/w4u_cdaudio.c', 'sim/unsorted_1003/w2e_mono.c',
    'sim/unsorted_1007/w3p_net.c', 'sim/unsorted_1000/w2e_names.c', 'sim/unsorted_1000/w2j_slots.c',
    'sim/unsorted_1002/w2e_object.c', 'sim/unsorted_1003/records.c', 'sim/unsorted_1004/w2i_misc.c',
    'sim/unsorted_1004/w2h_record.c', 'sim/unsorted_1006/w5b_calls.c', 'sim/unsorted_1007/w2f_misc.c',
    'sim/unsorted_1009/w3o_ray.c', 'sim/unsorted_100c/w2e_tables.c', 'sim/unsorted_100c/w4v_timers.c',
    'sim/unsorted_100d/w5x_calls.c', 'sim/unsorted_100d/w2f_records.c', 'sim/unsorted_1010/w2f_ld12.c',
    'sim/unsorted_100d/nodes.c', 'sim/unsorted_1001/w2e_names.c', 'sim/unsorted_1001/w2h_objects.c',
    'sim/unsorted_1002/w3o_vectors.c', 'sim/unsorted_100d/w2i_select.c', 'sim/unsorted_1005/w2h_tables.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
HANDLE = 0x00A90000        # an opaque resource handle the stubs hand out (never dereferenced)

ST = {'seed': 0, 'active': set()}
HITS = {}


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<H', v & 0xffff))


def w8(ctx, addr, v):
    ctx.m.uc.mem_write(addr, bytes([v & 0xff]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def cstr(ctx, data, extra=8):
    return ctx.buf(len(data) + 1 + extra, bytes(data) + b'\0' + rb(ctx, extra))


def text(ctx, alphabet, n):
    return bytes(ctx.rng.choice(alphabet) for _ in range(n))


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


# ---- stubs --------------------------------------------------------------------------------------------------
class Stub:
    def __init__(self, nargs, pol, csym=None):
        self.nargs, self.pol, self.csym = nargs, pol, csym


def pv(tag, options):
    """Answer from options by (trial, call number); a list in ST['v<tag>'] overrides."""
    return lambda uc, n, a: (pick(mix(ST['seed'], tag, n), ST.get('v%x' % tag) or options), [])


def cstring(uc, addr, maxn=0x60):
    out = b''
    while len(out) < maxn:
        c = bytes(uc.mem_read(addr + len(out), 1))
        if c == b'\0':
            break
        out += c
    return list(out)


def p_str(tag, options, which=(0,)):
    """pv, logging the texts behind the arguments `which` by content."""
    def pol(uc, n, a):
        extra = []
        for i in which:
            extra.append(tuple(cstring(uc, a[i])) if a[i] else ())
        return pick(mix(ST['seed'], tag, n), ST.get('v%x' % tag) or options), extra
    return pol


def p_list(key):
    """Answer n of the list ST[key] (the last one again when it runs out)."""
    def pol(uc, n, a):
        lst = ST[key]
        return lst[min(n, len(lst) - 1)], []
    return pol


STREAM = 0x00A7F000        # a FILE pointer the fopen stubs hand out (only ever passed on)
CODE = 0x00AFF000          # a `ret` (written by the generator) that stands in for the fixup callback of Cache_LoadItem


def p_fill(uc, n, a):
    """fread(buf, size, count, stream): deterministic bytes into buf (nothing for a null buffer); returns count."""
    size = min((a[1] * a[2]) & 0xffffffff, 0x400)
    if a[0]:
        uc.mem_write(a[0], bytes((mix(ST['seed'], n, i // 4) >> (8 * (i % 4))) & 0xff for i in range(size)))
    return a[2], []


def p_lines(uc, n, a):
    """fgets(buf, n, stream): line n of ST['lines'] into buf, returns buf; 0 when none is left."""
    lines = ST['lines']
    if n < len(lines):
        uc.mem_write(a[0], lines[n] + b'\0')
        return a[0], [lines[n]]
    return 0, []


def p_probe(uc, n, a):
    """Plugin_ProbeRaster(path, arg): logs the path, leaves a device count in [0x1015e56c]."""
    uc.mem_write(0x1015e56c, struct.pack('<I', pick(mix(ST['seed'], 0x6027, n), [0, 0, 1, 2, 7])))
    return 0, [tuple(cstring(uc, a[0])) if a[0] else ()]


def p_size_out(tag, key, out):
    """A loader (name, ext, &size): logs both texts, writes a size through argument `out`, answers ST[key][n]."""
    def pol(uc, n, a):
        lst = ST[key]
        val = lst[min(n, len(lst) - 1)]
        size = pick(mix(ST['seed'], tag, n), [0, 1, 0x10, 0x100, 0x3ff, 0x400])
        uc.mem_write(a[out], struct.pack('<I', size))
        return val, [tuple(cstring(uc, a[0])), tuple(cstring(uc, a[1])), size]
    return pol


def p_strtok(uc, n, a):
    """strtok(buf, delimiters) in place, like the original, but without its static next-token pointer (the
    original keeps a pointer into the caller's stack frame at 0x1017e884, which cannot agree between the runs)."""
    buf = bytes(cstring(uc, a[0], 0x100))
    delims = bytes(cstring(uc, a[1], 0x10))
    start = 0
    while start < len(buf) and buf[start] in delims:
        start += 1
    if start == len(buf):
        return 0, [tuple(buf), ()]
    end = start
    while end < len(buf) and buf[end] not in delims:
        end += 1
    if end < len(buf):
        uc.mem_write(a[0] + end, b'\0')
    return a[0] + start, [tuple(buf), tuple(buf[start:end])]


def p_type(uc, n, a):
    """Sim_100bda9f(table, ext, cmp): logs the extension text, answers the trial's type record."""
    return ST['type'][0], [tuple(cstring(uc, a[1]))]


def p_fixup(uc, n, a):
    """The fixup callback (block, &size): may change the size; answers ST['fixup'][n] (a replacement or 0)."""
    extra = []
    if pick(mix(ST['seed'], 0xf1, n), [0, 0, 1]):
        size = pick(mix(ST['seed'], 0xf2, n), [0, 5, 0x80, 0x1000])
        uc.mem_write(a[1], struct.pack('<I', size))
        extra.append(size)
    lst = ST['fixup']
    return lst[min(n, len(lst) - 1)], extra


STUBS = {
    # ---- unconverted or patched (the C calls them through SIM_FN)
    0x100ba7d5: Stub(3, p_str(0xa7d5, [0, HANDLE, HANDLE + 0x10, HANDLE + 0x20], (0, 1))),   # resource load
    0x100ff860: Stub(3, p_str(0xf860, [0], (0, 1))),                                        # _assert
    0x100bca49: Stub(3, p_size_out(0xca49, 'packed', 2)),                                   # pack loader (OFF)
    # ---- the fixup callback of Cache_LoadItem (an argument)
    CODE: Stub(2, p_fixup),
    # ---- converted C library and cache code, stubbed on both sides
    0x100fd850: Stub(2, p_str(0xd850, [STREAM, STREAM, STREAM, 0], (0, 1)), 'Sim_100fd850'),   # fopen
    0x100feb20: Stub(3, pv(0xeb20, [0]), 'Crt_FSeek'),
    0x100fe970: Stub(1, p_list('sizes'), 'Crt_FTell'),
    0x100fef30: Stub(1, p_list('blocks'), 'Heap_Malloc'),
    0x100fe830: Stub(4, p_fill, 'Crt_FRead'),
    0x100fd760: Stub(1, pv(0xd760, [0]), 'Crt_FClose'),
    0x100fefe0: Stub(1, pv(0xefe0, [0]), 'Heap_FreeBlock'),
    0x100fe770: Stub(3, p_lines, 'Stream_ReadLine'),                                         # fgets
    0x100baf3f: Stub(3, p_size_out(0xaf3f, 'cached', 2), 'Sim_100baf3f'),
    0x100bda9f: Stub(3, p_type, 'Sim_100bda9f'),
    0x100fd560: Stub(2, p_strtok, 'Str_NextToken_100fd560'),
    0x100b9b3f: Stub(4, p_str(0x9b3f, [0], (1, 2)), 'Sim_100b9b3f'),
    0x100bae0a: Stub(2, p_str(0xae0a, [STREAM, STREAM, 0], (0, 1)), 'Sim_100bae0a'),
    0x10035f90: Stub(1, pv(0x5f90, [0, 1, 3, 0x12345]), 'Plugin_ProbeAll'),
    0x10036027: Stub(2, p_probe, 'Plugin_ProbeRaster'),
    0x100f0205: Stub(5, pv(0x0205, [0])),                                                   # shape blit (hud)
    0x100e91dc: Stub(5, pv(0x91dc, [0])),
    0x100f2f01: Stub(6, p_str(0x2f01, [0], (4,))),                                          # text
    0x100f1e10: Stub(6, p_str(0x1e10, [0], (4,))),
    0x100ecea6: Stub(6, p_str(0xcea6, [0], (4,))),
    0x100ecce0: Stub(1, pv(0xcce0, [0, 1, 2, 7, 8, 13, 16, 0xffffffff])),                 # font height
    # ---- converted, stubbed on both sides
    0x1002ab9d: Stub(0, p_list('ticks'), 'Sim_1002ab9d'),
    0x100203cf: Stub(2, pv(0x03cf, [0]), 'Sim_100203cf'),
    0x100a9830: Stub(2, p_list('nodes'), 'Sim_100a9830'),
    0x100a9b8d: Stub(2, p_str(0x9b8d, [0], (1,)), 'Sim_100a9b8d'),
    0x10029ab3: Stub(2, pv(0x9ab3, [0]), 'Sim_10029ab3'),
    0x100aa0da: Stub(5, pv(0xa0da, [0]), 'Sim_100aa0da'),
    0x100ccb0d: Stub(2, p_str(0xcb0d, [0, 5, 17, 100, 0xffffffe2, 640], (0,)), 'Sim_100ccb0d'),
    0x100bb2b0: Stub(2, p_str(0xb2b0, [0, 1, 1], (0, 1)), 'Sim_100bb2b0'),
    0x1008d4f1: Stub(2, pv(0xd4f1, [0]), 'Sim_1008d4f1'),
}

# addresses inside the originals whose hits are counted (branch coverage of the trials)
COVER = {
    'b986 key found': 0x1003b9c3,
    'd2d ext table': 0x10057d69, 'd2d set bit': 0x10057d8f, 'd2d clear bit': 0x10057dc6,
    'd2d shift': 0x10057e28, 'd2d ctrl': 0x10057e64, 'd2d alt': 0x10057ea1,
    'bf33 switch': 0x1006bfba, 'bf33 closed': 0x1006c09a, 'bf33 code mismatch': 0x1006c0c6,
    'e0ff timer running': 0x1007e14e, 'e0ff font ok': 0x1007e173, 'e0ff shape ok': 0x1007e198,
    'e0ff text 2f01': 0x1007e208, 'e0ff text 1e10': 0x1007e234, 'e0ff text cea6': 0x1007e257,
    'e0ff expired': 0x1007e281,
    'd90e new object': 0x1009d981, 'd90e alloc failed': 0x1009d9b7, 'd90e POF owner': 0x1009da8c,
    'd90e child link': 0x1009daaa, 'd90e detach': 0x1009dac4,
    'ccc60 single scan': 0x100ccccd, 'ccc60 entry': 0x100cce2b, 'ccc60 kind1': 0x100cce75,
    'ccc60 kind2 single': 0x100cce88, 'ccc60 kind2': 0x100cce93, 'ccc60 kind4': 0x100cce9e,
    'ccc60 kind8': 0x100ccea8, 'ccc60 kind other': 0x100cceb3, 'ccc60 short': 0x100ccfbd,
    'ccc60 long': 0x100cd0a2, 'ccc60 cut at space': 0x100cd0fb, 'ccc60 mark 5': 0x100cd379,
    'ccc60 mark 6': 0x100cd490, 'ccc60 mark other': 0x100cd5a8,
    'd3d10 colour 0xf0': 0x100d3dd9, 'd3d10 colour 0x14': 0x100d3dd0, 'd3d10 colour 4': 0x100d3de2,
    'd3d10 colour default': 0x100d3deb, 'd3d10 inner 0': 0x100d3da3, 'd3d10 inner 1': 0x100d3dac,
    'd3d10 inner 2': 0x100d3db5, 'd3d10 inner 3': 0x100d3dbe, 'd3d10 inner 4': 0x100d3dc7,
    'd3d10 selected': 0x100d3e28, 'd3d10 box shape': 0x100d3e6f, 'd3d10 kind -1 shape': 0x100d3efb,
    'd3d10 kind shape': 0x100d3fb6, 'd3d10 shape at pos': 0x100d3f04, 'd3d10 count text': 0x100d410f,
    'd3d10 frame': 0x100d41d5, 'd3d10 no font': 0x100d4207,
    '363af no list file': 0x100363e2, '363af token': 0x1003642a, '363af closed': 0x1003644e,
    'bb405 cached': 0x100bb44c, 'bb405 file': 0x100bb49a, 'bb405 file no memory': 0x100bb4f3,
    'bb405 file fixup': 0x100bb53e, 'bb405 file replaced': 0x100bb584, 'bb405 pack': 0x100bb5f0,
    'bb405 pack none': 0x100bb616, 'bb405 pack fixup': 0x100bb627, 'bb405 pack replaced': 0x100bb66d,
    'bb405 path': 0x100bb6ef, 'bb405 path none': 0x100bb836, 'bb405 path no memory': 0x100bb748,
    'bb405 path fixup': 0x100bb793, 'bb405 path replaced': 0x100bb7da,
    '5aec row': 0x10035b2b,
}


def install(m):
    if getattr(m, '_w9c', False):
        return
    m._w9c = True

    def make(addr, st):
        name = 'stub_%x' % addr

        def cb(uc, address, size, ud):
            if addr not in ST['active']:
                return
            esp = uc.reg_read(UC_X86_REG_ESP)
            raw = list(struct.unpack('<%dI' % st.nargs, bytes(uc.mem_read(esp + 4, 4 * st.nargs)))) if st.nargs else []
            n = sum(1 for e in m.api_log if e[0] == name)
            val, extra = st.pol(uc, n, raw)
            m.api_log.append((name, tuple(logged(v) for v in raw), tuple(extra)))
            uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
            uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
            uc.reg_write(UC_X86_REG_ESP, esp + 4)
        return cb

    for addr, st in STUBS.items():
        m.raw.hook_add(UC_HOOK_CODE, make(addr, st), None, addr, addr)
        if st.csym:
            c = m.syms[st.csym]
            m.raw.hook_add(UC_HOOK_CODE, make(addr, st), None, c, c)

    def make_cover(name):
        def cb(uc, address, size, ud):
            HITS[name] = HITS.get(name, 0) + 1
        return cb

    for name, addr in COVER.items():
        m.raw.hook_add(UC_HOOK_CODE, make_cover(name), None, addr, addr)


def report():
    if HITS:
        print('coverage (hits in the original): ' + ', '.join('%s %d' % (k, HITS[k]) for k in sorted(HITS)))


atexit.register(report)


def begin(ctx, *active):
    m = ctx.m
    install(m)
    ST.clear()
    ST['seed'] = ctx.rng.getrandbits(32)
    ST['active'] = set(active)
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    return ctx.rng


# ---- 0x1002a2f6: record initialiser -------------------------------------------------------------------------
def g_a2f6(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015dc78, r.choice([0, 100, 0x7fffffff, r.getrandbits(32)]))
    return [newbuf(ctx, 0x170)]


# ---- 0x1003b986: entry table --------------------------------------------------------------------------------
def entries(ctx, n):
    """n random entries of 0x7e bytes at 0x109b9e20 (count 0x1015ea80); returns their keys (+0x60)."""
    r = ctx.rng
    keys = []
    w32(ctx, 0x1015ea80, n)
    for i in range(n):
        e = 0x109b9e20 + 0x7e * i
        wb(ctx, e, rb(ctx, 0x7e))
        k = r.choice([r.getrandbits(32), r.randrange(0, 5), 0])
        w32(ctx, e + 0x60, k)
        keys.append(k)
    return keys


def g_b986(ctx):
    r = begin(ctx)
    n = r.choice([0, 1, 2, 3, 5, 8, 12])
    keys = entries(ctx, n)
    if n > 1 and r.random() < 0.3:
        w32(ctx, 0x109b9e20 + 0x7e * r.randrange(0, n) + 0x60, keys[0])
    key = r.choice(keys + [r.getrandbits(32), 7]) if keys else r.getrandbits(32)
    return [key]


# ---- 0x10057d2d: keys ---------------------------------------------------------------------------------------
def g_d2d(ctx):
    r = begin(ctx)
    for k in range(8):
        w32(ctx, 0x1015e650 + 4 * k, r.getrandbits(32))
    scan = r.choice([0, 1, 0x1c, 0x1d, 0x2a, 0x36, 0x38, 0x45, 0x47, 0x4d, 0x50, 0x58, 0x59, 0x5a, 0x70, 0x71,
                     0xff, r.randrange(0, 256), r.randrange(0, 0x59)])
    ext = r.choice([0, 1, 1])
    lparam = (scan << 16) | (ext << 24) | r.getrandbits(16) | (r.getrandbits(7) << 25)
    return [ctx.u32(), lparam, r.choice([0, 0, 1, 1, 0x1234])]


# ---- 0x1006bf33: record events ------------------------------------------------------------------------------
def g_bf33(ctx):
    r = begin(ctx, 0x100203cf)
    cur = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, cur)
    s = newbuf(ctx, 0x100)
    obj = newbuf(ctx, 0x20)
    w32(ctx, obj + 4, r.choice([cur, cur, 9]))
    w32(ctx, s, obj)
    recs = newbuf(ctx, 8 * 0x128)
    w32(ctx, s + 0x84, recs)
    code = r.choice([9, 10, 11, 13, 14, 15, 16, r.randrange(0, 20)])
    for i in range(8):
        rec = recs + 0x128 * i
        w32(ctx, rec + 4, r.choice([-1, -1, r.randrange(0, 0x2e), r.randrange(0, 0x2e), 0x2e, 0x40, r.getrandbits(32)]))
        w32(ctx, rec + 0x30, r.choice([code, code, r.randrange(0, 20)]))
    return [ctx.u32(), code, s]


# ---- 0x1007e0ff: message bars -------------------------------------------------------------------------------
def g_e0ff(ctx):
    r = begin(ctx, 0x100ba7d5, 0x100f0205, 0x100e91dc, 0x100f2f01, 0x100f1e10, 0x100ecea6, 0x1002ab9d)
    ticks = []
    for i in range(2):
        m = 0x10149d28 + 0x3c * i
        wb(ctx, m, rb(ctx, 0x3c))
        w32(ctx, m, cstr(ctx, text(ctx, b'Message 0', r.randrange(0, 20))))
        w32(ctx, m + 4, r.choice([0, 10, 320, r.getrandbits(32)]))
        w32(ctx, m + 8, r.choice([0, 10, 200, r.getrandbits(32)]))
        w32(ctx, m + 0xc, r.choice([0, 1, 1, 5]))
        wb(ctx, m + 0x14, text(ctx, b'FONTab1', r.randrange(1, 12)) + b'\0')
        wb(ctx, m + 0x24, text(ctx, b'HMESBAR0', r.randrange(1, 12)) + b'\0')
        expiry = r.choice([0, 100, 5000, -1, 0x7fffffff, 0x80000000])
        w32(ctx, m + 0x34, expiry)
        w32(ctx, m + 0x38, newbuf(ctx, 0x20))
        ticks.append(r.choice([0, 99, 100, 101, 4999, 5000, 5001, -2, 0x7ffffffe, expiry, expiry - 1, expiry + 1,
                               r.getrandbits(31)]))
    ST['ticks'] = ticks
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    return []


# ---- 0x1009d90e: object tree --------------------------------------------------------------------------------
def g_d90e(ctx):
    r = begin(ctx, 0x100a9830, 0x100a9b8d, 0x10029ab3, 0x100aa0da)
    index = r.choice([r.randrange(0, 0x19)] * 9 + [-1, 0x19, 0x100])
    names = [b'WRF', b'WBZ', b'POF', b'abc', b'xWRFy', b'POFFY', b'', b'wbz', b'A_POF_B', b'hull']
    if not 0 <= index < 0x19:
        names = [nm for nm in names if b'POF' not in nm]      # the only caller keeps the index in range
    n = r.choice([0, 2, 4, 8])
    entries(ctx, n)
    objs = []

    def mkobj():
        o = newbuf(ctx, 0x100)
        nm = r.choice(names)
        if r.random() < 0.3:
            nm = text(ctx, b'ab', r.randrange(0, 4)) + nm + text(ctx, b'cd', r.randrange(0, 4))
        w32(ctx, o + 0x90, cstr(ctx, nm))
        objs.append(o)
        return o

    def mknode(depth):
        if depth > 3 or (depth > 0 and r.random() < 0.3):
            return 0
        node = newbuf(ctx, 0x28)
        w32(ctx, node + 8, mkobj() if r.random() < 0.92 else 0)
        w32(ctx, node + 0xc, r.choice([r.getrandbits(32), 0, 5, newbuf(ctx, 0x10)]))
        w32(ctx, node, mknode(depth + 1))
        w32(ctx, node + 4, mknode(depth + 1))
        return node

    root = mknode(0) if r.random() < 0.97 else 0
    for i in range(n):
        if objs and r.random() < 0.5:
            w32(ctx, 0x109b9e20 + 0x7e * i + 0x60, r.choice(objs))
    ST['nodes'] = [r.choice([newbuf(ctx, 0x100)] * 12 + [0]) for _ in range(40)]
    for i in range(0x19):
        ptr = newbuf(ctx, 0x40)
        owner = newbuf(ctx, 0x700)
        w32(ctx, owner + 0x5fb, r.randrange(0, 0x60))
        w32(ctx, ptr + 0x28, owner)
        w32(ctx, 0x109e8b40 + 4 * i, ptr)
    return [root, r.choice([0, 0, 1]), index]


# ---- 0x100b5bcb, 0x100c4f6c ---------------------------------------------------------------------------------
def g_b5bcb(ctx):
    begin(ctx)
    w32(ctx, 0x1015d1b8, newbuf(ctx, 0x30))
    w32(ctx, 0x1015084c, ctx.u32())
    return [ctx.u32(), ctx.u32()]


def g_4f6c(ctx):
    begin(ctx)
    return []


# ---- 0x100ccc60: HUD text lines -----------------------------------------------------------------------------
LABELS = (0x101c4980, 0x101c4988, 0x101c498c, 0x101c4994, 0x101c499c, 0x101c49a0, 0x101c49a4, 0x101c49a8)


def words(ctx, total, first_space_at=None):
    """A text of about `total` bytes made of words; with first_space_at the first space is at that index."""
    r = ctx.rng
    out = b''
    if first_space_at is not None:
        out = text(ctx, b'abcdXYZ', first_space_at) + b' '
    while len(out) < total:
        out += text(ctx, b'abcdXYZ09', r.randrange(1, 9)) + b' '
    return out[:total].rstrip(b' ') if first_space_at is None else out[:max(total, first_space_at + 2)]


def g_ccc60(ctx):
    r = begin(ctx, 0x100f2f01, 0x100f1e10, 0x100ecea6, 0x100ccb0d, 0x100ecce0)
    idx = r.choice([0, 0, 1, 2])
    w32(ctx, 0x1013bb40, idx)
    tab = 0x10473800 + idx * 0x4fc3a
    n = r.choice([0, 1, 2, 3, 4, 6])
    w32(ctx, tab, n)
    kind = r.choice([1, 2, 2, 4, 8, 3, 0, r.randrange(0, 256)])
    if r.random() < 0.3:
        kind |= r.getrandbits(24) << 8
    for i in range(n):
        e = 0x1047383a + idx * 0x4fc3a + i * 0x13f
        wb(ctx, e, rb(ctx, 0x13f))
        w8(ctx, e, r.choice([5, 6, 0, 1, 7]))
        w8(ctx, e + 1, r.choice([kind & 0xff, kind & 0xff, kind & 0xff, 1, 2, 4, 8, 0, 3]))
        w32(ctx, e + 2, r.choice([0x8000, 0x8001, 0x8002, 0, 1, r.getrandbits(32)]))
        w8(ctx, e + 0x74, r.choice([0, 1, 1, 1, 0x80]))
        c = r.random()
        if c < 0.4:
            s = words(ctx, r.choice([0, 1, 5, 0x1e, 0x1f]))
        elif c < 0.55:
            s = words(ctx, r.choice([0x20, 0x21, 0x40]), first_space_at=r.choice([0x21, 0x21, 0x10, 3]))
        else:
            s = words(ctx, r.choice([0x20, 0x21, 0x22, 0x30, 0x50, 0x80]))
        if len(s) > 0x8c:
            s = s[:0x8c]
        wb(ctx, e + 0xad, s + b'\0')
    panel = newbuf(ctx, 0x100)
    pane = newbuf(ctx, 0x20)
    w32(ctx, pane + 4, r.choice([0, 10, 100]))
    w32(ctx, pane + 0xc, r.choice([320, 640, 1280, 0]))
    w32(ctx, panel + 0xec, pane)
    pair = newbuf(ctx, 8)
    w32(ctx, pair, r.choice([0, 8, 300, r.getrandbits(32)]))
    w32(ctx, panel + 0xf0, pair)
    pos = ctx.buf(8, struct.pack('<2I', r.choice([0, 5, 100]), r.choice([0, 20, 400, 0xfffffff0])))
    font = newbuf(ctx, 0x20)
    w32(ctx, font + 8, r.choice([8, 10, 16, 0, 0xfffffffd]))
    w32(ctx, 0x1015ec58, r.choice([0, 1, 1]))
    hdr = newbuf(ctx, 0x20)
    w8(ctx, hdr + 0xa, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b8c4, hdr)
    w32(ctx, 0x101668e8, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    for a in LABELS:
        w32(ctx, a, cstr(ctx, text(ctx, b'LABEL:*', r.randrange(0, 8))))
    return [panel, pos, font, kind]


# ---- 0x100d3d10: weapon panel -------------------------------------------------------------------------------
def g_d3d10(ctx):
    r = begin(ctx, 0x100ba7d5, 0x100f0205, 0x100e91dc, 0x100f2f01, 0x100f1e10, 0x100ecea6, 0x100bb2b0,
              0x1008d4f1)
    panel = newbuf(ctx, 0x100)
    which = r.choice([r.randrange(0, 8)] * 8 + [-1, -5])
    w32(ctx, panel + 0xc, which)
    wb(ctx, panel + 0x10, text(ctx, b'Gun ab1', r.randrange(0, 14)) + b'\0')
    for off in (0xd4, 0xd8, 0xdc, 0xe0):
        w32(ctx, panel + off, r.choice([0, 10, 320, -4, r.getrandbits(32)]))
    pane = newbuf(ctx, 0x20)
    w32(ctx, panel + 0xec, pane)
    pair = newbuf(ctx, 8)
    w32(ctx, panel + 0xf0, pair)
    w32(ctx, 0x101355b8, r.choice([0, 1, 1, 1, 1]))
    cur = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, cur)
    ptr = newbuf(ctx, 0x40)
    w32(ctx, 0x109e8b40 + 4 * cur, ptr)
    block = newbuf(ctx, 0x600)
    w32(ctx, ptr + 0x28, block)
    weapons = newbuf(ctx, 8 * 0x128)
    w32(ctx, block + 0x84, weapons)
    w32(ctx, block + 0x4fc, r.choice([r.randrange(0, 8), which, which]))
    for i in range(8):
        w = weapons + 0x128 * i
        w32(ctx, w + 4, r.choice([-1, 0, 1, 2, 3, 0, 1, 4]))
        k8 = r.choice([-1, 0, 1, 1, 2, 3, 7, r.getrandbits(32)])
        w32(ctx, w + 8, k8)
        # +0xc above 4 with +8 = 1 leaves the colour uninitialised in the original: not generated
        w32(ctx, w + 0xc, r.randrange(0, 5) if k8 == 1 else r.choice([0, 5, 9, r.getrandbits(32)]))
        w32(ctx, w + 0x20, r.choice([-5, 0, 0, 1, 42, 1000, 0x7fffffff]))
    w32(ctx, 0x109b98c0, cstr(ctx, text(ctx, b'HUDFONT', r.randrange(1, 9))))
    w32(ctx, 0x109e84b8, cstr(ctx, text(ctx, b'WPNBOX', r.randrange(1, 9))))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    w32(ctx, 0x101c64f0, r.choice([0, 1, 1]))
    w32(ctx, 0x101668e8, r.choice([0, 0, 1]))
    return [panel]


# ---- 0x10035aec: row copy on the display record ---------------------------------------------------------------
def g_5aec(ctx):
    r = begin(ctx)
    h = r.choice([0, 1, 3, 4, 5, 7, 8, 16, 40, 64, -1, -3, -4, -5, -8, 0x80000000])
    s = r.choice([0, 3, 9, 0x13, 0x1d, 0x1e, 0x1f, 0x40, 0x100, 0xffffffff])
    pixels = newbuf(ctx, 0x1100)
    rec = ctx.buf(12, struct.pack('<3I', pixels, s, h & 0xffffffff))
    w32(ctx, 0x1015e698, rec)
    return []


# ---- 0x100363af: plugin probe by list file ---------------------------------------------------------------------
def g_363af(ctx):
    r = begin(ctx, 0x100fd850, 0x100fe770, 0x100fd760, 0x10035f90, 0x10036027, 0x100fd560)
    ST['vd850'] = [r.choice([STREAM, STREAM, STREAM, 0])]
    lines = []
    for _ in range(r.choice([0, 1, 2, 3, 5, 8])):
        c = r.random()
        if c < 0.5:
            lines.append(text(ctx, b' \t', r.randrange(0, 3)) + text(ctx, b'abcXYZ09_.\\', r.randrange(1, 20))
                         + r.choice([b'\n', b'\r\n', b'', b' \n', b'\tsecond token\n']))
        elif c < 0.7:
            lines.append(r.choice([b'\n', b'\r\n', b'', b' \t \n']))
        else:
            lines.append(text(ctx, b'plugin.dl ', r.randrange(0, 0xff)))
    ST['lines'] = lines
    w32(ctx, 0x1015e56c, ctx.u32())
    return [cstr(ctx, text(ctx, b'abcXYZ09_.\\', r.randrange(0, 20))), ctx.u32()]


# ---- 0x100bb405: cache loader --------------------------------------------------------------------------------
def g_bb405(ctx):
    r = begin(ctx, 0x100ff860, 0x100bca49, CODE, 0x100fd850, 0x100feb20, 0x100fe970, 0x100fef30, 0x100fe830,
              0x100fd760, 0x100fefe0, 0x100baf3f, 0x100bda9f, 0x100b9b3f, 0x100bae0a)
    ctx.m.uc.mem_write(CODE, b'\xc3')
    blocks = [newbuf(ctx, 0x400) for _ in range(3)]
    ST['cached'] = [r.choice([0, 0, 0, 0, blocks[0], HANDLE])]
    ST['vd850'] = [r.choice([STREAM, STREAM, 0, 0])]
    ST['vae0a'] = [r.choice([STREAM, STREAM, 0])]
    ST['packed'] = [r.choice([0, blocks[1], blocks[1], HANDLE + 0x40])]
    ST['sizes'] = [r.choice([0, 1, 0x10, 0x100, 0x3ff, 0x400])]
    ST['blocks'] = [r.choice([blocks[2], blocks[2], blocks[2], 0])]
    ST['fixup'] = [r.choice([0, 0, blocks[r.randrange(0, 3)], HANDLE + 0x80])]
    w32(ctx, 0x1017d5fc, r.choice([0, 1]))
    w32(ctx, 0x1017d604, r.choice([HANDLE + 0x100, newbuf(ctx, 0x10)]))
    ST['type'] = [newbuf(ctx, 0x30)]
    name = cstr(ctx, text(ctx, b'abcxyzXYZ09_', r.choice([0, 1, 3, 5, 7, 8, 12])))
    ext = cstr(ctx, r.choice([b'wtb', b'XEL', b'wav', b'3di', b'Tlt', text(ctx, b'abcZ9', r.randrange(0, 5))]))
    # the original reads an uninitialised local when no fixup is given (see w9c.h): only calls with one are proven
    return [name, ext, CODE, ctx.u32()]


def spec(name, orig, gen, trials, **kw):
    return dict(name=name, orig=orig, c=name, gen=gen, trials=trials, **kw)


SPECS = [
    spec('Sim_1002a2f6', 0x1002a2f6, g_a2f6, 400, void=True),
    spec('Sim_1003b986', 0x1003b986, g_b986, 600, void=True),
    spec('Key_SetState', 0x10057d2d, g_d2d, 1500, void=True),
    spec('Sim_1006bf33', 0x1006bf33, g_bf33, 1500, void=True),
    spec('Sim_1007e0ff', 0x1007e0ff, g_e0ff, 1200, void=True),
    spec('Sim_1009d90e', 0x1009d90e, g_d90e, 2000),
    spec('Sim_100b5bcb', 0x100b5bcb, g_b5bcb, 300, void=True),
    spec('Sim_100c4f6c', 0x100c4f6c, g_4f6c, 300),
    spec('Sim_100ccc60', 0x100ccc60, g_ccc60, 2000, void=True),
    spec('Sim_100d3d10', 0x100d3d10, g_d3d10, 2000, void=True),
    spec('Sim_10035aec', 0x10035aec, g_5aec, 500, void=True),
    spec('Plugin_ProbeList', 0x100363af, g_363af, 1000),
    spec('Cache_LoadItem', 0x100bb405, g_bb405, 1500),
]
