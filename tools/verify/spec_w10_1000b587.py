# Round 10: 0x1000b587 (fire decision, weapon-check variant). Stubbed on both sides (original and C function):
# Sim_1009c929, Sim_1000ceb7, Sim_1000bf73 / Sim_1000bba7 (write the flag), Sim_1000b9f1 (writes the entry
# pointer), Sim_1000c749, Sim_1000ca63 (writes its float), Sim_1000ae6e (writes the two limits), Sim_1000b01f,
# Sim_1000a6f0, Sim_1000c6e3 and Sim_1000c172; all writes are the same on both runs. Sim_1001448d / Sim_100144bb
# (float, st(0)) and the CRT fmod run for real. Game logic: exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1000/w10_misc.c'])
for _a, _c, _n in ((0x1009c929, 'Sim_1009c929', 1), (0x1000ceb7, 'Sim_1000ceb7', 1), (0x1000bf73, 'Sim_1000bf73', 2),
                   (0x1000bba7, 'Sim_1000bba7', 2), (0x1000b9f1, 'Sim_1000b9f1', 5), (0x1000c749, 'Sim_1000c749', 3),
                   (0x1000ca63, 'Sim_1000ca63', 3), (0x1000ae6e, 'Sim_1000ae6e', 3), (0x1000b01f, 'Sim_1000b01f', 4),
                   (0x1000a6f0, 'Sim_1000a6f0', 2), (0x1000c6e3, 'Sim_1000c6e3', 3), (0x1000c172, 'Sim_1000c172', 1)):
    helper(_a, _c, _n)


def e_flag(uc, k, raw, eax):
    uc.mem_write(raw[1], struct.pack('<I', ST['flag']))


def e_entry(uc, k, raw, eax):
    uc.mem_write(raw[3], struct.pack('<I', ST['entry']))


def e_chk(uc, k, raw, eax):
    if ST['chkw']:
        uc.mem_write(raw[2], struct.pack('<I', ST['chk']))


def e_lim(uc, k, raw, eax):
    uc.mem_write(raw[1], struct.pack('<I', ST['lim'][0]))
    uc.mem_write(raw[2], struct.pack('<I', ST['lim'][1]))


EFFECT.update({0x1000bf73: e_flag, 0x1000bba7: e_flag, 0x1000b9f1: e_entry, 0x1000ca63: e_chk, 0x1000ae6e: e_lim})


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def ang(r):
    return fb(r.choice([r.uniform(-30, 30), r.uniform(-720, 720), 0.0, 10.0])) if r.random() < 0.95 else rfloat(r)


def g(ctx):
    r = begin(ctx, 0x1009c929, 0x1000ceb7, 0x1000bf73, 0x1000bba7, 0x1000b9f1, 0x1000c749, 0x1000ca63, 0x1000ae6e,
              0x1000b01f, 0x1000a6f0, 0x1000c6e3, 0x1000c172)
    o = newbuf(ctx, 0x200)
    w32(ctx, o, r.choice([1, 9, 0, 2, 5]))
    rec = newbuf(ctx, 0x500)
    w32(ctx, o + 0x28, rec)
    w32(ctx, o + 0x54, newbuf(ctx, 0x30))
    now = r.randrange(0, 100000)
    w32(ctx, 0x1015dc78, now)
    w32(ctx, o + 0x1be, r.choice([now, now - 1, now + 1, now + 500]))
    w8(ctx, o + 0x1b6, r.getrandbits(8))
    for off in (0x74, 0x64, 0xec, 0xd4):
        w32(ctx, o + off, ang(r))
    w32(ctx, rec + 0x1c, ang(r))
    w32(ctx, o + 0x1c2, r.choice([0, 1]))
    slots = newbuf(ctx, 0x128 * 4)
    w32(ctx, rec + 0x84, slots)
    si = r.randrange(0, 4)
    w32(ctx, rec + 0x4fc, si)
    w32(ctx, slots + si * 0x128 + 4, r.randrange(0, 40))
    ent = newbuf(ctx, 0x60)
    w32(ctx, ent + 0x54, r.randrange(0, 100))
    ST['entry'] = r.choice([0, ent])
    ST['flag'] = r.choice([0, 1])
    ST['chkw'] = r.choice([0, 1])
    ST['chk'] = ang(r)
    ST['lim'] = [fb(r.uniform(0, 20)), fb(r.uniform(0, 20))]
    rets(0x1009c929, r.choice([0, 0, 1, 2]))
    rets(0x1000ceb7, r.choice([0, 1]))
    rets(0x1000bf73, r.choice([0, 1]))
    rets(0x1000bba7, r.choice([0, 1]))
    rets(0x1000b9f1, r.choice([0, 1]))
    rets(0x1000c749, r.choice([0, 1, 2]), r.choice([0, 1, 1]))
    rets(0x1000b01f, r.choice([0, 1, 1]))
    rets(0x1000a6f0, r.choice([0, 1]))
    rets(0x1000c6e3, r.choice([0, 1]))
    return [o, ang(r)]


SPECS = [dict(name='Sim_1000b587', orig=0x1000b587, c='Sim_1000b587', gen=g, trials=5000)]
