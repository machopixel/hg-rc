# Round 11: 0x1005573d (fatal-error box). Stubbed on both sides: Sim_100b7834 (text lookup), Sim_1005f9aa (log, text
# logged), Sim_100bc451, the CRT exit (0x100ff3b0) and the CRT sprintf (0x100fc780: logs exactly the arguments its
# format takes and writes the format into the buffer). The CRT strcmp runs for real. ShowCursor keeps a display
# counter like Windows; MessageBoxA answers 1 or 2.
exec(open('tools/verify/w10_common.py').read())

SOURCES = closure(['sim/unsorted_1005/w11_error.c'])
helper(0x100b7834, 'Sim_100b7834', 2, sargs=(1,))
helper(0x1005f9aa, 'Sim_1005f9aa', 1, sargs=(0,))
helper(0x100bc451, 'Sim_100bc451', 0)
helper(0x100ff3b0, None, 1)
helper(0x100fc780, None, 2)
NARGS = {0x10141dac: 9, 0x10141dc4: 10, 0x10141ddc: 10, 0x10141df8: 4}
ACTIVE = tuple(HELPERS)


def e_sprintf(uc, k, raw, eax):
    n = NARGS.get(raw[1], 2)
    esp = uc.reg_read(UC_X86_REG_ESP)
    rest = struct.unpack('<%dI' % (n - 2), bytes(uc.mem_read(esp + 12, 4 * (n - 2)))) if n > 2 else ()
    uc.mem_write(raw[0], ustr(uc, raw[1]) + b'\0')
    return [('sprintf_args', rest)], None


EFFECT.update({0x100fc780: e_sprintf})


def show(n, args):
    if n == 0:                                  # the first Windows call of each run: start from the trial's value
        ST['cursor'] = ST['cursor0']
    ST['cursor'] += 1 if args[0] else -1
    return ST['cursor'] & 0xffffffff


def g(ctx):
    r = begin(ctx, *ACTIVE)
    ST['cursor0'] = r.choice([-3, -1, 0, 0, 1, 2])
    ST['box'] = r.choice([1, 1, 2])
    w32(ctx, 0x101653c8, r.choice([0, 0, 0, 1]))
    for a in range(0x101c4bb8, 0x101c4bf0, 4):
        w32(ctx, a, r.getrandbits(32))
    for a in range(0x101c502c, 0x101c5034, 4):
        w32(ctx, a, r.getrandbits(32))
    for a in range(0x101c50d8, 0x101c5108, 4):
        w32(ctx, a, r.getrandbits(32))
    cap = newbuf(ctx, 8)
    wb(ctx, cap, b'Error\0')
    w32(ctx, 0x101c4be0, cap)
    w32(ctx, 0x1017d594, r.getrandbits(32))
    key = newbuf(ctx, 16)
    wb(ctx, key, r.choice([ustr(ctx.m.uc, 0x10141da4) + b'\0', b'MISSION1\0', b'X\0']))
    return [r.choice([0x6e, 0x6f, 0x70, 0x71, 0]), r.getrandbits(32), key]


SPECS = [dict(name='Sim_1005573d', orig=0x1005573d, c='Sim_1005573d', gen=g, void=True, trials=3000,
              api={'ShowCursor': show, 'GetFocus': lambda n, a: 0x1234, 'MessageBoxA': lambda n, a: ST['box']})]