# Round 12: 0x10019fe1 (two-mode handler of "key;a,b,c,d" motion records). Stubs on both sides: Sim_100cc6e0 /
# Sim_100cc8ba (the slot), Sim_100db770, Sim_100b38c5, Sim_100b399c, Sim_100096ee, Sim_100aa127 (the three floats
# logged), Sim_100aa799, and sscanf (the DLL's float conversion needs C runtime start-up tables the checker does not
# have, so a stub parses the fields: a float per comma-separated field, stopping at the first that does not parse).
# strchr and atoi run for real. The four sscanf locals [ebp-0x10..-0x1c] get the chosen
# start value 0.0 as the original's stack leftovers.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1001/w12_text.c'])
helper(0x100cc6e0, 'Sim_100cc6e0', 0)
helper(0x100cc8ba, 'Sim_100cc8ba', 1)
helper(0x100db770, 'Sim_100db770', 1)
helper(0x100b38c5, 'Sim_100b38c5', 1)
helper(0x100b399c, 'Sim_100b399c', 1)
helper(0x100096ee, 'Sim_100096ee', 1)
helper(0x100aa127, 'Sim_100aa127', 5)
helper(0x100aa799, 'Sim_100aa799', 1)
ACTIVE = (0x100cc6e0, 0x100cc8ba, 0x100db770, 0x100b38c5, 0x100b399c, 0x100096ee, 0x100aa127, 0x100aa799, 0x100fd710)
sscanf_stub(6)
EBP = frame(3)
HEAP = {}


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def heap(n, a):
    return HEAP['rec']


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, 0x1017e950, 0)
    mode = r.choice([0, 0, 0, 1, 1, 1, 2, -1])
    slot = newbuf(ctx, 8)
    rets(0x100cc6e0, r.getrandbits(32))
    rets(0x100cc8ba, slot)
    rets(0x100b38c5, r.choice([0xffffffff, 0, 3]))
    rets(0x100b399c, r.getrandbits(32))
    rets(0x100096ee, r.getrandbits(32))
    rec = newbuf(ctx, 0x20)
    HEAP['rec'] = r.choice([rec, rec, rec, 0])
    if mode == 1:
        old = newbuf(ctx, 0x20)
        tgt = newbuf(ctx, 8)
        w32(ctx, tgt, r.choice([0, r.getrandbits(32)]))
        w32(ctx, old, r.choice([0, tgt, tgt, tgt]))
        w32(ctx, old + 8, *[r.choice([fb(r.uniform(-100, 100)), 0, fb(1.5)]) for _ in range(3)])
        w32(ctx, old + 0x14, r.choice([fb(181.0), fb(r.uniform(-500, 500)), fb(180.0), 0]))
        w32(ctx, old + 0x18, r.getrandbits(32) if r.random() < 0.2 else r.randrange(0, 100000))
        w32(ctx, slot, r.choice([0, old, old, old]))
    f = lambda: r.choice(['0', '1.5', '-2', '0.0001', '1e-9', str(r.uniform(-1000, 1000)), str(r.randrange(-50, 50))])
    num = lambda: str(r.choice([0, 7, -3, r.randrange(-9999, 9999)]))
    n = r.randrange(0, 6)
    texts = [num() + ';' + ','.join(f() for _ in range(4)), num() + ';' + ','.join(f() for _ in range(n)),
             num(), num() + ';', num() + ';x,1', '']
    t = newbuf(ctx, 128)
    wb(ctx, t, r.choice(texts).encode()[:100] + b'\0')
    w32(ctx, EBP - 0x1c, 0, 0, 0, 0)         # the chosen start values (0.0) as the original's leftovers
    return [mode & 0xffffffff, t, r.choice([r.randrange(0, 200000), r.getrandbits(32)])]


SPECS = [dict(name='Sim_10019fe1', orig=0x10019fe1, c='Sim_10019fe1', gen=g, trials=4000, api={'HeapAlloc': heap})]