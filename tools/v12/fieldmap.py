# Round 33: the 1.0 -> 1.2 record-field offset map, from the survey's "layout" pairs (functions whose listings agree
# once displacements are blanked). Each aligned instruction pair with a different displacement counts one vote
# (1.0 offset -> 1.2 offset, with its base register form). Read-only; writes tools/v12/fieldmap.json, prints the map.
#     python -I tools/v12/fieldmap.py     (after survey.py)
import json, os, re
from collections import Counter, defaultdict
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
exec(open('tools/verify/v12_common.py').read())

DISP = re.compile(r'\[(\w+) ([+-]) (0x[0-9a-f]+|\d+)\]')
rows = json.load(open('tools/v12/survey.json'))['rows']
votes = Counter()
where = defaultdict(set)


def ops(path, a):
    b, img, _ = _img(path)
    out, far = [], a
    for i in _md.disasm(img[a - b:a - b + 0x8000], a):
        if (i.mnemonic.startswith('j') or i.mnemonic == 'loop') and i.op_str.startswith('0x'):
            t = int(i.op_str, 16)
            if a <= t < a + 0x8000:
                far = max(far, t)
        out.append((i.mnemonic, i.op_str))
        if i.mnemonic == 'ret' and i.address >= far:
            break
    return out


for r in rows:
    if r['kind'] != 'layout':
        continue
    A, B = ops(V10_DLL, int(r['a10'], 16)), ops(V12_DLL, int(r['a12'], 16))
    if len(A) != len(B):
        continue
    for (m1, o1), (m2, o2) in zip(A, B):
        d1, d2 = DISP.findall(o1), DISP.findall(o2)
        for (r1, s1, v1), (r2, s2, v2) in zip(d1, d2):
            if r1 == 'ebp' or r1 == 'esp':
                continue
            x1 = int(v1, 0) * (-1 if s1 == '-' else 1)
            x2 = int(v2, 0) * (-1 if s2 == '-' else 1)
            if x1 != x2:
                votes[(x1, x2)] += 1
                where[(x1, x2)].add(r['a12'])

shift = Counter(x2 - x1 for (x1, x2) in votes.elements())
out = [dict(off10=hex(x1), off12=hex(x2), delta=x2 - x1, votes=n, funcs12=sorted(where[(x1, x2)])[:6])
       for (x1, x2), n in sorted(votes.items())]
json.dump(dict(pairs=out, deltas=dict(shift.most_common())), open('tools/v12/fieldmap.json', 'w'), indent=1)
print('deltas (1.2 - 1.0) by votes:', shift.most_common(12))
for o in out:
    if o['delta'] != -4:
        print(o)
