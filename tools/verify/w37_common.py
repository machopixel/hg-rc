# DLL removal step 1: shared helpers of the graphics-library specs (spec_w37_*.py). exec after w10_common.py.
from unicorn.x86_const import UC_X86_REG_GDTR, UC_X86_REG_SS, UC_X86_REG_DS, UC_X86_REG_ES

GDT = 0x00F00F80             # inside the stub page the checker maps


def flat(ctx):
    """The library's routines push and pop ES (and copy DS to ES): a flat descriptor table and data selectors, as
    spec_w30_vfx.py does."""
    m = ctx.m
    if getattr(m, '_w37gdt', False):
        return
    m._w37gdt = True
    m.raw.mem_write(GDT, struct.pack('<3Q', 0, 0x00CF9A000000FFFF, 0x00CF92000000FFFF))
    m.raw.reg_write(UC_X86_REG_GDTR, (0, GDT, 0x17, 0))
    for reg in (UC_X86_REG_SS, UC_X86_REG_DS, UC_X86_REG_ES):
        m.raw.reg_write(reg, 0x10)


def put(ctx, data):
    p = newbuf(ctx, len(data))
    wb(ctx, p, data)
    return p


def start(ctx, *active):
    r = begin(ctx, *active)
    flat(ctx)
    return r


def window(ctx, r, cells=None, rows=None):
    """an 8-bit window {buffer, pitch - 1, height - 1} (the pitch is the row's byte count), mostly valid"""
    cells = cells or r.choice([640, 320, 64, 100, 33])
    rows = rows or r.choice([200, 40, 64, 13])
    buf = newbuf(ctx, cells * (rows + 2) + 64)
    win = newbuf(ctx, 12)
    w32(ctx, win, buf, r.choice([cells - 1, cells - 1, cells - 1, 0, -1 & 0xffffffff]),
        r.choice([rows - 1, rows - 1, rows - 1, 0, -1 & 0xffffffff]))
    return win, cells, rows


def pane(ctx, r, win, cells, rows):
    """a pane {window, x0, y0, x1, y1}: the whole window, or random edges inside / partly outside it"""
    p = newbuf(ctx, 20)
    x0 = r.choice([0, 0, r.randrange(-20, cells), r.randrange(0, cells)])
    y0 = r.choice([0, 0, r.randrange(-20, rows), r.randrange(0, rows)])
    x1 = r.choice([cells - 1, cells - 1, r.randrange(-5, cells + 20)])
    y1 = r.choice([rows - 1, rows - 1, r.randrange(-5, rows + 20)])
    w32(ctx, p, win, x0 & 0xffffffff, y0 & 0xffffffff, x1 & 0xffffffff, y1 & 0xffffffff)
    return p


def coord(r, n):
    return r.choice([r.randrange(0, n), r.randrange(0, n), r.randrange(-n, 2 * n), r.randrange(-3, 4),
                     r.randrange(-40, 0)]) & 0xffffffff
