# Round 28: the keyboard and mouse functions on the platform layer (src/sim/platform/w28_input.c). The original runs
# against a model of Windows (a queue of key messages for PeekMessageA, a cursor for GetCursorPos / SetCursorPos /
# ScreenToClient / ClientToScreen / ClipCursor, button state for GetAsyncKeyState); the C runs against stubs of
# platform/input.h fed from the same model (Input_NextKey takes the same key events, Input_MouseTake gives the
# cursor movement, Input_MouseButtons the buttons). The Windows calls themselves are the deliberate difference and
# are taken out of the original's call log, except the clip box: ClipCursor(box) / ClipCursor(0) and
# Input_MouseCapture(1) / (0) both log ('capture', on). The sim's key handling Key_SetState (0x10057d2d; lParam
# compared in bits 16..24, the only ones it reads) and Input_QueueKey (0x10057c1a) are stubs on both sides, so the
# key events reach the game in the same order with the same values. Model assumptions: the user's movement since
# the last poll reaches the cursor before the poll reads it (and is lost when the box is set up again, as the C
# capture drops it), and the clip box holds the cursor in 0..width-1 / 0..height-1. W28_NEG=1 builds W28I_MUTATE.
exec(open('tools/verify/w10_common.py').read())

if os.environ.get('W28_NEG'):
    CFLAGS.append('-DW28I_MUTATE')

INPUT_STUB = os.path.abspath(os.path.join('tools', 'verify', 'input_stub.c'))
SOURCES = closure(['sim/platform/w28_input.c']) + [INPUT_STUB]
helper(0x10057d2d, 'Key_SetState', 3)
helper(0x10057c1a, 'Input_QueueKey', 1)
MASK[(0x10057d2d, 1)] = 0x01ff0000
ACTIVE = tuple(HELPERS)
W = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


import copy

KEYS = ('msgs', 'cur', 'move', 'clip', 'buttons', 'org', 'w', 'h')


def fresh():
    """The model as the generator left it, again for each side's run (a new call log marks a new run)."""
    m = W['m']
    if W.get('log') is not m.api_log:
        W['log'] = m.api_log
        for k in KEYS:
            if k in W.get('init', {}):
                W[k] = copy.deepcopy(W['init'][k])


def seal():
    W['init'] = {k: copy.deepcopy(W[k]) for k in KEYS if k in W}
    W['log'] = None


def _raw(uc, k):
    return struct.unpack('<I', bytes(uc.mem_read(uc.reg_read(UC_X86_REG_ESP) + 4 + 4 * k, 4)))[0]


def _drop(m):
    m.api_log.pop()


# ---- the original's Windows model --------------------------------------------------------------------------------
def a_peek(n, args, uc):
    fresh()
    m = W['m']
    _drop(m)
    q = W['msgs']
    if not q:
        return 0
    kind, vk, sc, ext = q.pop(0)
    msg = _raw(uc, 0)
    lparam = sc << 16 | ext << 24 | 1 | (0xc0000000 if kind in (0x101, 0x105) else 0)
    m.uc.mem_write(msg, struct.pack('<7I', 0x1234, kind, vk, lparam, 0, 0, 0))
    return 1


def a_quiet(n, args, uc):
    fresh()
    _drop(W['m'])
    return 1


def a_getpos(n, args, uc):
    fresh()
    _drop(W['m'])
    if W['move'] is not None:            # the user's movement since the last poll reaches the cursor now
        cx, cy = W['cur']
        dx, dy = W['move']
        W['move'] = None
        cx, cy = cx + dx, cy + dy
        if W['clip']:
            ox, oy = W['org']
            cx = min(max(cx, ox), ox + W['w'] - 1)
            cy = min(max(cy, oy), oy + W['h'] - 1)
        W['cur'] = (cx, cy)
    W['m'].uc.mem_write(_raw(uc, 0), struct.pack('<2i', *W['cur']))
    return 1


def a_setpos(n, args, uc):
    fresh()
    _drop(W['m'])
    W['cur'] = (struct.unpack('<i', struct.pack('<I', _raw(uc, 0)))[0],
                struct.unpack('<i', struct.pack('<I', _raw(uc, 1)))[0])
    return 1


def _shift(uc, sign):
    fresh()
    _drop(W['m'])
    p = _raw(uc, 1)
    x, y = struct.unpack('<2i', bytes(uc.mem_read(p, 8)))
    ox, oy = W['org']
    W['m'].uc.mem_write(p, struct.pack('<2i', x + sign * ox, y + sign * oy))
    return 1


def a_s2c(n, args, uc):
    fresh()
    return _shift(uc, -1)


def a_c2s(n, args, uc):
    fresh()
    return _shift(uc, 1)


def a_clip(n, args, uc):
    fresh()
    m = W['m']
    m.api_log.pop()
    on = 1 if _raw(uc, 0) != 0 else 0
    W['clip'] = on
    if on:
        W['move'] = None                 # a new box: the movement before it is not seen (as the C capture drops it)
    m.api_log.append(('capture', on))
    return 1


def a_key(n, args, uc):
    fresh()
    _drop(W['m'])
    bit = {1: 1, 4: 2, 2: 4}.get(_raw(uc, 0), 0)
    return 0x8000 if W['buttons'] & bit else 0


API = {'PeekMessageA': a_peek, 'TranslateMessage': a_quiet, 'GetCursorPos': a_getpos, 'SetCursorPos': a_setpos,
       'ScreenToClient': a_s2c, 'ClientToScreen': a_c2s, 'ClipCursor': a_clip, 'GetAsyncKeyState': a_key}


# ---- the C side: stubs of platform/input.h -------------------------------------------------------------------------
def s_pump(uc, a):
    fresh()
    if a[0]:
        uc.mem_write(a[0], struct.pack('<i', 0))
    return 0


def s_nextkey(uc, a):
    fresh()
    q = W['msgs']
    while q and q[0][0] not in (0x100, 0x101, 0x104, 0x105):
        q.pop(0)                         # characters and the like: no key event
    if not q:
        return 0
    kind, vk, sc, ext = q.pop(0)
    uc.mem_write(a[0], struct.pack('<5i', 1 if kind in (0x100, 0x104) else 0, 0, sc, ext, vk))
    return 1


def s_capture(uc, a):
    fresh()
    W['m'].api_log.append(('capture', 1 if a[0] else 0))
    if a[0]:
        W['move'] = None
    return 0


def s_take(uc, a):
    fresh()
    dx, dy = W['move'] if W['move'] is not None else (0, 0)
    W['move'] = None
    uc.mem_write(a[0], struct.pack('<I', fb(float(dx))))
    uc.mem_write(a[1], struct.pack('<I', fb(float(dy))))
    return 0


def s_buttons(uc, a):
    fresh()
    return W['buttons']


SYMSTUB = {'Input_Pump': (1, s_pump), 'Input_NextKey': (1, s_nextkey), 'Input_MouseCapture': (1, s_capture),
           'Input_MouseTake': (2, s_take), 'Input_MouseButtons': (0, s_buttons)}


def _sym_cb(m, fn, n):
    def cb(uc, address, size, ud):
        uc = m.uc
        esp = uc.reg_read(UC_X86_REG_ESP)
        a = [struct.unpack('<I', bytes(uc.mem_read(esp + 4 + 4 * k, 4)))[0] for k in range(n)]
        uc.reg_write(UC_X86_REG_EAX, fn(uc, a) & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, struct.unpack('<I', bytes(uc.mem_read(esp, 4)))[0])
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def install(ctx):
    m = ctx.m
    W['m'] = m
    if getattr(m, '_w28', False):
        return
    m._w28 = True
    for sym, (n, fn) in SYMSTUB.items():
        m.raw.hook_add(UC_HOOK_CODE, _sym_cb(m, fn, n), None, m.syms[sym], m.syms[sym])


# ---- generators ------------------------------------------------------------------------------------------------------
def keys(r):
    out = []
    for _ in range(r.choice([0, 0, 1, 2, 5])):
        kind = r.choice([0x100, 0x101, 0x100, 0x101, 0x104, 0x105, 0x102])
        out.append((kind, r.randrange(1, 0xff), r.randrange(0, 0x60), r.choice([0, 0, 1])))
    return out


def world(ctx, r):
    install(ctx)
    W['msgs'] = keys(r)
    ctx_queue(ctx, r)
    seal()


def ctx_queue(ctx, r):
    rd, wr = r.randrange(0, 0x40), r.randrange(0, 0x40)
    if r.random() < 0.4:
        wr = rd
    w32(ctx, 0x10166868, rd)
    w32(ctx, 0x1016686c, wr)
    wb(ctx, 0x10165ee0, rb(ctx, 0x80))


def g_flush(ctx):
    r = begin(ctx, *ACTIVE)
    world(ctx, r)
    return []


def g_read(ctx):
    r = begin(ctx, *ACTIVE)
    world(ctx, r)
    return []


def g_poll(ctx):
    r = begin(ctx, *ACTIVE)
    install(ctx)
    W['msgs'] = []
    w, h = r.choice([(640, 480), (640, 480), (320, 200), (800, 600)])
    w32(ctx, 0x1017b918, w)
    w32(ctx, 0x1017b91c, h)
    w32(ctx, 0x1017b920, r.choice([1, 1, 1, 0]))
    clipped = r.choice([1, 1, 0])
    w32(ctx, 0x10179d64, clipped)
    w32(ctx, 0x10179d68, r.choice([0, 0, 0, 1]))
    W['w'], W['h'] = w, h
    W['org'] = (r.randrange(-50, 2000), r.randrange(-50, 1200))
    W['clip'] = clipped
    px, py = r.randrange(0, w), r.randrange(0, h)
    W['cur'] = (W['org'][0] + px, W['org'][1] + py)
    w32(ctx, ctx.m.syms['g_w28MouseX'], fb(float(px)))
    w32(ctx, ctx.m.syms['g_w28MouseY'], fb(float(py)))
    W['move'] = r.choice([(0, 0), (r.randrange(-30, 31), r.randrange(-30, 31)),
                          (r.randrange(-900, 900), r.randrange(-900, 900))])
    W['buttons'] = r.randrange(0, 8)
    pos = newbuf(ctx, 8)
    wb(ctx, pos, rb(ctx, 8))
    btn = newbuf(ctx, 4)
    w32(ctx, btn, r.getrandbits(32))
    seal()
    return [r.getrandbits(32), r.choice([pos, pos, 0]), r.choice([btn, btn, 0]) if ur32(ctx.m.uc, 0x1017b920) else btn]


def g_unclip(ctx):
    r = begin(ctx, *ACTIVE)
    install(ctx)
    w32(ctx, 0x10179d64, r.choice([0, 1]))
    W['clip'] = 1
    seal()
    return []


IGNORE = {'g_w28MouseX': 4, 'g_w28MouseY': 4}
SPECS = [dict(name='Sim_10057afc', orig=0x10057afc, c='Sim_10057afc', gen=g_flush, api=API, trials=3000),
         dict(name='Sim_10057f83', orig=0x10057f83, c='Sim_10057f83', gen=g_read, api=API, trials=3000),
         dict(name='Sim_1007d948', orig=0x1007d948, c='Sim_1007d948', gen=g_unclip, api=API, trials=500),
         dict(name='Sim_1007d9d8', orig=0x1007d9d8, c='Sim_1007d9d8', gen=g_poll, api=API, trials=6000,
              ignore_syms=IGNORE, ignore_addrs={0x10373040: 16})]   # the clip box, only handed to ClipCursor
