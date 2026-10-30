# Round 10: 0x1000b070 (AI fire control). All callees are converted; they are stubbed on both sides with their
# arguments logged: Sim_1009c929, Sim_1000ceb7, Sim_1000bf73 / Sim_1000bba7 (write the flag), Sim_1000b9f1 (writes the
# entry), Sim_1000c749, Sim_1000ca63 (writes its float), Sim_1000c1ea (may change the heading argument), Sim_1000b01f,
# Sim_1001403a (its short argument masked to 16 bits), Sim_1000a6f0, Sim_1000c6e3, Sim_1000c172, and the float stubs
# Sim_1001448d / Sim_100144bb answering in st(0). The CRT fmod runs for real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1000/w10_misc.c'])
for _a, _c, _n in ((0x1009c929, 'Sim_1009c929', 1), (0x1000ceb7, 'Sim_1000ceb7', 1), (0x1000bf73, 'Sim_1000bf73', 2),
                   (0x1000bba7, 'Sim_1000bba7', 2), (0x1000b9f1, 'Sim_1000b9f1', 5), (0x1000c749, 'Sim_1000c749', 3),
                   (0x1000ca63, 'Sim_1000ca63', 3), (0x1000c1ea, 'Sim_1000c1ea', 4), (0x1000b01f, 'Sim_1000b01f', 4),
                   (0x1001403a, 'Sim_1001403a', 2), (0x1000a6f0, 'Sim_1000a6f0', 2), (0x1000c6e3, 'Sim_1000c6e3', 3),
                   (0x1000c172, 'Sim_1000c172', 1)):
    helper(_a, _c, _n)
MASK[(0x1001403a, 1)] = 0xffff
ACTIVE = (0x1009c929, 0x1000ceb7, 0x1000bf73, 0x1000bba7, 0x1000b9f1, 0x1000c749, 0x1000ca63, 0x1000c1ea, 0x1000b01f,
          0x1001403a, 0x1000a6f0, 0x1000c6e3, 0x1000c172)


def e_flag(uc, k, raw, eax):
    uc.mem_write(raw[1], struct.pack('<I', ST['flag']))


def e_entry(uc, k, raw, eax):
    uc.mem_write(raw[3], struct.pack('<I', ST['entry']))


def e_ca63(uc, k, raw, eax):
    if ST['ca63']:
        uc.mem_write(raw[2], struct.pack('<I', ST['ca63']))


def e_aim(uc, k, raw, eax):
    if ST['head'] is not None:
        uc.mem_write(raw[1], struct.pack('<I', ST['head']))


EFFECT.update({0x1000bf73: e_flag, 0x1000bba7: e_flag, 0x1000b9f1: e_entry, 0x1000ca63: e_ca63, 0x1000c1ea: e_aim})

# ---- float stubs: the hook keeps esp and jumps to "fld dword [value]; ret", so the answer comes back in st(0)
FSTUB = {0x1001448d: ('Sim_1001448d', 3, None), 0x100144bb: ('Sim_100144bb', 3, None)}
FCODE = 0x00F00C00
FVAL = 0x00AFF100


def _fstub_cb(m, addr, slot):
    def cb(uc, address, size, ud):
        if ST.get('flog') is not m.api_log:
            ST['flog'] = m.api_log
            ST['fcount'] = {}
        esp = uc.reg_read(UC_X86_REG_ESP)
        m.api_log.append(('fstub_%x' % addr, tuple(ur32(uc, esp + 4 + 4 * i) for i in range(FSTUB[addr][1]))))
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



def fl(r, sc):
    return struct.unpack('<I', struct.pack('<f', r.uniform(-sc, sc)))[0] if r.random() < 0.97 else rfloat(r)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    for a in ACTIVE:
        rets(a, r.choice([0, 1, 1]), r.choice([0, 1]), r.choice([0, 1, 2]))
    rets(0x1009c929, r.choice([0, 0, 1, 3]))
    rets(0x1000c749, r.choice([0, 1]), r.choice([0, 1, 1, 2]))
    ST['flag'] = r.choice([0, 1])
    ent = newbuf(ctx, 0x60)
    w32(ctx, ent + 0x54, r.randrange(0, 100))
    ST['entry'] = r.choice([0, ent])
    ST['ca63'] = r.choice([0, fl(r, 90)])
    ST['head'] = r.choice([None, fl(r, 360)])
    ST['fvals'] = {0x1001448d: [fl(r, 2), fl(r, 2)], 0x100144bb: [fl(r, 2)]}
    w32(ctx, 0x1015dc78, r.randrange(0, 10000))
    rec = newbuf(ctx, 0x600)
    w32(ctx, rec + 0x4fc, r.randrange(0, 4))
    w32(ctx, rec + 0x5fb, r.choice([r32(ctx, rec + 0x4fc), 7]))
    wl = newbuf(ctx, 0x128 * 4)
    for k in range(4):
        w32(ctx, wl + 0x128 * k + 4, r.randrange(0, 40))
    w32(ctx, rec + 0x84, wl)
    w32(ctx, rec + 0x1c, fl(r, 360))
    ctl = newbuf(ctx, 0x30)
    obj = newbuf(ctx, 0x200)
    w32(ctx, obj, r.choice([1, 9, 2, 5]))
    w32(ctx, obj + 0x28, rec)
    w32(ctx, obj + 0x54, ctl)
    w32(ctx, obj + 0x1be, r.choice([0, r32(ctx, 0x1015dc78), r32(ctx, 0x1015dc78) + 5, 0x7fffffff]))
    w8(ctx, obj + 0x1b6, r.getrandbits(8))
    w32(ctx, obj + 0x1c2, r.choice([0, 1]))
    for off in (0x64, 0x74, 0xd4, 0xec):
        w32(ctx, obj + off, fl(r, 360))
    tid = r.randrange(0, 8)
    w16(ctx, obj + 0x1aa, r.choice([0x200 | tid, 0x200 | tid, 0x100 | tid, r.getrandbits(16)]))
    for i in range(8):
        e = newbuf(ctx, 0x60)
        c = newbuf(ctx, 0x10)
        w32(ctx, c + 8, r.choice([r32(ctx, 0x10111060), fl(r, 5)]))
        w32(ctx, e + 0x54, c)
        w32(ctx, 0x109e8b40 + 4 * i, e)
    return [obj, fl(r, 360)]


SPECS = [dict(name='Sim_1000b070', orig=0x1000b070, c='Sim_1000b070', gen=g, trials=5000)]
