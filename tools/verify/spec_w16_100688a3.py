# Round 16: 0x100688a3, the round 10 spec (tools/verify/spec_w10_100688a3.py) with more trials before it is hooked,
# and with NaNs and infinities in the node's position / angles and the radius. 0x100687a3 (converted in round 16) stays a logging stub here: its arguments are compared.
exec(open('tools/verify/spec_w10_100688a3.py').read())
SNAN = [0x7fc00001, 0xffc12345, 0x7f800000]   # quiet NaNs and infinity (the emulator does not quiet signalling NaNs on an x87 load, the real FPU does; they never occur in play)
_g10 = g


def g(ctx):
    args = _g10(ctx)
    r = ctx.rng
    obj, ent = args[0], args[1]
    if ent and r.random() < 0.3:
        node = r32(ctx, r32(ctx, ent) + 0x48)
        for off in (0x70, 0x74, 0x78) + tuple(range(0x4c, 0x7c, 4)):
            if r.random() < 0.2:
                w32(ctx, node + off, r.choice(SNAN))
    if obj and r.random() < 0.2:
        w32(ctx, obj + 0x53c, r.choice(SNAN))
    return args


SPECS[0]['gen'] = g
SPECS[0]['trials'] = 8000
