# Round 26: 0x10021e8c (one frame of a beam shot). Every callee is stubbed on both sides: the node position
# Sim_100a9cfb (writes its three floats), the muzzle segment Sim_100d6f16 (writes the 0x38-byte segment), the hit test
# Sim_1002dda9 (the segment logged, writes the hit record), and as float stubs answering in st(0) Sim_1008486e and the
# CRT fmod. Deliberate fix: the effect's camera point (Sim_100246b3 arguments 6..8) is the hit point; the original
# passes [ebp-0x8c], [ebp-0x94], [ebp-8], never written, so the generator seeds those three stack words with the
# hit point the segment stub will give. Exact; W26B_MUTATE (camera x / y swapped) must be caught.
exec(open('tools/verify/w10_common.py').read())

if os.environ.get('W26_NEG'):
    CFLAGS.append('-DW26B_MUTATE')

SOURCES = closure(['sim/unsorted_1002/w26_beam.c'])
SEG = 0x38
for _a, _c, _n in ((0x100a9cfb, 'Sim_100a9cfb', 4), (0x100d6f16, 'Sim_100d6f16', 2), (0x1002dda9, 'Sim_1002dda9', 3),
                   (0x1009c929, 'Sim_1009c929', 1), (0x100203cf, 'Sim_100203cf', 2), (0x10015630, 'Sim_10015630', 4),
                   (0x10052be7, 'Sim_10052be7', 1), (0x1006c161, 'Sim_1006c161', 5), (0x10015afd, 'Sim_10015afd', 2),
                   (0x10025aee, 'Sim_10025aee', 6), (0x100246b3, 'Sim_100246b3', 12)):
    helper(_a, _c, _n)
DEREF[0x1002dda9] = {0: SEG}
MASK[(0x10015630, 1)] = 0xffff
MASK[(0x10015630, 2)] = 0xffff
ACTIVE = tuple(HELPERS)
FSTUB.update({0x1008486e: ('Sim_1008486e', 2), 0x100fcb30: (None, 4)})

EBP = STACK_HI - 0x100 - 4 - 4 - 4     # one argument, the return address, then push ebp


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_pos(uc, k, raw, eax):
    v = ST['pos']
    for i in range(3):
        uc.mem_write(raw[1 + i], struct.pack('<I', v[i]))


def e_seg(uc, k, raw, eax):
    uc.mem_write(raw[1], ST['segb'])


def e_dda9(uc, k, raw, eax):
    mode = ST['dda9']
    if mode != 'keep':
        uc.mem_write(raw[1], struct.pack('<I', mode))


EFFECT.update({0x100a9cfb: e_pos, 0x100d6f16: e_seg, 0x1002dda9: e_dda9})


def fl(r, sc):
    return fb(r.uniform(-sc, sc)) if r.random() < 0.97 else rfloat(r)


SHOTS, SHOT = 0x109a3f80, 0x156


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015ecd0, r.choice([0, 0, 1]))
    units = []
    for i in range(8):
        u = newbuf(ctx, 0x160)
        rec = newbuf(ctx, 0xc50)
        w32(ctx, rec, r.getrandbits(32))
        w32(ctx, rec + 0x4f0, r.choice([2, 2, 1]))
        w32(ctx, rec + 0xbbf, r.choice([0, 0, 1]))
        w16(ctx, rec + 0x560, r.choice([0, 0x20, r.getrandbits(16)]))
        w32(ctx, rec + 0x4e0, fl(r, 100))
        w32(ctx, u + 8, r.choice([0, 1]))
        w32(ctx, u + 0xc, r.choice([self_, r.randrange(8)]))
        w32(ctx, u + 0x14, r.choice([3, 3, 1]))
        w32(ctx, u + 0x28, rec)
        w32(ctx, u + 0x68, fl(r, 360))
        w32(ctx, u + 0x150, r.getrandbits(32))
        units.append(u)
    w32(ctx, 0x109e8b40, *units)
    idx = r.randrange(0, 0x100)
    rec = bytearray(rb(ctx, SHOT))
    struct.pack_into('<i', rec, 0x34, r.choice([0, -1, 1, 5, 5]))
    struct.pack_into('<I', rec, 0x3c, newbuf(ctx, 0xa0))
    struct.pack_into('<I', rec, 0x48, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    struct.pack_into('<I', rec, 0x50, r.choice([self_, r.randrange(8), r.randrange(8), 0xffffffff]))
    struct.pack_into('<I', rec, 0x5c, fl(r, 50))
    struct.pack_into('<I', rec, 0x7e, r.choice([0, 0x40, r.getrandbits(32)]))
    kind = r.randrange(0, 0x30)
    struct.pack_into('<I', rec, 0x8e, kind)
    wb(ctx, SHOTS + idx * SHOT, bytes(rec))
    wp = 0x10145508 + kind * 0xd6
    w32(ctx, wp + 0x10, r.choice([4, 6, 1, 2, 2, r.getrandbits(32)]))
    rng = r.uniform(0, 1000)
    w32(ctx, wp + 0x50, fb(rng))
    w32(ctx, wp + 0x86, r.choice([8, 8, 0, r.getrandbits(32)]))
    hr = newbuf(ctx, 0x30)
    w16(ctx, hr + 2, r.choice([0x100, 0x100, 0x100, 0x200, 0x400, 0x800, 0, r.getrandbits(16)]))
    w16(ctx, hr + 0x24, r.choice([self_, r.randrange(8)]))
    w16(ctx, hr + 0x26, r.choice([0, 1, 2, 3]))
    ST['dda9'] = r.choice([hr, hr, hr, 0, 'keep'])
    rets(0x1002dda9, r.choice([1, 1, 1, 0]))
    rets(0x1009c929, r.choice([0, 9, 10, 50]))
    rets(0x10052be7, r.choice([0, 1, 2, 3]))
    ST['pos'] = tuple(fl(r, 100) for _ in range(3))
    seg = bytearray(rb(ctx, SEG))
    for off in (0xc, 0x10, 0x14):
        struct.pack_into('<I', seg, off, fl(r, 100))
    struct.pack_into('<I', seg, 0x30, r.choice([fb(rng), fb(rng * r.random()), fb(rng + 1 + r.random() * 50),
                                                fb(r.uniform(0, 1000)), rfloat(r)]))
    ST['segb'] = bytes(seg)
    # the original's three camera words: seeded with the hit point, what the C passes
    w32(ctx, EBP - 0x8c, struct.unpack_from('<I', seg, 0xc)[0])
    w32(ctx, EBP - 0x94, struct.unpack_from('<I', seg, 0x10)[0])
    w32(ctx, EBP - 8, struct.unpack_from('<I', seg, 0x14)[0])
    ST['fvals'] = {0x1008486e: [fl(r, 360)], 0x100fcb30: [fl(r, 360)]}
    return [idx]


SPECS = [dict(name='Sim_10021e8c', orig=0x10021e8c, c='Sim_10021e8c', gen=g, void=True, trials=15000)]
