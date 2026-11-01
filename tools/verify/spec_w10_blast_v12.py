# Round 33, round 4: the effect spawn / blast impact (1.0 0x100246b3) against 1.2's 0x1001b3a1, on
# spec_w10_100246b3.py's generator and stubs (hooked at 1.2's callees too, logged under the 1.0 names; its two float
# stubs at their 1.2 twins too). 1.2's change: the near-blast force feedback moved from before the switch into each
# blast kind's 0x400 branch, the impact within 2000 of the own unit's object (+0x58 / +0x5c / +0x60) -> request 0xd
# (Sim_100382fb, 1.2 0x100382fb, stubbed on both sides). The C is built with W10E_FIX_DIR=0 here (1.2's fixed angle
# 90); the game's build sends the computed direction (Diego's fix, DECISIONS.md).
# The player record (+0x28 unit -> +0x24) and the unit's object (+0x58 -> +0x54) hold their fields from +0x1c 4 bytes
# earlier in 1.2: shifted down by 4 for 1.2's run, back after it. The effect table and the effect slots are copied
# to 1.2's addresses and back.
# Negative checks: W10E_NEG12=1 builds the C with W10E_MUTATE (1.0's 3000); =d with the direction fix on (the game's
# build), which differs from 1.2 wherever the request is sent.
import os
exec(open('tools/verify/spec_w10_100246b3.py').read())
exec(open('tools/verify/v12_common.py').read())
_neg = os.environ.get('W10E_NEG12')
CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW10E_FIX_DIR=%d' % (_neg == 'd')] + \
    (['-DW10E_MUTATE'] if _neg == '1' else [])

F10, F12 = 0x100246b3, 0x1001b3a1
_P = v12_pairs(F10, F12)
FF12 = A12(0x100382fb)
helper(FF12, 'Sim_100382fb', 4)
ACTIVE = ACTIVE + (FF12,)
FSTUB12 = {a: A12(_P[a]) for a in FSTUB}
_fstubs10 = fstubs


def fstubs(ctx):
    m = ctx.m
    first = not getattr(m, '_fstub', False)
    _fstubs10(ctx)
    if first:
        for slot, (addr, _) in enumerate(sorted(FSTUB.items())):
            cb = _fstub_cb(m, addr, slot)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, FSTUB12[addr], FSTUB12[addr])


WIN = [(0x10147b80, 0x1011b968, 0x4d * 0x3c), (0x109a5a40, 0x107b97a0, 0x100 * 0x2c), (0x109f14e0, 0x107f9760, 0x18)]
SHIFT = {}


def g12(ctx):
    args = g(ctx)
    r = ctx.rng
    # distances around the new 2000 limit too (and 1.0's 3000), 2000 itself included
    ST['fvals'][0x10084a94] = [fb(r.choice([r.uniform(0, 400), 2000.0, r.uniform(1900, 3100)])) for _ in range(4)]
    SHIFT.clear()
    pe = r32(ctx, 0x109e8b40 + 4 * r32(ctx, 0x1017b8c8))
    SHIFT[pe] = 0x40
    SHIFT[r32(ctx, r32(ctx, pe + 0x28))] = 0x80
    return args


def to12_more(m):
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(0x109e8b40, 0x20)))
    for a10, a12, n in WIN:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, n)))
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def from12_more(m):
    for a10, a12, n in WIN:
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), n)))
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])


SPECS = [v12_spec(dict(SPECS[0], gen=g12), F12, to12_more=to12_more, from12_more=from12_more,
                  skip=(0x109e8b40, 0x10147b80, 0x109a5a40, 0x109f14e0), name='Sim_100246b3_v12')]


def lm(log_o, log_c):
    """pointers into 1.2's effect table (the sound record, +0x2c) as ours"""
    lo = A12(0x1011b968)

    def fix(a):
        if isinstance(a, int) and lo <= a < lo + 0x4d * 0x3c:
            return a - lo + 0x10147b80
        return a
    return [(e[0], tuple(fix(a) for a in e[1])) for e in log_o], log_c


SPECS[0]['log_map'] = lm
