# Round 33: a unit's per-frame movement (1.0 0x1006e22c) against 1.2's 0x1005d75c, on spec_w16_1006e22c.py's
# generator and stubs (hooked at 1.2's callees too, logged under the 1.0 names; its float stubs too, FSTUB12).
# 1.2's changes ported: the object's height is the unit record's body height +0x68 unless [0x10179ce0] is set (1.0:
# always +0x6c); the jump jets are also cut when the unit record's +0x560 bit 0x20 is set, whatever the throttle.
# And 1.2's four force-feedback calls, made only with the force-feedback library ([0x1015ef3c], 1.2 [0x10131718]):
# the request (Sim_100382fb, 1.2 0x100382fb) is stubbed on both sides, its arguments compared. The object records hold their fields from +0x1c 4 bytes earlier in 1.2: the unit's object and the
# collision answer's gear are shifted down by 4 before 1.2's run and back after it.
# Negative checks: W16_NEG12=y builds the C with W16MY_MUTATE (1.0's height), =j with W16MJ_MUTATE (1.0's jets),
# =f with W16MF_MUTATE (the bump's strength 50).
# Bug fix (Diego): 1.2's contact bump passes its angle (a double) unconverted as the request's a / b words with 100
# as c; the C passes (angle truncated to whole degrees, 100, 0). 1.2's logged call is turned into that form (lm).
import os
exec(open('tools/verify/spec_w16_1006e22c.py').read())
exec(open('tools/verify/v12_common.py').read())
_neg = os.environ.get('W16_NEG12')
if _neg:
    CFLAGS = list(globals().get('CFLAGS', [])) + [{'y': '-DW16MY_MUTATE', 'j': '-DW16MJ_MUTATE', 'f': '-DW16MF_MUTATE'}[_neg]]

F10, F12 = 0x1006e22c, 0x1005d75c
_P = v12_pairs(F10, F12)
_P.update({0x100d6e99: 0x100bcec7, 0x100d6f16: 0x100bcf44, 0x10061f8e: 0x1005372e, 0x100a9ca3: 0x10095133,
           0x1006788c: 0x1005815c})     # not aligned by the listing pairing
FSTUB12 = {a: A12(_P[a]) for a in FSTUB if a in _P}
FF12 = A12(0x100382fb)
helper(FF12, 'Sim_100382fb', 4)
SHIFT = {}
WIN = [(0x109e9bac, 0x107f254c, 4 * 0x50), (0x10a1d140, 0x10819580, 6 * 0x38),
       (0x1017e920, 0x1014d6c0, 0x10), (0x1017e87c, 0x1014d620, 0x10)]   # the C library's math state (fmod / sqrt
#                                                                             run for real) and errno


def _ground_normal12(uc, address, size, ud):
    # 1.2's ground height (0x10022591) leaves the normal in its own [0x10130818..20]
    for i, a in enumerate((0x10130818, 0x1013081c, 0x10130820)):
        uc.mem_write(A12(a), struct.pack('<I', T['normal'][i]))


def _collide12(uc, k, raw, eax):
    # the collision answer's directions, for 1.2's copies of the six globals as well
    v = _collide(uc, k, raw, eax)
    for a10, a12 in zip(range(0x1015dd4c, 0x1015dd64, 4), range(0x10130818, 0x10130830, 4)):
        uc.mem_write(A12(a12), bytes(uc.mem_read(a10, 4)))
    return v


EFFECT[0x100663c0] = _collide12


def g12(ctx):
    args = g(ctx)
    m = ctx.m
    if not getattr(m, '_w16_ground12', False):
        m._w16_ground12 = True
        m.raw.hook_add(UC_HOOK_CODE, _ground_normal12, None, A12(0x10022591), A12(0x10022591))
    ST['active'].add(FF12)
    w32(ctx, 0x1015ef3c, ctx.rng.choice([0, 1, 1]))
    SHIFT.clear()
    if args[0]:
        SHIFT[r32(ctx, args[0])] = 0x300
    for st in T['col']:
        if st['gear']:
            SHIFT[st['gear']] = 0x40
    return args


def to12_more(m):
    for a10, a12, n in WIN:
        if a12:
            m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, n)))
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def from12_more(m):
    for a10, a12, n in WIN:
        if a12:
            m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), n)))
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])


_s = v12_spec(dict(SPECS[0], gen=g12, trials=int(os.environ.get('W16_TRIALS') or 6000)), F12,
              to12_more=to12_more, from12_more=from12_more, skip=(0x109e9bac, 0x10a1d140),
              extra_pairs={0x1015ef3c: 0x10131718, 0x1015dd4c: 0x10130818, 0x1015dd50: 0x1013081c, 0x1015dd54: 0x10130820, 0x1015dd58: 0x10130824,
                           0x1015dd5c: 0x10130828, 0x1015dd60: 0x1013082c, 0x100d6e99: 0x100bcec7, 0x100d6f16: 0x100bcf44, 0x10061f8e: 0x1005372e,
                           0x100a9ca3: 0x10095133, 0x1006788c: 0x1005815c, 0x109e8230: 0x107f1d40, 0x1017b8c4: 0x1014a564, 0x1014cae0: 0x1011f9e8},
              name='Sim_1006e22c_v12')


def _ftol(d):
    if not (-9223372036854775808.0 <= d < 9223372036854775808.0):
        return 0
    return int(d) & 0xffffffff


def lm(log_o, log_c):
    """1.2's bump (0xf, double low, double high, 100) as the fixed call (0xf, (int)angle, 100, 0)"""
    out = []
    for e in log_o:
        if e[0] == 'stub_%x' % FF12 and len(e[1]) == 4 and e[1][0] == 0xf and e[1][3] == 0x64:
            d = struct.unpack('<d', struct.pack('<II', e[1][1], e[1][2]))[0]
            e = (e[0], (0xf, _ftol(d), 0x64, 0))
        out.append(e)
    return out, log_c


_s['log_map'] = lm
SPECS = [_s]
