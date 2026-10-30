# Round 22: 0x100918e6 and 0x1009221d, the beam (DECISIONS.md round 22). Sim_100d2669 is a logging stub that moves
# each point it gets (through its pointers); the driver slot [0x1014cb4c] (int3 padding) logs the four vertices whole.
# The original never writes their z (+8) nor the fields 0x1009ef50 refills: the spec zeroes its vertex locals, as
# the C does.
# Negative check: W22_NEG=1 leaves random bytes there instead, which the vertex log must catch.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/render/w22_items.c'])
NEG = bool(os.environ.get('W22_NEG'))
DRIVER = 0x1010d3e6
helper(DRIVER, None, 3, deref={1: 4 * 0x24})
helper(0x100d2669, 'Sim_100d2669', 3, deref={0: 4, 1: 4, 2: 4})
ACTIVE = (DRIVER, 0x100d2669)


def _project(uc, k, raw, eax):
    for i in range(3):
        uc.mem_write(raw[i], struct.pack('<f', ST['pts'][k % 2][i]))
    return 1


EFFECT[0x100d2669] = _project


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, 0x1014cb4c, DRIVER)
    ax, ay = r.uniform(0, 640), r.uniform(0, 480)
    kind = r.choice(['w', 'h', 'd', 'nan', 'r'])
    if kind == 'w':
        bx, by = ax + r.uniform(50, 200), ay + r.uniform(-20, 20)
    elif kind == 'h':
        bx, by = ax + r.uniform(-20, 20), ay + r.uniform(50, 200)
    elif kind == 'd':
        d = r.uniform(-100, 100)
        bx, by = ax + d, ay + r.choice([d, -d])
    elif kind == 'nan':
        bx, by = float('nan'), ay
    else:
        bx, by = r.uniform(0, 640), r.uniform(0, 480)
    ST['pts'] = [(ax, ay, r.uniform(1, 100)), (bx, by, r.uniform(1, 100))]
    obj = newbuf(ctx, 0x570)
    w32(ctx, obj + 0x566, r.choice([r.randrange(0, 256), r.getrandbits(32)]))
    ebp = frame(1)
    wb(ctx, ebp - 0x90, rb(ctx, 0x90) if NEG else bytes(0x90))
    return [r.choice([obj, obj, obj, 0])]


SPECS = [dict(name=n, orig=a, c=n, gen=g, trials=300 if NEG else 4000)
         for n, a in (('Sim_100918e6', 0x100918e6), ('Sim_1009221d', 0x1009221d))]
