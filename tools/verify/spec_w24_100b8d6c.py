# Round 24: 0x100b8d6c, load a resource from the cache or as a loose file (DECISIONS.md round 24). The cache loader
# 0x100ba7d5, the path builder Sim_100474db (its name logged as text), the file loader 0x10046ac4 (writes the size and
# the data pointer), _close, the block allocator 0x100b659a and the cache release / forget are logging stubs; memcpy
# runs for real. The checker's stack starts zeroed, so the original's two unset locals (the key word on the cache
# path, the 8 name bytes the extension is appended to) read 0 here, the C's fixed values.
# Negative check (mutation): W24_NEG=1 builds the C with W24R_MUTATE (the loose-file key word 0, not -1).
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_100b/w24_resource.c'])
NEG = bool(os.environ.get('W24_NEG'))
if NEG:
    CFLAGS = CFLAGS + ['-DW24R_MUTATE']
helper(0x100ba7d5, None, 3, sargs=(0,))
helper(0x100474db, 'Sim_100474db', 1, sargs=(0,))
helper(0x10046ac4, None, 4, sargs=(0,))
helper(0x101049c0, None, 1)
helper(0x100b659a, None, 2)
helper(0x100bb2b0, None, 2, sargs=(0,))
helper(0x100b9203, None, 2, sargs=(0,))
ST_FILE = {}


def _cache(uc, k, raw, eax):
    if eax:
        uc.mem_write(raw[2], struct.pack('<I', ST_FILE['size']))
    return None


def _file(uc, k, raw, eax):
    if eax != 0xffffffff:
        uc.mem_write(raw[1], struct.pack('<I', ST_FILE['size']))
        uc.mem_write(raw[2], struct.pack('<I', ST_FILE['data']))
    return None


EFFECT[0x100ba7d5] = _cache
EFFECT[0x10046ac4] = _file
ACTIVE = (0x100ba7d5, 0x100474db, 0x10046ac4, 0x101049c0, 0x100b659a, 0x100bb2b0, 0x100b9203)


def g(ctx):
    r = begin(ctx, *ACTIVE)
    key = newbuf(ctx, 0x10)
    wb(ctx, key, b'\x55\x55' + r.choice([b'TERRAIN1', b'GEAR\0\0\0\0', b'ABCDEFGH']))
    ext = ctx.buf(1, r.choice([b'.mgi\0', b'.3di\0', b'.cpi\0', b'.longer\0', b'\0', b'ab\0']))
    data = newbuf(ctx, 0x40)
    for i in range(0x10):
        w32(ctx, data + 4 * i, r.getrandbits(32))
    ST_FILE['size'] = r.choice([0, 1, 7, 0x40, 0xfffffff0])
    ST_FILE['data'] = data
    rets(0x100ba7d5, r.choice([0, data]))
    rets(0x10046ac4, r.choice([0xffffffff, 5]))
    rets(0x100474db, ctx.buf(1, b'path_name' + bytes(1)))
    rets(0x100b659a, r.choice([0, newbuf(ctx, 0x40)]))
    size = newbuf(ctx, 4)
    w32(ctx, size, r.getrandbits(32))
    copy = 0
    if r.random() < 0.5:
        copy = newbuf(ctx, 4)
        w32(ctx, copy, r.choice([0, 1, 0x12345]))
    if ST_FILE['size'] > 0x40 and ST_FILE['size'] != 0xfffffff0:
        ST_FILE['size'] = 0x40
    return [key, r.getrandbits(32), ext, r.getrandbits(32), size, copy]


SPECS = [dict(name='Sim_100b8d6c', orig=0x100b8d6c, c='Sim_100b8d6c', gen=g, trials=300 if NEG else 4000)]
