# Round 14: 0x1005cd4a (gear record from a design; the old part format, gear designs go to Sim_1005bb7a). Stubs on both
# sides: Sim_1005bb7a, Sim_1005ae58 (POF segment found or not; name logged), Sim_100aa4f0, Sim_1005b89b, Sim_1005b7e0
# and Sim_1005e295. Str_Copy, strstr and the DLL's sqrt run for real. Design names are NUL-terminated.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1005/w14_pof.c'])
CALLS = [(0x1005bb7a, 'Sim_1005bb7a', 2), (0x1005ae58, 'Sim_1005ae58', 2), (0x100aa4f0, 'Sim_100aa4f0', 2),
         (0x1005b89b, 'Sim_1005b89b', 2), (0x1005b7e0, 'Sim_1005b7e0', 1), (0x1005e295, 'Sim_1005e295', 3)]
for a, c, n in CALLS:
    helper(a, c, n)
SARGS[0x1005ae58] = {1}
ACTIVE = tuple(a for a, _, _ in CALLS)
KINDS = list(range(0, 0x2e))


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def g(ctx):
    r = begin(ctx, *ACTIVE)
    rets(0x1005ae58, *[r.choice([0, 0x00c10000, 0x00c10100]) for _ in range(3)])
    rets(0x1005b89b, r.randrange(0, 0x400), r.randrange(0, 0x400))
    design = newbuf(ctx, 0x100)
    w32(ctx, design + 0x8c, r.choice([2, 0, 0, 1, 3]))
    nm = r.choice([b'infantry', b'Strider', b'scout', b'Hunter', b'xinfa', b'', b'jaguar'])
    wb(ctx, design + 0x90, nm + b'\0')
    n = r.randrange(0, 12)
    w32(ctx, design + 0xb0, n)
    parts = newbuf(ctx, 0x6c * max(n, 1))
    for i in range(n):
        p = parts + 0x6c * i
        k = r.choice(KINDS + [1, 3, 0x20, 0x21, 0x2c, 0x16, 0x1b, 0xd])
        w32(ctx, p, k)
        w32(ctx, p + 4, r.choice([0, 1, 2, 3, 4, 5, 6, 7, 9, 0x29, r.randrange(0, 46)]))
        w32(ctx, p + 8, r.randrange(-20, 400))
        w32(ctx, p + 0xc, r.choice([0, 4, 100, 10000, r.randrange(-100, 5000)]))
        w32(ctx, p + 0x10, r.randrange(0, 21))
        for j in range(20):
            w32(ctx, p + 0x14 + 4 * j, r.choice([0, 0, 1, 2, 5, 7, 0xe, r.randrange(0, 0x1c)]))
        w32(ctx, p + 0x64, r.choice([0, 1, 2, 3, 4, 16]))
        w32(ctx, p + 0x68, r.randrange(0, 4))
    w32(ctx, design + 0xc8, parts)
    obj = newbuf(ctx, 0xc60)
    unit = newbuf(ctx, 0x200)
    w32(ctx, obj, unit)
    w32(ctx, obj + 0x84, newbuf(ctx, 0x128 * 12))
    w32(ctx, obj + 0x88, newbuf(ctx, 0x3c * 17))
    w32(ctx, obj + 0x8c, newbuf(ctx, 0x24 * 40))
    w32(ctx, obj + 0x4d4, fb(r.uniform(-100, 3000)))
    w32(ctx, 0x1013abd8, r.choice([-150, -100, -99, -50, -30, -29, 0, 50, 51, 100]))
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    w32(ctx, 0x1014c99c, r.choice([-1, 0]))
    if r.random() < 0.3:                     # a weapon kind whose link factor is -1 (none ships; covers the branch)
        w32(ctx, 0x10145538 + 4 * r.randrange(0, 2) + 0xd6 * r.randrange(0, 10), 0xffffffff)
    return [obj, design]


SPECS = [dict(name='Sim_1005cd4a', orig=0x1005cd4a, c='Sim_1005cd4a', gen=g, trials=10000)]
