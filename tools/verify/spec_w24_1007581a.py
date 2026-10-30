# Round 24: 0x1007581a, the in-mission menu draw (DECISIONS.md round 24). Each trial puts the same random key code into
# the original's [ebp-0x128] (frame of a call from this checker: esp at entry STACK_TOP - 0x108, ebp 4 lower) and into
# the C's g_w24MenuKey, so the first-frame path (selection on a text line, a digit key) is proven with the carried
# value. Stubs: the menu getter Sim_100765f6, the key step Sim_10075753 (logged; writes a step and sometimes a menu
# state), the sound Sim_1000ed52, the three text routines (text logged), the shape drawers, the underlines
# Sim_1008d65d / Sim_1008d6f7 (title logged), font height and character widths, the stack push / pop and the item
# routine 0x10077272. itoa, strlen, strncpy, strrchr and strcpy run for real. Labels of width + 2 or more characters
# (none ships) are included: the checker's stack starts zeroed, so the original's line is zero before its first wrap.
# Negative check (mutation): W24_NEG=1 builds the C with W24M_MUTATE (the carried key code dropped).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1007/w24_menu.c'])
NEG = bool(os.environ.get('W24_NEG'))
if NEG:
    CFLAGS = CFLAGS + ['-DW24M_MUTATE']
helper(0x100765f6, 'Sim_100765f6', 1)
helper(0x10075753, 'Sim_10075753', 4)
helper(0x1000ed52, 'Sim_1000ed52', 6)
for a in (0x100f2f01, 0x100f1e10, 0x100ecea6):
    helper(a, None, 6, sargs=(4,))
helper(0x100f0205, None, 5)
helper(0x100e91dc, None, 5)
helper(0x1008d65d, 'Sim_1008d65d', 5)
helper(0x1008d6f7, 'Sim_1008d6f7', 6, sargs=(1,))
helper(0x100ecce0, None, 1)
helper(0x100eccf3, None, 2)
helper(0x100f2cec, 'Sim_100f2cec', 2)
helper(0x10076624, 'Sim_10076624', 2)
helper(0x100765a7, 'Sim_100765a7', 1)
helper(0x10077272, None, 6)
STACK_TOP = 0x00800000      # difftest.py's call stack
SLOT_KEY = STACK_TOP - 0x108 - 4 - 0x128
ST_M = {}


def _step(uc, k, raw, eax):
    uc.mem_write(raw[3], struct.pack('<i', ST_M['steps'][k % 3]))
    if ST_M['state'][k % 3] is not None:
        uc.mem_write(raw[2], struct.pack('<i', ST_M['state'][k % 3]))


EFFECT[0x10075753] = _step
ACTIVE = (0x100765f6, 0x10075753, 0x1000ed52, 0x100f2f01, 0x100f1e10, 0x100ecea6, 0x100f0205, 0x100e91dc,
          0x1008d65d, 0x1008d6f7, 0x100ecce0, 0x100eccf3, 0x100f2cec, 0x10076624, 0x100765a7, 0x10077272)
WORDS = [b'Attack', b'My', b'Target', b'Flank', b'Get', b'to', b'cover', b'Return', b'Formation', b'Abcdefghijkl']


def label(r, width):
    words = [r.choice(WORDS) for _ in range(r.randrange(1, 7))]
    t = b' '.join(words)
    return t


def g(ctx):
    r = begin(ctx, *ACTIVE)
    s = newbuf(ctx, 0x100)
    for o in range(0, 0x100, 4):
        w32(ctx, s + o, r.randrange(-50, 400))
    w32(ctx, s, r.choice([0x5000, 0x5000, 0x5000, 0]))
    w32(ctx, s + 4, r.choice([0, 4, 8, 0xc, 2]))
    w32(ctx, s + 0x20, r.choice([0, 0x7000]))
    w32(ctx, s + 0x24, r.choice([0, 0x41]))
    w32(ctx, s + 0x34, r.choice([0, 0x42]))
    w32(ctx, s + 0x48, r.choice([0x6000, 0x6000, 0x6000, 0]))
    w32(ctx, s + 0x54, r.randrange(0, 10))
    lang = r.randrange(0, 2)
    w32(ctx, 0x101c64f0, lang)
    width = (25, 27)[lang]
    n = r.randrange(1, 12)
    menu = newbuf(ctx, 0x18 + 0x14 * n)
    w32(ctx, menu, r.choice([0, 0, 1, 2, 2, 3, 6]))
    w32(ctx, menu + 4, r.choice([ctx.buf(1, b'TITLE\0'), ctx.buf(1, b'TITLE\0'), 0]))
    w32(ctx, menu + 0xc, n)
    kinds = [r.choice([0, 1, 2, 3, 4, 5, 6]) for _ in range(n)]
    w32(ctx, menu + 0x10, r.randrange(0, n))
    if r.random() < 0.3:
        kinds[r32(ctx, menu + 0x10)] = 3        # the selection on a text line: the first-frame path
    for i in range(n):
        it = menu + 0x18 + 0x14 * i
        w32(ctx, it, kinds[i])
        w32(ctx, it + 4, r.choice([ctx.buf(1, label(r, width) + b'\0'), 0]) if r.random() < 0.9 else 0)
        w32(ctx, it + 8, r.choice([0, 0x10077272]))
        w32(ctx, it + 0xc, r.getrandbits(32))
        w32(ctx, it + 0x10, r.choice([0, 0x8000]))
    rets(0x100765f6, r.choice([menu, menu, menu, 0]))
    rets(0x100ecce0, r.choice([8, 9, 12]))
    rets(0x100eccf3, r.choice([5, 6]))
    rets(0x100f2cec, r.choice([5, 7]))
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    w32(ctx, 0x1015dc18, r.choice([0, 0, 0x30, 0x31, 0x32, 0x35, 0x39, 0x1b, 0xd, 0xc6, 0xc7, 0x209, 9]))
    ST_M['steps'] = [r.choice([0, 0, 1, -1]) for _ in range(3)]
    ST_M['state'] = [r.choice([None, None, 3, 4, 5]) for _ in range(3)]
    key = r.choice([0, 0x31, 0x1b, 0xd, 9, 0xc6, r.getrandbits(32)])
    w32(ctx, SLOT_KEY, key)
    w32(ctx, ctx.m.syms['g_w24MenuKey'], key)
    return [r.choice([s] * 30 + [0])]


SPECS = [dict(name='Sim_1007581a', orig=0x1007581a, c='Sim_1007581a', gen=g, void=True,
              ignore_syms={'g_w24MenuKey': 4}, trials=300 if NEG else 6000)]
