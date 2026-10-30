# Round 15: chunk census of every BWD world file in HEAVYG.PRJ and the game folder (for 0x100dcc12).
#   python -I tools/round15/bwdscan.py <game dir> [tag ...]
# A BWD file is a list of chunks: tag (4 bytes), size (4 bytes, header included), data. The first chunk has the file
# tag [0x101349e8] and is 0xc bytes long (Sim_100279f1). Prints the number of files, the chunk count per tag and,
# for the tags given, one line per chunk (file, offset, hex of the first 0x40 bytes).
import os, sys, collections
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'prj'))
import prjtool


def chunks(data):
    pos = 0
    while pos + 8 <= len(data):
        tag = data[pos:pos + 4]
        size = 0xc if pos == 0 else int.from_bytes(data[pos + 4:pos + 8], 'little')
        if size < 8 or pos + size > len(data):
            yield tag, pos, data[pos:], True
            return
        yield tag, pos, data[pos:pos + size], False
        pos += size


def files(game, loose=True):
    """(name, data) of every BWD entry of HEAVYG.PRJ, then (with loose) the *.BWD files in the game folder."""
    _, entries, prj = prjtool.open_archive(game)
    for e in entries:
        if e.ext.upper() == 'BWD':
            yield e.filename, prjtool.read_entry(prj, e)
    if loose:
        import glob
        for p in sorted(glob.glob(os.path.join(game, '*.BWD'))):
            yield os.path.basename(p) + ' (loose)', open(p, 'rb').read()


if __name__ == '__main__':
    game, want = sys.argv[1], {t.encode().ljust(4, b'\0') for t in sys.argv[2:]}
    count, nfiles, bad = collections.Counter(), 0, 0
    for name, data in files(game):
        nfiles += 1
        for tag, pos, c, broken in chunks(data):
            bad += broken
            count[tag] += 1
            if tag in want:
                print(name, hex(pos), len(c), c[:0x40].hex())
    print('files', nfiles, 'broken', bad)
    for t, n in sorted(count.items()):
        print(t, n)
