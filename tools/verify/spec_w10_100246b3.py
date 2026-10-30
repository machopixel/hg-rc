# Round 10: 0x100246b3 (spawn an effect). Stubbed on both sides: Sim_100cbc6b (writes four floats), Sim_10045f3b,
# Sim_10025c8c, Sim_10058575, Sim_100585e1 (light record: the words it reads with mask 0xf logged), Sim_1005892e,
# Sim_100ac21a, Sim_100ac620 (its whole 0x14-byte record logged), Sim_100096ee, Sim_100a9cfb (writes three floats),
# Sim_10009408, Sim_100d7111, Sim_100aa0da, Sim_100a9d47, Sim_100aa39f, Sim_100aa406, Sim_100aa799, Sim_1000716a,
# Sim_1000711c (word arguments masked), Sim_100d36d6, Sim_10024616, Sim_1009c929, Sim_100448ff, Sim_1000ed52, and
# Sim_10084a94 / Sim_1002d961 as float stubs. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1002/w10_effects.c'])
STUBS = ((0x100cbc6b, 'Sim_100cbc6b', 7), (0x10045f3b, 'Sim_10045f3b', 3), (0x10025c8c, 'Sim_10025c8c', 4),
         (0x10058575, 'Sim_10058575', 0), (0x100585e1, 'Sim_100585e1', 3), (0x1005892e, 'Sim_1005892e', 2),
         (0x100ac21a, 'Sim_100ac21a', 2), (0x100096ee, 'Sim_100096ee', 1),
         (0x100a9cfb, 'Sim_100a9cfb', 4), (0x10009408, 'Sim_10009408', 2), (0x100d7111, 'Sim_100d7111', 2),
         (0x100aa0da, 'Sim_100aa0da', 5), (0x100a9d47, 'Sim_100a9d47', 4), (0x100aa39f, 'Sim_100aa39f', 1),
         (0x100aa406, 'Sim_100aa406', 1), (0x100aa799, 'Sim_100aa799', 1), (0x1000716a, 'Sim_1000716a', 2),
         (0x1000711c, 'Sim_1000711c', 2), (0x100d36d6, 'Sim_100d36d6', 3), (0x10024616, 'Sim_10024616', 8),
         (0x1009c929, 'Sim_1009c929', 1), (0x100448ff, 'Sim_100448ff', 0), (0x1000ed52, 'Sim_1000ed52', 6))
for _a, _c, _n in STUBS:
    helper(_a, _c, _n)
helper(0x100ac620, 'Sim_100ac620', 3, deref={2: 0x14})
for _a, _k in ((0x10009408, 1), (0x1000716a, 0), (0x1000711c, 0), (0x1000711c, 1)):
    MASK[(_a, _k)] = 0xffff
ACTIVE = tuple(a for a, _, _ in STUBS) + (0x100ac620,)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_four(uc, k, raw, eax):
    for i in range(4):
        uc.mem_write(raw[3 + i], struct.pack('<I', ST['q'][i]))


def e_three(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[1 + i], struct.pack('<I', ST['q'][i]))


def e_light(uc, k, raw, eax):
    w = struct.unpack('<4I', bytes(uc.mem_read(raw[2], 16))) + (ur32(uc, raw[2] + 0x2c), ur32(uc, raw[2] + 0x34))
    return ([('light_rec', w)], None)


EFFECT.update({0x100cbc6b: e_four, 0x100a9cfb: e_three, 0x100585e1: e_light})

# ---- float stubs: the hook keeps esp and jumps to "fld dword [value]; ret", so the answer comes back in st(0)
FSTUB = {0x10084a94: ('Sim_10084a94', 3, None), 0x1002d961: ('Sim_1002d961', 3, None)}
FCODE = 0x00F00C00
FVAL = 0x00AFF100


def _fstub_cb(m, addr, slot):
    def cb(uc, address, size, ud):
        if ST.get('flog') is not m.api_log:
            ST['flog'] = m.api_log
            ST['fcount'] = {}
        esp = uc.reg_read(UC_X86_REG_ESP)
        m.api_log.append(('fstub_%x' % addr, tuple(ur32(uc, esp + 4 + 4 * i) for i in range(3))))
        k = ST['fcount'].get(addr, 0)
        ST['fcount'][addr] = k + 1
        vals = ST['fvals'][addr]
        m.uc.mem_write(FVAL + 4 * slot, struct.pack('<I', vals[k % len(vals)]))
        uc.reg_write(UC_X86_REG_EIP, FCODE + 0x10 * slot)
    return cb


def fstubs(ctx):
    m = ctx.m
    if getattr(m, '_fstub', False):
        return
    m._fstub = True
    for slot, (addr, (cname, n, nb)) in enumerate(sorted(FSTUB.items())):
        m.raw.mem_write(FCODE + 0x10 * slot, b'\xd9\x05' + struct.pack('<I', FVAL + 4 * slot) + b'\xc3')
        cb = _fstub_cb(m, addr, slot)
        m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
        m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[cname], m.syms[cname])



def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    ST['q'] = [fb(r.uniform(-200, 200)) for _ in range(4)]
    ST['fvals'] = {0x10084a94: [fb(r.uniform(0, 400)) for _ in range(4)], 0x1002d961: [fb(r.uniform(-10, 10))]}
    rets(0x10058575, r.choice([-1 & 0xffffffff, 3]))
    rets(0x100ac21a, r.choice([-1 & 0xffffffff, 2]))
    rets(0x100096ee, r.choice([0, 0x7000]))
    rets(0x1009c929, r.randrange(0, 100))
    rets(0x100448ff, r.choice([0, 1]))
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    u = newbuf(ctx, 0x80)
    w32(ctx, u + 0x58, *[fb(r.uniform(-300, 300)) for _ in range(3)])
    me = newbuf(ctx, 0x10)
    w32(ctx, me, u)
    pe = newbuf(ctx, 0x40)
    w32(ctx, pe + 0x28, me)
    w32(ctx, 0x109e8b40 + 4 * self_, pe)
    for a in (0x1015ef3c, 0x1015ef0c, 0x10152418, 0x10134894, 0x1015dc04):
        w32(ctx, a, r.choice([0, 1]))
    w32(ctx, 0x1015dc08, r.choice([0, newbuf(ctx, 0x10)]))
    w32(ctx, 0x1015dc80, r.randrange(0, 50))
    cam = newbuf(ctx, 0x40)
    w32(ctx, cam, *[fb(r.uniform(-300, 300)) for _ in range(3)])
    w32(ctx, cam + 0x1c, *[fb(r.uniform(-300, 300)) for _ in range(3)])
    w16(ctx, cam + 0x28, r.getrandbits(16))
    w16(ctx, cam + 0x2a, r.choice([5, 0x100, 0x20, 0x8000]))
    w32(ctx, 0x1014cae0, cam)
    code = r.choice(list(range(0, 0x4e)) + [36, 11, 62, 68, 69, 5, 59, 67, 1, 4] * 3)
    hi = r.choice([0, 0x100, 0x400, 0x2000, 0x1000, 0x200, 0x4100, 0x4400, 0x100 | 0x400])
    for c in set([code, 0x4b, 0x4a, 0x2f, 0x34, 0x2a, 0x2c, 0x32, 9, 0x12]):
        if 0 <= c < 0x4d:
            fx = 0x10147b80 + c * 0x3c
            w32(ctx, fx, r.choice([0x7100, 0]))
            w32(ctx, fx + 4, r.choice([-1 & 0xffffffff, 2]))
            w32(ctx, fx + 8, r.choice([0, 1]))
            w32(ctx, fx + 0xc, r.choice([-1 & 0xffffffff, 50]))
            w32(ctx, fx + 0x10, r.choice([0, 0, 1]))
            w32(ctx, fx + 0x14, r.choice([0, 1, 1]))
            w16(ctx, fx + 0x1c, r.getrandbits(16))
            w32(ctx, fx + 0x28, r.choice([0, 1, 2, 3]))
    for i in range(0x100):
        e = 0x109a5a40 + i * 0x2c
        w32(ctx, e, r.choice([0, newbuf(ctx, 0x90)]) if r.random() < 0.05 else 0)
        w32(ctx, e + 0x18, 1 if r.random() < 0.5 else 0)
        w32(ctx, e + 0x1c, r.choice([-1 & 0xffffffff, 3]))
        w32(ctx, e + 0x24, r.choice([code & 0xff, 99]) if r.random() < 0.1 else 99)
        w32(ctx, e + 8, *[fb(r.uniform(-300, 300)) for _ in range(3)])
    node = newbuf(ctx, 0x10)
    w16(ctx, node + 2, r.choice([0x100, 0x500, 0]))
    args = [r.randrange(0, 8), r.choice([0, node]), (code | hi) if r.random() < 0.97 else 0x80000000]
    args += [fb(r.uniform(-300, 300)) for _ in range(3)] + [r.getrandbits(32) for _ in range(6)]
    return args


SPECS = [dict(name='Sim_100246b3', orig=0x100246b3, c='Sim_100246b3', gen=g, void=True, trials=6000)]
