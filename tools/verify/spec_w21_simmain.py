# Round 21: SimMain (0x10084feb), the exported entry point (DECISIONS.md round 21). Every one of its 149 callees is a
# logging stub (the two of its __finally, 0x1008663f and _abnormal_termination 0x1010026a, run for real in the
# original: the C build here has no __try, and with no exception both answer 0), every Windows call goes through the
# import stubs and is logged, and the five function pointers it calls (0x1014cb3c, 0x109e3d28, 0x109e3d34 and the
# display driver's +0x14 / +0x20) point at logging stubs too. Stub effects end its loops: the frame loop (0x10086b99
# sets [0x1017b8bc] = 3 after a few frames and toggles [0x1017b928]), the driver-less loop (0x100d33ec clears
# [0x1015e694]), and the ShowCursor / PeekMessage handlers count down.
# The display driver's start record (0x50 bytes at [ebp-0x88]) is logged whole. The original never sets 5 to 9 of
# its dwords; the C sets them to 0 (the round 21 deviation), so the spec zeroes the original's stack there.
# Negative check: W21_NEG=1 leaves random leftovers there instead, which the record comparison must catch.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1008/w21_simmain.c', 'sim/platform/sim_platform_iat.c'])
NEG = bool(os.environ.get('W21_NEG'))

CALLEES = {
    0x10006a2f: 0, 0x10006e52: 0, 0x100085b7: 2, 0x1000c894: 0, 0x10010f70: 0, 0x1001c693: 0, 0x1001db6a: 0,
    0x1001db6f: 0, 0x1001dcff: 0, 0x1001f26e: 0, 0x1001f3ac: 0, 0x1001f415: 1, 0x1001f454: 0, 0x1001f47d: 0,
    0x1001fb6d: 0, 0x10021ac0: 0, 0x10021d65: 0, 0x1002528f: 0, 0x10025d93: 0, 0x10025dac: 0, 0x10029067: 0,
    0x1002986c: 0, 0x10029f50: 0, 0x10029fba: 0, 0x1002a019: 0, 0x1002a086: 0, 0x1002a0ca: 0, 0x1002a10e: 0,
    0x1002a1c1: 0, 0x1002a7e0: 0, 0x1002a8c2: 0, 0x1002a983: 0, 0x1002ab26: 0, 0x1002abd6: 0, 0x1002ad50: 3,
    0x10033aeb: 0, 0x10033c24: 0, 0x10034c11: 0, 0x10034cca: 0, 0x10034dc2: 0, 0x100352d5: 0, 0x10035478: 0,
    0x100354ff: 0, 0x10036876: 1, 0x1003693c: 0, 0x1003695b: 3, 0x1003795b: 6, 0x10037cab: 0, 0x10039241: 0,
    0x1003924b: 0, 0x1003c970: 0, 0x1003ca04: 0, 0x1003dc19: 1, 0x1003e26a: 0, 0x1003e291: 0, 0x1003edda: 0,
    0x10041e61: 0, 0x10042a11: 0, 0x10044420: 0, 0x100445d6: 0, 0x10045d70: 2, 0x10045db9: 0, 0x10045e19: 0,
    0x10046fe5: 2, 0x1004712a: 2, 0x100473e4: 0, 0x10048f02: 3, 0x1004b710: 0, 0x1004f0a6: 0, 0x1005502a: 0,
    0x10055048: 0, 0x100556c2: 0, 0x10058389: 0, 0x10058493: 0, 0x1005f6c6: 0, 0x1005f8d0: 0, 0x1005f949: 0,
    0x10061bc5: 1, 0x10062386: 0, 0x10074b80: 1, 0x10074c8f: 0, 0x1007520b: 0, 0x10075371: 0, 0x10075585: 0,
    0x10079b68: 2, 0x1007c42f: 0, 0x1007c884: 0, 0x1007c89b: 0, 0x1007df6c: 4, 0x1007e0ff: 0, 0x1007e66c: 0,
    0x1008181e: 5, 0x10083495: 1, 0x10086b99: 0, 0x10086c32: 0, 0x10086d8d: 1, 0x10086ebf: 1, 0x1008988f: 0,
    0x10089bad: 0, 0x1008b6a0: 0, 0x100908b3: 0, 0x10090e23: 0, 0x10098710: 0, 0x100988fc: 2, 0x100989a1: 2,
    0x10098be0: 0, 0x10099510: 0, 0x1009c860: 1, 0x100a0063: 1, 0x100a2a66: 0, 0x100a774e: 0, 0x100ab920: 0,
    0x100abcb3: 0, 0x100ac001: 0, 0x100b388a: 0, 0x100b418e: 0, 0x100b6590: 1, 0x100b82a0: 1, 0x100b8938: 0,
    0x100b979f: 1, 0x100b9eab: 2, 0x100ba5d1: 0, 0x100ba7d5: 3, 0x100bc280: 2, 0x100cc8c5: 1, 0x100d1df9: 0,
    0x100d33ec: 0, 0x100d3491: 0, 0x100d3877: 1, 0x100d7fc0: 0, 0x100d82f6: 0, 0x100e122f: 1, 0x100e1398: 0,
    0x100e46d0: 2, 0x100e86a1: 7, 0x100eb070: 2, 0x100ef727: 7, 0x100f0865: 2, 0x100fc690: 2, 0x100fc6a0: 2,
    0x100fc780: 2, 0x100fc7f0: 3, 0x100fc970: 2, 0x100fcab0: 3, 0x100fd760: 1, 0x100fd7e0: 2, 0x100fd850: 2,
    0x100ff1e0: 2, 0x10100130: 1,
}
PAD = 0x1010d3e6                    # int3 padding: the three function pointers kept in DLL data point here
DRV_BLIT, DRV_READY = 0x10002c9d, 0x10002c57    # slots +0x14 / +0x20 of the driver table 0x1014b838
SLOTS = {0x1014cb3c: (PAD, 0), 0x109e3d28: (PAD + 1, 1), 0x109e3d34: (PAD + 2, 0)}
for a, n in CALLEES.items():
    helper(a, None, n)
for slot, (a, n) in SLOTS.items():
    helper(a, None, n)
helper(DRV_BLIT, None, 4)
helper(DRV_READY, None, 0)
DEREF[PAD + 1] = {0: 0x50}          # the display driver's start record, whole
ACTIVE = tuple(CALLEES) + tuple(a for a, n in SLOTS.values()) + (DRV_BLIT, DRV_READY)


def _extra_arg(fmt_has):            # SystemError and fprintf take a third argument only in some calls
    def eff(uc, k, raw, eax):
        if fmt_has(uc, raw):
            esp = uc.reg_read(UC_X86_REG_ESP)
            return ([('arg3', ur32(uc, esp + 12))], None)
        return None
    return eff


EFFECT[0x100bc280] = _extra_arg(lambda uc, raw: raw[0] in (0x11, 0x53))
EFFECT[0x100fd7e0] = _extra_arg(lambda uc, raw: raw[1] != 0x1014b4bc)


def _frame(uc, k, raw, eax):        # top of the frame loop: ends it after ST['frames'] frames
    if k >= ST['frames']:
        uc.mem_write(0x1017b8bc, struct.pack('<I', 3))
    uc.mem_write(0x1017b928, struct.pack('<I', ST['active_flags'][k % 4]))
    if k >= ST['frames'] + 3:       # a frame loop that keeps skipping through [0x1017b928] == 0
        uc.mem_write(0x1017b928, struct.pack('<I', 1))
    return None


def _noflip(uc, k, raw, eax):       # the driver-less loop's last call: ends it after ST['waits'] passes
    if k + 1 >= ST['waits']:
        uc.mem_write(0x1015e694, struct.pack('<I', 0))
    return None


EFFECT[0x10086b99] = _frame


def _plugin_name(uc, k, raw, eax):
    """round 33: the plugin-name test that sets the 3DFX flag. 1.2 (and the C) run the 3D start-up block in that
    mode even with 3D off, 1.0 does not: this 1.0 comparison answers "no match" when 3D is off"""
    if raw[1] == 0x1014b210 and ur32(uc, 0x1017b940) == 0:
        return 0
    return None


EFFECT[0x100fc970] = _plugin_name
EFFECT[0x100d33ec] = _noflip


def _countdown(name, key, yes, no):  # the first ST[key] calls of this run answer yes
    def h(n, args):
        k = sum(1 for e in ST['m'].api_log if e[0] == name)
        return yes if k <= ST[key] else no
    return h


def _memstatus(n, args, uc):
    p = ur32(uc, uc.reg_read(UC_X86_REG_ESP) + 4)
    uc.mem_write(p + 8, struct.pack('<I', ST['phys']))
    return 0


def _heapalloc(n, args):
    return ST['heap'][args[2] == 0x10]


def _devcaps(n, args):
    return ST['caps'][n % 2]


API = {'ShowCursor': _countdown('ShowCursor', 'cursor', 0, 0xffffffff),
       'PeekMessageA': _countdown('PeekMessageA', 'peek', 1, 0),
       'GlobalMemoryStatus': _memstatus, 'HeapAlloc': _heapalloc, 'GetDeviceCaps': _devcaps,
       'HeapCreate': lambda n, args: ST['hc'][n % len(ST['hc'])]}


def g(ctx):
    r = begin(ctx, *ACTIVE)
    m = ctx.m
    if not getattr(m, '_w21_teb', False):      # fs is 0 in the checker: page 0 stands for the thread block, whose
        m._w21_teb = True                       # exception-list head the original links its frame into and unlinks
        m.raw.mem_map(0, 0x1000)
    m.raw.mem_write(0, struct.pack('<I', 0xffffffff))
    ST['m'] = m
    ST['frames'] = r.choice([0, 1, 2, 3])
    ST['active_flags'] = [r.choice([0, 1, 1]) for _ in range(4)]
    ST['waits'] = r.choice([1, 1, 2, 3])
    ST['cursor'] = r.choice([0, 1, 3])
    ST['peek'] = r.choice([0, 1, 2])
    ST['phys'] = r.choice([0x1e84800, 0x1e84801, 0x16e3600, 0x16e3601, 0x800000, r.getrandbits(32)])
    ST['heap'] = [newbuf(ctx, 0x13c), newbuf(ctx, 0x10)]
    ST['caps'] = [r.choice([-1, 8, 16]) for _ in range(2)]
    ST['hc'] = [r.choice([0, 0x1234, 0x5678]) if r.random() < 0.2 else 0x1000 + k for k in range(7)]
    rets(0x1002ad50, *[r.choice([1, 1, 1, 0]) for _ in range(3)])
    res = newbuf(ctx, 0x48)
    w32(ctx, res + 0x44, r.choice([1, 1, 0, 2]))
    rets(0x10036876, res)
    rets(0x1003e26a, *[r.choice([1, 1, 1, 0]) for _ in range(3)])
    rets(0x10046fe5, *[r.choice([0, 0, 1, 0xffffffff]) for _ in range(3)])
    rets(0x1004712a, *[r.choice([0, 0, 0xffffffff]) for _ in range(3)])
    rec = newbuf(ctx, 0x70)
    if r.random() < 0.3:
        wb(ctx, rec + 0x48, b'\0')
    w32(ctx, 0x1015d1b8, rec)       # never 0: page 0 stands for fs here, so a null record would read the SEH link
    for a in (0x1017b940, 0x1017b944, 0x1017b954, 0x101668f4, 0x1015ec58, 0x1015f1e8, 0x1017d108, 0x1017b920,
              0x1015e694, 0x1017692c, 0x1017b92c, 0x1017b930, 0x1015f204, 0x1017b8b8, 0x1015dca0, 0x1015ebf8,
              0x1017b8cc):
        w32(ctx, a, r.choice([0, 0, 1, r.getrandbits(32)]))
    w32(ctx, 0x1017b8bc, r.choice([0, 0, 0, 2, 3]))
    w32(ctx, 0x1017b94c, r.choice([0, 1, 2, 3, 3, 0x80000000, r.getrandbits(32)]))
    w32(ctx, 0x1015dc7c, r.getrandbits(32))
    w32(ctx, 0x1015e9bc, r.choice([0, 1, 2]))
    w32(ctx, 0x1017b918, r.choice([0x280, 0x280, 0x320, r.getrandbits(32)]))
    w32(ctx, 0x1017b91c, r.choice([0x1e0, 0x1e0, 0x258, r.getrandbits(32)]))
    w32(ctx, 0x104737e0, r.choice([0, 1, 3, -1]))
    w8(ctx, 0x109e89fe, r.choice([0, ord('a')]))
    w8(ctx, 0x10185720, r.choice([0, ord('a')]))
    w32(ctx, 0x1017b8c8, r.randrange(0, 8))
    for k in range(8):
        w32(ctx, 0x109e8b40 + 4 * k, newbuf(ctx, 0x18))
    w32(ctx, 0x109e3e00, newbuf(ctx, 0x20))
    drv = newbuf(ctx, 0x24)
    w32(ctx, drv, r.choice([0, 1]))
    w32(ctx, drv + 0x14, DRV_BLIT)
    w32(ctx, drv + 0x20, DRV_READY)
    w32(ctx, 0x1015e688, drv)
    for slot, (a, n) in SLOTS.items():
        w32(ctx, slot, a)
    rets(DRV_READY, *[r.choice([0, 0, 1, 0xffffffff]) for _ in range(3)])
    for i in r.sample(range(0x200), 4):         # a few named entries in the table the /LOG listing walks
        for j in range(r.randrange(0, 3)):
            w8(ctx, 0x10a51ac0 + i * 0x300 + j * 0x18, ord('n'))
    base = frame(6)
    wb(ctx, base - 0x88, rb(ctx, 0x50) if NEG else bytes(0x50))
    net = r.choice([0, newbuf(ctx, 0x13c)])
    # the 5th argument: 1.0 ignores it; from round 33 the C reads 1.2's "no CD" bit 2 there (DECISIONS.md), so the
    # 1.0 comparison keeps to values without it (the CD needed, 1.0's path)
    return [r.getrandbits(32), r.getrandbits(32), ctx.buf(0x20, 'text'), net,
            r.choice([1, 0, r.getrandbits(32) & ~2]), r.getrandbits(32)]


def post_c(m):
    """round 33: 1.0's "3dfx" flag [0x1017d0ac] is the C's g_w33Voodoo (1.2's [0x1014b954]): put it where 1.0 has it"""
    v = bytes(m.uc.mem_read(m.syms['g_w33Voodoo'], 4))
    if v != bytes(4):
        m.uc.mem_write(0x1017d0ac, v)
        m.dirty.add(0x1017d0ac >> 12)                 # a host write after the run: mark its page for the comparison


# round 33: 1.2 also sets the 3DFX flag for a "POWERVR" plugin path (a second name test, read from 1.2's SimMain);
# this 1.0 comparison builds the C without it (W21V_MUTATE), so both make the same calls
CFLAGS = list(CFLAGS) + ['-DW21V_MUTATE']
SPECS = [dict(name='SimMain', orig=0x10084feb, c='SimMain@24', gen=g, api=API, trials=300 if NEG else 4000,
              post_c=post_c,
              ignore_syms={'g_w33CdNeeded': 4,      # 1.2's CD flag (round 33), no 1.0 counterpart
                           'g_w33Voodoo': 4})]      # compared at 1.0's [0x1017d0ac] (post_c)

# round 31: the texture name table is ours now (w31_map.py: the mapping step around the C run)
exec(open('tools/verify/w31_map.py').read())
SOURCES = SOURCES + [x for x in W31_SOURCES + ['../tools/verify/plat_stub.c'] if x not in SOURCES]
SPECS = w31_wrap(SPECS, None)
_w31_post = SPECS[0]['post_c']


def _post_both(m):
    _w31_post(m)
    post_c(m)


SPECS[0]['post_c'] = _post_both
