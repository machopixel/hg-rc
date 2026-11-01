# Round 33: a gear record's per-frame update (1.0 0x10070eb5) against 1.2's 0x10060491, on spec_w14_10070eb5.py's
# generator and stubs (hooked at 1.2's callees too, logged under the 1.0 names; its float stubs too, FSTUB12).
# 1.2's changes: the player's force-feedback rumble (kind 0x10, strength pitch * 1.25 + 30 with the jets, + 15
# without) after the speed, and the start-up sound for the player's own unit only (1.0: unless Sim_10052be7 answers
# 1), with the force-feedback kind 0xe. The request (Sim_100382fb, 1.2 0x100382fb) is stubbed on both sides, its
# arguments compared; the library flag [0x1015ef3c] (1.2 [0x10131718]) varies. The object holds its fields from
# +0x1c 4 bytes earlier in 1.2: shifted down by 4 for 1.2's run, back after it.
# Negative checks: W14_NEG12=f builds the C with W14UF_MUTATE (one rumble strength), =s with W14US_MUTATE (1.0's
# sound test).
import os
exec(open('tools/verify/spec_w14_10070eb5.py').read())
exec(open('tools/verify/v12_common.py').read())
_neg = os.environ.get('W14_NEG12')
if _neg:
    CFLAGS = list(globals().get('CFLAGS', [])) + [{'f': '-DW14UF_MUTATE', 's': '-DW14US_MUTATE'}[_neg]]

F10, F12 = 0x10070eb5, 0x10060491
_P = v12_pairs(F10, F12)
# callees the listing pairing misses or misaligns (checked against the survey's pairs)
FIX = {0x1007df6c: 0x1006a0ec, 0x1004f158: 0x1003fab8, 0x100d4500: 0x100ba4c0, 0x10052be7: 0x10042b07,
       0x10061c32: 0x100533d2, 0x100fc780: 0x100da920, 0x1015fd54: 0x101334f4, 0x10179d00: 0x10148e48,
       0x1014cae0: 0x1011f9e8, 0x1017b8c4: 0x1014a564, 0x101c4648: 0x10186fdc, 0x109b98c0: 0x107ca7e0,
       0x1007cbb4: 0x10069a95}
_P.update(FIX)
FSTUB12 = {a: A12(_P[a]) for a in FSTUB if a in _P}
SARGS[0x1000ebfb] = {0}     # the sound names by content (1.2 has its own copies)
SARGS[0x1000ec60] = {0}
SARGS[0x1000ecb0] = {0}
FF12 = A12(0x100382fb)
helper(FF12, 'Sim_100382fb', 4)
SHIFT = {}
WIN = [(0x1017e920, 0x1014d6c0, 0x10), (0x1017e87c, 0x1014d620, 0x10)]    # the C library's math state and errno


def g12(ctx):
    args = g(ctx)
    ST['active'].add(FF12)
    w32(ctx, 0x1015ef3c, ctx.rng.choice([0, 1, 1]))
    SHIFT.clear()
    SHIFT[r32(ctx, args[0])] = 0x100
    return args


def to12_more(m):
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


_s = v12_spec(dict(SPECS[0], gen=g12, trials=int(os.environ.get('W14_TRIALS') or 6000)), F12,
              to12_more=to12_more, from12_more=from12_more, 
              extra_pairs=dict(FIX, **{}) | {0x1015ef3c: 0x10131718},
              name='Sim_10070eb5_v12')
SPECS = [_s]
