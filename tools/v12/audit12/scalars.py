# every aligned (1.0, 1.2) data operand of the paired code: compare the bytes the instruction reads (its operand size)
# for direct (non-indexed) accesses into initialised .data / .rdata; indexed tables listed separately
import os, re, json, struct, difflib
from collections import defaultdict
HERE = os.path.dirname(os.path.abspath(__file__))
exec(open(os.path.join(HERE, 'audit.py')).read().split('\ndef main(')[0])
I10, I12 = Img(V10), Img(V12)
F10, _, _, _ = build(I10, load_ghidra10())
F12, _, _, _ = build(I12, load_ghidra12())
A = json.load(open(os.path.join(HERE, 'audit.json')))
C = json.load(open(os.path.join(HERE, 'cside.json')))
conv = set(int(x, 16) for x in C['conv'] + C['marks'])
import pefile
def secs(p):
    pe = pefile.PE(p); b = pe.OPTIONAL_HEADER.ImageBase
    return [(b + s.VirtualAddress, b + s.VirtualAddress + s.SizeOfRawData, s.Name.rstrip(b'\0').decode()) for s in pe.sections]
S10, S12 = secs(V10), secs(V12)
def sec(S, a):
    for lo, hi, n in S:
        if lo <= a < hi: return n
SZ = {'byte': 1, 'word': 2, 'dword': 4, 'qword': 8, 'tbyte': 10}
direct = {}; indexed = defaultdict(set)
for r in A['rows']:
    f10, f12 = int(r['a10'], 16), int(r['a12'], 16)
    c10 = canon(I10, f10, F10[f10]); c12 = canon(I12, f12, F12[f12])
    sm = difflib.SequenceMatcher(None, [x[0] for x in c10], [x[0] for x in c12], autojunk=False)
    for t, i1, i2, j1, j2 in sm.get_opcodes():
        if t != 'equal': continue
        for k in range(i2 - i1):
            x, y = c10[i1 + k], c12[j1 + k]
            if x[0].startswith(('call', 'jmp F')): continue
            ins = F10[f10][x[2]]
            m = re.search(r'(byte|word|dword|qword|tbyte) ptr \[([^\]]*)\]', ins.op_str)
            for u, v in zip(x[1], y[1]):
                if sec(S10, u) not in ('.data', '.rdata') or sec(S12, v) not in ('.data', '.rdata'): continue
                if m and ('*' in m.group(2) or '+' in m.group(2) and not m.group(2).startswith('0x')):
                    indexed[(u, v)].add((r['a10'], f10 in conv))
                elif m:
                    direct.setdefault((u, v, SZ[m.group(1)]), set()).add((r['a10'], f10 in conv))
out = []
for (u, v, n), users in direct.items():
    a = I10.img[u - I10.base:u - I10.base + n]; b = I12.img[v - I12.base:v - I12.base + n]
    if a != b and u + n <= max(hi for lo, hi, nm in S10 if nm in ('.data', '.rdata')):
        # skip pointers (relocated)
        if u in I10.relocs or v in I12.relocs: continue
        out.append((hex(u), hex(v), n, a.hex(), b.hex(), sorted(users)[:3]))
json.dump(dict(direct=out, indexed=[(hex(u), hex(v), sorted(s)[:3]) for (u, v), s in indexed.items()]), open(os.path.join(HERE, 'scalars.json'), 'w'))
print('direct differing', len(out), 'of', len(direct), '; indexed tables', len(indexed))
for o in sorted(out): print(o)
