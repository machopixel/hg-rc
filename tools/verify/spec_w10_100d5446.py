# Round 10: 0x100d5446 (launch a missile). Stubbed on both sides: Sim_10045f3b, Sim_100d6d6a, Sim_100d6ea7 (writes the
# three direction floats), Sim_10084cee (writes three floats), Sim_1000716a, Sim_1000711c (word arguments masked), the
# original aim routine 0x100d71b9, Sim_100aa39f, Sim_100aa406, Sim_100aa799, Sim_100a9cfb (writes three floats),
# Sim_10058575, Sim_100585e1 (with mask 7 it reads the light record's +0..+0xc and +0x2c: those words are logged),
# Sim_100ac21a, Sim_100d5c9b, Sim_100203cf, and Sim_100c97c0 as a float stub. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100d/w10_missile.c'])
STUBS = ((0x10045f3b, 'Sim_10045f3b', 3), (0x100d6d6a, 'Sim_100d6d6a', 1), (0x100d6ea7, 'Sim_100d6ea7', 5),
         (0x10084cee, 'Sim_10084cee', 3), (0x1000716a, 'Sim_1000716a', 2), (0x1000711c, 'Sim_1000711c', 2),
         (0x100d71b9, None, 8), (0x100aa39f, 'Sim_100aa39f', 1), (0x100aa406, 'Sim_100aa406', 1),
         (0x100aa799, 'Sim_100aa799', 1), (0x100a9cfb, 'Sim_100a9cfb', 4), (0x10058575, 'Sim_10058575', 0),
         (0x100585e1, 'Sim_100585e1', 3), (0x100ac21a, 'Sim_100ac21a', 2), (0x100d5c9b, 'Sim_100d5c9b', 1),
         (0x100203cf, 'Sim_100203cf', 2))
for _a, _c, _n in STUBS:
    helper(_a, _c, _n)
MASK[(0x1000711c, 0)] = 0xffff
MASK[(0x1000711c, 1)] = 0xffff
# Controls round: the C makes 1.2's force-feedback request Sim_100382fb(kind, 0, 0, 0), only while the library flag
# [0x1015ef3c] is set, where 1.0 called Sim_10045f3b(kind, 0x2d, 0x32) always. FFC has no 1.0 address (the stub of
# the C function only); lm_ff maps the original's call to that form. The flag is always set here; the v12 spec covers
# it clear.
FFC = 0x00F00F00
helper(FFC, 'Sim_100382fb', 4)
ACTIVE = tuple(a for a, _, _ in STUBS) + (FFC,)
FF = {'flag': 0}


def lm_ff(log_o, log_c):
    out = []
    for e in log_o:
        if e[0] == 'stub_10045f3b' and len(e[1]) == 3:
            if not FF['flag']:
                continue
            e = ('stub_%x' % FFC, (e[1][0], 0, 0, 0))
        out.append(e)
    return out, log_c


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_dir(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[1 + i], struct.pack('<I', ST['d'][(k * 3 + i) % 7]))


def e_norm(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[i], struct.pack('<I', ST['d'][(k + i + 3) % 7]))


def e_light(uc, k, raw, eax):
    words = struct.unpack('<4I', bytes(uc.mem_read(raw[2], 16))) + (ur32(uc, raw[2] + 0x2c),)
    return ([('light_rec', words)], None)


EFFECT.update({0x100d6ea7: e_dir, 0x10084cee: e_norm, 0x100a9cfb: e_dir, 0x100585e1: e_light})

# ---- float stubs: the hook keeps esp and jumps to "fld dword [value]; ret", so the answer comes back in st(0)
FSTUB = {0x100c97c0: ('Sim_100c97c0', 2, None)}
FCODE = 0x00F00C00
FVAL = 0x00AFF100


def _fstub_cb(m, addr, slot):
    def cb(uc, address, size, ud):
        if ST.get('flog') is not m.api_log:
            ST['flog'] = m.api_log
            ST['fcount'] = {}
        esp = uc.reg_read(UC_X86_REG_ESP)
        m.api_log.append(('fstub_%x' % addr, tuple(ur32(uc, esp + 4 + 4 * i) for i in range(2))))
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
    FF['flag'] = 1      # flag clear: 1.0 still calls, and the stubs' answers (picked by log position) would shift
    w32(ctx, 0x1015ef3c, FF['flag'])
    ST['d'] = [fb(r.uniform(-2, 2)) for _ in range(7)]
    ST['fvals'] = {0x100c97c0: [fb(r.uniform(0, 3000))]}
    rets(0x100d5c9b, r.choice([0, 1]))
    rets(0x10058575, r.randrange(0, 20))
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    for i in range(8):
        e = newbuf(ctx, 0x40)
        w32(ctx, e + 8, r.randrange(0, 3))
        mr = newbuf(ctx, 0xc60)
        w32(ctx, mr + 0xbcb, r.getrandbits(32))
        w32(ctx, mr + 0xc49, r.choice([0, 0, 50, 2000]))
        w32(ctx, e + 0x28, mr)
        w32(ctx, 0x109e8b40 + 4 * i, e)
    rp = newbuf(ctx, 4)
    w8(ctx, rp, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b8c4, rp)
    w32(ctx, 0x1015dc80, r.randrange(0, 60))
    w32(ctx, 0x1017e67c, r.choice([0, 1]))
    kind = r.choice([0x3e, 0x3f, 0x41, 0x45, 5])
    nfree = r.choice([0, 1, 3])
    for i in range(0x200):
        p = 0x1096fc80 + i * 0x156
        w32(ctx, p, kind if r.random() < 0.3 else 0)
        w32(ctx, p + 0x48, 1 if r.random() < 0.6 else 0)
        w32(ctx, p + 0x3c, newbuf(ctx, 4) if r.random() < 0.8 else 0)
    owner = newbuf(ctx, 0x60)
    w32(ctx, owner + 4, r.choice([self_, self_, r.randrange(0, 8)]))
    ctl = newbuf(ctx, 0x20)
    w32(ctx, ctl + 8, fb(r.choice([0.0, 0.5, 1.0])))
    w32(ctx, owner + 0x54, ctl)
    rec = newbuf(ctx, 0xc50)
    w32(ctx, rec, owner)
    w32(ctx, rec + 0xc3d, r.choice([0, 0, 0, 1]))
    w32(ctx, rec + 0xbd3, r.getrandbits(32))
    w32(ctx, rec + 0xbd7, r.randrange(0, 10))
    w16(ctx, rec + 0x562, r.getrandbits(16))
    for k in range(8):
        w32(ctx, rec + 0x498 + 4 * k, r.choice([0, 1, 1, 1]))
    obj = newbuf(ctx, 0x200)
    w32(ctx, obj + 4, r.choice([self_, self_, r.randrange(0, 8)]))
    w32(ctx, obj + 0x28, rec)
    w16(ctx, obj + 0x1aa, r.choice([self_ | 0x200, r.getrandbits(16)]))
    weapon = newbuf(ctx, 0xd0)
    wb(ctx, weapon, bytes(r.getrandbits(8) for _ in range(0xd0)))
    w32(ctx, weapon + 0x10, kind)
    w32(ctx, weapon + 0x24, r.choice([0, 1, 1]))
    w32(ctx, weapon + 0x3c, fb(r.uniform(0, 500)))
    w32(ctx, weapon + 0x86, r.choice([0x21, 0x20, 1, 0]))
    w16(ctx, weapon + 0x74, r.randrange(0, 8))
    slot = newbuf(ctx, 0x120)
    wb(ctx, slot, bytes(r.getrandbits(8) for _ in range(0x120)))
    w32(ctx, slot + 0x20, r.choice([-1, -2, 1, 2, 5, 0, 30]) & 0xffffffff)
    w32(ctx, slot + 0x24, r.randrange(0, 8))
    clip = newbuf(ctx, 0x10)
    w16(ctx, clip + 2, r.choice([0, 1, 2, 5, 0xffff]))
    w32(ctx, slot + 0x110, clip)
    n = r.randrange(0, 4)
    w32(ctx, slot + 0x108, n)
    w32(ctx, slot + 0x38, r.choice([n + 1, n + 2]))
    w32(ctx, slot + 0x11c, r.randrange(0, 8))
    if r.random() < 0.03:
        obj = 0
    return [obj, weapon, slot]


SPECS = [dict(name='Sim_100d5446', orig=0x100d5446, c='Sim_100d5446', gen=g, trials=4000, log_map=lm_ff)]
