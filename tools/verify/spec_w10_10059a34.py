# Round 10: 0x10059a34 (model record from a saved image). HeapAlloc answers buffers of a pool made by the generator
# (by call number); HeapAlloc / HeapFree are logged by the checker. Sim_1004b8b0 and Mem_Move run for real. Exact.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1005/w10_misc.c'])


def g(ctx):
    r = begin(ctx)
    ST['pool'] = [newbuf(ctx, 0x1800) for _ in range(6)]
    w32(ctx, 0x1017b910, r.choice([0, 0x00AA0000]))
    w32(ctx, 0x1017b900, r.choice([0x00AB0000, 0x00AC0000]))
    w32(ctx, 0x1017b8f8, 0x00AD0000)
    nparts = r.choice([0, 1, 2, 3])
    nrec = r.choice([0, 1, 2])
    head = bytearray(rb(ctx, 0x88))
    head[0:4] = struct.pack('<I', r.choice([0, 0, 0, 0, 1]))
    head[0x84:0x88] = struct.pack('<I', nparts)
    img = bytes(head)
    for i in range(nparts):
        p = bytearray(rb(ctx, 0x438))
        p[8:12] = struct.pack('<I', r.choice([0, 1, 2, 5, 9, 0x30, r.getrandbits(8)]))
        p[0x54:0x58] = struct.pack('<I', r.choice([0, r.randrange(0, 4)]))
        p[0x78:0x7c] = struct.pack('<I', r.choice([0, r.randrange(0, 4)]))
        img += bytes(p)
    mid = bytearray(rb(ctx, 0x3c))
    mid[0:4] = struct.pack('<I', r.choice([1, 2, 2, 0, 3]))
    mid[0x24:0x28] = struct.pack('<I', nrec)
    img += bytes(mid) + rb(ctx, 0x6c * nrec)
    size = len(img)
    if r.random() < 0.5:
        extra = r.randrange(0, 40)
        img += struct.pack('<I', extra) + rb(ctx, extra)
        size = len(img) if r.random() < 0.9 else size + 1
    buf = ctx.buf(len(img) + 8, img + bytes(8))
    if r.random() < 0.03:
        buf = 0
    return [buf, size]


SPECS = [dict(name='Sim_10059a34', orig=0x10059a34, c='Sim_10059a34', gen=g, trials=2000,
              api={'HeapAlloc': lambda n, a: ST['pool'][n % 6]})]
