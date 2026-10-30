# Round 12 helpers, after w10_common.py:  exec(open('tools/verify/w12_common.py').read())
# STACK_TOP / frame(nargs): the original's frame pointer in a checker call (difftest.py: arguments below
# STACK_TOP - 0x100, then the return address, then push ebp), so a spec can put the chosen start value of an
# uninitialised local into the original's stack as its leftover.
# sscanf_stub(): the DLL's sscanf (0x100fd710) as a stub on both sides. Its float conversion needs C runtime
# start-up tables the checker does not have (every %f faults), so the stub converts the fields itself (literals,
# white space, %d, %f, %s, %Ns), stopping at the first mismatch, and returns the count.
import re

STACK_TOP = 0x00800000
_FLOAT = re.compile(rb'\s*[-+]?(\d+\.?\d*|\.\d+)([eE][-+]?\d+)?')
_INT = re.compile(rb'\s*[-+]?\d+')


def frame(nargs):
    return STACK_TOP - 0x100 - 4 * nargs - 4 - 4


def _cstr(uc, a, limit=0x400):
    out = b''
    while len(out) < limit:
        try:
            chunk = bytes(uc.mem_read(a + len(out), 16))
        except Exception:
            break
        if b'\0' in chunk:
            return out + chunk.split(b'\0')[0]
        out += chunk
    return out


def _sscanf(uc, k, raw, eax):
    """A small sscanf: literal characters must match, white space in the format skips any white space, and %d, %f,
    %s and %Ns (width N) convert; it stops at the first mismatch and returns the number of conversions."""
    text, fmt = _cstr(uc, raw[0]), _cstr(uc, raw[1])
    ti, fi, n, out = 0, 0, 0, list(raw[2:])

    def skipws(i):
        while i < len(text) and text[i:i + 1].isspace():
            i += 1
        return i
    while fi < len(fmt):
        c = fmt[fi:fi + 1]
        if c.isspace():
            ti = skipws(ti)
            fi += 1
            continue
        if c != b'%':
            if text[ti:ti + 1] != c:
                break
            ti += 1
            fi += 1
            continue
        m = re.match(rb'%(\d*)([dfs])', fmt[fi:])
        width, kind = int(m.group(1) or 0), m.group(2)
        fi += len(m.group(0))
        ti = skipws(ti)
        if not out:
            break
        dst = out.pop(0)
        if kind == b's':
            e = ti
            while e < len(text) and not text[e:e + 1].isspace() and (width == 0 or e - ti < width):
                e += 1
            if e == ti:
                break
            uc.mem_write(dst, text[ti:e] + b'\0')
        else:
            mm = (_FLOAT if kind == b'f' else _INT).match(text, ti)
            if not mm or mm.end() == ti:
                break
            e = mm.end()
            v = text[ti:e]
            uc.mem_write(dst, struct.pack('<f', float(v)) if kind == b'f' else
                         struct.pack('<i', max(-2**31, min(2**31 - 1, int(v)))))
        ti = e
        n += 1
    return n

def sscanf_stub(nargs):
    helper(0x100fd710, 'Sim_100fd710', nargs)
    EFFECT[0x100fd710] = _sscanf
    return 0x100fd710