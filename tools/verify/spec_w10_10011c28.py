# Round 10: 0x10011c28 (AI table on the debug monitor). Stubbed on both sides: Mono_PutText (text logged, its word
# arguments masked), Sim_10011b9e (names of the generator; word arguments masked), Sim_1001403a (word masked),
# Sim_1005f9aa (text logged) and the CRT sprintf (Crt_SPrintf / 0x100fc780): the stub reads the format, logs every
# argument it consumes (%s as text, %f as the 8 raw bytes of the double) and writes the format text into the buffer.
# Mem_Move (memcpy) runs for real. A speed below [0x10111110] is the original's int 3 (both fault). Exact.
exec(open('tools/verify/w10_common.py').read())
import re as _re

SOURCES = closure(['sim/unsorted_1001/w10_orders.c'])
helper(0x100346ad, 'Mono_PutText', 4, sargs=(0,))
helper(0x10011b9e, 'Sim_10011b9e', 3)
helper(0x1001403a, 'Sim_1001403a', 2)
helper(0x1005f9aa, 'Sim_1005f9aa', 1, sargs=(0,))
for _a, _k in ((0x100346ad, 1), (0x100346ad, 2), (0x100346ad, 3), (0x10011b9e, 0), (0x10011b9e, 2), (0x1001403a, 1)):
    MASK[(_a, _k)] = 0xffff
ACTIVE = (0x100346ad, 0x10011b9e, 0x1001403a, 0x1005f9aa)


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


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    sprintf_stub(ctx)
    names = [ctx.buf(16, bytes(r.choice(b'abcdefg') for _ in range(r.randrange(0, 12))).ljust(16, b'\0')) for _ in range(4)]
    rets(0x10011b9e, *names)
    w32(ctx, 0x10a3ade8, r.choice([1, 1, 0]))
    rp = newbuf(ctx, 4)
    w8(ctx, rp + 1, r.choice([0, 1]))
    w32(ctx, 0x1017b8c4, rp)
    self_ = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015dc78, r.randrange(0, 100000))
    w16(ctx, 0x1015d10c, r.choice([0, 5, 9, 0x7fff]))
    w32(ctx, 0x1037305c, r.randrange(0, 5000))
    lim = struct.unpack('<f', struct.pack('<I', r32(ctx, 0x10111110)))[0]
    n = r.randrange(0, 6)
    w32(ctx, 0x1015dc64, n)
    for i in range(n):
        u = newbuf(ctx, 0x2a0)
        wb(ctx, u, bytes(r.getrandbits(8) for _ in range(0x2a0)))
        w32(ctx, u + 4, r.choice([self_, i, 30]))
        side = r.randrange(0, 3)
        w32(ctx, u + 8, side)
        w32(ctx, 0x104727a0 + 4 * side, r.choice([-1, 0, 2]) & 0xffffffff)
        sq = r32(ctx, 0x104727a0 + 4 * side)
        if sq != 0xffffffff:
            p = 0x1047383a + side * 0x4fc3a + sq * 0x13f
            w32(ctx, p + 0x67, r.randrange(0, 5000))
            w32(ctx, p + 0x6f, r.choice([-1, 0, 30]) & 0xffffffff)
        w32(ctx, u + 0xd4, fb(lim + r.uniform(0, 100)) if r.random() < 0.98 else fb(lim - 1))
        ctl = newbuf(ctx, 0x60)
        for off in (0x1e, 0x1f, 0x20, 0x21, 0x22, 0x26, 0x28):
            w8(ctx, ctl + off, r.choice([0, 0, 1]))
        w32(ctx, ctl + 8, fb(r.uniform(-5, 5)))
        w32(ctx, ctl + 0xc, fb(r.uniform(-5, 5)))
        w32(ctx, u + 0x54, ctl)
        rec = newbuf(ctx, 0x500)
        w32(ctx, rec + 0x4f0, r.choice([0, 4, r.getrandbits(32)]))
        blk = newbuf(ctx, 0x50)
        w32(ctx, blk + 0x44, fb(r.uniform(0, 100)))
        w32(ctx, rec + 0x88, blk)
        w32(ctx, u + 0x28, rec)
        w32(ctx, u + 0x24a, r.choice([0, newbuf(ctx, 8)]))
        w16(ctx, u + 0x1a6, r.choice([3, 1, r.getrandbits(16)]))
        w16(ctx, u + 0x1a8, r.choice([0, 0x201, r.getrandbits(16)]))
        w16(ctx, u + 0x1aa, r.choice([0, 0x203, r.getrandbits(16)]))
        w8(ctx, u + 0x124 + r.randrange(0, 16), 0)
        w32(ctx, 0x109e8b40 + 4 * i, u)
        if r.random() < 0.2:
            w32(ctx, 0x1015ef14, u)
    return []


SPECS = [dict(name='Sim_10011c28', orig=0x10011c28, c='Sim_10011c28', gen=g, void=True, trials=3000)]
