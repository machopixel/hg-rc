# Round 32, E / F: the checker side of 1.2's particle families (1.2-only functions, no 1.0 spec to wrap). A spec
#     exec(open('tools/verify/w32_common.py').read())
# then declares its stubs with stub12(addr10, cname, nargs, addr12) (a converted 1.0 helper: stubbed at 1.0's
# address, at the C function and at 1.2's twin, logged under the 1.0 name) or helper(A12(f12), 'Sim12_...', n) (a 1.2
# function the C has too), and builds SPECS with spec12(). GLOBALS pairs the C's globals with 1.2's: (1.0 address or
# C symbol name, 1.2 address, size); to12 copies them to 1.2's side before its run, from12 back after it. In the
# logs, 1.2's addresses of paired globals and functions become the C's (FUNCS: 1.2 address -> C symbol).
import struct
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

GLOBALS = [
    (0x1014cae0, 0x1011f9e8, 4),        # the camera
    (0x1015dc78, 0x10130768, 4),        # the frame counter
    (0x1015eef4, 0x101316dc, 4),        # the effect scale of the camera code (1.0 0x10044d0b)
    (0x1017b940, 0x1014a5e8, 4),        # 3D mode
    (0x1017cc20, 0x1014b590, 4),        # the pool's created count
    (0x1017cc24, 0x1014b594, 4),        # its active count
    ('g_w32_1014b58c', 0x1014b58c, 4),
    ('g_w32_1011e700', 0x1011e700, 22 * 0x1e),
]
ADDRS = {0x109e3e00: 0x107ee480}       # 1.0 -> 1.2, addresses that only show up in logs
FUNCS = {}                             # 1.2 address -> C symbol
PAIRS12 = {}                           # 1.0 helper -> 1.2 twin
W32_TYPE = 0x1e


def stub12(addr10, cname, nargs, addr12, **kw):
    helper(addr10, cname, nargs, **kw)
    PAIRS12[addr10] = addr12


def _ga(m, g):
    return m.syms[g] if isinstance(g, str) else g


TO12_MORE = []                         # extra steps (m) before 1.2's run
FROM12_MORE = []                       # and after it


def to12(m):
    for f in TO12_MORE:
        f(m)
    if not getattr(m, '_w32c', False):
        m._w32c = True
        for a10, a12 in PAIRS12.items():
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, a10), None, A12(a12), A12(a12))
    for g, a12, n in GLOBALS:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(_ga(m, g), n)))


def from12(m):
    for g, a12, n in GLOBALS:
        m.uc.mem_write(_ga(m, g), bytes(m.uc.mem_read(A12(a12), n)))
    for f in FROM12_MORE:
        f(m)


def _inv(m):
    if getattr(m, '_w32inv', None) is None:
        inv = {A12(a12): _ga(m, g) for g, a12, n in GLOBALS}
        inv.update({A12(a12): a10 for a10, a12 in ADDRS.items()})
        inv.update({A12(a12): m.syms[c] for a12, c in FUNCS.items()})
        m._w32inv = inv
    return m._w32inv


def log_map_for(m):
    inv = _inv(m)

    def fix(v):
        return inv.get(v, v) if isinstance(v, int) else v

    def lm(log_o, log_c):
        return [(e[0], tuple(fix(v) for v in e[1])) if isinstance(e[1], tuple) else e for e in log_o], log_c
    return lm


def spec12(name, f12, gen, trials, void=False):
    def g2(ctx):
        args = gen(ctx)
        ST['spec'] = s
        s['log_map'] = log_map_for(ctx.m)
        return args
    s = dict(name=name, orig12=f12, c=name, gen=g2, trials=trials, to12=to12, from12=from12)
    if void:
        s['void'] = True
    return s


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def type_row(ctx, t, **kw):
    """random settings for particle type t in the C's table (1.2's gets them by to12)"""
    r = ctx.rng
    a = ctx.m.syms['g_w32_1011e700'] + t * W32_TYPE
    group = kw.get('group', r.choice([-1, 0, 0x100, 0x123, 0x1ff]))
    count = kw.get('count', r.choice([0, 1, 2, 5, 32]))
    wb(ctx, a, struct.pack('<iI', group, count) + bytes(r.getrandbits(8) for _ in range(6)))
    w8(ctx, a + 0xd, kw.get('size', r.choice([0, 1, 2, 8, 33, 255])))
    w32(ctx, a + 0x12, r.choice([0, 0, 1, r.getrandbits(32)]))
    w32(ctx, a + 0x16, r.choice([0, 1, 2, 3]))
    w32(ctx, a + 0x1a, r.choice([0, 1, 2, 3]))


def e_rand(uc, k, raw, eax):
    m = raw[0] if raw[0] else 1
    return (k * 2654435761 + ST['seed']) % m
