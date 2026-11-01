# Round 32 (C): 1.2's PartSpec.ini reader (0x10078e34) and particle type setup (0x100790be) against 1.2's own code
# (difftest 'orig12'). The settings file is a per-trial table both sides read through their own path: 1.2 through
# its imports GetCurrentDirectoryA / GetPrivateProfileIntA / GetPrivateProfileStringA, the C through Plat_CurrentDir /
# Plat_IniInt / Plat_IniString; both log one form (the path, section, key and default as text). Texture registration
# (1.2 0x1000189d / C Sim_10006ba7) and the record binding (1.2 0x10001ab2 / C Sim_10006db8) are logging stubs.
# The type table (1.2 0x1011e700, C g_w32_1011e700) and the 14-byte records (1.2 0x108b6d60, 1.0 0x10ac1b00) are
# mapped across; the table's section pointers stay each side's own (1.2's strings / the C's).
# W32_NEG=1 builds the C with W32C_MUTATE (the reader returns one texture short).
import os, struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_EIP

SOURCES = ['sim/fx/w32_partspec.c', 'sim/render/w31_tex.c', '../tools/verify/plat_stub.c',
           'sim/render/w3q_texture.c', 'sim/unsorted_1000/w2f_records.c', 'sim/unsorted_100f/w2e_string.c',
           'sim/unsorted_1000/w2h_reset.c', 'sim/render/w33_dx12.c']     # (round 33: g_w33Voodoo)
CFLAGS = []
if os.environ.get('W32_NEG'):
    CFLAGS.append('-DW32C_MUTATE')
if os.environ.get('W32PV_NEG'):            # round 33: the 3DFX mode not tested in the reset
    CFLAGS.append('-DW32PV_MUTATE')

BASE12 = 0x20000000


def A12(a):
    return BASE12 + a - 0x10000000


TAB12, REC10, REC12 = A12(0x1011e700), 0x10ac1b00, A12(0x108b6d60)
NAMES = ['railgun', 'haywire', 'accel', 'laser', 'shockwave', 'skate', 'gun', 'smoke', 'grenade', 'mortar',
         'missile', 'chunket', 'tracer', 'spark', 'missilespark', 'laserspark', 'pweaponspark', 'projpuff',
         'laserpuff', 'pweaponpuff', 'missilepuff', 'footstep']
CWD = b'C:\\HG\\GAME'
ST = {}


def r32(uc, a):
    return struct.unpack('<I', bytes(uc.mem_read(a, 4)))[0]


def w32(uc, a, v):
    uc.mem_write(a, struct.pack('<I', v & 0xffffffff))


def cstr(uc, a, n=64):
    if a == 0:
        return None
    return bytes(uc.mem_read(a, n)).split(b'\0')[0]


def ret(uc, value, pop=0):
    esp = uc.reg_read(UC_X86_REG_ESP)
    uc.reg_write(UC_X86_REG_EAX, value & 0xffffffff)
    uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
    uc.reg_write(UC_X86_REG_ESP, esp + 4 + pop)


def ini_value(section, key):
    return ST['ini'].get((section.lower(), key.lower()))


def ini_int(section, key, default):
    v = ini_value(section, key)
    if v is None:
        return default & 0xffffffff
    s = v.strip()
    neg = s.startswith(b'-')
    if neg or s.startswith(b'+'):
        s = s[1:]
    n = 0
    for c in s:
        if not 48 <= c <= 57:
            break
        n = n * 10 + c - 48
    return (-n if neg else n) & 0xffffffff


def ini_str(uc, section, key, default, out, size):
    v = ini_value(section, key)
    if v is None:
        v = default or b''
    v = v[:max(size - 1, 0)]
    uc.mem_write(out, v + b'\0')
    return len(v)


# ---- the 1.2 side: its imports (handlers read the raw arguments from the stack) ----
def api_cwd(n, args, uc):
    esp = uc.reg_read(UC_X86_REG_ESP)
    size, buf = r32(uc, esp + 4), r32(uc, esp + 8)
    uc.mem_write(buf, CWD + b'\0')
    ST['m'].api_log[-1] = ('cwd', ())
    return len(CWD)


def api_ini_int(n, args, uc):
    esp = uc.reg_read(UC_X86_REG_ESP)
    sec, key, dflt, path = (r32(uc, esp + 4 + 4 * k) for k in range(4))
    s, k = cstr(uc, sec), cstr(uc, key)
    ST['m'].api_log[-1] = ('ini_int', (cstr(uc, path, 128), s, k, dflt))
    return ini_int(s, k, dflt)


def api_ini_str(n, args, uc):
    esp = uc.reg_read(UC_X86_REG_ESP)
    sec, key, dflt, out, size, path = (r32(uc, esp + 4 + 4 * k) for k in range(6))
    s, k = cstr(uc, sec), cstr(uc, key)
    ST['m'].api_log[-1] = ('ini_str', (cstr(uc, path, 128), s, k, cstr(uc, dflt), size))
    return ini_str(uc, s, k, cstr(uc, dflt), out, size)


API = {'GetCurrentDirectoryA': api_cwd, 'GetPrivateProfileIntA': api_ini_int,
       'GetPrivateProfileStringA': api_ini_str}


def install(m):
    if getattr(m, '_w32c', False):
        return
    m._w32c = True
    uc = m.uc

    def c_cwd(uc_, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        out, size_ = r32(uc, esp + 4), r32(uc, esp + 8)
        uc.mem_write(out, CWD + b'\0')
        m.api_log.append(('cwd', ()))
        ret(uc, len(CWD))

    def c_ini_int(uc_, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        path, sec, key, dflt = (r32(uc, esp + 4 + 4 * k) for k in range(4))
        s, k = cstr(uc, sec), cstr(uc, key)
        m.api_log.append(('ini_int', (cstr(uc, path, 128), s, k, dflt)))
        ret(uc, ini_int(s, k, dflt))

    def c_ini_str(uc_, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        path, sec, key, dflt, out, size_ = (r32(uc, esp + 4 + 4 * k) for k in range(6))
        s, k = cstr(uc, sec), cstr(uc, key)
        m.api_log.append(('ini_str', (cstr(uc, path, 128), s, k, cstr(uc, dflt), size_)))
        ret(uc, ini_str(uc, s, k, cstr(uc, dflt), out, size_))

    def reg(uc_, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        name, group, value = (r32(uc, esp + 4 + 4 * k) for k in range(3))
        n = sum(1 for e in m.api_log if e[0] == 'register')
        m.api_log.append(('register', (cstr(uc, name), group, value)))
        seq = ST['reg']
        ret(uc, seq[n % len(seq)])

    def bind(uc_, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        a, b = r32(uc, esp + 4), r32(uc, esp + 8)
        m.api_log.append(('bind', (a, b)))
        ret(uc, 1)

    for sym, cb in (('Plat_CurrentDir', c_cwd), ('Plat_IniInt', c_ini_int), ('Plat_IniString', c_ini_str),
                    ('Sim_10006ba7', reg), ('Sim_10006db8', bind)):
        m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[sym], m.syms[sym])
    for a, cb in ((A12(0x1000189d), reg), (A12(0x10001ab2), bind)):
        m.raw.hook_add(UC_HOOK_CODE, cb, None, a, a)


def random_ini(r, sections):
    ini = {}
    for s in sections:
        if r.random() < 0.15:
            continue
        for key in (b'Red', b'Green', b'Blue', b'Intensity', b'Size', b'Lock', b'Order', b'Loop'):
            if r.random() < 0.5:
                ini[(s.lower(), key.lower())] = r.choice([b'%d' % r.randrange(0, 300), b'-1', b' 12', b'x',
                                                           b'%d' % r.getrandbits(31), b'7z'])
        n = r.choice([0, 1, 3, 10, 32, 34])
        for i in range(n):
            if r.random() < 0.9:
                ini[(s.lower(), b'texture%02d' % i)] = r.choice([b'fxboom00', b'fxsmok0%d' % (i % 5), b'',
                                                                b'averylongname', b'ab'])
    return ini


def begin(ctx):
    m, r = ctx.m, ctx.rng
    ST['m'] = m
    install(m)
    ST['reg'] = [r.choice([0, 0, 0, 1, 0xffffffff]) for _ in range(r.choice([1, 3, 8]))]


# ---- the reader ----
def g_load(ctx):
    begin(ctx)
    r, uc = ctx.rng, ctx.m.uc
    name = r.choice(NAMES).encode()
    sec = ctx.buf(16, name + b'\0')
    t = bytearray(rb := bytes(r.getrandbits(8) for _ in range(0x1e)))
    struct.pack_into('<I', t, 0xe, sec)
    tp = ctx.buf(0x1e, bytes(t))
    names = ctx.buf(0x20 * 9 + 9, bytes(r.choice([0, 0, 0, 0x41]) for _ in range(0x20 * 9 + 9)))
    ST['ini'] = random_ini(r, [name])
    return [tp, names, r.choice([0, 1, 5, 0x20, 0x21, 40, 0xfffffff0])]


# ---- the setup ----
def g_init(ctx):
    begin(ctx)
    r, uc, m = ctx.rng, ctx.m.uc, ctx.m
    w32(uc, m.syms['g_w31Tex'], 1)
    w32(uc, 0x1017b940, r.choice([0, 1, 1, 1]))
    w32(uc, m.syms['g_w33Voodoo'], r.choice([0, 0, 1]))
    tab = bytearray(uc.mem_read(m.syms['g_w32_1011e700'], 22 * 0x1e))
    for i in range(22):
        o = i * 0x1e
        struct.pack_into('<i', tab, o, r.choice([0, 0, 0, -1, 1, 0x150]))
        tab[o + 8:o + 0xe] = bytes(r.getrandbits(8) for _ in range(6))
        struct.pack_into('<iiI', tab, o + 0x12, r.getrandbits(8), r.getrandbits(8), r.getrandbits(8))
        if r.random() < 0.05:
            struct.pack_into('<I', tab, o + 0xe, 0)             # no section
    uc.mem_write(m.syms['g_w32_1011e700'], bytes(tab))
    recs = bytearray(0x200 * 14)
    for i in range(0x200):
        struct.pack_into('<h', recs, i * 14 + 8, r.choice([-2, -2, 1, 0]) if i >= 0x100 else 1)
    uc.mem_write(REC10, bytes(recs))
    ST['ini'] = random_ini(r, [n.encode() for n in NAMES])
    return []


def sec_ptrs12(uc):
    return [r32(uc, TAB12 + i * 0x1e + 0xe) for i in range(22)]


def to12(m):
    uc = m.uc
    if 'sec12' not in ST:
        ST['sec12'] = sec_ptrs12(uc)          # 1.2's own section pointers (its snapshot)
    tab = bytearray(uc.mem_read(m.syms['g_w32_1011e700'], 22 * 0x1e))
    for i in range(22):
        c = struct.unpack_from('<I', tab, i * 0x1e + 0xe)[0]
        struct.pack_into('<I', tab, i * 0x1e + 0xe, ST['sec12'][i] if c else 0)
    uc.mem_write(TAB12, bytes(tab))
    uc.mem_write(REC12, bytes(uc.mem_read(REC10, 0x200 * 14)))
    w32(uc, A12(0x1014a5e8), r32(uc, 0x1017b940))
    w32(uc, A12(0x1014b954), r32(uc, m.syms['g_w33Voodoo']))    # round 33: the C's 3DFX flag


def from12(m):
    uc = m.uc
    c_tab = bytes(uc.mem_read(m.syms['g_w32_1011e700'], 22 * 0x1e))
    tab = bytearray(uc.mem_read(TAB12, 22 * 0x1e))
    for i in range(22):
        o = i * 0x1e + 0xe
        p12 = struct.unpack_from('<I', tab, o)[0]
        tab[o:o + 4] = c_tab[o:o + 4] if p12 else bytes(4)
    uc.mem_write(m.syms['g_w32_1011e700'], bytes(tab))
    uc.mem_write(REC10, bytes(uc.mem_read(REC12, 0x200 * 14)))


def to12_load(m):
    pass


SPECS = [
    dict(name='Sim12_10078e34', orig12=0x10078e34, c='Sim12_10078e34', gen=g_load, trials=3000,
         to12=to12_load, from12=to12_load, api=API),
    dict(name='Sim12_100790be', orig12=0x100790be, c='Sim12_100790be', gen=g_init, trials=1500, void=True,
         to12=to12, from12=from12, api=API),
]
