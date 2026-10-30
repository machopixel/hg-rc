# Round 11: 0x1005f6c6 (language setup). Stubbed on both sides: Lang_ReadIni, Sim_100b69d0, Text_InitStrings,
# Sim_10083495 (the original passes it a text it ignores: no argument logged), and the three language-DLL
# functions (IND1 / IND2 / IND3, cdecl; the language word of IND3 masked to 16 bits, see the C). LoadLibraryA,
# GetProcAddress (answers the stub by the name asked for) and FreeLibrary are Windows stubs.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1005/w11_lang.c'])
helper(0x1005f4e0, 'Lang_ReadIni', 2, sargs=(0, 1))
helper(0x100b69d0, 'Sim_100b69d0', 1, sargs=(0,))
helper(0x100ae7c0, 'Text_InitStrings', 0)
helper(0x10083495, 'Sim_10083495', 0)
helper(IND1, None, 1)
helper(IND2, None, 0)
helper(IND3, None, 4)
MASK[(IND3, 0)] = 0xffff
ACTIVE = tuple(HELPERS)
NAMES = {}


def gpa(n, args, uc):
    name = ustr(uc, args[1]) if isinstance(args[1], int) else b''
    return NAMES.get(name, 0)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    if not NAMES:
        for a, ind in ((0x1014427c, IND1), (0x1014428c, IND2), (0x101442a0, IND3)):
            NAMES[ustr(ctx.m.uc, a)] = ind
    ST['lib'] = r.choice([0, 0x77000000, 0x77000000])
    ST['free'] = r.choice([0, 1])
    rets(0x100b69d0, r.getrandbits(32))
    rets(IND2, r.choice([0, 1, 5]))
    rec = newbuf(ctx, 0x20)
    w32(ctx, rec + 0xe, r.choice([0, 1]))
    w32(ctx, 0x10144230, rec)
    return []


SPECS = [dict(name='Sim_1005f6c6', orig=0x1005f6c6, c='Sim_1005f6c6', gen=g, void=True, trials=2000,
              api={'LoadLibraryA': lambda n, a: ST['lib'], 'GetProcAddress': gpa,
                   'FreeLibrary': lambda n, a: ST['free']})]