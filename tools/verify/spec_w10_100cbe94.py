# Round 10: 0x100cbe94 (nearest target). Sim_100cbe68 (steps the selection) and Sim_100cb4fb (accepts it) are
# stubbed on both sides (original address and C function): the stepper writes the next (selection +0xf0,
# distance +0xd8) pair of a sequence made by the generator into the unit, the same on both runs. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100c/w10_misc.c'])
helper(0x100cbe68, 'Sim_100cbe68', 1)
helper(0x100cb4fb, 'Sim_100cb4fb', 1)


def e_step(uc, k, raw, eax):
    seq = ST['seq']
    sel, dist = seq[k] if k < len(seq) else (0x1000, 0)
    unit = ST['unit']
    uc.mem_write(unit + 0xf0, struct.pack('<II', sel & 0xffffffff, 0))
    uc.mem_write(unit + 0xd8, struct.pack('<I', dist & 0xffffffff))
    return None


EFFECT[0x100cbe68] = e_step


def dist(r):
    x = r.random()
    if x < 0.7:
        return struct.unpack('<I', struct.pack('<f', r.uniform(0.0, 3000.0)))[0]
    if x < 0.8:
        return r.choice([0x7f7fc99e, 0x7f7fc99f, 0x7f7fc99d, 0, 0x80000000, 0x7fc00000, 0x7f800000])
    return rfloat(r)


def g(ctx):
    r = begin(ctx, 0x100cbe68, 0x100cb4fb)
    unit = newbuf(ctx, 0x200)
    ST['unit'] = unit
    idx = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, idx)
    w32(ctx, 0x109e8b40 + 4 * idx, unit)
    rec = newbuf(ctx, 0x510)
    w32(ctx, rec + 0x50c, r.choice([0, 1, 1, 2]))
    w32(ctx, unit + 0x28, rec)
    w32(ctx, unit + 0x54, newbuf(ctx, 0x50))
    w32(ctx, unit + 0xf0, r.getrandbits(32))
    # a sequence of selections: ids that repeat (so the loop comes back to the best), sometimes one with 0x1000
    ids = [r.randrange(0, 0x500) for _ in range(r.randrange(1, 6))]
    seq = []
    for _ in range(r.randrange(1, 14)):
        if r.random() < 0.06:
            seq.append((r.randrange(0, 0x10000) | 0x1000, dist(r)))
        else:
            seq.append((r.choice(ids) & ~0x1000, dist(r)))
    ST['seq'] = seq
    w32(ctx, 0x1012bc48, r.choice([dist(r), struct.unpack('<I', struct.pack('<f', 1000.0))[0]]))
    rets(0x100cb4fb, r.choice([0, 1]), r.choice([0, 1]))
    return []


SPECS = [dict(name='Sim_100cbe94', orig=0x100cbe94, c='Sim_100cbe94', gen=g, void=True, trials=3000)]
