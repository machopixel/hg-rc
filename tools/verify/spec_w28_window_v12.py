# Focus / cache round (1.2): the window procedure Sim_100867a2 against 1.2's 0x10071397, on spec_w28_window.py's
# generator and platform stubs. 1.2's change (message 0x1c): on getting the focus back the game resumes
# (0x10075070 = 1.0 Sim_1008b900) before the key state is cleared; on losing it the game pauses (0x1007504e = 1.0
# Sim_1008b8de) before the sound pointer. Both are logging stubs here.
# Our fix (DECISIONS.md, "Focus pause"): no pause while the clock is already held (timer flag 0x200 in
# [0x1015317c]) or in a network game ([0x1015ecd0]); the resume only after a pause of its own (g_w28FocusPaused).
# 1.2's log is brought to the fix's form by log_map from the trial's state (chosen by the generator): its pause is
# dropped when the fix skips it, its resume when the flag was clear. The flag the C leaves is checked by post_c
# against what the fix must leave (the C global itself is not memory the original has: ignore_syms).
# Negative check: W28_NEG12=1 builds the C with W28F_MUTATE (resume without the flag test).
import os
exec(open('tools/verify/spec_w28_window.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W28_NEG12'):
    CFLAGS.append('-DW28F_MUTATE')
SOURCES = closure(['sim/platform/w28_window.c'])     # closure adds plat_stub.c itself (round 32)

helper(0x1008b8de, 'Sim_1008b8de', 0)
helper(0x1008b900, 'Sim_1008b900', 0)
ACTIVE = tuple(HELPERS)      # g_proc's begin() reads it: the two new stubs are switched on too
PAUSE, RESUME = 'stub_%x' % 0x1008b8de, 'stub_%x' % 0x1008b900
T = {}


def g12(ctx, _g=g_proc):
    args = _g(ctx)
    r = ctx.rng
    held = r.choice([0, 0, 1])
    net = r.choice([0, 0, 0, 1, 2])
    flag = r.choice([0, 1])
    w32(ctx, 0x1015317c, r.getrandbits(32) & ~0x200 | (0x200 if held else 0))
    w32(ctx, 0x1015ecd0, net)
    w32(ctx, ctx.m.syms['g_w28FocusPaused'], flag)
    T.update(held=held, net=net, flag=flag, msg=args[1], wp=args[2], quit=r32(ctx, 0x1017b8b8))
    return args


def lm(log_o, log_c):
    """1.2's pause / resume as the fix makes them; the palette handed to the driver is 1.2's own copy (logged as
    1.0's address)"""
    out = []
    for e in log_o:
        if e[0] == PAUSE and (T['held'] or T['net']):
            continue
        if e[0] == RESUME and not T['flag']:
            continue
        if e[0] == 'stub_%x' % FN['palette'] and e[1][2] == A12(0x10130fe0):
            e = (e[0], e[1][:2] + (0x1015e6a0,) + e[1][3:])
        out.append(e)
    return out, log_c


def post_c(m):
    want = T['flag']
    if T['msg'] == 0x1c and T['quit'] == 0:
        if T['wp'] == 1:
            want = 0
        elif not (T['held'] or T['net']):
            want = 1
    got = struct.unpack('<I', bytes(m.uc.mem_read(m.syms['g_w28FocusPaused'], 4)))[0]
    if got != want:     # shows as a memory difference at a spare argument word
        m.uc.mem_write(T['spare'], struct.pack('<I', 0xBAD0F1A6))


def g12s(ctx):
    args = g12(ctx)
    T['spare'] = newbuf(ctx, 4)
    w32(ctx, T['spare'], 0)
    return args


SPECS = [v12_spec(dict(SPECS[0], gen=g12s, trials=20000), 0x10071397,
                  extra_pairs={0x1008b8de: 0x1007504e, 0x1008b900: 0x10075070,
                               0x10057f53: 0x1004ae03})]    # the aligner pairs the key clear with the new resume
SPECS[0]['log_map'] = lm
SPECS[0]['post_c'] = post_c
SPECS[0]['ignore_syms'] = {'g_w28FocusPaused': 4}
