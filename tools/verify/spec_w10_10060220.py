# Round 10, interpreted (DECISIONS.md): 0x10060220 (rotation matrix from angles). Stubbed on both sides (called through
# their original addresses): 0x1006018d (identity; the stub writes one), 0x1005fcdc and 0x1006007d (the matrix they
# read logged). The CRT fabs runs for real; the CRT sin / cos of the original against the platform math library of
# the C (libm()). Exact but for the matrix floats, which may differ within the stated tolerance. Angles also take
# inf / NaN (domain wrappers); huge finite angles are left out (the CRT reduces with an 80-bit 2 pi).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/util/w10_interp.c'])
helper(0x1006018d, None, 1)
helper(0x1005fcdc, None, 2, deref={0: 0x30})
helper(0x1006007d, None, 2, deref={0: 0x30})
ACTIVE = tuple(HELPERS)
IDENT = struct.pack('<12f', 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_ident(uc, k, raw, eax):
    uc.mem_write(raw[0], IDENT)


EFFECT.update({0x1006018d: e_ident})


def ang(r):
    return r.choice([fb(r.uniform(-360, 360)), fb(r.uniform(-360, 360)), 0, 0, 0x80000000, fb(1e-7), fb(-1.1e-7),
                     fb(r.uniform(-1e-6, 1e-6)), fb(90.0), fb(180.0), 0x7f800000, 0x7fc00000])


def g(ctx):
    r = begin(ctx, *ACTIVE)
    libm(ctx)
    m = newbuf(ctx, 0x30)
    return [m, ang(r), ang(r), ang(r), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32),
            r.choice([0, 1, 2, 3, 4, 5, 6, 7, r.getrandbits(32)])]


SPECS = [dict(name='Sim_10060220', orig=0x10060220, c='Sim_10060220', gen=g, void=True, trials=15000, fp=True,
              tol=1.2e-7)]
