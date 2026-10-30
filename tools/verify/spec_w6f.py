# Differential tests for round 6, letter f (see difftest.py and docs/checker.md).
#
# Callees that are converted run as real code on both sides (the C callee for the C run, the original for the
# original run); both are verified already. Callees that are not converted (or that another hook file patches)
# are called through their original address by both versions. Some of them run for real in the emulator (the C
# library's sqrt and _strcmpi, 0x100fcb70, 0x10060656); the others are replaced by a stub: an emulator code
# hook at the callee's address that appends a record (callee id, its arguments, and for some the memory an
# argument points to) to a log in argument memory, answers with a value chosen by the generator and returns to
# the caller by setting EIP/ESP (no code bytes are patched). The log is compared like all memory. Pointers into
# the stack are logged as a marker. Windows imports (HeapFree) are stubbed by the checker itself.
# The function that returns a double in st(0) is called through a machine-code trampoline (original) or a C
# wrapper (w6f_test.c) that stores st(0) at OUTF.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG

SOURCES = [
    'sim/unsorted_1008/w6f_cells.c', 'sim/unsorted_1008/w6f_display.c', 'sim/unsorted_1008/w6f_camera.c',
    'sim/unsorted_1009/w6f_effects.c', 'sim/unsorted_1009/w6f_modes.c',
    '../tools/verify/w6f_test.c',
    # converted callees and what they need
    'sim/view/w2l_lists.c', 'sim/view/objlist.c', 'sim/unsorted_1000/w5z_calls.c', 'sim/win/w4a_heap.c',
    'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_1008/w5y_misc.c', 'sim/unsorted_1008/w3q_place.c',
    'sim/unsorted_100d/state.c', 'sim/unsorted_100d/w5c_calls.c', 'sim/unsorted_1003/w2b_misc.c',
    'sim/util/field.c', 'sim/unsorted_100a/w5y_misc.c', 'sim/unsorted_100a/w5x_calls.c', 'sim/unsorted_100a/w5b_calls.c',
    'sim/unsorted_1000/w2e_names.c', 'sim/unsorted_1009/w2g_objects.c', 'sim/unsorted_1000/w5x_calls.c',
    'sim/util/ring.c', 'sim/unsorted_100a/w3t_misc.c', 'sim/unsorted_1008/w3o_length.c',
    'sim/unsorted_1009/w5y_misc.c', 'sim/unsorted_1009/w2f_misc.c', 'sim/unsorted_1000/w5a_misc.c',
    'sim/unsorted_1002/w5a_misc.c', 'sim/view/w2d_view.c',
    # what those need in turn (other functions of the same files)
    'sim/view/matrix.c', 'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_1003/w2e_mono.c', 'sim/view/viewrect.c',
    'sim/unsorted_1001/w2e_names.c', 'sim/unsorted_1001/w5y_misc.c', 'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1002/w2l_tables.c',
    'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1003/w5a_misc.c',
    'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1008/cell_link.c', 'sim/unsorted_1008/w3s_grid.c', 'sim/unsorted_100a/w5z_calls.c',
    'sim/unsorted_100a/w2k_list.c', 'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_100d/w2g_misc.c', 'sim/unsorted_1000/w3s_model.c',
    'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2l_fields.c', 'sim/unsorted_1001/w2j_misc.c', 'sim/unsorted_1001/w4u_cd.c',
    'sim/unsorted_1001/w2e_tables.c', 'sim/unsorted_1002/w5y_misc.c', 'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_1003/w2e_misc.c',
    'sim/unsorted_1007/w5a_misc.c', 'sim/unsorted_1007/w2e_state.c', 'sim/win/w4u_cdaudio.c', 'sim/unsorted_1000/w2j_slots.c',
    'sim/unsorted_1002/w2e_object.c', 'sim/core/w2a_timers.c', 'sim/unsorted_100c/w2e_tables.c', 'sim/unsorted_100d/w2j_nodes.c',
    'sim/util/lookup.c', 'sim/core/timer.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
LOG = 0x00AFE000        # count, then records of RECW dwords
LOGMAX = 40
RECW = 20
OUTF = 0x00AFF000       # the double result of 0x10084b8c
TRAMP = 0x00F00900      # trampoline for the original 0x10084b8c (in the page of the return address, never compared)

# stubbed callees: address -> (number of arguments, {deref: {arg index: dwords}})
STUBS = {
    0x100aa742: (1, {}), 0x10088a2d: (1, {}), 0x1000e737: (12, {}), 0x1002feef: (1, {}),
    0x100ed743: (2, {}), 0x100e90a3: (6, {}), 0x10099c59: (0, {}),
    0x100988fc: (2, {}), 0x100989a1: (2, {}), 0x100b5bcb: (2, {}),
    0x100d2306: (1, {'deref': {0: 0x40}}), 0x100d1f90: (1, {'deref': {0: 0x40}}),
    0x100ac603: (1, {}), 0x100585c4: (1, {}), 0x10090d15: (1, {}), 0x1001f790: (3, {}),
}
IDS = {a: i + 1 for i, a in enumerate(sorted(STUBS))}
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def pick(ctx, *choices):
    return ctx.rng.choice(choices)


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def rfl(ctx, scale=1000.0):
    """A float as a bit pattern: ordinary values, zeros, small and large, both signs (no NaN)."""
    r = ctx.rng
    c = r.random()
    if c < 0.55:
        return fbits(r.uniform(-scale, scale))
    if c < 0.75:
        return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, 2.0, 16.0, 100.0, 256.0, -3.0, 5.0, 1e-7, 1e-30]))
    if c < 0.9:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-12, 12))
    return r.getrandbits(32) & 0xbf7fffff         # exponent below the NaN/inf range


# ---- stubs --------------------------------------------------------------------------------------
def _install(m):
    def make(addr):
        nargs, opt = STUBS[addr]

        def cb(uc, address, size, ud):
            esp = uc.reg_read(UC_X86_REG_ESP)
            a = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
            n = struct.unpack('<I', bytes(uc.mem_read(LOG, 4)))[0]
            rec = [IDS[addr]] + [0x5afe0000 + i if STACK_LO <= v <= STACK_HI else v for i, v in enumerate(a)]
            for i, nd in opt.get('deref', {}).items():
                try:
                    rec += list(struct.unpack('<%dI' % nd, bytes(uc.mem_read(a[i], 4 * nd))))
                except Exception:
                    rec += [0xdeaddead]
            if addr == 0x10099c59 and ST.get('watch'):
                rec += list(struct.unpack('<5I', bytes(uc.mem_read(ST['watch'], 20))))
            rets = ST['ret'].get(addr) or [0]
            if n < LOGMAX:
                rec = (rec + [0] * RECW)[:max(RECW, len(rec))]
                uc.mem_write(LOG + 4 + n * RECW * 4, struct.pack('<%dI' % RECW, *rec[:RECW]))
                if len(rec) > RECW:         # long records (memory dumps) go to their own area
                    uc.mem_write(LOG + 0x800 + n * 0x40 * 4, struct.pack('<%dI' % min(0x40, len(rec) - RECW), *rec[RECW:RECW + 0x40]))
            uc.mem_write(LOG, struct.pack('<I', n + 1))
            uc.reg_write(UC_X86_REG_EAX, rets[n % len(rets)] & 0xffffffff)
            uc.reg_write(UC_X86_REG_EIP, struct.unpack('<I', bytes(uc.mem_read(esp, 4)))[0])
            uc.reg_write(UC_X86_REG_ESP, esp + 4)
        return cb

    for addr in STUBS:
        m.raw.hook_add(UC_HOOK_CODE, make(addr), None, addr, addr)


def tramp_double(target, nargs):
    """Calls target(args as dwords) and stores st(0) as a double at OUTF; returns 0."""
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\xdd\x1d' + struct.pack('<I', OUTF)
    code += b'\x31\xc0\x5d\xc3'
    return code


def begin(ctx):
    """Installs the stubs (once), sets the x87 state like the Windows control word, clears the log and
    chooses the answers of the stubs."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w6f', False):
        m._w6f = True
        _install(m)
        m.raw.mem_write(TRAMP, tramp_double(0x10084b8c, 3))
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    m.uc.mem_write(LOG, bytes(0x1000))
    ST['ret'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(4)] for a in STUBS}
    ST['watch'] = None
    # the random number table of Sim_1009c929 and its position
    w32(ctx, 0x1017cd38, r.randrange(0, 0x7f))
    w32(ctx, 0x101cffc0, *[r.choice([r.randrange(0, 1000), r.getrandbits(32), r.randrange(-50, 50)]) for _ in range(0x7f)])
    return r


# ---- shared worlds ------------------------------------------------------------------------------
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
    if r.random() < 0.95:      # the owner (read by Sim_1008a46e when the grid is on)
        own = newbuf(ctx, 0x80)
        w32(ctx, own, r.choice([0, 0, 1]))
        w32(ctx, own + 0x70, rfl(ctx, 3000), rfl(ctx, 3000), rfl(ctx, 3000))
        w32(ctx, obj + 0x28, own)
    else:
        w32(ctx, obj + 0x28, 0)
    w32(ctx, obj + 0x2c, 0)
    return obj


def node_tree(ctx, depth, p_obj=0.7):
    """Scene node: first child +4, next sibling +8, object +0x8c (or 0)."""
    r = ctx.rng
    node = zbuf(ctx, 0xa0)
    wb(ctx, node + 0x30, rb(ctx, 0x60))
    w32(ctx, node, r.choice([0, 0, 1]))
    w32(ctx, node + 0x8c, scene_obj(ctx) if r.random() < p_obj else 0)
    kids = [node_tree(ctx, depth - 1, p_obj) for _ in range(r.choice([0, 1, 1, 2, 3]))] if depth > 0 else []
    if kids:
        w32(ctx, node + 4, kids[0])
        for a, b in zip(kids, kids[1:]):
            w32(ctx, a + 8, b)
    return node


def cell_links(ctx, cell):
    """Links of a cell (the anchor of Sim_1008a8f5): 0 or spare buffers."""
    r = ctx.rng
    for off in (4, 8, 0x14, 0x18, 0x1c, 0x20):
        w32(ctx, cell + off, r.choice([0, 0, spare(ctx)]))
    w16(ctx, cell, r.choice([0, 0, 0x800]))
    w32(ctx, cell + 0x28, 0)


def grid_clear(ctx):
    wb(ctx, 0x1017ba28, bytes(9 * 0x18c))


def name_table(ctx):
    """The table of names at 0x10a39200 (count 0x10a38de0), sometimes holding the names the code uses."""
    r = ctx.rng
    w32(ctx, 0x10a38de0, r.choice([0, 1, 2, 3, 5, 8]))
    for i in range(12):
        nm = bytes(r.choice(b'abcAFXWL') for _ in range(r.randrange(0, 9)))
        if r.random() < 0.15:
            nm = b'AFXWLLLC'
        wb(ctx, 0x10a39200 + i * 0x14, struct.pack('<I', r.choice([0, 1, 2])) + nm + b'\0' + rb(ctx, 15 - len(nm)))
    w32(ctx, 0x101321b4, ctx.u32())
    for i in range(16):
        w32(ctx, 0x1012df30 + 4 * i, ctx.u32())


def node_world(ctx):
    """The flags the node helpers read."""
    w32(ctx, 0x1017c8c4, pick(ctx, 0, 0, 1))
    w32(ctx, 0x1017c8cc, pick(ctx, 0, 0x1234))
    w32(ctx, 0x1014b8e0, fbits(ctx.rng.choice([100.0, 1000.0, 250.0])))
    w32(ctx, 0x1014b8e4, fbits(ctx.rng.choice([100.0, 1000.0, 250.0])))
    for c in (0x1017b988, 0x1017b9b0, 0x1017b9d8):
        cell_links(ctx, c)


# ---- generators: 0x1008 -------------------------------------------------------------------------
def g_84b8c(ctx):
    begin(ctx)
    return [rfl(ctx), rfl(ctx), rfl(ctx)]


def g_86d8d(ctx):
    r = begin(ctx)
    name = r.choice([b'MCGA.DLL', b'mcga.dll', b'VESA480.DLL', b'vesa480.dll', b'Vesa480.Dll', b'VESA768.DLL',
                     b'vesa768.dll', b'VESA.DLL', b'VESA480.DL', b'VESA768.DLLX', b'', b'MCGA', b'xyz',
                     bytes(r.choice(b'VESA4807.DLmcg') for _ in range(r.randrange(0, 13)))])
    w32(ctx, 0x1017b918, ctx.u32(), ctx.u32())
    return [ctx.buf(len(name) + 8, name + b'\0' + rb(ctx, 7))]


def g_88986(ctx):
    r = begin(ctx)
    node_world(ctx)
    return [0 if r.random() < 0.08 else node_tree(ctx, r.choice([0, 1, 2, 3]))]


def g_88b24(ctx):
    r = begin(ctx)
    node_world(ctx)
    grid_clear(ctx)
    for _ in range(r.randrange(0, 6)):
        cell_links(ctx, 0x1017ba28 + r.randrange(0, 9) * 0x18c + r.randrange(0, 9) * 0x2c)
    row = r.choice([0, 1, 4, 8, r.randrange(0, 9), r.randrange(0, 9), -1, 9])
    col = r.choice([0, 2, 7, 8, r.randrange(0, 9), r.randrange(0, 9), -1, 9])
    if row in (-1, 9) or col in (-1, 9):
        cell_links(ctx, 0x1017ba28 + row * 0x18c + col * 0x2c)
    return [0 if r.random() < 0.06 else node_tree(ctx, r.choice([0, 1, 2, 3])), row & 0xffffffff, col & 0xffffffff]


def chain_blocks(ctx, n):
    head = 0
    for _ in range(n):
        b = newbuf(ctx, 0x20)
        w32(ctx, b + 0xc, head)
        head = b
    return head


def cell_list(ctx, cell, n):
    """A list of n objects hanging from cell+8 (prev +4, next +8); now and then a node that does not point
    back at its predecessor (the head then stays and the loop stops)."""
    r = ctx.rng
    prev = cell
    for _ in range(n):
        obj = scene_obj(ctx, links=False)
        w16(ctx, obj, 0)
        w32(ctx, obj + 0x2c, chain_blocks(ctx, r.choice([0, 0, 1, 2])))
        w32(ctx, obj + 0xc, r.choice([0, 0, spare(ctx)]))
        w32(ctx, obj + 0x10, r.choice([0, 0, spare(ctx)]))
        w32(ctx, prev + 8, obj)
        w32(ctx, obj + 4, prev if r.random() < 0.93 else r.choice([0, spare(ctx)]))
        prev = obj
    w32(ctx, prev + 8, 0)


def g_8a5e3(ctx):
    r = begin(ctx)
    grid_clear(ctx)
    w32(ctx, 0x1017c8c4, pick(ctx, 0, 1, 1))
    w32(ctx, 0x1017b8f8, r.choice([0x00c10000, ctx.u32()]))
    for c in (0x1017b988, 0x1017b9b0, 0x1017b9d8, 0x1017c818, 0x1017ba00):
        wb(ctx, c, bytes(0x2c))
        cell_list(ctx, c, r.choice([0, 0, 1, 2, 3]))
    for _ in range(r.choice([0, 1, 3, 6])):
        cell_list(ctx, 0x1017ba28 + r.randrange(0, 9) * 0x18c + r.randrange(0, 9) * 0x2c, r.choice([1, 2, 3]))
    cur = r.choice([0x1017b988, 0x1017b988, 0, 0x1017b9b0])
    if r.random() < 0.1:
        cur = r32(ctx, 0x1017b9e0) or cur      # an object that Sim_1008a16d refuses to unlink
    w32(ctx, 0x1017c8cc, cur)
    return []


def g_8d2d0(ctx):
    r = begin(ctx)
    rect = ctx.buf(0x20, struct.pack('<8i', *[r.randrange(-50, 400) for _ in range(8)]))
    if r.random() < 0.85:   # a sane rectangle: right >= left, bottom >= top
        x0, y0 = r.randrange(-20, 300), r.randrange(-20, 200)
        w32(ctx, rect + 4, x0, y0, x0 + r.choice([0, 1, 5, r.randrange(0, 640)]), y0 + r.choice([0, 1, 7, r.randrange(0, 480)]))
    else:
        w32(ctx, rect + 4, ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32())
    w = r.choice([0, 1, 2, 64, 320, 640, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)])
    h = r.choice([0, 1, 2, 48, 200, 480, 0x7fff, 0x8000, 0x8001, 0xffff, r.getrandbits(16)])
    ST['ret'][0x100ed743] = [(w << 16) | h]
    out = newbuf(ctx, 0x20)
    return [rect, out, ctx.u32(), r.randrange(0, 20)]


def g_8e02b(ctx):
    r = begin(ctx)
    now = ctx.u32()
    w32(ctx, 0x1015dc78, now)
    w32(ctx, 0x1017cc10, (now - r.choice([0, 1, 0x59, 0x5a, 0x5b, 200, -5, r.getrandbits(32)])) & 0xffffffff)
    w32(ctx, 0x1014b9b0, r.choice([0xef, 0xf0, 0xf1, 0xf6, 0xf7, 0xf8, 0xf3, r.getrandbits(32)]))
    w32(ctx, 0x1017cc14, r.choice([1, -1, 0, 2, -2, r.getrandbits(32)]))
    w32(ctx, 0x1017cc0c, r.choice([0, 1, 2, 3, -1, -2, -3, 0x7fffffff, 0x80000000, r.getrandbits(32)]))
    bmp = newbuf(ctx, 0x40)
    w32(ctx, bmp + 0xc, r.choice([r.randrange(0, 640), ctx.u32()]), r.choice([r.randrange(0, 480), ctx.u32()]))
    return [bmp]


def g_8e266(ctx):
    r = begin(ctx)
    rect = ctx.buf(0x14, struct.pack('<5i', *[r.randrange(-100, 700) for _ in range(5)]))
    if r.random() < 0.8:
        x0, y0 = r.randrange(-50, 100), r.randrange(-50, 100)
        w32(ctx, rect + 4, x0, y0, x0 + r.choice([0, 1, 2, r.randrange(0, 640)]), y0 + r.choice([0, 1, 3, r.randrange(0, 480)]))
    else:
        w32(ctx, rect + 4, ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32())
    hw = (r32(ctx, rect + 0xc) - r32(ctx, rect + 4) + 1) & 0xffffffff
    px = r.choice([r.randrange(-100, 700), (hw >> 1) if hw < 0x80000000 else 0, ctx.u32()])
    pt = ctx.buf(8, struct.pack('<2I', px & 0xffffffff, r.choice([r.randrange(-100, 500), ctx.u32()]) & 0xffffffff))
    return [rect, pt, newbuf(ctx, 8)]


def camera(ctx):
    """The camera [0x1014cae0] (position +0..+8, angles +0xc..+0x14, range +0x40) with random floats."""
    r = ctx.rng
    cam = ctx.buf(0x140, b''.join(struct.pack('<I', rfl(ctx)) for _ in range(0x50)))
    w32(ctx, 0x1014cae0, cam)
    return cam


def g_8fb20(ctx):
    r = begin(ctx)
    camera(ctx)
    wb(ctx, 0x10185400, b''.join(struct.pack('<I', r.choice([r.randrange(-10, 700), r.randrange(0, 4), ctx.u32()]) & 0xffffffff) for _ in range(11 * 5 + 20)))
    w32(ctx, 0x1015241c, r.choice([0, 1, 2, 5, -1]))
    wb(ctx, 0x1014cae8, rb(ctx, 0x68))
    w32(ctx, 0x1017e650, ctx.u32())
    w32(ctx, 0x1017d14c, ctx.u32())
    pos = ctx.buf(0x18, b''.join(struct.pack('<I', rfl(ctx)) for _ in range(6)))
    preset = r.choice([0, 1, 2, 3, 5, 9, 10, r.randrange(0, 11), -1, 11, 12])
    scale = r.choice([0, 1, 100, 256, 320, -50, r.randrange(-1000, 1000), ctx.u32()])
    depth = r.choice([0, 1, 100, 5000, r.randrange(-1000, 100000), ctx.u32()])
    return [pos, preset & 0xffffffff, scale & 0xffffffff, depth & 0xffffffff]


def g_8fd5a(ctx):
    r = begin(ctx)
    camera(ctx)
    wb(ctx, 0x10185400, b''.join(struct.pack('<I', r.choice([r.randrange(-10, 700), ctx.u32()]) & 0xffffffff) for _ in range(5)))
    w32(ctx, 0x1015241c, r.choice([0, 1, 2, -1]))
    wb(ctx, 0x1036f1e0, rb(ctx, 0x68))
    wb(ctx, 0x1036f0e0, b''.join(struct.pack('<I', rfl(ctx)) for _ in range(0x38)))
    w32(ctx, 0x1036f078, ctx.u32())
    w32(ctx, 0x1036f0cc, ctx.u32())
    return []


# ---- generators: effects ------------------------------------------------------------------------
def effect_obj(ctx, limit, rec=None):
    """An effect object with a node tree at +0x552, age around its limit."""
    r = ctx.rng
    obj = newbuf(ctx, 0x580)
    w32(ctx, obj + 0x552, node_tree(ctx, r.choice([0, 0, 1, 2]), 0.8))
    now = ctx.u32()
    w32(ctx, 0x1015dc78, now)
    lim = limit if limit is not None else r.choice([0, 1, 10, 0x5a, r.randrange(-5, 300), ctx.u32()])
    if limit is None:
        w32(ctx, obj + 0x562, lim)
    lim = lim if lim < 0x80000000 else lim - (1 << 32)
    age = r.choice([lim, lim + 1, lim - 1, 0, -1, r.randrange(-3, max(lim, 0) + 4), r.randrange(-3, max(lim, 0) + 4),
                    r.randrange(-3, max(lim, 0) + 4), r.randrange(-3, max(lim, 0) + 4), r.randrange(-300, 0),
                    r.randrange(-3, max(lim, 0) + 4), r.getrandbits(32) if r.random() < 0.3 else lim + 2])
    w32(ctx, obj + 0x55e, (now - age) & 0xffffffff)
    for off in (0x56e, 0x572, 0x576):
        w32(ctx, obj + off, rfl(ctx, 50))
    w16(ctx, obj + 0x10, r.getrandbits(16))
    return obj


def g_upd_117d(ctx):
    r = begin(ctx)
    node_world(ctx)
    obj = effect_obj(ctx, None)
    w32(ctx, obj + 0x57a, newbuf(ctx, 0x100))
    return [obj]


def g_upd_rec(ctx):
    r = begin(ctx)
    node_world(ctx)
    name_table(ctx)
    w32(ctx, 0x1015d0a4, 0 if r.random() < 0.3 else newbuf(ctx, 0x320))
    obj = effect_obj(ctx, None)
    w32(ctx, obj + 0x57a, newbuf(ctx, 0x20))
    return [obj]


def upd(limit):
    def g(ctx):
        begin(ctx)
        node_world(ctx)
        return [effect_obj(ctx, limit)]
    return g


def pool_world(ctx):
    """The effect pool of Sim_10090c7d (as in spec_w2g.py) with node trees, the counts and the camera."""
    r = ctx.rng
    n = 6
    w32(ctx, 0x10215ae8, r.choice([-1, 0, r.randrange(0, n), r.randrange(0, n), r.randrange(0, n), r.randrange(0, n), n - 1, n - 1]))
    w32(ctx, 0x10216b80, *[r.randrange(0, n) for _ in range(n + 2)])
    w32(ctx, 0x1014b9b8, r.choice([0, 1, 1, 1, 1, 1, ctx.u32()]))
    for i in range(n):
        o = 0x10217c20 + 0x57e * i
        wb(ctx, o, rb(ctx, 0x57e))
        node = node_tree(ctx, r.choice([0, 0, 1]), 1.0)
        w32(ctx, o + 0x552, node)
    mode = r.random()
    if mode < 0.5:
        w32(ctx, 0x1017cc28, 0, 0)
    else:
        last = newbuf(ctx, 16)
        w32(ctx, 0x1017cc28, newbuf(ctx, 16), last)
    limit = r.randrange(0, 40)
    w32(ctx, 0x1017cc20, limit)
    w32(ctx, 0x1017cc24, r.choice([limit - 2, limit - 3, limit - 5, r.randrange(-1, 20), limit - 1, 0, 0, 0xffffffff]))
    w32(ctx, 0x1015dc78, ctx.u32())


def g_92956(ctx):
    r = begin(ctx)
    node_world(ctx)
    name_table(ctx)
    pool_world(ctx)
    rec = newbuf(ctx, 0x40)
    return [ctx.u32(), rec, ctx.u32()]


def spawn_args(ctx):
    """(x, y, z) near the camera (or far), (sx, sy, sz); the camera range +0x40."""
    r = ctx.rng
    cam = camera(ctx)
    near = r.random() < 0.85
    pos = []
    for k in range(3):
        c = struct.unpack('<f', struct.pack('<I', r32(ctx, cam + 4 * k)))[0]
        if abs(c) > 1e6:
            c = r.uniform(-1000, 1000)
            w32(ctx, cam + 4 * k, fbits(c))
        pos.append(fbits(c + r.uniform(-50, 50)) if near else rfl(ctx, 5000))
    w32(ctx, cam + 0x40, fbits(r.choice([100.0, 60.0, 200.0, 0.0, 1e9, 1e9, r.uniform(0, 300)])))
    return pos + [rfl(ctx, 400), rfl(ctx, 400), rfl(ctx, 400)]


def g_spawn(ctx):
    begin(ctx)
    node_world(ctx)
    pool_world(ctx)
    return spawn_args(ctx)


def g_spawn_939a3(ctx):
    return g_spawn(ctx) + [ctx.rng.choice([0, 1, 2, 0, 1, 2, 0, 1, 2, 3, -1, ctx.u32()]) & 0xffffffff]


# ---- generators: modes --------------------------------------------------------------------------
def g_98721(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015d1b8, 0 if r.random() < 0.1 else newbuf(ctx, 0x80))
    return [ctx.u32(), ctx.u32()]


def g_999fc(ctx):
    r = begin(ctx)
    name_table(ctx)
    w32(ctx, 0x1015d1ac, r.choice([0, 1, 2, 3, 4, 5, -1, 4, 4]))
    w32(ctx, 0x1015d1cc, r.choice([0, 1, 2, 3, 4, 5, 3, 5, 6, 0xffffffff]))
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
        nm = r.choice([b'', b'', b'AFXWLLLC', b'abc', b'snd1'])
        w32(ctx, rec + off, ctx.buf(len(nm) + 4, nm + b'\0' + rb(ctx, 3)))
    w32(ctx, 0x1014c4d8, rec)
    return []


def g_9b8d8(ctx):
    r = begin(ctx)
    cur = newbuf(ctx, 0x20)
    save = newbuf(ctx, 0x20)
    view = ctx.buf(8, struct.pack('<2I', cur, save))
    ST['watch'] = cur
    n = newbuf(ctx, 0x10)
    o = newbuf(ctx, 0x20)
    # (the converted Sim_1008f747 does not raise the original's divide error when (n+8 << 16) / o[0] does not
    # fit 32 bits or o[0] is 0, so the interpolation numbers stay in the range where neither does)
    w32(ctx, n, r.choice([1, 1, 0, 2]))
    w32(ctx, n + 4, r.choice([0, 0, 1]))
    w32(ctx, n + 8, r.randrange(0, 0x2000))
    w32(ctx, o, r.choice([0x100, 0x400, 0x1000, 0x8000, 0x10000]))
    for off in (4, 8, 0xc):
        w32(ctx, o + off, newbuf(ctx, 0x20))
    w32(ctx, 0x1015dc80, r.randrange(0, 0x200))
    pair = ctx.buf(8, struct.pack('<2I', n, o))
    return [r.choice([0, 1, 0, 5]), view, pair, r.choice([0, 1, 1, 7])]


def spec(name, addr, gen, trials=300, **kw):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials)
    d.update(kw)
    return d


SPECS = [
    dict(name='Sim_10084b8c', orig=TRAMP, c='Test_10084b8c', gen=g_84b8c, trials=500),
    spec('Display_SizeFromDriverName', 0x10086d8d, g_86d8d, 400, void=True),
    spec('Sim_10088986', 0x10088986, g_88986, 500, void=True),
    spec('Sim_10088b24', 0x10088b24, g_88b24, 500, void=True),
    spec('Sim_1008a5e3', 0x1008a5e3, g_8a5e3, 500, void=True),
    spec('Sim_1008d2d0', 0x1008d2d0, g_8d2d0, 500),
    spec('Sim_1008e02b', 0x1008e02b, g_8e02b, 500),
    spec('Sim_1008e266', 0x1008e266, g_8e266, 500),
    spec('Sim_1008fb20', 0x1008fb20, g_8fb20, 400, void=True),
    spec('Sim_1008fd5a', 0x1008fd5a, g_8fd5a, 300, void=True),
    spec('Sim_1009117d', 0x1009117d, g_upd_117d, 400),
    spec('Sim_100917ed', 0x100917ed, g_upd_rec, 400),
    spec('Sim_10092117', 0x10092117, g_upd_rec, 400),
    spec('Sim_10093295', 0x10093295, upd(0x5a), 500),
    spec('Sim_10093376', 0x10093376, upd(0x5a), 500),
    spec('Sim_10093457', 0x10093457, upd(0x5a), 500),
    spec('Sim_1009589c', 0x1009589c, upd(0x16), 500),
    spec('Sim_10096095', 0x10096095, upd(0x2d), 500),
    spec('Sim_100967cc', 0x100967cc, upd(0x2d), 400),
    spec('Sim_10092956', 0x10092956, g_92956, 500, void=True),
    spec('Sim_100939a3', 0x100939a3, g_spawn_939a3, 800, void=True),
    spec('Sim_100941c3', 0x100941c3, g_spawn, 700, void=True),
    spec('Sim_100949e8', 0x100949e8, g_spawn, 700, void=True),
    spec('Sim_10094fcb', 0x10094fcb, g_spawn, 700, void=True),
    spec('Sim_10095616', 0x10095616, g_spawn, 700, void=True),
    spec('Sim_10095e0a', 0x10095e0a, g_spawn, 700, void=True),
    spec('Sim_10096541', 0x10096541, g_spawn, 700, void=True),
    spec('Sim_10098721', 0x10098721, g_98721, 300),
    spec('Sim_100999fc', 0x100999fc, g_999fc, 600),
    spec('Sim_1009b8d8', 0x1009b8d8, g_9b8d8, 500),
]
