# Round 12: 0x1001a4fa (three-mode handler of "key;n,name,n" records). Stubs on both sides: Sim_100cc6e0 /
# Sim_100cc8ba (the slot), Sim_100db770, Sim_100b38c5, Sim_100b399c, Sim_1000f029, Sim_100096ee, Sim_1000ee23.
# strchr, sscanf, atoi and Sim_100b8d5c run for real. The unset name buffer only reaches Sim_100b8d5c, which ignores
# it, so no leftover needs setting (DECISIONS.md).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1001/w12_text.c'])
helper(0x100cc6e0, 'Sim_100cc6e0', 0)
helper(0x100cc8ba, 'Sim_100cc8ba', 1)
helper(0x100db770, 'Sim_100db770', 1)
helper(0x100b38c5, 'Sim_100b38c5', 1)
helper(0x100b399c, 'Sim_100b399c', 1)
helper(0x1000f029, 'Sim_1000f029', 1)
helper(0x100096ee, 'Sim_100096ee', 1)
helper(0x1000ee23, 'Sim_1000ee23', 1)
ACTIVE = (0x100cc6e0, 0x100cc8ba, 0x100db770, 0x100b38c5, 0x100b399c, 0x1000f029, 0x100096ee, 0x1000ee23)
HEAP = {}


def heap(n, a):
    return HEAP['rec']


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, 0x1017e950, 0)
    mode = r.choice([0, 0, 0, 1, 1, 2, 3, -1])
    slot = newbuf(ctx, 8)
    rets(0x100cc6e0, r.getrandbits(32))
    rets(0x100cc8ba, slot)
    rets(0x100b38c5, r.choice([0xffffffff, 0, 3, r.randrange(0, 100)]))
    rets(0x100b399c, r.getrandbits(32))
    rets(0x100096ee, r.getrandbits(32))
    rec = newbuf(ctx, 0x20)
    HEAP['rec'] = r.choice([rec, rec, rec, 0])
    if mode in (1, 2):
        old = newbuf(ctx, 0x20)
        tgt = newbuf(ctx, 8)
        w32(ctx, tgt, r.choice([0, r.getrandbits(32)]))
        w32(ctx, old + 0xc, r.choice([0, tgt, tgt]))
        w32(ctx, old + 0x14, r.choice([0, 1, r.getrandbits(32)]))
        w32(ctx, slot, r.choice([0, old, old, old]))
    num = lambda: str(r.choice([0, 1, -5, 12345, r.randrange(-99999, 99999)])).encode()
    texts = [num() + b';' + num() + b',' + b'name' + b',' + num(), num() + b';' + num() + b',,' + num(),
             num(), num() + b';', num() + b';' + num(), b'abc;1,x,2', b'',
             num() + b';' + bytes(r.randrange(32, 127) for _ in range(r.randrange(0, 40)))]
    t = newbuf(ctx, 128)
    wb(ctx, t, r.choice(texts)[:100] + b'\0')
    return [mode & 0xffffffff, t]


SPECS = [dict(name='Sim_1001a4fa', orig=0x1001a4fa, c='Sim_1001a4fa', gen=g, trials=4000, api={'HeapAlloc': heap})]