# Round 28: SystemError 0x100bc280 (src/sim/platform/w28_error.c), pure game logic: the message text and the
# choice by code. The text lookup (Ini_FindSection, Ini_GetValue answering a text) and the three reactions
# (Sim_100bc535 warning, Sim_100bc451 shut-down, Sim_100bc47e fatal box) are stubs on both sides; sprintf, strcat,
# strlen and vsprintf run as they are. Every code 0..0x5a and above, with and without details (formats with
# %s / %d / %x and their arguments). W28_NEG=1 builds W28E_MUTATE (no shut-down before the fatal box).
exec(open('tools/verify/w10_common.py').read())

if os.environ.get('W28_NEG'):
    CFLAGS.append('-DW28E_MUTATE')

SOURCES = closure(['sim/platform/w28_error.c'])
helper(0x100531e0, 'Ini_FindSection', 1, sargs=(0,))
helper(0x10053323, 'Ini_GetValue', 2, sargs=(0,))
helper(0x100bc535, 'Sim_100bc535', 1, sargs=(0,))
helper(0x100bc451, 'Sim_100bc451', 0)
helper(0x100bc47e, 'Sim_100bc47e', 1, sargs=(0,))
ACTIVE = tuple(HELPERS)

FORMATS = [b'%s', b'%d', b'file %s line %d', b'%x / %s', b'no details', b'%s: %s (%d)']


def cs(ctx, b):
    p = newbuf(ctx, len(b) + 1)
    wb(ctx, p, b + b'\0')
    return p


def g_err(ctx):
    r = begin(ctx, *ACTIVE)
    rets(0x10053323, r.choice([cs(ctx, b'Out of memory'), cs(ctx, b'Unable to open file'), cs(ctx, b'')]))
    code = r.choice([r.randrange(0, 0x5a), r.randrange(0, 0x5a), r.getrandbits(32), 0x58, 0x59, 1])
    wb(ctx, 0x1017d618, rb(ctx, 0x100))
    fmt = r.choice([0, 0, cs(ctx, r.choice(FORMATS))])
    extra = [r.choice([cs(ctx, b'HEAVYG.PRJ'), cs(ctx, b'x' * 40), r.randrange(-500, 500) & 0xffffffff])
             for _ in range(3)]
    if fmt:
        txt = bytes(ctx.m.uc.mem_read(fmt, 40)).split(b'\0')[0]
        k = 0
        for spec_ in txt.split(b'%')[1:]:
            if spec_[:1] == b's':
                extra[k] = cs(ctx, r.choice([b'HEAVYG.PRJ', b'M09_SCN1', b'']))
            elif spec_[:1] in b'dx':
                extra[k] = r.randrange(-500, 500) & 0xffffffff
            k += 1
    return [code, fmt] + extra


SPECS = [dict(name='Sim_100bc280', orig=0x100bc280, c='Sim_100bc280', gen=g_err, trials=8000, void=True)]
