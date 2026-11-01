# Round 32: Sim_1006ad98 (part destroyed) in 1.2 mode (g_w31Tex) against 1.2's 0x1005ab68, which adds the
# network-only branch (a kind-4 unit's part 2 destroyed by the local player: 16 shockwaves and the tally, part not
# marked). The 1.0 spec's generator with kind-4 units and part 2 more often; the unit's node pointer is +0x48 in our
# unit record and +0x44 in 1.2's (its moved layout), copied there for 1.2's run and put back after it. The tally is
# stubbed on both sides (1.0 Score_RecordKill = 1.2 0x10059e9d; 1.2's tally changes are not ported, DECISIONS.md).
# Negative check: W10_NEG=1 builds the C with W10N_MUTATE (one shockwave fewer at the second node).
import os
exec(open('tools/verify/spec_w10_1006ad98.py').read())
exec(open('tools/verify/v12_common.py').read())
if os.environ.get('W10_NEG'):
    CFLAGS = list(CFLAGS) + ['-DW10N_MUTATE']
_g10 = g


def g12(ctx):
    args = _g10(ctx)
    r = ctx.rng
    unit = U32_(ctx, args[1])
    if r.random() < 0.5:
        w32(ctx, unit, 4)
        w32(ctx, 0x1015ec58, r.choice([0, 1, 1, 1]))
        args[2] = r.choice([2, 2, 2, 1, 7])
        if r.random() < 0.7:
            args[0] = G32_(ctx, 0x1017b8c8)
    w32(ctx, ctx.m.syms['g_w31Tex'], 1)
    ST['unit'] = unit
    return args


def U32_(ctx, a):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(a, 4)))[0]


def G32_(ctx, a):
    return U32_(ctx, a)


def to12_more(m):
    u = ST['unit']
    ST['u44'] = bytes(m.uc.mem_read(u + 0x44, 4))
    m.uc.mem_write(u + 0x44, bytes(m.uc.mem_read(u + 0x48, 4)))


def from12_more(m):
    m.uc.mem_write(ST['unit'] + 0x44, ST['u44'])


# the aligner mispairs some callees here (1.2's new branch shifts the listing): the pairs given
PAIRS12 = {0x100203cf: 0x100176df, 0x1006abf6: 0x1005a9c6, 0x1009c929: 0x10089e69, 0x1009c969: 0x10089ea9,
           0x100a9cfb: 0x1009518b, 0x100ab054: 0x100964e4, 0x100aafe4: 0x10096474, 0x1006a12d: 0x10059e9d,
           0x10092f9d: 0x1007c1a1}
SPECS = [v12_spec(dict(SPECS[0], gen=g12), 0x1005ab68, extra_pairs=PAIRS12, to12_more=to12_more,
                  from12_more=from12_more)]
