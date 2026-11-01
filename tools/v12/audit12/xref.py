# python -I xref.py 10|12 addr... : callers (functions whose body references addr, thunks followed) with pair info
import os, sys, json, re
HERE = os.path.dirname(os.path.abspath(__file__))
exec(open(os.path.join(HERE, 'audit.py')).read().split('\ndef main(')[0])
w = sys.argv[1]
I = Img(V12 if w == '12' else V10)
F, _, _, _ = build(I, load_ghidra12() if w == '12' else load_ghidra10())
A = json.load(open(os.path.join(HERE, 'audit.json')))
p12 = {r['a12']: r['a10'] for r in A['rows']}; p10 = {r['a10']: r['a12'] for r in A['rows']}
tw = p12 if w == '12' else p10
targets = [int(x, 16) for x in sys.argv[2:]]
for t in targets:
    hits = []
    for f, b in F.items():
        for a, i in b.items():
            for x in re.findall(r'0x1[0-9a-f]{7}', i.op_str):
                if thunk(I, int(x, 16)) == t:
                    hits.append((hex(f), hex(a), i.mnemonic))
    print(hex(t), 'twin', tw.get(hex(t)), [(h[0], h[1], h[2], 'twin ' + str(tw.get(h[0]))) for h in hits][:12])
