# Round 12: 0x100e2aa3 (ReadWTB, one object of a WTB model). Stubs on both sides: Sim_10008f1c / Sim_10008cbc (write
# the trial's tail pointer), Sim_10008e8b, Sim_10009136 (the vertex floats logged), Sim_100e2837, Sim_100da14c,
# Sim_100091f7 (hands out a polygon record), Sim_10009278, Sim_100094ba, fopen, fclose, fprintf (the strings logged)
# and the error report 0x100bc280. Str_CopyN, Crt_StrUpper, Str_CompareN, Str_Compare and Str_CompareNoCase run for
# real. A name of exactly 16 characters: the byte after the original's buffer ([ebp-0x84]) gets the chosen start
# value 0 as its stack leftover. The checksum (+4) is computed here so the success path runs too.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_100e/w12_wtb.c'])
helper(0x10008f1c, 'Sim_10008f1c', 4)
helper(0x10008cbc, 'Sim_10008cbc', 6)
helper(0x10008e8b, 'Sim_10008e8b', 2)
helper(0x10009136, 'Sim_10009136', 12)
helper(0x100e2837, 'Sim_100e2837', 2, sargs=(0,))
helper(0x100da14c, 'Sim_100da14c', 2)
helper(0x100091f7, 'Sim_100091f7', 3)
MASK[(0x100091f7, 1)] = 0xffff
helper(0x10009278, 'Sim_10009278', 3)
MASK[(0x10009278, 2)] = 0xff
helper(0x100094ba, 'Sim_100094ba', 1)
helper(0x100fd850, None, 2, sargs=(0, 1))
helper(0x100fd760, None, 1)
helper(0x100fd7e0, 'Crt_FPrintf', 2, sargs=(1,))
helper(0x100bc280, None, 2)
ACTIVE = (0x10008f1c, 0x10008cbc, 0x10008e8b, 0x10009136, 0x100e2837, 0x100da14c, 0x100091f7, 0x10009278, 0x100094ba,
          0x100fd850, 0x100fd760, 0x100fd7e0, 0x100bc280)
EBP = frame(3)
T = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def f32(u):
    return struct.unpack('<f', struct.pack('<I', u))[0]


def _tail(slot):
    def fx(uc, k, raw, eax):
        if T['alloc'] == 0:
            return 0
        uc.mem_write(raw[slot], struct.pack('<I', T['tail']))
        return T['alloc']
    return fx


def _fprintf(uc, k, raw, eax):
    fmt = ustr(uc, raw[1])
    if fmt and b'%s' in fmt:
        esp = uc.reg_read(UC_X86_REG_ESP)
        a, b = struct.unpack('<II', bytes(uc.mem_read(esp + 12, 8)))
        return ([('fprintf_args', ustr(uc, a, 40), ustr(uc, b, 40))], 0)
    return 0


EFFECT[0x10008f1c] = _tail(3)
EFFECT[0x10008cbc] = _tail(5)
EFFECT[0x100fd7e0] = _fprintf


def trunc(x):
    if x != x or not (-9.2e18 < x < 9.2e18):
        return 0
    return int(x) & 0xffffffff


def fold(v):
    v = struct.unpack('<i', struct.pack('<I', v & 0xffffffff))[0]
    a = (-v if v < 0 else v) & 0xfffff
    return (-a if v < 0 else a) & 0xffffffff


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, 0x1017e950, 0)
    nv, npoly = r.randrange(0, 4), r.randrange(0, 4)
    base = newbuf(ctx, 0x40)
    off = newbuf(ctx, 4)
    w32(ctx, off, 0)
    size = 0x20 + nv * 0x2c + npoly * 0x3e + 0x40
    hdr = newbuf(ctx, size)
    w32(ctx, off, (hdr - base) & 0xffffffff)
    w32(ctx, hdr, r.choice([r32(ctx, 0x1012bdc0)] * 5 + [0]))
    name = r.choice([b'SOL_ROCK\0', b'TREE01\0', b'WALL\0', b'WXYZ\0', b'POF_HULL\0', b'ABCDEFGHIJKLMNOP',
                     b'abc_def\0', bytes((0x100 - c) & 0xff for c in b'DUM_X\0') + b'\x90' * 10])
    wb(ctx, hdr + 8, (name + bytes(16))[:16])
    w16(ctx, hdr + 0x18, nv)
    w16(ctx, hdr + 0x1a, npoly)
    w16(ctx, hdr + 0x1c, r.choice([0, 1, 2, 0x1001, 0x2002, 5]))
    total = 0
    for i in range(nv):
        v = hdr + 0x20 + i * 0x2c
        fl = [r.choice([0.0, 1.5, r.uniform(-500, 500)]) for _ in range(5)]
        w32(ctx, v, fb(fl[0]), fb(fl[1]), fb(fl[2]))
        wb(ctx, v + 0xc, struct.pack('<ddd', *[r.uniform(-1, 1) for _ in range(3)]))
        w32(ctx, v + 0x24, fb(fl[3]), fb(fl[4]))
        f = [f32(r32(ctx, v + o)) for o in (0, 4, 8, 0x24, 0x28)]
        total = fold(total + trunc(((((f[0] + f[1]) + f[2]) + f[3]) + f[4])))
    p = hdr + 0x20 + nv * 0x2c
    mats = []
    for i in range(npoly):
        n = r.choice([3, 4, 3, 5])
        wb(ctx, p, r.choice([b'Solid Rock\0', b'Solid Sand\0', b'grass\0', b'\0', b'stone\0']))
        w16(ctx, p + 0x10, r.randrange(0, 100))
        wb(ctx, p + 0x16, struct.pack('<ddd', *[r.uniform(-1, 1) for _ in range(3)]))
        w16(ctx, p + 0x2e, n)
        for k in range(min(n, 8)):
            w16(ctx, p + 0x30 + 2 * k, r.randrange(0, 4))
        c = [struct.unpack('<H', bytes(ctx.m.uc.mem_read(p + 0x30 + 2 * k, 2)))[0] for k in range(max(n, 4))]
        total = fold(total + sum(c[:n]) + sum(c[n:4]) + r32(ctx, p + 0x10) % 0x10000 + n)
        p += 0x38 if n <= 4 else 0x3e
    w32(ctx, hdr + 4, total if r.random() < 0.7 else r.getrandbits(32))
    outv = newbuf(ctx, 4)
    model = newbuf(ctx, 0x100)
    blk = newbuf(ctx, 0x18 + 4 * 0x58 + 4 * 0x28 + 0x40)
    w32(ctx, model + 0x30, blk)
    w32(ctx, blk + 8, 0x18 + 4 * 0x58)
    for i in range(4):
        rec = blk + 0x18 + 4 * 0x58 + i * 0x28
        w16(ctx, rec + 2, r.choice([3, 3, 4, 2]))
        w32(ctx, rec + 4, 0x10)
        wb(ctx, rec + 0x10, bytes(r.randrange(0, 4) for _ in range(4)))
        w32(ctx, rec + 8, r.getrandbits(32))
    for i in range(4):
        w32(ctx, blk + 0x18 + i * 0x58 + 8, fb(r.choice([0.0, 1.4, 1.6, -2.5, r.uniform(-5, 5)])))
    w32(ctx, outv, r.choice([0, model]))
    T['alloc'] = r.choice([model, model, 0])
    T['tail'] = newbuf(ctx, 0x40)
    madeb = newbuf(ctx, 0x40)
    rets(0x100091f7, r.choice([madeb, madeb, madeb, 0]))
    m = [newbuf(ctx, 0x60) for _ in range(5)]
    for x in m:
        wb(ctx, x + 0x20, r.choice([b'grass\0', b'GRASS\0', b'STONE\0', b'Solid Rock\0']))
        w32(ctx, x + 0x40, r.choice([0xffffffff, 0, 3, 4, 8, 9, 0x101, r.randrange(0, 12)]))
        w32(ctx, x + 0x44, r.choice([0, m[0], m[1]]) if x != m[0] else 0)
        w32(ctx, x + 0x48, r.randrange(0, 16))
        wb(ctx, x + 0x5d, bytes(r.getrandbits(8) for _ in range(3)))
    for k in range(3):
        w32(ctx, 0x1017e860 + 4 * k, r.choice([0, m[2], m[3], m[4]]))
    w32(ctx, 0x1017e6c0, m[0])
    w32(ctx, 0x1017e6bc, m[1])
    w32(ctx, 0x1017e6c4, r.choice([0, 1]))
    w32(ctx, 0x1017e6c8, r.choice([1, 1, 0]))
    found = newbuf(ctx, 0x40)
    w32(ctx, found + 0x34, r.choice([m[2], m[3]]))
    rets(0x100e2837, r.choice([0, found, found]))
    w32(ctx, 0x1017b954, r.choice([0, 1]))
    w32(ctx, 0x10152648, r.choice([0, 1]))
    w32(ctx, 0x1012da38, r.choice([0x1ff, 5]))
    w32(ctx, 0x1012da3c, r.choice([0x1ff, 5]))
    cfg = newbuf(ctx, 0x20)
    w32(ctx, cfg + 0x14, r.choice([0, 1]))
    w32(ctx, 0x1015d1b8, cfg)
    w32(ctx, 0x1017e858, r.getrandbits(32))
    w8(ctx, EBP - 0x84, 0)                   # the chosen start value: the name ends after its 16th byte
    return [base, off, outv]


SPECS = [dict(name='Sim_100e2aa3', orig=0x100e2aa3, c='Sim_100e2aa3', gen=g, trials=6000)]