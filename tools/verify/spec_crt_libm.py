# C runtime replacement (DECISIONS.md, "C runtime replacement"): OpenLibm (src/sim/libm, hg_*) against the original
# C runtime's x87 routines it replaces, called through st(0) trampolines (the result stored as a double at OUTF and
# compared with the rest of memory). Inputs as the game passes them: distances (sqrt), angles against 360 (fmod),
# any double (fabs), the gamma table's x in 0..1 and exponents (pow), sines in -1..1 (asin), angles in radians
# (sin / cos / acos), with a few special values (zeros, negatives, infinities, NaN). A mismatch is a result that
# differs; the per-function counts are the "may differ" list. errno writes of the original (EDOM / ERANGE at
# 0x1017e888) count as mismatches too.
exec(open('tools/verify/w10_common.py').read())
import math
LIBM_ORIGINAL = True        # run the original routines here (difftest answers them with OpenLibm everywhere else)

SOURCES = closure(['sim/libm/olm_e_sqrt.c', 'sim/libm/olm_e_fmod.c', 'sim/libm/olm_s_fabs.c', 'sim/libm/olm_e_pow.c',
                   'sim/libm/olm_e_asin.c', 'sim/libm/olm_e_acos.c', 'sim/libm/olm_s_sin.c', 'sim/libm/olm_s_cos.c',
                   'sim/libm/olm_s_atan.c'])
TR = 0x00F00A00            # trampoline pairs, 0x80 apart: original at +0, C at +0x40
FUNCS = [('sqrt', 0x100fca04, 'hg_sqrt', 1), ('fmod', 0x100fcb30, 'hg_fmod', 2), ('fabs', 0x100fcb70, 'hg_fabs', 1),
         ('pow', 0x100fed29, 'hg_pow', 2), ('asin', 0x100fcc74, 'hg_asin', 1), ('sin', 0x100ff644, 'hg_sin', 1),
         ('atan', 0x10100044, 'hg_atan', 1)]
# cos / acos have no DLL routine the C called through an address (round 10's interpreted functions run fcos and the
# acos sequence inline): their OpenLibm use is checked through those functions' own specs.


def tramp(target, ndw):
    code = b'\x55\x89\xe5'
    for k in reversed(range(ndw)):
        code += b'\xff\x75' + bytes([8 + 4 * k])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * ndw])
    code += b'\xdd\x1d' + struct.pack('<I', OUTF)              # fstp qword [OUTF]
    code += b'\x31\xc0\x5d\xc3'
    return code


def setup(ctx):
    m = ctx.m
    if getattr(m, '_libm_tr', False):
        return
    m._libm_tr = True
    for i, (n, orig, c, nd) in enumerate(FUNCS):
        m.raw.mem_write(TR + 0x80 * i, tramp(orig, 2 * nd))
        m.raw.mem_write(TR + 0x80 * i + 0x40, tramp(m.syms[c], 2 * nd))
        m.syms['Tr_' + c] = TR + 0x80 * i + 0x40


def dw(x):
    a, b = struct.unpack('<2I', struct.pack('<d', x))
    return [a, b]


def special(r):
    return r.choice([0.0, -0.0, float('inf'), float('-inf'), float('nan'), 1e-310, -1.0, 5e-324])


def gen_for(kind):
    def g(ctx):
        r = begin(ctx)
        setup(ctx)
        w32(ctx, OUTF, 0, 0)
        w32(ctx, 0x1017e888, 0)
        sp = r.random() < 0.03
        if kind == 'sqrt':
            x = special(r) if sp else r.choice([r.uniform(0, 1e6), r.uniform(0, 4), r.uniform(0, 1e12)])
            return dw(x)
        if kind == 'fmod':
            # angles, as the game passes them (the emulator's fprem is inexact for large quotients: real x87 fprem
            # and OpenLibm are both exact there, checked against Python's math.fmod)
            x = special(r) if sp else r.choice([r.uniform(-4000, 4000), r.uniform(-360, 720), r.uniform(-1e5, 1e5)])
            y = r.choice([360.0, 360.0, 360.0, r.uniform(-500, 500)]) if r.random() > 0.02 else special(r)
            return dw(x) + dw(y)
        if kind == 'fabs':
            x = special(r) if sp else r.uniform(-1e5, 1e5)
            return dw(x)
        if kind == 'pow':
            x = r.choice([r.randrange(0, 0x40) / 63.0, r.uniform(0, 1), r.uniform(0, 4)]) if not sp else special(r)
            e = r.choice([r.uniform(0.2, 5.0), 1.0 / r.uniform(0.2, 5.0), r.uniform(-3, 3), 2.0])
            return dw(x) + dw(e)
        if kind == 'asin':
            x = special(r) if sp else r.choice([r.uniform(-1, 1), r.uniform(-1, 1) ** 3, r.choice([1.0, -1.0, 0.5])])
            return dw(x)
        if kind == 'sin':
            x = special(r) if sp else r.choice([r.uniform(-7, 7), r.uniform(-1, 1), r.uniform(-100, 100)])
            return dw(x)
        if kind == 'atan':
            x = special(r) if sp else r.choice([r.uniform(-2, 2), r.uniform(-100, 100), r.uniform(-1e-3, 1e-3)])
            return dw(x)
    return g


# 0x1017e900..0x1017e97f: the original C runtime's own maths scratch (saved operands and control words for its
# error records), which nothing outside it reads: not compared. errno (0x1017e888) is.
SPECS = [dict(name='libm_' + n, orig=TR + 0x80 * i, c='Tr_' + c, gen=gen_for(n), trials=20000,
              ignore_addrs={0x1017e900: 0x80})
         for i, (n, orig, c, nd) in enumerate(FUNCS)]
