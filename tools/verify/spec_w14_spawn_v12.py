# Round 33, round 3: the respawn (1.0 0x1009d200) against 1.2's 0x1008a560, on spec_w14_1009d200.py's generator and
# stubs (hooked at 1.2's callees too, logged under the 1.0 names). 1.2's changes: no player record -> 0; the gear's
# +0x150 = 0x80000000; 1.0's network call Sim_1007caab dropped; the start-point loop asks the collision probe
# Sim_1002dc60 (stubbed: its answer chosen, its arguments compared) instead of comparing every gear's position; a
# gear of a played side has its actions (Sim_10011093) and group selection (Sim_10016632) redone; a computer gear
# whose group leader is a remote player's goes to that group (Sim_100526e3, Sim_100128dd, a message for the local
# player). The C is built with W14S_FIX_HEIGHT / W14S_FIX_RETRY = 0 here (1.2 exactly); the fixes have their own
# check (W14_FIX=1: the probe at the height, retried while occupied, compared against 1.2's run with the probe's
# second argument and the loop logged in the fixed form is not possible, so the fixes are checked by
# spec_w14_spawn_fix.py on the C alone).
# The player records and the gear's object hold their fields from +0x1c 4 bytes earlier in 1.2: shifted down by 4
# for 1.2's run, back after it.
# Negative check: W14_NEG12=1 builds the C with W14S_MUTATE (no +0x150).
import os
exec(open('tools/verify/spec_w14_1009d200.py').read())
exec(open('tools/verify/v12_common.py').read())
_neg = os.environ.get('W14_NEG12')
# compared with 1.2 exactly: both fixes off (the game's build has them on, DECISIONS.md)
_fix = {'1': ('0', '0', ['-DW14S_MUTATE']), 'r': ('0', '1', []), 'h': ('1', '0', [])}.get(_neg, ('0', '0', []))
CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW14S_FIX_HEIGHT=' + _fix[0], '-DW14S_FIX_RETRY=' + _fix[1]] + _fix[2]

F10, F12 = 0x1009d200, 0x1008a560
MORE = [(0x1002dc60, 'Sim_1002dc60', 4), (0x10011093, 'Sim_10011093', 1), (0x10016632, 'Sim_10016632', 1),
        (0x10052aaf, 'Sim_10052aaf', 1), (0x100526e3, 'Sim_100526e3', 2), (0x100128dd, 'Sim_100128dd', 3),
        (0x1007df6c, 'Sim_1007df6c', 4)]
for a, c, n in MORE:
    helper(a, c, n)
helper(0x1007caab, None, 1)          # 1.0's network call, which the C (as 1.2) no longer makes: not linked
SARGS[0x1007df6c] = {0, 1}
ACTIVE = ACTIVE + tuple(a for a, _, _ in MORE)
FIX = {0x1002dc60: 0x10022890, 0x10011093: 0x1000a223, 0x10016632: 0x1000f7d8, 0x10052aaf: 0x100429cf,
       0x100526e3: 0x10042603, 0x100128dd: 0x1000ba6d, 0x1007df6c: 0x1006a0ec, 0x100500ab: 0x10040ac4,
       0x1006da46: 0x1005cf76, 0x1002c113: 0x10021433, 0x1009d90e: 0x1008ad13, 0x1005cd4a: 0x1004f86d,
       0x101c4554: 0x10186ee8, 0x109b98c0: 0x107ca7e0,
       0x100b35b0: 0x1009cb60, 0x1009c929: 0x10089e69, 0x100a9d47: 0x100951d7, 0x100aa0da: 0x1009556a,
       0x1007caab: 0x10000000,
       0x1017b8c8: 0x1014a568, 0x1015dc64: 0x10130754, 0x1014c99c: 0x1011f8b0, 0x1015ec58: 0x101314a0}      # 1.2 has no such call (the listing pairing put it on 0x10040ac4): a dead hook
FSTUB12 = {0x1002d961: A12(0x10022591)}     # the ground height, at 1.2's twin too
SHIFT = {}


def g12(ctx):
    args = g(ctx)
    r = ctx.rng
    rets(0x1002dc60, *[r.choice([0, 0, 1]) for _ in range(3)])
    rets(0x10052aaf, r.choice([-1 & 0xffffffff, 0, 1, 2, 3]))
    w32(ctx, 0x101c4554, ctx.buf(16, b'MSG389\0'))
    w32(ctx, 0x109b98c0, ctx.buf(16, b'LBL\0'))
    SHIFT.clear()
    for gi in range(0x20):
        e = r32(ctx, 0x109e8b40 + 4 * gi)
        if e and e not in SHIFT:
            w32(ctx, e + 0x14, r.choice([0, 1, 2, 3]))
            w32(ctx, e + 8, r.randrange(0, 4))
            w32(ctx, e + 4, r.randrange(0, 6))
            SHIFT[e] = 0x80
            rec = r32(ctx, e + 0x28)
            if rec:
                unit = r32(ctx, rec)
                w32(ctx, unit + 0x14, r.choice([0, 1, 2, 3]))
                SHIFT[unit] = 0x300
    return args


WIN = [(0x109b8420, 0x107c95c0, 4 * 0x74), (0x109b7d7c, 0x107c8f1c, 4 * 0x6c),   # start zones, side matrices
       (0x1017cf78, 0x1014b8b0, 0x20 * 4), (0x1015d1b0, 0x107cb4d0, 0x10)]          # model table, hurt bytes


def to12_more(m):
    m.uc.mem_write(A12(0x107f24bc), bytes(m.uc.mem_read(0x109e8b3c, 0x84)))
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


_s = v12_spec(dict(SPECS[0], gen=g12, trials=6000), F12, to12_more=to12_more, from12_more=from12_more,
              extra_pairs=FIX, skip=(0x109e8b40,), name='Sim_1009d200_v12')


def lm(log_o, log_c):
    """pointers into a shifted record's fields (from +0x18 up) in 1.2's log, as ours (4 higher)"""
    spans = [(b + 0x18, b + (v[0] if isinstance(v, tuple) else v) - 4) for b, v in SHIFT.items()]

    def fix(a):
        if isinstance(a, int):
            for lo, hi in spans:
                if lo <= a < hi:
                    return a + 4
        return a
    return [(e[0], tuple(fix(a) for a in e[1])) for e in log_o], log_c


_s['log_map'] = lm
SPECS = [_s]
