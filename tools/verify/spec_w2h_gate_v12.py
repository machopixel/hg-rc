# Round 33: 1.2's unit action gate (1.2 0x10010cd2, Sim12_10010cd2 in w2h_objects.c, with its tests 0x10010bb4,
# 0x10010bf4, 0x10010c72 and the setter 0x10010cb0 run for real on both sides) against 1.2's own code. The object is
# built in our layout (unit +0x28, control block +0x54) and shifted down by 4 for 1.2's run (its +0x24 / +0x50),
# back after it.
# Negative check: W2H_NEG12=1 builds the C with W2HG_MUTATE (the jets test ignored).
import os
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/v12_common.py').read())

SOURCES = closure(['sim/unsorted_1001/w2h_objects.c'])
if os.environ.get('W2H_NEG12'):
    CFLAGS = list(globals().get('CFLAGS', [])) + ['-DW2HG_MUTATE']
SHIFT = {}


def g(ctx):
    r = begin(ctx)
    obj = newbuf(ctx, 0x100)
    unit = newbuf(ctx, 0x600)
    ctl = newbuf(ctx, 0x60)
    w32(ctx, obj, r.choice([1, 9, 1, 9, 0, 2, 4, 10]))
    w32(ctx, obj + 0xc, r.choice([0, 1, 2]))
    w32(ctx, obj + 0x14, r.choice([0, 1, 2, 2, 3, 3]))
    w32(ctx, obj + 0x28, unit)
    w32(ctx, obj + 0x54, ctl)
    w32(ctx, 0x1017b8c8, r.choice([0, 1, 2]))
    w16(ctx, unit + 0x560, r.choice([0, 0x20, 0x2020, r.getrandbits(16)]))
    w16(ctx, unit + 0x562, r.choice([0, 2, 4, 8, 0x200, 0x400, 0xa, 0x408, 0x60e, r.getrandbits(16)]))
    w8(ctx, ctl + 0x55, r.choice([0, 0, 7]))
    SHIFT.clear()
    if r.random() < 0.03:
        return [0]
    SHIFT[obj] = 0x100
    return [obj]


def to12(m):
    m.uc.mem_write(A12(0x1014a568), bytes(m.uc.mem_read(0x1017b8c8, 4)))
    for buf, n in list(SHIFT.items()):
        old = bytes(m.uc.mem_read(buf, n))
        SHIFT[buf] = (n, old)
        m.uc.mem_write(buf + 0x18, old[0x1c:])


def from12(m):
    for buf, (n, old) in SHIFT.items():
        cur = bytes(m.uc.mem_read(buf, n))
        m.uc.mem_write(buf + 0x1c, cur[0x18:n - 4])
        m.uc.mem_write(buf + 0x18, old[0x18:0x1c])


SPECS = [dict(name='Sim12_10010cd2', orig12=0x10010cd2, c='Sim12_10010cd2', gen=g, trials=4000, to12=to12,
              from12=from12)]
