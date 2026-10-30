# Round 10: 0x100d4590 (weapon handling of a unit block). Stubbed on both sides: Sim_100203cf, Sim_100d67af,
# Sim_100d650f, Sim_100d666d, Sim_100d60b6, Sim_100c398d, Sim_10024684, the sounds Sim_1000ebfb / Sim_1000eb12 (names
# logged as text), and the kind records' routines +0 / +4 (IND1 / IND2; +4 answers the rounds used). Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100d/w10_weapons.c'])
for _a, _c, _n in ((0x100203cf, 'Sim_100203cf', 2), (0x100d67af, 'Sim_100d67af', 1), (0x100d650f, 'Sim_100d650f', 1),
                   (0x100d666d, 'Sim_100d666d', 1), (0x100d60b6, 'Sim_100d60b6', 2), (0x100c398d, 'Sim_100c398d', 2),
                   (0x10024684, 'Sim_10024684', 2)):
    helper(_a, _c, _n)
helper(0x1000ebfb, 'Sim_1000ebfb', 5, sargs=(0,))
helper(0x1000eb12, 'Sim_1000eb12', 7, sargs=(3,))
helper(IND1, None, 3)
helper(IND2, None, 3)
ACTIVE = tuple(HELPERS)
KINDS = 0x10145508


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    self_ = r.randrange(0, 4)
    now = r.randrange(1000, 5000)
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015243c, r.choice([0, 1]))
    w32(ctx, 0x1015dc78, now)
    w32(ctx, 0x1015dc80, r.choice([0, 10, 33, 100, r.randrange(0, 300)]))
    w32(ctx, 0x1015ef0c, r.choice([0, 0, 1]))
    w32(ctx, 0x1017e67c, r.choice([0, 1]))
    cam = newbuf(ctx, 0x10)
    w32(ctx, cam, *[fb(r.uniform(-500, 500)) for _ in range(3)])
    w32(ctx, 0x1014cae0, cam)
    rets(0x100d60b6, r.choice([0, 1]))
    rets(IND2, *[r.choice([0, 1, 1, 2, 5]) for _ in range(5)])
    for k in range(4):
        rec = KINDS + k * 0xd6
        w32(ctx, rec, r.choice([0, IND1]))
        w32(ctx, rec + 4, r.choice([0, IND2, IND2]))
        w32(ctx, rec + 8, r.choice([0, 1, 2, 3]))
        w32(ctx, rec + 0x10, r.choice([0x3d, 0x3d, 7]))
        w32(ctx, rec + 0x18, r.choice([0x1a, 3, 9]))
        w32(ctx, rec + 0x20, r.choice([0, 1]))
        w32(ctx, rec + 0x2c, r.choice([1, 3, 6]))
        w32(ctx, rec + 0x54, r.choice([0, 50, 500, 5000]))
        w32(ctx, rec + 0x58, r.choice([0, 5, 20, 100, 400]))
        w32(ctx, rec + 0x86, r.choice([0, 1]))
        wb(ctx, rec + 0xb2, r.choice([b'\0', b'GUNFIRE\0', b'MISSILE1\0']))
    ctl = newbuf(ctx, 0x60)
    for off in (0x26, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e):
        w8(ctx, ctl + off, r.choice([0, 0, 0, 1]))
    name = newbuf(ctx, 4)
    w8(ctx, name, r.choice([0x53, 0x41]))
    tbl = newbuf(ctx, 0x100)
    w32(ctx, tbl + 0x90, name)
    obj = newbuf(ctx, 0x100)
    w32(ctx, obj, r.choice([9, 1, 2]))
    w32(ctx, obj + 4, r.choice([self_, self_, 7]))
    w32(ctx, obj + 0x48, tbl)
    w32(ctx, obj + 0x54, ctl)
    w32(ctx, obj + 0x58, *[fb(r.uniform(-500, 500)) for _ in range(3)])
    slots = newbuf(ctx, 8 * 0x128)
    for i in range(8):
        s = slots + i * 0x128
        w32(ctx, s + 4, r.choice([0xffffffff, 0, 1, 2, 3]))
        w32(ctx, s + 8, r.choice([0, 1, 2, 2, 3, 0xffffffff]))
        w32(ctx, s + 0x10, now - r.choice([0, 40, 600, 6000]) if r.random() < 0.6 else r.randrange(0, 400))
        w32(ctx, s + 0x20, r.choice([0xfffffffe, 0, 1]))
        w32(ctx, s + 0x2c, r.choice([0, 1, 3, 6, 0xffffffff]))
        w32(ctx, s + 0x30, r.choice([3, 4, 0]))
        w32(ctx, s + 0x104, r.randrange(0, 4))
        w32(ctx, s + 0x11c, r.randrange(0, 10))
    u = newbuf(ctx, 0xc50)
    w32(ctx, u, obj)
    w32(ctx, u + 0x84, slots)
    w32(ctx, u + 0x4f0, r.choice([2, 2, 2, 1]))
    w32(ctx, u + 0x4fc, r.choice([0xffffffff, 0, 1, r.randrange(8)]))
    w16(ctx, u + 0x560, r.choice([0, 1, 0x80, 0x81, 0x4000, r.getrandbits(16)]))
    w16(ctx, u + 0x562, r.choice([0, 0x800, 0x1000, 0x1800, r.getrandbits(16)]))
    return [u]


SPECS = [dict(name='Sim_100d4590', orig=0x100d4590, c='Sim_100d4590', gen=g, void=True, trials=15000)]
