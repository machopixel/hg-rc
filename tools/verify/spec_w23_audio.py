# Round 23: 0x1001efbe / 0x1001f14f, the audio option setters (DECISIONS.md round 23). The music start / stop
# (Sim_1001f335 / Sim_1001f399) and the CD volume (Sim_1009ebeb) are logging stubs; the live settings and the saved
# record are compared as memory. No local is read before it is written.
# Negative check (mutation): W23_NEG=1 builds the C with W23A_MUTATE (option 4 sets bit 2 instead of bit 1).
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1001/w23_audio.c'])
NEG = bool(os.environ.get('W23_NEG'))
if NEG:
    CFLAGS = CFLAGS + ['-DW23A_MUTATE']
helper(0x1001f335, 'Sim_1001f335', 0)
helper(0x1001f399, 'Sim_1001f399', 0)
helper(0x1009ebeb, 'Sim_1009ebeb', 0)
ACTIVE = (0x1001f335, 0x1001f399, 0x1009ebeb)


def g(ctx, nargs):
    r = begin(ctx, *ACTIVE)
    rec = newbuf(ctx, 0x20)
    w32(ctx, 0x1015d1b8, rec)
    for a in (0x101321b0, 0x101321b4, 0x101321b8, 0x101321bc, 0x101321c0):
        w32(ctx, a, r.getrandbits(32))
    for o in (0, 4, 8, 0xc):
        w32(ctx, rec + o, r.choice([0, 1, r.getrandbits(32)]))
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    which = r.choice([-1, 0, 1, 2, 3, 4, 5, 2, 4])
    return [which, r.choice([0, 1, r.getrandbits(32)])][:nargs]


SPECS = [dict(name='Sim_1001efbe', orig=0x1001efbe, c='Sim_1001efbe', gen=lambda ctx: g(ctx, 2), void=True,
              trials=300 if NEG else 3000),
         dict(name='Sim_1001f14f', orig=0x1001f14f, c='Sim_1001f14f', gen=lambda ctx: g(ctx, 1), void=True,
              trials=300 if NEG else 3000)]
