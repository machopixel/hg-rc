# Round 17: 0x100680dd, the collision damage against a world object (DECISIONS.md round 17). The ORIGINAL runs with the
# original distance estimate 0x10084a94 and kill credit 0x10067ec6 inside it, the C with Sim_10084a94 and
# Sim_10067ec6_Credit. The original never sets its shot's kind word +0x7c ([ebp-0xf4]): before each call that slot
# gets one of the leftovers traced for it (stack and record addresses, the world objects' flags words 0 / 0x10 / 0xb0
# / 0xd0 / 0xe0 / 0x800, the runtime words 0x20 / 0x50 / 0x60 / 0x70 / 0x1a0 / 0x400 / 0xff9f / 0x1xx), the rest of
# the stack below the call random bytes. 0x1006c161 (logging stub) logs the shot fields it reads, the kind as its
# class (1..4 itself, anything else "other"), and sometimes adds 2 to +0x70 as the real one does; 0x1009c929,
# 0x10024616, 0x1002597b, 0x10025aee are logging stubs (all converted).
# Negative check: W17_NEG=1 gives the original's kind 1..4, which must show as mismatches.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w17_damage.c'])
NEG = bool(os.environ.get('W17_NEG'))
helper(0x1009c929, 'Sim_1009c929', 1)
helper(0x1006863d, 'Sim_1006863d', 1)
helper(0x1006c161, 'Sim_1006c161', 5)
helper(0x10024616, 'Sim_10024616', 8)
helper(0x1002597b, 'Sim_1002597b', 1)
helper(0x10025aee, 'Sim_10025aee', 6)
FSTUB[0x1008486e] = ('Sim_1008486e', 2)
ACTIVE = (0x1009c929, 0x1006863d, 0x1006c161, 0x10024616, 0x1002597b, 0x10025aee)
BUMP = {}
KINDS = [0, 0x10, 0xb0, 0xd0, 0xe0, 0x800, 0x20, 0x50, 0x60, 0x70, 0x1a0, 0x400, 0xff9f, 0x100, 0x1b0, 0x900,
         0xc300, 0xf53c, 0xf4c8, 0x0000, 0x2350, 0x8b40]


def _c161(uc, k, raw, eax):
    shot = raw[3]
    f = struct.unpack('<IIIHxxxxxxxxI', bytes(uc.mem_read(shot + 0x70, 0x1a)))
    kind = f[3] if 1 <= f[3] <= 4 else 'other'
    if BUMP.get(k) and shot != 0x101450a8:
        uc.mem_write(shot + 0x70, struct.pack('<I', (f[0] + 2) & 0xffffffff))
    return [('shot', f[:3] + (kind, f[4]))], None


EFFECT[0x1006c161] = _c161


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def speed(r):
    x = r.random()
    if x < 0.05:
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])
    if x < 0.15:
        return fb(r.choice([0.0, -0.0, 1.0, -1.0, 5.0]))
    return fb(r.uniform(-20, 20) if r.random() < 0.7 else r.uniform(-3, 3))


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    rets(0x1009c929, *[r.choice([0, 0, 1, 3]) for _ in range(4)])
    for k in range(4):
        BUMP[k] = r.random() < 0.3
    own = r.choice([0, 2, 5])
    w32(ctx, 0x1017b8c8, own)
    w32(ctx, 0x1015ec58, r.choice([0, 0, 1]))
    w32(ctx, 0x10179ce0, 1 if r.random() < 0.95 else 0)
    sw = newbuf(ctx, 8)
    w8(ctx, sw + 3, 1 if r.random() < 0.9 else 0)
    w32(ctx, 0x1017b8c4, sw)
    unit = newbuf(ctx, 0x80)
    rec = newbuf(ctx, 0xc20)
    blk = newbuf(ctx, 0x60)
    w32(ctx, rec, unit)
    w32(ctx, unit, r.choice([1, 1, 1, 9, 3, 4]))
    w32(ctx, unit + 4, own if r.random() < 0.5 else r.choice([0, 1, 7, 12]))
    w32(ctx, unit + 0xc, own if r.random() < 0.5 else 3)
    w32(ctx, unit + 0x14, r.choice([3, 3, 1]))
    w32(ctx, unit + 0x54, blk)
    w8(ctx, blk + 0x59, r.choice([0, 0, 1, 0x80]))
    w32(ctx, unit + 0x68, fb(r.uniform(-400, 400)))
    w32(ctx, unit + 0x74, fb(r.uniform(-30, 30)))
    w32(ctx, rec + 0xbdb, r.choice([5, 0, 3, 20, 50, -7]))
    for off in (0x554, 0x558, 0x55c):
        w32(ctx, rec + off, speed(r))
    obj = 0
    if r.random() < 0.85:
        obj = newbuf(ctx, 0x60)
        w16(ctx, obj + 2, r.choice([0, 0x10, 0xb0, 0xb0, 0xd0, 0xe0, 0x800, 0x200, 0x2b0, 0x2d0, 0x210, r.getrandbits(16)]))
        idx = r.randrange(0, 8)
        w16(ctx, obj + 0x24, idx)
        w32(ctx, 0x109e9ba0 + idx * 0x50 + 2, r.choice([0, 1, 2, 3, r.getrandbits(32)]))
        for off in (0x44, 0x48, 0x4c):
            w32(ctx, obj + off, fb(r.uniform(-500, 500)))
    w32(ctx, 0x1015dd5c, r.choice([fb(r.uniform(-1, 1)), fb(-0.9), fb(0.9), rfloat(r)]))
    w32(ctx, 0x1015dd58, fb(r.uniform(-1, 1)))
    w32(ctx, 0x1015dd60, fb(r.uniform(-1, 1)))
    ST['fvals'] = {0x1008486e: [fb(r.uniform(-180, 180)) if r.random() < 0.9 else rfloat(r)]}
    lo = frame(2) - 0x600
    ctx.m.uc.mem_write(lo, bytes(r.getrandbits(8) for _ in range(0x600)))
    if NEG:
        kind = r.choice([1, 2, 3, 4])
    else:
        kind = r.choice(KINDS + [(r.getrandbits(14) << 2) | 0x100, r.getrandbits(16) & 0xfffc])
        if 1 <= kind <= 4:
            kind = 0
    w16(ctx, frame(2) - 0xf4, kind)
    return [rec, obj]


SPECS = [dict(name='Sim_100680dd', orig=0x100680dd, c='Sim_100680dd', gen=g, void=True,
              trials=400 if NEG else 20000)]
