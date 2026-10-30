# Round 17: 0x10067ec6, the kill credit (DECISIONS.md round 17). The original's never-written killer [ebp-4] is given a
# leftover before each call (random bits, the values seen in play with the original code 0x3ff15798 / 0x3e3fffa0 / 0,
# own-unit ids 0..7, -2); the C core Sim_10067ec6_Credit gets that same value as its unsetKiller argument through a
# trampoline that pushes the dword at its own [ebp-4] (the same stack slot, as the trampoline's frame is the original's).
# 0x1006863d and 0x1006c161 (converted) are logging stubs, so the killer each hit passes on is compared;
# 0x1008486e (converted, answers in st(0)) is a float stub.
# Negative check: W17_NEG=1 gives the C a different unsetKiller (the leftover + 1), which must show as mismatches.
exec(open('tools/verify/w10_common.py').read())
exec(open('tools/verify/w12_common.py').read())

SOURCES = closure(['sim/unsorted_1006/w17_damage.c'])
NEG = bool(os.environ.get('W17_NEG'))
helper(0x1006863d, 'Sim_1006863d', 1)
helper(0x1006c161, 'Sim_1006c161', 5, deref={3: 0x8a})
FSTUB[0x1008486e] = ('Sim_1008486e', 2)
SEEN = [0x3ff15798, 0x3e3fffa0, 0, 0x80000000, 1, 7, 0xfffffffe]
TRAMP = 0x00F00A00


def fb(v):
    return struct.unpack('<I', struct.pack('<f', v))[0]


def install(ctx):
    m = ctx.m
    if getattr(m, '_w17_tramp', False):
        return
    m._w17_tramp = True
    code = b'\x55\x89\xe5'
    code += b'\x8b\x45\xfc' + (b'\x40' if NEG else b'') + b'\x50'      # mov eax, [ebp-4] (+ inc eax); push eax
    code += b'\xff\x75\x10\xff\x75\x0c\xff\x75\x08'
    code += b'\xb8' + struct.pack('<I', m.syms['Sim_10067ec6_Credit']) + b'\xff\xd0'
    code += b'\x83\xc4\x10\x5d\xc3'
    m.raw.mem_write(TRAMP, code)
    m.syms['w17_tramp_credit'] = TRAMP


def unit_rec(ctx, r):
    unit = newbuf(ctx, 0x80)
    rec = newbuf(ctx, 0x40)
    w32(ctx, rec, unit)
    w32(ctx, unit, r.choice([9, 9, 1, 3, 4, 10]))
    w32(ctx, unit + 4, r.choice([0, 1, 5, 7, 12, r.getrandbits(32)]))
    w32(ctx, unit + 0x68, r.choice([fb(r.uniform(-400, 400)), fb(r.uniform(-30, 30)), rfloat(r)]))
    w32(ctx, unit + 0x74, r.choice([fb(r.uniform(-400, 400)), fb(r.uniform(-30, 30)), rfloat(r)]))
    return rec


def g(ctx):
    r = begin(ctx, 0x1006863d, 0x1006c161)
    fstubs(ctx)
    install(ctx)
    a = unit_rec(ctx, r)
    b = unit_rec(ctx, r) if r.random() < 0.85 else 0
    shot = newbuf(ctx, 0x8a)
    w32(ctx, shot + 0x74, r.choice([0x3f000000, 0x3f19999a, 0x3f333333, rfloat(r)]))
    w32(ctx, 0x1015dd5c, r.choice([fb(r.uniform(-1, 1)), fb(-0.866), fb(0.7071), 0x3f350481, 0xbf5db22d, rfloat(r)]))
    w32(ctx, 0x1015dd58, fb(r.uniform(-1, 1)) if r.random() < 0.9 else rfloat(r))
    w32(ctx, 0x1015dd60, fb(r.uniform(-1, 1)) if r.random() < 0.9 else rfloat(r))
    ST['fvals'] = {0x1008486e: [r.choice([fb(r.uniform(-180, 180)), fb(r.choice([45.0, 135.0, -45.0, -135.0, 0.0])),
                                          rfloat(r)])]}
    w32(ctx, frame(3) - 4, r.choice(SEEN + [r.getrandbits(32)] * 3))
    return [a, b, shot]


SPECS = [dict(name='Sim_10067ec6', orig=0x10067ec6, c='w17_tramp_credit', gen=g, void=True,
              trials=400 if NEG else 20000)]
