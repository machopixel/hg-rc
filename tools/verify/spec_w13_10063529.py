# Round 13: 0x10063529 (waypoint marker and target markers). Stubs on both sides: Sim_100ca996, Sim_100d2669 (writes
# the trial's projection), Sim_1006519d (marker name logged), the loader 0x100ba7d5, the HUD library (text
# 0x100f2f01 / 0x100f1e10 / 0x100ecea6 logged as text, lines 0x100ef727 / 0x100e86a1, font height 0x100ecce0),
# Sim_100ccb0d (text logged), Sim_100bb2b0, Sim_10052be7, Sim_10052c02, Sim_100cba68, Sim_100648a0, Sim_10064d05,
# Sim_10063037 and sprintf (its %f needs C runtime start-up tables: the stub formats with Python, same text on both
# sides). Str_Copy, strlen and the absolute value run for real.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w13_marker.c'])
helper(0x100ca996, 'Sim_100ca996', 0)
helper(0x100d2669, 'Sim_100d2669', 3)
helper(0x1006519d, 'Sim_1006519d', 3, sargs=(2,))
helper(0x100ba7d5, None, 3)
helper(0x100f2f01, None, 6, sargs=(4,))
helper(0x100f1e10, None, 6, sargs=(4,))
helper(0x100ecea6, None, 6, sargs=(4,))
helper(0x100ef727, None, 7)
helper(0x100e86a1, None, 7)
helper(0x100ecce0, None, 1)
helper(0x100ccb0d, 'Sim_100ccb0d', 2, sargs=(0,))
helper(0x100bb2b0, 'Sim_100bb2b0', 2)
helper(0x10052be7, 'Sim_10052be7', 1)
helper(0x10052c02, 'Sim_10052c02', 1)
helper(0x100cba68, 'Sim_100cba68', 0)
helper(0x100648a0, 'Sim_100648a0', 2)
helper(0x10064d05, 'Sim_10064d05', 3)
helper(0x10063037, 'Sim_10063037', 1)
helper(0x100fc780, None, 2)     # dst and format only: the text is compared where it is used
ACTIVE = (0x100ca996, 0x100d2669, 0x1006519d, 0x100ba7d5, 0x100f2f01, 0x100f1e10, 0x100ecea6, 0x100ef727,
          0x100e86a1, 0x100ecce0, 0x100ccb0d, 0x100bb2b0, 0x10052be7, 0x10052c02, 0x100cba68, 0x100648a0,
          0x10064d05, 0x10063037, 0x100fc780)
T = {}
_CONV = re.compile(rb'%(\d*)(?:\.(\d+))?([sf])')


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def _proj(uc, k, raw, eax):
    for a, v in zip(raw, T['proj']):
        uc.mem_write(a, struct.pack('<I', v))
    return T['on']


def _sprintf(uc, k, raw, eax):
    fmt = _cstr(uc, raw[1])
    at = uc.reg_read(UC_X86_REG_ESP) + 12
    out, pos = b'', 0
    for m in _CONV.finditer(fmt):
        out += fmt[pos:m.start()]
        pos = m.end()
        if m.group(3) == b's':
            out += _cstr(uc, struct.unpack('<I', bytes(uc.mem_read(at, 4)))[0])
            at += 4
        else:
            v = struct.unpack('<d', bytes(uc.mem_read(at, 8)))[0]
            out += (b'%' + m.group(0)[1:]) % v
            at += 8
    out += fmt[pos:]
    uc.mem_write(raw[0], out + b'\0')
    return len(out)


EFFECT[0x100d2669] = _proj
EFFECT[0x100fc780] = _sprintf


def text(ctx, s):
    a = newbuf(ctx, len(s) + 1)
    wb(ctx, a, s + b'\0')
    return a


def g(ctx):
    r = begin(ctx, *ACTIVE)
    T['on'] = r.choice([0, 1, 1])
    T['proj'] = [fb(r.uniform(-50, 700)), fb(r.uniform(-50, 500)), r.choice([fb(r.uniform(1, 900)), 0])]
    rets(0x100ba7d5, r.choice([0, 0x00c00000, 0x00c00000, 0x00c00000]))
    rets(0x100ccb0d, *[r.randrange(0, 80) for _ in range(3)])
    rets(0x100ecce0, r.randrange(4, 12))
    rets(0x100cba68, r.choice([0, 0x00c00000]))
    w32(ctx, 0x1017e580, r.randrange(-50, 700) & 0xffffffff)
    w32(ctx, 0x1017e584, r.randrange(-50, 500) & 0xffffffff)
    if r.random() < 0.3:
        w32(ctx, 0x1017e580, struct.unpack('<f', struct.pack('<I', T['proj'][0]))[0].__int__() + r.randrange(-31, 32))
        w32(ctx, 0x1017e584, struct.unpack('<f', struct.pack('<I', T['proj'][1]))[0].__int__() + r.randrange(-31, 32))
    w32(ctx, 0x10176954, r.choice([0, 6, 13, 0x7fffffff, -1 & 0xffffffff, -8 & 0xffffffff]))
    for k in range(-6, 7):                   # a negative counter indexes before the table (more name pointers)
        w32(ctx, 0x109e857c + 4 * k, text(ctx, b'HOLF%d' % k))
    w32(ctx, 0x101c4ed0, text(ctx, r.choice([b'NAV ', b'WAY%'.replace(b'%', b'')])))
    w32(ctx, 0x101c4ea4, text(ctx, b'RNG '))
    w32(ctx, 0x1015e4c4, fb(r.choice([r.uniform(0, 200000), r.uniform(0, 100000), 100000.0, 100000.01, -5.0])))
    for i in range(4):
        w32(ctx, 0x109a8640 + 0x54 * i, r.choice([0, 1, 1]))
        wb(ctx, 0x109a8668 + 0x54 * i, r.choice([b'\0', b'Alpha\0']))
        w32(ctx, 0x109e8b40 + 4 * i, newbuf(ctx, 0x40))
    for a in (0x101668e8, 0x1017b940):
        w32(ctx, a, r.choice([0, 1]))
    w16(ctx, 0x101659f8, r.getrandbits(16))
    w32(ctx, 0x109b98c0, r.getrandbits(32))
    w32(ctx, 0x109b9900, r.getrandbits(32))
    w32(ctx, 0x1014cec4, r.getrandbits(32))
    obj = newbuf(ctx, 0x120)
    w32(ctx, obj + 0x118, r.choice([0, 0x100, 0x100, 0x1100, 0x200, 0x101, 0x102, 0x103]) | r.choice([0, 0, 0x10000]))
    w32(ctx, obj + 0x104, fb(r.uniform(-1000, 1000)), fb(r.uniform(-50, 300)), fb(r.uniform(-1000, 1000)))
    w32(ctx, obj + 0xf0, r.choice([0, 0x100, 0x1200, 0x200, 0x201, 0x400, 0x403, 0x300, 0x100]))
    holder = newbuf(ctx, 8)
    w32(ctx, holder, obj)
    return [holder]


SPECS = [dict(name='Sim_10063529', orig=0x10063529, c='Sim_10063529', gen=g, void=True, trials=8000)]
