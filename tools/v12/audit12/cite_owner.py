# every 0x10xxxxxx literal in src (and in v12 specs) that falls inside a 1.2 function body -> that function
import os, json, re, glob
HERE = os.path.dirname(os.path.abspath(__file__))
exec(open(os.path.join(HERE, 'audit.py')).read().split('\ndef main(')[0])
I12 = Img(V12)
F12, _, _, _ = build(I12, load_ghidra12())
own = {}
for f, b in F12.items():
    for a, i in b.items():
        for k in range(i.size):
            own.setdefault(a + k, f)
def scan(paths, need12):
    hits = {}
    for p in paths:
        t = open(p, encoding='utf-8', errors='replace').read()
        for m in re.finditer(r'(1\.2|HGSIM12|Sim12_)[^\n]{0,80}', t):
            for x in re.findall(r'(?:0x|Sim12_)(10[0-9a-f]{6})', m.group(0)):
                a = int(x, 16)
                if a in own:
                    hits.setdefault(hex(own[a]), set()).add(os.path.basename(p))
    return hits
src = scan(glob.glob('src/**/*.[ch]', recursive=True), True)
spec = {}
for p in glob.glob('tools/verify/spec*.py'):
    t = open(p, encoding='utf-8', errors='replace').read()
    if 'v12_spec' not in t and 'orig12' not in t:
        continue
    for x in re.findall(r'0x(10[0-9a-f]{6})', t):
        a = int(x, 16)
        if a in F12:
            spec.setdefault(hex(a), set()).add(os.path.basename(p))
json.dump(dict(src={k: sorted(v) for k, v in src.items()}, spec={k: sorted(v) for k, v in spec.items()}),
          open(os.path.join(HERE, 'cite_owner.json'), 'w'))
print(len(src), len(spec))
