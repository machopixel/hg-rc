# Round 14: 0x10070eb5 (per-frame update of a gear's record). Every callee is stubbed on both sides; the float answers
# of Sim_10084766 and Sim_10084a94 come from float stubs, sprintf copies its format (the message is the shipped
# "MASC malfunction.", DECISIONS.md round 14) and Sim_1007df6c logs the text. The DLL's fabs runs for real.
# No negative check: an overflowing message would overflow the C buffer as well.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1007/w14_unit.c'])
CALLS = [
    (0x100d4590, 'Sim_100d4590', 1), (0x100d63d4, 'Sim_100d63d4', 0), (0x1006b52f, 'Sim_1006b52f', 5),
    (0x10011681, 'Sim_10011681', 1), (0x1001349a, 'Sim_1001349a', 0), (0x100d6392, 'Sim_100d6392', 1),
    (0x100cbdb8, 'Sim_100cbdb8', 3), (0x100ca670, 'Sim_100ca670', 3), (0x100cbe94, 'Sim_100cbe94', 0),
    (0x100cbe3c, 'Sim_100cbe3c', 1), (0x100cbe68, 'Sim_100cbe68', 1), (0x100cbdea, 'Sim_100cbdea', 1),
    (0x100cbe13, 'Sim_100cbe13', 1), (0x100cbaf8, 'Sim_100cbaf8', 0), (0x100cb4fb, 'Sim_100cb4fb', 1),
    (0x100d6833, 'Sim_100d6833', 1), (0x10069c90, 'Sim_10069c90', 1), (0x1000ebfb, 'Sim_1000ebfb', 5),
    (0x100203cf, 'Sim_100203cf', 2), (0x1004f257, 'Sim_1004f257', 1), (0x1000ec60, 'Sim_1000ec60', 5),
    (0x1000ecb0, 'Sim_1000ecb0', 1), (0x10061dfc, 'Sim_10061dfc', 1), (0x10079feb, 'Sim_10079feb', 1),
    (0x1009c929, 'Sim_1009c929', 1), (0x100fc780, None, 2), (0x1007df6c, 'Sim_1007df6c', 4),
    (0x1004f158, 'Sim_1004f158', 1), (0x100d4500, 'Sim_100d4500', 1), (0x1006c9a0, 'Sim_1006c9a0', 2),
    (0x10052be7, 'Sim_10052be7', 1), (0x10061c32, 'Sim_10061c32', 1), (0x1007cbb4, 'Sim_1007cbb4', 0),
]
for a, c, n in CALLS:
    helper(a, c, n)
SARGS[0x1007df6c] = {0}
FSTUB[0x10084766] = ('Sim_10084766', 1)
FSTUB[0x10084a94] = ('Sim_10084a94', 3)
ACTIVE = tuple(a for a, _, _ in CALLS)


def _sprintf(uc, k, raw, eax):
    fmt = _cstr(uc, raw[1])
    assert b'%' not in fmt
    uc.mem_write(raw[0], fmt + b'\0')
    return len(fmt)


EFFECT[0x100fc780] = _sprintf


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def fv(r, *special):
    x = r.random()
    if x < 0.25 and special:
        return fb(r.choice(special))
    if x < 0.3:
        return r.choice([0, 0x80000000, 0x7fc00000, 0xff800000])
    if x < 0.6:
        return fb(r.uniform(-2, 2))
    return fb(r.uniform(-400, 400))


def text(ctx, s):
    a = newbuf(ctx, len(s) + 1)
    wb(ctx, a, s + b'\0')
    return a


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    ST['fvals'] = {0x10084766: [fv(r, 1.0, 0.5) for _ in range(2)], 0x10084a94: [fv(r, 0.0, 30.0) for _ in range(2)]}
    rets(0x10079feb, r.choice([0, 1]), r.choice([0, 1]))
    rets(0x1009c929, r.choice([0xc, 0xc, 0, 5, 100]))
    rets(0x10052be7, r.choice([0, 1, 1, 2]))
    me = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, me)
    rec = newbuf(ctx, 0xc50)
    unit = newbuf(ctx, 0x100)
    blk = newbuf(ctx, 0x60)
    w32(ctx, rec, unit)
    w32(ctx, unit + 0x54, blk)
    w32(ctx, unit, r.choice([0, 1, 1, 2, 4, 5, 5, 8, 9, 10, r.randrange(0, 16)]))
    w32(ctx, unit + 4, r.choice([me, me, me, (me + 1) % 4]))
    w32(ctx, unit + 0xc, r.choice([me, (me + 2) % 4]))
    w32(ctx, unit + 0x14, r.choice([0, 0, 1, 2, 3]))
    w32(ctx, unit + 0x88, r.getrandbits(4))
    w32(ctx, unit + 0xf0, r.choice([0, 0x1000, r.getrandbits(32)]))
    w16(ctx, unit + 0x1c, r.getrandbits(16))
    w32(ctx, unit + 0x68, fv(r, 0.0, 180.0, -180.0, 200.0, -200.0, 90.0, -90.0, 350.0))
    w16(ctx, rec + 0x560, r.choice([0, 0, 0x200, r.getrandbits(16)]))
    w16(ctx, rec + 0x562, r.choice([0, 0, 1, 2, 0x200, r.getrandbits(16)]))
    w32(ctx, rec + 0x4f0, r.choice([2, 2, 2, 2, 0, 1, 3, 4, 7, 5, 8]))
    w32(ctx, rec + 0x4d8, r.choice([0, 100, 5000, r.getrandbits(32)]))
    w32(ctx, rec + 0x50c, r.choice([0, 0, 1, 6]))
    w32(ctx, rec + 0x510, r.choice([-2, -2, -1, 0, 1, 0x2d4, 100, 30]))
    w32(ctx, rec + 0x514, r.choice([0, 1, 3]))
    w32(ctx, rec + 0x518, r.choice([0, 0, 1]))
    for off in (0xc35, 0xc3d, 0xbcb, 0xbd3):
        w32(ctx, rec + off, r.choice([0, 0, 1, 0x200, r.getrandbits(32)]))
    for off in (8, 0xc, 0x18, 0x28, 0x38, 0x48, 0x58, 0x4e0, 0x4e8, 0x4ec, 0x540, 0x548, 0x550):
        w32(ctx, rec + off, fv(r, 0.0, 1.0, -1.0, 0.5, -0.5, 1e-8))
    w32(ctx, rec + 0x2c, fv(r, 0.0, 15.0, 14.0, -1.0, 1e-9))
    w32(ctx, rec + 0x78, fv(r, 0.015625, 0.0))
    w32(ctx, rec + 0x7c, fv(r, 0.015625, 0.0, 0.0161))
    w32(ctx, rec + 0x4d4, fv(r, 0.0, 10.0))
    w32(ctx, rec + 0x500, fv(r, 0.0, 1e-8, 3.0))
    w32(ctx, rec + 0x534, fv(r, 10.0, 0.0))
    for off in range(0, 0x18, 4):
        w32(ctx, blk + off, fv(r, 0.0, 1e-8, 1.0, -46.0, -50.0, 200.0))
    for off in range(0x18, 0x60):
        w8(ctx, blk + off, 1 if r.random() < 0.12 else 0)
    w32(ctx, 0x1014b930, r.choice([-1, 0, 3]))
    w32(ctx, 0x1014b934, r.choice([-1, 0, 2]))
    for a in (0x1015ec58, 0x10a3ade8, 0x1015f1e0, 0x10179ce8, 0x10179cf4, 0x1017cc5c, 0x10179cb8, 0x10179cc4,
              0x10179ce4, 0x10179ce0, 0x1015ef0c):
        w32(ctx, a, r.choice([0, 0, 1]))
    w32(ctx, 0x1015dc78, r.choice([0, 200, 5000, r.getrandbits(32)]))
    w32(ctx, 0x1015dc80, r.choice([1, 2, 30, 1000]))
    w32(ctx, 0x10179d00, r.choice([0, 4900, r.getrandbits(32)]))
    w32(ctx, 0x10179cdc, fv(r, 0.0, 180.0, -180.0, 200.0, -200.0, 95.0, -95.0))
    w32(ctx, 0x10179cec, fv(r, 0.0, 1e-8, 3.0, 60.0))
    w32(ctx, 0x1013abc0, fv(r, 1.0))
    sb = newbuf(ctx, 0x10)
    w8(ctx, sb + 1, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b8c4, sb)
    w32(ctx, 0x1014cae0, newbuf(ctx, 0x10))
    msg = b'MASC malfunction.'
    w32(ctx, 0x101c4648, text(ctx, msg))
    w32(ctx, 0x109b98c0, r.getrandbits(32))
    return [rec]


SPECS = [dict(name='Sim_10070eb5', orig=0x10070eb5, c='Sim_10070eb5', gen=g, void=True, trials=15000)]
