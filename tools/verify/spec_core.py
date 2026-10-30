# Differential tests for src/sim/core (see difftest.py).
import struct
SOURCES = ['sim/core/timer.c']

def tables(ctx):
    """Random timer tables: some used slots, some free, an end marker at slot 63."""
    rng = ctx.rng
    for base in (0x10153180, 0x10153280):
        slots = [rng.choice([0, 0, 0, rng.getrandbits(32), 1, 0xfffffff0]) for _ in range(63)]
        if rng.random() < 0.5:
            slots[rng.randrange(0, 63)] = 0xFFFFFFFF
        ctx.m.uc.mem_write(base, struct.pack('<64I', *(slots + [0xFFFFFFFF])))
    for a in (0x10153380, 0x10153384, 0x1015317c):
        ctx.m.uc.mem_write(a, struct.pack('<I', ctx.rng.choice([0, 1, 7, 1000, ctx.rng.getrandbits(32)]) if a != 0x1015317c else ctx.rng.getrandbits(10)))

def tnum(ctx):
    return ctx.rng.choice([0, 1, 5, 62, 0x80, 0x81, 0x85, 0xBE, ctx.rng.randrange(0, 0x100)])

def g_one(ctx):
    tables(ctx); return [tnum(ctx)]
def g_alloc(ctx):
    tables(ctx); return [ctx.rng.choice([0, 0x80, 0x100, 0x180, ctx.rng.getrandbits(32)])]
def g_set(ctx):
    tables(ctx); return [tnum(ctx), ctx.u32()]
def g_pause(ctx):
    tables(ctx); return [ctx.rng.choice([0x80, 0x100, 0x180, 0, ctx.rng.getrandbits(16)]), ctx.rng.choice([0, 1, 0xffff, ctx.rng.getrandbits(16)])]
def g_none(ctx):
    tables(ctx); return []

SPECS = [
    dict(name='Timer_Step', orig=0x100e4550, c='Timer_Step', gen=g_none, void=True, nop=[0x100e4550, 0x100e4577]),
    dict(name='Timer_Alloc', orig=0x100e4579, c='Timer_Alloc', gen=g_alloc),
    dict(name='Timer_Elapsed', orig=0x100e45e7, c='Timer_Elapsed', gen=g_one),
    dict(name='Timer_Reset', orig=0x100e4622, c='Timer_Reset', gen=g_one, void=True),
    dict(name='Timer_SetElapsed', orig=0x100e465d, c='Timer_SetElapsed', gen=g_set, void=True),
    dict(name='Timer_Free', orig=0x100e469b, c='Timer_Free', gen=g_one, void=True),
    dict(name='Timer_Pause', orig=0x100e46d0, c='Timer_Pause', gen=g_pause, void=True),
]
