# Round 10: 0x10031447 (quadtree build, recursive). Stubbed on both sides: Sim_1003196f (face in cell: a hit
# pattern by call number, the y range written through its two pointers) and Sim_1003184b (hands out records of a
# pool made by the generator, all seven arguments logged). The recursion runs the C (or the original) for real.
# Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w10_misc.c'])
helper(0x1003196f, 'Sim_1003196f', 8)
helper(0x1003184b, 'Sim_1003184b', 7)


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def e_face(uc, k, raw, eax):
    hits = ST['hits']
    hit = hits[k] if k < len(hits) else 0
    if hit:
        uc.mem_write(raw[6], struct.pack('<I', ST['ys'][k % len(ST['ys'])][0]))
        uc.mem_write(raw[7], struct.pack('<I', ST['ys'][k % len(ST['ys'])][1]))
    return hit


def e_node(uc, k, raw, eax):
    # the record as Sim_1003184b fills it: the six bounds, the count, no children
    pool = ST['pool']
    if k >= len(pool):
        return 0
    uc.mem_write(pool[k], struct.pack('<7I', *raw[:7]) + bytes(16))
    return pool[k]


EFFECT[0x1003196f] = e_face
EFFECT[0x1003184b] = e_node


def g(ctx):
    r = begin(ctx, 0x1003196f, 0x1003184b)
    nf = r.choice([0, 1, 5, 50, 101, 120, 150])
    mesh = newbuf(ctx, 0x10)
    w16(ctx, mesh + 6, nf)
    w32(ctx, mesh + 8, 0x10)
    big = r.random() < 0.5
    hits = []
    for k in range(3000):
        if big and k < 105:
            hits.append(1)
        else:
            hits.append(1 if r.random() < r.choice([0.02, 0.3, 0.6]) else 0)
    ST['hits'] = hits
    ST['ys'] = [(fb(r.uniform(-50, 50)), fb(r.uniform(-50, 50))) if r.random() < 0.9 else (rfloat(r), rfloat(r))
                for _ in range(17)]
    if r.random() < 0.3:     # a child minimum of exactly [0x101112f8] (2^31): the node's range is cleared
        ST['ys'] = [(fb(2147483648.0), fb(2147483648.0))] * 17
    ST['pool'] = [newbuf(ctx, 0x2c + 4 * 101) for _ in range(r.choice([40, 40, 3, 0]))]
    lo = [r.uniform(-500, 0), r.uniform(-500, 0)]
    box = newbuf(ctx, 0x18)
    wf(ctx, box, lo[0], lo[0] + r.choice([r.uniform(0, 600), 0.0, -1.0]))
    wf(ctx, box + 0x10, lo[1], lo[1] + r.choice([r.uniform(0, 600), 0.0]))
    if r.random() < 0.03:
        w32(ctx, box + 4 * r.choice([0, 1, 4, 5]), rfloat(r))
    return [box, r.randrange(0, 4), mesh]


SPECS = [dict(name='Sim_10031447', orig=0x10031447, c='Sim_10031447', gen=g, trials=1500)]
