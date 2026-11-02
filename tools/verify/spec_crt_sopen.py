# C runtime replacement, files (DECISIONS.md): _sopen (0x101099a0), converted to C as Crt_SOpen (w6k_handles.c),
# against the original on the handle tables of spec_w6k.py. Everything below it runs for real on both sides (the
# original's _alloc_osfhnd, _set_osfhnd, _dosmaperr, _lseek, _read, _chsize, _close against the C copies); the C
# opens through Plat_FileCreate, whose checker stand-in (plat_stub.c) calls CreateFileA as the original did, so the
# Windows calls are logged and answered alike: CreateFileA / GetFileType / GetLastError and, for a text file opened
# for reading and writing, the Ctrl-Z check (SetFilePointer, ReadFile, SetEndOfFile, CloseHandle).
exec(open('tools/verify/spec_w6k.py').read())

SOURCES = SOURCES + ['sim/unsorted_1010/w8A_lowio.c']
STUBS.pop(0x101099a0)           # here _sopen itself is compared


def ctz_read(n, args, uc):
    """ReadFile: the last byte, Ctrl-Z or not, or nothing read (the buffer and count pointers are read from the
    stack: the checker shows stack addresses as 'stack')."""
    from unicorn.x86_const import UC_X86_REG_ESP
    esp = uc.reg_read(UC_X86_REG_ESP)
    buf, done = r32(uc, esp + 8), r32(uc, esp + 16)
    v = ST['read'][n % len(ST['read'])]
    if v is None:
        uc.mem_write(done, struct.pack('<I', 0))
        return 1
    uc.mem_write(buf, bytes([v]))
    uc.mem_write(done, struct.pack('<I', 1))
    return 1


def g_sopen(ctx):
    begin(ctx)
    r = ctx.rng
    nblk = r.choice([1, 1, 2])
    handle_world(ctx, nblk)
    blk = r32(ctx.m.uc, HANDLES + 4 * r.randrange(nblk))
    k = r.randrange(32)
    wb(ctx, blk + 8 * k + 4, [0])                           # a free slot
    w32(ctx, NHANDLES, nblk * 32)
    w32(ctx, 0x1017ed18, r.choice([0x4000, 0x8000, 0x4000]))     # _fmode
    w32(ctx, 0x1017e890, r.choice([0, 0x80, 0x12]))               # umask
    path = cstr(ctx, r.choice([b'HG.INI', b'game\\OBJECT.DAT', b'c:\\x\\y.cfg', b'']))
    oflag = r.choice([0, 1, 2]) | r.choice([0, 8]) | r.choice([0, 0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700]) \
        | r.choice([0, 0x4000, 0x8000, 0]) | r.choice([0, 0, 0x10, 0x20, 0x40, 0x80, 0x1000])
    if r.random() < 0.05:
        oflag = ctx.u32()
    shflag = r.choice([0x10, 0x20, 0x30, 0x40, 0x40, 0x40, r.choice([0, 0x50, ctx.u32()])])
    pmode = r.choice([0x1a4, 0x80, 0x100, 0, ctx.u32()])
    ST['CreateFileA'] = [r.choice([0x200, 0x200, 0x200, 0xffffffff])]
    ST['GetFileType'] = [r.choice([1, 1, 1, 2, 3, 0])]
    ST['GetLastError'] = [r.choice([2, 3, 5, 0x83, 80, 0x20])]
    ST['SetFilePointer'] = [r.choice([0x40, 0, 0xffffffff]), r.choice([0, 0xffffffff])]
    ST['SetEndOfFile'] = [r.choice([1, 1, 0])]
    ST['CloseHandle'] = [1]
    ST['read'] = [r.choice([0x1a, 0x1a, ord('a'), None])]
    return [path, oflag, shflag, pmode]


API2 = dict(API)
for _n in ('GetFileType', 'GetLastError', 'SetFilePointer', 'SetEndOfFile', 'CloseHandle'):
    API2[_n] = api_seq(_n)
API2['ReadFile'] = lambda n, args, uc: ctz_read(count_calls(ST['m'], 'ReadFile'), args, uc)

SPECS = [dict(name='Crt_SOpen', orig=0x101099a0, c='Crt_SOpen', gen=g_sopen, trials=4000, api=API2)]
