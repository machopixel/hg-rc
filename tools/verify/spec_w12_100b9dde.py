# Round 12: 0x100b9dde (the language letter from lang.ini). fopen, fgets and fclose of the DLL are stubs on both sides
# (fgets writes the trial's line); strstr, strlen and toupper run for real. The original's unset reads get the
# chosen start values as stack leftovers (DECISIONS.md): the line buffer [ebp-0x104] empty, the pointer [ebp-4] at
# the buffer.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100b/w12_lang.c'])
helper(0x100fd850, None, 2, sargs=(0, 1))
helper(0x100fe770, None, 3)
helper(0x100fd760, None, 1)
ACTIVE = (0x100fd850, 0x100fe770, 0x100fd760)
STACK_TOP = 0x00800000
EBP = STACK_TOP - 0x100 - 4 - 4         # no arguments: the return address, then push ebp
LINE = {}


def _fgets(uc, k, raw, eax):
    text = LINE.get('text')
    if text is None:
        return 0
    uc.mem_write(raw[0], text[:raw[1] - 1] + b'\0')
    return raw[0]


EFFECT[0x100fe770] = _fgets


def g(ctx):
    r = begin(ctx, *ACTIVE)
    w32(ctx, 0x1017e950, 0)
    rets(0x100fd850, r.choice([0, 0x00c00000, 0x00c00000, 0x00c00000]))
    rets(0x100fd760, 0)
    words = [b'language =', b'language =E', b'language = G \n', b'language =F  ', b'language=E', b'foo=bar', b'x', b'',
             b'language =' + bytes(r.randrange(32, 127) for _ in range(r.randrange(0, 8))) + b'\n' * r.randrange(3),
             bytes(r.randrange(1, 256) for _ in range(r.randrange(0, 300)))]
    LINE['text'] = r.choice(words + [None])
    wb(ctx, EBP - 0x104, rb(ctx, 0x104))
    w8(ctx, EBP - 0x104, 0)                  # the chosen start values, as the original's leftovers
    w32(ctx, EBP - 4, EBP - 0x104)
    return []


SPECS = [dict(name='Sim_100b9dde', orig=0x100b9dde, c='Sim_100b9dde', gen=g, trials=3000)]