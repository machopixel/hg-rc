# Round 32: a 1.2 -> 1.0 function address map. Every function of each DLL (a "push ebp / mov ebp, esp" start after
# a ret, int3 or nop) is listed as v12_common's _listing does (absolute addresses and stack offsets left out); a 1.2
# function whose listing is the same as exactly one 1.0 function's (and no other 1.2 function's) is paired with it.
# Writes tools/v12/funcmap.json {"0x1.2": "0x1.0"}; prints how many were paired.
#     python -I tools/v12/funcmap.py
import hashlib, json, os
exec(open('tools/verify/v12_common.py').read())


def starts(path):
    b, img, _ = _img(path)
    out = []
    for o in range(1, len(img) - 3):
        if img[o:o + 3] == b'\x55\x8b\xec' and (img[o - 1] in (0xc3, 0xcc, 0x90) or (o > 3 and img[o - 3] == 0xc2)):
            out.append(b + o)
    return out


def table(path):
    t = {}
    for a in starts(path):
        try:
            lst = _listing(path, a)
        except Exception:
            continue
        if len(lst) < 4:
            continue
        h = hashlib.sha1('\n'.join(x[0] for x in lst).encode()).hexdigest()
        t.setdefault(h, []).append(a)
    return t


t10, t12 = table(V10_DLL), table(V12_DLL)
out = {}
for h, a12 in t12.items():
    if len(a12) == 1 and len(t10.get(h, ())) == 1:
        out['0x%08x' % a12[0]] = '0x%08x' % t10[h][0]
json.dump(out, open(os.path.join('tools', 'v12', 'funcmap.json'), 'w'), indent=0, sort_keys=True)
print('%d of %d 1.2 functions paired' % (len(out), sum(len(v) for v in t12.values())))

# Globals: in every paired function the absolute addresses of the same instructions pair up; an address pairs when
# every function that uses it agrees. Writes tools/v12/globalmap.json {"0x1.2": "0x1.0"}.
votes = {}
for a12, a10 in out.items():
    L12, L10 = _listing(V12_DLL, int(a12, 16)), _listing(V10_DLL, int(a10, 16))
    for (k12, n12), (k10, n10) in zip(L12, L10):
        if k12.startswith('call'):
            continue
        for x12, x10 in zip(n12, n10):
            votes.setdefault(x12, set()).add(x10)
gm = {'0x%08x' % k: '0x%08x' % next(iter(v)) for k, v in votes.items() if len(v) == 1}
json.dump(gm, open(os.path.join('tools', 'v12', 'globalmap.json'), 'w'), indent=0, sort_keys=True)
print('%d globals paired, %d ambiguous' % (len(gm), sum(1 for v in votes.values() if len(v) > 1)))
