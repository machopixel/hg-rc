# Round 12: 0x100c7120 (target readout pane). Stubs on both sides: the pane method +0x114 (IND1, text logged), the
# loader, the HUD library (lines 0x100ef727 / 0x100e86a1, text 0x100f2f01 / 0x100f1e10 / 0x100ecea6, font height
# 0x100ecce0), Sim_100bb2b0, Sim_10052c02, Sim_10052be7, Sim_100cba68, Sim_100d2669 (writes the trial's projection),
# Sim_100ccb0d, Sim_1009c929, Sim_1008dba2 and sprintf (its %f needs C runtime start-up tables: the stub formats with
# Python, same text on both sides). Sim_1000946e, strlen and Str_Copy run for real. For other kinds the original's
# unset position / size / rank get the chosen start value 0 as stack leftovers.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_100c/w12_readout.c'])
helper(IND1, None, 2, sargs=(1,))
helper(0x100ba7d5, None, 3)
helper(0x100ef727, None, 7)
helper(0x100e86a1, None, 7)
helper(0x100f2f01, None, 6, sargs=(4,))
helper(0x100f1e10, None, 6, sargs=(4,))
helper(0x100ecea6, None, 6, sargs=(4,))
helper(0x100ecce0, None, 1)
helper(0x100bb2b0, 'Sim_100bb2b0', 2)
helper(0x10052c02, 'Sim_10052c02', 1)
helper(0x10052be7, 'Sim_10052be7', 1)
helper(0x100cba68, 'Sim_100cba68', 0)
helper(0x100d2669, 'Sim_100d2669', 3)
helper(0x100ccb0d, 'Sim_100ccb0d', 2, sargs=(0,))
helper(0x1009c929, 'Sim_1009c929', 1)
helper(0x1008dba2, 'Sim_1008dba2', 3, sargs=(1,))
helper(0x100fc780, None, 2)     # dst and format only: the value is compared where the text is used
_SPRINTF_ARGS = 4
ACTIVE = (IND1, 0x100ba7d5, 0x100ef727, 0x100e86a1, 0x100f2f01, 0x100f1e10, 0x100ecea6, 0x100ecce0, 0x100bb2b0,
          0x10052c02, 0x10052be7, 0x100cba68, 0x100d2669, 0x100ccb0d, 0x1009c929, 0x1008dba2, 0x100fc780)
EBP = frame(1)
T = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def _proj(uc, k, raw, eax):
    for a, v in zip(raw, T['proj']):
        uc.mem_write(a, struct.pack('<I', v))
    return T['on']


def _sprintf(uc, k, raw, eax):
    fmt = bytes(uc.mem_read(raw[1], 16)).split(b'\0')[0]
    if b'f' in fmt:
        v = struct.unpack('<d', bytes(uc.mem_read(uc.reg_read(UC_X86_REG_ESP) + 12, 8)))[0]
        s = b'\n%2.2fk' % v
    else:
        s = b'\n%3ldm' % struct.unpack('<i', bytes(uc.mem_read(uc.reg_read(UC_X86_REG_ESP) + 12, 4)))[0]
    uc.mem_write(raw[0], s + b'\0')
    return len(s)


EFFECT[0x100d2669] = _proj
EFFECT[0x100fc780] = _sprintf


def g(ctx):
    r = begin(ctx, *ACTIVE)
    T['on'] = r.choice([0, 1])
    T['proj'] = [fb(r.uniform(-50, 700)), fb(r.uniform(-50, 500)), r.choice([fb(r.uniform(1, 900)), 0])]
    rets(0x100ba7d5, r.choice([0, 0x00c00000, 0x00c00000, 0x00c00000]))
    rets(0x10052c02, r.choice([0, 1, 2, 3]))
    rets(0x10052be7, r.choice([0, 1, 2, 3]))
    rets(0x100ccb0d, *[r.randrange(0, 80) for _ in range(4)])
    rets(0x100ecce0, r.randrange(4, 12))
    rets(0x1009c929, r.randrange(0, 100))
    model = newbuf(ctx, 0x60)
    w32(ctx, model + 0x44, fb(r.uniform(-1000, 1000)), fb(r.uniform(0, 300)), fb(r.uniform(-1000, 1000)),
        fb(r.uniform(0, 40)))
    obj = newbuf(ctx, 0x90)
    w32(ctx, obj + 0x8c, model)
    rets(0x100cba68, r.choice([0, obj, obj]))
    pane = newbuf(ctx, 0x120)
    w16(ctx, pane + 4, r.choice([0, 1, 1, 1]))
    wb(ctx, pane + 0x10, r.choice([b'\0', b'TARGET\0']))
    w32(ctx, pane + 0xec, r.getrandbits(32))
    w32(ctx, pane + 0x114, IND1)
    self_ = r.randrange(8)
    w32(ctx, 0x1017b8c8, self_)
    for i in range(8):
        u = newbuf(ctx, 0x140)
        rec = newbuf(ctx, 0xc00)
        w32(ctx, u + 0x28, rec)
        w32(ctx, u + 0x58, fb(r.uniform(-1000, 1000)), fb(r.uniform(0, 300)), fb(r.uniform(-1000, 1000)))
        w16(ctx, u + 0x1c, r.choice([0, 0x20, 0x100, 0x120]))
        wb(ctx, u + 0x124, r.choice([b'\0', b'Bob\0']))
        wb(ctx, u + 0x13a, b'unit\0')
        w32(ctx, rec + 0x53c, fb(r.uniform(0, 30)))
        w32(ctx, rec + 0xbdf, r.randrange(0, 9))
        w32(ctx, rec + 0xbe3, r.randrange(0, 9))
        w32(ctx, 0x109e8b40 + 4 * i, u)
        w16(ctx, 0x109a8664 + 0x54 * i, r.choice([0, 0x20, 0x100, 0x120]))
        wb(ctx, 0x109a8668 + 0x54 * i, r.choice([b'\0', b'tree\0']))
        w16(ctx, 0x109e9ba0 + 0x50 * i, r.choice([0, 0x20, 0x100, 0x120]))
        wb(ctx, 0x109e9bb8 + 0x50 * i, r.choice([b'\0', b'bldg\0']))
    me = r32(ctx, 0x109e8b40 + 4 * self_)
    myrec = r32(ctx, me + 0x28)
    head = newbuf(ctx, 0x100)
    w32(ctx, myrec, head)
    w32(ctx, head + 0xf0, r.choice([0, 0x100, 0x200, 0x400, 0x300, 0x1200]) | r.randrange(0, 8))
    w32(ctx, head + 0xd8, fb(r.choice([r.uniform(0, 300000), r.uniform(0, 90000), -500.0, 0.0])))
    w32(ctx, 0x1017e58c, r.choice([0, r32(ctx, head + 0xf0)]))
    w32(ctx, 0x1017e5ac, r.choice([0, 1, 2]))
    w32(ctx, 0x1017e588, r.choice([0, 0, 50, 5000]))
    w32(ctx, 0x1015dc78, r.choice([10, 1000, 6000]))
    w8(ctx, 0x1015da30, r.choice([0, 0x41]))
    w32(ctx, 0x1017e570, r.choice([0, 1]))
    for a in (0x101c47b4, 0x101c47b8, 0x101c47c0, 0x101c47c4, 0x101c47d4, 0x101c47d8, 0x101c47e4):
        s = newbuf(ctx, 16)
        wb(ctx, s, b'txt%x\0' % (a & 0xff))
        w32(ctx, a, s)
    for k in range(5):
        w32(ctx, 0x1014be71 + 0x1d * k, r.choice([0, 100, 1000, 5000]))
    cam = newbuf(ctx, 0x100)
    w32(ctx, cam + 0x94, fb(r.uniform(100, 600)))
    w32(ctx, 0x1014cae0, cam)
    w32(ctx, 0x1015dc38, r.choice([0, 2, 8]))
    w32(ctx, 0x101c64f0, r.choice([0, 1]))
    w32(ctx, 0x109e3de8, r.randrange(100, 700))
    w32(ctx, 0x109e3da8, r.randrange(100, 500))
    w32(ctx, 0x10179ca4, r.choice([0, 1]))
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    for off in (0x28, 0x70, 0x74, 0x78, 0x7c):  # the chosen start values as the original's leftovers
        w32(ctx, EBP - off, 0)
    return [pane]


SPECS = [dict(name='Sim_100c7120', orig=0x100c7120, c='Sim_100c7120', gen=g, void=True, trials=8000)]