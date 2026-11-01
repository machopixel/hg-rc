# Tiny sweep (AI): the unit-against-unit range test Sim_10066907 against 1.2's 0x100571d7, on
# spec_w16_10066907.py's generator. 1.2's only changes are the unit object's fields from +0x1c sitting 4 bytes
# earlier (+0x1c -> +0x18, +0x28 -> +0x24, +0x58 -> +0x54, +0x84 -> +0x80, +0x29c -> +0x298): the objects are
# shifted for 1.2's run and back after it. The C is unchanged (it keeps 1.0's layout until the DLL goes).
# Negative check: W16R_NEG12=1 builds the C with W16R_MUTATE.
import os
exec(open('tools/verify/spec_w16_10066907.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W16R_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW16R_MUTATE']
SHIFT = {}


def g12(ctx):
    args = g(ctx)
    SHIFT.clear()
    seen = set()
    for i in range(0x1a):
        u = ctx.m.uc.mem_read(0x109e8b40 + 4 * i, 4)
        u = struct.unpack('<I', bytes(u))[0]
        if u:
            seen.add(u)
    rec = args[0]
    seen.add(struct.unpack('<I', bytes(ctx.m.uc.mem_read(rec, 4)))[0])
    for u in seen:
        SHIFT[u] = 0x2a0
    return args


def to12_more(m):
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def from12_more(m):
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])


SPECS = [v12_spec(dict(SPECS[0], gen=g12), 0x100571d7, to12_more=to12_more, from12_more=from12_more,
                  windows={0x109e8b40: 0x1a * 4})]
