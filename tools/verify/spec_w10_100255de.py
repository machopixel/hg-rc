# Round 10: 0x100255de (blast damage). Stubbed on both sides: Sim_1006c161 (with flag 0 it reads the shot record's
# +0x70..+0x7d and +0x86: those words are logged) and Sim_10084a94 as a float stub (distance). Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1002/w10_misc.c'])
helper(0x1006c161, 'Sim_1006c161', 5)


def e_hit(uc, k, raw, eax):
    return ([('shot_rec', bytes(uc.mem_read(raw[3] + 0x70, 0xe)), ur32(uc, raw[3] + 0x86))], None)


EFFECT[0x1006c161] = e_hit

# ---- float stubs: the hook keeps esp and jumps to "fld dword [value]; ret", so the answer comes back in st(0)
FSTUB = {0x10084a94: ('Sim_10084a94', 3, None)}
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
    r = begin(ctx, 0x1006c161)
    fstubs(ctx)
    ST['fvals'] = {0x10084a94: [fb(r.uniform(0, 300)) for _ in range(3)]}
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015ecd0, r.choice([0, 1]))
    w32(ctx, 0x1015dc80, r.choice([0, 16, 33, 100]))
    n = r.randrange(0, 8)
    w32(ctx, 0x1015dc64, n)
    for i in range(n):
        u = newbuf(ctx, 0x80)
        w32(ctx, u + 0xc, r.choice([self_, 9]))
        w32(ctx, u + 0x14, r.choice([3, 1]))
        w32(ctx, u + 0x58, *[fb(r.uniform(-100, 100)) for _ in range(3)])
        rec = newbuf(ctx, 0x580)
        w16(ctx, rec + 0x560, r.choice([0, 0, 0x100]))
        w32(ctx, rec + 0x53c, fb(r.uniform(0, 50)))
        w32(ctx, u + 0x28, rec)
        w32(ctx, 0x109e8b40 + 4 * i, u)
    return [r.randrange(0, 8)] + [fb(r.uniform(-100, 100)) for _ in range(3)] + [fb(r.uniform(0, 200)), fb(r.uniform(0, 5))]


SPECS = [dict(name='Sim_100255de', orig=0x100255de, c='Sim_100255de', gen=g, void=True, trials=4000)]
