# Round 16: 0x1006e22c, the per-frame movement of a unit. Run for real on both sides: the distance 0x10084a94, the
# segment step 0x1009cd36, the node reads 0x100a9cfb and the C library (fabs 0x100fcb70, sqrt 0x100fca04, fmod
# 0x100fcb30, 0x100fcc74). Float stubs (fixed answers, the same on both sides): the timers 0x10047bcd, the ground
# height 0x1002d961, the eye height 0x100d6e99, the angle 0x1008486e and the heading 0x10084766. Every other callee is a
# logging stub; 0x100663c0 (the collision tests) also writes what the real one can (hit object, hit gear, the new
# position, the contact counter), 0x100d6f16 writes a whole segment, 0x100a9ca3 its three angles, and 0x1006c161 logs
# the fields of the fall-damage record it is given.
#
# Start values (w16.h, DECISIONS.md round 16): the original's [ebp-0x2c] (the drive speed, read unset on the side-step
# path) and [ebp-0x238] (the second collision test's hit object, unset when that test returns early) are 0 before the
# call; the C has both 0.
# Negative check: W16_NEG=1 gives the original's [ebp-0x2c] a non-zero leftover.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w16_move.c'])
NEG = bool(os.environ.get('W16_NEG'))
CALLS = [
    (0x100949e8, 'Sim_100949e8', 6), (0x100941c3, 'Sim_100941c3', 6), (0x10094fcb, 'Sim_10094fcb', 6),
    (0x10013eba, 'Sim_10013eba', 2), (0x100654a1, 'FlashText_Start', 1), (0x1000ebfb, 'Sim_1000ebfb', 5),
    (0x1000ec60, 'Sim_1000ec60', 5), (0x1000ecb0, 'Sim_1000ecb0', 1), (0x1000e9c9, 'Sim_1000e9c9', 2),
    (0x1009c929, 'Sim_1009c929', 1), (0x100415eb, 'Sim_100415eb', 4), (0x10061f8e, 'Sim_10061f8e', 3),
    (0x10061ecd, 'Sim_10061ecd', 2), (0x1006863d, 'Sim_1006863d', 1), (0x1006c161, 'Sim_1006c161', 5),
    (0x10079feb, 'Sim_10079feb', 1), (0x1006788c, None, 2), (0x100680dd, None, 2), (0x100a9d47, 'Sim_100a9d47', 4),
    (0x100aa0da, 'Sim_100aa0da', 5), (0x100aa799, 'Sim_100aa799', 1), (0x100a9d7c, 'Sim_100a9d7c', 4),
    (0x100386d5, 'Sim_100386d5', 3), (0x10038913, 'Sim_10038913', 3), (0x100d6d6a, 'Sim_100d6d6a', 1),
    (0x100663c0, 'Sim_100663c0', 9), (0x100d6f16, 'Sim_100d6f16', 2), (0x100a9ca3, 'Sim_100a9ca3', 4),
]
for a, c, n in CALLS:
    helper(a, c, n)
SARGS[0x1000ebfb] = {0}
SARGS[0x1000ec60] = {0}
SARGS[0x1000ecb0] = {0}
SARGS[0x1000e9c9] = {0}
for a, c in ((0x10047bcd, 'Sim_10047bcd'), (0x1002d961, 'Sim_1002d961'), (0x100d6e99, 'Sim_100d6e99'),
             (0x1008486e, 'Sim_1008486e'), (0x10084766, 'Sim_10084766')):
    FSTUB[a] = (c, {0x10047bcd: 1, 0x1002d961: 3, 0x100d6e99: 1, 0x1008486e: 2, 0x10084766: 1}[a])
ACTIVE = tuple(a for a, _, _ in CALLS)
T = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def fl(r, lo=-50.0, hi=50.0):
    x = r.random()
    if x < 0.04:
        return r.choice([0, 0x80000000, 0x3f800000, 0xbf800000, 0x33d6bf95, 0xb3d6bf95])
    if x < 0.06:
        return r.choice([0x7fc00000, 0x7f800000])
    return fb(r.uniform(lo, hi))


def _collide(uc, k, raw, eax):
    if not eax:
        return
    st = T['col'][k % len(T['col'])]
    if st['obj'] is not None:
        uc.mem_write(raw[1], struct.pack('<I', st['obj']))
    uc.mem_write(raw[2], struct.pack('<I', st['gear']))
    for i in range(3):
        if st['move'][i] is not None:
            uc.mem_write(raw[6 + i], struct.pack('<I', st['move'][i]))
    uc.mem_write(T['rec'] + 0x4f4, struct.pack('<I', st['count']))
    for i, a in enumerate((0x1015dd4c, 0x1015dd50, 0x1015dd54, 0x1015dd58, 0x1015dd5c, 0x1015dd60)):
        uc.mem_write(a, struct.pack('<I', st['dirs'][i]))


def _segment(uc, k, raw, eax):
    vals = T['seg']
    uc.mem_write(raw[1], struct.pack('<13I', *vals) + struct.pack('<I', 1))


def _angles(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[1 + i], struct.pack('<I', T['ang'][i]))


def _damage(uc, k, raw, eax):
    s = raw[3]
    return [('shot', bytes(uc.mem_read(s + 0x70, 0x0e)), bytes(uc.mem_read(s + 0x86, 4)))], None


EFFECT[0x100663c0] = _collide
EFFECT[0x100d6f16] = _segment
EFFECT[0x100a9ca3] = _angles
EFFECT[0x1006c161] = _damage


def _ground_normal(uc, address, size, ud):
    # the real ground height (0x1002d961) also leaves the ground normal in [0x1015dd4c..54]
    for i, a in enumerate((0x1015dd4c, 0x1015dd50, 0x1015dd54)):
        uc.mem_write(a, struct.pack('<I', T['normal'][i]))


def ground_hook(ctx):
    m = ctx.m
    if getattr(m, '_w16_ground', False):
        return
    m._w16_ground = True
    m.raw.hook_add(UC_HOOK_CODE, _ground_normal, None, 0x1002d961, 0x1002d961)
    m.raw.hook_add(UC_HOOK_CODE, _ground_normal, None, m.syms['Sim_1002d961'], m.syms['Sim_1002d961'])


def part(ctx, r):
    p = newbuf(ctx, 0xc0)
    for off in range(0x30, 0x80, 4):
        w32(ctx, p + off, fl(r, -300, 300))
    w32(ctx, p + 0x8c, newbuf(ctx, 0xc0) if r.random() < 0.7 else 0)
    w32(ctx, p + 4, 0)
    w32(ctx, p + 8, 0)
    return p


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    ground_hook(ctx)
    T['normal'] = [r.choice([0, fl(r, -1, 1), fl(r, -0.01, 0.01)]) for _ in range(3)]
    ST['fvals'] = {0x10047bcd: [fl(r)], 0x1002d961: [fl(r, -5, 5), r.choice([fl(r, -5, 5), fl(r, 100, 2000), fl(r, -2000, -100)])], 0x100d6e99: [fl(r, 0, 10)],
                   0x1008486e: [fl(r, -180, 180), fl(r, -180, 180)], 0x10084766: [fl(r, -1, 1), fl(r, -1, 1)]}
    rets(0x10079feb, *[r.choice([0, 0, 1]) for _ in range(5)])
    rets(0x1009c929, r.randrange(0, 1000))
    rets(0x100663c0, *[r.choice([0, 0, 1, 2]) for _ in range(4)])
    rets(0x100386d5, r.choice([0, 7]))
    rets(0x10038913, r.choice([0, 9]))
    T['ang'] = [fl(r, -90, 90) for _ in range(3)]
    T['seg'] = [fl(r, -300, 300) for _ in range(13)]
    # the unit, its record, its block +0x54 and its node +0x48 (the model code at node +0x90)
    unit = newbuf(ctx, 0x300)
    rec = newbuf(ctx, 0x640)
    T['rec'] = rec
    blk = newbuf(ctx, 0x60)
    for off in (0x1d, 0x1e, 0x25, 0x54, 0x55, 0x56, 0x57, 0x58, 0x5b):
        w8(ctx, blk + off, r.choice([0, 0, 0, 1]))
    w32(ctx, blk + 8, r.choice([0, 0x3f800000, fl(r, -2, 2)]))
    code = ctx.buf(4, r.choice([b'SA\0\0', b'SN\0\0', b'D1\0\0', b'G\0\0\0']))
    node = part(ctx, r)
    w32(ctx, node + 0x90, code)
    if r.random() < 0.5:
        n2 = part(ctx, r)
        w32(ctx, node + r.choice([4, 8]), n2)
        if r.random() < 0.5:
            w32(ctx, n2 + 8, part(ctx, r))
    w32(ctx, rec, unit)
    w32(ctx, unit, r.choice([0, 1, 2, 4, 5, 8, 9, 10]))
    uid = r.randrange(0, 6)
    w32(ctx, unit + 4, uid)
    w32(ctx, 0x1017b8c8, r.choice([uid, uid, (uid + 1) % 6]))
    w32(ctx, unit + 0xc, r.randrange(0, 6))
    w32(ctx, unit + 0x14, r.choice([0, 1, 2, 3]))
    w32(ctx, unit + 0x48, node)
    w32(ctx, unit + 0x4c, part(ctx, r))
    w32(ctx, unit + 0x54, blk)
    for off in (0x58, 0x5c, 0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c, 0xcc, 0xd0, 0x23e, 0x242):
        w32(ctx, unit + off, fl(r, -300, 300) if off < 0x80 else fl(r, -2, 2))
    w32(ctx, unit + 0x80, r.choice([0, 1, 1]))
    w32(ctx, unit + 0x88, r.getrandbits(32))
    w32(ctx, unit + 0x158, r.choice([0, 1]))
    w16(ctx, unit + 0x1a6, r.choice([0, 2, 3]))
    for off in range(0x4, 0x80, 4):
        w32(ctx, rec + off, fl(r, -40, 40))
    for off in (0x520, 0x53c, 0x540, 0x564, 0x4d0, 0x4d4):
        w32(ctx, rec + off, fl(r, -5, 20))
    for off in (0x548, 0x54c, 0x550, 0x554, 0x558, 0x55c):
        w32(ctx, rec + off, fl(r, -30, 30))
    w32(ctx, rec + 0x4c0, part(ctx, r))
    w32(ctx, rec + 0x4c4, part(ctx, r))
    for off in (0x48c, 0x490, 0x494):
        w32(ctx, rec + off, part(ctx, r) if r.random() < 0.6 else 0)
    w32(ctx, rec + 0x4f0, r.choice([0, 2, 2, 4]))
    w32(ctx, rec + 0x4f4, r.choice([0, 1, 3]))
    w32(ctx, rec + 0x4fc, r.randrange(0, 3))
    w32(ctx, rec + 0x84, newbuf(ctx, 0x40))
    w32(ctx, rec + 0x510, r.choice([-2, -1, 0, 0x2d4, 100]))
    w32(ctx, rec + 0x514, r.choice([0, 0, 10, 3]))
    w32(ctx, rec + 0x518, r.choice([0, 0, 1]))
    w16(ctx, rec + 0x560, r.choice([0, 0, 0x20, 0x100, 0x2000, 0x2020]))
    w16(ctx, rec + 0x562, r.choice([0, 0, 1, 4, 8, 0x10, 0x80, 0x100, 0x200, 0x400, 0x60e, r.getrandbits(16)]))
    w32(ctx, rec + 0x58c, r.choice([0, 2]))
    w32(ctx, rec + 0x5ff, r.choice([0, 1]))
    w32(ctx, rec + 0x607, r.choice([0, 0, 1]))
    w32(ctx, rec + 0x60b, r.randrange(0, 200))
    w32(ctx, rec + 0x60f, r.randrange(-50, 200))
    for off in (0xc15, 0xc19, 0xc1d, 0xc21, 0xc25, 0xc29, 0xc2d):
        w32(ctx, rec + off, r.choice([0, 0, 1, 2]))
    w32(ctx, rec + 0xc39, r.choice([0, 0, 5, 200]))
    w32(ctx, rec + 0xc3d, r.choice([0, 0, 0, 3, 200]))
    w32(ctx, rec + 0xc41, r.choice([0, 0, node, part(ctx, r)]))
    # world state
    w32(ctx, 0x1015dc80, r.choice([1, 16, 33, 44, 45, 60]))
    w32(ctx, 0x1013abc0, fl(r, -20, -1))
    w32(ctx, 0x10179ce0, r.choice([0, 1, 1]))
    w32(ctx, 0x1015ec58, r.choice([0, 0, 1]))
    w32(ctx, 0x1015ef0c, r.choice([0, 1]))
    w32(ctx, 0x10179cb4, r.choice([0, 0, 1]))
    w32(ctx, 0x10179cf8, r.choice([0, 0, 1]))
    w32(ctx, 0x101453c4, r.choice([0, 1]))
    cfg = newbuf(ctx, 0x10)
    w8(ctx, cfg + 3, r.choice([0, 1]))
    w32(ctx, 0x1017b8c4, cfg)
    w32(ctx, 0x1014cae0, newbuf(ctx, 0x40))
    hit = newbuf(ctx, 0x30)
    w16(ctx, hit + 0x24, r.randrange(0, 4))
    w16(ctx, hit + 2, r.choice([0, 0x200, 0x100]))
    w32(ctx, 0x109e8230, r.choice([0, hit]))
    slot = 0x10a1d140 + uid * 0x38
    w32(ctx, slot + 0x18, r.choice([0, 0, 1]))
    w32(ctx, slot + 0x1c, r.choice([3, 4, 5, 0]))
    w32(ctx, slot + 0x30, r.choice([0, 1]))
    for a in (0x1015dd4c, 0x1015dd50, 0x1015dd54, 0x1015dd58, 0x1015dd5c, 0x1015dd60):
        w32(ctx, a, fl(r, -1, 1))
    # what the collision tests answer
    gear = newbuf(ctx, 0x40)
    w32(ctx, gear + 0x28, newbuf(ctx, 0x40))
    obj = newbuf(ctx, 0x40)
    w16(ctx, obj + 2, r.choice([0, 0x200]))
    T['col'] = []
    for _ in range(3):
        T['col'].append(dict(obj=r.choice([None, 0, obj]), gear=r.choice([0, 0, gear]),
                             move=[r.choice([None, fl(r, -300, 300)]) for _ in range(3)],
                             count=r.choice([0, 1, 4]), dirs=[fl(r, -1, 1) for _ in range(6)]))
    w32(ctx, 0x109e9bac + r.randrange(0, 4) * 0x50, r.choice([0, 0x3e8]))
    if r.random() < 0.25:
        # a jump without charges while side-stepping on the ground: the path that reads the unset drive speed
        w32(ctx, unit + 0x80, 1)
        w32(ctx, rec + 0x54c, r.choice([0, 0x80000000, 0x33000000]))
        w32(ctx, rec + 0x510, r.choice([0, 0x2d4]))
        w32(ctx, rec + 0x514, 0)
        if r.random() < 0.5:
            w8(ctx, blk + 0x1e, 1)
        else:
            w32(ctx, rec + 0x518, 1)
        w32(ctx, rec + 0x4f0, 2)
        w8(ctx, blk + r.choice([0x57, 0x58]), 1)
        w32(ctx, rec + 0xc3d, 0)
        w16(ctx, rec + 0x562, r.choice([0, 0x100, 0x80, 0x10]))
        wb(ctx, code, b'D1\0\0')
    if r.random() < 0.1:
        # a fast fall onto the ground in state 4 (handed to 0x1006863d on landing)
        w32(ctx, unit + 0x80, 0)
        w32(ctx, rec + 0x4f0, r.choice([4, 2]))
        w32(ctx, rec + 0x54c, fl(r, -300, -40))
        w32(ctx, rec + 0x520, fl(r, 0, 3))
        ST['fvals'][0x1002d961] = [fb(r.uniform(500, 3000))]
    if r.random() < 0.1:
        # the body height: a step down onto lower ground
        w32(ctx, unit + 0x7c, fl(r, -300, 300))
        w32(ctx, unit + 0x5c, fl(r, -300, 300))
    if r.random() < 0.01:
        return [0]
    # the original's start values (and the negative check)
    ebp = frame(1)
    w32(ctx, ebp - 0x2c, fb(r.uniform(1, 30)) if NEG else 0)
    w32(ctx, ebp - 0x238, 0)
    return [rec]


SPECS = [dict(name='Sim_1006e22c', orig=0x1006e22c, c='Sim_1006e22c', gen=g, void=True,
              trials=int(os.environ.get('W16_TRIALS') or (400 if NEG else 12000)))]
