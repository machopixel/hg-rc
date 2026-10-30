# Round 10: 0x10044d0b (free camera step). Stubbed on both sides: Sim_10044b2c (writes its six floats), Sim_10044909,
# Sim_100cbc6b (writes its four angles; dx, dy, dz logged) and, as float stubs that answer in st(0), Sim_10084766
# (its float argument logged) and Sim_1004487c (the three camera floats logged). The CRT fmod / fabs and the
# interpolators Sim_10047e1a / Sim_10047cf2 / Sim_10047bcd run for real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1004/w10_misc.c'])
helper(0x10044b2c, 'Sim_10044b2c', 6)
helper(0x10044909, 'Sim_10044909', 1)
helper(0x100cbc6b, 'Sim_100cbc6b', 7)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_six(uc, k, raw, eax):
    for i in range(6):
        uc.mem_write(raw[i], struct.pack('<I', ST['six'][i]))


def e_ang(uc, k, raw, eax):
    for i in range(4):
        uc.mem_write(raw[3 + i], struct.pack('<I', ST['ang'][i]))


EFFECT.update({0x10044b2c: e_six, 0x100cbc6b: e_ang})

# ---- float stubs: the hook keeps esp and jumps to "fld dword [value]; ret", so the answer comes back in st(0)
FSTUB = {0x10084766: ('Sim_10084766', 1, None), 0x1004487c: ('Sim_1004487c', 1, 12)}
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


def fl(r, sc):
    return fb(r.uniform(-sc, sc)) if r.random() < 0.97 else rfloat(r)


def interp(ctx, r, a, period):
    w32(ctx, a, r.randrange(0, 2000))
    w32(ctx, a + 4, fl(r, 400), fl(r, 400), r.choice([fb(r.uniform(1, 100)), 0]))
    if period:
        w32(ctx, a + 0x10, fb(360.0))


def g(ctx):
    r = begin(ctx, 0x10044b2c, 0x10044909, 0x100cbc6b)
    fstubs(ctx)
    ST['six'] = [fl(r, 1000) for _ in range(6)]
    ST['ang'] = [fl(r, 360) for _ in range(4)]
    ST['fvals'] = {0x10084766: [fb(r.uniform(-1, 1)), fb(r.uniform(-1, 1))],
                   0x1004487c: [r.choice([fl(r, 1000), fb(-1e9), fb(1e9)])]}
    ent = newbuf(ctx, 0x40)
    rec = newbuf(ctx, 0x600)
    w32(ctx, ent + 0x28, rec)
    w32(ctx, 0x109e8b40, ent)
    w32(ctx, rec + 0x53c, fl(r, 100))
    w32(ctx, rec + 0x520, fl(r, 100))
    w16(ctx, rec + 0x560, r.getrandbits(16))
    cam = newbuf(ctx, 0x20)
    w32(ctx, cam, *[fl(r, 1000) for _ in range(6)])
    w32(ctx, 0x1014cae0, cam)
    if r.random() < 0.3:      # target x on (or near) the camera's x: the fabs branch
        ST['six'][3] = r.choice([r32(ctx, cam), fb(struct.unpack('<f', struct.pack('<I', r32(ctx, cam)))[0] + r.uniform(-2, 2))])
    w32(ctx, 0x1013aed4, r.choice([0, 1, 1, 2]))
    w32(ctx, 0x1015eeec, r.choice([r32(ctx, 0x10111584), r32(ctx, 0x10111584), fl(r, 100)]))
    for a in (0x1015eef0, 0x1015eef4, 0x1015eef8):
        w32(ctx, a, fl(r, 100))
    w32(ctx, 0x1015dc78, r.randrange(0, 5000))
    w32(ctx, 0x1013aec8, fl(r, 720))
    for a, per in ((0x109b9aa0, 1), (0x109b9ac0, 1), (0x109b9bb0, 0), (0x109b9a90, 0), (0x109b9af0, 0)):
        interp(ctx, r, a, per)
    for a in (0x109b9a84, 0x109b9ad4, 0x109b9b8c):
        w32(ctx, a, fl(r, 100))
    return [fl(r, 20), fl(r, 20), fl(r, 360), fl(r, 360)]


SPECS = [dict(name='Sim_10044d0b', orig=0x10044d0b, c='Sim_10044d0b', gen=g, void=True, trials=4000)]
