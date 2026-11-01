# Round 33, the music from the MUSIC folder (platform/music.h; Diego, 2026-10-31): the converted music functions,
# built with HG_MUSIC_FILES as in the game, against 1.2's WAV-map versions (1.2 with a wavmap.ini).
#   Sim_1001f26e start     = 1.2 0x10015e83     Sim_1001f3ac restart poll = 1.2 0x10015fbd
#   Sim_1001f335 pause     = 1.2 0x10015f1b     Win_AuxSetVolume volume   = 1.2 0x1001603f
#   Sim_1001f367 resume    = 1.2 0x10015f59     Sim_1001ee7c option 2     = 1.2 0x1001550d
#   Sim_1001f399 stop      = 1.2 0x10015f97     Sim_1001edd0 option read  = 1.2 0x10015450
# Both sides log the same music events: the C side's platform/music.h calls (tools/verify/music_stub.c, hooked),
# 1.2's calls to its music stream (0x1002e9ce play file, 0x1002ea0a pause / go on, 0x1002e9df stop, 0x1002e9fd
# "playing", 0x1002e9ec volume) and, for the C side's option setter, the volume through the original 1.0
# 0x1009ebeb -> 0x1009eb85 (hooked there):
#     ('play', track) ('pause', 0 / 1) ('stop',) ('playing?',) ('volume', min(max(v, 0), 0xffff))
# 1.2's track file names are the 255-byte entries 0x1012d670 + (track - 1) * 0xff (wavmap.ini Track<N>), logged as
# the track number. "playing?" answers come from the generator, the same for both sides.
# 1.2's state is mapped across (to12 / from12): track 0x101321a8 = 1.2 0x10108c18, option flags 0x101321c0 =
# 0x10108c30, paused 0x1015d1bc = 0x1012fe50, read-once 0x1015d1c0 = 0x1012fe54, the poll timer 0x1015d1c4 = 1.2's
# WAV poll timer 0x1012fe64, frame ticks 0x1015dc80 = 0x10130770, the options 0x101321b0 / b4 / b8 / bc =
# 0x10108c20 / c24 / c28 / c2c, networked 0x1015ec58 = 0x101314a0, the mission's track text 0x1015dcb5 =
# 0x1028ce85. 1.2's music table 0x10108c8c..0x10108ca0 holds its WAV functions (as its wavmap.ini reader sets it),
# its last WAV index 0x1012fe48 = the C side's Music_LastTrack() - 1.
# Not compared, by design: no music files (1.2 then plays the CD; ours has no CD: Music_LastTrack() is >= 2 in
# every trial), and 1.2's option setter setting the volume only while the CD is open (its WAV music never followed
# the option, a bug not kept): 1.2's CD flag 0x1014b944 is 1 in every trial. Only option 2 (the music) of the
# setter is compared; the others call the 1.0 / 1.2 sound code that differs by address only.
# Negative check: MUSIC_NEG12=1 builds the C with W5AM_MUTATE (the poll restarts a track that is still playing).
import os
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1001/w6d_calls.c', 'sim/unsorted_1001/w4u_cd.c', 'sim/unsorted_1001/w5a_misc.c',
                   'sim/unsorted_1001/w7l_misc.c', 'sim/unsorted_1001/w9d_misc.c', 'sim/win/w4w_win.c'])
SOURCES = SOURCES + ['../tools/verify/music_stub.c']
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe', '-DHG_MUSIC_FILES']
if os.environ.get('MUSIC_NEG12'):
    CFLAGS.append('-DW5AM_MUTATE')

MAP = [  # (1.0 address, 1.2 address, bytes)
    (0x101321a8, 0x10108c18, 4), (0x101321c0, 0x10108c30, 4), (0x1015d1bc, 0x1012fe50, 4),
    (0x1015d1c0, 0x1012fe54, 4), (0x1015d1c4, 0x1012fe64, 4), (0x1015dc80, 0x10130770, 4),
    (0x101321b0, 0x10108c20, 4), (0x101321b4, 0x10108c24, 4), (0x101321b8, 0x10108c28, 4),
    (0x101321bc, 0x10108c2c, 4), (0x1015ec58, 0x101314a0, 4), (0x1015dcb5, 0x1028ce85, 16),
]
WAV_TABLE = [0x10015e83, 0x10015f1b, 0x10015f59, 0x10015f97, 0x10015fbd, 0x1001603f]
NAMES12 = 0x1012d670
MS = {}


def clamp(v):
    v = v - (1 << 32) if v >= 1 << 31 else v
    return min(max(v, 0), 0xffff)


def _event(m, name, args):
    if MS.get('log') is not m.api_log:
        MS['log'] = m.api_log
        MS['asks'] = 0
    if name == 'playing?':
        m.api_log.append(('playing?',))
        k = MS['asks']
        MS['asks'] = k + 1
        return MS['playing'][k % len(MS['playing'])]
    if name == 'volume':
        m.api_log.append(('volume', clamp(args[0])))
        return 1
    if name == 'last':
        return MS['last']
    m.api_log.append((name,) + tuple(args))
    return 1


def _cb(m, name, nargs, conv=None):
    def cb(uc, address, size, ud):
        uc = m.uc
        esp = uc.reg_read(UC_X86_REG_ESP)
        a = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
        if conv:
            a = conv(a)
        eax = _event(m, name, a)
        uc.reg_write(UC_X86_REG_EAX, eax & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)        # cdecl
    return cb


def s32(v):
    v &= 0xffffffff
    return v - (1 << 32) if v >= 1 << 31 else v


def _track12(a):
    """1.2's play call (stream, file name): the track whose name entry it is, in 1.2's 32-bit address arithmetic
    (0xff is odd, so (track - 1) * 0xff has one inverse modulo 2^32, also for tracks far out of range)"""
    return [s32((a[1] - A12(NAMES12)) * pow(0xff, -1, 1 << 32) + 1)]


def install(m):
    if getattr(m, '_music', False):
        return
    m._music = True
    hooks = [  # C side: platform/music.h
        (m.syms['Music_Play'], 'play', 1, lambda a: [s32(a[0])]), (m.syms['Music_Pause'], 'pause', 1, None),
        (m.syms['Music_Stop'], 'stop', 0, None), (m.syms['Music_IsPlaying'], 'playing?', 0, None),
        (m.syms['Music_SetVolume'], 'volume', 1, None), (m.syms['Music_LastTrack'], 'last', 0, None),
        (0x1009eb85, 'volume', 1, None),                     # the C setter's volume through the 1.0 original
        # 1.2 side: its music stream
        (A12(0x1002e9ce), 'play', 2, _track12), (A12(0x1002ea0a), 'pause', 2, lambda a: [a[1]]),
        (A12(0x1002e9df), 'stop', 1, lambda a: []), (A12(0x1002e9fd), 'playing?', 1, None),
        (A12(0x1002e9ec), 'volume', 2, lambda a: [a[1]]),
    ]
    for addr, name, n, conv in hooks:
        m.raw.hook_add(UC_HOOK_CODE, _cb(m, name, n, conv), None, addr, addr)


def to12(m):
    for a10, a12, n in MAP:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, n)))
    m.uc.mem_write(A12(0x10108c8c), struct.pack('<6I', *[A12(a) for a in WAV_TABLE]))
    m.uc.mem_write(A12(0x1012fe48), struct.pack('<I', MS['last'] - 1))
    m.uc.mem_write(A12(0x1014b944), struct.pack('<I', 1))
    m.uc.mem_write(A12(0x10131310), struct.pack('<I', 0x00ae0000))     # 1.2's stream object (its handle only)


def from12(m):
    for a10, a12, n in MAP:
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), n)))


TEXTS = [b'', b'2', b'3', b'7', b'12', b'13', b'40', b'41', b'0', b'-1', b'-5', b'x', b' 9', b'5a', b'999999']


def state(ctx):
    """The music state of a trial, written on the 1.0 / C side (to12 copies it to 1.2's)."""
    r = ctx.rng
    install(ctx.m)
    MS.clear()
    MS['last'] = r.choice([2, 3, 12, 12, 12, 40, r.randrange(2, 41)])
    MS['playing'] = [r.choice([0, 1]) for _ in range(4)]
    w32(ctx, 0x101321a8, r.choice([-1, -1, 0, 1, 2, 5, 12, 13, 40, 41, -7, r.getrandbits(32)]))
    w32(ctx, 0x101321c0, r.choice([0, 8, 8, 0xb, 0xf, 2, r.getrandbits(32)]))
    w32(ctx, 0x1015d1bc, r.choice([0, 0, 1, 1, 2]))
    w32(ctx, 0x1015d1c0, r.choice([0, 1, 1]))
    w32(ctx, 0x1015d1c4, r.choice([0, 0x100, 0x169, 0x16a, 0x200, -5, r.getrandbits(32)]))
    w32(ctx, 0x1015dc80, r.choice([0, 1, 16, 33, 0x16a, -3, r.getrandbits(32)]))
    for a in (0x101321b0, 0x101321b4, 0x101321b8, 0x101321bc):
        w32(ctx, a, r.choice([0, 0x10000, 0xffff, 0x7fff, 1, -1, r.getrandbits(32)]))
    w32(ctx, 0x1015ec58, r.choice([0, 0, 1]))
    wb(ctx, 0x1015dcb5, r.choice(TEXTS).ljust(16, b'\0')[:16])


def g0(ctx):
    state(ctx)
    return []


def g_volume(ctx):
    state(ctx)
    return [ctx.rng.choice([0, 1, 0x7fff, 0xfffe, 0xffff, 0x10000, -1, -0x10000, ctx.rng.getrandbits(32)]) & 0xffffffff]


def g_set(ctx):
    state(ctx)
    r = ctx.rng
    return [2, r.choice([0, 0, 1, 0xffff, 0x10000, r.getrandbits(32)])]


def g_get(ctx):
    state(ctx)
    return [ctx.rng.choice([-1, 0, 1, 2, 2, 3, 4, 5])]


def spec(name, c, a12, gen, void=True, trials=1500):
    return dict(name=name, c=c, orig12=a12, gen=gen, void=void, trials=trials, to12=to12, from12=from12)


SPECS = [
    spec('Music_Start_v12', 'Sim_1001f26e', 0x10015e83, g0),
    spec('Music_Pause_v12', 'Sim_1001f335', 0x10015f1b, g0),
    spec('Music_Resume_v12', 'Sim_1001f367', 0x10015f59, g0),
    spec('Music_Stop_v12', 'Sim_1001f399', 0x10015f97, g0),
    spec('Music_Poll_v12', 'Sim_1001f3ac', 0x10015fbd, g0),
    spec('Music_Volume_v12', 'Win_AuxSetVolume', 0x1001603f, g_volume, void=False),
    spec('Music_Option_v12', 'Sim_1001ee7c', 0x1001550d, g_set),
    spec('Music_OptionRead_v12', 'Sim_1001edd0', 0x10015450, g_get, void=False),
]
