# Controls round: 1.2's force-feedback request 0x100382fb (new in 1.2) against Sim_100382fb. 1.2's table
# (0x10110430, 0xc0 bytes) is kept at 1.0's 0x1013af00 in the C: the window is copied to 1.2's address before its run
# and back after it, so both sides are compared at 0x1013af00.
# Negative check: FF_NEG=1 builds the C with W2K_FF_MUTATE (kind 7 stores a wrong code).
import os
import struct

SOURCES = ['sim/unsorted_1004/w2k_flags.c', 'sim/unsorted_1004/fields.c']
CFLAGS = ['-DW2K_FF_MUTATE'] if os.environ.get('FF_NEG') else []
exec(open('tools/verify/v12_common.py').read())

TAB10, TAB12, TABN = 0x1013af00, 0x10110430, 0xc0
CODES = [0x1e, 0x21, 0x23, 0x2d, 0x32, 0x37, 0x3c, 0x78, 0xa, 0x14, 0x28]


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def g(ctx):
    r = ctx.rng
    tab = []
    for i in range(TABN // 4):
        tab.append(r.choice([0, 1, 0xffffffff, r.getrandbits(32), r.choice(CODES), r.randrange(0, 0x170),
                             r.randrange(0, 0x70)]))
    w32(ctx, TAB10, *tab)
    kind = r.choice([r.randrange(0, 18), r.randrange(0, 18), r.randrange(0, 18), 18, 0x12345, 0xffffffff, 0x80000000])
    a = r.choice([0, 1, -1, 0x167, 0x168, 0x169, -0x168, -0x169, 0x7fffffff, -0x80000000, r.randrange(-400, 400),
                  r.randrange(0, 0x168), r.getrandbits(32)])
    b = r.choice([0, 1, 99, 100, 101, 0xffffffff, 0x80000000, r.randrange(0, 0x80), r.getrandbits(32)])
    c = r.choice([0, 1, 99, 100, 101, 0xffffffff, 0x80000000, r.randrange(0, 0x80), r.getrandbits(32)])
    return [kind, a & 0xffffffff, b, c]


def to12(m):
    m.uc.mem_write(A12(TAB12), bytes(m.uc.mem_read(TAB10, TABN)))


def from12(m):
    m.uc.mem_write(TAB10, bytes(m.uc.mem_read(A12(TAB12), TABN)))


SPECS = [dict(name='Sim_100382fb', orig12=0x100382fb, c='Sim_100382fb', gen=g, to12=to12, from12=from12, void=True,
              trials=4000)]
