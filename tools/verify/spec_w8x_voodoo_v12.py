# Round 33: the deferred draw list against 1.2's own code, its 3DFX / PowerVR mode (g_w33Voodoo = 1.2 [0x1014b954]):
#   Sim_100a4c45 (flush) against 1.2 0x10091065: around the last group, the renderer's state 0xe (1.0's 0xf) off and
#     on when [0x1014cb1c] is not 1;
#   Sim_100a4d8e (fill and sort) against 1.2 0x100911ec: with [0x1014cb1c] 0, 1.2's new partition 0x10090a85
#     (Sim12_10090a85: kinds 0x8000 / 0x9000 of the objects' word +2 apart) before the depth sort, and with fog on
#     the renderer's state 1 (1.0's 2) set to 0x200000.
# Stubbed on both sides: the depth sort (1.0 0x100a4700 = 1.2 0x10090940, logs its range), the draw of an entry
# (0x100a539e = 0x1009185b), the scene offer Sim_100a4933 (= 0x10090d53: appends entries {object, key} to the list),
# the list set-up Sim_100a88db (= 0x1009409b), the visible cells (0x10087d83 = 0x100720e3) and the renderer's state
# slot. Sim_100787f4 (= 0x10065cc4, identical) runs for real. 1.2's state codes are logged in 1.0's numbering.
# Negative checks: W8X_NEG=1 builds the C with W7OV_MUTATE (the flush's first state call left out) and W8XV_MUTATE
# (0x9000 not special in the partition's second scan).
import os
exec(open('tools/verify/w32_common.py').read())

SOURCES = closure(['sim/unsorted_100a/w7o_misc.c', 'sim/unsorted_100a/w8x_scene.c'])
if os.environ.get('W8X_NEG'):
    CFLAGS = list(CFLAGS) + ['-DW7OV_MUTATE', '-DW8XV_MUTATE']
STATE = 0x1010d3f2                      # stand-in in the state slot
helper(STATE, None, 2)
stub12(0x100a4700, None, 2, 0x10090940)
stub12(0x100a539e, None, 1, 0x1009185b)
stub12(0x100a4933, 'Sim_100a4933', 1, 0x10090d53)
stub12(0x100a88db, 'Sim_100a88db', 0, 0x1009409b)
stub12(0x10087d83, None, 1, 0x100720e3)
LISTS10, LISTS12 = 0x1036f2a0, 0x1028c960
GLOBALS = [
    (0x1014cae0, 0x1011f9e8, 4),        # the camera
    (0x1017b940, 0x1014a5e8, 4),        # 3D mode
    (0x1017d130, 0x1014b9c0, 4),
    (0x1017d134, 0x1014b9c4, 4),
    (0x1017d138, 0x1014b9c8, 4),
    (0x1017d13c, 0x1014b9cc, 4),
    (0x1017d168, 0x1014b9ec, 4),
    (0x101c9ea0, 0x10189128, 4),
    (0x101c9ea4, 0x1018912c, 4),
    (0x101cc2ac, 0x1018b528, 4),
    (0x101c672c, 0x10188d0c, 4),
    (0x101c673c, 0x10188d10, 4),
    (0x1017d108, 0x1014b9b0, 4),        # fog
    (0x1014cae8, 0x1011f9f0, 0x68),     # the render settings ([0x1014cb1c] at +0x34)
    (0x109e3d40, 0x107ee400, 4),        # the state slot
    (LISTS10, LISTS12, 4 * 6),
    ('g_w33Voodoo', 0x1014b954, 4),
]
TO12_MORE.append(lambda m: ST.__setitem__('run12', True))
FROM12_MORE.append(lambda m: ST.__setitem__('run12', False))


def e_offer(uc, k, raw, eax):
    """Sim_100a4933 appends a few entries {object, key} at list[base + count] and sometimes says stop."""
    cnt = 0x1017d138 if not ST.get('run12') else A12(0x1014b9c8)
    base = 0x1017d134 if not ST.get('run12') else A12(0x1014b9c4)
    tab = ur32(uc, 0x1017d168 if not ST.get('run12') else A12(0x1014b9ec))
    n, b = ur32(uc, cnt), ur32(uc, base)
    for j in range(ST['adds'][k % len(ST['adds'])]):
        if b + n >= ST['cap']:
            break
        obj, key = ST['entries'][(k * 3 + j) % len(ST['entries'])]
        uc.mem_write(tab + 8 * (b + n), struct.pack('<II', obj, key))
        n += 1
    uc.mem_write(cnt, struct.pack('<I', n))
    return ST['stops'][k % len(ST['stops'])]


EFFECT[0x100a4933] = e_offer


def world(ctx, r):
    w32(ctx, ctx.m.syms['g_w33Voodoo'], r.choice([0, 1, 1]))
    w32(ctx, 0x1014cb1c, r.choice([0, 0, 1, 4, 2]))
    w32(ctx, 0x1017d108, r.choice([0, 1]))
    w32(ctx, 0x109e3d40, STATE)


def g_flush(ctx):
    r = begin(ctx, STATE, 0x100a539e)
    world(ctx, r)
    a = r.choice([0, 0, 1, 2, 5])
    b = r.choice([0, 0, 1, 3, 6])
    c = min(r.choice([0, 0, 1, 2, b]), a + b)      # the last group lies inside the list
    w32(ctx, 0x1017d134, a)
    w32(ctx, 0x101c9ea0, b)
    w32(ctx, 0x1017d130, c)
    tab = newbuf(ctx, 8 * 20)
    w32(ctx, 0x1017d168, tab)
    return []


def g_fill(ctx):
    r = begin(ctx, STATE, 0x100a4700, 0x100a539e, 0x100a4933, 0x100a88db, 0x10087d83)
    world(ctx, r)
    objs = []
    for _ in range(6):
        o = newbuf(ctx, 0x10)
        w16(ctx, o + 2, r.choice([0x8000, 0x9000, 0x1000, 0, 0xa000]) | r.getrandbits(12))
        objs.append(o)
    ST['entries'] = [(r.choice(objs + [0]), r.getrandbits(32)) for _ in range(12)]
    ST['adds'] = [r.choice([0, 1, 2, 3, 5]) for _ in range(5)]
    ST['stops'] = [r.choice([0, 0, 0, 1]) for _ in range(5)]
    ST['cap'] = 40
    ST['run12'] = False
    w32(ctx, 0x1017d134, r.choice([0, 0, 1, 3]))
    w32(ctx, 0x1017d168, newbuf(ctx, 8 * 52) + 16)  # room on both sides: the scans read one entry past each end
    w32(ctx, 0x1017b940, r.choice([0, 0, 1]))
    n = r.choice([0, 1, 2, 4])
    for i in range(n):
        lst = newbuf(ctx, 0x40)
        a = ctx.u32() | 1
        w32(ctx, lst + 4, a)
        w32(ctx, lst + 8, r.choice([a ^ 2, a ^ 0x100, a ^ 6]))       # non-zero and not +4 (else both loop for ever)
        os_ = [newbuf(ctx, 0x40) for _ in range(r.choice([0, 1, 2, 4]))]
        w32(ctx, lst + 0x20, os_[0] if os_ else 0)
        for x, y in zip(os_, os_[1:] + [0]):
            w32(ctx, x + 0x20, y)
        w32(ctx, LISTS10 + 4 * i, lst)
    w32(ctx, LISTS10 + 4 * n, 0)
    return []


def lm_states(spec):
    base = spec['log_map']

    def lm(log_o, log_c):
        log_o, log_c = base(log_o, log_c)
        out = []
        for e in log_o:
            if e[0] == 'stub_%x' % STATE and e[1] and isinstance(e[1][0], int):
                e = (e[0], (e[1][0] + 1,) + tuple(e[1][1:]))
            out.append(e)
        return out, log_c
    return lm


def spec_s(name, f12, gen, trials):
    s = spec12(name, f12, gen, trials, void=True)
    g0 = s['gen']

    def g2(ctx):
        a = g0(ctx)
        s['log_map'] = lm_states(s)
        return a
    s['gen'] = g2
    return s


SPECS = [spec_s('Sim_100a4c45', 0x10091065, g_flush, 2000),
         spec_s('Sim_100a4d8e', 0x100911ec, g_fill, 3000)]
