# Round 31: the texture name table moved out of the DLL (src/sim/render/w31.h). The 1.0 specs that seed and compare
# the 1.0 table at 0x10a51ac0 (512 groups x 32 frames x 0x18 bytes: name and +0x12 word in the first 0x14 bytes,
# the image at +0x14) run the C on our table (g_w31TexNames, 1.2's layout: frames of 0x1c bytes, the same first
# 0x14 bytes, +0x14 the image kind, +0x18 the image) through a mapping step:
#   w31_pre_c   before the C run: the seeded 1.0 table copied into ours (kind 0);
#   w31_post_c  after the C run: ours copied back into the 1.0 table, so the checker's memory comparison compares
#               the C's table with the original's; a kind other than 0 (only .TEX mode sets one) is written as a
#               poisoned name byte, so it shows as a difference. The sim main loop's pointer into the table
#               ([0x1015dafc]) is mapped the same way.
# Specs: exec this file, add W31_SOURCES to SOURCES and pass SPECS through w31_wrap(SPECS, names).
import struct

W31_SOURCES = ['sim/render/w31_tex.c']
T10, G10, F10 = 0x10a51ac0, 0x300, 0x18
G12, F12 = 0x380, 0x1c
NG, NF = 0x200, 0x20


def _w31_table(m):
    return m.syms['g_w31TexNames']


def w31_pre_c(m):
    _W31_BASE[0] = _w31_table(m)
    old = bytes(m.uc.mem_read(T10, NG * G10))
    out = bytearray(NG * G12)
    for g in range(NG):
        for f in range(NF):
            src = g * G10 + f * F10
            dst = g * G12 + f * F12
            out[dst:dst + 0x14] = old[src:src + 0x14]
            out[dst + 0x18:dst + 0x1c] = old[src + 0x14:src + 0x18]
    m.uc.mem_write(_w31_table(m), bytes(out))


def w31_post_c(m):
    new = bytes(m.uc.mem_read(_w31_table(m), NG * G12))
    out = bytearray(NG * G10)
    for g in range(NG):
        for f in range(NF):
            src = g * G12 + f * F12
            dst = g * G10 + f * F10
            out[dst:dst + 0x14] = new[src:src + 0x14]
            out[dst + 0x14:dst + 0x18] = new[src + 0x18:src + 0x1c]
            if new[src + 0x14:src + 0x18] != bytes(4):
                out[dst] ^= 0x80
    m.uc.mem_write(T10, bytes(out))
    base = _w31_table(m)
    p = struct.unpack('<I', bytes(m.uc.mem_read(0x1015dafc, 4)))[0]
    if base <= p < base + NG * G12:
        off = p - base
        g, r = divmod(off, G12)
        f, b = divmod(r, F12)
        m.uc.mem_write(0x1015dafc, struct.pack('<I', T10 + g * G10 + f * F10 + min(b, F10 - 1)))


_W31_BASE = [0]


def _w31_addr(v):
    """an address in our table as the 1.0 table's address of the same byte"""
    base = _W31_BASE[0]
    if isinstance(v, int) and base and base <= v < base + NG * G12:
        g, r = divmod(v - base, G12)
        f, b = divmod(r, F12)
        return T10 + g * G10 + f * F10 + min(b, F10 - 1)
    return v


def w31_log_map(log_o, log_c):
    """the C's pointers into our table (e.g. a frame handed to the cache release) as the 1.0 table's"""
    def entry(e):
        if isinstance(e, tuple) and len(e) == 2 and isinstance(e[1], (tuple, list)):
            return (e[0], tuple(_w31_addr(v) for v in e[1]))
        if isinstance(e, (tuple, list)):
            return type(e)(_w31_addr(v) for v in e)
        return _w31_addr(e)
    # both logs alike: a generated value that happens to fall in our table's range then maps the same on both sides
    return [entry(e) for e in log_o], [entry(e) for e in log_c]


def w31_wrap(specs, names=None):
    """the mapping step and the ignored C table for the specs named (all when names is None)"""
    for s in specs:
        if names is not None and s['name'] not in names:
            continue
        s['pre_c'] = w31_pre_c
        s['post_c'] = w31_post_c
        ig = dict(s.get('ignore_syms', {}))
        ig['g_w31TexNames'] = NG * G12
        s['ignore_syms'] = ig
        s.setdefault('log_map', w31_log_map)
    return specs
