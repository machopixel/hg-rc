# Round 17: the flags word (model word +2) that 0x100db78b gives each world object: the low 16 bits of the OBJ chunk's
# dword +0x34. Counts the values over every BWD file of HEAVYG.PRJ and the game folder.
#   python -I tools/round17/objflags.py <game dir>
import os, sys, struct, collections
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'round15'))
import bwdscan

vals, files, objs, small = collections.Counter(), 0, 0, []
for name, data in bwdscan.files(sys.argv[1]):
    files += 1
    for tag, pos, c, broken in bwdscan.chunks(data):
        if tag == b'OBJ\x00' and not broken and len(c) >= 0x38:
            objs += 1
            w = struct.unpack_from('<I', c, 0x34)[0] & 0xffff
            vals[w] += 1
            if 1 <= w <= 4:
                small.append((name, pos, c[0x3a:0x46].split(b'\0')[0]))
print('files', files, 'OBJ chunks', objs)
for v, n in sorted(vals.items()):
    print('  word %#06x  %d' % (v, n))
print('values 1..4:', len(small))
for s in small[:40]:
    print('   ', s)
