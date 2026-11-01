# Round 28: the renderer plugin's load 0x1003650d and choice 0x1003695b (src/sim/platform/w28_plugin.c). The C's
# Plat_LoadLibrary / Plat_LibraryFunction / Plat_FreeLibrary log the same entries the original's LoadLibraryA /
# GetProcAddress / FreeLibrary calls log, with the same answers (a library handle or 0; GetFuncDesc and CheckFunc as
# stub functions of the test, the 16 entry points as numbers or 0). GetFuncDesc writes the kind ("RASTER" or not)
# and the type; CheckFunc answers 0 or 1. Str_CompareNoCase and Sim_10036820 run as they are ([0x1017e950] = 0, the C
# locale). The trace hook g_w28Trace stays unset. W28_NEG=1 builds W28P_MUTATE.
exec(open('tools/verify/w10_common.py').read())

if os.environ.get('W28_NEG'):
    CFLAGS.append('-DW28P_MUTATE')

PLAT_STUB = os.path.abspath(os.path.join('tools', 'verify', 'plat_stub.c'))
SOURCES = closure(['sim/platform/w28_plugin.c'])
if not any(x.replace(os.sep, '/').endswith('verify/plat_stub.c') for x in SOURCES):
    SOURCES = SOURCES + [PLAT_STUB]
DESC, CHECK = 0x00F00E00, 0x00F00E10
helper(DESC, None, 3)
helper(CHECK, None, 1)
ACTIVE = tuple(HELPERS)
W = {}


def _raw(uc, k):
    return struct.unpack('<I', bytes(uc.mem_read(uc.reg_read(UC_X86_REG_ESP) + 4 + 4 * k, 4)))[0]


def e_desc(uc, k, raw, eax):
    uc.mem_write(raw[0], W['kind'] + b'\0')
    uc.mem_write(raw[1], struct.pack('<i', W['id']))
    return W['descret']


EFFECT[DESC] = e_desc


def _name(uc, p):
    return bytes(uc.mem_read(p, 32)).split(b'\0')[0]


def proc(uc, h, p):
    n = _name(uc, p)
    if n == b'GetFuncDesc':
        return W['desc']
    if n == b'CheckFunc':
        return CHECK
    return W['procs'].get(n, 0)


def a_load(n, args, uc):
    return W['lib']


def a_proc(n, args, uc):
    return proc(W['m'].uc, _raw(uc, 0), _raw(uc, 1))


def a_free(n, args, uc):
    return 1


API = {'LoadLibraryA': a_load, 'GetProcAddress': a_proc, 'FreeLibrary': a_free}


def s_load(uc, a):
    W['m'].api_log.append(('LoadLibraryA', (a[0],)))
    return W['lib']


def s_proc(uc, a):
    W['m'].api_log.append(('GetProcAddress', (a[0], a[1])))
    return proc(uc, a[0], a[1])


def s_free(uc, a):
    W['m'].api_log.append(('FreeLibrary', (a[0],)))
    return 0


SYMSTUB = {'Plat_LoadLibrary': (1, s_load), 'Plat_LibraryFunction': (2, s_proc), 'Plat_FreeLibrary': (1, s_free)}


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
    if getattr(m, '_w28p', False):
        return
    m._w28p = True
    for sym, (n, fn) in SYMSTUB.items():
        m.raw.hook_add(UC_HOOK_CODE, _sym_cb(m, fn, n), None, m.syms[sym], m.syms[sym])


NAMES = [b'SetLumaTable', b'FirstDevice', b'LastDevice', b'Render', b'RenderRefresh', b'GetSocketCaps', b'SetState',
         b'SetTexturePalette', b'LockDisplay', b'UnlockDisplay', b'RefreshDisplay', b'PreloadTexture',
         b'RenderNoClip', b'RestoreDevice', b'LostDeviceDisplay', b'UpdateTexture']


def cs(ctx, b):
    p = newbuf(ctx, len(b) + 1)
    wb(ctx, p, b + b'\0')
    return p


def g_load(ctx):
    r = begin(ctx, *ACTIVE)
    install(ctx)
    w32(ctx, 0x1017e950, 0)
    W['lib'] = r.choice([0x70000000, 0x70000000, 0x70000000, 0])
    W['desc'] = r.choice([DESC, DESC, DESC, 0])
    W['kind'] = r.choice([b'RASTER', b'raster', b'SOUND', b'RASTE'])
    W['id'] = r.choice([0, 1, 2, 3, -1])
    W['descret'] = r.getrandbits(32)
    W['procs'] = {n: r.choice([0x71000000 + 0x10 * k, 0]) for k, n in enumerate(NAMES)}
    rets(CHECK, r.choice([0, 1, 1, 2]))
    path = cs(ctx, r.choice([b'plugin\\HGVK.DLL', b'DX5DRAW.DLL', b'plugin\\hgvk.dll']))
    w32(ctx, 0x1015e578, r.choice([0, 1]))
    wb(ctx, 0x1015e57c, r.choice([b'\0', b'plugin\\HGVK.DLL\0', b'DX5DRAW.DLL\0']) + b'\0' * 8)
    w32(ctx, 0x1015e5bc, r.choice([0, 1, 2, 3]))
    wb(ctx, 0x1015e59c, r.choice([b'\0', b'card0\0']))
    w32(ctx, 0x109e3d20, r.choice([0, 1, 2]))
    w32(ctx, 0x1015e570, r.choice([0, 1]))
    w32(ctx, 0x1015e568, r.getrandbits(32))
    return [path]


def g_choice(ctx):
    r = begin(ctx, *ACTIVE)
    install(ctx)
    w32(ctx, 0x1017e950, 0)
    kind = cs(ctx, r.choice([b'RASTER', b'raster', b'SOUND', b'DRIVER', b'OTHER']))
    name = r.choice([0, cs(ctx, b'plugin\\HGVK.DLL'), cs(ctx, b'card0')])
    return [kind, name, r.getrandbits(32)]


SPECS = [dict(name='Sim_1003650d', orig=0x1003650d, c='Sim_1003650d', gen=g_load, api=API, trials=8000, void=True),
         dict(name='Sim_1003695b', orig=0x1003695b, c='Sim_1003695b', gen=g_choice, trials=3000, void=True)]
