# Round 33: the missile launch (1.0 0x100d5446) against 1.2's 0x100bb425, on spec_w10_100d5446.py's generator and
# stubs. 1.2's change ported here: after the shot's sound slot, the launch smoke (1.2's gun effect 0x1007f19f, stubbed
# on both sides: its arguments are compared) at the weapon's mount node (unit record +0x498 + 4 * slot +0x11c),
# moving (0, 5, 0). The C runs with the 1.2 particle switch on.
# Controls round: 1.2's force-feedback request 0x100382fb(kind, 0, 0, 0), only while [0x10131718] (1.0 0x1015ef3c)
# is set, ported; 1.2's call (hooked as 1.0's Sim_10045f3b, its aligned twin) is logged as the C's Sim_100382fb (FFC).
# 1.2's object records hold the fields from +0x1c 4 bytes earlier (unit +0x24, control +0x50, +0x1aa -> +0x1a6): the
# object, its owner and the 8 player records are shifted down by 4 before 1.2's run and back after it.
# Negative check: W10_NEG12=1 builds the C with W10G_MUTATE (the smoke moving sideways).
import os
exec(open('tools/verify/spec_w10_100d5446.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W10_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW10G_MUTATE']

GUN12 = A12(0x1007f19f)
helper(GUN12, 'Sim12_1007f19f', 6)
SHIFT = {}


def g12(ctx):
    obj, weapon, slot = g(ctx)
    m = ctx.m
    FF['flag'] = ctx.rng.choice([0, 1, 1])      # 1.2 skips the request while clear, as the C does
    w32(ctx, 0x1015ef3c, FF['flag'])
    ST['active'].add(GUN12)
    if not getattr(m, '_fstub12', False):         # the float stub at 1.2's twin of Sim_100c97c0 as well
        m._fstub12 = True
        m.raw.hook_add(UC_HOOK_CODE, _fstub_cb(m, 0x100c97c0, 0), None, A12(0x100af2a0), A12(0x100af2a0))
    w32(ctx, m.syms['g_w31Tex'], 1)
    w32(ctx, 0x1017b940, 1)
    SHIFT.clear()
    if obj:
        SHIFT[obj] = 0x200
        SHIFT[r32(ctx, r32(ctx, obj + 0x28))] = 0x60
    for i in range(8):
        SHIFT[r32(ctx, 0x109e8b40 + 4 * i)] = 0x40
    return [obj, weapon, slot]


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
    """1.2's force-feedback request logged as the C's Sim_100382fb(kind, 0, 0, 0); pointers into 1.2's missile array
    as the 1.0 array's"""
    lo, n = A12(0x1078d0e0), 0x200 * 0x156
    out = []
    for e in log_o:
        e = (e[0], tuple(v - lo + 0x1096fc80 if isinstance(v, int) and lo <= v < lo + n else v for v in e[1]))
        if e[0] == 'stub_10045f3b' and len(e[1]) == 3 and e[1][1:] == (0, 0):
            e = ('stub_%x' % FFC, (e[1][0], 0, 0, 0))
        out.append(e)
    return out, log_c


_s = v12_spec(dict(SPECS[0], gen=g12, trials=4000), 0x100bb425, to12_more=to12_more, from12_more=from12_more,
              windows={0x109e8b40: 0x20, 0x1096fc80: 0x200 * 0x156}, extra_pairs={0x1015dc80: 0x10130770})
_s['log_map'] = lm
SPECS = [_s]
