# Round 14, interpreted (DECISIONS.md): 0x1001a706 (a unit following a path). Stubs on both sides: Sim_100cc6e0 /
# Sim_100cc8ba (the record slot), sscanf 0x100fd710 (fills the %[^,] fields), the interpolator starts Sim_10047b76 /
# Sim_10047c92, the key and unit lookups, Sim_100096ee, the node helpers, and the float answers Sim_10047bcd,
# Sim_1008486e and Sim_10047cf2; HeapAlloc hands out a block. Crt_StrChr, Crt_AToI and Str_CompareNoCase run for real
# (locale handle [0x1017e950] = 0). The task texts have three non-empty words of up to 8 characters, the shipped range
# (DECISIONS.md round 14). The CRT asin of the original against the platform math of the C (libm()): exact but for the
# pitch, which may differ within the stated tolerance.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/util/w10_interp.c'])
CALLS = [
    (0x100cc6e0, 'Sim_100cc6e0', 0), (0x100cc8ba, 'Sim_100cc8ba', 1), (0x100fd710, 'Sim_100fd710', 5),
    (0x10047b76, 'Sim_10047b76', 5), (0x10047c92, 'Sim_10047c92', 6), (0x100db770, 'Sim_100db770', 1),
    (0x100b38c5, 'Sim_100b38c5', 1), (0x100b399c, 'Sim_100b399c', 1), (0x100096ee, 'Sim_100096ee', 1),
    (0x100a9d47, 'Sim_100a9d47', 4), (0x100aa0da, 'Sim_100aa0da', 5), (0x100aa799, 'Sim_100aa799', 1),
]
for a, c, n in CALLS:
    helper(a, c, n)
SARGS[0x100fd710] = {0, 1}
FSTUB[0x10047bcd] = ('Sim_10047bcd', 1)
FSTUB[0x1008486e] = ('Sim_1008486e', 2)
FSTUB[0x10047cf2] = ('Sim_10047cf2', 1)
ACTIVE = tuple(a for a, _, _ in CALLS)
PATHS = [b'landing', b'pengine', b'pseg2', b'PSEG3', b'pseg7']
T = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def _sscanf(uc, k, raw, eax):
    text = _cstr(uc, raw[0])
    n = 0
    for dst, word in zip(raw[2:], text.split(b',')[:3]):
        if not word:
            break
        uc.mem_write(dst, word + b'\0')
        n += 1
    return n


def heap(n, args, uc=None):
    return T['heap'] if T['heap_ok'] else 0


EFFECT[0x100fd710] = _sscanf


def text(ctx, s):
    a = newbuf(ctx, len(s) + 1)
    wb(ctx, a, s + b'\0')
    return a


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    libm(ctx)
    ST['fvals'] = {0x10047bcd: [fb(r.uniform(-3000, 3000)) for _ in range(3)],
                   0x1008486e: [fb(r.uniform(-180, 180))],
                   0x10047cf2: [fb(r.uniform(-360, 360)) for _ in range(3)]}
    w32(ctx, 0x1017e950, 0)
    T['heap'] = newbuf(ctx, 0x88)
    T['heap_ok'] = r.random() < 0.9
    npaths = r.randrange(0, len(PATHS) + 1)
    w32(ctx, 0x1015d17c, npaths)
    for k in range(npaths):
        p = 0x10a00040 + k * 0x744
        wb(ctx, p, PATHS[k] + b'\0')
        n = r.choice([0, 1, 2, 2, 3, 5])
        w32(ctx, p + 0x40, n)
        for i in range(max(n, 1)):
            q = p + 0x44 + i * 0x1c
            w32(ctx, q, fb(r.uniform(-5000, 5000)), fb(r.uniform(-500, 500) * r.choice([1, 1, 60])),
                fb(r.uniform(-5000, 5000)), fb(r.uniform(-90, 90)), fb(r.uniform(-400, 400)), fb(r.uniform(-90, 90)),
                r.choice([0, 1, 50, 300, 1000]))
    rec = newbuf(ctx, 0x88)
    unit = newbuf(ctx, 0x10)
    w32(ctx, unit, r.choice([0, 0x00c70000, 0x00c70000]))
    w32(ctx, rec, r.choice([0, unit, unit, unit]))
    w32(ctx, rec + 8, r.choice([0] + [0x10a00040 + k * 0x744 for k in range(max(npaths, 1))] * 2))
    w32(ctx, rec + 0xc, r.randrange(0, 5000))
    w32(ctx, rec + 0x10, r.randrange(0, 3000))
    w32(ctx, rec + 0x14, r.choice([0, 1]))
    w32(ctx, rec + 0x18, r.choice([0, 1, 2]))
    for off in (0x50, 0x54, 0x64, 0x68, 0x78, 0x7c):
        w32(ctx, rec + off, fb(r.uniform(-720, 720)))
    slot = newbuf(ctx, 4)
    w32(ctx, slot, r.choice([rec, rec, rec, 0]))
    rets(0x100cc8ba, slot)
    rets(0x100b38c5, r.choice([-1, 0, 3]))
    rets(0x100b399c, 0x00c80000)
    rets(0x100096ee, r.choice([0, 0x00c90000, 0x00c90000]))
    rets(0x100db770, r.randrange(0, 40), r.randrange(0, 40))
    mode = r.choice([1, 1, 1, 1, 0, 0, -1, 2])
    how = r.choice([b'loop', b'REPEAT', b'oneshot', b'repeat', b'once'])
    turn = r.choice([b'rotate', b'norotate', b'Rotate'])
    name = r.choice(PATHS + [b'pseg9'])
    s = b'%d;%s,%s,%s' % (r.randrange(0, 60), how, turn, name)
    if r.random() < 0.05:
        s = b'%d' % r.randrange(0, 60)
    return [mode & 0xffffffff, text(ctx, s), r.randrange(0, 12000)]


SPECS = [dict(name='Sim_1001a706', orig=0x1001a706, c='Sim_1001a706', gen=g, trials=15000, fp=True, tol=1.2e-7,
              api={'HeapAlloc': heap})]
