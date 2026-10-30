# Round 28: the loading animation, StartSupanim 0x100b82a0 and its tick 0x100b8834 (src/sim/platform/w28_anim.c).
# The callees are stubs on both sides: the file loader Sim_100ba7d5 (answers a block and its size), Sim_100bb2b0,
# Sim_100b9203, the shape library 0x100eb070 / 0x100ed7e6 / 0x100e91dc / 0x100ed8c3, the debug print 0x10083495 and
# the display drivers' entries (+0x14 palette, +0x1c fade, +0x20 buffer ready; screen +0x18, +0x1c). HeapAlloc /
# HeapFree answer the same blocks on both sides (the C reaches them through Heap_AllocRaw / Heap_AllocZero /
# Heap_FreeRaw). GetCurrentDirectoryA and Plat_CurrentDir give the same folder. The deliberate difference: the
# original starts a Windows timer (timeGetDevCaps, timeBeginPeriod, timeSetEvent; answered with a PC's caps 1..1000000
# ms and the timer id 0, and taken out of the call log), the C keeps the tick for its own poll (g_w28Anim /
# g_w28AnimLast, ignored). With the id 0 both leave [0x1017d5cc] 0 and [0x10199448] 10. W28_NEG=1 builds W28A_MUTATE.
exec(open('tools/verify/w10_common.py').read())

if os.environ.get('W28_NEG'):
    CFLAGS.append('-DW28A_MUTATE')

PLAT_STUB = os.path.abspath(os.path.join('tools', 'verify', 'plat_stub.c'))
SOURCES = closure(['sim/platform/w28_anim.c']) + [PLAT_STUB]
FN = {'pal': 0x00F00E00, 'fade': 0x00F00E10, 'ask': 0x00F00E20, 'refresh': 0x00F00E30, 'box': 0x00F00E40}
helper(0x100ba7d5, 'Sim_100ba7d5', 3, sargs=(0, 1))
helper(0x100bb2b0, 'Sim_100bb2b0', 2, sargs=(0, 1))
helper(0x100b9203, 'Sim_100b9203', 2, sargs=(0, 1))
helper(0x100eb070, None, 2)
helper(0x100ed7e6, None, 3)
helper(0x100e91dc, None, 5)
helper(0x100ed8c3, None, 1)
helper(0x10083495, None, 1, sargs=(0,))
helper(FN['pal'], None, 4)
helper(FN['fade'], None, 2)
helper(FN['ask'], None, 0)
helper(FN['refresh'], None, 0)
helper(FN['box'], None, 4)
ACTIVE = tuple(HELPERS)
W = {}


def _raw(uc, k):
    return struct.unpack('<I', bytes(uc.mem_read(uc.reg_read(UC_X86_REG_ESP) + 4 + 4 * k, 4)))[0]


def _fresh():
    m = W['m']
    if W.get('log') is not m.api_log:
        W['log'] = m.api_log
        W['heap'] = list(W['heap0'])
        W['loads'] = list(W['loads0'])


def a_curdir(n, args, uc):
    W['m'].uc.mem_write(_raw(uc, 1), W['dir'])
    return len(W['dir']) - 1


def a_alloc(n, args, uc):
    _fresh()
    return W['heap'].pop(0) if W['heap'] else 0


def a_free(n, args, uc):
    return 1


def a_caps(n, args, uc):
    W['m'].api_log.pop()
    W['m'].uc.mem_write(_raw(uc, 0), struct.pack('<2I', 1, 1000000))
    return 0


def a_quiet(n, args, uc):
    W['m'].api_log.pop()
    return 0


API = {'GetCurrentDirectoryA': a_curdir, 'HeapAlloc': a_alloc, 'HeapFree': a_free, 'timeGetDevCaps': a_caps,
       'timeBeginPeriod': a_quiet, 'timeSetEvent': a_quiet}


def e_load(uc, k, raw, eax):
    _fresh()
    blk, size = W['loads'].pop(0) if W['loads'] else (0, 0)
    if blk:
        uc.mem_write(raw[2], struct.pack('<I', size))
    return blk


EFFECT[0x100ba7d5] = e_load


def s_curdir(uc, a):
    W['m'].api_log.append(('GetCurrentDirectoryA', (a[1], a[0])))   # the same entry the original's call logs
    uc.mem_write(a[0], W['dir'])
    return len(W['dir']) - 1


def s_ticks(uc, a):
    return 0


SYMSTUB = {'Plat_CurrentDir': (2, s_curdir), 'Plat_TicksMs': (0, s_ticks)}


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
    if getattr(m, '_w28a', False):
        return
    m._w28a = True
    for sym, (n, fn) in SYMSTUB.items():
        m.raw.hook_add(UC_HOOK_CODE, _sym_cb(m, fn, n), None, m.syms[sym], m.syms[sym])


def world(ctx, r):
    install(ctx)
    W['log'] = None
    W['dir'] = r.choice([b'C:\\Coding\\HeavyGear\\Project\0', b'D:\\HG\0', b'E:\\' + b'x' * 200 + b'\0'])
    drv = newbuf(ctx, 0x30)
    w32(ctx, drv, r.choice([3, 3, 1, 0]))
    w32(ctx, drv + 0x14, FN['pal'])
    w32(ctx, drv + 0x1c, FN['fade'])
    w32(ctx, drv + 0x20, FN['ask'])
    w32(ctx, 0x1015e688, drv)
    scr = newbuf(ctx, 0x30)
    w32(ctx, scr + 0x18, FN['refresh'])
    w32(ctx, scr + 0x1c, FN['box'])
    w32(ctx, 0x1015e68c, scr)
    rets(FN['ask'], r.choice([0, 0, 0, 1]))
    w32(ctx, 0x1017b920, r.choice([1, 1, 0]))
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    w32(ctx, 0x1015e538, r.choice([1, 1, 0]))
    for a in (0x109e3e20, 0x109e3e24, 0x109e3e28, 0x109e3e2c, 0x109e3e30, 0x1017d5e4, 0x1017d598, 0x1017d59c,
              0x10199440, 0x10199444, 0x1017d5cc, 0x10199448):
        w32(ctx, a, r.getrandbits(32))
    w32(ctx, 0x109e3d14, r.choice([640, 800, 1920]))
    w32(ctx, 0x109e3d18, r.choice([480, 600, 1080]))
    w32(ctx, 0x1017d5d8, r.choice([0, 1, 7, 12]))
    w32(ctx, 0x1017d5d0, r.choice([0, newbuf(ctx, 0x40)]))
    w32(ctx, 0x1017d5d4, r.choice([0, newbuf(ctx, 0x40)]))
    W['heap0'] = [r.choice([newbuf(ctx, 0x400)] * 6 + [0]) for _ in range(4)]
    W['loads0'] = [r.choice([(newbuf(ctx, 0x40), r.randrange(1, 0x40)), (0, 0)]) for _ in range(2)]
    rets(0x100ed8c3, r.randrange(0, 20))


def g_start(ctx):
    r = begin(ctx, *ACTIVE)
    world(ctx, r)
    return [r.choice([0, 1, 2])]


def g_tick(ctx):
    r = begin(ctx, *ACTIVE)
    world(ctx, r)
    return [r.getrandbits(32) for _ in range(5)]


IGNORE = {'g_w28Anim': 4, 'g_w28AnimLast': 4}
SPECS = [dict(name='Sim_100b82a0', orig=0x100b82a0, c='Sim_100b82a0', gen=g_start, api=API, trials=8000,
              ignore_syms=IGNORE),
         dict(name='Sim_100b8834', orig=0x100b8834, c='Sim_100b8834@20', gen=g_tick, api=API, trials=4000, void=True)]
