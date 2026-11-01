# Round 33: the sky and ground settings Sim_100a0063 against 1.2's 0x1008ceab, which reads four more keys from
# skygnd.par (hambient, l1, l2, gl: coloured light), makes the two light directions unit vectors and clamps the
# colours to 0..255. The 1.0 spec's lines get the new keys; sscanf is the checker's stand-in on both sides (now 50
# targets). The new values are C globals here (g_w33*), paired with 1.2's addresses; the unset locals of 1.2's frame
# get the C's start values (0 / empty) as leftovers, as the 1.0 spec does for 1.0's.
# Negative check: W12_NEG=1 builds the C with W12S_MUTATE (light 2's direction not normalised).
import os
exec(open('tools/verify/spec_w12_100a0063.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W12_NEG'):
    CFLAGS = list(CFLAGS) + ['-DW12S_MUTATE']
sscanf_stub(50)
# the format and target addresses differ between the two DLLs (1.2's own data): not compared in the log; what
# sscanf writes is compared as memory
for _k in range(1, 50):
    MASK[(0x100fd710, _k)] = 0
_g10 = g
UNSET12 = [0x254, 0x2cc, 8, 0x26c, 0x2d4, 0x29c, 0x234, 0x264, 0x2a4, 0x25c, 0x2e0, 0x298,
           0x230, 0x258, 0x260, 0x268, 0x270, 0x294]
C12 = [('g_w33Ambient', 0x10138d20, 12), ('g_w33Light1', 0x10138d38, 12), ('g_w33Light1Dir', 0x10138d2c, 12),
       ('g_w33Light2', 0x10138d58, 12), ('g_w33Light2Dir', 0x10138d4c, 12), ('g_w33Global', 0x10138d6c, 12)]


def g12(ctx):
    r = ctx.rng
    args = _g10(ctx)
    d = lambda: str(r.choice([0, 0x80, 255, 300, -5, r.randrange(-1000, 1000)])).encode()
    v = lambda: str(r.choice([0, 1, -1, 3, 100, -7, r.randrange(-300, 300)])).encode()
    lines = []
    for t in LINES['l']:
        if t.startswith(b'mission=') and r.random() < 0.8:
            extra = (b' hambient=' + b','.join(d() for _ in range(3)) +
                     b' l1=' + b','.join([d(), d(), d(), v(), v(), v()]) +
                     b' l2=' + b','.join([d(), d(), d(), v(), v(), v()]) +
                     b' gl=' + b','.join(d() for _ in range(3)))
            if r.random() < 0.3:
                extra = extra[:r.randrange(0, len(extra))]
            t = (t.split(b' #')[0] + extra) if b' #' in t else t + extra
        lines.append(t)
    LINES['l'] = lines
    for name, a12, n in C12:
        wb(ctx, ctx.m.syms[name], bytes(r.getrandbits(8) for _ in range(n)))
    return args


def to12_more(m):
    ebp = frame(1)
    for off in UNSET12:
        m.uc.mem_write(ebp - off, bytes(4 if off not in (0x254, 0x2cc) else 32))
    for name, a12, n in C12:
        m.uc.mem_write(A12(a12), bytes(m.uc.mem_read(m.syms[name], n)))


def from12_more(m):
    for name, a12, n in C12:
        m.uc.mem_write(m.syms[name], bytes(m.uc.mem_read(A12(a12), n)))


PAIRS12 = {0x100fd850: 0x100db910, 0x1009ff33: 0x1008cd7b, 0x10058549: 0x1004b249, 0x100fd710: 0x100db7d0}
SPECS = [v12_spec(dict(SPECS[0], gen=g12), 0x1008ceab, extra_pairs=PAIRS12, windows={0x1017d0b0: 32, 0x1017d0d0: 32},
                  to12_more=to12_more,
                  from12_more=from12_more)]
