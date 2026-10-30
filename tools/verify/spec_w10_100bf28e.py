# Round 10: 0x100bf28e (start an AI order). Stubbed on both sides: Sim_1009c929 (random), Sim_100c274a, Sim_100c00c7,
# Sim_100157ca, Sim_10033c42 (a path record or 0), Sim_1003359f, and Sim_100c3725 as a float stub. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100b/w10_misc.c'])
for _a, _c, _n in ((0x1009c929, 'Sim_1009c929', 1), (0x100c274a, 'Sim_100c274a', 2), (0x100c00c7, 'Sim_100c00c7', 4),
                   (0x100157ca, 'Sim_100157ca', 1), (0x10033c42, 'Sim_10033c42', 1), (0x1003359f, 'Sim_1003359f', 2)):
    helper(_a, _c, _n)
MASK[(0x100c274a, 1)] = 0xff
MASK[(0x100c00c7, 2)] = 0xffff
ACTIVE = (0x1009c929, 0x100c274a, 0x100c00c7, 0x100157ca, 0x10033c42, 0x1003359f)

# ---- float stubs: the hook keeps esp and jumps to "fld dword [value]; ret", so the answer comes back in st(0)
FSTUB = {0x100c3725: ('Sim_100c3725', 1, None)}
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



def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    ST['fvals'] = {0x100c3725: [fb(1.5)]}
    rets(0x1009c929, r.choice([0, 1, 3, 7]), r.choice([0, 1]))
    rets(0x10033c42, r.choice([0, newbuf(ctx, 0x40)]))
    w32(ctx, 0x1015dc78, r.randrange(0, 100000))
    obj = newbuf(ctx, 0x280)
    wb(ctx, obj, bytes(r.getrandbits(8) for _ in range(0x280)))
    w16(ctx, obj + 0x1f8, r.choice(list(range(0x1c)) + [0x1c, 0xffff, 0x8000]))
    w32(ctx, obj, r.choice([8, 1, 2]))
    lim = struct.unpack('<f', struct.pack('<I', r32(ctx, 0x1012bb14)))[0]
    w32(ctx, obj + 0xd4, fb(lim * r.uniform(0, 2)) if r.random() < 0.95 else rfloat(r))
    rec = newbuf(ctx, 0x500)
    w32(ctx, rec + 0x4f4, r.choice([0, 1]))
    w32(ctx, obj + 0x28, rec)
    w32(ctx, obj + 0x54, newbuf(ctx, 0x60))
    w16(ctx, obj + 0x1fa, r.choice([0xa, 0xa, 3]))
    w32(ctx, obj + 0x84, r.choice([-1, -1, 5]) & 0xffffffff)
    w32(ctx, obj + 0x218, r.choice([0, 2, 1, 5]))
    w32(ctx, obj + 0x23a, r.choice([-1, 0, 4]) & 0xffffffff)
    return [obj]


SPECS = [dict(name='Sim_100bf28e', orig=0x100bf28e, c='Sim_100bf28e', gen=g, void=True, trials=4000)]
