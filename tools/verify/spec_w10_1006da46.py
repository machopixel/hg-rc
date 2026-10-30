# Round 10: 0x1006da46 (reset a unit for a (re)start). Stubbed on both sides: Sim_1009d8b9, Sim_1002bca0,
# Sim_10047b76 (the double rate logged as its two words), Sim_100a9d7c, Sim_100aa799, Sim_100a9ca3, Sim_100a9cfb,
# Sim_1002fd4c, Sim_1010d240, Sim_1006d911, Sim_100ac21a, Sim_1004f0d6, Sim_100aa454, Sim_10011093 and Sim_1002d961 as a
# float stub answering in st(0). Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w10_reset.c'])
STUBS = ((0x1009d8b9, 'Sim_1009d8b9', 1), (0x1002bca0, 'Sim_1002bca0', 0), (0x10047b76, 'Sim_10047b76', 5),
         (0x100a9d7c, 'Sim_100a9d7c', 4), (0x100aa799, 'Sim_100aa799', 1), (0x100a9ca3, 'Sim_100a9ca3', 4),
         (0x100a9cfb, 'Sim_100a9cfb', 4), (0x1002fd4c, 'Sim_1002fd4c', 7), (0x1010d240, 'Sim_1010d240', 3),
         (0x1006d911, 'Sim_1006d911', 1), (0x100ac21a, 'Sim_100ac21a', 2), (0x1004f0d6, 'Sim_1004f0d6', 1),
         (0x100aa454, 'Sim_100aa454', 1), (0x10011093, 'Sim_10011093', 1))
for _a, _c, _n in STUBS:
    helper(_a, _c, _n)
ACTIVE = tuple(a for a, _, _ in STUBS)
# ---- float stubs: the hook keeps esp and jumps to "fld dword [value]; ret", so the answer comes back in st(0)
FSTUB = {0x1002d961: ('Sim_1002d961', 3, None)}
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


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    ST['fvals'] = {0x1002d961: [fb(r.uniform(-100, 100))]}
    rets(0x1010d240, r.choice([0, 1]))
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    for i in range(8):
        w32(ctx, 0x109e8b40 + 4 * i, newbuf(ctx, 0x20))
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    w32(ctx, 0x1017b8d4, r.choice([0, 1]))
    w32(ctx, 0x1013abc4, fb(r.choice([1.0, 2.0, 0.5])))
    unit = newbuf(ctx, 0x180)
    wb(ctx, unit, bytes(r.getrandbits(8) for _ in range(0x180)))
    w32(ctx, unit, r.choice([1, 9, 0xa, 8, 2]))
    w32(ctx, unit + 4, r.choice([self_, r.randrange(0, 8)]))
    w32(ctx, unit + 0xc, r.choice([self_, r.randrange(0, 8)]))
    w32(ctx, unit + 0x14, r.choice([3, 1, 2]))
    node = newbuf(ctx, 0x90)
    model = newbuf(ctx, 0x70)
    w32(ctx, node + 0x8c, model)
    w32(ctx, unit + 0x48, node)
    w32(ctx, unit + 0x54, newbuf(ctx, 0x50))
    rec = newbuf(ctx, 0xc60)
    wb(ctx, rec, bytes(r.getrandbits(8) for _ in range(0xc60)))
    w32(ctx, rec, unit)
    w32(ctx, rec + 0x4d4, fb(r.uniform(0, 100)))
    obj = newbuf(ctx, 0x2a0)
    w32(ctx, obj, r.choice([0xa, 4, 1]))
    w32(ctx, obj + 4, r.randrange(0, 8))
    w32(ctx, 0x1014c99c, r.choice([r32(ctx, obj + 4), 99]))
    w32(ctx, obj + 0x28, rec if r.random() < 0.97 else 0)
    w32(ctx, obj + 0x48, node)
    return [obj]


SPECS = [dict(name='Sim_1006da46', orig=0x1006da46, c='Sim_1006da46', gen=g, void=True, trials=3000)]
