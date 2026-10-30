# Round 12: 0x100a0063 (sky and ground settings from skygnd.par). Stubs on both sides: fopen, Sim_1009ff33 (hands out
# the trial's lines), sscanf (w12_common.py, the float conversion cannot run in the checker), Sim_10058549.
# Str_CopyUpper, strchr, Sim_1010d240 (strnicmp) and Str_Copy run for real. The original's unset sscanf locals get the
# chosen start values (0 / empty) as stack leftovers.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_100a/w12_sky.c'])
helper(0x100fd850, None, 2, sargs=(0, 1))
helper(0x1009ff33, 'Sim_1009ff33', 3)
helper(0x10058549, 'Sim_10058549', 1)
sscanf_stub(27)
ACTIVE = (0x100fd850, 0x1009ff33, 0x10058549, 0x100fd710)
EBP = frame(1)
LINES = {}
UNSET = [0x2b8, 0x278, 4, 0x254, 0x2b0, 0x27c, 0x228, 0x250, 0x284, 0x24c]


def _line(uc, k, raw, eax):
    if k >= len(LINES['l']):
        return 0
    uc.mem_write(raw[1], LINES['l'][k][:raw[2] - 1] + b'\0')
    return 1


EFFECT[0x1009ff33] = _line


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, 0x1017e950, 0)
    rets(0x100fd850, r.choice([0, 0x00c00000, 0x00c00000, 0x00c00000, 0x00c00000]))
    key = r.choice([b'm1aa', b'M2BB', b'tr01', b'x'])
    arg = newbuf(ctx, 16)
    wb(ctx, arg, key + b'\0')
    d = lambda: str(r.choice([0, 0x80, 255, 300, -5, r.randrange(-1000, 1000)])).encode()
    f = lambda: r.choice([b'1.5', b'0', b'-2.25', b'100', str(round(r.uniform(-50, 50), 3)).encode()])
    w = lambda: r.choice([b'sky1.bmp', b'h', b'tex_' + str(r.randrange(100)).encode()])

    def full(m):
        return (b'mission=' + m + b' globalintensity=' + d() + b' hardwareintensity=' + d() + b' hardwareambient=' + d()
                + b' gintensity=' + d() + b' gambient=' + d() + b' skytexture=' + w() + b' horizontexture=' + w()
                + b' hardwarehorizon=' + w() + b' horizontile=' + f() + b' skycolor=' + b','.join(d() for _ in range(6))
                + b' fogswitch=' + d() + b' fogcolor=' + b','.join(d() for _ in range(4)) + b' skyspeed=' + f()
                + b' density=' + b','.join(f() for _ in range(3)))
    lines = []
    for _ in range(r.randrange(0, 5)):
        m = r.choice([key, key.upper(), b'zz99', b'm1ab', key[:3]])
        t = full(m)
        if r.random() < 0.4:
            t = t[:r.randrange(0, len(t))]
        lines.append(r.choice([t, t, b'# comment', b'', t + b' # tail', b'#' + t]))
    LINES['l'] = lines
    w32(ctx, 0x1017b940, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b944, r.choice([0, 0, 1]))
    w32(ctx, 0x1014cb38, r.choice([0, 0, 1]))
    w32(ctx, 0x1014cae0, newbuf(ctx, 0x40))
    w32(ctx, 0x1014c9c8, struct.unpack('<I', struct.pack('<f', r.uniform(1, 500)))[0])
    w32(ctx, 0x1017d108, r.getrandbits(32))
    wb(ctx, EBP - 0x248, bytes(32))          # the chosen start values as the original's leftovers
    wb(ctx, EBP - 0x2ac, bytes(32))
    for off in UNSET:
        w32(ctx, EBP - off, 0)
    return [arg]


SPECS = [dict(name='Sim_100a0063', orig=0x100a0063, c='Sim_100a0063', gen=g, void=True, trials=4000)]