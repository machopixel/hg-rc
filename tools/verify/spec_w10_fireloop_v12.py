# Bug fix (Diego, 2026-11-01, "keep my new selection"): a unit's weapon handling (1.0 0x100d4590) against 1.2's
# 0x100ba550, on spec_w10_100d4590.py's generator and stubs (hooked at 1.2's callees too, logged under the 1.0 names).
# The two differ only by a distance calculation; 1.2 makes the slot that fires a burst shot the current weapon
# (+0x4fc). W10F_NEG12 unset: the C built with W10F_KEEP_SEL=0 (1.2's line) must match exactly. W10F_NEG12=fix: the
# game's build (the local player's gear keeps its +0x4fc): 1.2's result has the player's +0x4fc put back to its value
# before the run (the fix's only difference), then everything else - shots, counts, timing, sounds, computer gears -
# must match exactly. W10F_NEG12=1: the fix applied to every gear (W10F_MUTATE), which must mismatch (computer gears).
# The unit's object holds its fields from +0x1c 4 bytes earlier in 1.2 (+0x48 -> +0x44, controls +0x54 -> +0x50):
# shifted for 1.2's run, back after it; the weapon kind table (0x10145508 = 1.2 0x101192f0) copied across, with the
# 0xd6 bytes before it: a current slot without a weapon (+4 = -1) reads 'kind -1' there in both versions.
import os
exec(open('tools/verify/spec_w10_100d4590.py').read())
exec(open('tools/verify/v12_common.py').read())
_neg = os.environ.get('W10F_NEG12')
CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW10F_KEEP_SEL=%d' % (_neg in ('fix', '1'))] + \
    (['-DW10F_MUTATE'] if _neg == '1' else [])
SHIFT = {}
KEEP = {}
_g10 = g


def g12(ctx):
    args = _g10(ctx)
    u = args[0]
    obj = r32(ctx, u)
    SHIFT.clear()
    SHIFT[obj] = 0x100
    KEEP.clear()
    if _neg in ('fix', '1') and r32(ctx, obj + 4) == r32(ctx, 0x1017b8c8):
        KEEP[u] = r32(ctx, u + 0x4fc)
    return args


def to12_more(m):
    m.uc.mem_write(A12(0x101192f0) - 0xd6, bytes(m.uc.mem_read(0x10145508 - 0xd6, 5 * 0xd6)))
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def from12_more(m):
    m.uc.mem_write(0x10145508 - 0xd6, bytes(m.uc.mem_read(A12(0x101192f0) - 0xd6, 5 * 0xd6)))
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])
    for u, v in KEEP.items():          # the fix: the player's current weapon stays as it was
        m.uc.mem_write(u + 0x4fc, struct.pack('<I', v))


SPECS = [v12_spec(dict(SPECS[0], gen=g12), 0x100ba550, to12_more=to12_more, from12_more=from12_more,
                  extra_pairs={0x1000ebfb: 0x10008723, 0x1000eb12: 0x1000863a, 0x10024684: 0x1001b372,
                               0x1017b8c8: 0x1014a568, 0x1014cae0: 0x1011f9e8},
                  skip=(0x10145508,), name='Sim_100d4590_v12')]


def lm(log_o, log_c):
    """pointers into 1.2's weapon kind table as ours"""
    lo = A12(0x101192f0)

    def fix(a):
        if isinstance(a, int) and lo - 0xd6 <= a < lo + 4 * 0xd6:
            return a - lo + 0x10145508
        return a
    return [(e[0], tuple(fix(a) for a in e[1])) for e in log_o], log_c


SPECS[0]['log_map'] = lm
