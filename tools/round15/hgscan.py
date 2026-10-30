# Round 15: the HGBT / HGXP chunks of the shipped BWD files (archive and loose), the fields 0x100dcc12 reads.
#   python -I tools/round15/hgscan.py <game dir>
import os, sys, struct
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bwdscan

game = sys.argv[1]
src = list(bwdscan.files(game))
print('files', len(src))
for name, data in src:
    for tag, pos, c, broken in bwdscan.chunks(data):
        if tag == b'HGBT':
            cnt, = struct.unpack_from('<h', c, 0x23a)
            nm = c[0x23c:].split(b'\0')[0]
            ids = struct.unpack_from('<%dh' % max(cnt, 0), c, 0xe) if 0 < cnt < 256 else ()
            print(name, 'HGBT size', len(c), 'count', cnt, 'name', nm, 'w20c', struct.unpack_from('<hh', c, 0x20c),
                  'w234', struct.unpack_from('<h', c, 0x234), 'ids', ids[:12])
        elif tag == b'HGXP':
            cnt, = struct.unpack_from('<h', c, 0x23c)
            nm = c[0x23e:].split(b'\0')[0]
            print(name, 'HGXP size', len(c), 'count', cnt, 'name', nm)
lens = {}
for name, data in src:
    for tag, pos, c, broken in bwdscan.chunks(data):
        if tag in (b'HGBT', b'HGXP', b'PITF', b'VPTF', b'GP\0\0'):
            off, coff = {b'HGBT': (0x23c, 0x23a), b'HGXP': (0x23e, 0x23c)}.get(tag, (0xa, None))
            n = len(c[off:].split(b'\0')[0])
            cnt = struct.unpack_from('<h', c, coff)[0] if coff else 0
            k = lens.setdefault(tag, [0, 0, 0])
            k[0] += 1
            k[1] = max(k[1], n)
            k[2] = max(k[2], cnt)
for tag in (b'HGBT', b'HGXP', b'PITF', b'VPTF', b'GP\0\0'):
    k = lens.get(tag, [0, 0, 0])
    print('%-8r chunks %d, longest name %d, largest count %d' % (tag, k[0], k[1], k[2]))
