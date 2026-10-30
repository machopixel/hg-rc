# Round 10: 0x1001c693 (animation driver). Stubbed on both sides: Sim_1001c618, Sim_100d2669 (projection: writes three
# floats), Anim_RecordOfId (records of the generator or 0) and the animation step Sim_1001c1ab. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1001/w10_misc.c'])
helper(0x1001c618, 'Sim_1001c618', 1)
helper(0x100d2669, 'Sim_100d2669', 3)
helper(0x1001b902, 'Anim_RecordOfId', 1)
helper(0x1001c1ab, 'Sim_1001c1ab', 5)
ACTIVE = (0x1001c618, 0x100d2669, 0x1001b902, 0x1001c1ab)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_proj(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[i], struct.pack('<I', ST['p'][k % 4][i]))


EFFECT[0x100d2669] = e_proj


def anim(ctx, r):
    hdr = newbuf(ctx, 0x10)
    w32(ctx, hdr + 4, r.choice([1, 2, 10, 33, 100]))
    w32(ctx, hdr + 8, r.choice([0, 1, 30, 100, -5 & 0xffffffff]))
    a = newbuf(ctx, 0x20)
    w32(ctx, a, r.choice([1, 1, 0]))
    w32(ctx, a + 0x14, hdr)
    return a


def g(ctx):
    r = begin(ctx, *ACTIVE)
    ST['p'] = [(fb(r.uniform(-100, 800)), fb(r.uniform(-100, 600)), fb(r.uniform(-5, 500))) for _ in range(4)]
    rets(0x1001c618, r.choice([0, 0, 1]), r.choice([0, 0, 1]))
    recs = []
    for k in range(3):
        rr = newbuf(ctx, 8)
        w32(ctx, rr, anim(ctx, r))
        w32(ctx, rr + 4, r.choice([0, 1, 1]))
        recs.append(rr)
    rets(0x1001b902, *[r.choice(recs + [0]) for _ in range(3)])
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015ec58, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b94c, r.getrandbits(32))
    w32(ctx, 0x1015dc80, r.choice([0, 1, 16, 33, 100]))
    cam = newbuf(ctx, 0xa0)
    w32(ctx, cam + 0x94, fb(r.uniform(100, 600)))
    w32(ctx, 0x1014cae0, cam)
    w32(ctx, 0x10185bec, 640)
    w32(ctx, 0x10185bd0, 0)
    w32(ctx, 0x10185be0, 480)
    w32(ctx, 0x10185bf0, 0)
    w32(ctx, 0x10185bf8, fb(r.choice([1.0, 100.0])))
    n = r.randrange(0, 5)
    w32(ctx, 0x1015dc64, n)
    for i in range(n):
        u = newbuf(ctx, 0xa0)
        w32(ctx, u, r.choice([1, 9, 5, 5, 2]))
        w32(ctx, u + 4, r.choice([self_, i + 10]))
        node = newbuf(ctx, 0xa0)
        w32(ctx, node + 0x90, ctx.buf(4, r.choice([b'V10\0', b'V1X\0', b'GE\0\0', b'V\0\0\0'])))
        w32(ctx, u + 0x48, node)
        w32(ctx, u + 0x58, fb(r.uniform(-100, 100)), fb(r.uniform(-100, 100)), fb(r.uniform(-100, 100)))
        w32(ctx, u + 0x8c, r.choice([1, 2, 3, 4, 5, 8, 0xd, 0x11, 0x15, 0x18, 0x19, 7, 0x103]))
        w32(ctx, u + 0x90, r.choice([r32(ctx, u + 0x8c), 6, 9]))
        w32(ctx, u + 0x94, r.choice([1, 3, 2]))
        rec = newbuf(ctx, 0xc20)
        own = newbuf(ctx, 0x90)
        w32(ctx, own + 0x80, r.choice([0, 1]))
        w32(ctx, rec, own)
        w32(ctx, rec + 0x4f0, r.choice([4, 0, 2]))
        w16(ctx, rec + 0x560, r.getrandbits(16))
        w16(ctx, rec + 0x562, r.getrandbits(16))
        w32(ctx, rec + 0x53c, fb(r.uniform(0, 30)))
        w32(ctx, u + 0x28, rec)
        w32(ctx, 0x109e8b40 + 4 * i, u)
        st = 0x10a1d140 + i * 0x38
        w32(ctx, st + 0x14, r.choice([0, anim(ctx, r), anim(ctx, r)]))
        w32(ctx, st + 0x18, r.choice([0, 0, 1]))
        w32(ctx, st + 8, r.randrange(0, 60))
        w32(ctx, st + 0xc, r.randrange(0, 60))
        w32(ctx, st + 0x1c, r.randrange(0, 9))
    return []


SPECS = [dict(name='Sim_1001c693', orig=0x1001c693, c='Sim_1001c693', gen=g, void=True, trials=4000)]
