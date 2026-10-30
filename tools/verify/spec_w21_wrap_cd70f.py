# Round 21: the objective-line wrap of 0x100cd70f (the current objective, w9g) (DECISIONS.md round 21). It copies cut + 1 bytes of a long
# objective without a terminating zero and then searches the buffer for its last space, so the original's search runs
# on into stack leftovers; the C now stops at the copy. This runs spec_w9g.py's generator with the stack below the
# caller, where the buffer lies, filled with random bytes that hold no space and end in a zero: with no space in the
# leftovers the original finds the same space as the C, which is the wrap E shows.
# Negative check: W21W_NEG=1 lets the filler hold spaces, which moves the original's cut and must be caught.
import os
exec(open('tools/verify/spec_w9g.py').read())
W21W_NEG = bool(os.environ.get('W21W_NEG'))


def g_w21wrap(ctx):
    args = g_d70f(ctx)
    r = ctx.rng
    top = 0x00800000 - 0x100 - 4 * 3 - 8     # the frame pointer of the call
    n = 0x600
    if W21W_NEG:
        data = bytes(r.choice(b' a b c Z9_.') for _ in range(n))
    else:
        data = bytes(r.choice([x for x in range(1, 256) if x != 0x20]) for _ in range(n))
    z = r.randrange(0x40, n)                      # a zero somewhere in the leftovers
    ctx.m.uc.mem_write(top - n, data[:z] + b'\0' + data[z + 1:])
    return args


SPECS = [dict(name='Sim_100cd70f', orig=0x100cd70f, c='Sim_100cd70f', gen=g_w21wrap, trials=300 if W21W_NEG else 4000, void=True)]
