# Network round: the message loop Net_ProcessMessages (1.0 0x1003e291, w9b_net.c) against 1.2's 0x10031a86, on
# spec_w9b.py's stubs and generator (round 9), each stub also hooked at its 1.2 twin. 1.2's changes checked here:
# the "player left" notice "d2" / "d6" (1.0: "eD"; NETLEFT_FIX, Diego's B, reads the key after the tag: built off
# here unless NETLP_FIX=1, then 1.2's code reads it there too through a hook), a player already gone skipped, the
# "DI" (0xd1) and "AK" (0xe) dispatch, "KI" at 0xc bytes, the time-out over 10 players handing a silent player's
# computer gear on (Sim_10040564) on the host, and the 10-player tables (ours g_net10Heard / Left / Ready).
# 1.2's unit records lack our dword +0x18 (turned into its layout for its run and back).
# Negative check: NETLP_NEG12=1 builds the C with NETLOOP_MUTATE (1.0's notice and time-out).
import os
_ns = {}
exec(open('tools/verify/w10_common.py').read(), _ns)
exec(open('tools/verify/v12_common.py').read())
exec(open('tools/verify/spec_w9b.py').read())

FIX = os.environ.get('NETLP_FIX') == '1'
from unicorn.x86_const import UC_X86_REG_EDX
SOURCES = _ns['closure'](['sim/unsorted_1003/w9b_net.c'])
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe', '-DNETLEFT_FIX=%d' % FIX]
if os.environ.get('NETLP_NEG12'):
    CFLAGS = CFLAGS + ['-DNETLOOP_MUTATE']

# the converted callees of 1.2's loop: 1.0 address -> 1.2 twin
TWINS = {0x100420a0: 0x10035eb9, 0x1002ac1b: 0x100201bb, 0x1006abf6: 0x1005a9c6, 0x1006a12d: 0x10059e9d,
         0x100421b0: 0x10035fe7, 0x1007df6c: 0x1006a0ec, 0x1003f3f8: 0x10032cbe, 0x100aa39f: 0x1009582f,
         0x100aa454: 0x100958e4, 0x100aa799: 0x10095c29, 0x100aa351: 0x100957e1, 0x100aa406: 0x10095896,
         0x1004044c: 0x10033ec2, 0x10040191: 0x10033a8a, 0x1004158c: 0x1003524d, 0x1004170e: 0x100354eb,
         0x100418e3: 0x100356ce, 0x10041b2d: 0x10035923, 0x10041c4c: 0x10035a50, 0x10041a16: 0x10035801,
         0x1003ff72: 0x10033860, 0x100405e6: 0x1003406a, 0x1004084a: 0x10034468, 0x1004094e: 0x1003458e,
         0x10040a37: 0x10034682, 0x10040af8: 0x1003474e, 0x10040f2d: 0x10034bbc, 0x100410ff: 0x10034d99,
         0x10041247: 0x10034eef, 0x1004131b: 0x10034fce, 0x10040376: 0x10033dec, 0x1003eebc: 0x10032782,
         0x10041a47: 0x1003583d, 0x1004227f: 0x100360c4, 0x10040eb3: 0x10034b31, 0x10042908: 0x100366ce,
         0x100419e9: 0x100357d4, 0x100408bd: 0x100344ef, 0x1004185b: 0x10035646, 0x10041888: 0x10035673,
         0x10041997: 0x10035782, 0x1002abb0: 0x10020150, 0x1002abc3: 0x10020163, 0x10040564: 0x10033fe8}
STUBS[0x1003f3f8] = Stub(3, None, 'Sim_1003f3f8')
STUBS[0x10040af8] = Stub(2, None, 'Sim_10040af8')
STUBS[0x10040564] = Stub(1, None, 'Sim_10040564')
STUBS[A12(0x10033d7b)] = Stub(1, None, 'Sim12_10033d7b')
STUBS[A12(0x10035348)] = Stub(1, None, 'Sim12_10035348')
for _a in (0x1004044c,):            # 1.2 passes the sender too; the C does not: one argument logged
    STUBS[_a].nargs = 1

PAIRS = {0x1017b8b8: 0x1014a558, 0x1015ecdc: 0x10131530, 0x1017b8bc: 0x1014a55c, 0x1015eccc: 0x10131520,
         0x1015ec58: 0x101314a0, 0x1015ec60: 0x101314a8, 0x1015dc7c: 0x1013076c, 0x1015f1fc: 0x10131968,
         0x1015ec6c: 0x101314b4, 0x1015d008: 0x107caa78, 0x1015d004: 0x107caa74, 0x1017b8c8: 0x1014a568,
         0x1015dc64: 0x10130754, 0x1015ec70: 0x101314b8, 0x1015d08c: 0x107cb43c, 0x1015ec7c: 0x101314c8,
         0x1017b94c: 0x1014a5f8, 0x1015ece0: 0x10131534, 0x1015ece4: 0x10131538, 0x1013ac9c: 0x10110138,
         0x1015ec68: 0x101314b0, 0x1015ecd0: 0x10131524, 0x1013aca0: 0x1011013c, 0x1013aca4: 0x10110140,
         0x1015f1e4: 0x10131950, 0x1015f1d0: 0x1013193c, 0x1015ec54: 0x1013149c, 0x1015dc84: 0x10130774,
         0x1015dc78: 0x10130768, 0x1015ec84: 0x101314d0, 0x101c4a00: 0x101873a0, 0x109b98c0: 0x107ca7e0,
         0x1015ecd4: 0x10131528, 0x1015eab4: 0x10131348, 0x1015eab8: 0x1013134c, 0x1013acac: 0x1011014c, 0x10372d98: 0x1028f9f0}
TABLES = [('g_net10Heard', 0x107cab60, 0x28), ('g_net10Left', 0x107cb440, 0x28), ('g_net10Ready', 0x107cb430, 10),
          ('g_net10Mark', 0x107cb4d0, 10), ('g_w34NetOver', 0x101314c0, 4)]
UNIT = 0x200
TAGS12 = TAGS + [b'DI', b'AK', b'd2', b'd6', b'd2']
EXTRA.update({b'KI': 0xc, b'DI': 0xd1, b'AK': 0xe, b'DA': 0x42})


def install12(m):
    if getattr(m, '_netlp', False):
        return
    m._netlp = True

    def make(addr, st):
        name = 'stub_%x' % addr

        def cb(uc, address, size, ud):
            if st.mine and addr not in ST.get('active', ()):
                return
            esp = uc.reg_read(UC_X86_REG_ESP)
            raw = list(struct.unpack('<%dI' % st.nargs, bytes(uc.mem_read(esp + 4, 4 * st.nargs)))) if st.nargs else []
            n = sum(1 for e in m.api_log if e[0] == name)
            val, extra = st.pol(uc, n, raw)
            args = tuple((v & 0xffff) if i in st.words else logged(v) for i, v in enumerate(raw))
            m.api_log.append((name, args, tuple(extra)))
            uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
            uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
            uc.reg_write(UC_X86_REG_ESP, esp + 4)
        return cb

    for a10, a12 in TWINS.items():
        m.raw.hook_add(UC_HOOK_CODE, make(a10, STUBS[a10]), None, A12(a12), A12(a12))
    if FIX:
        # Diego's B on 1.2's side: after "mov ax, word ptr [edx]" (0x10031bc3) the key is taken after the tag
        def key_after_tag(uc, address, size, ud):
            edx = uc.reg_read(UC_X86_REG_EDX)
            w = struct.unpack('<H', bytes(uc.mem_read(edx + 2, 2)))[0]
            uc.reg_write(UC_X86_REG_EAX, (uc.reg_read(UC_X86_REG_EAX) & 0xffff0000) | w)
        m.raw.hook_add(UC_HOOK_CODE, key_after_tag, None, A12(0x10031bc6), A12(0x10031bc6))


def g_loop(ctx):
    r = begin(ctx, 0x1006a12d, 0x1003eebc, 0x1004227f)
    count, self_, entries = table(ctx, r.choice([1, 2, 2, 3, 4, 8]))
    net_globals(ctx)
    w32(ctx, 0x1017b8b8, r.choice([0, 1]))
    w32(ctx, 0x1015ecdc, r.choice([0, 1]))
    w32(ctx, 0x1017b8bc, r.getrandbits(32))
    w32(ctx, 0x1015eccc, r.choice([1, 1, 1, 1, 0, 2]))
    w32(ctx, 0x1015ec58, r.choice([2, 3, 8, 8, 1, 0]))
    w32(ctx, 0x1015d008, zbuf(ctx, 0x100))
    w32(ctx, 0x1017b94c, r.choice([0, 1, 1, 2, 3, r.getrandbits(32)]))
    t = ST['t0']
    for k in range(10):
        w32(ctx, ctx.m.syms['g_net10Heard'] + 4 * k, (t - r.choice([0, 1, 0x38e, 0x38f, 0x1000, -5])) & 0xffffffff)
        w32(ctx, ctx.m.syms['g_net10Left'] + 4 * k, r.choice([0, 1]))
    wb(ctx, ctx.m.syms['g_net10Ready'], rb(ctx, 10))
    wb(ctx, ctx.m.syms['g_net10Mark'], rb(ctx, 10))
    w32(ctx, ctx.m.syms['g_w34NetOver'], r.choice([0, 1]))
    w32(ctx, 0x1015d08c, r.choice([0x27, 2, r.getrandbits(8)]))
    w32(ctx, 0x1015ec7c, r.choice([0, 1, 1, 0xffff]))
    w32(ctx, 0x1015ece0, t - r.choice([-5, 0, 1, 100]))
    w32(ctx, 0x1015ece4, t - r.choice([-5, 0, 1, 100]))
    w32(ctx, 0x1015ec68, r.getrandbits(32))
    w32(ctx, 0x1013aca0, r.choice([0, 1, 2]))
    w32(ctx, 0x1013aca4, r.choice([0, 1, 2]))
    root = G32(ctx, 0x1015d004)
    wb(ctx, root + 0x137, bytes([r.choice([0, 1, 2]), r.choice([0, 1, 2])]))
    w32(ctx, 0x1015ec54, r.choice([0, 1]))
    w32(ctx, 0x1015dc84, r.choice([2, 2, 0, 1]))
    w32(ctx, 0x1015ec70, r.getrandbits(32))
    w32(ctx, 0x1015ec6c, r.getrandbits(32))
    w32(ctx, 0x1015ec60, r.getrandbits(32))
    w32(ctx, 0x1017e950, 0)                     # the C's locale handle: Str_CompareNoCase's ASCII path
    w32(ctx, A12(0x1014d700), 0)                # 1.2's (its _strcmpi tests [0x1014d6f8 + 8])
    w32(ctx, 0x10372d98, r.getrandbits(32))
    free_player(ctx, entries)
    for e in entries:
        w32(ctx, e + 0x18, 0)                   # (not in 1.2's record; the old time-out cleared it)
    msgs = []
    for _ in range(r.choice([0, 1, 1, 2, 3, 5])):
        tag = r.choice(TAGS12 + [b'xx', b'da', b'Da', b'ed', b'ss'])
        key = r.choice(ctx.m._keys + [0xfa01, 0x1234, ctx.m._keys[self_]])
        if tag in (b'd2', b'd6', b'eD'):
            typ = r.choice([0, 0, 0, key])
        else:
            typ = r.choice([key, key, 0, r.getrandbits(16)])
        extra = r.choice([EXTRA.get(tag.upper(), 2), EXTRA.get(tag.upper(), 2), 5, 0x27, r.getrandbits(8)])
        data = bytearray(rb(ctx, 0x40))
        data[0:2] = tag
        if typ == 0:
            data[2:4] = struct.pack('<H', key)
        msgs.append((typ, extra, bytes(data)))
    ST['msgs'] = msgs
    ST['units'] = entries
    return []


def to12(m):
    install12(m)
    for a10, a12 in PAIRS.items():
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(a10, 4)))
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(PTRTAB, 0x64)))
    for name, a12, n in TABLES:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(m.syms[name], n)))
    ST['usave'] = {}
    for u in ST['units']:
        d = bytes(m.uc.mem_read(u, UNIT))
        ST['usave'][u] = d
        m.uc.mem_write(u, d[:0x18] + d[0x1c:] + bytes(4))


def from12(m):
    for a10, a12 in PAIRS.items():
        m.uc.mem_write(a10, bytes(m.uc.mem_read(A12(a12), 4)))
    for name, a12, n in TABLES:
        m.uc.mem_write(m.syms[name], bytes(m.uc.mem_read(A12(a12), n)))
    for u, old in ST['usave'].items():
        d = bytes(m.uc.mem_read(u, UNIT))
        m.uc.mem_write(u, d[:0x18] + old[0x18:0x1c] + d[0x18:UNIT - 4])


SPECS = [dict(name='Net_ProcessMessages_v12' + ('_fix' if FIX else ''), orig12=0x10031a86, c='Net_ProcessMessages',
              gen=g_loop, trials=3000, to12=to12, from12=from12)]
