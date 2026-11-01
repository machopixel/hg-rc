# 1.2 audit port: the fire routine (1.0 0x100d4f71) against 1.2's 0x100baf31, on spec_w10_100d4f71.py's generator and
# stubs. 1.2's change ported: for the own unit's shot, right after the shot counter [0x101653f3], the force-feedback
# request 0x100382fb(weapon +0xd2, 0, 0, 0), only while [0x10131718] (1.0 0x1015ef3c) is set. 1.2's request is
# stubbed at its own address and logged as the C's Sim_100382fb (FFC). 1.2's object records hold the fields from
# +0x1c 4 bytes earlier (the unit's record +0x28 -> +0x24): the object and the 8 player records are shifted down by 4
# before 1.2's run and back after it.
# Negative check: W10F_NEG12=1 builds the C with W10F_MUTATE (no request).
import os
exec(open('tools/verify/spec_w10_100d4f71.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W10F_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW10F_MUTATE']

FFC = 0x00F00F00
FF12 = A12(0x100382fb)
helper(FFC, 'Sim_100382fb', 4)
helper(FF12, None, 4)
FF = {'flag': 0}
SHIFT = {}


def g12(ctx):
    r = ctx.rng
    obj, weapon, slot = g(ctx)
    m = ctx.m
    ST['active'].update((FFC, FF12))
    if not getattr(m, '_fstub12', False):         # the float stub at 1.2's twin of Sim_100d6e99 as well
        m._fstub12 = True
        m.raw.hook_add(UC_HOOK_CODE, _fstub_cb(m, 0x100d6e99, 0), None, A12(0x100bcec7), A12(0x100bcec7))
    w12 = newbuf(ctx, 0xe0)                         # the weapon record with its +0xd2 (the effect kind)
    wb(ctx, w12, bytes(m.uc.mem_read(weapon, 0xd0)))
    w32(ctx, w12 + 0xd2, r.choice([0xd, 0xe, 0xf, 0x10, 0x11, r.getrandbits(32)]))
    FF['flag'] = r.choice([0, 1, 1])
    w32(ctx, 0x1015ef3c, FF['flag'])
    SHIFT.clear()
    if obj:
        SHIFT[obj] = 0x40
    for i in range(8):
        SHIFT[r32(ctx, 0x109e8b40 + 4 * i)] = 0x40
    return [obj, w12, slot]


def to12_more(m):
    m.uc.mem_write(A12(0x10131718), struct.pack('<I', FF['flag']))
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def from12_more(m):
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])


def lm(log_o, log_c):
    """1.2's request logged as the C's Sim_100382fb; pointers into 1.2's shot array as the 1.0 array's"""
    lo, n = A12(0x107b7ce0) - 0x100, 20 * 0x156 + 0x200
    out = []
    for e in log_o:
        if not isinstance(e[1], tuple) or (e[1] and e[1][0] == 'deref'):
            out.append(e)
            continue
        e = (e[0], tuple(v - A12(0x107b7ce0) + 0x109a3f80 if isinstance(v, int) and lo <= v < lo + n else v
                         for v in e[1]))
        out.append(('stub_%x' % FFC, e[1]) if e[0] == 'stub_%x' % FF12 else e)
    return out, log_c


_s = v12_spec(dict(SPECS[0], gen=g12, trials=4000), 0x100baf31, to12_more=to12_more, from12_more=from12_more,
              windows={0x109e8b40: 0x20, 0x109a3f80: 20 * 0x156},
              extra_pairs={0x1015ec58: 0x101314a0, 0x1017b8c4: 0x1014a564, 0x1016540c: 0x1028cb0c,
                           0x109a3f80: 0x107b7ce0,
                           # the listing alignment is thrown by 1.2's inserted request: the callees by hand
                           0x100d6d6a: 0x100bcd98, 0x100d6f16: 0x100bcf44, 0x100d6e99: 0x100bcec7,
                           0x100a9cfb: 0x1009518b, 0x1009201f: 0x1007acf3, 0x10091085: 0x1007a0a6,
                           0x10092956: 0x1007b956})
_s['log_map'] = lm
SPECS = [_s]
