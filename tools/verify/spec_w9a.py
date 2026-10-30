# Differential tests for round 9, letter a (see difftest.py and docs/checker.md): Text_InitStrings (0x100ae7c0) and
# the three text-table fillers it calls. Nothing is stubbed: the text lookup Sim_100b7834 runs for real on both
# sides (the original through its thunk, the C by name; its bsearch 0x10100620, the compare 0x100b6b87 and strcmp
# are the original code in the emulator), and so do the converted callees of Text_InitStrings.
import struct

MINE = ['sim/unsorted_1000/w9a_text.c', 'sim/unsorted_1002/w9a_text.c', 'sim/unsorted_1007/w9a_text.c',
        'sim/unsorted_100a/w9a_text.c']
# converted code the functions above call (and what that calls)
OTHERS = ['sim/unsorted_100b/w5y_misc.c', 'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_1007/w2l_fields.c',
          'sim/unsorted_100d/state.c', 'sim/unsorted_100b/w2g_misc.c', 'sim/unsorted_1009/w2h_misc.c',
          'sim/unsorted_1005/w2j_misc.c', 'sim/unsorted_1009/w2g_objects.c', 'sim/unsorted_100d/w2g_misc.c',
          'sim/unsorted_1003/w2g_misc.c']
SOURCES = MINE + OTHERS
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

TEXT_LO, TEXT_HI = 0x101c4280, 0x101c5300      # the text pointer globals (and the three text buffers)
TABLE = 0x1017d594                              # pointer to the text lookup table (count, entries)
NAMES = 0x10145592                              # 46 name records of 0xd6 bytes (Sim_10074710 writes them)
TEXTCHARS = b'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 _.,!?<>-'


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def rbytes(ctx, n):
    return bytes(ctx.rng.getrandbits(8) for _ in range(n))


def ustr(uc, addr, limit=400):
    out = b''
    while len(out) < limit:
        c = bytes(uc.mem_read(addr + len(out), 1))
        if c == b'\0':
            break
        out += c
    return out


def text(ctx, n):
    """A text of n characters (plus the terminator) in argument memory."""
    r = ctx.rng
    return ctx.buf(n + 1, bytes(r.choice(TEXTCHARS) for _ in range(n)) + b'\0')


def text_len(ctx):
    r = ctx.rng
    return r.choice([0, 1, r.randrange(2, 20), r.randrange(20, 0xd5), 0xd5, 0xd5 + r.randrange(1, 40)])


def fill_text_globals(ctx):
    """Random dwords in every text pointer global: the copy functions move them as they are."""
    ctx.m.uc.mem_write(TEXT_LO, rbytes(ctx, TEXT_HI - TEXT_LO))


def fill_name_records(ctx):
    ctx.m.uc.mem_write(NAMES, rbytes(ctx, 46 * 0xd6 + 0x40))


def g_copy(ctx):
    fill_text_globals(ctx)
    return []


# the 46 source globals of Sim_10074710, in the order of the original
NAME_SOURCES = [
    0x101c4e38, 0x101c4dfc, 0x101c4de8, 0x101c4e20, 0x101c4e10, 0x101c4ddc, 0x101c4dd8, 0x101c4de4, 0x101c4de0,
    0x101c4b80, 0x101c4b84, 0x101c4b64, 0x101c4b68, 0x101c4b70, 0x101c4b78, 0x101c4a34, 0x101c4b44, 0x101c4b54,
    0x101c4b60, 0x101c4b6c, 0x101c4b74, 0x101c4a30, 0x101c4b40, 0x101c4b50, 0x101c4b5c, 0x101c4f2c, 0x101c4f34,
    0x101c4f40, 0x101c4f4c, 0x101c4b4c, 0x101c4b58, 0x101c4f28, 0x101c4f30, 0x101c4f3c, 0x101c4f48, 0x101c4efc,
    0x101c4f08, 0x101c4f14, 0x101c4f20, 0x101c4f38, 0x101c4f44, 0x101c4ef8, 0x101c4f04, 0x101c4f10, 0x101c4f1c,
    0x101c4edc]


def g_names(ctx):
    r = ctx.rng
    fill_text_globals(ctx)
    fill_name_records(ctx)
    for a in NAME_SOURCES:
        w32(ctx, a, text(ctx, text_len(ctx)) if r.random() < 0.9 else r.choice([0x1014fc08, 0x1014fbc0]))
    return []


def literals(ctx):
    """The 630 text literals Text_InitStrings looks up: the `push imm32` before every read of [0x1017d594] in the
    original code (cached per machine)."""
    m = ctx.m
    lits = getattr(m, 'w9a_lits', None)
    if lits is None:
        code = bytes(m.uc.mem_read(0x100ae7c0, 0x100b25a7 - 0x100ae7c0))
        lits = []
        i = 0
        while True:
            i = code.find(b'\x68', i)
            if i < 0:
                break
            after = code[i + 5:i + 11]
            if after[:5] == b'\xa1\x94\xd5\x17\x10' or after[:2] in (b'\x8b\x0d', b'\x8b\x15') and after[2:6] == b'\x94\xd5\x17\x10':
                lits.append(struct.unpack('<I', code[i + 1:i + 5])[0])
                i += 5
            else:
                i += 1
        assert len(lits) == 630, len(lits)
        m.w9a_lits = lits
        m.w9a_names = {}
        for a in lits:
            m.w9a_names.setdefault(ustr(m.uc, a), a)
    return lits, m.w9a_names


def g_text(ctx):
    r = ctx.rng
    lits, names = literals(ctx)
    fill_text_globals(ctx)
    fill_name_records(ctx)
    c = r.random()
    if c < 0.12:
        table = 0                   # every lookup gives the literal itself
    else:
        pool = sorted(names)        # bytes order == strcmp order (unsigned, shorter prefix first)
        n = r.choice([1, 2, 5, 20, 60, 200, len(pool), len(pool)])
        chosen = set(r.sample(pool, min(n, len(pool))))
        for _ in range(r.choice([0, 0, 1, 3, 8])):          # names the code never asks for
            chosen.add(bytes(r.choice(TEXTCHARS) for _ in range(r.randrange(1, 12))))
        if r.random() < 0.3:
            chosen.add(b'')
        entries = sorted(chosen)
        arr = ctx.buf(8 * len(entries), 'zero')
        for i, nm in enumerate(entries):
            if nm in names and r.random() < 0.5:
                np_ = names[nm]                             # the literal itself as the entry's name
            else:
                np_ = ctx.buf(len(nm) + 1, nm + b'\0')
            if r.random() < 0.25:
                val = np_
            else:
                val = text(ctx, text_len(ctx))
            w32(ctx, arr + 8 * i, np_, val)
        table = ctx.buf(8, struct.pack('<2I', len(entries), arr))
    w32(ctx, TABLE, table)
    return []


SPECS = [
    dict(name='Sim_1000f560', orig=0x1000f560, c='Sim_1000f560', gen=g_copy, trials=300, void=True),
    dict(name='Sim_100204c4', orig=0x100204c4, c='Sim_100204c4', gen=g_copy, trials=300, void=True),
    dict(name='Sim_10074710', orig=0x10074710, c='Sim_10074710', gen=g_names, trials=400, void=True),
    dict(name='Text_InitStrings', orig=0x100ae7c0, c='Text_InitStrings', gen=g_text, trials=400, void=True),
]
