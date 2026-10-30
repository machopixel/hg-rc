# Differential tests for round 7, letter o (see difftest.py and docs/checker.md).
#
# Converted callees run as real code on both sides (the original callee in the original run, its C conversion in
# the C run); both are verified already. Callees that are not converted (or that another hook file patches) are
# called through their original address by both versions: some run for real in the emulator (the C library's
# 0x100fcb70, 0x10084766, 0x10060d2e, Render_ProjectVertex and the edge clippers behind Render_ClipPolygon), the
# others are replaced, for the trials that ask for it, by a stub: an emulator code hook at the callee's address
# that appends ('stub_<address>', argument words) to the checker's call log (compared between the two runs like
# the Windows calls; stack addresses shown as 'stack'), may write results through pointer arguments (the same
# bytes for both runs, computed from what it is given), answers with a value chosen by the generator and returns
# to the caller (EIP/ESP set; no code bytes are patched). The display driver's polygon routine (slot 0x1014cb4c)
# points at a `ret` with a hook that writes the count, the flags and the vertex fields the caller sets into a log
# buffer in argument memory (compared like all memory). Windows imports are stubbed by the checker itself.
# Every FPU spec here is checked bit for bit (no float tolerance).
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_1009/w7o_effects.c', 'sim/unsorted_1009/w7o_modes.c', 'sim/unsorted_1009/w7o_misc.c',
    'sim/unsorted_1008/w7o_cells.c', 'sim/render/w7o_draw.c', 'sim/unsorted_100a/w7o_nodes.c',
    'sim/unsorted_100a/w7o_misc.c',
    # converted callees and what they need
    'sim/win/w4u_cdaudio.c', 'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_1010/w6k_text.c', 'sim/unsorted_100a/w5a_misc.c',
    'sim/unsorted_100f/w6i_mem.c', 'sim/view/w3r_matrix.c', 'sim/unsorted_1010/w2g_crt.c', 'sim/unsorted_100f/w2e_string.c',
    'sim/unsorted_100f/w6i_exit.c', 'sim/render/w2n_project.c', 'sim/view/w2a_points.c', 'sim/unsorted_1000/w3s_model.c',
    'sim/unsorted_1002/w3o_ray.c', 'sim/render/w3p_view.c', 'sim/unsorted_1006/w6e_misc.c', 'sim/unsorted_1007/w2e_state.c',
    'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1008/w6f_cells.c', 'sim/unsorted_1008/w2g_rect.c', 'sim/view/w2l_rects.c',
    'sim/view/objlist.c', 'sim/unsorted_1009/w2g_objects.c', 'sim/unsorted_1009/w6f_modes.c', 'sim/util/ring.c',
    'sim/render/w3q_vertex.c', 'sim/render/w3r_draw.c', 'sim/unsorted_100a/w3o_misc.c', 'sim/unsorted_100a/w2h_misc.c',
    'sim/unsorted_100a/w5c_calls.c', 'sim/unsorted_100a/w5b_calls.c', 'sim/util/field.c', 'sim/unsorted_100a/w3t_misc.c',
    'sim/unsorted_100a/w5z_calls.c', 'sim/unsorted_100a/w5x_calls.c', 'sim/unsorted_100a/w5y_misc.c',
    'sim/unsorted_100a/w6g_nodes.c', 'sim/unsorted_100d/w5c_calls.c', 'sim/unsorted_100d/state.c',
    'sim/unsorted_1010/w6k_stream.c', 'sim/view/viewrect.c', 'sim/unsorted_100f/w2k_string.c', 'sim/win/w4v_heap.c',
    'sim/view/matrix.c', 'sim/unsorted_100f/w2h_mem.c', 'sim/unsorted_1006/w3q_quat.c', 'sim/unsorted_1000/w5z_calls.c',
    'sim/unsorted_1000/w2f_records.c', 'sim/unsorted_1000/w2k_misc.c', 'sim/unsorted_1000/w3o_sphere.c',
    'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1000/w5x_calls.c', 'sim/unsorted_1001/w2h_tables.c',
    'sim/unsorted_1002/w5a_misc.c', 'sim/unsorted_1002/w5z_calls.c', 'sim/unsorted_1002/w5y_misc.c',
    'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_1002/w2e_tables.c', 'sim/core/w2a_timers.c', 'sim/unsorted_1002/w2h_misc.c',
    'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1003/w2l_records.c', 'sim/unsorted_1004/w5y_misc.c',
    'sim/unsorted_1004/w2e_strings.c', 'sim/unsorted_1004/w3o_misc.c', 'sim/unsorted_1004/w2k_flags.c',
    'sim/unsorted_1006/w5a_math.c', 'sim/unsorted_1006/w5b_calls.c', 'sim/unsorted_1006/w5c_math.c',
    'sim/unsorted_1008/w3o_grid.c', 'sim/view/w2l_lists.c', 'sim/unsorted_1008/cell_link.c', 'sim/view/w2d_view.c',
    'sim/unsorted_1008/w3s_grid.c', 'sim/unsorted_1009/w5y_misc.c', 'sim/unsorted_1009/w2f_misc.c',
    'sim/unsorted_1009/w3o_ray.c', 'sim/unsorted_100a/w2i_misc.c', 'sim/unsorted_100a/w2k_list.c', 'sim/render/w2n_polygon.c',
    'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_100d/w2g_misc.c', 'sim/unsorted_1010/w2h_misc.c', 'sim/win/w4a_heap.c',
    'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_1000/w2l_objects.c', 'sim/unsorted_1000/w2e_names.c',
    'sim/unsorted_1002/w3o_vectors.c', 'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1002/w2l_tables.c',
    'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1003/w5a_misc.c', 'sim/unsorted_1004/fields.c',
    'sim/unsorted_1005/w2h_tables.c', 'sim/unsorted_100d/w2j_nodes.c', 'sim/core/timer.c', 'sim/unsorted_1001/w2e_names.c',
    'sim/unsorted_1003/w2e_misc.c', 'sim/unsorted_1004/tables.c', 'sim/unsorted_1007/w5a_misc.c',
    'sim/unsorted_1002/w2e_object.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
STACK_TOP = 0x00800000
CODE = 0x00AF8000       # the display driver routine (a `ret` with a logging hook)
DLOG_CAP = 0x4000

# stubbed callee -> number of argument words logged
STUBS = {
    0x100aa742: 1, 0x10088a2d: 1,           # inside Sim_100aa799
    0x100ef727: 7, 0x100e86a1: 7,           # Sim_1008d4f1
    0x100246b3: 12,                         # Sim_10090f71
    0x10099c36: 0, 0x10099c59: 0,           # Sim_1009b99b, inside Sim_1009b8d8
    0x100ff280: 1,                          # toupper (Str_CopyUpper)
    0x100fc780: 2,                          # sprintf (Sim_10099510)
    0x100a539e: 1,                          # Sim_100a4c45
    0x101049c0: 1, 0x10105fe0: 3,           # close / seek (Sim_100a7c4c, Sim_100a7e55)
    0x100d2306: 1, 0x1007dd90: 0,           # Sim_100ab920
    0x100d2669: 3,                          # point projection (Sim_10094d7f, Sim_10095453)
    0x100a65e7: 2, 0x100a9ba7: 1,           # Sim_100a4510 (and inside Sim_100aa307)
    0x10060b76: 2,                          # inside Sim_100a9c25 / Sim_100a9ba7
    0x100035df: 5, 0x10001e1f: 5, 0x10002775: 5,    # the cell tests of Sim_1002d961 (thunks)
    0x100fef50: 2, 0x100fd820: 3,           # inside Heap_Malloc / Sim_100fd850 (File_OpenRead)
    0x10105b50: 3,                          # the read inside File_ReadChunked (Sim_100a7e55)
    0x1000e737: 12, 0x1001f790: 3,          # inside Sim_1000ebfb / Sim_100203cf (Sim_100999fc)
}
EFFECT = {}
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def ur32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def bitsf(u):
    return struct.unpack('<f', struct.pack('<I', u & 0xffffffff))[0]


def f32(x):
    return bitsf(fbits(max(-3.4e38, min(3.4e38, x))))


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


# ---- stubs --------------------------------------------------------------------------------------
def _stub_cb(m, addr):
    nargs = STUBS[addr]

    def cb(uc, address, size, ud):
        if addr not in ST.get('active', ()):
            return
        esp = uc.reg_read(UC_X86_REG_ESP)
        raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % addr, tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)))
        val = None
        if addr in EFFECT:
            val = EFFECT[addr](uc, n, raw)
        if val is None:
            rets = ST['rets'].get(addr) or [0]
            val = rets[n % len(rets)]
        uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def _driver_cb(m):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        count, verts, flags = struct.unpack('<3I', bytes(uc.mem_read(esp + 4, 12)))
        log = ST['dlog']
        pos = ur32(uc, log)
        rec = [0xd0d0d0d0, count, flags]
        for i in range(min(count, 16)):
            try:
                v = struct.unpack('<9I', bytes(uc.mem_read(verts + 36 * i, 36)))
            except Exception:
                rec.append(0xbadbad)
                break
            rec += [v[k] for k in ST['dfields']]
        data = struct.pack('<%dI' % len(rec), *rec)
        if pos + len(data) <= DLOG_CAP:
            uc.mem_write(log + 4 + pos, data)
            uc.mem_write(log, struct.pack('<I', pos + len(data)))
        uc.reg_write(UC_X86_REG_EAX, mix(ST['seed'], pos))
    return cb


def begin(ctx, *active, fill=False):
    """Installs the hooks (once per machine), activates the given stubs, the x87 state of the game (control word
    0x27f), the random number table of Sim_1009c929, the driver slot and its log. Must come first in a generator."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w7o', False):
        m._w7o = True
        for addr in STUBS:
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr), None, addr, addr)
        m.raw.hook_add(UC_HOOK_CODE, _driver_cb(m), None, CODE, CODE)
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['active'] = set(active)
    ST['seed'] = r.getrandbits(32)
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}
    ST['dfields'] = (0, 1, 2, 3, 4, 5, 7, 8)
    ST['dlog'] = zbuf(ctx, 4 + DLOG_CAP + 4)
    m.uc.mem_write(CODE, b'\xc3')
    w32(ctx, 0x1014cb4c, CODE)
    w32(ctx, 0x1017cd38, r.randrange(0, 0x7f))
    w32(ctx, 0x101cffc0, *[r.choice([r.randrange(0, 1000), r.getrandbits(31), r.randrange(0, 50)]) for _ in range(0x7f)])
    if fill:
        # the stack below the arguments is filled with one repeated byte, so locals the original leaves unset
        # (and hands to its callees) read the same in both runs
        pat = r.choice([0, 0x55, 0xa5, 0x3f, 0x41])
        m.uc.mem_write(STACK_TOP - 0x20000, bytes([pat]) * (0x20000 - 0x100))
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


# ---- worlds (as in spec_w6f.py / spec_w6g.py) ----------------------------------------------------------
def spare(ctx):
    return newbuf(ctx, 0x40)


def scene_obj(ctx, links=True):
    """A scene object: flags word +0, kind word +2, links +4/+8, +0xc/+0x10, +0x14/+0x18, +0x1c/+0x20 that are 0
    or point at spare buffers, owner +0x28 (0 or an object with position +0x70..+0x78), cell +0x8c/+0x90."""
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    sp = [spare(ctx) for _ in range(2)]
    for off in (4, 8, 0xc, 0x10, 0x14, 0x18, 0x1c, 0x20):
        w32(ctx, obj + off, r.choice([0, 0, 0, sp[0], sp[1]]) if links else 0)
    w16(ctx, obj, r.choice([0, 0x1000, 0x1000, 0x800, 0x1800, r.getrandbits(16)]))
    w16(ctx, obj + 2, r.choice([0x70, 0x70, 0x10, 0x75, r.getrandbits(16)]))
    if r.random() < 0.95:
        own = newbuf(ctx, 0x80)
        w32(ctx, own, r.choice([0, 0, 1]))
        w32(ctx, own + 0x70, rfl(ctx, 3000), rfl(ctx, 3000), rfl(ctx, 3000))
        w32(ctx, obj + 0x28, own)
    else:
        w32(ctx, obj + 0x28, 0)
    w32(ctx, obj + 0x2c, 0)
    return obj


def node_tree(ctx, depth, p_obj=0.7):
    """Scene node: first child +4, next sibling +8, object +0x8c (or 0); sane floats +0x30..+0x78."""
    r = ctx.rng
    node = zbuf(ctx, 0xa0)
    sane_floats(ctx, node + 0x30, 0x13, 3000)
    w32(ctx, node, r.choice([0, 0, 1]))
    w32(ctx, node + 0x8c, scene_obj(ctx) if r.random() < p_obj else 0)
    kids = [node_tree(ctx, depth - 1, p_obj) for _ in range(r.choice([0, 1, 1, 2, 3]))] if depth > 0 else []
    if kids:
        w32(ctx, node + 4, kids[0])
        for a, b in zip(kids, kids[1:]):
            w32(ctx, a + 8, b)
    return node


def cell_links(ctx, cell):
    r = ctx.rng
    for off in (4, 8, 0x14, 0x18, 0x1c, 0x20):
        w32(ctx, cell + off, r.choice([0, 0, spare(ctx)]))
    w16(ctx, cell, r.choice([0, 0, 0x800]))
    w32(ctx, cell + 0x28, 0)


def node_world(ctx):
    """The flags the node helpers read."""
    r = ctx.rng
    w32(ctx, 0x1017c8c4, r.choice([0, 0, 1]))
    w32(ctx, 0x1017c8cc, r.choice([0, 0x1234]))
    w32(ctx, 0x1014b8e0, fbits(r.choice([100.0, 1000.0, 250.0])))
    w32(ctx, 0x1014b8e4, fbits(r.choice([100.0, 1000.0, 250.0])))
    for c in (0x1017b988, 0x1017b9b0, 0x1017b9d8):
        cell_links(ctx, c)


def camera(ctx, scale=3000.0):
    """The camera [0x1014cae0] with sane floats."""
    cam = ctx.buf(0x140, b''.join(struct.pack('<I', rfl(ctx, scale)) for _ in range(0x50)))
    w32(ctx, 0x1014cae0, cam)
    return cam


def particles(ctx, obj, scale=1000.0):
    for i in range(16):
        sane_floats(ctx, obj + 0x12 + i * 0x2a, 6, scale)


def effect_obj(ctx, limit):
    """An effect object with a node tree at +0x552, age around its limit (as in spec_w6f.py)."""
    r = ctx.rng
    obj = newbuf(ctx, 0x580)
    w32(ctx, obj + 0x552, node_tree(ctx, r.choice([0, 0, 1, 2]), 0.8))
    now = ctx.u32()
    w32(ctx, 0x1015dc78, now)
    lim = limit if limit is not None else r.choice([0, 1, 10, 0x5a, r.randrange(-5, 300), r.randrange(1, 300)])
    if limit is None:
        w32(ctx, obj + 0x562, lim)
    lim = lim if lim < 0x80000000 else lim - (1 << 32)
    age = r.choice([lim, lim + 1, lim - 1, 0, -1, r.randrange(-3, max(lim, 0) + 4), r.randrange(-3, max(lim, 0) + 4),
                    r.randrange(-3, max(lim, 0) + 4), r.randrange(-3, max(lim, 0) + 4), r.randrange(-300, 0),
                    r.randrange(-3, max(lim, 0) + 4), lim + 2, r.getrandbits(32) if r.random() < 0.1 else lim + 5])
    w32(ctx, obj + 0x55e, (now - age) & 0xffffffff)
    for off in (0x56e, 0x572, 0x576):
        w32(ctx, obj + off, rfl(ctx, 50))
    w16(ctx, obj + 0x10, r.getrandbits(16))
    particles(ctx, obj)
    return obj


def pool_world(ctx):
    """The effect pool of Sim_10090c7d (as in spec_w6f.py) with node trees, the counts."""
    r = ctx.rng
    n = 6
    w32(ctx, 0x10215ae8, r.choice([-1, 0, r.randrange(0, n), r.randrange(0, n), r.randrange(0, n), n - 1, n - 1]))
    w32(ctx, 0x10216b80, *[r.randrange(0, n) for _ in range(n + 2)])
    w32(ctx, 0x1014b9b8, r.choice([0, 1, 1, 1, 1, 1, ctx.u32()]))
    for i in range(n):
        o = 0x10217c20 + 0x57e * i
        wb(ctx, o, rb(ctx, 0x57e))
        w32(ctx, o + 0x552, node_tree(ctx, r.choice([0, 0, 1]), 1.0))
    if r.random() < 0.5:
        w32(ctx, 0x1017cc28, 0, 0)
    else:
        w32(ctx, 0x1017cc28, newbuf(ctx, 16), newbuf(ctx, 16))
    limit = r.randrange(0, 40)
    w32(ctx, 0x1017cc20, limit)
    w32(ctx, 0x1017cc24, r.choice([limit - 2, limit - 3, limit - 5, r.randrange(-1, 20), limit - 1, 0, 0, 0xffffffff]))
    w32(ctx, 0x1015dc78, ctx.u32())


# ---- 0x1009 effects -----------------------------------------------------------------------------------
NODE_STUBS = (0x100aa742, 0x10088a2d)


def upd(limit):
    def g(ctx):
        begin(ctx, *NODE_STUBS)
        node_world(ctx)
        return [effect_obj(ctx, limit)]
    return g


def g_90f71(ctx):
    r = begin(ctx, 0x100246b3, *NODE_STUBS)
    node_world(ctx)
    obj = effect_obj(ctx, None)
    src = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x57a, src)
    return [obj]


def g_92a71(ctx):
    r = begin(ctx, *NODE_STUBS)
    node_world(ctx)
    return [effect_obj(ctx, None)]


def g_93ce7(ctx):
    r = begin(ctx, *NODE_STUBS)
    node_world(ctx)
    obj = effect_obj(ctx, 0xb5)
    for i in range(16):
        tab = newbuf(ctx, 0x200)
        w32(ctx, obj + 0x12 + i * 0x2a + 0x1a, tab + 0x100)
    return [obj]


def g_9524d(ctx):
    r = begin(ctx, *NODE_STUBS)
    node_world(ctx)
    obj = effect_obj(ctx, 0xb5)
    for i in range(0x40):     # no zero in the table (a random number of 0 would divide by 0)
        ctx.m.uc.mem_write(0x10217b90 - 0x20 + i, bytes([r.randrange(1, 256)]))
    return [obj]


def g_91597(ctx):
    r = begin(ctx, *NODE_STUBS)
    node_world(ctx)
    pool_world(ctx)
    cam = camera(ctx)
    src = newbuf(ctx, 0x80)
    near = r.random() < 0.8
    pos = []
    for k in range(3):
        c = bitsf(r32(ctx, cam + 4 * k))
        pos.append(fbits(c + r.uniform(-100, 100)) if near else rfl(ctx, 5000))
    w32(ctx, src + 0x70, *pos)
    w32(ctx, cam + 0x40, fbits(r.choice([100.0, 60.0, 200.0, 0.0, 1e9, 1e9, r.uniform(0, 300)])))
    size = r.choice([0, 1, 2, 3, 5, -1, 10, r.randrange(-100, 100)])
    return [src, size & 0xffffffff]


# ---- 0x1009 view modes ----------------------------------------------------------------------------------
def name_table(ctx):
    r = ctx.rng
    w32(ctx, 0x10a38de0, r.choice([0, 1, 2, 3, 5, 8]))
    for i in range(12):
        nm = bytes(r.choice(b'abcAFXWL') for _ in range(r.randrange(0, 9)))
        wb(ctx, 0x10a39200 + i * 0x14, struct.pack('<I', r.choice([0, 1, 2])) + nm + b'\0' + rb(ctx, 15 - len(nm)))
    w32(ctx, 0x101321b4, ctx.u32())
    for i in range(16):
        w32(ctx, 0x1012df30 + 4 * i, ctx.u32())


def mode_entry(ctx):
    """An entry of the table 0x1014c4c8: +0 current 5-dword block, +4 save block, +0x14 the pair (n, o) that
    Sim_1008f747 interpolates (as in spec_w6f.py g_9b8d8)."""
    r = ctx.rng
    cur = newbuf(ctx, 0x20)
    save = newbuf(ctx, 0x20)
    entry = newbuf(ctx, 0x20)
    w32(ctx, entry, cur, save)
    n = newbuf(ctx, 0x10)
    o = newbuf(ctx, 0x20)
    w32(ctx, n, r.choice([1, 1, 0, 2]))
    w32(ctx, n + 4, r.choice([0, 0, 1]))
    w32(ctx, n + 8, r.randrange(0, 0x2000))
    w32(ctx, o, r.choice([0x100, 0x400, 0x1000, 0x8000, 0x10000]))
    for off in (4, 8, 0xc):
        w32(ctx, o + off, newbuf(ctx, 0x20))
    pair = ctx.buf(8, struct.pack('<2I', n, o))
    w32(ctx, entry + 0x14, pair if r.random() < 0.9 else 0)
    return entry


def g_modes(ctx):
    r = begin(ctx, 0x10099c36, 0x10099c59, 0x1000e737, 0x1001f790)
    name_table(ctx)
    w32(ctx, 0x1015d1ac, r.choice([0, 1, 2, 3, 4, 5, -1, 4, 0, 1, 2]))
    w32(ctx, 0x1015d1cc, r.choice([0, 1, 2, 3, 4, 5, 3, 5, 6, 0xffffffff, 0, 1, 2]))
    w32(ctx, 0x1015d098, r.choice([0, 1, 2, 4, 4, 5]))
    w32(ctx, 0x1017cc54, r.choice([0, 1, 0, 7]))
    w32(ctx, 0x10179cd4, r.choice([0, 0, 1]))
    cur = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, cur)
    pl = newbuf(ctx, 0x80)
    w16(ctx, pl + 0x1c, r.choice([0, 2, 4, 6, 1, 8, r.getrandbits(16)]))
    sub = newbuf(ctx, 0x700)
    w32(ctx, sub + 0x613, r.randrange(0, 3))
    w32(ctx, pl + 0x28, sub)
    w32(ctx, 0x109e8b40 + 4 * cur, pl)
    for a in (0x10144fa0, 0x10144fa4, 0x10144fa8, 0x1014bef0, 0x1014bef4, 0x1014bef8, 0x1014beec, 0x1014cb3c):
        w32(ctx, a, ctx.u32())
    rec = newbuf(ctx, 0x20)
    for off in (0xc, 0x10):
        nm = r.choice([b'', b'', b'abc'])
        w32(ctx, rec + off, ctx.buf(len(nm) + 4, nm + b'\0' + rb(ctx, 3)))
    w32(ctx, 0x1014c4d8, rec)
    for i in range(-1, 3):      # (0x1014c4d8 is the sound name record above)
        w32(ctx, 0x1014c4c8 + 4 * i, mode_entry(ctx) if r.random() < 0.85 else 0)
    w32(ctx, 0x1015d1c8, r.choice([0, 1, 3, 2]))
    w32(ctx, 0x1015d104, r.choice([0, 1, 3, 2]))
    w32(ctx, 0x1015dc80, r.randrange(0, 0x200))
    return []


# ---- CD poll ----------------------------------------------------------------------------------------------
def a_mci(n, a, uc):
    """mciSendCommandA(id, msg, flags, parms): MCI_STATUS answers the item asked for (mode, number of tracks or a
    position) in the parameter block; now and then fails."""
    esp = uc.reg_read(UC_X86_REG_ESP)
    msg = ur32(uc, esp + 8)
    p = ur32(uc, esp + 16)
    h = mix(ST['seed'], n, 77)
    if h % 100 < ST['mci_fail']:
        return 0x111
    if msg == 0x814:
        item = ur32(uc, p + 8)
        if item == 4:
            val = ST['modes'][n % len(ST['modes'])]
        elif item == 3:
            val = (h >> 8) % 6
        else:
            val = h >> 4
        uc.mem_write(p + 4, struct.pack('<I', val))
    return 0


def a_heapalloc(n, a):
    return ST['heap'][n % len(ST['heap'])]


CD_API = {'mciSendCommandA': a_mci, 'HeapAlloc': a_heapalloc}


def g_cdpoll(ctx):
    r = begin(ctx)
    ST['mci_fail'] = r.choice([0, 0, 0, 10, 40])
    ST['modes'] = [r.choice([0x20d, 0x20e, 0x211, 0x212, 0x20c, 0x213, 0x20f, 1]) for _ in range(4)]
    ST['heap'] = [newbuf(ctx, 0x40), newbuf(ctx, 0x40), r.choice([0, newbuf(ctx, 0x40)])]
    w32(ctx, 0x1017d08c, r.choice([1, 1, 1, 0, 5]))
    w32(ctx, 0x1014c9b0, r.choice([1, 2, ctx.u32()]))
    w32(ctx, 0x1017d078, r.choice([0, 1, 1, 2, 3, 4, 5, ctx.u32()]))
    w32(ctx, 0x1017d080, r.choice([1, 1, 2, 0]), r.randrange(0, 6), r.choice([0, 0, newbuf(ctx, 0x40)]))
    w32(ctx, 0x1017d068, r.randrange(0, 9), r.getrandbits(8), r.getrandbits(6), r.getrandbits(5))
    w32(ctx, 0x1017d090, ctx.u32(), ctx.u32())
    w32(ctx, 0x1015c494, r.choice([5, 5, 0xffffffff]))
    # Proc_Exit(0): no at-exit list, empty C library tables, not exiting yet
    w32(ctx, 0x10ac4bc8, 0)
    wb(ctx, 0x1012d514, bytes(0x1012d724 - 0x1012d514))
    wb(ctx, 0x1012d828, bytes(0x1012d92c - 0x1012d828))
    w32(ctx, 0x1017e8d0, 0)
    w32(ctx, 0x1017e8cc, r.choice([0, 5]))
    return []


# ---- small 0x1009 functions -----------------------------------------------------------------------------
def e_toupper(uc, n, raw):
    c = raw[0] & 0xffffffff
    lo = c & 0xff
    h = mix(ST['seed'], n, 9)
    if 0x61 <= lo <= 0x7a:
        lo -= 0x20
    return ((h & 0xffff0000) if h % 3 == 0 else 0) | lo


EFFECT[0x100ff280] = e_toupper


def gst_handler(n, args, uc):
    """GetStringTypeA(locale, type, src, count, out): on success writes count words made from the source bytes."""
    esp = uc.reg_read(UC_X86_REG_ESP)
    raw = struct.unpack('<5I', bytes(uc.mem_read(esp + 4, 20)))
    ok = ST['gst'][n % len(ST['gst'])]
    if ok:
        cnt = raw[3] if raw[3] < 8 else 1
        src = bytes(uc.mem_read(raw[2], cnt))
        words = [mix(ST['seed'], *src, i) & 0xffff for i in range(cnt)]
        uc.mem_write(raw[4], struct.pack('<%dH' % cnt, *words))
    return ok


def g_upper(ctx):
    r = begin(ctx, 0x100ff280)
    ST['gst'] = [r.choice([1, 1, 1, 0]) for _ in range(3)]
    w32(ctx, 0x1015965c, r.choice([1, 1, 2, 2, 0, 3, -1]))
    w32(ctx, 0x1017e93c, 2)
    w32(ctx, 0x1017e950, ctx.u32())
    tab = newbuf(ctx, 0x204)
    w32(ctx, 0x10159450, tab + 0x102)
    n = r.choice([0, 1, 2, 3, 5, 8, 13, 30, r.randrange(0, 60)])
    s = bytes(r.choice(b'abcXYZ019 _.\x80\xe9\xff-') for _ in range(n))
    src = ctx.buf(n + 8, s + b'\0' + rb(ctx, 7))
    dst = newbuf(ctx, n + 8)
    return [dst, src]


def g_slab(ctx):
    r = begin(ctx)
    def coord(scale):
        return r.choice([fbits(r.uniform(-scale, scale)), fbits(0.0), fbits(-0.0), rfl(ctx, scale)])
    lo = r.uniform(-1000, 1000)
    hi = lo + r.choice([0.0, r.uniform(0, 500), r.uniform(0, 5)])
    p = r.choice([r.uniform(lo - 100, hi + 100), lo, hi, r.uniform(-3000, 3000)])
    d = r.choice([fbits(r.uniform(-2, 2)), fbits(0.0), fbits(-0.0), fbits(1e-8), fbits(-1e-8), fbits(1e-7),
                  fbits(-1.0000000116860974e-07), fbits(1.0000000116860974e-07), fbits(2e-7), coord(10)])
    if r.random() < 0.15:
        lo, hi = hi, lo
    return [fbits(p), d, fbits(lo), fbits(hi), newbuf(ctx, 4), newbuf(ctx, 4)]


def e_sprintf(uc, n, raw):
    uc.mem_write(raw[0], b' :\\video\\B51.x\0')


EFFECT[0x100fc780] = e_sprintf


def a_drives(n, args, uc):
    p = args[1]
    uc.mem_write(p, ST['drives'])
    return len(ST['drives'])


def a_drivetype(n, args):
    return ST['dtype']


def a_findclose(n, args):
    return ST['fc'][n % len(ST['fc'])]


def a_heapfree(n, args):
    return ST['hf']


DRIVE_API = {'HeapAlloc': a_heapalloc, 'GetLogicalDriveStringsA': a_drives, 'GetDriveTypeA': a_drivetype,
             'FindClose': a_findclose, 'HeapFree': a_heapfree}


def g_drive(ctx):
    r = begin(ctx, 0x100fc780)
    ctx.m.uc.mem_write(0x1017cc38, bytes([r.choice([0, 0, 0, 0x43, 0x44, 0xc4])]))
    ST['heap'] = [newbuf(ctx, 0x70)]
    ST['drives'] = r.choice([b'\0\0', b'C:\\\0\0', b'C:\\\0D:\\\0\0', b'E:\\\0\0', b'\0'])
    ST['dtype'] = r.choice([5, 3, 1])
    ST['fc'] = [r.choice([0, 1, 0xffffffff, r.getrandbits(32)]) for _ in range(2)]
    ST['hf'] = r.choice([0, 1, r.getrandbits(32)])
    w32(ctx, 0x1017b8f8, ctx.u32())
    w32(ctx, 0x1017cc3c, ctx.u32())
    return []


# ---- 0x1008 --------------------------------------------------------------------------------------------------
def grid_clear(ctx):
    wb(ctx, 0x1017ba28, bytes(9 * 0x18c))


def g_88a2d(ctx):
    r = begin(ctx)
    node_world(ctx)
    w32(ctx, 0x1017c8c4, r.choice([0, 1, 1, 1, 1]))
    w32(ctx, 0x1014b8e0, fbits(254008.0))
    w32(ctx, 0x1014b8e4, fbits(246044.5))
    grid_clear(ctx)
    for _ in range(r.randrange(0, 6)):
        cell_links(ctx, 0x1017ba28 + r.randrange(0, 9) * 0x18c + r.randrange(0, 9) * 0x2c)
    obj = newbuf(ctx, 0xa0)
    w16(ctx, obj, r.choice([0, 0, 0x1000, r.getrandbits(16)]))
    node = node_tree(ctx, r.choice([0, 1, 2]))
    w32(ctx, obj + 0x28, node)
    x = r.choice([r.uniform(-254008, 246000), r.uniform(-300000, 300000), -254008.0, 0.0, 55562.0 * r.randrange(-6, 6)])
    z = r.choice([r.uniform(-246044, 254000), r.uniform(-300000, 300000), 0.0, 55562.0 * r.randrange(-6, 6) - 0.5])
    wf(ctx, node + 0x70, x, r.uniform(-100, 100), z)
    col = int((x + 254008.0) / 55562.0 + 0.5)
    row = int((z + 246044.5) / 55562.0 + 0.5)
    w32(ctx, obj + 0x8c, r.choice([col, col, r.randrange(-1, 10)]) & 0xffffffff, r.choice([row, row, r.randrange(-1, 10)]) & 0xffffffff)
    if row in range(9) and col in range(9):
        cell_links(ctx, 0x1017ba28 + col * 0x18c + row * 0x2c)
        cell_links(ctx, 0x1017ba28 + row * 0x18c + col * 0x2c)
    return [0 if r.random() < 0.05 else obj]


def g_8d4f1(ctx):
    r = begin(ctx, 0x100ef727, 0x100e86a1)
    w32(ctx, 0x1017b940, r.choice([0, 1, 3]))
    wb(ctx, 0x101659e0, rb(ctx, 0x200))
    bmp = ctx.buf(0x20, struct.pack('<8i', *[r.randrange(-50, 700) for _ in range(8)]))
    if r.random() < 0.2:
        w32(ctx, bmp + 4, ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32())
    return [bmp, r.choice([0, 1, 0xff, r.randrange(0, 256), r.randrange(0, 256)])]


# ---- render -----------------------------------------------------------------------------------------------
def proj_globals(ctx):
    r = ctx.rng
    wide = r.random() < 0.7
    w32(ctx, 0x10185bd4, r.choice([320, 160, 0, r.randrange(-100, 700)]))
    w32(ctx, 0x10185bf4, r.choice([240, 100, 0, r.randrange(-100, 500)]))
    w32(ctx, 0x10185bd0, 0 if wide else r.randrange(-50, 300))
    w32(ctx, 0x10185bec, 639 if wide else r.randrange(100, 700))
    w32(ctx, 0x10185bf0, 0 if wide else r.randrange(-50, 200))
    w32(ctx, 0x10185be0, 479 if wide else r.randrange(100, 600))
    w32(ctx, 0x101c7680, r.getrandbits(8))
    w32(ctx, 0x101cc2a0, r.getrandbits(8))
    w32(ctx, 0x101c673c, r.choice([0, 5, 0xffffffff, r.getrandbits(32)]))


def pool_setup(ctx, cap=None):
    r = ctx.rng
    cap = cap if cap is not None else r.choice([4, 12, 40, 600, 600])
    buf = ctx.buf(0x40 * 620)
    top = buf + 0x40 * 620
    w32(ctx, 0x1017d174, top)
    w32(ctx, 0x1017d178, top - 0x40 * cap - 0xc8)


def view_matrix(ctx):
    import math
    r = ctx.rng
    c = r.random()
    if c < 0.5:
        m = [1, 0, 0, 0, 1, 0, 0, 0, 1]
    elif c < 0.75:
        a = r.uniform(-3, 3)
        m = [math.cos(a), 0, -math.sin(a), 0, 1, 0, math.sin(a), 0, math.cos(a)]
    else:
        m = [r.uniform(-2, 2) for _ in range(9)]
    wb(ctx, 0x10185ba0, struct.pack('<9f', *m))
    pos = [r.choice([0.0, 0.0, r.uniform(-50, 50)]) for _ in range(3)]
    wf(ctx, 0x10185c48, pos[0])
    wf(ctx, 0x10185c44, pos[1])
    wf(ctx, 0x10185c4c, pos[2])


def clip_env(ctx):
    r = ctx.rng
    proj_globals(ctx)
    pool_setup(ctx)
    view_matrix(ctx)
    w32(ctx, 0x101c9ea4, 1 if r.random() < 0.93 else 0)
    w32(ctx, 0x101c9ea8, r.choice([0, 0, 7, 300]))
    wf(ctx, 0x10185c50, r.uniform(-5, 5))
    wf(ctx, 0x10185bf8, r.choice([100.0, 1000.0, 5000.0]))


def g_89907(ctx):
    r = begin(ctx, fill=True)
    ST['dfields'] = (0, 1, 2, 3, 4, 5, 7, 8)
    clip_env(ctx)
    cam = camera(ctx)
    wf(ctx, cam + 0x3c, r.choice([0.5, 1.0, 4.0, 10.0, 0.0, -1.0, 100.0]))
    spread = r.choice([50.0, 300.0, 3000.0])
    x = r.uniform(-spread, spread)
    z = r.choice([r.uniform(-20, 60), r.uniform(-5, 1500), r.uniform(0, 120), r.uniform(500, 3000)])
    s = r.choice([1.0, 5.0, 20.0, 100.0, r.uniform(0, 300), 0.0])
    return [fbits(x), fbits(r.uniform(-spread, spread)), fbits(z), fbits(s), ctx.u32()]


def e_project(uc, n, raw):
    """0x100d2669(&x, &y, &z) projects in place: new x, y (from the values it is given), z kept."""
    x = bitsf(ur32(uc, raw[0]))
    y = bitsf(ur32(uc, raw[1]))
    uc.mem_write(raw[0], struct.pack('<f', f32(x * 0.5 + 160.0)))
    uc.mem_write(raw[1], struct.pack('<f', f32(120.0 - y * 0.25)))
    return ST['vis'][n % len(ST['vis'])]


EFFECT[0x100d2669] = e_project


def g_particles(ctx):
    r = begin(ctx, 0x100d2669, fill=True)
    ST['dfields'] = (0, 1, 3, 4, 7, 8)
    ST['vis'] = [r.choice([0, 1, 1, 1, 5]) for _ in range(5)]
    obj = newbuf(ctx, 0x580)
    particles(ctx, obj)
    w32(ctx, obj + 0x552, node_tree(ctx, 0))
    for off in (0x56e, 0x572, 0x576):
        w32(ctx, obj + off, rfl(ctx, 50))
    w32(ctx, obj + 0x566, ctx.u32())
    w32(ctx, obj + 0x56a, r.choice([0, 1, 2, 4, 8, -3, r.randrange(-100, 100)]) & 0xffffffff)
    return [0 if r.random() < 0.05 else obj]


def model_block(ctx, nverts, npolys):
    """A model block: +4 vertex count, +6 polygon count, +8 polygon offset, +0x10 stamp, vertices of 0x58 bytes
    from +0x18 (position +0..+8 sane), polygons of 0x28 bytes (normal +0xc..+0x14 sane), +0xc next record."""
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


def g_a4510(ctx):
    r = begin(ctx, 0x100a65e7, 0x100a9ba7)
    cam = camera(ctx)
    wf(ctx, cam + 0xb8, r.choice([1.0, 0.5, 2.0, r.uniform(0, 3)]))
    w32(ctx, 0x101668d0, 0)
    w32(ctx, 0x109afdcc, r.choice([0, 1]))
    for a in (0x109b16c0, 0x109b16c4, 0x109b16c8):
        w32(ctx, a, rfl(ctx, 1000))
    w32(ctx, 0x101c9ea4, r.choice([1, 1, 1, 0]))
    rets(0x100a65e7, 0)
    obj = newbuf(ctx, 0xc0)
    w16(ctx, obj, r.getrandbits(16))
    node = 0
    if r.random() < 0.97:
        node = zbuf(ctx, 0xa0)
        sane_floats(ctx, node + 0x4c, 12, 2)
    w32(ctx, obj + 0x28, node)
    chain = []
    for k in range(r.choice([0, 1, 1, 2, 3, 4])):
        blk = model_block(ctx, r.choice([0, 1, 2, 3]), r.choice([0, 1, 2, 3]))
        wf(ctx, blk, r.choice([0.0, 0.0, 50.0 * (k + 1), r.uniform(0, 500), 1000.0 * k]) if k == 0 else
           r.choice([50.0 * (k + 1), r.uniform(0, 500), 1000.0 * k]))
        w32(ctx, blk + 0x10, r.choice([0, 1, ctx.u32()]))
        if chain:
            w32(ctx, chain[-1] + 0xc, blk)
        chain.append(blk)
    w32(ctx, obj + 0x2c, chain[0] if chain else 0)
    w32(ctx, obj + 0x9c, r.choice([0, 1, ctx.u32()]))
    w32(ctx, obj + 0xb0, ctx.u32())
    rng = r.choice([0.0, 10.0, 100.0, 300.0, 1000.0, r.uniform(0, 2000), -5.0])
    return [0 if r.random() < 0.04 else obj, fbits(rng)]


def edge_obj(ctx, cached):
    """A scene vertex for Sim_100a5c10: world +0..+8, camera +0x20..+0x28, +0x10, +0x18/+0x1c, +0x40..+0x48
    (sane floats), cached projected vertex +0x2c (0 or one of the pool)."""
    r = ctx.rng
    o = newbuf(ctx, 0x60)
    sane_floats(ctx, o, 0x18, 500)
    wf(ctx, o + 0x28, r.choice([r.uniform(-20, 60), r.uniform(-5, 1500), 0.0, 1.0, r.uniform(0, 10)]))
    w32(ctx, o + 0x2c, cached if cached and r.random() < 0.3 else 0)
    return o


def g_edge(near):
    def g(ctx):
        r = begin(ctx)
        clip_env(ctx)
        cached = newbuf(ctx, 0x40)
        sane_floats(ctx, cached, 0x10, 500)
        a = edge_obj(ctx, None if near else cached)
        b = edge_obj(ctx, None if near else cached)
        if r.random() < 0.15:
            wf(ctx, b + 0x28, bitsf(r32(ctx, a + 0x28)) + r.choice([0.0, 1e-8, 1e-7, -1e-8]))
        wf(ctx, 0x10185c50, r.choice([1.0, 0.5, 4.0, r.uniform(-5, 50)]))
        wf(ctx, 0x10185bf8, r.choice([100.0, 1000.0, r.uniform(0, 1500)]))
        if near:
            sa = ctx.buf(8, struct.pack('<2i', r.randrange(-100, 700), r.randrange(-100, 500)))
            sb = ctx.buf(8, struct.pack('<2i', r.randrange(-100, 700), r.randrange(-100, 500)))
            return [a, r.choice([0, sa]), b, r.choice([0, sb])]
        return [a, b]
    return g


# ---- 0x100a nodes ----------------------------------------------------------------------------------------
def node_obj(ctx):
    """A scene node of 0xa0 bytes: pointer fields cleared, sane matrices +0xc..+0x8b."""
    n = newbuf(ctx, 0xc0)
    w32(ctx, n, 0, 0, 0)
    sane_floats(ctx, n + 0xc, 0x20, 2)
    w32(ctx, n + 0x8c, 0)
    return n


def scene_item(ctx):
    """The object a node keeps at +0x8c (as in spec_w6g.py) with sane floats +0x38..+0x88."""
    r = ctx.rng
    o = zbuf(ctx, 0x100)
    w16(ctx, o, r.choice([0, 0x800, 0x1000, 0x1800, 0x8000, 0xffff, r.getrandbits(16)]))
    w16(ctx, o + 2, r.choice([0, 0x70, 0x10, r.getrandbits(16)]))
    w16(ctx, o + 0x24, r.getrandbits(16))
    w16(ctx, o + 0x26, r.choice([0, 1, 2, 5]))
    w32(ctx, o + 0x34, r.choice([4, 0, 1]))
    sane_floats(ctx, o + 0x38, 0x14, 300)
    owner = zbuf(ctx, 0xc0)
    w32(ctx, owner + 0x70, rfl(ctx), rfl(ctx), rfl(ctx))
    w32(ctx, o + 0x28, owner)
    if r.random() < 0.5:
        anchor = zbuf(ctx, 0x100)
        w32(ctx, anchor + 8, o)
        w32(ctx, o + 4, anchor)
    return o


def obj_tree(ctx, depth, parent=0):
    r = ctx.rng
    node = node_obj(ctx)
    w32(ctx, node, parent)
    if r.random() < 0.6:
        w32(ctx, node + 0x8c, scene_item(ctx))
    w32(ctx, node + 0x94, r.choice([0, 1, 0x4000, 0x8000, 0xc001, r.getrandbits(32)]))
    kids = [obj_tree(ctx, depth - 1, node) for _ in range(r.choice([0, 1, 2, 3]))] if depth > 0 else []
    if kids:
        w32(ctx, node + 4, kids[0])
        for a, b in zip(kids, kids[1:]):
            w32(ctx, a + 8, b)
    return node


def records_world(ctx):
    r = ctx.rng
    w32(ctx, 0x1015ea80, r.choice([0, 0, 1, 2]))
    for i in range(2):
        rec = 0x109b9e20 + i * 0x7e
        w32(ctx, rec + 0x60, ctx.u32())
        w32(ctx, rec + 0x50, 0, 0, 0, 0)


def g_turn(ctx):
    r = begin(ctx, 0x10060b76)
    node = node_obj(ctx)
    w32(ctx, node + 0x94, r.choice([0, 1, 0x4000, 0x4001, r.getrandbits(32)]))
    w32(ctx, node + 0x98, r.choice([1, 1, 2, 0, 5, 0xffffffff]))
    w32(ctx, 0x1014d1a8, r.choice([0x40, 0x7f, 0xbf, 1, ctx.u32()]))
    m = ctx.buf(0x30, b''.join(struct.pack('<I', rfl(ctx, 2)) for _ in range(12)))
    return [node, m]


def g_aa64a(ctx):
    r = begin(ctx, 0x10060b76)
    parent = 0
    if r.random() < 0.6:
        parent = node_obj(ctx)
    node = obj_tree(ctx, r.choice([0, 1, 2]), parent)
    return [node]


def g_aaa08(ctx):
    r = begin(ctx, 0x10060b76, *NODE_STUBS)
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    parent = obj_tree(ctx, r.choice([0, 1]))
    w32(ctx, parent + 0x8c, scene_item(ctx))
    node = obj_tree(ctx, r.choice([0, 1]))
    w32(ctx, node + 0x8c, scene_item(ctx))
    if r.random() < 0.5:
        old = obj_tree(ctx, 1)
        kids = []
        k = r32(ctx, old + 4)
        while k:
            kids.append(k)
            k = r32(ctx, k + 8)
        kids.insert(r.randrange(0, len(kids) + 1), node)
        w32(ctx, old + 4, kids[0])
        for a, b in zip(kids, kids[1:] + [0]):
            w32(ctx, a + 8, b)
        w32(ctx, node, old)
    return [node, parent]


def g_aafe4(ctx):
    r = begin(ctx, 0x10060b76, *NODE_STUBS)
    records_world(ctx)
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    w32(ctx, 0x1017c8cc, 0)
    node = obj_tree(ctx, r.choice([0, 1, 2, 3])) if r.random() < 0.95 else 0
    return [node, r.choice([0, 1, 2, 5, 0x10001])]


def follower_node(ctx):
    r = ctx.rng
    node = obj_tree(ctx, r.choice([0, 1]))
    if r.random() < 0.9:
        item = scene_item(ctx)
        w32(ctx, node + 0x8c, item)
        if r.random() < 0.9:
            blk = model_block(ctx, r.choice([0, 1, 2, 3, 5, 8]), 0)
            for i in range(r16(ctx, blk + 4)):
                v = blk + 0x18 + i * 0x58
                wf(ctx, v, r.choice([r.uniform(-60000, 60000), r.uniform(-500, 500), 0.0]))
                wf(ctx, v + 8, r.choice([r.uniform(-60000, 60000), r.uniform(-500, 500), 0.0]))
            w32(ctx, item + 0x2c, blk)
    return node


def r16(ctx, addr):
    return struct.unpack('<h', bytes(ctx.m.uc.mem_read(addr, 2)))[0]


def g_a9300(ctx):
    r = begin(ctx, 0x10060b76)
    w32(ctx, 0x1017c8cc, r.choice([0, 0x1234]))
    node = follower_node(ctx)
    return [0 if r.random() < 0.03 else node]


def g_a94fa(ctx):
    r = begin(ctx, 0x10060b76, *NODE_STUBS)
    cam = camera(ctx)
    wf(ctx, cam, r.choice([r.uniform(-300000, 300000), r.uniform(-5000, 5000), 0.0]))
    wf(ctx, cam + 8, r.choice([r.uniform(-300000, 300000), r.uniform(-5000, 5000), 0.0]))
    step = r.choice([0x4000, 0x8000, 0xc000, 0x10000, 0x4000 * r.randrange(1, 20)])
    w32(ctx, 0x1017d1a4, step)
    w32(ctx, 0x1017d1a0, r.choice([(step + (step >> 3)) >> 1, r.randrange(0, 0x20000)]))
    w32(ctx, 0x1017d1b0, r.choice([1, 1, 1, 0]))
    w32(ctx, 0x1017d1a8, r.choice([1, 1, 1, 0]))
    w32(ctx, 0x1017d1b4, r.choice([0, 1, 1]))
    w32(ctx, 0x1017d1ac, r.choice([0, 1]))
    w32(ctx, 0x1017d1b8, obj_tree(ctx, r.choice([0, 1])) if r.random() < 0.9 else 0)
    if r32(ctx, 0x1017d1b8) == 0 and r.random() < 0.8:
        w32(ctx, 0x1017d1ac, 1)     # (else Sim_100aa39f(0) reads address 0x8c in both versions)
    gx = r.randrange(-20, 20)
    gz = r.randrange(-20, 20)
    w32(ctx, 0x1017d190, gx, gz, (gx * step) & 0xffffffff, (gz * step) & 0xffffffff)
    if r.random() < 0.3:
        w32(ctx, 0x1017d198, r.randrange(-300000, 300000), r.randrange(-300000, 300000))
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    return []


def e_celltest(uc, n, raw):
    """The cell tests (node, x, y, z, &out): store a height and answer hit or miss."""
    h = mix(ST['seed'], n, raw[0])
    uc.mem_write(raw[4], struct.pack('<f', f32(((h >> 8) % 4000) - 2000.0)))
    return h % 3 != 0


for _a in (0x100035df, 0x10001e1f, 0x10002775):
    EFFECT[_a] = e_celltest


def g_ac001(ctx):
    r = begin(ctx, 0x100035df, 0x10001e1f, 0x10002775, *NODE_STUBS)
    # ground: the grid is off, so Sim_1002d961 tests the node chain of the cell [0x1017c8cc] only
    w32(ctx, 0x1017c8c4, 0)
    cell = zbuf(ctx, 0x40)
    first = 0
    for _ in range(r.choice([0, 1, 2, 3])):
        nd = zbuf(ctx, 0x40)
        w32(ctx, nd + 0x34, r.choice([5, 8, 0, 1]))
        w32(ctx, nd + 0x18, first)
        first = nd
    w32(ctx, cell + 0x18, first)
    w32(ctx, 0x1017c8cc, cell)
    w32(ctx, 0x1036f2a4, 0)
    wf(ctx, 0x101112c8, r.choice([5.0, 50.0, 500.0]))
    w32(ctx, 0x1015dc80, r.choice([1, 2, 3, 0, 10, r.randrange(0, 100)]))
    # the attachment records 0x101c57e0 (0x34 bytes, index +0); the list [0x1017d1d0]/[0x1017d1d4]
    count = r.choice([0, 1, 2, 3, 5, 8])
    w32(ctx, 0x1017d1c8, count if r.random() < 0.95 else 0)
    w32(ctx, 0x1017d1c4, r.choice([-1, 0, 1, count - 1, 2]) & 0xffffffff)
    recs = []
    for i in range(count):
        e = 0x101c57e0 + i * 0x34
        wb(ctx, e, rb(ctx, 0x34))
        kind = r.choice([1, 2, 6, 6, 0, 3])
        w32(ctx, e, r.choice([i, i, i, r.randrange(-1, 9)]) & 0xffffffff)
        w32(ctx, e + 4, kind)
        src = newbuf(ctx, 0x80)
        sane_floats(ctx, src + 0x58, 0xa, 3000)
        holder = zbuf(ctx, 0x40)
        w32(ctx, holder + 0x3c, node_obj(ctx))
        sane_floats(ctx, r32(ctx, holder + 0x3c) + 0x70, 3, 3000)
        w32(ctx, e + 8, src if kind == 1 else holder)
        w32(ctx, e + 0x30, obj_tree(ctx, 0))
        lim = r.choice([0, 1, 10, 100, 1000, -5])
        w32(ctx, e + 0x20, lim & 0xffffffff)
        w32(ctx, e + 0x28, r.choice([0, lim - 1, lim, lim - 3, r.randrange(-10, 120)]) & 0xffffffff)
        w32(ctx, e + 0x24, rfl(ctx, 10))
        recs.append(e)
    r.shuffle(recs)
    for a, b in zip(recs, recs[1:] + [0]):
        w32(ctx, a + 0x10, b)
        if b:
            w32(ctx, b + 0xc, a)
    head = recs[0] if recs else 0
    if head:
        w32(ctx, head + 0xc, 0)
    w32(ctx, 0x1017d1d0, head if r.random() < 0.95 else 0)
    w32(ctx, 0x1017d1d4, recs[-1] if recs else r.choice([0, 1]))
    return []


# ---- 0x100a misc ------------------------------------------------------------------------------------------
CB_FREE, CB_ALLOC = CODE + 0x10, CODE + 0x20


def _cb_hooks(m):
    def free_cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        m.api_log.append(('free', ur32(uc, esp + 4)))
        uc.reg_write(UC_X86_REG_EAX, mix(ST['seed'], len(m.api_log)))

    def alloc_cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        n = len(m.api_log)
        m.api_log.append(('alloc', ur32(uc, esp + 4)))
        uc.reg_write(UC_X86_REG_EAX, ST['bufs'][n % len(ST['bufs'])])
    m.raw.hook_add(UC_HOOK_CODE, free_cb, None, CB_FREE, CB_FREE)
    m.raw.hook_add(UC_HOOK_CODE, alloc_cb, None, CB_ALLOC, CB_ALLOC)


def archive(ctx):
    r = begin(ctx, 0x101049c0, 0x10105fe0, 0x10105b50)
    m = ctx.m
    if not getattr(m, '_w7o_cb', False):
        m._w7o_cb = True
        _cb_hooks(m)
    m.uc.mem_write(CB_FREE, b'\xc3')
    m.uc.mem_write(CB_ALLOC, b'\xc3')
    w32(ctx, 0x1017d160, CB_FREE)
    w32(ctx, 0x1017d15c, CB_ALLOC)
    index = r.randrange(0, 4)
    rec = 0x101c65e0 + index * 0x14a
    wb(ctx, rec, rb(ctx, 0x14a))
    for i in range(0x40):
        w32(ctx, rec + 0x2a + 4 * i, r.choice([0, 0, ctx.u32()]))
    return r, index, rec


def g_a7c4c(ctx):
    r, index, rec = archive(ctx)
    return [index]


def g_a7e55(ctx):
    r, index, rec = archive(ctx)
    n = r.choice([0, 1, 2, 3, 5, 0x20])
    hdr = newbuf(ctx, 0xe + 0x18 * n + 8)
    w16(ctx, hdr + 0xc, n)
    for i in range(n):
        e = hdr + 0xe + i * 0x18
        w32(ctx, e + 4, r.choice([0, 0x100, ctx.u32()]), r.choice([0, 1, 16, 0x200, r.randrange(0, 0x3000)]))
    w32(ctx, rec + 6, hdr)
    w32(ctx, rec, r.choice([3, 4, 0xffffffff]))
    ST['bufs'] = [r.choice([newbuf(ctx, 0x40), newbuf(ctx, 0x40), 0]) for _ in range(3)]
    ST['short'] = r.choice([0, 0, 5, 30])
    return [index]


def e_read(uc, n, raw):
    """0x10105b50(handle, buffer, count): reads all of it, now and then less."""
    h = mix(ST['seed'], n, 3)
    return raw[2] if h % 100 >= ST['short'] else raw[2] // 2


EFFECT[0x10105b50] = e_read


def g_a4c45(ctx):
    r = begin(ctx, 0x100a539e)
    a = r.choice([0, 0, 1, 2, 5])
    b = r.choice([0, 0, 1, 3, 6])
    c = r.choice([0, 0, 1, 2, b])
    w32(ctx, 0x1017d134, a)
    w32(ctx, 0x101c9ea0, b)
    w32(ctx, 0x1017d130, c)
    tab = newbuf(ctx, 8 * 20)
    w32(ctx, 0x1017d168, tab)
    wb(ctx, 0x1014cae8, rb(ctx, 0x68))
    for k in range(0x30):
        w32(ctx, 0x10149d28 + 4 * k, ctx.u32())
    return []


def g_ab920(ctx):
    r = begin(ctx, 0x100d2306, 0x1007dd90)
    camera(ctx, 10)
    # (values whose (v << 16) / 0x13f or / 0xc7 fits 32 bits: beyond that the original raises a divide error)
    def fits():
        return r.choice([r.randrange(-10, 700), r.randrange(0, 0x10000), r.randrange(-0x630000, 0x630000)]) & 0xffffffff
    wb(ctx, 0x10185400, b''.join(struct.pack('<I', fits()) for _ in range(11 * 5 + 20)))
    w32(ctx, 0x1015241c, r.choice([0, 1, 2, 5, -1]))
    w32(ctx, 0x109e3e14, r.choice([319, 639, 1023, r.randrange(0, 2000)]))
    w32(ctx, 0x109e3de0, r.choice([199, 479, 767, r.randrange(0, 2000)]))
    pt = ctx.buf(0x20, struct.pack('<8I', *[fits() for _ in range(8)]))
    w32(ctx, 0x1014bfe4, pt)
    w32(ctx, 0x1014cb50, fits())
    for a in (0x1012dc80, 0x1012dc88, 0x1012dc90):
        w32(ctx, a, r.randrange(0, 0x10000), r.randrange(0, 0x10000))
    view = 0x109e3e20
    return []


def g_ae2e0(ctx):
    r = begin(ctx, 0x100fef50, 0x100fd820)
    w32(ctx, 0x1017e968, ctx.u32())
    rets(0x100fef50, r.choice([0, newbuf(ctx, 0x20), newbuf(ctx, 0x20)]))
    rets(0x100fd820, r.choice([0, 0x00A0F000, ctx.u32()]))
    return [ctx.buf(16, b'name.dat\0' + rb(ctx, 7))]


def spec(name, addr, gen, trials=400, **kw):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials)
    d.update(kw)
    return d


SPECS = [
    spec('Sim_10090f71', 0x10090f71, g_90f71, 500),
    spec('Sim_10091597', 0x10091597, g_91597, 600, void=True),
    spec('Sim_10092a71', 0x10092a71, g_92a71, 500),
    spec('Sim_10093ce7', 0x10093ce7, g_93ce7, 500),
    spec('Sim_10094423', 0x10094423, upd(0x21f), 500),
    spec('Sim_10094c2a', 0x10094c2a, upd(0x16a), 500),
    spec('Sim_1009524d', 0x1009524d, g_9524d, 500),
    spec('Sim_1009b99b', 0x1009b99b, g_modes, 600, void=True),
    spec('Sim_1009bc94', 0x1009bc94, g_modes, 600, void=True),
    spec('Cd_PollDisc', 0x1009e5d0, g_cdpoll, 600, api=dict(CD_API, GetStringTypeA=gst_handler)),
    spec('Str_CopyUpper', 0x1009ffad, g_upper, 500, api={'GetStringTypeA': gst_handler}),
    spec('Sim_1009cfd4', 0x1009cfd4, g_slab, 800),
    spec('Sim_10099510', 0x10099510, g_drive, 400, api=DRIVE_API),
    spec('Sim_10088a2d', 0x10088a2d, g_88a2d, 600, void=True),
    spec('Sim_1008d4f1', 0x1008d4f1, g_8d4f1, 400, void=True),
    spec('Sim_10089907', 0x10089907, g_89907, 600, void=True),
    spec('Sim_10094d7f', 0x10094d7f, g_particles, 400),
    spec('Sim_10095453', 0x10095453, g_particles, 400),
    spec('Sim_100a4510', 0x100a4510, g_a4510, 600),
    spec('Render_ClipEdgeNear', 0x100a5d38, g_edge(True), 800),
    spec('Render_ClipEdgeFar', 0x100a6022, g_edge(False), 800),
    spec('Sim_100a9ec6', 0x100a9ec6, g_turn, 500, void=True),
    spec('Sim_100a9f6e', 0x100a9f6e, g_turn, 500, void=True),
    spec('Sim_100aa64a', 0x100aa64a, g_aa64a, 500, void=True),
    spec('Sim_100aaa08', 0x100aaa08, g_aaa08, 500, void=True),
    spec('Sim_100aafe4', 0x100aafe4, g_aafe4, 500, void=True),
    spec('Sim_100a9300', 0x100a9300, g_a9300, 500, void=True),
    spec('Sim_100a94fa', 0x100a94fa, g_a94fa, 600, void=True),
    spec('Sim_100ac001', 0x100ac001, g_ac001, 600, void=True),
    spec('Sim_100a7c4c', 0x100a7c4c, g_a7c4c, 400, void=True),
    spec('Sim_100a7e55', 0x100a7e55, g_a7e55, 500),
    spec('Sim_100a4c45', 0x100a4c45, g_a4c45, 400, void=True),
    spec('Sim_100ab920', 0x100ab920, g_ab920, 400, void=True),
    spec('File_OpenRead', 0x100ae2e0, g_ae2e0, 400),
]
