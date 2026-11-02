# Round 27: joystick support removed. 0x1005621b (joystick name) and 0x100567cb (joystick open) against the original's
# "no joystick" path: Windows answers joyGetDevCapsA with an error (MMSYSERR_NODRIVER 6, JOYERR_PARMS 0xa5,
# JOYERR_UNPLUGGED 0xa7), or the open has no state block (+0x70 = 0). The C makes no Windows call at all, so the
# joystick calls are taken out of the original's call log (the one deliberate difference); everything else (result,
# memory) must be exact. Also the count 0x10056210 (Win_JoyGetNumDevs), now 0. W27_NEG=1 (W27J_MUTATE: the name
# returns 0, the count 1) must be caught.
exec(open('tools/verify/w10_common.py').read())

if os.environ.get('W27_NEG'):
    CFLAGS.append('-DW27J_MUTATE')

SOURCES = closure(['sim/win/w4w_win.c'])
ACTIVE = ()
M = {}


def _nojoy(n, args, uc):
    # the joystick call is the deliberate difference: not logged
    M['m'].api_log.pop()
    return M['err']


API = {'joyGetDevCapsA': _nojoy, 'joyGetPosEx': _nojoy}


def _noslots(n, args, uc):
    # the original asks Windows for the number of joystick slots; the C answers 0 itself: not logged, and Windows
    # answers 0 here (other answers only change how many slots Input_AddDevice tries, see DECISIONS.md round 27)
    M['m'].api_log.pop()
    return 0


API_COUNT = {'joyGetNumDevs': _noslots}


def _world(ctx, r):
    M['m'] = ctx.m
    M['err'] = r.choice([6, 0xa5, 0xa7])
    dev = newbuf(ctx, 0x80)
    wb(ctx, dev, rb(ctx, 0x80))
    return dev


def g_open(ctx):
    r = begin(ctx, *ACTIVE)
    dev = _world(ctx, r)
    return [r.choice([0, 1, 2, 15, r.getrandbits(32)]), dev]


def g_count(ctx):
    r = begin(ctx, *ACTIVE)
    _world(ctx, r)
    return []


def g_poll(ctx):
    r = begin(ctx, *ACTIVE)
    dev = _world(ctx, r)
    blk = newbuf(ctx, 0x88)
    w32(ctx, blk, r.choice([0, 1, 15]))
    w32(ctx, dev + 0x70, r.choice([0, blk, blk]))
    return [dev]


SPECS = [dict(name='Win_JoyGetNumDevs', orig=0x10056210, c='Win_JoyGetNumDevs', gen=g_count, api=API_COUNT,
              trials=300),
    # dict(name='Sim_1005621b', orig=0x1005621b, c='Sim_1005621b', gen=g_open, api=API, trials=3000),   # deleted: no joystick (DECISIONS.md "dropped features")
    # dict(name='Sim_100567cb', orig=0x100567cb, c='Sim_100567cb', gen=g_poll, api=API, trials=3000),   # deleted: no joystick (DECISIONS.md "dropped features")
]
