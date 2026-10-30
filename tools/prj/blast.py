"""PKWARE DCL "implode" decompressor (port of zlib contrib/blast.c)."""

_LITLEN = bytes([
    11, 124, 8, 7, 28, 7, 188, 13, 76, 4, 10, 8, 12, 10, 12, 10, 8, 23, 8,
    9, 7, 6, 7, 8, 7, 6, 55, 8, 23, 24, 12, 11, 7, 9, 11, 12, 6, 7, 22, 5,
    7, 24, 6, 11, 9, 6, 7, 22, 7, 11, 38, 7, 9, 8, 25, 11, 8, 11, 9, 12,
    8, 12, 5, 38, 5, 38, 5, 11, 7, 5, 6, 21, 6, 10, 53, 8, 7, 24, 10, 27,
    44, 253, 253, 253, 252, 252, 252, 13, 12, 45, 12, 45, 12, 61, 12, 45,
    44, 173])
_LENLEN = bytes([2, 35, 36, 53, 38, 23])
_DISTLEN = bytes([2, 20, 53, 230, 247, 151, 248])
_BASE = (3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264)
_EXTRA = (0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8)
_MAXBITS = 13


def _construct(rep):
    lengths = []
    for b in rep:
        lengths += [b & 15] * ((b >> 4) + 1)
    count = [0] * (_MAXBITS + 1)
    for n in lengths:
        count[n] += 1
    offs = [0] * (_MAXBITS + 1)
    for n in range(1, _MAXBITS):
        offs[n + 1] = offs[n] + count[n]
    symbol = [0] * len(lengths)
    for sym, n in enumerate(lengths):
        if n:
            symbol[offs[n]] = sym
            offs[n] += 1
    return count, symbol


_LITCODE = _construct(_LITLEN)
_LENCODE = _construct(_LENLEN)
_DISTCODE = _construct(_DISTLEN)


class _Bits:
    def __init__(self, data):
        self.data, self.pos, self.buf, self.cnt = data, 0, 0, 0

    def bits(self, need):
        while self.cnt < need:
            if self.pos >= len(self.data):
                raise ValueError("implode: ran out of input")
            self.buf |= self.data[self.pos] << self.cnt
            self.pos += 1
            self.cnt += 8
        val = self.buf & ((1 << need) - 1)
        self.buf >>= need
        self.cnt -= need
        return val

    def decode(self, table):
        count, symbol = table
        code = first = index = 0
        for n in range(1, _MAXBITS + 1):
            code |= self.bits(1) ^ 1  # codes are stored bit-inverted
            c = count[n]
            if code < first + c:
                return symbol[index + code - first]
            index += c
            first = (first + c) << 1
            code <<= 1
        raise ValueError("implode: bad code")


def explode(data):
    """Decompress a PKWARE DCL implode stream and return the bytes."""
    s = _Bits(data)
    lit = s.bits(8)
    dict_bits = s.bits(8)
    if lit > 1 or dict_bits not in (4, 5, 6):
        raise ValueError("implode: bad header")
    out = bytearray()
    while True:
        if s.bits(1):
            sym = s.decode(_LENCODE)
            length = _BASE[sym] + s.bits(_EXTRA[sym])
            if length == 519:  # end of stream
                break
            sym = 2 if length == 2 else dict_bits
            dist = (s.decode(_DISTCODE) << sym) + s.bits(sym) + 1
            if dist > len(out):
                raise ValueError("implode: distance too far back")
            start = len(out) - dist
            for i in range(length):
                out.append(out[start + i])
        else:
            out.append(s.decode(_LITCODE) if lit else s.bits(8))
    return bytes(out)
