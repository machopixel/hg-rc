# Round 14: 0x1005bb7a (POF model loader: a gear record from its design). Every callee is stubbed on both sides (the
# part tables 0x1004bf87..0x1004d344, Sim_1005ae58 (the POF segment found or not), Sim_100aa4f0, the debug print
# 0x10083495 (texts logged), Sim_1005b89b, Sim_1005b923, Sim_1004bce5 (adds to the top speed it is given),
# Sim_1004be11, Sim_1005b7e0, and the float answer Sim_1004bc65). Str_Copy runs for real. Every design has exactly one
# chassis part (kind 2) with a sub-kind 0..15, as all 41 shipped designs (DECISIONS.md round 14).
# Negative check: W14_NEG=1 gives designs without a chassis part and puts non-zero leftovers into the original's
# [ebp-0x14] / [ebp-0xc0] / [ebp-0xa8] (the C starts them at 0), which must mismatch.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1005/w14_pof.c'])
NEG = bool(os.environ.get('W14_NEG'))
ONE = ['1004ce50', '1004ce3d', '1004cdd1', '1004ce04', '1004cd9e', '1004cd8d', '1004cdf3', '1004cde2', '1004bfa9',
       '1004bf87', '1004c30d', '1004c325', '1004c362', '1004c3dd', '1004c426', '1004d330', '1004d344', '1004d26d',
       '1004d27a', '1005b923', '1005b7e0']
TWO = ['1004ce64', '1004cfb5', '1004d106', '1004c085', '1004c161', '1004c1c1', '1004c221', '1004c297', '1004bfbd',
       '1004c5c1', '1004c6f5', '1004c785', '1004c815', '1004c965', '1004ca99', '1004cb29', '1004cbb9', '1004cc49',
       '1004c3ad', '1004c4a1', '1004c4d1', '1005ae58', '100aa4f0', '1005b89b', '1004be11']
CALLS = [(int(a, 16), 'Sim_' + a, 1) for a in ONE] + [(int(a, 16), 'Sim_' + a, 2) for a in TWO] + [
    (0x1004c0e5, 'Sim_1004c0e5', 3), (0x1004c5b7, 'Sim_1004c5b7', 0), (0x1004c95b, 'Sim_1004c95b', 0),
    (0x1004bce5, 'Sim_1004bce5', 5), (0x10083495, None, 3)]
for a, c, n in CALLS:
    helper(a, c, n)
SARGS[0x10083495] = {0, 1, 2}
SARGS[0x1005ae58] = {1}
FSTUB[0x1004bc65] = ('Sim_1004bc65', 2)
ACTIVE = tuple(a for a, _, _ in CALLS)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def _top(uc, k, raw, eax):
    v = struct.unpack('<i', bytes(uc.mem_read(raw[0], 4)))[0]
    uc.mem_write(raw[0], struct.pack('<i', v + 7))


EFFECT[0x1004bce5] = _top


def g(ctx):
    r = begin(ctx, *ACTIVE)
    fstubs(ctx)
    ST['fvals'] = {0x1004bc65: [r.choice([fb(r.uniform(0.2, 3)), fb(1.0), 0])]}
    for a, _, _ in CALLS:
        if a not in (0x1005ae58, 0x1004d26d, 0x1004bce5, 0x10083495):
            rets(a, *[r.choice([0, 1, 2, 9, 0x2d, 0x2e, r.randrange(-50, 300)]) for _ in range(3)])
    rets(0x1005ae58, *[r.choice([0, 0x00c10000, 0x00c10100]) for _ in range(3)])
    rets(0x1004d26d, *[r.randrange(0, 6) for _ in range(3)])
    rets(0x1004bf87, r.choice([9, 1, 9]))
    rets(0x1004c30d, r.choice([0x2d, 0x2e, 3]))
    design = newbuf(ctx, 0x100)
    n = r.randrange(1, 12)
    kinds = [r.choice([1, 3, 4, 5, 6, 7] + list(range(8, 17)) + [0, 17]) for _ in range(n)]
    if not NEG:
        kinds[r.randrange(0, n)] = 2
    parts = newbuf(ctx, 0x6c * n)
    for i, k in enumerate(kinds):
        p = parts + 0x6c * i
        w32(ctx, p, k)
        w32(ctx, p + 4, r.randrange(0, 16) if k == 2 else r.randrange(0, 10))
        w32(ctx, p + 8, r.choice([0, 1, 5, 40, r.randrange(-100, 1000)]))
        w32(ctx, p + 0x64, r.randrange(0, 8), r.randrange(0, 8))
    w32(ctx, design + 0xc8, parts)
    w32(ctx, design + 0x8c + 0x24, n)
    w32(ctx, design + 0x8c + 0x38, r.getrandbits(8))
    obj = newbuf(ctx, 0xc60)
    unit = newbuf(ctx, 0x200)
    w32(ctx, obj, unit)
    w32(ctx, obj + 0x84, newbuf(ctx, 0x128 * 12))
    w32(ctx, obj + 0x88, newbuf(ctx, 0x3c * 17))
    w32(ctx, obj + 0x8c, newbuf(ctx, 0x24 * 12))
    w32(ctx, obj + 0x4d4, fb(r.uniform(-100, 3000)))
    w32(ctx, 0x1013abd8, r.choice([-150, -100, -99, -50, -30, -29, 0, 50, 51, 100]))
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    w32(ctx, 0x1014c99c, r.choice([-1, 0]))
    if NEG:
        fr = frame(2)
        for off in (0x14, 0xc0, 0xa8):
            w32(ctx, fr - off, r.choice([3, 1, 0x40, 7]))
    return [obj, design]


# Round 22: the C-side mount table of the dropped-gun fix (Sim_NoteWeaponMount) has no counterpart in the original.
SPECS = [dict(name='Sim_1005bb7a', orig=0x1005bb7a, c='Sim_1005bb7a', gen=g, trials=300 if NEG else 10000,
              ignore_syms={'g_w22Mounts': 512 * 16})]
