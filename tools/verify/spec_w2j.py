# Differential tests for the wave 2 functions of letter j (see difftest.py).
import struct

SOURCES = [
    'sim/core/timer.c', 'sim/core/w2j_clock.c',
    'sim/util/lookup.c',
    'sim/unsorted_1000/w2j_anim.c', 'sim/unsorted_1000/w2j_misc.c', 'sim/unsorted_1000/w2j_slots.c',
    'sim/unsorted_1001/w2j_misc.c',
    'sim/unsorted_1002/w2j_nodes.c',
    'sim/unsorted_1003/w2j_misc.c',
    'sim/unsorted_1004/w2j_misc.c', 'sim/unsorted_1004/w2j_strtab.c',
    'sim/unsorted_1005/w2j_misc.c',
    'sim/unsorted_1007/w2j_misc.c',
    'sim/unsorted_1008/w2j_misc.c',
    'sim/unsorted_1009/w2j_misc.c', 'sim/unsorted_1009/w2j_tables.c',
    'sim/unsorted_100a/w2j_lists.c',
    'sim/unsorted_100b/w2j_misc.c',
    'sim/unsorted_100d/w2j_nodes.c',
    'sim/unsorted_100e/w2j_misc.c',
    'sim/unsorted_100f/w2j_str.c', 'sim/unsorted_100f/w2j_fixdiv.c',
    'sim/unsorted_1010/w2j_misc.c',
]


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def put(ctx, addr, fmt, *vals):
    """Writes little-endian values; ints are masked to the size of their format character."""
    out = b''
    for f, v in zip(fmt, vals):
        size = struct.calcsize('<' + f)
        out += (v & ((1 << (8 * size)) - 1)).to_bytes(size, 'little')
    ctx.m.uc.mem_write(addr, out)


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little')


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def s32(v):
    v &= 0xffffffff
    return v - (1 << 32) if v >= 1 << 31 else v


# ---- timer module state (for the users of the timer) -----------------------------------------
def timer_state(ctx, number):
    """Counters and slot tables; returns the elapsed value of timer `number`."""
    rng = ctx.rng
    for base in (0x10153180, 0x10153280):
        slots = [rng.choice([0, 0, rng.getrandbits(32), 1, 0xfffffff0]) for _ in range(63)]
        ctx.m.uc.mem_write(base, struct.pack('<64I', *(slots + [0xFFFFFFFF])))
    ca = rng.choice([0, 1, 7, 1000, rng.getrandbits(32)])
    cb = rng.choice([0, 1, 7, 1000, rng.getrandbits(32)])
    w32(ctx, 0x10153380, ca)
    w32(ctx, 0x10153384, cb)
    w32(ctx, 0x1015317c, rng.getrandbits(10))
    if number & 0x80:
        return (ca - r32(ctx, 0x10153180 + 4 * (number ^ 0x80))) & 0xffffffff
    return (cb - r32(ctx, 0x10153280 + 4 * number)) & 0xffffffff


# ---- 0x1002a983 ----------------------------------------------------------------------------
def g_clock(ctx):
    rg = ctx.rng
    number = rg.choice([0x80, 0x81, 0x85, 0x90, 0, 3, 0x7f])
    elapsed = timer_state(ctx, number)
    w32(ctx, 0x101354d0, number)
    mode = rg.choice([0, 0, 0, 3, 3, 1, 2, 5])
    limit = rg.choice([0, 0, -5, 7, 100, rg.randrange(-3, 40)])
    if mode == 0 and s32(limit) > 0:
        prev = (elapsed - limit - rg.choice([0, 1, 5, 30, 0x1000])) & 0xffffffff   # the wait loop would never end otherwise
    else:
        prev = rg.choice([rg.getrandbits(32), (elapsed - rg.randrange(-20, 60)) & 0xffffffff, 0,
                          rg.randrange(0, 200)])
    w32(ctx, 0x1015dca0, rg.choice([1, 1, 1, 1, 5, 0]))
    w32(ctx, 0x1015dc84, mode)
    w32(ctx, 0x1015dc88, rg.choice([0, 0, 1, 7]))
    w32(ctx, 0x1015dc94, limit)
    w32(ctx, 0x1015dc98, prev)
    w32(ctx, 0x1015dc9c, rg.choice([0, 1, 2, 3, 5]))
    w32(ctx, 0x1015dc78, rg.choice([rg.getrandbits(32), rg.randrange(0, 500), (prev + rg.randrange(-10, 100)) & 0xffffffff]))
    w32(ctx, 0x1015dc80, rg.getrandbits(32))
    w32(ctx, 0x1015dc8c, rg.choice([0, 0, 1, 2]))
    w32(ctx, 0x1015dc90, rg.choice([0, 0, 1, 2]))
    w32(ctx, 0x1015ec58, rg.choice([0, 0, 0, 1]))
    return []


# ---- animation records (0x10006a2f) ----------------------------------------------------------
def g_anim(ctx):
    rg = ctx.rng
    now = rg.choice([rg.randrange(0, 5000), rg.getrandbits(32), 0, 0xffffff00 + rg.randrange(0, 256)])
    w32(ctx, 0x1015dc78, now)
    frames = bytearray()
    for _ in range(4 * 0x300 // 0x18):
        row = bytearray(rb(ctx, 0x18))
        if rg.random() < 0.75:
            row[0] = rg.choice([1, 2, 0x80, 0xff, 5])
        elif rg.random() < 0.8:
            row[0] = 0
        frames += row
    ctx.m.uc.mem_write(0x10a51ac0, bytes(frames))
    table = bytearray()
    active = rg.choice([0, 1, 3, 8, 20])
    live = set(rg.sample(range(0x200), active))
    for i in range(0x200):
        if i in live:
            period = rg.choice([1, 2, 3, 5, 10, 20, 0xffff])
            last = rg.choice([0, now, (now - rg.randrange(0, 40 * min(period, 20))) & 0xffffffff,
                              (now - rg.randrange(0, 3 * period + 1)) & 0xffffffff,
                              (now + rg.randrange(1, 50)) & 0xffffffff])
            if period == 0xffff:
                last = (now - rg.randrange(0, 150000)) & 0xffffffff
            ent = struct.pack('<hhHHhI', rg.choice([0, 1, 2, 3, 3, -1]),
                              rg.choice([rg.randrange(0, 32), rg.randrange(0, 32), -1, 31, 32, 0x7fff, -0x8000, rg.randrange(-100, 100)]),
                              period, rg.choice([1, 1, 2, 2, 3, 0xffff]),
                              rg.choice([0, 0, 1, 5, -1, -0x8000]), last)
        else:
            ent = (rb(ctx, 9) + bytes([0x80 | rg.getrandbits(7)]) + rb(ctx, 4)) if rg.random() < 0.3 else struct.pack('<hhHHhI', 0, 0, rg.choice([0, 5]), 0 if rg.random() < 0.7 else 1,
                                                                  rg.choice([0, -1]), rg.getrandbits(32))
            if rg.random() < 0.3:
                ent = ent[:4] + b'\0\0' + ent[6:]
        table += ent
    ctx.m.uc.mem_write(0x10ac1b00, bytes(table))
    return []


# ---- simple globals ---------------------------------------------------------------------------
def g_swap(ctx):
    w32(ctx, 0x1015d09c, ctx.u32())
    return [ctx.u32()]


def g_none(ctx):
    return []


def g_u32(ctx):
    return [ctx.u32()]


def g_two(ctx):
    return [ctx.u32(), ctx.u32()]


def g_five(ctx):
    return [ctx.u32() for _ in range(5)]


def g_global(addr):
    def gen(ctx):
        w32(ctx, addr, ctx.u32())
        return [ctx.u32()]
    return gen


def g_global0(addr):
    def gen(ctx):
        w32(ctx, addr, ctx.u32())
        return []
    return gen


def g_shift(ctx):
    return [ctx.rng.choice([ctx.u32(), ctx.rng.getrandbits(32), 0x80000000, 0xffff0000, 0x7fffffff, 1 << 16 | 5])]


# ---- record with a block of 0x28-byte entries (0x100091f7) -------------------------------------
def g_91f7(ctx):
    rg = ctx.rng
    obj = newbuf(ctx, 0x40)
    block = newbuf(ctx, 0x600)
    count = rg.choice([0, 1, 2, 5, 8, rg.randrange(0, 9), -1])
    put(ctx, block + 6, 'h', count)
    put(ctx, block + 8, 'I', rg.choice([0x10, 0x20, 0x34, 0x100, 0xc0]))
    put(ctx, obj + 0x30, 'I', 0 if rg.random() < 0.1 else block)
    return [obj, rg.choice([rg.getrandbits(16), 0, 0xffff, rg.getrandbits(32)]),
            rg.choice([rg.getrandbits(32), block, block + rg.randrange(0, 0x400), 0])]


def g_obj40(ctx):
    return [newbuf(ctx, 0x40)]


# ---- group slots (0x10017892 and users) ----------------------------------------------------------
def slot_world(ctx):
    """Pointer table 0x109e8b40 with records, the current group and its slot count."""
    rg = ctx.rng
    count = rg.randrange(1, 9)
    group = rg.randrange(0, 3)
    cur = rg.randrange(0, count)
    pc = rg.choice([0.2, 0.7, 1.0, 1.0])
    recs = []
    for i in range(count):
        rec = newbuf(ctx, 0x200)
        g = group if (i == cur or rg.random() < 0.8) else rg.randrange(0, 3)
        put(ctx, rec + 8, 'I', g)
        put(ctx, rec + 0x10, 'I', i + 1 if rg.random() < 0.8 else rg.randrange(0, 12))
        put(ctx, rec + 0x1a6, 'h', 0xc if rg.random() < pc else rg.choice([0, 1, 5, -1]))
        recs.append(rec)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
    put(ctx, recs[cur] + 8, 'I', group)
    w32(ctx, 0x1015dc64, count if rg.random() < 0.9 else rg.randrange(0, count + 1))
    w32(ctx, 0x1017b8c8, cur)
    for g in range(3):
        w32(ctx, 0x109b7d44 + g * 0x6c, rg.choice([0, 1, 2, 3, 4, 5, 6, 7, 3, 4]))
    return recs


def g_17892(ctx):
    slot_world(ctx)
    return []


def g_slots_obj(ctx):
    rg = ctx.rng
    slot_world(ctx)
    obj = newbuf(ctx, 0x400)
    put(ctx, obj + 8, 'i', rg.choice([-1, 0, 1, 2, 3, 4, 5, 6, 7, 9]))
    put(ctx, obj + 0xc, 'I', rg.choice([1, 2, 3, 4, 5, 6, 8, 10]))
    return [rg.getrandbits(32), 0 if rg.random() < 0.07 else obj]


def g_slot_test(ctx):
    slot_world(ctx)
    return [ctx.rng.choice([-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 100, ctx.u32()])]


def g_mode(ctx):
    slot_world(ctx)
    return [ctx.rng.choice([0, 0, 0, 1, 1, 2, 2, 3, -1, ctx.u32()])]


# ---- 0x100107ce -------------------------------------------------------------------------------------
def g_107ce(ctx):
    w32(ctx, 0x1013bb40, ctx.rng.choice([0, 1, 2, 3, 0xfffffffe]))
    return [ctx.rng.choice([0, 0, 1, 2, 5, 10, 0xffffffff, 0xfffffff0])]


# ---- hashed nodes (0x10027c70..0x1002828e, 0x1000f029) ----------------------------------------------
def bucket(key, name4):
    total = key + sum(b - 256 if b >= 128 else b for b in name4)
    total = s32(total)
    r = abs(total) % 0x3f1
    return -r if total < 0 else r


def hash_world(ctx, n=None):
    """Hash table of nodes with a tail list state; returns [(node, key, name bytes)]."""
    rg = ctx.rng
    n = rg.choice([0, 1, 3, 6, 10, 14]) if n is None else n
    base = zbuf(ctx, 0x800 + 0x3f1 * 4 + 0x400)
    tab = base + 0x800
    nodes = []
    for _ in range(n):
        key = rg.choice([rg.randrange(0, 16), rg.randrange(0, 0x10000), rg.randrange(0, 0x100)])
        name = rg.choice([rb(ctx, 4), rb(ctx, 4), bytes([1, 2, 3, 4]), bytes([0x80, 0xff, 0x7f, 1]), b'\xff\xff\xff\xff'])
        node = zbuf(ctx, 0x20)
        nodes.append((node, key, name))
    for node, key, name in nodes:
        b = bucket(key, name)
        head = r32(ctx, tab + 4 * b)
        put(ctx, node, 'HhII', key, rg.choice([0, 0, 1, 0xffff, 2]), struct.unpack('<I', name)[0], head)
        put(ctx, node + 0xc, 'II', rg.getrandbits(32) if rg.random() < 0.2 else 0, 0)
        w32(ctx, tab + 4 * b, node)
    w32(ctx, 0x1015dc30, tab)
    if nodes and rg.random() < 0.6:
        w32(ctx, 0x109f13f0, rg.choice(nodes)[0])
        w32(ctx, 0x109f13f4, rg.choice(nodes)[0])
    else:
        w32(ctx, 0x109f13f0, 0)
        w32(ctx, 0x109f13f4, 0)
    return nodes


def g_27c70(ctx):
    nodes = hash_world(ctx, ctx.rng.choice([1, 3, 6]))
    return [ctx.rng.choice(nodes)[0]]


def g_27d77(ctx):
    hash_world(ctx)
    return []


def lookup_args(ctx, nodes):
    rg = ctx.rng
    if nodes and rg.random() < 0.7:
        node, key, name = rg.choice(nodes)
        if rg.random() < 0.15:
            key = key + 0x10000 if rg.random() < 0.5 else key ^ 1
    else:
        key = rg.choice([rg.randrange(0, 16), rg.randrange(0, 0x10000), -1, -5, 0x7fffffff, 0x80000000 | rg.getrandbits(8)])
        name = rb(ctx, 4)
    return key & 0xffffffff, ctx.buf(4, name)


def g_27f33(ctx):
    nodes = hash_world(ctx)
    return list(lookup_args(ctx, nodes))


def g_2828e(ctx):
    nodes = hash_world(ctx)
    return list(lookup_args(ctx, nodes))


def g_f029(ctx):
    rg = ctx.rng
    nodes = hash_world(ctx)
    key, name = lookup_args(ctx, nodes)
    w32(ctx, 0x1014ceb4, name)
    base = newbuf(ctx, 0x400)
    w32(ctx, 0x1015d0a4, base)
    obj = newbuf(ctx, 0x40)
    put(ctx, obj + 4, 'I', rg.choice([0, 1, 5, 0x30, rg.randrange(0, 0x40)]))
    put(ctx, obj + 8, 'I', 0 if rg.random() < 0.12 else rg.choice([1, 0x1234, rg.getrandbits(32)]))
    put(ctx, obj + 0x1c, 'H', key & 0xffff)
    return [0 if rg.random() < 0.06 else obj]


def g_28a26(ctx):
    return [newbuf(ctx, 0x200), ctx.u32()]


# ---- 0x1003b449 ---------------------------------------------------------------------------------------
def g_3b449(ctx):
    return [ctx.rng.choice([ctx.rng.randrange(-3, 200), 0, 1, 2]), ctx.u32()]


# ---- 0x100405e6 ---------------------------------------------------------------------------------------
def g_405e6(ctx):
    rg = ctx.rng
    obj = zbuf(ctx, 8)
    idx = rg.randrange(0, 16)
    ident = rg.randrange(0, 256)
    put(ctx, obj + 2, 'B', idx)
    put(ctx, obj + 4, 'B', ident)
    w32(ctx, 0x1017b8c8, ident if rg.random() < 0.25 else rg.getrandbits(8))
    for i in range(16):
        rec = 0 if rg.random() < 0.1 else newbuf(ctx, 0x40)
        if rec:
            put(ctx, rec + 0x14, 'I', rg.choice([3, 3, 3, 0, 1, rg.getrandbits(32)]))
        w32(ctx, 0x109e8b40 + 4 * i, rec)
    return [obj]


def g_449fc(ctx):
    rg = ctx.rng
    src = newbuf(ctx, 0x40)
    put(ctx, src + 0x18, 'I', rg.choice([0, 1, 7, rg.getrandbits(32)]))
    dst = newbuf(ctx, 0x40)
    return [0 if rg.random() < 0.08 else dst, 0 if rg.random() < 0.08 else src]


def g_47af2(ctx):
    rg = ctx.rng
    now = rg.choice([rg.randrange(0, 5000), rg.getrandbits(32)])
    w32(ctx, 0x1015dc78, now)
    p = newbuf(ctx, 0x20)
    target = rg.choice([rg.randrange(-500, 500), rg.getrandbits(32), 0])
    value = rg.choice([rg.randrange(-500, 500), rg.getrandbits(32), target, target + 1, target - 1])
    last = rg.choice([(now - rg.randrange(-5, 300)), rg.getrandbits(32)])
    duration = rg.choice([rg.randrange(1, 100), rg.randrange(1, 100), -rg.randrange(1, 100), 1, -1, 0, rg.getrandbits(32), 0x7fffffff])
    put(ctx, p, 'IIII', last, target, value, duration)
    return [p]


# ---- string table (0x1004e4c2, 0x1004e61a) ----------------------------------------------------------
def strtab_world(ctx, filled=True):
    rg = ctx.rng
    count = rg.choice([0, 1, 5, 100, 0xffe, 0xfff, 0x1000, 0x1001, rg.randrange(0, 0x1000)])
    w32(ctx, 0x1015f350, count)
    slots = rg.choice([0.0, 0.05, 0.3, 0.7])
    words = [rg.getrandbits(32) if rg.random() < slots else 0xffff for _ in range(0x26f5)]
    if 0xffff not in words:
        words[rg.randrange(0, 0x26f5)] = 0xffff
    tab = ctx.buf(0x26f5 * 4, struct.pack('<%dI' % 0x26f5, *words))
    data = ctx.buf(0x1100, rb(ctx, 0x1100))
    prefixes = ctx.buf(0x1100 * 4, rb(ctx, 0x1100 * 4))
    w32(ctx, 0x1015f35c, tab)
    w32(ctx, 0x1015f354, data)
    w32(ctx, 0x1015f358, prefixes)
    return tab


def g_4e4c2(ctx):
    rg = ctx.rng
    tab = strtab_world(ctx)
    prefix = rg.choice([0xffff, 0xffff, rg.randrange(0, 0x1000), rg.getrandbits(32), 0, 0x26f5, 0x7f7])
    byte = rg.choice([rg.randrange(0, 256), rg.getrandbits(32), 0, 0xff])
    if rg.random() < 0.5:
        # make the first probe hit a used slot
        slot = (((byte & 0xff) << 8) ^ prefix) % 0x26f5
        ctx.m.uc.mem_write(tab + 4 * slot, struct.pack('<I', rg.randrange(0, 0xffff)))
    return [prefix, byte]


def g_4e61a(ctx):
    rg = ctx.rng
    strtab_world(ctx)
    bits = rg.choice([0, 1, 2, 3, 5, 7, 8, 8, 9, 10, 11, 12, 12, 13])
    if rg.random() < 0.15:
        bits |= rg.choice([0x20, 0x100, 0xff00])     # only the low five bits count
    return [bits]


def g_4e1a1(ctx):
    w32(ctx, 0x1015f340, ctx.rng.choice([0, 1, 5, ctx.rng.randrange(0, 16)]))
    ctx.m.uc.mem_write(0x1015f230, rb(ctx, 32))
    w32(ctx, 0x1015f348, ctx.u32())
    return []


# ---- 0x10058389, 0x1005892e, 0x1005fa30 --------------------------------------------------------------
def fill_records(ctx):
    ctx.m.uc.mem_write(0x109b16e0, rb(ctx, 0x100 * 0x4c + 16))
    ctx.m.uc.mem_write(0x109b12c0, rb(ctx, 0x400))
    ctx.m.uc.mem_write(0x109b0eac, rb(ctx, 8))
    ctx.m.uc.mem_write(0x101668d0, rb(ctx, 8))


def g_58389(ctx):
    fill_records(ctx)
    return []


def g_5892e(ctx):
    rg = ctx.rng
    fill_records(ctx)
    count = rg.choice([0x100, 0x100, 5, 0, 1, -1, rg.randrange(0, 0x100)])
    w32(ctx, 0x109b0eac, count)
    index = rg.choice([rg.randrange(-3, 6), rg.randrange(0, 0x100), count, count - 1, count + 1, -1, 0, 0x7fffffff, 0x80000000])
    return [index & 0xffffffff, rg.choice([0, 1, 2, 0xffffffff, ctx.u32()])]


def g_5fa30(ctx):
    ctx.m.uc.mem_write(0x101c4000, rb(ctx, 0x1300))
    ctx.m.uc.mem_write(0x10144300, rb(ctx, 0x700))
    return []


# ---- 0x10072e35 -----------------------------------------------------------------------------------------
def g_72e35(ctx):
    rg = ctx.rng
    w32(ctx, 0x1015ec58, rg.choice([0, 1, 7]))
    w32(ctx, 0x1014c99c, rg.choice([0xffffffff, 0, 5, ctx.u32()]))
    return [newbuf(ctx, 0x2400)]


def g_765f6(ctx):
    rg = ctx.rng
    obj = newbuf(ctx, 0x20)
    arr = newbuf(ctx, 0x40)
    put(ctx, obj + 8, 'I', arr)
    put(ctx, obj + 0xc, 'i', rg.choice([-2, -1, 0, 1, 2, 5, 10]))
    return [obj]


# ---- 0x1008cfce, 0x1008f64c --------------------------------------------------------------------------------
def g_8cfce(ctx):
    rg = ctx.rng
    base = newbuf(ctx, 0x80)
    vals = [rg.choice([rg.randrange(-6000000, 6000000), rg.randrange(-3000, 3000), 0, 1, -1, 319, 199, 65536, -65536,
                       rg.randrange(-100000, 100000), 6500000, -6500000]) for _ in range(4)]
    src = base + 0x20
    put(ctx, src, 'iiii', *vals)
    r = rg.random()
    dst = src if r < 0.15 else (src - 4 if r < 0.25 else base + 0x40)
    return [src, dst]


def g_8f64c(ctx):
    rg = ctx.rng
    p = newbuf(ctx, 0x20)
    target = newbuf(ctx, 0x20)
    put(ctx, p, 'I', target if rg.random() < 0.97 else p)
    return [p]


# ---- 0x1009a6b3 ---------------------------------------------------------------------------------------------
def g_9a6b3(ctx):
    rg = ctx.rng
    obj = newbuf(ctx, 0x40)
    put(ctx, obj + 0x24, 'H', rg.getrandbits(16) | (0x20 if rg.random() < 0.7 else 0) if rg.random() < 0.7 else rg.getrandbits(16) & ~0x20)
    put(ctx, obj + 0x26, 'H', rg.choice([0, 0, 1, 0x8000, rg.getrandbits(16)]))
    return [obj, rg.choice([rg.randrange(0, 32), rg.randrange(0, 40), ctx.u32()])]


def g_9b362(ctx):
    rg = ctx.rng
    vals = [0, 1, 2, 3, 4, 5, 0xffffffff, 0x80000000, 0x7fffffff, rg.getrandbits(32)]
    w32(ctx, 0x1015d1ac, rg.choice([2, 4, 4, 2, 1, 3] + vals))
    w32(ctx, 0x1015d1cc, rg.choice([2, 4, 4, 2, 1, 3] + vals))
    return []


def g_98a49(ctx):
    w32(ctx, 0x1015d1b8, newbuf(ctx, 0x40))
    return [ctx.u32()]


def g_905e8(ctx):
    ctx.m.uc.mem_write(0x10215ae0, rb(ctx, 0x2180))
    ctx.m.uc.mem_write(0x101d09c0, rb(ctx, 0x20))
    ctx.m.uc.mem_write(0x1036f040, rb(ctx, 0x20))
    return []


# ---- lists (0x100aac3d, 0x100abab0, 0x100abc5d) ------------------------------------------------------------------
def g_aac3d(ctx):
    rg = ctx.rng
    k = rg.choice([0, 1, 2, 3, 6])
    nodes = [newbuf(ctx, 0x10) for _ in range(k)]
    for i, nd in enumerate(nodes):
        put(ctx, nd + 8, 'I', nodes[i + 1] if i + 1 < k else 0)
    obj = newbuf(ctx, 0x10)
    put(ctx, obj + 4, 'I', nodes[0] if nodes else 0)
    return [0 if rg.random() < 0.05 else obj, rg.choice([-2, -1, 0, 1, 2, k - 1, k, k + 1, rg.randrange(-3, 9)])]


def list_state(ctx):
    rg = ctx.rng
    head = zbuf(ctx, 8)
    tail = head + 4 if rg.random() < 0.9 else head
    if rg.random() < 0.5:
        a = newbuf(ctx, 0x20)
        put(ctx, head, 'I', a if rg.random() < 0.8 else 0)
        put(ctx, tail, 'I', a)
    return head, tail


def g_abab0(ctx):
    rg = ctx.rng
    head, tail = list_state(ctx)
    return [0 if rg.random() < 0.06 else newbuf(ctx, 0x20), head, tail]


def g_abc5d(ctx):
    rg = ctx.rng
    w32(ctx, 0x1017d1c4, rg.choice([-1, 0, 1, 5, rg.randrange(-2, 10), 0x80000000, 0xffffffff]))
    ctx.m.uc.mem_write(0x101c56c0, struct.pack('<16I', *[rg.choice([rg.randrange(0, 40), rg.getrandbits(6)]) for _ in range(16)]))
    ctx.m.uc.mem_write(0x101c57e0, rb(ctx, 0x34 * 0x30))
    if rg.random() < 0.5:
        a = newbuf(ctx, 0x20)
        w32(ctx, 0x1017d1d0, a)
        w32(ctx, 0x1017d1d4, a)
    else:
        w32(ctx, 0x1017d1d0, 0)
        w32(ctx, 0x1017d1d4, 0)
    return []


# ---- 0x100b487e, 0x100b53bb, 0x100bda0b ------------------------------------------------------------------------------
def entries88(ctx):
    rg = ctx.rng
    count = rg.choice([0, 1, 3, 8, 12, rg.randrange(0, 13), -1])
    w32(ctx, 0x1017d218, count)
    zero_p = rg.choice([0, 0.1, 0.4])
    data = bytearray(rb(ctx, 0x88 * 16 + 0x30))
    for i in range(16):
        data[i * 0x88] = 0 if rg.random() < zero_p else rg.choice([1, 0x7f, 0x80, 0xff, 5])
    ctx.m.uc.mem_write(0x101a2100, bytes(data))
    return count


def g_b487e(ctx):
    entries88(ctx)
    return []


def g_b53bb(ctx):
    rg = ctx.rng
    count = entries88(ctx)
    return [rg.choice([rg.randrange(-2, 14), count, count - 1, count + 1, 0, 0x7fffffff, 0x80000000]) & 0xffffffff]


def g_bda0b(ctx):
    rg = ctx.rng
    n = rg.choice([-1, 0, 1, 3, 8, rg.randrange(0, 12)])
    arr = ctx.buf(0x40, struct.pack('<16I', *[rg.choice([rg.randrange(0, 6), rg.getrandbits(32)]) for _ in range(16)]))
    obj = newbuf(ctx, 0x10)
    put(ctx, obj, 'i', n)
    put(ctx, obj + 8, 'I', arr)
    vals = [r32(ctx, arr + 4 * i) for i in range(16)]
    return [obj, rg.choice([rg.choice(vals), rg.choice(vals), rg.randrange(0, 8), rg.getrandbits(32)])]


# ---- 0x100d1780 -----------------------------------------------------------------------------------------------------
def g_d1780(ctx):
    rg = ctx.rng
    m = rg.choice([1, 2, 3, 6])
    chain = [zbuf(ctx, 0x20) for _ in range(m)]
    owner = 0 if rg.random() < 0.3 else zbuf(ctx, 0x20)
    other = zbuf(ctx, 0x20)
    for i, nd in enumerate(chain):
        put(ctx, nd + 4, 'II', chain[i - 1] if i else 0, chain[i + 1] if i + 1 < m else 0)
        put(ctx, nd + 0x14, 'I', rg.getrandbits(32))
    a = zbuf(ctx, 0x20)
    nxt = other if rg.random() < 0.5 else 0
    put(ctx, a + 4, 'II', 0, nxt)
    put(ctx, a + 0x14, 'I', owner)
    if nxt:
        put(ctx, other + 4, 'I', a)
    return [a, rg.choice(chain)]


# ---- fixed point division (0x100f9f50, 0x100f9f88) -----------------------------------------------------------------------
def g_f9f50(ctx):
    rg = ctx.rng
    mb = rg.choice([1, 2, 3, 7, 100, 0x10000, 0x7fffffff, 0x80000000, rg.randrange(1, 1 << 31), rg.randrange(1, 1000),
                    rg.randrange(1, 1 << rg.randrange(1, 32))])
    limit = min((1 << 31) - 1, 4 * mb - 1)
    ma = rg.choice([rg.randrange(0, limit + 1), rg.randrange(0, limit + 1), 0, limit, limit - 1 if limit else 0,
                    rg.randrange(0, min(limit, 1 << 20) + 1)])
    ma = max(0, min(ma, limit))
    a = -ma if rg.random() < 0.5 else ma
    b = -mb if rg.random() < 0.5 else mb
    return [a & 0xffffffff, b & 0xffffffff]


def g_f9f88(ctx):
    rg = ctx.rng
    mb = rg.choice([0x4001, 0x4002, 0x8000, 0x10000, 0x7fffffff, 0x80000000, rg.randrange(0x4001, 1 << 31),
                    rg.randrange(0x4001, 0x100000), 3 << 14])
    return [(-mb if rg.random() < 0.5 else mb) & 0xffffffff]


# ---- string routines ---------------------------------------------------------------------------------------------------------
ALPHA = b'abcAB 019_\x80\xff\x01'


def text(ctx, n, alphabet=ALPHA):
    return bytes(ctx.rng.choice(alphabet) for _ in range(n))


def g_cat(ctx):
    rg = ctx.rng
    dbuf = ctx.buf(0x200, rb(ctx, 0x200))
    sbuf = ctx.buf(0x100, rb(ctx, 0x100))
    do, so = rg.randrange(0, 4), rg.randrange(0, 4)
    dl, sl = rg.choice([0, 1, 2, 3, 4, 5, 7, 8, rg.randrange(0, 40)]), rg.choice([0, 1, 2, 3, 4, 5, 7, 8, rg.randrange(0, 70)])
    ctx.m.uc.mem_write(dbuf + do, text(ctx, dl) + b'\0')
    ctx.m.uc.mem_write(sbuf + so, text(ctx, sl) + b'\0')
    return [dbuf + do, sbuf + so]


def g_ncat(ctx):
    rg = ctx.rng
    args = g_cat(ctx)
    sl = struct.unpack('<B', bytes(ctx.m.uc.mem_read(args[1], 1)))[0]
    slen = 0
    while bytes(ctx.m.uc.mem_read(args[1] + slen, 1)) != b'\0':
        slen += 1
    return args + [rg.choice([0, 1, 2, 3, 4, 5, 8, slen, slen + 1, max(slen - 1, 0), rg.randrange(0, 80), 100])]


def g_span(ctx):
    rg = ctx.rng
    sbuf = ctx.buf(0x100, rb(ctx, 0x100))
    tbuf = ctx.buf(0x100, rb(ctx, 0x100))
    so, to = rg.randrange(0, 4), rg.randrange(0, 4)
    small = rg.choice([b'ab', b'abc\x80', ALPHA, b'a', bytes(range(1, 256))])
    st = text(ctx, rg.choice([0, 1, 5, 12, rg.randrange(0, 60)]), small + b'z')
    tt = text(ctx, rg.choice([0, 0, 1, 3, 8, rg.randrange(0, 40)]), small)
    ctx.m.uc.mem_write(sbuf + so, st + b'\0')
    ctx.m.uc.mem_write(tbuf + to, tt + b'\0')
    return [sbuf + so, tbuf + to]


def g_last(ctx):
    rg = ctx.rng
    sbuf = ctx.buf(0x100, rb(ctx, 0x100))
    so = rg.randrange(0, 4)
    st = text(ctx, rg.choice([0, 1, 2, 5, 12, rg.randrange(0, 60)]))
    ctx.m.uc.mem_write(sbuf + so, st + b'\0')
    c = rg.choice([rg.choice(ALPHA), rg.choice(ALPHA), 0, rg.choice(ALPHA) | 0x100, 0xffffff00 | rg.choice(ALPHA), rg.getrandbits(32), ord('z')])
    return [sbuf + so, c]


def g_flags(ctx):
    rg = ctx.rng
    return [rg.choice([rg.getrandbits(32), rg.getrandbits(16), rg.randrange(0, 0x2000), 0, 0x1000, 0xc00, 0x300])]


SPECS = [
    dict(name='Sim_10006a2f', orig=0x10006a2f, c='Sim_10006a2f', gen=g_anim, void=True, trials=600),
    dict(name='Sim_100089aa', orig=0x100089aa, c='Sim_100089aa', gen=g_swap, trials=300),
    dict(name='Sim_100091f7', orig=0x100091f7, c='Sim_100091f7', gen=g_91f7, trials=500),
    dict(name='Sim_1000943e', orig=0x1000943e, c='Sim_1000943e', gen=g_obj40, trials=300),
    dict(name='Sim_1000f029', orig=0x1000f029, c='Sim_1000f029', gen=g_f029, void=True, trials=600),
    dict(name='Sim_1000fd00', orig=0x1000fd00, c='Sim_1000fd00', gen=g_slots_obj, trials=800),
    dict(name='Sim_1000fe60', orig=0x1000fe60, c='Sim_1000fe60', gen=g_slot_test, trials=500),
    dict(name='Sim_1000ff5b', orig=0x1000ff5b, c='Sim_1000ff5b', gen=g_slots_obj, trials=800),
    dict(name='Sim_100107ce', orig=0x100107ce, c='Sim_100107ce', gen=g_107ce, void=True, trials=300),
    dict(name='Sim_100109e7', orig=0x100109e7, c='Sim_100109e7', gen=g_mode, trials=500),
    dict(name='Sim_100125d4', orig=0x100125d4, c='Sim_100125d4', gen=g_none, void=True, trials=300),
    dict(name='Sim_10017892', orig=0x10017892, c='Sim_10017892', gen=g_17892, trials=300),
    dict(name='Sim_10027c70', orig=0x10027c70, c='Sim_10027c70', gen=g_27c70, void=True, trials=500),
    dict(name='Sim_10027d77', orig=0x10027d77, c='Sim_10027d77', gen=g_27d77, void=True, trials=400),
    dict(name='Sim_10027f33', orig=0x10027f33, c='Sim_10027f33', gen=g_27f33, trials=600),
    dict(name='Sim_1002828e', orig=0x1002828e, c='Sim_1002828e', gen=g_2828e, void=True, trials=600),
    dict(name='Sim_10028a26', orig=0x10028a26, c='Sim_10028a26', gen=g_28a26, void=True, trials=300),
    dict(name='Sim_1002a983', orig=0x1002a983, c='Sim_1002a983', gen=g_clock, void=True, trials=1000),
    dict(name='Sim_1002c10e', orig=0x1002c10e, c='Sim_1002c10e', gen=g_none, void=True, trials=300),
    dict(name='Sim_10034877', orig=0x10034877, c='Sim_10034877', gen=g_u32, void=True, trials=300),
    dict(name='Sim_10034884', orig=0x10034884, c='Sim_10034884', gen=g_u32, void=True, trials=300),
    dict(name='Sim_10035589', orig=0x10035589, c='Sim_10035589', gen=g_global(0x109e3d80), trials=300),
    dict(name='Sim_1003b449', orig=0x1003b449, c='Sim_1003b449', gen=g_3b449, void=True, trials=300),
    dict(name='Sim_100405e6', orig=0x100405e6, c='Sim_100405e6', gen=g_405e6, void=True, trials=600),
    dict(name='Sim_100449fc', orig=0x100449fc, c='Sim_100449fc', gen=g_449fc, trials=400),
    dict(name='Sim_10047af2', orig=0x10047af2, c='Sim_10047af2', gen=g_47af2, trials=1000),
    dict(name='Sim_1004bc5b', orig=0x1004bc5b, c='Sim_1004bc5b', gen=g_none, trials=300),
    dict(name='Sim_1004d27a', orig=0x1004d27a, c='Sim_1004d27a', gen=g_shift, trials=300),
    dict(name='Sim_1004e1a1', orig=0x1004e1a1, c='Sim_1004e1a1', gen=g_4e1a1, void=True, trials=300),
    dict(name='Sim_1004e4c2', orig=0x1004e4c2, c='Sim_1004e4c2', gen=g_4e4c2, trials=800),
    dict(name='Sim_1004e61a', orig=0x1004e61a, c='Sim_1004e61a', gen=g_4e61a, void=True, trials=300),
    dict(name='Sim_1005570e', orig=0x1005570e, c='Sim_1005570e', gen=g_global0(0x1013f140), void=True, trials=300),
    dict(name='Sim_10056df3', orig=0x10056df3, c='Sim_10056df3', gen=g_none, trials=300),
    dict(name='Sim_10058389', orig=0x10058389, c='Sim_10058389', gen=g_58389, void=True, trials=300),
    dict(name='Sim_1005892e', orig=0x1005892e, c='Sim_1005892e', gen=g_5892e, void=True, trials=600),
    dict(name='Sim_1005fa30', orig=0x1005fa30, c='Sim_1005fa30', gen=g_5fa30, void=True, trials=300),
    dict(name='Sim_10072e35', orig=0x10072e35, c='Sim_10072e35', gen=g_72e35, void=True, trials=400),
    dict(name='Sim_100765f6', orig=0x100765f6, c='Sim_100765f6', gen=g_765f6, trials=400),
    dict(name='Sim_10076667', orig=0x10076667, c='Sim_10076667', gen=g_obj40, void=True, trials=300),
    dict(name='Sim_100877a0', orig=0x100877a0, c='Sim_100877a0', gen=g_none, void=True, trials=300),
    dict(name='Sim_100877b5', orig=0x100877b5, c='Sim_100877b5', gen=g_none, trials=300),
    dict(name='Sim_1008cfce', orig=0x1008cfce, c='Sim_1008cfce', gen=g_8cfce, trials=800),
    dict(name='Sim_1008f64c', orig=0x1008f64c, c='Sim_1008f64c', gen=g_8f64c, void=True, trials=300),
    dict(name='Sim_100905e8', orig=0x100905e8, c='Sim_100905e8', gen=g_905e8, void=True, trials=300),
    dict(name='Sim_10098a49', orig=0x10098a49, c='Sim_10098a49', gen=g_98a49, trials=300),
    dict(name='Sim_1009a6b3', orig=0x1009a6b3, c='Sim_1009a6b3', gen=g_9a6b3, trials=500),
    dict(name='Sim_1009b2fc', orig=0x1009b2fc, c='Sim_1009b2fc', gen=g_9b362, void=True, trials=400),
    dict(name='Sim_1009b362', orig=0x1009b362, c='Sim_1009b362', gen=g_9b362, void=True, trials=500),
    dict(name='Sim_1009e5a2', orig=0x1009e5a2, c='Sim_1009e5a2', gen=g_global0(0x1017d064), trials=300),
    dict(name='Sim_100aac3d', orig=0x100aac3d, c='Sim_100aac3d', gen=g_aac3d, trials=600),
    dict(name='Sim_100abab0', orig=0x100abab0, c='Sim_100abab0', gen=g_abab0, void=True, trials=500),
    dict(name='Sim_100abc5d', orig=0x100abc5d, c='Sim_100abc5d', gen=g_abc5d, trials=600),
    dict(name='Sim_100b487e', orig=0x100b487e, c='Sim_100b487e', gen=g_b487e, trials=500),
    dict(name='Sim_100b53bb', orig=0x100b53bb, c='Sim_100b53bb', gen=g_b53bb, trials=500),
    dict(name='Sim_100bda0b', orig=0x100bda0b, c='Sim_100bda0b', gen=g_bda0b, trials=500),
    dict(name='Sim_100d1780', orig=0x100d1780, c='Sim_100d1780', gen=g_d1780, trials=600),
    dict(name='Sim_100d8ce7', orig=0x100d8ce7, c='Sim_100d8ce7', gen=g_five, void=True, trials=300),
    dict(name='Sim_100d9fc0', orig=0x100d9fc0, c='Sim_100d9fc0', gen=g_none, void=True, trials=300),
    dict(name='Sim_100e122a', orig=0x100e122a, c='Sim_100e122a', gen=g_none, void=True, trials=300),
    dict(name='Sim_100e2740', orig=0x100e2740, c='Sim_100e2740', gen=g_two, void=True, trials=300),
    dict(name='Sim_100f9f50', orig=0x100f9f50, c='Sim_100f9f50', gen=g_f9f50, trials=1000),
    dict(name='Sim_100f9f88', orig=0x100f9f88, c='Sim_100f9f88', gen=g_f9f88, trials=500),
    dict(name='Str_Cat', orig=0x100fc6a0, c='Str_Cat', gen=g_cat, trials=1000),
    dict(name='Str_NCat', orig=0x100fd870, c='Str_NCat', gen=g_ncat, trials=1500),
    dict(name='Str_CSpan', orig=0x100ff1a0, c='Str_CSpan', gen=g_span, trials=800),
    dict(name='Str_FindLast', orig=0x100ffdb0, c='Str_FindLast', gen=g_last, trials=800),
    dict(name='Sim_10108710', orig=0x10108710, c='Sim_10108710', gen=g_flags, trials=600),
    dict(name='Sim_101091f0', orig=0x101091f0, c='Sim_101091f0', gen=g_none, trials=300),
]
