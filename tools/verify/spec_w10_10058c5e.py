# Round 10: 0x10058c5e (polygon vertex lighting). Stubbed on both sides: Sim_100a648c (light of a vertex, its six raw
# words logged), Sim_1005897b (does a light source touch the polygon) and the fog factor 0x10058ac2 as a float stub
# answering in st(0). The CRT fabs runs for real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1005/w10_misc.c'])
helper(0x100a648c, None, 6)          # the C calls it through its address (SIM_CALL_100a648c)
helper(0x1005897b, 'Sim_1005897b', 3)

# ---- float stubs: the hook keeps esp and jumps to "fld dword [value]; ret", so the answer comes back in st(0)
FSTUB = {0x10058ac2: ('Render_FogFactor_10058ac2', 2, None)}
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
        if cname in m.syms:        # (the C calls 0x10058ac2 through its address)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[cname], m.syms[cname])



def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def fl(r, c, sc):
    return fb(c + r.uniform(-sc, sc)) if r.random() < 0.97 else rfloat(r)


def g(ctx):
    r = begin(ctx, 0x100a648c, 0x1005897b)
    fstubs(ctx)
    rets(0x100a648c, r.choice([0, 5, 40, 200, -7 & 0xffffffff]), r.choice([0, 30, 300]), r.choice([1, 90]))
    rets(0x1005897b, r.choice([0, 1]), r.choice([0, 1]))
    ST['fvals'] = {0x10058ac2: [fb(r.uniform(0, 1)), fb(r.choice([0.0, 1.0, 0.5]))]}
    nv = r.randrange(3, 8)
    verts = newbuf(ctx, 0x58 * nv)
    for k in range(nv):
        w32(ctx, verts + 0x58 * k, fl(r, 0, 500), fl(r, 0, 500), fl(r, 0, 500))
    poly = newbuf(ctx, 0x20)
    w32(ctx, poly + 4, 8)
    w8(ctx, poly + 8, r.randrange(0, nv))
    n = r.randrange(0, 6)
    w32(ctx, 0x101c9ea8, n)
    lst = newbuf(ctx, 4 * max(n, 1))
    for k in range(n):
        v = newbuf(ctx, 0x58)
        w32(ctx, v + 8, fl(r, 300, 300))
        w32(ctx, v + 0xc, *[fl(r, 0, 500) for _ in range(3)])
        w8(ctx, v + 0x29, r.choice([0, 0, 2, r.getrandbits(8)]))
        w32(ctx, v + 0x2c, fl(r, 0, 50))
        w32(ctx, v + 0x30, *[fl(r, 0, 1) for _ in range(3)])
        w32(ctx, v + 0x3c, r.choice([0, 0x7f, 0x80, 0x100]))
        w32(ctx, lst + 4 * k, v)
    cam = newbuf(ctx, 0x20)
    w32(ctx, cam, *[fl(r, 0, 500) for _ in range(3)])
    w32(ctx, 0x1014cae0, cam)
    w32(ctx, 0x1014cb1c, r.choice([3, 1, 0]))
    for a in (0x101668d8, 0x109afdcc, 0x10143fd4, 0x1017b940, 0x1017d108, 0x10143fd8, 0x1014cb38):
        w32(ctx, a, r.choice([0, 1]))
    for a in (0x101668c0, 0x101668c4, 0x101668c8, 0x109b16c4, 0x109b16c8, 0x109b16c0, 0x10143fc0, 0x101668b8,
              0x101668bc):
        w32(ctx, a, fl(r, 0, 500))
    w32(ctx, 0x101668b4, r.randrange(0, 0x40))
    w32(ctx, 0x10143fbc, r.choice([0, 1, 4, 16, 100]))
    w32(ctx, 0x1014cb2c, r.choice([0, 0, 200, -1 & 0xffffffff, 600]))
    w32(ctx, 0x10143fcc, fb(r.choice([128.0, 255.0, 64.5])))
    srcs = 0
    for k in range(r.randrange(0, 3)):
        sp = newbuf(ctx, 0x30)
        w32(ctx, sp, srcs)
        if r.random() < 0.4:
            c = poly + 8
            ci = r32(ctx, poly + 8) & 0xff
            w32(ctx, sp + 0x20, r32(ctx, verts + 0x58 * ci), r32(ctx, verts + 0x58 * ci + 4), r32(ctx, verts + 0x58 * ci + 8))
        else:
            w32(ctx, sp + 0x20, *[fl(r, 0, 500) for _ in range(3)])
        srcs = sp
    w32(ctx, 0x101668d0, srcs)
    if r.random() < 0.03:
        poly = 0
    return [poly, verts, lst, r.choice([1024, 512, 2048, 300, 0])]


SPECS = [dict(name='Sim_10058c5e', orig=0x10058c5e, c='Sim_10058c5e', gen=g, void=True, trials=5000)]
