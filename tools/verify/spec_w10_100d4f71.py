# Round 10: 0x100d4f71 (fire a shot). Stubbed on both sides: Sim_100d6d6a, Sim_100d6f16 (fills the 0x38 bytes of the
# segment it builds), Sim_100a9cfb (writes three floats), the trail part routines Sim_1009201f / Sim_10091085 /
# Sim_10092956, and Sim_100d6e99 as a float stub answering in st(0). Sim_1009cd36 (and Sim_1009cbfe) run for real on
# the segment. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100d/w10_misc.c'])
helper(0x100d6d6a, 'Sim_100d6d6a', 1)
helper(0x100d6f16, 'Sim_100d6f16', 2)
helper(0x100a9cfb, 'Sim_100a9cfb', 4)
helper(0x1009201f, 'Sim_1009201f', 4)
helper(0x10091085, 'Sim_10091085', 3)
helper(0x10092956, 'Sim_10092956', 3)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_seg(uc, k, raw, eax):
    uc.mem_write(raw[1], struct.pack('<14I', *ST['seg']))


def e_three(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[1 + i], struct.pack('<I', ST['seg'][i]))


EFFECT.update({0x100d6f16: e_seg, 0x100a9cfb: e_three})

# ---- float stubs: the hook keeps esp and jumps to "fld dword [value]; ret", so the answer comes back in st(0)
FSTUB = {0x100d6e99: ('Sim_100d6e99', 1, None)}
FCODE = 0x00F00C00
FVAL = 0x00AFF100


def _fstub_cb(m, addr, slot):
    def cb(uc, address, size, ud):
        if ST.get('flog') is not m.api_log:
            ST['flog'] = m.api_log
            ST['fcount'] = {}
        esp = uc.reg_read(UC_X86_REG_ESP)
        a0 = ur32(uc, esp + 4)
        nbytes = FSTUB[addr][2]
        m.api_log.append(('fstub_%x' % addr, ('deref', bytes(uc.mem_read(a0, nbytes))) if nbytes else a0))
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
    r = begin(ctx, 0x100d6d6a, 0x100d6f16, 0x100a9cfb, 0x1009201f, 0x10091085, 0x10092956)
    fstubs(ctx)
    ST['seg'] = [fb(r.uniform(-500, 500)) for _ in range(13)] + [r.choice([0, 1, 1])]
    ST['fvals'] = {0x100d6e99: [r.choice([fb(r.uniform(0, 300)), fb(10.0), 0])]}
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    for i in range(8):
        e = newbuf(ctx, 0x40)
        w32(ctx, e + 8, r.randrange(0, 3))
        w32(ctx, e + 0x28, newbuf(ctx, 0x10))
        w32(ctx, 0x109e8b40 + 4 * i, e)
    rp = newbuf(ctx, 4)
    w8(ctx, rp, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b8c4, rp)
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    nfree = r.choice([0, 1, 3, 20])
    for i in range(20):
        w32(ctx, 0x109a3f80 + i * 0x156 + 0x48, 0 if (i >= 20 - nfree and r.random() < 0.7) else 1)
    owner = newbuf(ctx, 8)
    w32(ctx, owner + 4, r.choice([self_, self_, r.randrange(0, 8)]))
    rec = newbuf(ctx, 0xc50)
    w32(ctx, rec, owner)
    w32(ctx, rec + 0xc3d, r.choice([0, 0, 0, 1]))
    w32(ctx, rec + 0xbd3, r.getrandbits(32))
    for k in range(8):
        w32(ctx, rec + 0x498 + 4 * k, r.getrandbits(32))
    obj = newbuf(ctx, 0x40)
    w32(ctx, obj + 4, r.choice([self_, self_, r.randrange(0, 8)]))
    w32(ctx, obj + 0xc, r.choice([self_, r.randrange(0, 8)]))
    w32(ctx, obj + 0x14, r.choice([3, 3, 1, 2]))
    w32(ctx, obj + 0x28, rec)
    weapon = newbuf(ctx, 0xd0)
    wb(ctx, weapon, bytes(r.getrandbits(8) for _ in range(0xd0)))
    w32(ctx, weapon + 0x10, r.choice([6, 4, 1, 2]))
    slot = newbuf(ctx, 0x120)
    wb(ctx, slot, bytes(r.getrandbits(8) for _ in range(0x120)))
    w32(ctx, slot + 0x20, r.choice([-1, -2, 1, 2, 5, 0, 30]) & 0xffffffff)
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


SPECS = [dict(name='Sim_100d4f71', orig=0x100d4f71, c='Sim_100d4f71', gen=g, trials=4000)]
