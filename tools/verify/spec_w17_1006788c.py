# Round 17: 0x1006788c, the unit-against-unit collision damage (DECISIONS.md round 17). The ORIGINAL runs with the
# original distance estimate 0x10084a94 and the original kill credit 0x10067ec6 inside it, the C with Sim_10084a94 and
# Sim_10067ec6_Credit, so the killer the original reads from its never-written local is whatever its own calls left
# there: the stack below the call is filled with random bytes first, so a killer that did not come from the distance
# call would show as a mismatch. 0x1009c929 (the draw), 0x1006863d and 0x1006c161 (all converted) are logging stubs:
# every hit's killer, record, flag, part and the shot fields 0x1006c161 reads (+0x70 / +0x74 / +0x78 / +0x7c / +0x86)
# are compared, and the stub sometimes adds 2 to +0x70 as the real one does. 0x1008486e (st(0)) is a float stub.
# The speed differences include exact zeros, -0.0, |x| == |z|, NaNs and infinities.
# Negative check: W17_NEG=1 hands the C kill credit the bits of the larger of |x| and |z| (max instead of min).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w17_damage.c'])
NEG = bool(os.environ.get('W17_NEG'))
helper(0x1009c929, 'Sim_1009c929', 1)
helper(0x1006863d, 'Sim_1006863d', 1)
helper(0x1006c161, 'Sim_1006c161', 5)
FSTUB[0x1008486e] = ('Sim_1008486e', 2)
ACTIVE = (0x1009c929, 0x1006863d, 0x1006c161)
BUMP = {}


def _c161(uc, k, raw, eax):
    shot = raw[3]
    f = struct.unpack('<IIIHxxxxxxxxI', bytes(uc.mem_read(shot + 0x70, 0x1a)))
    if BUMP.get(k):
        uc.mem_write(shot + 0x70, struct.pack('<I', (f[0] + 2) & 0xffffffff))
    return [('shot', f)], None


EFFECT[0x1006c161] = _c161


def _neg(uc, address, size, ud):
    """W17_NEG: at the entry of the C Sim_10067ec6_Credit, unsetKiller := bits of max(|x|, |z|)."""
    esp = uc.reg_read(UC_X86_REG_ESP)
    a, b = struct.unpack('<II', bytes(uc.mem_read(esp + 4, 8)))

    def f(p, off):
        return struct.unpack('<f', bytes(uc.mem_read(p + off, 4)))[0]

    def bits(v):
        return struct.unpack('<I', struct.pack('<f', v))[0] & 0x7fffffff
    x, z = bits(f(a, 0x554) - f(b, 0x554)), bits(f(a, 0x55c) - f(b, 0x55c))
    fx, fz = (struct.unpack('<f', struct.pack('<I', v))[0] for v in (x, z))
    uc.mem_write(esp + 0x10, struct.pack('<I', z if not (fx >= fz) else x))


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def speed(r):
    x = r.random()
    if x < 0.05:
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])
    if x < 0.2:
        return fb(r.choice([0.0, -0.0, 1.0, -1.0, 5.0]))
    return fb(r.uniform(-20, 20) if r.random() < 0.7 else r.uniform(-3, 3))


def side(ctx, r, own):
    unit = newbuf(ctx, 0x80)
    rec = newbuf(ctx, 0xc20)
    blk = newbuf(ctx, 0x60)
    w32(ctx, rec, unit)
    w32(ctx, unit, r.choice([1, 1, 1, 9, 9, 3, 4]))
    w32(ctx, unit + 4, own if r.random() < 0.4 else r.choice([0, 1, 7, 12]))
    w32(ctx, unit + 0xc, own if r.random() < 0.5 else 3)
    w32(ctx, unit + 0x14, r.choice([3, 3, 1]))
    w32(ctx, unit + 0x54, blk)
    w8(ctx, blk + 0x59, r.choice([0, 0, 1, 0x80]))
    w32(ctx, unit + 0x68, fb(r.uniform(-400, 400)))
    w32(ctx, unit + 0x74, fb(r.uniform(-30, 30)))
    w32(ctx, rec + 0xbdb, r.choice([5, 0, 3, 20, 50, -7]))
    return rec


def g(ctx):
    r = begin(ctx, *ACTIVE)
    if NEG and not getattr(ctx.m, '_w17_neg', False):
        ctx.m._w17_neg = True
        ctx.m.raw.hook_add(UC_HOOK_CODE, _neg, None, ctx.m.syms['Sim_10067ec6_Credit'],
                           ctx.m.syms['Sim_10067ec6_Credit'])
    fstubs(ctx)
    rets(0x1009c929, *[r.choice([0, 0, 1, 3]) for _ in range(4)])
    for k in range(4):
        BUMP[k] = r.random() < 0.3
    own = r.choice([0, 2, 5])
    w32(ctx, 0x1017b8c8, own)
    w32(ctx, 0x1015ec58, r.choice([0, 0, 1]))
    sw = newbuf(ctx, 8)
    w8(ctx, sw + 3, 1 if r.random() < 0.95 else 0)
    w32(ctx, 0x1017b8c4, sw)
    a, b = side(ctx, r, own), side(ctx, r, own)
    for off in (0x554, 0x558, 0x55c):
        va = speed(r)
        w32(ctx, a + off, va)
        w32(ctx, b + off, va if r.random() < 0.15 else speed(r))
    if r.random() < 0.15:                       # |x| == |z|
        w32(ctx, b + 0x554, 0)
        w32(ctx, b + 0x55c, 0)
        w32(ctx, a + 0x55c, r32(ctx, a + 0x554) ^ (0x80000000 if r.random() < 0.5 else 0))
    w32(ctx, 0x1015dd5c, r.choice([fb(r.uniform(-1, 1)), fb(-0.9), fb(0.9), rfloat(r)]))
    w32(ctx, 0x1015dd58, fb(r.uniform(-1, 1)))
    w32(ctx, 0x1015dd60, fb(r.uniform(-1, 1)))
    ST['fvals'] = {0x1008486e: [fb(r.uniform(-180, 180)) if r.random() < 0.9 else rfloat(r)]}
    lo = frame(2) - 0x600                       # random leftovers everywhere below the call
    ctx.m.uc.mem_write(lo, bytes(r.getrandbits(8) for _ in range(0x600)))
    return [a, b]


SPECS = [dict(name='Sim_1006788c', orig=0x1006788c, c='Sim_1006788c', gen=g, void=True,
              trials=400 if NEG else 20000)]
