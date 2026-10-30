# Round 18: the node trees of every gear design (GDF) in HEAVYG.PRJ and the game folder, walked the way 0x1005ada5 /
# 0x1005a91a / 0x1005ab44 walk them, to find a named node whose parent key would be the walk's never-written local
# (a nameless node's +0x78 sub-tree reaching a named node through +0x54 links or more nameless nodes).
#   python -I tools/round18/gdfscan.py <game dir>
# GDF layout (Sim_10059e5a / 0x10059a34): first dword 0, the 0x88-byte head (+0x84 item count), then 0x438-byte items:
# +0x54 next and +0x78 sub-tree as item indexes (0 = none), +0x7c six names of 0x20 bytes.
import os, sys, struct, glob
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'prj'))
import prjtool

game = sys.argv[1]
_, entries, prj = prjtool.open_archive(game)
files = [(e.filename, prjtool.read_entry(prj, e)) for e in entries if e.ext.upper() == 'GDF']
for p in glob.glob(os.path.join(game, '*.GDF')) + glob.glob(os.path.join(game, '*.gdf')):
    files.append((os.path.basename(p) + ' (loose)', open(p, 'rb').read()))

total_walks = affected = nameless_with_sub = 0
stats = [0, 0, 0]
per = {}
bad = []
for name, d in files:
    if len(d) < 0x88 or struct.unpack_from('<I', d, 0)[0] != 0:
        print('skip', name, 'size', len(d))
        continue
    n = struct.unpack_from('<i', d, 0x84)[0]
    items = []
    for i in range(n):
        o = 0x88 + i * 0x438
        if o + 0x438 > len(d):
            break
        nxt, sub = struct.unpack_from('<I', d, o + 0x54)[0], struct.unpack_from('<I', d, o + 0x78)[0]
        names = [d[o + 0x7c + 0x20 * k:o + 0x7c + 0x20 * (k + 1)].split(b'\0')[0] for k in range(6)]
        items.append((nxt, sub, names))
    if not items:
        continue
    calls = [0]

    def walk(known, node, idx, depth, path):
        global affected, nameless_with_sub
        calls[0] += 1
        if node == 0 or node >= len(items) or depth > 200:
            return
        nxt, sub, names = items[node]
        e = idx + 1 if names[idx][:1] in (b'g', b'G') else idx
        stats[0] += 1
        stats[1 if names[e][:1] != b'' else 2] += 1
        if names[e][:1] != b'':
            if not known:
                affected += 1
                bad.append((name, idx, node, names[e], path))
            walk(known, nxt, idx, depth + 1, path + ['n%d' % node])
            walk(True, sub, idx, depth + 1, path + ['s%d' % node])
        else:
            if sub:
                nameless_with_sub += 1
            walk(known, nxt, idx, depth + 1, path + ['n%d' % node])
            walk(False, sub, idx, depth + 1, path + ['S%d' % node])

    # 0x1005ada5: for i = 0..3 while the root's name `entry` is non-empty (entry = i, or i + 1 when the root's first
    # name starts with 'g' / 'G'), 0x1005a91a(obj, i): the walk of the root's +0x78 sub-tree with the name slot i
    root = items[0]
    entry = 1 if root[2][0][:1] in (b'g', b'G') else 0
    for i in range(4):
        if entry >= 6 or root[2][entry][:1] == b'':
            break
        total_walks += 1
        walk(True, root[1], i, 0, ['root'])
        entry += 1
    per[name.upper()] = calls[0]
print('files', len(files), 'walks', total_walks, 'nameless nodes with a sub-tree', nameless_with_sub,
      'named nodes reached with the unset parent', affected)
for b in bad[:30]:
    print('  ', b)
print('walk calls per design:', ' '.join('%s=%d' % (k, v) for k, v in sorted(per.items())))
print('named / nameless nodes visited:', stats[1], stats[2])
