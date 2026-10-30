# Round 28: the window procedure 0x100867a2 on the platform layer (src/sim/platform/w28_window.c). The original's
# Windows calls and the C's plat.h calls both log the same events with the same answers:
#     ShowWindow(w, 0/4/6/9)           Plat_WindowShow(w, HIDE/NOACTIVATE/MINIMIZE/RESTORE)   ('show', w, 0/4/6/9)
#     ValidateRect(w, 0)               Plat_WindowPainted(w)                                  ('painted', w)
#     DefWindowProcA(w, m, wp, lp)     Plat_WindowDefault(w, m, wp, lp)                       ('default', w, m, wp, lp)
#     GetMessageTime()                 Plat_EventTime()                                       ('time',)
#     PostQuitMessage(0)               Plat_RequestQuit()                                     ('quit',)
#     ShowCursor loop to >= 0 / < 0    Plat_CursorVisible(1 / 0)                              ('cursor', 1 / 0)
# (the cursor model keeps the Windows display count at 0 while [0x1017b93c] says shown, -1 while hidden, so the loops
# end after one call; a call that crosses 0 logs the event). The game's helpers are stubs on both sides: the key
# state clear 0x10057f53, the mode set 0x10037f71, Sim_100766ca, the display driver's refresh (+0x18) and palette
# (+0x14) and the sound pause / resume pointers. Key messages (0x100..0x108) are left out: the original hands them
# to 0x10057b4e, the C leaves them to the SDL3 keys (W28_PumpKeys, spec_w28_input.py), the deliberate difference.
# W28_NEG=1 builds W28W_MUTATE.
exec(open('tools/verify/w10_common.py').read())

if os.environ.get('W28_NEG'):
    CFLAGS.append('-DW28W_MUTATE')

PLAT_STUB = os.path.abspath(os.path.join('tools', 'verify', 'plat_stub.c'))
SOURCES = closure(['sim/platform/w28_window.c']) + [PLAT_STUB]
FN = {'refresh': 0x00F00E00, 'palette': 0x00F00E10, 'resume': 0x00F00E20, 'pause': 0x00F00E30}
helper(0x10057f53, 'Sim_10057f53', 0)
helper(0x10037f71, None, 0)
helper(0x100766ca, None, 1)
helper(FN['refresh'], None, 0)
helper(FN['palette'], None, 4)
helper(FN['resume'], None, 0)
helper(FN['pause'], None, 0)
ACTIVE = tuple(HELPERS)
W = {}
SHOW = {0: 0, 1: 4, 2: 6, 3: 9}


def _raw(uc, k):
    return struct.unpack('<I', bytes(uc.mem_read(uc.reg_read(UC_X86_REG_ESP) + 4 + 4 * k, 4)))[0]


def _fresh():
    m = W['m']
    if W.get('log') is not m.api_log:
        W['log'] = m.api_log
        W['count'] = W['count0']


def _ev(m, *e):
    m.api_log.append(e)


# ---- the original's Windows calls ---------------------------------------------------------------------------------
def a_show(n, args, uc):
    m = W['m']
    m.api_log.pop()
    _ev(m, 'show', _raw(uc, 0), _raw(uc, 1))
    return 1


def a_validate(n, args, uc):
    m = W['m']
    m.api_log.pop()
    _ev(m, 'painted', _raw(uc, 0))
    return 1


def a_default(n, args, uc):
    m = W['m']
    m.api_log.pop()
    _ev(m, 'default', *[_raw(uc, k) for k in range(4)])
    return W['def']


def a_time(n, args, uc):
    m = W['m']
    m.api_log.pop()
    _ev(m, 'time')
    return W['time']


def a_quit(n, args, uc):
    m = W['m']
    m.api_log.pop()
    _ev(m, 'quit')
    return 0


def a_cursor(n, args, uc):
    m = W['m']
    m.api_log.pop()
    _fresh()
    before = W['count']
    W['count'] += 1 if _raw(uc, 0) else -1
    if (before < 0) != (W['count'] < 0):
        _ev(m, 'cursor', 1 if W['count'] >= 0 else 0)
    return W['count'] & 0xffffffff


API = {'ShowWindow': a_show, 'ValidateRect': a_validate, 'DefWindowProcA': a_default, 'GetMessageTime': a_time,
       'PostQuitMessage': a_quit, 'ShowCursor': a_cursor}


# ---- the C side: plat.h stubs -------------------------------------------------------------------------------------
def s_show(uc, a):
    _ev(W['m'], 'show', a[0], SHOW[a[1]])
    return 0


def s_painted(uc, a):
    _ev(W['m'], 'painted', a[0])
    return 0


def s_default(uc, a):
    _ev(W['m'], 'default', *a)
    return W['def']


def s_time(uc, a):
    _ev(W['m'], 'time')
    return W['time']


def s_quit(uc, a):
    _ev(W['m'], 'quit')
    return 0


def s_cursor(uc, a):
    _ev(W['m'], 'cursor', 1 if a[0] else 0)
    return 0


SYMSTUB = {'Plat_WindowShow': (2, s_show), 'Plat_WindowPainted': (1, s_painted), 'Plat_WindowDefault': (4, s_default),
           'Plat_EventTime': (0, s_time), 'Plat_RequestQuit': (0, s_quit), 'Plat_CursorVisible': (1, s_cursor)}


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
    if getattr(m, '_w28w', False):
        return
    m._w28w = True
    for sym, (n, fn) in SYMSTUB.items():
        m.raw.hook_add(UC_HOOK_CODE, _sym_cb(m, fn, n), None, m.syms[sym], m.syms[sym])


MESSAGES = [0x0f, 0x02, 0x1c, 0x1c, 0x1c, 0x10, 0xa0, 0x46, 0x46, 0x200, 0x30f, 0x01, 0x05, 0x84, 0x20, 0x7e, 0x14, 0x300]


def g_proc(ctx):
    r = begin(ctx, *ACTIVE)
    install(ctx)
    win = r.choice([0x1234, 0x10004, r.getrandbits(32)])
    w32(ctx, 0x1017b8f0, r.choice([win, r.getrandbits(32)]))
    msg = r.choice(MESSAGES)
    wp = r.choice([0, 1, 1, r.getrandbits(32)])
    for a in (0x1017b924, 0x1017b8b8, 0x1017b940, 0x1017b934, 0x1017b92c, 0x1017b938):
        w32(ctx, a, r.choice([0, 0, 1]))
    w32(ctx, 0x1015e9bc, r.choice([2, 2, 1, 3]))
    w32(ctx, 0x1015e9c4, r.choice([640, 320, 641, 800, -1 & 0xffffffff]))
    w32(ctx, 0x1015e9c8, r.choice([480, 200, 481, 600]))
    w32(ctx, 0x1015e9c0, r.choice([0, 1, 1, 2]))
    shown = r.choice([0, 1])
    w32(ctx, 0x1017b93c, shown)
    W['count0'] = 0 if shown else -1
    W['log'] = None
    drv = newbuf(ctx, 0x20)
    w32(ctx, drv, r.choice([0, 1, 2]))
    w32(ctx, drv + 0x14, FN['palette'])
    w32(ctx, 0x1015e688, r.choice([drv, drv, 0]))
    drv2 = newbuf(ctx, 0x20)
    w32(ctx, drv2 + 0x18, FN['refresh'])
    w32(ctx, 0x1015e68c, drv2)
    w32(ctx, 0x109e3d5c, FN['resume'])
    w32(ctx, 0x109e3d60, FN['pause'])
    for a in (0x109e3780, 0x109e3784, 0x109e3788, 0x109e378c, 0x1017b8bc, 0x1017b928, 0x1017b920):
        w32(ctx, a, r.getrandbits(32))
    t = r.getrandbits(32)
    W['time'] = t
    w32(ctx, 0x1015dbc8, r.choice([t, t - 1, t + 1, r.getrandbits(32)]) & 0xffffffff)
    w32(ctx, 0x1015da2c, r.choice([t, t - 5, t + 5, r.getrandbits(32)]) & 0xffffffff)
    W['def'] = r.getrandbits(32)
    rets(0x100766ca, r.choice([1, 1, 0, 2]))
    lp = r.getrandbits(32)
    if msg == 0x46:
        lp = newbuf(ctx, 0x20)
        wb(ctx, lp, rb(ctx, 0x20))
    return [win, msg, wp, lp]


SPECS = [dict(name='Sim_100867a2', orig=0x100867a2, c='Sim_100867a2@16', gen=g_proc, api=API, trials=20000)]
