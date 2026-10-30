# Round 10: 0x100225c7 (one frame of a shot). Every callee is stubbed on both sides: the node position Sim_100a9cfb
# (writes its three floats), the segment helpers Sim_1009cab0 / Sim_1009cc82 / Sim_1009cbfe / Sim_1009cf67 (the whole
# 0x38-byte segment logged; they write new bytes into it), the hit test Sim_1002dda9 (writes the hit record), the
# steering Sim_100241ed (its three velocity floats logged and rewritten), Sim_100585e1 (the three floats it reads
# logged), and as float stubs answering in st(0) Sim_10084a94, Sim_1008486e, Sim_1002d961 and the CRT fmod. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1002/w10_shot.c'])
SEG = 0x38
for _a, _c, _n in ((0x100a9cfb, 'Sim_100a9cfb', 4), (0x1009cab0, 'Sim_1009cab0', 7), (0x1002dda9, 'Sim_1002dda9', 3),
                   (0x100a9b71, 'Sim_100a9b71', 1), (0x100b53bb, 'Sim_100b53bb', 1), (0x1009c929, 'Sim_1009c929', 1),
                   (0x100203cf, 'Sim_100203cf', 2), (0x10015630, 'Sim_10015630', 4), (0x10052be7, 'Sim_10052be7', 1),
                   (0x1006c161, 'Sim_1006c161', 5), (0x10061f8e, 'Sim_10061f8e', 3), (0x10015afd, 'Sim_10015afd', 2),
                   (0x10024548, 'Sim_10024548', 9), (0x100241ed, 'Sim_100241ed', 7), (0x100a9d47, 'Sim_100a9d47', 4),
                   (0x100aa799, 'Sim_100aa799', 1), (0x100585e1, 'Sim_100585e1', 3), (0x10096ac3, 'Sim_10096ac3', 6),
                   (0x10095e0a, 'Sim_10095e0a', 6), (0x10094fcb, 'Sim_10094fcb', 6)):
    helper(_a, _c, _n)
for _a in (0x1009cc82, 0x1009cbfe):
    helper(_a, 'Sim_%x' % _a, 1, deref={0: SEG})
helper(0x1009cf67, 'Sim_1009cf67', 2, deref={0: SEG})
DEREF[0x1002dda9] = {0: SEG}
DEREF[0x100241ed] = {4: 4, 5: 4, 6: 4}
MASK[(0x10015630, 1)] = 0xffff
MASK[(0x10015630, 2)] = 0xffff
ACTIVE = tuple(HELPERS)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_pos(uc, k, raw, eax):
    v = ST['pos'][k % len(ST['pos'])]
    for i in range(3):
        uc.mem_write(raw[1 + i], struct.pack('<I', v[i]))


def e_seg(uc, k, raw, eax):
    uc.mem_write(raw[0], ST['segb'][k % len(ST['segb'])])


def e_dda9(uc, k, raw, eax):
    mode = ST['dda9']
    if mode != 'keep':
        uc.mem_write(raw[1], struct.pack('<I', mode))


def e_steer(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[4 + i], struct.pack('<I', ST['steer'][i]))


def e_585(uc, k, raw, eax):
    return [('src_585e1', bytes(uc.mem_read(raw[2] + 4, 12)))], None


EFFECT.update({0x100a9cfb: e_pos, 0x1009cab0: e_seg, 0x1009cc82: e_seg, 0x1009cbfe: e_seg, 0x1009cf67: e_seg,
               0x1002dda9: e_dda9, 0x100241ed: e_steer, 0x100585e1: e_585})

# ---- float stubs: the hook keeps esp and jumps to "fld dword [value]; ret", so the answer comes back in st(0)
FSTUB = {0x10084a94: ('Sim_10084a94', 3), 0x1008486e: ('Sim_1008486e', 2), 0x1002d961: ('Sim_1002d961', 3),
         0x100fcb30: (None, 4)}
FCODE = 0x00F00C00
FVAL = 0x00AFF100


def _fstub_cb(m, addr, slot):
    def cb(uc, address, size, ud):
        if ST.get('flog') is not m.api_log:
            ST['flog'] = m.api_log
            ST['fcount'] = {}
        esp = uc.reg_read(UC_X86_REG_ESP)
        n = FSTUB[addr][1]
        m.api_log.append(('fstub_%x' % addr, struct.unpack('<%dI' % n, bytes(uc.mem_read(esp + 4, 4 * n)))))
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
    for slot, (addr, (cname, n)) in enumerate(sorted(FSTUB.items())):
        m.raw.mem_write(FCODE + 0x10 * slot, b'\xd9\x05' + struct.pack('<I', FVAL + 4 * slot) + b'\xc3')
        cb = _fstub_cb(m, addr, slot)
        m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
        if cname is not None:
            m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[cname], m.syms[cname])


def fl(r, sc):
    return fb(r.uniform(-sc, sc)) if r.random() < 0.97 else rfloat(r)


SHOTS, SHOT = 0x1096fc80, 0x156


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015ecd0, r.choice([0, 0, 1]))
    w32(ctx, 0x1015dc80, r.choice([0, 1, r.randrange(0, 60), r.randrange(0, 60), -5 & 0xffffffff]))
    units = []
    for i in range(8):
        u = newbuf(ctx, 0x160)
        rec = newbuf(ctx, 0xc50)
        w32(ctx, rec + 0x4f0, r.choice([2, 2, 1]))
        w32(ctx, rec + 0xbbf, r.choice([0, 0, 1]))
        w16(ctx, rec + 0x560, r.choice([0, 0x20, r.getrandbits(16)]))
        w32(ctx, rec + 0x4e0, fl(r, 100))
        w32(ctx, u + 8, r.choice([0, 1]))
        w32(ctx, u + 0xc, r.choice([self_, r.randrange(8)]))
        w32(ctx, u + 0x14, r.choice([3, 3, 1]))
        w32(ctx, u + 0x28, rec)
        w32(ctx, u + 0x68, fl(r, 360))
        units.append(u)
    w32(ctx, 0x109e8b40, *units)
    idx = r.randrange(0, 0x200)
    blob = bytearray(0x200 * SHOT)
    for _ in range(r.choice([0, 3, 12])):
        j = r.randrange(0, 0x200)
        struct.pack_into('<I', blob, j * SHOT, r.choice([5, 5, 4]))
        struct.pack_into('<I', blob, j * SHOT + 0x48, r.choice([1, 0x8000, 0]))
        struct.pack_into('<I', blob, j * SHOT + 0x3c, r.getrandbits(32))
    rec = bytearray(rb(ctx, SHOT))
    kind = r.choice([0x3e, 0x3f, 0x3e, 0x3f, r.randrange(0x37, 0x47), r.randrange(0x37, 0x47), 5])
    struct.pack_into('<I', rec, 0, kind)
    for off, sc in ((0x18, 2), (0x1c, 2), (0x20, 2), (0x24, 0.01), (0x28, 0.01), (0x2c, 0.01), (0x5c, 50)):
        struct.pack_into('<I', rec, off, fl(r, sc))
    struct.pack_into('<i', rec, 0x30, r.randrange(-3, 4))
    struct.pack_into('<i', rec, 0x34, r.choice([0, -1, 1, 5]))
    struct.pack_into('<I', rec, 0x3c, newbuf(ctx, 0xa0))
    struct.pack_into('<I', rec, 0x40, r.randrange(0, 8))
    struct.pack_into('<I', rec, 0x44, r.choice([0, 0x200, 0x400, 0x200, 0x400, 1]))
    struct.pack_into('<I', rec, 0x48, r.choice([0, 0x8000, 0x8001, 1]))
    struct.pack_into('<I', rec, 0x50, r.choice([self_, r.randrange(8), r.randrange(8), 0xffffffff]))
    struct.pack_into('<I', rec, 0x60, r.choice([0, 1]))
    struct.pack_into('<I', rec, 0x7e, r.choice([0, 0x40, r.getrandbits(32)]))
    struct.pack_into('<i', rec, 0x82, r.randrange(-3, 4))
    struct.pack_into('<I', rec, 0x8e, r.randrange(0, 4))
    blob[idx * SHOT:(idx + 1) * SHOT] = rec
    wb(ctx, SHOTS, blob)
    w32(ctx, 0x1014558e + struct.unpack_from('<I', rec, 0x8e)[0] * 0xd6, r.choice([8, 8, 0, r.getrandbits(32)]))
    hr = newbuf(ctx, 0x30)
    w16(ctx, hr + 2, r.choice([0x100, 0x100, 0x100, 0x200, 0x400, 0x800, 0, r.getrandbits(16)]))
    w16(ctx, hr + 0x24, r.choice([self_, r.randrange(8)]))
    w16(ctx, hr + 0x26, r.choice([0, 1, 2, 3]))
    hit = r.choice([hr, hr, 0])
    rets(0x100a9b71, hit)
    rets(0x100b53bb, hit)
    ST['dda9'] = r.choice([hr, hr, 0, 'keep'])
    rets(0x1002dda9, r.choice([1, 1, 0]))
    rets(0x1009c929, r.choice([0, 9, 10, 50]))
    rets(0x10052be7, r.choice([0, 1, 2, 3]))
    ST['pos'] = [tuple(fl(r, 100) for _ in range(3)) for _ in range(4)]
    ST['segb'] = [rb(ctx, SEG) for _ in range(3)]
    ST['steer'] = [fl(r, 2) for _ in range(3)]
    ST['fvals'] = {
        0x10084a94: [r.choice([fb(3000.0), fb(2999.0), fb(3001.0), fb(r.uniform(0, 6000)), rfloat(r)])] +
                    [r.choice([fb(250.0), fb(r.uniform(0, 500)), fb(r.uniform(0, 500)), rfloat(r)]) for _ in range(5)],
        0x1008486e: [fl(r, 360)],
        0x1002d961: [r.choice([fl(r, 200), fl(r, 200), 0x7fc00000]) for _ in range(2)],
        0x100fcb30: [fl(r, 360)],
    }
    return [idx]


SPECS = [dict(name='Sim_100225c7', orig=0x100225c7, c='Sim_100225c7', gen=g, void=True, trials=15000)]
