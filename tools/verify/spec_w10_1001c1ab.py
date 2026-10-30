# Round 10: 0x1001c1ab (animation step). Every callee is stubbed on both sides (original address and C function):
# Sim_100aadaa (bone node by name, name logged as text), Sim_1001bb9d (key-framed rotation: writes 16 bytes through
# its pointer), the original slerp 0x100608ef (both rotations logged as 16-byte records, the result written through
# its pointer, which may be the shared identity 0x10131740 in the DLL's data), Sim_100aa227 / Sim_100aa1de /
# Sim_100aa190 (the rotation logged as a 16-byte record), Sim_100aa799, Sim_1001c075 (an event record of the
# generator or 0), Sim_100a9ccf / Sim_100a9cfb (write three floats), Sim_100a9d7c. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1001/w10_misc.c'])
helper(0x100aadaa, 'Sim_100aadaa', 2, sargs=(1,))
helper(0x1001bb9d, 'Sim_1001bb9d', 4)
helper(0x100608ef, None, 4, deref={0: 16, 1: 16})
for _a, _c in ((0x100aa227, 'Sim_100aa227'), (0x100aa1de, 'Sim_100aa1de'), (0x100aa190, 'Sim_100aa190')):
    helper(_a, _c, 2, deref={1: 16})
helper(0x100aa799, 'Sim_100aa799', 1)
helper(0x1001c075, 'Sim_1001c075', 3)
helper(0x100a9ccf, 'Sim_100a9ccf', 4)
helper(0x100a9cfb, 'Sim_100a9cfb', 4)
helper(0x100a9d7c, 'Sim_100a9d7c', 4)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_q(uc, k, raw, eax):
    uc.mem_write(raw[3], struct.pack('<4I', *ST['q'][k % len(ST['q'])]))


def e_slerp(uc, k, raw, eax):
    uc.mem_write(raw[3], struct.pack('<4I', *ST['q'][(k + 3) % len(ST['q'])]))


def e_three(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[1 + i], struct.pack('<I', ST['three'][(k + i) % len(ST['three'])]))


EFFECT.update({0x1001bb9d: e_q, 0x100608ef: e_slerp, 0x100a9ccf: e_three, 0x100a9cfb: e_three})


def anim_rec(ctx, nb):
    r = ctx.rng
    a = newbuf(ctx, 0x320)
    hdr = newbuf(ctx, 0x10)
    w32(ctx, hdr + 4, r.randrange(1, 200))
    w32(ctx, hdr + 0xc, nb)
    w32(ctx, a + 0x14, hdr)
    w32(ctx, a, r.choice([1, 1, 0]))
    for i in range(nb):
        bone = ctx.buf(16, bytes(4) + struct.pack('<I', r.choice([1, 2, 3, 4, 5])) + b'\0' * 8)  # the id at +4 is also the name's first bytes
        w32(ctx, a + 0x1c + i * 0xc, bone)
        keys = newbuf(ctx, 8)
        w32(ctx, keys, r.choice([0, 1, 3]))
        w32(ctx, a + 0x24 + i * 0xc, keys)
    w32(ctx, a + 0x31c, r.getrandbits(32))
    return a


def g(ctx):
    r = begin(ctx, 0x100aadaa, 0x1001bb9d, 0x100608ef, 0x100aa227, 0x100aa1de, 0x100aa190, 0x100aa799, 0x1001c075,
              0x100a9ccf, 0x100a9cfb, 0x100a9d7c)
    ST['q'] = [[fb(r.uniform(-1, 1)) for _ in range(4)] for _ in range(5)]
    ST['three'] = [fb(r.uniform(-50, 50)) if r.random() < 0.9 else r.choice([0x7f7fc99e, rfloat(r)]) for _ in range(7)]
    for a, v in ((0x1015d190, 1), (0x1015d194, 2), (0x1015d198, 3), (0x1015d19c, 4)):
        w32(ctx, a, v if r.random() < 0.9 else 9)
    w32(ctx, 0x10131738, r.choice([0, 1]))
    nb = r.randrange(0, 5)
    anim = anim_rec(ctx, nb)
    state = newbuf(ctx, 0x40)
    w32(ctx, state + 0x10, anim_rec(ctx, max(nb, 1)))
    w32(ctx, state + 0x18, r.choice([0, 1]))
    w32(ctx, state + 4, r.randrange(0, 500))
    w32(ctx, state + 0x2c, r.choice([0, 1, 2, 3]))
    obj = newbuf(ctx, 0x160)
    w32(ctx, obj, r.choice([1, 9, 2]))
    w32(ctx, obj + 0x15c, r.choice([0, 0, 1]))
    w32(ctx, obj + 0x7c, fb(r.uniform(-50, 50)))
    rec = newbuf(ctx, 0x570)
    w32(ctx, rec + 0x4c0, r.choice([0, 0x7000]))
    w32(ctx, rec + 0x4c4, r.choice([0, 0x7100]))
    w16(ctx, rec + 0x562, r.getrandbits(16))
    w32(ctx, obj + 0x28, rec)
    w32(ctx, obj + 0x48, newbuf(ctx, 0x20))
    targets = []
    for i in range(3):
        tg = newbuf(ctx, 8)
        w32(ctx, tg, r.choice([0, 1]))
        targets.append(tg)
    rets(0x100aadaa, *[r.choice(targets + [0]) for _ in range(3)])
    rets(0x1001bb9d, r.choice([0, 0, 1]), r.choice([0, 1]))
    ev = newbuf(ctx, 0x20)
    w32(ctx, ev + 0xd, r.choice([0, 1, 2, 3]))
    w32(ctx, ev + 0x11, r.choice([0, 1]))
    rets(0x1001c075, r.choice([0, ev, ev]))
    if r.random() < 0.03:
        obj = 0
    return [obj, r.choice([0, r.randrange(1, 1000)]), anim, r.randrange(-20, 30), state]


SPECS = [dict(name='Sim_1001c1ab', orig=0x1001c1ab, c='Sim_1001c1ab', gen=g, void=True, trials=4000)]
