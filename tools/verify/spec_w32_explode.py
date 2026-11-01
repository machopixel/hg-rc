# Round 32, E: 1.2's explosions against 1.2's own code: tracer (spawn 0x10084362, update 0x10083dde, draw 0x10083e8a),
# missile (0x10082556, 0x10081afb, 0x10081bdc).
# Stubbed on both sides (w32_common.py): the pool allocation (answers the generator's record), the engine node calls,
# the projection (answers visible or not, writes nothing), the line of sight, the random ring (by call number), the
# 2D pixel and the particle draw (logs the visible points only: the others are stack leftovers in both). The length
# Sim_10084a94 runs for real (1.2's twin 0x1006f714 is the same code). The C marks 0x20 particles as 1.2 does
# (DECISIONS.md round 32).
# Negative check: W32_NEG=1 builds the C with W32E_MUTATE (the particle one size step bigger).
import os
exec(open('tools/verify/w32_common.py').read())

SOURCES = closure(['sim/fx/w32_explode.c', 'sim/fx/w32_family.c', 'sim/fx/w32_partspec.c', 'sim/fx/w32_draw.c'])
CFLAGS = list(CFLAGS) + (['-DW32C_MUTATE'] if os.environ.get('W32_NEG') == 'c' else ['-DW32E_MUTATE', '-DW32M_MUTATE', '-DW32X_MUTATE', '-DW32C_MUTATE', '-DW32K_MUTATE', '-DW32FS_MUTATE'] if os.environ.get('W32_NEG') else [])
stub12(0x10090c7d, 'Sim_10090c7d', 4, 0x10079c54)
stub12(0x100a9cfb, 'Sim_100a9cfb', 4, 0x1009518b)
stub12(0x100d2669, 'Sim_100d2669', 3, 0x100b87f9)
stub12(0x1009cab0, 'Sim_1009cab0', 7, 0x10089f80)
stub12(0x1009cc82, 'Sim_1009cc82', 1, 0x1008a152)
stub12(0x1002dda9, 'Sim_1002dda9', 3, 0x100229d9)
stub12(0x100aa39f, 'Sim_100aa39f', 1, 0x1009582f)
stub12(0x100a9d47, 'Sim_100a9d47', 4, 0x100951d7)
stub12(0x100aa799, 'Sim_100aa799', 1, 0x10095c29)
stub12(0x100a9d7c, 'Sim_100a9d7c', 4, 0x1009520c)
stub12(0x100aa351, 'Sim_100aa351', 1, 0x100957e1)
stub12(0x1009c929, 'Sim_1009c929', 1, 0x10089e69)
stub12(0x100e84f0, None, 4, 0x100ca6c4)
helper(A12(0x10078c29), 'Sim12_10078c29', 4)
FUNCS.update({0x10083dde: 'Sim12_10083dde', 0x10083e8a: 'Sim12_10083e8a', 0x10081afb: 'Sim12_10081afb',
              0x10081bdc: 'Sim12_10081bdc', 0x1007ffb3: 'Sim12_1007ffb3', 0x1008011d: 'Sim12_1008011d',
              0x10084a87: 'Sim12_10084a87', 0x10084b83: 'Sim12_10084b83', 0x1007e99d: 'Sim12_1007e99d',
              0x1007eb46: 'Sim12_1007eb46', 0x1007dbe7: 'Sim12_1007dbe7', 0x1007dd89: 'Sim12_1007dd89',
              0x1007bb48: 'Sim12_1007bb48', 0x1007bca5: 'Sim12_1007bca5', 0x1007c996: 'Sim12_1007c996',
              0x1007ca8b: 'Sim12_1007ca8b', 0x1007cb80: 'Sim12_1007cb80', 0x1007cc7c: 'Sim12_1007cc7c',
              0x1007cd78: 'Sim12_1007cd78'})
GLOBALS.append((0x1036f050, 0x1028c7d0, 0x20))     # the sparks' colours, kind 0
GLOBALS.append((0x10215b18, 0x1018ef70, 0x8))      # and kind 1
GLOBALS.append((0x10215b20, 0x1018ef80, 0x80))     # the skate's two frame rows
GLOBALS.append((0x101d09c0, 0x1018ef20, 0x20))     # the missile's colour fade
GLOBALS.append((0x10215af0, 0x1018ef40, 0x20))     # the grenade's
GLOBALS.append((0x10215bb0, 0x1018f000, 0x20))     # the dust's size and colour
GLOBALS.append((0x10215bd0, 0x1018f020, 0x20))     # the smoke's colour
GLOBALS.append((0x10217b20, 0x10190f80, 0x20))     # and size
GLOBALS.append((0x10185760, 0x10152a60, 0x400))    # the colour table
GLOBALS.append((0x101a2100, 0x10164d40, 0x402 * 0x88))  # the scene objects, which the smoke's colour reads
EFFECT[0x1009c929] = e_rand
ALL = (0x10090c7d, 0x100a9cfb, 0x100d2669, 0x1009cab0, 0x1009cc82, 0x1002dda9, 0x100aa39f, 0x100a9d47, 0x100aa799,
       0x100a9d7c, 0x100aa351, 0x1009c929, 0x100e84f0, A12(0x10078c29))


def e_pos(uc, k, raw, eax):
    for j in range(3):
        uc.mem_write(raw[1 + j], struct.pack('<I', ST['pos'][j]))
    return None


def e_draw(uc, k, raw, eax):
    out = []
    for i in range(raw[3] if raw[3] <= 16 else 0):
        p = bytes(uc.mem_read(raw[1] + i * 16, 16))
        if struct.unpack_from('<I', p, 12)[0] != 0:
            out.append(('point', (i,) + struct.unpack('<3I', p[:12])))
    return out, None


EFFECT[0x100a9cfb] = e_pos
EFFECT[A12(0x10078c29)] = e_draw


def world(ctx, r):
    ST['seed'] = r.getrandbits(16)
    cam = newbuf(ctx, 0x48)
    wb(ctx, cam, b''.join(struct.pack('<f', r.uniform(-5000, 5000)) for _ in range(18)))
    wf(ctx, cam + 0x18, r.choice([1.0, 0.5, 2.0, 0.0]))
    wf(ctx, cam + 0x40, r.choice([3000.0, 1e9, 0.0, 500.0]))
    w32(ctx, 0x1014cae0, cam)
    w32(ctx, 0x1015dc78, r.getrandbits(16))
    wf(ctx, 0x1015eef4, r.choice([0.0, 1.0, 37.5, r.uniform(-100, 100)]))
    w32(ctx, 0x1017b940, 1 if r.random() < 0.9 else 0)
    w32(ctx, ctx.m.syms['g_w31Tex'], 1)
    w32(ctx, 0x1017cc20, r.choice([1000, 1000, 5]))
    w32(ctx, 0x1017cc24, r.choice([0, 3, 4, 999]))
    w32(ctx, ctx.m.syms['g_w32_1014b58c'], r.choice([0, 1]))
    type_row(ctx, 12)
    type_row(ctx, 10)
    type_row(ctx, 8)
    for t in (0x11, 0x12, 0x13, 0x14):
        type_row(ctx, t)
    wb(ctx, 0x10215bb0, bytes(r.getrandbits(8) for _ in range(0x20)))
    wb(ctx, 0x10215bd0, bytes(r.getrandbits(8) for _ in range(0x20)))
    wb(ctx, 0x10217b20, bytes(r.getrandbits(8) for _ in range(0x20)))
    wb(ctx, 0x10185760, bytes(r.getrandbits(8) for _ in range(0x400)))
    wb(ctx, 0x101a2100 + 290 * 0x88, bytes(r.getrandbits(8) for _ in range(30 * 0x88)))
    wb(ctx, 0x1036f050, bytes(r.getrandbits(8) for _ in range(0x20)))
    wb(ctx, 0x10215b18, bytes(r.getrandbits(8) for _ in range(8)))
    for t in (4, 5, 6, 7, 0xd, 0xe, 0xf, 0x10):
        type_row(ctx, t)
    wb(ctx, 0x10215b20, bytes(r.getrandbits(8) for _ in range(0x80)))
    wb(ctx, 0x10215af0, bytes(r.getrandbits(8) for _ in range(0x20)))
    wb(ctx, 0x101d09c0, bytes(r.getrandbits(8) for _ in range(0x20)))
    rets(0x100d2669, *[r.choice([0, 1, 1, 1]) for _ in range(9)])
    rets(0x1002dda9, r.choice([0, 0, 1]))


def make_rec(ctx, r, t=12):
    rec = newbuf(ctx, 0x680)
    ST['recs'] = ST.get('recs', []) + [rec]
    node = newbuf(ctx, 0x100)
    block = newbuf(ctx, 0x60)
    w32(ctx, node + 0x8c, block)
    w32(ctx, rec + 0x332, node)
    w32(ctx, rec + 0x402, t)
    w16(ctx, rec + 0x10, r.choice([0, 1, 4, 5, 2, 0xffff]))
    w32(ctx, rec + 0x3fa, r.choice([0xffffffff, 0, 1, 4, 31, 40]))
    w32(ctx, rec + 0x33e, r.getrandbits(16))
    wf(ctx, rec + 0x3fe, r.choice([0.0, 1.5, 0.25, float('nan')]))
    for i in range(8):
        p = rec + 0x12 + i * 0x32
        wf(ctx, p, r.uniform(-50, 50), r.uniform(-50, 50), r.uniform(-50, 50))
        if t == 5:
            w32(ctx, p + 0x1a, 0x10215b20 + r.choice([0, 0x40]))
    return rec


def g_spawn(ctx, t=12):
    r = begin(ctx, *ALL)
    ST['recs'] = []
    world(ctx, r)
    rets(0x10090c7d, make_rec(ctx, r, t) if r.random() < 0.9 else 0)
    p = [fb(r.uniform(-5000, 5000)) for _ in range(3)]
    v = [fb(r.choice([r.uniform(-300, 300), 0.0])) for _ in range(3)]
    return p + v


def g_update(ctx, t=12):
    r = begin(ctx, *ALL)
    ST['recs'] = []
    world(ctx, r)
    rec = make_rec(ctx, r, t)
    w32(ctx, 0x1015dc78, (r32(ctx, rec + 0x33e) + r.choice([0, 5, 22, 23, 90, 91, 100, 110, 111, 181, 182, 362, 363, 543, 544, r.randrange(0, 544)])) &
        0xffffffff)
    return [rec]


def g_draw(ctx, t=12):
    r = begin(ctx, *ALL)
    ST['recs'] = []
    world(ctx, r)
    ST['pos'] = [fb(r.choice([r.uniform(-5000, 5000), r.uniform(15000, 30000)])) for _ in range(3)]
    rec = make_rec(ctx, r, t)
    wf(ctx, rec + 0x34e, r.uniform(-300, 300), r.uniform(-300, 300), r.uniform(-300, 300))
    w32(ctx, rec + 0x346, r.choice([0x1f, 0, r.getrandbits(32)]))
    return [0] if r.random() < 0.02 else [rec]


SPECS = [spec12('Sim12_10084362', 0x10084362, g_spawn, 3000, void=True),
         spec12('Sim12_10083dde', 0x10083dde, g_update, 1500),
         spec12('Sim12_10083e8a', 0x10083e8a, g_draw, 3000),
         spec12('Sim12_10082556', 0x10082556, lambda c: g_spawn(c, 10), 3000, void=True),
         spec12('Sim12_10081afb', 0x10081afb, lambda c: g_update(c, 10), 1500),
         spec12('Sim12_10081bdc', 0x10081bdc, lambda c: g_draw(c, 10), 3000),
         spec12('Sim12_100805dc', 0x100805dc, lambda c: g_spawn(c, 8), 3000, void=True),
         spec12('Sim12_1007ffb3', 0x1007ffb3, lambda c: g_update(c, 8), 1500),
         spec12('Sim12_1008011d', 0x1008011d, lambda c: g_draw(c, 8), 3000)]


def g_dust(ctx):
    r = begin(ctx, *ALL)
    world(ctx, r)
    rets(0x10090c7d, make_rec(ctx, r, 0x11) if r.random() < 0.9 else 0)
    p = [fb(r.uniform(-5000, 5000)) for _ in range(3)]
    v = [fb(r.choice([r.uniform(-300, 300), 0.0])) for _ in range(3)]
    kind = r.choice([0, 0xe, 0xf, 0x14, 0x1b, 0x1c, 0x20, 0x21, 0x23, 0x24, 0x25, 0x28, 0x2d, r.getrandbits(32)])
    return p + v + [r.getrandbits(32), kind]


def g_dust_draw(ctx):
    args = g_draw(ctx, r_type(ctx))
    w32(ctx, args[0] + 0x34a, ctx.rng.choice([0, 1, 4, 30, 255, ctx.rng.getrandbits(32)])) if args[0] else None
    return args


def r_type(ctx):
    return ctx.rng.choice([0x11, 0x12, 0x13, 0x14])


SPECS += [spec12('Sim12_1008514d', 0x1008514d, g_dust, 3000, void=True),
          spec12('Sim12_10084a87', 0x10084a87, lambda c: g_update(c, r_type(c)), 1500),
          spec12('Sim12_10084b83', 0x10084b83, g_dust_draw, 3000)]



def g_smoke_draw(ctx):
    args = g_draw(ctx, ctx.rng.choice([6, 7]))
    if args[0]:
        w32(ctx, args[0] + 0x34a, ctx.rng.choice([0, 1, 4, 30, 255, ctx.rng.getrandbits(32)]))
    return args


SPECS += [spec12('Sim12_1007f8a9', 0x1007f8a9, lambda c: g_spawn(c, 7), 3000, void=True),
          spec12('Sim12_1007f19f', 0x1007f19f, lambda c: g_spawn(c, 6), 3000, void=True),
          spec12('Sim12_1007e99d', 0x1007e99d, lambda c: g_update(c, c.rng.choice([6, 7])), 1500),
          spec12('Sim12_1007eb46', 0x1007eb46, g_smoke_draw, 3000)]



def _rows(m, frm, to):
    for rec in ST.get('recs', []):
        for i in range(8):
            a = rec + 0x2c + i * 0x32
            v = struct.unpack('<I', bytes(m.uc.mem_read(a, 4)))[0]
            if frm <= v < frm + 0x80:
                m.uc.mem_write(a, struct.pack('<I', v - frm + to))


TO12_MORE.append(lambda m: _rows(m, 0x10215b20, A12(0x1018ef80)))
FROM12_MORE.append(lambda m: _rows(m, A12(0x1018ef80), 0x10215b20))


def g_skate_draw(ctx):
    args = g_draw(ctx, 5)
    if args[0]:
        w32(ctx, args[0] + 0x34a, ctx.rng.choice([0, 1, 4, 30, 255, ctx.rng.getrandbits(32)]))
    return args


SPECS += [spec12('Sim12_1007e25c', 0x1007e25c, lambda c: g_spawn(c, 5), 3000, void=True),
          spec12('Sim12_1007dbe7', 0x1007dbe7, lambda c: g_update(c, 5), 1500),
          spec12('Sim12_1007dd89', 0x1007dd89, g_skate_draw, 3000)]

# round 33: the footstep dust (type 0x15), 1.2's copy of the skate dust's spawn, update and draw
FUNCS.update({0x10085992: 'Sim12_10085992', 0x10085b34: 'Sim12_10085b34'})


def g_foot_draw(ctx):
    args = g_draw(ctx, 0x15)
    if args[0]:
        w32(ctx, args[0] + 0x34a, ctx.rng.choice([0, 1, 4, 30, 255, ctx.rng.getrandbits(32)]))
    return args


SPECS += [spec12('Sim12_10086007', 0x10086007, lambda c: g_spawn(c, 0x15), 3000, void=True),
          spec12('Sim12_10085992', 0x10085992, lambda c: g_update(c, 0x15), 1000),
          spec12('Sim12_10085b34', 0x10085b34, g_foot_draw, 1000)]



def g_shock(ctx):
    args = g_spawn(ctx, 4)
    return args + [ctx.rng.choice([0, 1, 2, 3, 5, -1, 0x10000])]


def g_shock_update(ctx):
    args = g_update(ctx, 4)
    if args[0]:
        w32(ctx, args[0] + 0x342, ctx.rng.choice([181, 362, 1, 905]))
        wf(ctx, args[0] + 0x1e, ctx.rng.uniform(-50, 50), ctx.rng.uniform(-50, 50), ctx.rng.uniform(-50, 50))
    return args


def g_shock_draw(ctx):
    args = g_draw(ctx, 4)
    if args[0]:
        w32(ctx, args[0] + 0x34a, ctx.rng.choice([0, 1, 4, 30, 255, ctx.rng.getrandbits(32)]))
    return args


SPECS += [spec12('Sim12_1007c1a1', 0x1007c1a1, g_shock, 3000, void=True),
          spec12('Sim12_1007bb48', 0x1007bb48, g_shock_update, 1500),
          spec12('Sim12_1007bca5', 0x1007bca5, g_shock_draw, 3000)]



def g_spark(ctx):
    args = g_spawn(ctx, 0xd)
    r = ctx.rng
    return args + [r.choice([0, 1, 2, 3, -1]), r.choice([0, 0xe, 0xf, 0x14, 0x1b, 0x1c, 0x21, 0x23, 0x25, 0x28, 0x2d,
                                                         r.getrandbits(32)])]


def r_spark(ctx):
    return ctx.rng.choice([0xd, 0xe, 0xf, 0x10])


SPECS += [spec12('Sim12_1007d342', 0x1007d342, g_spark, 3000, void=True),
          spec12('Sim12_1007c996', 0x1007c996, lambda c: g_update(c, r_spark(c)), 1500),
          spec12('Sim12_1007ca8b', 0x1007ca8b, lambda c: g_update(c, r_spark(c)), 1500),
          spec12('Sim12_1007cb80', 0x1007cb80, lambda c: g_update(c, r_spark(c)), 600),
          spec12('Sim12_1007cc7c', 0x1007cc7c, lambda c: g_update(c, r_spark(c)), 600),
          spec12('Sim12_1007cd78', 0x1007cd78, g_dust_draw, 1500)]
