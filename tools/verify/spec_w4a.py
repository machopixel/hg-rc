# Differential tests for round 4, letter a: Windows heap and library wrappers (see difftest.py).
# The Windows imports are stubbed by the checker: it logs name and arguments of every call, compares
# the two logs, and answers with the handlers below (fixed, repeatable values).
SOURCES = ['sim/win/w4a_heap.c']
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

HEAP = 0x1017b8f8


def g_free(ctx):
    r = ctx.rng.random()
    return [0 if r < 0.3 else ctx.buf(16)]


def g_alloc(ctx):
    return [ctx.u32()]


def g_none(ctx):
    return []


def g_load(ctx):
    return [ctx.buf(24, 'text')]


API = {'HeapAlloc': lambda n, a: 0x00a30000 + 0x100 * n, 'LoadLibraryA': lambda n, a: 0x60000000 + 0x1000 * n}

SPECS = [
    dict(name='Heap_Free', orig=0x100092bd, c='Heap_Free', gen=g_free, trials=200, void=True, api=API),
    dict(name='Heap_AllocZero', orig=0x10028559, c='Heap_AllocZero', gen=g_alloc, trials=200, api=API),
    dict(name='Sim_10027d5a', orig=0x10027d5a, c='Sim_10027d5a', gen=g_none, trials=100, void=True, api=API),
    dict(name='Lib_Load_1003c569', orig=0x1003c569, c='Lib_Load_1003c569', gen=g_load, trials=200, api=API),
]
