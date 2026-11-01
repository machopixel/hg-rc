# Round 33: the world-file chunk reader (1.0 0x100dcc12) against 1.2's 0x100c1152, on spec_w15_100dcc12.py's
# generator and stubs (hooked at 1.2's callees too, logged under the 1.0 names), with files of the chunks 1.2
# changed and a few plain ones (VIEW, GPS, CLIM, HTXT, unknown tags). 1.2's changes: the VIEW chunk's second value
# is stored two and a half times ([0x1014ca40]); in a network game a GPS gear whose type name (+0x36) holds "tank"
# (any case: 1.2 lowers a copy with its _strlwr 0x100ea880) is made infantry ("d01infnt", "mmb00std", "Infantry",
# "GREL", words +8 / +0xa zeroed). The gear records hold their fields from +0x1c 4 bytes earlier in 1.2: the table's
# 26 records and the current gear [0x1017e6b8] are shifted down by 4 before 1.2's run and back after it.
# Negative checks: W15_NEG12=v builds the C with W15V_MUTATE (1.0's VIEW value), =t with W15T_MUTATE (no swap), =l
# with W7RL_MUTATE (K..Z not lowered).
import os
exec(open('tools/verify/spec_w15_100dcc12.py').read())
exec(open('tools/verify/v12_common.py').read())
_neg = os.environ.get('W15_NEG12')
if _neg:
    CFLAGS = list(globals().get('CFLAGS', [])) + [{'v': '-DW15V_MUTATE', 't': '-DW15T_MUTATE', 'l': '-DW7RL_MUTATE'}[_neg]]

FSTUB12 = {0x10089325: A12(0x10073685)}
PLAIN = ['VIEW', 'VIEW', 'GPS', 'GPS', 'GPS', 'CLIM', 'HTXT', '?']
TANKS = [b'tank', b'Tank', b'TANK', b'heavytank', b'TankZ', b'Tan', b'Grizzly']
SHIFT = {}
_make_chunk = make_chunk


def make_chunk(r, t, nested):
    c = _make_chunk(r, t, nested)
    if t == 'GPS' and r.random() < 0.6:
        nm = r.choice(TANKS)
        pos = r.randrange(0, 22 - len(nm))
        c = bytearray(c)
        c[0x36 + pos:0x36 + pos + len(nm)] = nm
        c = bytes(c)
    return c


def make_file(r, n, nested):
    ctx = T['ctx']
    data = bytearray(tag(0x101349e8) + rb(ctx, 8))
    for _ in range(n):
        data += make_chunk(r, r.choice(PLAIN), nested)
    d = ctx.buf(len(data) + 16, bytes(data) + bytes(16))
    f = ctx.buf(0x40, 'zero')
    wb(ctx, f + 6, name(r, 1, 8) + b'\0')
    w32(ctx, f + 0xf, len(data))
    w32(ctx, f + 0x13, 0x1000)
    w32(ctx, f + 0x17, d)
    w32(ctx, f + 0x1b, d)
    return f


def g12(ctx):
    args = g(ctx)
    r = ctx.rng
    w32(ctx, 0x1015ec58, r.choice([0, 1, 1]))
    w32(ctx, 0x1017e950, 0)
    SHIFT.clear()
    for i in range(0x1a):
        p = r32(ctx, 0x109e8b40 + 4 * i)
        if p:
            SHIFT[p] = 0x300
    cur = r32(ctx, 0x1017e6b8)
    if cur:
        SHIFT[cur] = 0x300
    for p in SHIFT:          # +0x18 is the field 1.2 removed; our C still clears it (network state 2): start at 0
        w32(ctx, p + 0x18, 0)
    return args


def to12_more(m):
    m.uc.mem_write(A12(0x107f24c0), bytes(m.uc.mem_read(0x109e8b40, 0x68)))
    m.uc.mem_write(A12(0x107c8ee0), bytes(m.uc.mem_read(0x109b7d40, 0x10 * 0x6c + 8)))     # the groups
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def _thunks10():
    b, img, _ = _img(V10_DLL)
    out = {}
    for a in range(0x10001000, 0x10004000, 5):
        t = _thunk(V10_DLL, a)
        if t != a:
            out.setdefault(t, a)
    return out


THUNK10 = _thunks10()


def _tablemap():
    """the 11 gear-type descriptors (1.0 0x10135300, 1.2 0x1010b5a8, 0x20 bytes): 1.2's entries -> 1.0's"""
    b, img, _ = _img(V10_DLL)
    b2, img2, _ = _img(V12_DLL)
    out = {}
    for i in range(11 * 8):
        v10 = struct.unpack_from('<I', img, 0x10135300 - b + 4 * i)[0]
        v12 = struct.unpack_from('<I', img2, 0x1010b5a8 - b2 + 4 * i)[0]
        if v12 >= 0x10000000:
            out[A12(v12)] = v10
    return out


TABLE12 = _tablemap()


def from12_more(m):
    m.uc.mem_write(0x109e8b40, bytes(m.uc.mem_read(A12(0x107f24c0), 0x68)))
    m.uc.mem_write(0x109b7d40, bytes(m.uc.mem_read(A12(0x107c8ee0), 0x10 * 0x6c + 8)))
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])
        # the gear-type tables' function pointers: 1.2's functions back to the 1.0 thunks the C copies
        for o in range(0, n, 4):
            v = struct.unpack('<I', bytes(m.uc.mem_read(buf + o, 4)))[0]
            if v in TABLE12:
                m.uc.mem_write(buf + o, struct.pack('<I', TABLE12[v]))


_s = v12_spec(dict(SPECS[0], gen=g12, trials=int(os.environ.get('W15_TRIALS') or 3000)), 0x100c1152,
              windows={0x1015da30: 0x40}, to12_more=to12_more, from12_more=from12_more, skip=(0x109e8b40,) + tuple(range(0x109b7d40, 0x109b7d40 + 0x10 * 0x6c + 8)),
              extra_pairs={0x1015ec58: 0x101314a0, 0x1017e950: 0x1014d700, 0x1014ca3c: 0x1011f944,
                           0x1014ca40: 0x1011f948, 0x1017e6b8: 0x1014d4a0, 0x1015dc64: 0x10130754,
                           0x1017b8c8: 0x1014a568, 0x1017e820: 0x1014d5d0})
INV12 = {A12(v): k for k, v in _s['_v12_pairs'].items()}
_part10 = _part


def _part12(uc, k, raw, eax):
    """1.2 passes its own copies of the part names: answered as for 1.0's"""
    raw = list(raw)
    raw[1] = INV12.get(raw[1], raw[1])
    return _part10(uc, k, raw, eax)


EFFECT[0x1005ae58] = _part12
import json as _json
FN12 = {A12(int(a, 16)): int(b, 16) for a, b in _json.load(open('tools/v12/funcmap.json')).items()}
for _r in _json.load(open('tools/v12/survey.json'))['rows']:
    if _r['a10']:
        FN12[A12(int(_r['a12'], 16))] = int(_r['a10'], 16)
_lm0 = _s.get('log_map')


def lm(log_o, log_c):
    """1.2's gear-type init functions (0x1002a1f2's second argument) logged as their 1.0 twins"""
    out = []
    for e in log_o:
        if e[0] == 'stub_1002a1f2' and len(e[1]) == 2 and e[1][1] in TABLE12:
            e = (e[0], (e[1][0], TABLE12[e[1][1]]))
        out.append(e)
    outc = log_c
    return _lm0(out, outc) if _lm0 else (out, outc)


_s['log_map'] = lm
SPECS = [_s]
