# Differential tests for wave 3A (renderer-plugin glue, src/sim/render, see difftest.py).
#
# The plugin slots (table 0x109e3d24..) get a stub each in the emulator: a `ret` at a fixed
# address with a code hook that appends a record of its arguments to a log buffer (argument
# memory, compared after every run) and sets the return value. The same stub addresses serve
# the original and the C run. Pointers into the stack are logged by what they point to, since
# the stack frames of the original and of the C code differ. The cache loader (thunk 0x10003413,
# not converted, uses the file system) is replaced the same way at 0x100ba7d5 by a stub that logs
# (name, arg2, arg3) and returns a buffer chosen by the generator.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX

SOURCES = ['sim/render/w2m_texture.c', 'sim/render/w2m_display.c']

STUB_BASE = 0x00F00100
LOADER = 0x100ba7d5
SLOTS = {0x109e3d24: 'luma', 0x109e3d48: 'lock', 0x109e3d4c: 'unlock', 0x109e3d50: 'refresh', 0x109e3d54: 'preload'}
IDS = {'luma': 1, 'lock': 2, 'unlock': 3, 'refresh': 4, 'preload': 5, 'load': 6}
ST = {}      # per-trial state of the stubs
RECW = 16    # dwords per log record
LOGMAX = 60


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))

def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))

def r32(uc, a):
    return struct.unpack('<I', bytes(uc.mem_read(a, 4)))[0]


def _hook(m):
    uc = m.uc
    lo, hi = m.base, m.base + m.dll_size

    def inimg(x):
        return lo <= x < hi

    def cb(uc, address, size, ud):
        try:
            stub = None
            for slot, name in SLOTS.items():
                if address == STUB_BASE + (slot - 0x109e3d24) * 4:
                    stub = name
            if address == LOADER:
                stub = 'load'
            if stub is None:
                return
            esp = uc.reg_read(UC_X86_REG_ESP)
            a = struct.unpack('<5I', bytes(uc.mem_read(esp + 4, 20)))
            log = ST['log']
            n = r32(uc, log)
            if n >= LOGMAX:
                return
            rec = [IDS[stub]] + [0] * (RECW - 1)
            ret = ST['ret'].get(stub, 0)
            if stub == 'luma':
                rec[1] = a[0]
            elif stub == 'lock':
                uc.mem_write(a[0], struct.pack('<I', ST['stride'] & 0xffffffff))
                ret = ST['lockptr']
            elif stub == 'preload':
                rec[1] = a[0] if inimg(a[0]) else 0x11111111     # the handle cell is on the stack or in the data
                rec[2] = a[1]
                tex = r32(uc, a[0])
                rec[3] = tex if (inimg(tex)) else 0x22222222
                try:
                    rec[4:9] = struct.unpack('<5I', bytes(uc.mem_read(tex, 20)))
                except Exception:
                    rec[4] = 0xBADBAD
            elif stub == 'load':
                rec[1:4] = a[:3]
                ret = ST['loads'].get(a[0], 0)
            uc.mem_write(log + 4 + n * RECW * 4, struct.pack('<%dI' % RECW, *rec))
            uc.mem_write(log, struct.pack('<I', n + 1))
            uc.reg_write(UC_X86_REG_EAX, ret & 0xffffffff)
        except Exception as e:          # never let an exception leave the hook silently
            ST['err'] = repr(e)
            raise
    uc.hook_add(UC_HOOK_CODE, cb, None, STUB_BASE, STUB_BASE + 0x100)
    uc.hook_add(UC_HOOK_CODE, cb, None, LOADER, LOADER)
    uc.mem_write(STUB_BASE, b'\xc3' * 0x100)
    uc.mem_write(LOADER, b'\xc3')


def begin(ctx):
    """Installs the stubs (once) and prepares a fresh log; returns nothing."""
    m = ctx.m
    if not getattr(m, '_w2m', False):
        _hook(m)
        m._w2m = True
    ST['log'] = ctx.buf(4 + LOGMAX * RECW * 4, 'zero')
    ST['ret'] = {s: ctx.rng.choice([0, 1, ctx.rng.getrandbits(32)]) for s in IDS}
    ST['loads'] = {}
    ST['stride'] = 0
    ST['lockptr'] = 0
    for slot in SLOTS:
        w32(ctx, slot, STUB_BASE + (slot - 0x109e3d24) * 4)


# ---------------------------------------------------------------------------
def g_luma(ctx):
    r = ctx.rng
    begin(ctx)
    src = ctx.buf(8192)
    if r.random() < 0.5:
        w32(ctx, 0x1015cff0, 0)
        ST['loads'][0x10a3ae80] = src if r.random() > 0.03 else 0
    else:
        w32(ctx, 0x1015cff0, src)
    w32(ctx, 0x1014cf0c, ctx.u32())
    wb(ctx, 0x10a3ae80, bytes(r.getrandbits(8) for _ in range(40)))
    wb(ctx, 0x10ab1ae0, bytes(r.getrandbits(8) for _ in range(8192)))
    return []


WIDTHS = [0x100, 0x80, 0x40, 0x20, 0x10, 8, 4, 2, 1, 0]


def g_tex(ctx):
    r = ctx.rng
    begin(ctx)
    ngroups = r.choice([1, 2, 4, 8])
    recs = bytearray(0x200 * 14)
    used = r.sample(range(0x200), r.choice([2, 8, 20, 40]))
    for i in used:
        flag = r.choice([1, 1, 1, 0, 3, 0xfffd, 0x8001])
        name = r.randrange(0, ngroups)
        recs[i * 14:i * 14 + 14] = struct.pack('<hhhhHhh', name, r.randrange(-9, 9), r.randrange(-9, 9), 0, flag, 0, 0)[:14]
    # fill the junk bytes of the other records too, but keep bit 0 of word 8 clear
    for i in range(0x200):
        if i not in used and r.random() < 0.3:
            struct.pack_into('<hhhh', recs, i * 14, r.randrange(0, 8), 0, 0, 0)
            struct.pack_into('<H', recs, i * 14 + 8, r.choice([0, 2, 0x100, 0xfffe]))
    wb(ctx, 0x10ac1b00, recs)
    for g in range(ngroups):
        names = bytearray(0x300)
        for k in range(32):
            c = r.random()
            if c < 0.07:
                first = b'\0\0'
            elif c < 0.4:
                first = b'WX'
            elif c < 0.5:
                first = b'W' + bytes([r.choice(b'Wxy')])
            else:
                first = bytes([r.choice(b'ABab\x80\xfe0')]) + bytes([r.choice(b'X_aW')])
            body = first + bytes(r.choice(b'abc012') for _ in range(6)) + b'\0'
            ent = bytearray(0x18)
            ent[:len(body)] = body
            if first[0] == 0:
                ent[0:2] = b'\0\0'
            struct.pack_into('<h', ent, 0x12, r.choice([0, 1, -1, r.randrange(-300, 300)]))
            if r.random() < 0.9:
                img = ctx.buf(16)
                w = r.choice(WIDTHS)
                if r.random() < 0.1: w = r.randrange(-4, 0x180)
                wb(ctx, img, struct.pack('<hhI', w, r.randrange(-3, 300), r.getrandbits(32)))
                struct.pack_into('<I', ent, 0x14, img)
            names[k * 0x18:(k + 1) * 0x18] = ent
        wb(ctx, 0x10a51ac0 + g * 0x300, names)
    return []


def g_files(ctx):
    r = ctx.rng
    begin(ctx)
    for fcell, ncell, tcell, hcell, nameaddr in ((0x1017d0fc, 0x1017d0d0, 0x101ccc80, 0x101cfc90, 0x1017d0d0),
                                                 (0x1017d100, 0x1017d0b0, 0x101cfca0, 0x101cfcb8, 0x1017d0b0)):
        pre = r.random() < 0.3
        w32(ctx, fcell, ctx.buf(16) if pre else 0)
        wb(ctx, ncell, bytes([r.choice([0, 0x41, 0x80, 0xff, r.randrange(1, 256)])]) + bytes(r.choice(b'abc._') for _ in range(20)))
        wb(ctx, tcell, bytes(r.getrandbits(8) for _ in range(20)))
        w32(ctx, hcell, r.choice([tcell, tcell, r.getrandbits(32), 0, 0x10185760]))
        if r.random() < 0.95:
            f = ctx.buf(32)
            wb(ctx, f, struct.pack('<hhI', r.choice([r.randrange(-40, 700), -32768, -1]), r.choice([r.randrange(-40, 700), -32768]), r.getrandbits(32)))
            ST['loads'][ncell] = f
        else:
            ST['loads'][ncell] = 0
    w32(ctx, 0x1014ceb8, ctx.u32())
    return []


def g_flag(ctx):
    begin(ctx)
    w32(ctx, 0x1017b984, ctx.rng.choice([0, 1, 2, 0x80000000, ctx.rng.getrandbits(32)]))
    return []


def g_fill(ctx):
    r = ctx.rng
    begin(ctx)
    buf = ctx.buf(0x20000)
    ST['lockptr'] = buf + 0x10000
    ST['stride'] = r.choice([0, 2, 40, 64, -64, 100, 1280, -2, r.randrange(-300, 300)])
    x0 = r.randrange(-5, 30)
    y0 = r.randrange(-5, 30)
    x1 = x0 + r.choice([-3, -1, 0, 0, 1, 5, 10, 24, r.randrange(0, 40)])
    y1 = y0 + r.choice([-3, -1, 0, 0, 1, 5, 10, 24, r.randrange(0, 40)])
    return [x0, y0, x1, y1, r.choice([0, 0xffff, r.getrandbits(32), r.getrandbits(16)])]


def g_lock(ctx):
    r = ctx.rng
    begin(ctx)
    rec = ctx.buf(8)
    w32(ctx, 0x1015e698, rec)
    ST['lockptr'] = r.choice([0, ctx.u32(), r.getrandbits(32)])
    ST['stride'] = r.choice([0, 1, 640, 1280, 0x80000000, r.getrandbits(32)])
    w32(ctx, 0x1017b984, r.choice([0, 1, r.getrandbits(32)]))
    return []


def g_none(ctx):
    begin(ctx)
    return []


SPECS = [
    dict(name='Render_LoadLumaTable', orig=0x1000656f, c='Render_LoadLumaTable', gen=g_luma, void=True, trials=400),
    dict(name='Render_PreloadTextures', orig=0x10006e52, c='Render_PreloadTextures', gen=g_tex, void=True, trials=400),
    dict(name='Render_PreloadFileTextures', orig=0x100a2a66, c='Render_PreloadFileTextures', gen=g_files, void=True, trials=600),
    dict(name='PluginDisp_UnlockRefresh', orig=0x100877c3, c='PluginDisp_UnlockRefresh', gen=g_flag, trials=300),
    dict(name='PluginDisp_Refresh_100877df', orig=0x100877df, c='PluginDisp_Refresh_100877df', gen=g_none, trials=300),
    dict(name='PluginDisp_Refresh_100877ec', orig=0x100877ec, c='PluginDisp_Refresh_100877ec', gen=g_none, trials=300),
    dict(name='PluginDisp_Refresh_100877f9', orig=0x100877f9, c='PluginDisp_Refresh_100877f9', gen=g_none, trials=300),
    dict(name='PluginDisp_Refresh_10087806', orig=0x10087806, c='PluginDisp_Refresh_10087806', gen=g_none, trials=300),
    dict(name='PluginDisp_Refresh_10087813', orig=0x10087813, c='PluginDisp_Refresh_10087813', gen=g_none, trials=300),
    dict(name='PluginDisp_FillRect', orig=0x10087820, c='PluginDisp_FillRect', gen=g_fill, trials=500),
    dict(name='PluginDisp_Lock', orig=0x10087a0c, c='PluginDisp_Lock', gen=g_lock, trials=300),
]

# round 31: the texture name table is ours now (w31_map.py: the mapping step around the C run)
exec(open('tools/verify/w31_map.py').read())
SOURCES = SOURCES + [x for x in W31_SOURCES + ['../tools/verify/plat_stub.c'] if x not in SOURCES]
SPECS = w31_wrap(SPECS, {'Render_PreloadTextures'})
