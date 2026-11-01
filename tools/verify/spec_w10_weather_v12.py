# Round 33, round 4: the two weather-emitter spawns, style 2 (1.0 0x100d965e) and style 1 (1.0 0x100d9064), against
# 1.2's 0x100beafe / 0x100be504, in 1.2's particle mode (g_w31Tex and the 3D mode set: the effect pool holds 1.2's
# 0x406-byte records, engine node +0x332, time +0x33e, velocity +0x34e, 16 points of 0x32 bytes from +0x12), on
# spec_w10_100d965e.py's generator and stubs (hooked at 1.2's callees too, logged under the 1.0 names). The pool
# allocation's two callbacks are logged as 1.0's thunks (1.2 0x100be7a6 / 0x100be8df = 1.0 0x100034e0 / 0x100034b8;
# 0x100be1aa / 0x100be2e3 = 0x10001659 / 0x1000320b). Sim_10084a94 (length) runs for real on both sides.
# One function per run (W10W_ONE=9064 for style 1). Negative check: W10W_NEG12=1 builds the C with 1.0's 32 points (W10W_MUTATE).
import os
exec(open('tools/verify/spec_w10_100d965e.py').read())
exec(open('tools/verify/v12_common.py').read())
SOURCES = closure(['sim/unsorted_100d/w10_misc.c', 'sim/unsorted_100d/w6h_calls.c'])
if os.environ.get('W10W_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW10W_MUTATE']

def g12(ctx):
    """spec_w10_100d965e.py's generator with the object's engine node at 1.2's +0x332, in 1.2's particle mode"""
    r = begin(ctx, 0x100d7ee0, 0x100d7f41, 0x10090c7d, 0x1009c929, 0x100aa39f, 0x100a9d47, 0x100aa799)
    ST['seed'] = r.getrandbits(16)
    w32(ctx, ctx.m.syms['g_w31Tex'], 1)
    w32(ctx, 0x1017b940, 1)
    w32(ctx, 0x1015dc78, r.getrandbits(32))
    cam = ctx.buf(0x48, b''.join(struct.pack('<I', fb(r.uniform(-1000, 1000))) for _ in range(18)))
    wf(ctx, cam + 0x40, r.choice([r.uniform(0, 3000), 1e9, 0.0]))
    w32(ctx, 0x1014cae0, cam)
    slot = newbuf(ctx, 0x20)
    obj = newbuf(ctx, 0x580)
    node = newbuf(ctx, 0x100)
    rec = newbuf(ctx, 0x60)
    w32(ctx, obj + 0x332, node)
    w32(ctx, node + 0x8c, rec)
    rets(0x100d7ee0, slot if r.random() < 0.9 else 0)
    rets(0x10090c7d, obj if r.random() < 0.9 else 0)
    p = [fb(r.uniform(-1000, 1000)) if r.random() < 0.95 else rfloat(r) for _ in range(3)]
    v = [fb(r.uniform(-300, 300)) if r.random() < 0.95 else rfloat(r) for _ in range(3)]
    return p + v + [r.getrandbits(32)]


CB = {A12(0x100be7a6): 0x100034e0, A12(0x100be8df): 0x100034b8, A12(0x100be1aa): 0x10001659,
      A12(0x100be2e3): 0x1000320b}


def lm(log_o, log_c):
    return [(e[0], tuple(CB.get(a, a) if isinstance(a, int) else a for a in e[1])) for e in log_o], log_c


PAIRS = {0x100d7ee0: 0x100bd380, 0x100d7f41: 0x100bd3e1, 0x10090c7d: 0x10079c54, 0x1009c929: 0x10089e69,
         0x100aa39f: 0x1009582f, 0x100a9d47: 0x100951d7, 0x100aa799: 0x10095c29, 0x1014cae0: 0x1011f9e8,
         0x1015dc78: 0x10130768}
SPECS = []
for name, f10, f12 in (('Sim_100d965e', 0x100d965e, 0x100beafe), ('Sim_100d9064', 0x100d9064, 0x100be504)):
    _s = v12_spec(dict(name=name, orig=f10, c=name, gen=g12, void=True, trials=2000), f12, extra_pairs=PAIRS,
                  name=name + '_v12')
    _s['log_map'] = lm
    SPECS.append(_s)
# one function per run: two v12 specs in one run would hook 1.2's callees twice (W10W_ONE=9064 for style 1)
SPECS = SPECS[1:] if os.environ.get('W10W_ONE') == '9064' else SPECS[:1]
