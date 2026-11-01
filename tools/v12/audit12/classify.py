# Second pass over audit.json's logic rows: is the difference only layout (displacements, reordering, strides)?
# Residual = 1.2 instructions (fully blanked) not matched by a 1.0 instruction of the same blanked form.
import sys, os, re, difflib, json
from collections import Counter
HERE = os.path.dirname(os.path.abspath(__file__))
exec(open(os.path.join(HERE, 'audit.py')).read().split('\ndef build(')[0])
I10, I12 = Img(V10), Img(V12)
I10.starts = set(); I12.starts = set()
NUM = re.compile(r'0x[0-9a-f]+|\b\d+\b')


def keys(I, f):
    b = I.body(f)
    rows = canon(I, f, b)
    return rows, b


def blank(k):
    # all numbers blanked, registers kept
    return NUM.sub('N', k)


def strides(k):
    return k.startswith('imul') or k.startswith('lea') or k.startswith('push N') or k.startswith('sub esp')


A = json.load(open(os.path.join(HERE, 'audit.json')))
out = []
for r in A['rows']:
    if not r['logic']:
        continue
    a10, a12 = int(r['a10'], 16), int(r['a12'], 16)
    R10, B10 = keys(I10, a10)
    R12, B12 = keys(I12, a12)
    k10 = [blank(x[0]) for x in R10]
    k12 = [blank(x[0]) for x in R12]
    # register-insensitive too (reordering changes register choice)
    REG = re.compile(r'\b(e[abcd]x|e[sd]i|[abcd][xlh])\b')
    c10 = Counter(REG.sub('R', x) for x in k10)
    c12 = Counter(REG.sub('R', x) for x in k12)
    extra12 = c12 - c10
    extra10 = c10 - c12
    # numeric-only differences: aligned same-form lines whose numbers differ (strides, constants)
    sm = difflib.SequenceMatcher(None, k10, k12, autojunk=False)
    numdiff = []
    for t, i1, i2, j1, j2 in sm.get_opcodes():
        if t != 'equal':
            continue
        for k in range(i2 - i1):
            x, y = R10[i1 + k][0], R12[j1 + k][0]
            if x == y:
                continue
            nx = [int(v, 0) for v in NUM.findall(x)]
            ny = [int(v, 0) for v in NUM.findall(y)]
            if 'ptr [' in x and all(p - q in (0, 4, 8) for p, q in zip(nx, ny)):
                continue   # record shift (checked by fieldmap elsewhere)
            if x.startswith(('sub esp', 'add esp')) or 'ebp' in x and all(p - q in (0, 4, 8) or abs(p - q) < 0x200 for p, q in zip(nx, ny)) and 'ptr [ebp' in x:
                continue   # frame / stack slot
            numdiff.append((hex(R10[i1 + k][2]), x, hex(R12[j1 + k][2]), y))
    kind = 'layout' if not extra12 and not extra10 and not numdiff else (
        'numbers' if not extra12 and not extra10 else 'logic')
    out.append(dict(a10=r['a10'], a12=r['a12'], n10=r['n10'], n12=r['n12'], kind=kind,
                    add=sum(extra12.values()), rem=sum(extra10.values()), numdiff=numdiff[:12],
                    nnum=len(numdiff)))
json.dump(out, open(os.path.join(HERE, 'classify.json'), 'w'), indent=0)
print(Counter(o['kind'] for o in out))
