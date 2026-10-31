# Round 32, D: the effects switch Sim_10090d15 (1.0 0x10090d15, 1.2 0x10079cec), against 1.0 with the 1.0 record
# (0x57e bytes, node +0x552) and against 1.2's own code with 1.2's (0x406, node +0x332; the C with the Vulkan switch
# and the 3D mode on). Stubbed on both sides: Sim_100aa351 (the node's release), Sim_10090513 (the record's free) and
# the CRT sprintf (its format logged as text). The active list is made of nodes (+4 next, +8 index) in fresh memory;
# 1.2's addresses in the stubs' log are mapped back to 1.0's.
import os
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/fx/w32_pool.c', 'sim/render/w31_tex.c'])
if os.environ.get('W32_NEG'):
    CFLAGS = list(CFLAGS) + ['-DW32P_MUTATE']
helper(0x100aa351, 'Sim_100aa351', 1)
helper(0x10090513, 'Sim_10090513', 1)
helper(0x100fc780, None, 3, sargs=(1,))
NREC = 10


def gen(v12):
    rec, obj = (0x406, 0x332) if v12 else (0x57e, 0x552)

    def g(ctx):
        r = begin(ctx, 0x100aa351, 0x10090513, 0x100fc780)
        m = ctx.m
        w32(ctx, 0x1017b940, 1 if v12 else r.choice([0, 1]))
        w32(ctx, m.syms['g_w31Tex'], 1 if v12 else 0)
        for i in range(NREC):
            wb(ctx, 0x10217c20 + i * rec, rb(ctx, rec))
            w32(ctx, 0x10217c20 + i * rec + obj, r.choice([0, r.getrandbits(32), newbuf(ctx, 0x20)]))
        n = r.choice([0, 1, 2, 4, 7, NREC])
        first = 0
        prev = 0
        for k in range(n):
            nd = newbuf(ctx, 0x10)
            w32(ctx, nd + 4, 0)
            w32(ctx, nd + 8, r.randrange(0, NREC))
            if prev:
                w32(ctx, prev + 4, nd)
            else:
                first = nd
            prev = nd
        w32(ctx, 0x1017cc28, first)
        w32(ctx, 0x10217c00, r.getrandbits(32))
        w32(ctx, 0x1017cc24, r.choice([0, n, r.getrandbits(32)]))
        w32(ctx, 0x1014b9b8, r.choice([0, 1, r.getrandbits(32)]))
        return [r.choice([0xffffffff, 0, 0, 0, 1, r.getrandbits(32)])]
    return g


SPEC10 = dict(name='Sim_10090d15', orig=0x10090d15, c='Sim_10090d15', gen=gen(False), trials=600)
exec(open('tools/verify/v12_common.py').read())


def _v12():
    s = v12_spec(dict(SPEC10, gen=gen(True)), 0x10079cec, windows={0x10217c20: NREC * 0x406, 0x10215be0: 0x40},
                 skip={0x1014b9bc})
    inv = {}
    for a10, a12 in s['_v12_pairs'].items():
        inv.setdefault(A12(a12), a10)

    def lm(log_o, log_c):
        def fix(v):
            return inv.get(v, v) if isinstance(v, int) else v
        return [(e[0], tuple(fix(v) for v in e[1])) if isinstance(e[1], tuple) else e for e in log_o], log_c
    s['log_map'] = lm
    return s


SPECS = [SPEC10, _v12()]
