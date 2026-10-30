# Round 24: the labels 0x1007581a (menu list draw) can get, against its wrap width (DECISIONS.md round 24). A label
# reaches the never-written bytes of [ebp-0x104] only when it has at least width + 2 characters and no space in its
# first width + 2 (strncpy(width + 2) leaves no NUL, buf[width + 1] is not a space, strchr then reads on). The width
# is [0x10149950 + 4 * [0x101c64f0]]: 25 (320 x 200), 27 (640 x 480), 0 (1024 x 768).
# Labels: every default text of the sim text loader 0x100ae7c0 (the shipped game has no text file, so the defaults
# are the texts) and the group names that Sim_1000fee8 copies into three label buffers (0x101c4a40 / 5180 / 4860):
# +0x57 of each 0x97-byte row of every MTBL chunk, in every HEAVYG.PRJ entry and every loose file of the game folder.
#   python -I tools/round24/menuscan.py <HGSIM.DLL> <game dir>
import os, sys, struct, glob, bisect
import pefile, capstone
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'prj'))
import prjtool

pe = pefile.PE(sys.argv[1])
img = pe.get_memory_mapped_image()
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
texts = {}
last = None
for i in md.disasm(img[0xae7c0:0xae7c0 + 15847], 0x100ae7c0):
    if i.mnemonic == 'push' and i.op_str.startswith('0x101'):
        last = int(i.op_str, 16)
    elif i.mnemonic == 'mov' and i.op_str.startswith('dword ptr [0x101c') and i.op_str.endswith('eax') and last:
        texts[i.op_str[11:21]] = img[last - 0x10000000:last - 0x10000000 + 300].split(b'\0')[0]
        last = None
for a in (0x1014e290, 0x1014e29c, 0x1014e2a8):      # the three inline buffers' defaults
    texts[hex(a)] = img[a - 0x10000000:a - 0x10000000 + 300].split(b'\0')[0]
widths = [struct.unpack_from('<i', img, 0x149950 + 4 * k)[0] for k in range(3)]

game = sys.argv[2]
_, entries, prj = prjtool.open_archive(game)
names = []
nchunks = nfiles = 0
sources = [(e.filename, lambda e=e: prjtool.read_entry(prj, e)) for e in entries]
for p in glob.glob(os.path.join(game, '**', '*'), recursive=True):
    if os.path.isfile(p) and not p.upper().endswith(('.PRJ', '.MAP')):
        sources.append((p, lambda p=p: open(p, 'rb').read()))
for fname, load in sources:
    d = load()
    nfiles += 1
    i = d.find(b'MTBL')
    while i >= 0:
        size = struct.unpack_from('<I', d, i + 4)[0] if i + 8 <= len(d) else 0
        if 0x26 <= size <= len(d) - i:
            nchunks += 1
            for r in range((size - 0x26) // 0x97):
                m = i + 0x26 + r * 0x97
                names.append((fname, struct.unpack_from('<I', d, m)[0], d[m + 0x57:m + 0x97].split(b'\0')[0]))
        i = d.find(b'MTBL', i + 4)
print('default texts %d, longest %d' % (len(texts), max(len(t) for t in texts.values())))
print('files %d, MTBL chunks %d, group names %d, longest %d' %
      (nfiles, nchunks, len(names), max((len(n) for _, _, n in names), default=0)))
for k, w in enumerate(widths):
    bad_t = [t for t in texts.values() if len(t) >= w + 2 and b' ' not in t[:w + 2]]
    bad_n = [(f, n) for f, _, n in names if len(n) >= w + 2 and b' ' not in n[:w + 2]]
    print('width %d (index %d): %d texts and %d group names reach the unwritten bytes' % (w, k, len(bad_t), len(bad_n)))
    for f, n in bad_n[:5]:
        print('   ', f, n)
