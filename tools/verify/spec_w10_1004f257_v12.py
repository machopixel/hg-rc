# Round 33: the unit animation (1.0 0x1004f257) against 1.2's 0x1003fbb7, on spec_w10_1004f257.py's generator and
# stubs. 1.2's one change: a moving unit whose step bit (+0x88 bit 8) is set raises footstep dust at its lower foot
# (the nodes +0x4c4 / +0x4c0 of its unit record) against the record's velocity +0x548 (1.2's 0x10086007, stubbed on
# both sides: its arguments are compared), for every unit; the step sound stays the player's. The C runs with the
# 1.2 particle switch on.
# 1.2's object record holds the fields from +0x1c 4 bytes earlier (unit +0x24, control +0x50, animation +0x88..+0x90,
# flags +0x84); so does the owner record (+0x158). Before 1.2's run both buffers are shifted down by 4, after it back.
# Negative check: W10_NEG12=1 builds the C with W10FS_MUTATE (the higher foot).
import os
exec(open('tools/verify/spec_w10_1004f257.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W10_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW10FS_MUTATE']

DUST12 = A12(0x10086007)
helper(DUST12, 'Sim12_10086007', 6)
SHIFT = {}                          # buffer -> (length, its bytes before 1.2's run)


def g12(ctx):
    obj, = g(ctx)
    m, r = ctx.m, ctx.rng
    ST['active'].add(DUST12)
    w32(ctx, m.syms['g_w31Tex'], 1)
    w32(ctx, 0x1017b940, 1)
    rec = r32(ctx, obj + 0x28)
    for off in (0x4c0, 0x4c4):
        node = newbuf(ctx, 0x80)
        w32(ctx, node + 0x70, *[fb(r.choice([r.uniform(-50, 50), 7.0])) for _ in range(3)])
        w32(ctx, rec + off, node)
    w32(ctx, rec + 0x548, *[fb(r.uniform(-20, 20)) for _ in range(3)])
    w32(ctx, obj + 0x88, r.choice([8, 8, 0, 9]))
    SHIFT.clear()
    SHIFT[obj] = 0xa0
    SHIFT[r32(ctx, rec)] = 0x160
    return [obj]


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


PAIRS12 = {0x1014cae0: 0x1011f9e8, 0x1017b8c8: 0x1014a568, 0x1015ef0c: 0x101316f4, 0x1015f3a8: 0x10131ac4}
_s = v12_spec(dict(SPECS[0], gen=g12, trials=6000), 0x1003fbb7, extra_pairs=PAIRS12, to12_more=to12_more,
              from12_more=from12_more)


def lm(log_o, log_c):
    """1.2's copy of the sound name "AFXHGWKL" (0x10110e08) logged as 1.0's (0x1013bab8)"""
    out = []
    for e in log_o:
        if e[0] == 'stub_1000ed52' and len(e[1]) > 3 and e[1][3] == A12(0x10110e08):
            e = (e[0], e[1][:3] + (0x1013bab8,) + e[1][4:])
        out.append(e)
    return out, log_c


_s['log_map'] = lm
SPECS = [_s]
