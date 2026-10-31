# Round 32, D: the effect pool in 1.2 mode against 1.2's own code. The per-frame pass (1.0 0x10090e23 -> 1.2
# 0x10079dfa) and the draw dispatcher (1.0 0x10090f17 -> 1.2 0x10079eee, which in 1.2 also notes Sim_100448ff() == 1
# in 0x1014b58c and marks a system whose particle type has Lock with bit 2 of +0x10), on the round-20 (w3o) specs'
# generator with 1.2's record (0x406 bytes, update / draw at +0x336 / +0x33a, type at +0x402). The C runs with the
# Vulkan switch and the 3D mode on (W32_PARTICLES()). Pointers to records in the stubs' log (memory) are mapped
# back from 1.2's pool (0x10191060) to the C's (0x10217c20). The other pool functions are 1.0's with only the record
# size and those offsets changed (DECISIONS.md, round 32: the aligned listings).
import os
exec(open('tools/verify/spec_w3o.py').read())
HELPERS = globals().get('HELPERS', {})
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W32_NEG'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW32F_MUTATE']

POOL_REC, POOL_UPD, POOL_DRAW = 0x406, 0x336, 0x33a
NREC = 10
POOL10, POOL12 = 0x10217c20, 0x10191060
TYPES12 = 0x1011e700


def _r32(m, a):
    return struct.unpack('<I', bytes(m.uc.mem_read(a, 4)))[0]


def _w32(m, a, v):
    m.uc.mem_write(a, struct.pack('<I', v & 0xffffffff))


def mode12(gen, f12):
    def g2(ctx):
        args = gen(ctx)
        m = ctx.m
        r = ctx.rng
        w32(ctx, 0x1017b940, 1)
        w32(ctx, m.syms['g_w31Tex'], 1)
        w32(ctx, 0x1015ef04, r.choice([0, 1, 1, 2]))                   # Sim_100448ff()
        w32(ctx, m.syms['g_w32_1014b58c'], r.getrandbits(32))
        for i in range(NREC):
            w32(ctx, POOL10 + i * POOL_REC + 0x402, r.randrange(0, 0x16))
        for t in range(0x16):
            w32(ctx, m.syms['g_w32_1011e700'] + t * 0x1e + 0x12, r.choice([0, 0, 1, r.getrandbits(32)]))
        arm(m, A12(f12))
        return args
    return g2


def to12_more(m):
    m.uc.mem_write(A12(TYPES12), bytes(m.uc.mem_read(m.syms['g_w32_1011e700'], 0x16 * 0x1e)))
    _w32(m, A12(0x1014b58c), _r32(m, m.syms['g_w32_1014b58c']))


def from12_more(m):
    _w32(m, m.syms['g_w32_1014b58c'], _r32(m, A12(0x1014b58c)))
    log = ST['log']
    lo, hi = A12(POOL12), A12(POOL12) + NREC * POOL_REC
    pos = min(_r32(m, log), LOGCAP)
    data = bytearray(m.uc.mem_read(log + 4, pos))
    for k in range(0, pos - 15, 16):                  # emit(): each value as 4 dwords 0x7fc000XX, low byte first
        w = struct.unpack_from('<4I', data, k)
        v = sum((x & 0xff) << (8 * j) for j, x in enumerate(w))
        if lo <= v < hi:
            v = v - lo + POOL10
            struct.pack_into('<4I', data, k, *[0x7fc00000 | ((v >> (8 * j)) & 0xff) for j in range(4)])
    m.uc.mem_write(log + 4, bytes(data))


_BY = {s['name']: s for s in SPECS}
EXTRA = {0x1015ef04: 0x101316ec, 0x10215be0: 0x1018f040, 0x10217c00: 0x10191040, 0x10215ae8: 0x1018ef30, 0x10216b80: 0x1018ffe0, 0x1017cc20: 0x1014b590,
         0x1017cc28: 0x1014b598, 0x1017cc2c: 0x1014b59c, POOL10: POOL12}
WINDOWS = {POOL10: NREC * POOL_REC, 0x10216b80: 0x400, 0x10215be0: 0x100}


def mk(name, f12):
    s = _BY[name]
    return v12_spec(dict(s, gen=mode12(s['gen'], f12)), f12, windows=WINDOWS, extra_pairs=EXTRA,
                    skip=set(range(POOL10 + 4, POOL10 + NREC * POOL_REC)), to12_more=to12_more,
                    from12_more=from12_more, name=name + '_v12')


SPECS = [mk('Sim_10090e23', 0x10079dfa), mk('Sim_10090f17', 0x10079eee)]
