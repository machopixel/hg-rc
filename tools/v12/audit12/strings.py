# aligned immediate data addresses (push / mov reg, imm) whose target bytes look like text: compare the strings
import os, re, json, difflib
HERE = os.path.dirname(os.path.abspath(__file__))
exec(open(os.path.join(HERE, 'audit.py')).read().split('\ndef main(')[0])
I10, I12 = Img(V10), Img(V12)
F10, _, _, _ = build(I10, load_ghidra10())
F12, _, _, _ = build(I12, load_ghidra12())
A = json.load(open(os.path.join(HERE, 'audit.json')))
C = json.load(open(os.path.join(HERE, 'cside.json')))
conv = set(C['conv'] + C['marks'])
def s(I, a):
    o = a - I.base
    if not (0 <= o < len(I.img)): return None
    e = I.img.find(b'\0', o, o + 400)
    if e < 0: return None
    t = I.img[o:e]
    if len(t) >= 2 and all(32 <= c < 127 or c in (9, 10, 13) for c in t): return t
    return None
seen = set(); out = []
for r in A['rows']:
    f10, f12 = int(r['a10'], 16), int(r['a12'], 16)
    c10 = canon(I10, f10, F10[f10]); c12 = canon(I12, f12, F12[f12])
    sm = difflib.SequenceMatcher(None, [x[0] for x in c10], [x[0] for x in c12], autojunk=False)
    for t, i1, i2, j1, j2 in sm.get_opcodes():
        if t != 'equal': continue
        for k in range(i2 - i1):
            x, y = c10[i1 + k], c12[j1 + k]
            if 'ptr' in x[0] or x[0].startswith(('call', 'jmp')): continue
            for u, v in zip(x[1], y[1]):
                if (u, v) in seen: continue
                seen.add((u, v))
                a, b = s(I10, u), s(I12, v)
                if (a or b) and a != b:
                    out.append((r['a10'], 'C' if r['a10'] in conv else '-', hex(u), a, hex(v), b))
for o in out: print(o)
print(len(out), 'of', len(seen))
