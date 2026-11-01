import os, sys, json, re
HERE = os.path.dirname(os.path.abspath(__file__))
exec(open(os.path.join(HERE, 'audit.py')).read().split('\ndef main(')[0])
I = Img(V12)
F, _, _, P = build(I, load_ghidra12())
A = json.load(open(os.path.join(HERE, 'audit.json')))
O = json.load(open(os.path.join(HERE, 'cite_owner.json')))
C = json.load(open(os.path.join(HERE, 'cside.json')))
conv = set(C['conv']) | set(C['marks'])
p12 = {r['a12']: r['a10'] for r in A['rows']}
new = [int(x, 16) for x in A['new12']]
refs = {n: [] for n in new}
nset = set(new)
for f, b in F.items():
    for a, i in b.items():
        for x in re.findall(r'0x1[0-9a-f]{7}', i.op_str):
            t = thunk(I, int(x, 16))
            if t in nset and t != f:
                refs[t].append(f)
for r in I.relocs:
    if I.text(r): continue
    t = I.d32(r)
    if t in nset: refs[t].append('data')
def strs(f):
    out = []
    for a, i in F[f].items():
        if i.mnemonic == 'push':
            for x in re.findall(r'0x1[0-9a-f]{7}', i.op_str):
                v = int(x, 16)
                if not I.text(v) and I.base <= v < I.base + len(I.img):
                    o = v - I.base
                    e = I.img.find(b'\0', o)
                    s = I.img[o:e]
                    if 3 <= len(s) < 80 and all(32 <= c < 127 for c in s):
                        out.append(s.decode())
    return out
rows = []
for n in new:
    cs = sorted(set(refs[n]), key=str)
    cinfo = []
    for c in cs:
        if c == 'data':
            cinfo.append('data'); continue
        tw = p12.get(hex(c))
        cinfo.append('%x(%s%s)' % (c, tw or 'new', ',C' if tw in conv else ''))
    rows.append(dict(a12=hex(n), n=len(F[n]), callers=cinfo, cite=O['src'].get(hex(n)), spec=O['spec'].get(hex(n)), strs=strs(n)[:3]))
json.dump(rows, open(os.path.join(HERE, 'new12.json'), 'w'), indent=0)
for r in rows:
    print(r['a12'], r['n'], r['callers'][:4], 'CITE' if r['cite'] else '', 'SPEC' if r['spec'] else '', r['strs'])
