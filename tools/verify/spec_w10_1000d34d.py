# Round 10: 0x1000d34d (fire decision). Stubbed on both sides (original address and C function): Sim_1009c929
# (random ring), Sim_1001403a (its 16-bit target argument masked: the original pushes eax with a stale upper
# half), the weapon checks Sim_1000bf73 / Sim_1000bba7 (they also write the flag through their pointer, the same on
# both runs), Sim_1000a6f0 and Sim_1000c172. Sim_1001448d / Sim_100144bb (float, st(0)) and the CRT fmod run
# for real. Exact (game logic).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1000/w10_misc.c'])
helper(0x1009c929, 'Sim_1009c929', 1)
helper(0x1001403a, 'Sim_1001403a', 2)
MASK[(0x1001403a, 1)] = 0xffff
helper(0x1000bf73, 'Sim_1000bf73', 2)
helper(0x1000bba7, 'Sim_1000bba7', 2)
helper(0x1000a6f0, 'Sim_1000a6f0', 2)
helper(0x1000c172, 'Sim_1000c172', 1)


def e_flag(uc, k, raw, eax):
    uc.mem_write(raw[1], struct.pack('<I', ST['flag']))
    return None


EFFECT[0x1000bf73] = e_flag
EFFECT[0x1000bba7] = e_flag


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def ang(r):
    x = r.random()
    if x < 0.6:
        return fb(r.uniform(-30.0, 30.0))
    if x < 0.8:
        return fb(r.choice([10.0, -10.0, 9.999999, -9.999999, 0.0, 360.0, 350.0, 5.0]))
    if x < 0.95:
        return fb(r.uniform(-720.0, 720.0))
    return rfloat(r)


def g(ctx):
    r = begin(ctx, 0x1009c929, 0x1001403a, 0x1000bf73, 0x1000bba7, 0x1000a6f0, 0x1000c172)
    obj = newbuf(ctx, 0x200)
    rec = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x28, rec)
    w32(ctx, obj + 0x54, newbuf(ctx, 0x30))
    now = r.randrange(0, 100000)
    w32(ctx, 0x1015dc78, now)
    w32(ctx, obj + 0x1be, r.choice([now, now - 1, now + 1, now - 500, now + 500, r.getrandbits(32)]))
    w8(ctx, obj + 0x1b6, r.getrandbits(8))
    for off in (0x74, 0x64, 0xec):
        w32(ctx, obj + off, ang(r))
    w32(ctx, rec + 0x1c, ang(r))
    if r.random() < 0.5:
        w32(ctx, obj + 0x74, struct.unpack('<I', struct.pack('<f', 0.0))[0])
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    w16(ctx, obj + 0x1aa, r.choice([self_, self_ | 0x100, r.getrandbits(16)]))
    w32(ctx, obj + 0x1c2, r.choice([0, 1]))
    ST['flag'] = r.choice([0, 1, 2])
    rets(0x1009c929, r.choice([0, 0, 1, 3]), r.choice([0, 1]))
    rets(0x1000bf73, r.choice([0, 1]))
    rets(0x1000bba7, r.choice([0, 1]))
    rets(0x1000a6f0, r.choice([0, 1, 1]))
    a = ang(r) if r.random() < 0.5 else struct.unpack('<I', struct.pack('<f', struct.unpack('<f', struct.pack('<I', r.choice([0, 0x3f800000])))[0]))[0]
    return [obj, a]


SPECS = [dict(name='Sim_1000d34d', orig=0x1000d34d, c='Sim_1000d34d', gen=g, trials=4000)]
