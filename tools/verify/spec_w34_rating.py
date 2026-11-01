# Round 33: 1.2's 10-player kill table and rating against 1.2's own code: the kill tally Score_RecordKill (1.2
# 0x10059e9d), the "KI" send Sim_100413c7 (0x10035088), the rating sum Sim_10041444 (0x10035105), the "KI" receive
# Sim_1004158c (0x1003524d), 1.2's new "AK" send / receive (0x100352b7 / 0x10035348) and the winner pick
# Sim_1007b02d (0x10067f2d). Their callees are stubbed on both sides (the network send logs its message whole).
# 1.2's player records are ours without the dword at +0x18 (its flags +0x18 are our +0x1c, ...): the records are
# turned into 1.2's layout for its run and back after it. The C's new tables (g_w34Kills, g_w34Rating, the guard
# g_w34NetOver, the "AK" buffer) are paired with 1.2's; the score counters 0x101653e0.. are 1.2's 0x1028cae0..
# Negative check: W34_NEG=1 builds the C with W34_MUTATE (the kind-3 kill counted in the wrong column).
import os
exec(open('tools/verify/w32_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w9b_tally.c', 'sim/unsorted_1004/w3p_net.c', 'sim/unsorted_1007/w2f_misc.c'])
if os.environ.get('W34_NEG'):
    CFLAGS = list(CFLAGS) + ['-DW34_MUTATE']
PTRTAB = 0x109e8b40
FN3 = 0x1010d3f0                       # a stand-in for the session call [0x1015eb20]
helper(FN3, None, 3)

stub12(0x10015c61, 'Sim_10015c61', 1, 0x1000edfa)
stub12(0x1000ecb0, 'Sim_1000ecb0', 1, 0x100087d8, sargs=(0,))
stub12(0x10052be7, 'Sim_10052be7', 1, 0x10042b07)
stub12(0x100203cf, 'Sim_100203cf', 2, 0x100176df)
stub12(0x1001150d, 'Sim_1001150d', 1, 0x1000a69d)
stub12(0x10052aaf, 'Sim_10052aaf', 1, 0x100429cf)
stub12(0x10012843, 'Sim_10012843', 1, 0x1000b9d3)
stub12(0x1007df6c, 'Sim_1007df6c', 4, 0x1006a0ec)
stub12(0x100128dd, 'Sim_100128dd', 3, 0x1000ba6d)
stub12(0x100fc780, 'Crt_SPrintf', 5, 0x100da920, sargs=(1,))
stub12(0x1005f9aa, 'Sim_1005f9aa', 1, 0x100517ca)
stub12(0x10020256, 'Sim_10020256', 2, 0x10017566)
stub12(0x1003ad51, 'Gear_SetLod', 2, 0x1002eeb1)
stub12(0x100ac58e, 'Sim_100ac58e', 1, 0x1009733e)
stub12(0x1007e920, 'Net_DpSend_1007e920', 3, 0x1006a860, deref={0: 0x10})
STUBBED = {'Sim_1007b02d': (0x1007b02d, 0, 0x10067f2d), 'Sim_100413c7': (0x100413c7, 0, 0x10035088),
           'Sim_10041444': (0x10041444, 0, 0x10035105)}
helper(A12(0x100352b7), 'Sim12_100352b7', 2)
CALLS = (FN3, 0x10015c61, 0x1000ecb0, 0x10052be7, 0x100203cf, 0x1001150d, 0x10052aaf, 0x10012843, 0x1007df6c,
         0x100128dd, 0x100fc780, 0x1005f9aa, 0x10020256, 0x1003ad51, 0x100ac58e, 0x1007e920, A12(0x100352b7))

GLOBALS = [
    (0x1017b8c8, 0x1014a568, 4),        # the local player
    (0x1015ec58, 0x101314a0, 4),        # network game
    (0x1017b8c4, 0x1014a564, 4),        # the game options
    (PTRTAB, 0x107f24c0, 0x28),         # the players
    (0x109e8480, 0x107f1e60, 0x77 * 4), # the sound names
    (0x101c64f0, 0x10188b44, 4),
    (0x1017e678, 0x1014d460, 4),
    (0x1015ebbc, 0x10131424, 4),
    (0x10179cac, 0x10148df8, 4),
    (0x1015dc78, 0x10130768, 4),
    (0x101c42a8, 0x10186ee8, 4),
    (0x109b98c0, 0x107ca7e0, 4),
    (0x101653e0, 0x1028cae0, 0x14c),    # the score counters
    (0x101655ac, 0x1028ccf4, 4),        # the winner pick
    (0x1015ec98, 0x101314e4, 4),        # the "KI" buffer
    (0x1015eb20, 0x101313b4, 4),        # the session call
    (0x1015ecd4, 0x10131528, 4),        # the session
    (0x1013bb40, 0x10110e58, 4),        # the mission
    (0x10473800, 0x10290c80, 0xa60),    # its player slots (mission 0)
    (0x10473800 + 0x4fc3a, 0x10290c80 + 0x4fc3a, 0xa60),
    ('g_w34Kills', 0x1028cc2c, 200),
    ('g_w34Rating', 0x107caa60, 20),
    ('g_w34NetOver', 0x101314c0, 4),
]
ADDRS = {0x101362e0: 0x1010c520}
FUNCS = {}
UNIT = 0x200


def to12_units(m):
    """1.2's records lack our dword +0x18"""
    ST['usave'] = {}
    for u in ST.get('units', ()):
        d = bytes(m.uc.mem_read(u, UNIT))
        ST['usave'][u] = d
        m.uc.mem_write(u, d[:0x18] + d[0x1c:] + bytes(4))
    m.uc.mem_write(A12(0x101314e8), struct.pack('<I', m.syms['g_w34AkBuf']))


def from12_units(m):
    for u, old in ST['usave'].items():
        d = bytes(m.uc.mem_read(u, UNIT))
        m.uc.mem_write(u, d[:0x18] + old[0x18:0x1c] + d[0x18:UNIT - 4])
    # 1.2's sound-name pointers point into its own table
    for i in range(0x77):
        a = 0x109e8480 + 4 * i
        v = struct.unpack('<I', bytes(m.uc.mem_read(a, 4)))[0]
        if A12(0x1010c520) <= v < A12(0x1010c520) + 0x10000:
            m.uc.mem_write(a, struct.pack('<I', v - A12(0x1010c520) + 0x101362e0))


TO12_MORE.append(to12_units)
FROM12_MORE.append(from12_units)


def stub_c(names):
    """the C's own functions stubbed on both sides for one spec"""
    for n in names:
        a10, k, a12 = STUBBED[n]
        stub12(a10, n, k, a12)


def unit(ctx, r, i, count):
    e = newbuf(ctx, UNIT)
    blk = newbuf(ctx, 0xc50)
    wb(ctx, e, bytes(r.getrandbits(8) for _ in range(UNIT)))
    wb(ctx, blk, bytes(r.getrandbits(8) for _ in range(0xc50)))
    w32(ctx, e, r.choice([1, 1, 3, 4, 5, 8, 10, 2, 0]))
    w32(ctx, e + 4, i)
    w32(ctx, e + 8, r.choice([0, 1, 2]))
    w32(ctx, e + 0xc, r.choice([ST['self'], ST['self'], r.randrange(count)]))
    w32(ctx, e + 0x10, r.choice([0, 1, 2, 3]))
    w32(ctx, e + 0x14, r.choice([3, 3, 1, 0]))
    w16(ctx, e + 0x1c, r.choice([0, 0, 2, 4, 0x1400, 0x1000, 0x400, r.getrandbits(16)]))
    w32(ctx, e + 0x28, blk)
    w32(ctx, blk, e)
    w32(ctx, blk + 0x4f0, r.choice([4, 5, 5, 7, 2, r.getrandbits(32)]))
    w32(ctx, blk + 0x613, r.choice([0, 1, 2]))
    w32(ctx, blk + 0xbdb, r.randrange(7))
    w32(ctx, PTRTAB + 4 * i, e)
    return e


def world(ctx, *calls):
    r = begin(ctx, *[c for c in calls if c in HELPERS])
    ST['seed'] = r.getrandbits(16)
    count = r.choice([1, 2, 3, 4, 8, 9, 10])
    ST['self'] = r.randrange(count)
    w32(ctx, 0x1017b8c8, ST['self'])
    w32(ctx, 0x1015ec58, r.choice([0, 1, 2, 2]))
    ST['units'] = [unit(ctx, r, i, count) for i in range(count)]
    for i in range(count, 10):
        w32(ctx, PTRTAB + 4 * i, 0)
    w32(ctx, 0x1017b8c4, ctx.buf(0x10, bytes([0, r.choice([0, 1]), 0, 0, 0, 0, 0, 0, 0, 0, r.choice([0, 0, 1]),
                                               0, 0, 0, 0, 0])))
    wb(ctx, ctx.m.syms['g_w34Kills'], bytes(r.getrandbits(8) for _ in range(200)))
    wb(ctx, ctx.m.syms['g_w34Rating'], bytes(r.getrandbits(8) for _ in range(20)))
    w32(ctx, ctx.m.syms['g_w34NetOver'], r.choice([0, 0, 0, 1]))
    w32(ctx, 0x1015ec98, newbuf(ctx, 0x100))
    w32(ctx, 0x1015eb20, FN3)
    w32(ctx, 0x1015ecd4, r.getrandbits(32))
    wb(ctx, 0x101653e0, bytes(r.getrandbits(8) for _ in range(0x14c)))
    w32(ctx, 0x101655ac, r.getrandbits(32))
    return r, count


def g_kill(ctx):
    r, count = world(ctx, *CALLS, 0x1007b02d, 0x100413c7)
    v = r.randrange(count)
    e = ST['units'][v]
    blk = U32_(ctx, e + 0x28)
    w32(ctx, blk + 0xbdf, r.getrandbits(32))
    w32(ctx, U32_(ctx, ST['units'][ST['self']] + 0x28) + 0xbe3,
        (U32_(ctx, blk + 0xbdf) - r.choice([0, 3, 5, 6, 7, 100, -2])) & 0xffffffff)
    w32(ctx, 0x101c64f0, r.choice([0, 1]))
    w32(ctx, 0x1017e678, r.choice([0, 0, newbuf(ctx, 0x30)]))
    if U32_(ctx, 0x1017e678) != 0:
        w16(ctx, U32_(ctx, 0x1017e678) + 2, r.choice([0x100, 0x100, 0]))
        w16(ctx, U32_(ctx, 0x1017e678) + 0x24, r.choice([v, r.randrange(10)]))
    w32(ctx, 0x1015ebbc, r.choice([0, 1]))
    w32(ctx, 0x10179cac, r.getrandbits(32))
    w32(ctx, 0x101c42a8, newbuf(ctx, 0x10))
    w32(ctx, 0x109b98c0, newbuf(ctx, 0x10))
    rets(0x10015c61, r.randrange(10), r.randrange(10))
    rets(0x10052be7, *[r.choice([0, 1, 2, 3]) for _ in range(5)])
    rets(0x10052aaf, *[r.choice([v, ST['self'], r.randrange(10)]) for _ in range(3)])
    rets(0x10012843, *[r.choice([ST['self'], r.randrange(10)]) for _ in range(3)])
    killer = r.choice([0xfffffffe, 0xffffffff, ST['self'], ST['self'], r.randrange(count), r.randrange(10), 10,
                       0x8000])
    return [killer, blk]


def g_none(ctx):
    world(ctx, *CALLS, 0x10041444, 0x1007b02d)
    return []


def g_recv(ctx):
    r, count = world(ctx, *CALLS, 0x10041444, 0x1007b02d)
    msg = newbuf(ctx, 0x20)
    wb(ctx, msg, bytes(r.getrandbits(8) for _ in range(0x20)))
    w8(ctx, msg + 3, r.randrange(10))
    return [msg, r.randrange(10)]


def g_ak(ctx):
    r, count = world(ctx, *[c for c in CALLS if c != A12(0x100352b7)], 0x10041444)
    return [r.getrandbits(8), r.randrange(10)]


def g_ak_recv(ctx):
    a = g_recv(ctx)
    return a[:1]


def g_pick(ctx):
    r, count = world(ctx)
    w32(ctx, 0x1013bb40, r.choice([0, 1]))
    base = 0x10473800 + U32_(ctx, 0x1013bb40) * 0x4fc3a
    n = r.choice([0, 1, 2, 3, 8])
    w32(ctx, base, n)
    for i in range(n):
        rec = base + 0x3a + i * 0x13f
        w8(ctx, rec + 0x74, r.choice([0, 0, 1, 2]))
        w8(ctx, rec + 1, r.choice([2, 2, 1, 0]))
    return []


def U32_(ctx, a):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(a, 4)))[0]


PART = os.environ.get('W34_PART', 'kill')
if PART == 'kill':
    stub_c(['Sim_1007b02d', 'Sim_100413c7'])
    SPECS = [spec12('Score_RecordKill', 0x10059e9d, g_kill, 3000, void=True)]
elif PART == 'net':
    stub_c(['Sim_10041444', 'Sim_1007b02d'])
    SPECS = [spec12('Sim_100413c7', 0x10035088, g_none, 1500, void=True),
             spec12('Sim_1004158c', 0x1003524d, g_recv, 1500, void=True),
             spec12('Sim12_100352b7', 0x100352b7, g_ak, 1500, void=True),
             spec12('Sim12_10035348', 0x10035348, g_ak_recv, 1500, void=True)]
else:
    SPECS = [spec12('Sim_10041444', 0x10035105, g_none, 2000, void=True),
             spec12('Sim_1007b02d', 0x10067f2d, g_pick, 2000, void=True)]
