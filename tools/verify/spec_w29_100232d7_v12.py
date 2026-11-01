# Controls round: the kind-0 shot (1.0 0x100232d7) against 1.2's 0x10019f1d, on spec_w29_100232d7.py's generator and
# stubs. 1.2's one logic change: when the own unit is hit and the force-feedback flag ([0x10131718], 1.0 0x1015ef3c)
# is set, a jolt 0x100382fb(0xf, the fmod'ed heading, 0x32, 0); stubbed as the C's Sim_100382fb (FFC) on both sides.
# The float stubs are hooked at their 1.2 twins too (FSTUB12_). 1.2's object records hold the fields from +0x1c 4 bytes
# earlier: the 8 objects and the first 0x60 bytes of their unit records are shifted for 1.2's run and back after it.
# The never-written segment the C fixes is seeded at 1.2's ebp-0xe0 as well.
# Negative check: W29_NEG12=1 hands 1.2 the flag set always (the C's clear trials then differ).
import os
exec(open('tools/verify/spec_w29_100232d7.py').read())
exec(open('tools/verify/v12_common.py').read())

_P12 = v12_pairs(0x100232d7, 0x10019f1d)
FSTUB12_ = {a: A12(_P12[a]) for a in (0x10084a94, 0x1008486e, 0x1002d961, 0x100fcb30)}
SHIFT = {}


def g12(ctx):
    args = g(ctx)
    m = ctx.m
    if not getattr(m, '_ff12', False):
        m._ff12 = True
        m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, FFC), None, A12(0x100382fb), A12(0x100382fb))
        for slot, (addr, _) in enumerate(sorted(FSTUB.items())):    # this spec's own float stubs, slots as fstubs
            m.raw.hook_add(UC_HOOK_CODE, _fstub_cb(m, addr, slot), None, FSTUB12_[addr], FSTUB12_[addr])
    wb(ctx, EBP - 0xe0, ST['segb'][0])           # 1.2's never-written segment sits 4 bytes lower (1.0: ebp-0xdc)
    FF['flag'] = ctx.rng.choice([0, 1, 1])
    w32(ctx, 0x1015ef3c, FF['flag'])
    SHIFT.clear()
    for i in range(8):
        u = r32(ctx, 0x109e8b40 + 4 * i)
        SHIFT[u] = 0x160
        SHIFT[r32(ctx, u + 0x28)] = 0x60
    return args


def to12_more(m):
    flag = 1 if os.environ.get('W29_NEG12') else FF['flag']
    m.uc.mem_write(A12(0x10131718), struct.pack('<I', flag))
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
    """pointers into 1.2's shot array as the 1.0 array's (a word the C logged as it is stays: random float bits)"""
    lo, n = A12(0x1078d0e0), 0x200 * SHOT

    def fix(e, ec):
        if not isinstance(e[1], tuple):
            return e
        same = ec[1] if ec and isinstance(ec[1], tuple) and len(ec[1]) == len(e[1]) else (None,) * len(e[1])
        return (e[0], tuple(v - lo + SHOTS if isinstance(v, int) and lo <= v < lo + n and v != c else v
                            for v, c in zip(e[1], same)))
    return [fix(e, log_c[k] if k < len(log_c) else None) for k, e in enumerate(log_o)], log_c


_s = v12_spec(dict(SPECS[0], gen=g12, trials=6000), 0x10019f1d, to12_more=to12_more, from12_more=from12_more,
              windows={0x109e8b40: 0x20, SHOTS: 0x200 * SHOT, 0x1014558e: 4 * 0xd6})
_s['log_map'] = lm
SPECS = [_s]
