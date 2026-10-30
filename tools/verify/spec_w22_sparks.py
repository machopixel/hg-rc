# Round 22: 0x1009353e (fountain) and 0x1009596a (cloud), sparks like 0x100945b8 (DECISIONS.md round 22). Callees are logging stubs: 0x100a9cfb
# writes a base position, Sim_100d2669 answers visible or not and moves the point (both through their pointers), the
# library 0x100e84f0 and the driver slot [0x1014cb4c] (int3 padding; the four vertices logged whole) log their
# arguments. The original never writes z (+8) and the fields 0x1009ef50 refills: the spec zeroes its vertex locals,
# as the C does.
# Negative check: W22_NEG=1 leaves random bytes there instead, which the vertex log must catch.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w22_items.c'])
NEG = bool(os.environ.get('W22_NEG'))
DRIVER = 0x1010d3e6
helper(DRIVER, None, 3, deref={1: 4 * 0x24})
helper(0x100a9cfb, 'Sim_100a9cfb', 4)
helper(0x100d2669, 'Sim_100d2669', 3, deref={0: 4, 1: 4, 2: 4})
helper(0x100e84f0, None, 4)
ACTIVE = (DRIVER, 0x100a9cfb, 0x100d2669, 0x100e84f0)


def _base(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[1 + i], struct.pack('<f', ST['base'][i]))
    return None


def _project(uc, k, raw, eax):
    vis, dz = ST['proj'][k % len(ST['proj'])]
    if vis:
        uc.mem_write(raw[2], struct.pack('<f', dz))
    return vis


EFFECT[0x100a9cfb] = _base
EFFECT[0x100d2669] = _project


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, 0x1014cb4c, DRIVER)
    ST['base'] = [r.uniform(-100, 100) for _ in range(3)]
    ST['proj'] = [(r.choice([0, 1, 1]), r.choice([r.uniform(200, 4000), r.uniform(4000, 50000), -2.0])) for _ in range(5)]
    obj = newbuf(ctx, 0x580)
    for i in range(16):
        for o in (0x12, 0x16, 0x1a, 0x1e, 0x22, 0x26):
            wf(ctx, obj + o + i * 0x2a, r.uniform(-300, 300))
    w32(ctx, obj + 0x56a, r.choice([0, 1, 3, r.randrange(0, 8)]))   # size = this * 2000 / depth, kept small
    w32(ctx, 0x1017b940, r.choice([0, 1, 1]))
    w32(ctx, obj + 0x566, r.randrange(0, 256))
    wf(ctx, 0x1012b980, r.choice([100.0, 1000.0, 5000.0]))
    wf(ctx, 0x1012b958, r.choice([1.0, 256.0]))
    ebp = frame(1)
    wb(ctx, ebp - 0x90, rb(ctx, 0x90) if NEG else bytes(0x90))
    return [r.choice([obj, obj, obj, 0])]


SPECS = [dict(name=n, orig=a, c=n, gen=g, trials=300 if NEG else 4000)
         for n, a in (('Sim_1009353e', 0x1009353e), ('Sim_1009596a', 0x1009596a))]
