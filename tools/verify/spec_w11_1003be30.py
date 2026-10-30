# Round 11: 0x1003be30 (fetch the network library's functions). GetProcAddress is the checker's Windows stub
# (arguments logged; it answers a value made from the call number and the name, or 0).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1003/w11_sound.c'])


def gpa(n, args, uc):
    if ST['zero'] and n % 7 == 3:
        return 0
    return (0x70000000 + n * 0x10 + len(ustr(uc, args[1]))) & 0xffffffff


def g(ctx):
    r = begin(ctx)
    ST['zero'] = r.random() < 0.3
    w32(ctx, 0x1015eb68, r.choice([0, 0x10000000, r.getrandbits(32)]))
    return []


SPECS = [dict(name='Sim_1003be30', orig=0x1003be30, c='Sim_1003be30', gen=g, void=True, trials=500,
              api={'GetProcAddress': gpa})]