# Round 10: 0x100c8ba7 (HUD weapon gauge). Stubbed on both sides: the line routines 0x100ef727 / 0x100e86a1, the text
# routines 0x100f2f01 / 0x100f1e10 / 0x100ecea6 (text logged), the loader 0x100ba7d5, Sim_100bb2b0, Sim_100ccb0d
# (text logged), Sim_100c83ed (its float logged), Sim_100c9572 as a float stub, and the CRT sprintf (format-driven
# logging as in spec_w10_10011c28.py, the format text written to the buffer). Exact.
exec(open('tools/verify/w10_common.py').read())
import re as _re

SOURCES = closure(['sim/unsorted_100c/w10_misc.c'])
helper(0x100ef727, None, 7)
helper(0x100e86a1, None, 7)
for _a in (0x100f2f01, 0x100f1e10, 0x100ecea6):
    helper(_a, None, 6, sargs=(4,))
helper(0x100ba7d5, None, 3)
helper(0x100bb2b0, 'Sim_100bb2b0', 2)
helper(0x100ccb0d, 'Sim_100ccb0d', 2, sargs=(0,))
helper(0x100c83ed, 'Sim_100c83ed', 3)
ACTIVE = (0x100ef727, 0x100e86a1, 0x100f2f01, 0x100f1e10, 0x100ecea6, 0x100ba7d5, 0x100bb2b0, 0x100ccb0d, 0x100c83ed)


def _sprintf_cb(m):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        buf, fmt = ur32(uc, esp + 4), ur32(uc, esp + 8)
        text = ustr(uc, fmt)
        args, at = [], esp + 12
        for spec in _re.findall(rb'%[-+ #0]*\d*(?:\.\d+)?l?([a-zA-Z%])', text.encode('latin-1') if isinstance(text, str) else text):
            c = chr(spec[0]) if isinstance(spec, bytes) else spec
            if c == '%':
                continue
            if c in 'feg':
                args.append(('f', bytes(uc.mem_read(at, 8))))
                at += 8
            elif c == 's':
                args.append(('s', ustr(uc, ur32(uc, at))))
                at += 4
            else:
                args.append(('i', ur32(uc, at)))
                at += 4
        m.api_log.append(('sprintf', text, tuple(args)))
        out = (text.encode('latin-1') if isinstance(text, str) else bytes(text))[:0x60] + b'\0'
        m.uc.mem_write(buf, out)
        uc.reg_write(UC_X86_REG_EAX, len(out) - 1)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def sprintf_stub(ctx):
    m = ctx.m
    if getattr(m, '_spf', False):
        return
    m._spf = True
    cb = _sprintf_cb(m)
    m.raw.hook_add(UC_HOOK_CODE, cb, None, 0x100fc780, 0x100fc780)
    m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms['Crt_SPrintf'], m.syms['Crt_SPrintf'])



# ---- float stubs: the hook keeps esp and jumps to "fld dword [value]; ret", so the answer comes back in st(0)
FSTUB = {0x100c9572: ('Sim_100c9572', 3, None)}
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
    sprintf_stub(ctx)
    fstubs(ctx)
    c2c = struct.unpack('<f', struct.pack('<I', r32(ctx, 0x1012bc2c)))[0]
    ST['fvals'] = {0x100c9572: [fb(r.choice([r.uniform(0, 5000), 0.5, 2000.0, c2c, c2c * 0.5, 0.0]))]}
    rets(0x100ccb0d, r.randrange(0, 80))
    rets(0x100ba7d5, r.choice([0x7000, 0]))
    for a in (0x101c47c4, 0x101c47cc, 0x101c47d0):
        w32(ctx, a, ctx.buf(16, bytes(r.choice(b'LOCKOUT ') for _ in range(r.randrange(1, 10))).ljust(16, b'\0')))
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    rec = newbuf(ctx, 0x600)
    slots = newbuf(ctx, 0x128 * 4)
    si = r.randrange(0, 4)
    w32(ctx, rec + 0x4fc, si)
    w32(ctx, slots + si * 0x128 + 0x20, r.choice([-2, 0, 5]) & 0xffffffff)
    w32(ctx, rec + 0x84, slots)
    w16(ctx, rec + 0x560, r.choice([0x80, 0x40, 0, 0xc0]))
    me = newbuf(ctx, 0x40)
    w32(ctx, me + 0x28, rec)
    w32(ctx, 0x109e8b40 + 4 * self_, me)
    for a in (0x101668e8, 0x1017b940):
        w32(ctx, a, r.choice([0, 1]))
    w32(ctx, 0x1017e574, r.choice([0, 1, 2, 5]))
    obj = newbuf(ctx, 0x100)
    w32(ctx, obj + 0xf0, r.choice([0x203, 0x403, 0x103, 0x1203, 0]))
    w32(ctx, obj + 0xd8, fb(r.choice([r.uniform(0, 6000), 100.0])))
    wp = newbuf(ctx, 0xc0)
    w8(ctx, wp + 0xb2, r.choice([1, 1, 0]))
    w32(ctx, wp + 0x86, r.choice([0x40, 0x41, 1, 0, 0x20]))
    w32(ctx, wp + 0x50, fb(r.choice([r.uniform(0, 6000), 1000.0])))
    return [obj, r.randrange(0, 640), r.randrange(0, 480), wp]


SPECS = [dict(name='Sim_100c8ba7', orig=0x100c8ba7, c='Sim_100c8ba7', gen=g, void=True, trials=4000)]
