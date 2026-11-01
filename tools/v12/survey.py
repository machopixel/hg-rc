# Round 33: survey of the 1.2 sim changes still to port. Read-only; writes tools/v12/survey.json and prints counts.
#     python -I tools/v12/survey.py
# 1. Every function start of both DLLs (funcmap.py's prologue rule and every call target); listings as v12_common's _listing (addresses and stack
#    offsets normalised). Exact pairs: tools/v12/funcmap.json. The rest: a 1.2 function is paired with the unpaired
#    1.0 function of the most similar listing (difflib ratio >= 0.6), best pairs first.
# 2. A pair is "layout" when the listings agree once record-field displacements are blanked, else "logic".
# 3. Ported: the 1.0 twin's C body (// FUNCTION: HGSIM <addr>) mentions 1.2, or the 1.2 address is cited in src
#    (HGSIM12 markers, "1.2 0x...", "1.2 (0x...").
# 4. Area: keywords in the C text of the 1.0 twin (for a new 1.2 function: of the nearest paired 1.2 neighbour).
import difflib, glob, json, os, re, struct, sys
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
exec(open('tools/verify/v12_common.py').read())

V10, V12 = V10_DLL, V12_DLL


def func_starts(path):
    """prologue starts (push ebp / mov ebp, esp after a ret / int3 / nop) and the targets of every call rel32 in
    .text (incremental-link jmp thunks followed), as the round-30 analysis counted"""
    b, img, _ = _img(path)
    pe = _pefile.PE(path)
    text = next(s for s in pe.sections if s.Name.startswith(b'.text'))
    lo, hi = text.VirtualAddress, text.VirtualAddress + text.Misc_VirtualSize
    out = set()
    for o in range(1, len(img) - 3):
        if img[o:o + 3] == b'\x55\x8b\xec' and (img[o - 1] in (0xc3, 0xcc, 0x90) or (o > 3 and img[o - 3] == 0xc2)):
            out.add(b + o)
    for o in range(lo, hi - 5):
        if img[o] == 0xe8:
            t = o + 5 + struct.unpack_from('<i', img, o + 1)[0]
            if lo <= t < hi:
                a = _thunk(path, b + t)
                if lo <= a - b < hi:
                    out.add(a)
    return sorted(out)


def listing(path, a):
    try:
        return [x[0] for x in _listing(path, a)]
    except Exception:
        return []


DISP = re.compile(r'([+-] )(0x[0-9a-f]+|\d+)\]')


def blank(lst):
    return [DISP.sub(r'\1D]', x) for x in lst]


# ---- the C side ----
SRC = {}            # 1.0 address -> (file, text of the function and the comment above it)
CITED12 = set()     # 1.2 addresses cited in src
for path in glob.glob('src/**/*.c', recursive=True) + glob.glob('src/**/*.h', recursive=True):
    text = open(path, encoding='utf-8', errors='replace').read()
    for m in re.finditer(r'HGSIM12 (0x1[0-9a-f]{7})', text):
        CITED12.add(int(m.group(1), 16))
    for m in re.finditer(r'1\.2[^\n]{0,40}?(0x10[0-9a-f]{6})', text):
        CITED12.add(int(m.group(1), 16))
    if not path.endswith('.c'):
        continue
    marks = [(m.start(), int(m.group(1), 16)) for m in re.finditer(r'// FUNCTION: HGSIM (0x1[0-9a-f]{7})', text)]
    for i, (pos, a) in enumerate(marks):
        start = text.rfind('\n\n', 0, pos)
        end = marks[i + 1][0] if i + 1 < len(marks) else len(text)
        SRC[a] = (path.replace('\\', '/').replace('src/sim/', ''), text[max(start, 0):end])

AREAS = [
    ('network', r'\bnet\b|network|packet|anet|\bdp[A-Z]|multiplayer|session|lobby|\bplayers?\b.*\bremote'),
    ('sound', r'sound|audio|voice|\.wav|music|\bcd ?audio|dsound|mixer'),
    ('input', r'keyboard|\bkeys?\b|mouse|joystick|input|controller|force feedback|command dispatcher'),
    ('HUD', r'\bhud\b|radar|gauge|cockpit|message line|status bar|\bfont\b|on-screen text|\bmfd\b'),
    ('weapons', r'weapon|\bshots?\b|ammo|missile|rocket|grenade|\bgun\b|projectile|beam|explosion|\bfire\b'),
    ('AI', r'\bai\b|pilot|enemy|squad|behaviour|behavior|tactic|\bpath\b|waypoint|order|target'),
    ('damage', r'damage|\bhit\b|armou?r|destroy|kill|wreck|part destroyed'),
    ('mission', r'mission|script|objective|trigger|briefing|scenario|event'),
    ('render', r'render|polygon|texture|camera|\bsky\b|light|draw|view|particle|terrain|clip|project'),
    ('C runtime', r'unsorted_10(0f|10)/|\bcrt\b|locale|stdio|heap|malloc|printf'),
    ('platform', r'platform/|win/|window|direct ?draw|display mode'),
]


def area_of(a10):
    if a10 not in SRC:
        return 'not converted'
    f, t = SRC[a10]
    s = (f + '\n' + t).lower()
    for name, rx in AREAS:
        if re.search(rx, s):
            return name
    return 'other'


def main():
    s10, s12 = func_starts(V10), func_starts(V12)
    L10 = {a: listing(V10, a) for a in s10}
    L12 = {a: listing(V12, a) for a in s12}
    L10 = {a: l for a, l in L10.items() if len(l) >= 4}
    L12 = {a: l for a, l in L12.items() if len(l) >= 4}
    exact = {int(k, 16): int(v, 16) for k, v in json.load(open('tools/v12/funcmap.json')).items()}
    pairs = {a12: a10 for a12, a10 in exact.items() if a12 in L12 and a10 in L10}
    used10 = set(pairs.values())
    rest12 = [a for a in L12 if a not in pairs]
    rest10 = [a for a in L10 if a not in used10]
    # 1.2 functions whose listing equals a 1.0 one up to addresses but were ambiguous in funcmap: pair by equality
    by10 = {}
    for a in rest10:
        by10.setdefault(tuple(L10[a]), []).append(a)
    for a12 in list(rest12):
        c = by10.get(tuple(L12[a12]))
        if c:
            a10 = min(c, key=lambda x: abs((x - 0x10000000) - (a12 - 0x10000000)))
            pairs[a12] = a10
            c.remove(a10)
            used10.add(a10)
            rest12.remove(a12)
    rest10 = [a for a in L10 if a not in used10]
    # similarity pairing
    cand = []
    sm = difflib.SequenceMatcher(autojunk=False)
    for a10 in rest10:
        sm.set_seq2(L10[a10])
        n10 = len(L10[a10])
        for a12 in rest12:
            n12 = len(L12[a12])
            if min(n10, n12) / max(n10, n12) < 0.5:
                continue
            sm.set_seq1(L12[a12])
            if sm.real_quick_ratio() < 0.6 or sm.quick_ratio() < 0.6:
                continue
            r = sm.ratio()
            if r >= 0.6:
                cand.append((r, a12, a10))
    cand.sort(reverse=True)
    for r, a12, a10 in cand:
        if a12 in pairs or a10 in used10:
            continue
        pairs[a12] = a10
        used10.add(a10)
    new12 = sorted(a for a in L12 if a not in pairs)
    gone10 = sorted(a for a in L10 if a not in used10)
    rows = []
    paired12 = sorted(pairs)
    for a12, a10 in sorted(pairs.items()):
        A, B = L10[a10], L12[a12]
        if A == B:
            kind = 'same'
        elif blank(A) == blank(B):
            kind = 'layout'
        else:
            kind = 'logic'
        if kind == 'same':
            continue
        body = SRC.get(a10, ('', ''))[1]
        ported = a12 in CITED12 or bool(re.search(r'1\.2', body))
        sm2 = difflib.SequenceMatcher(None, A, B, autojunk=False)
        d = sum(max(i2 - i1, j2 - j1) for t, i1, i2, j1, j2 in sm2.get_opcodes() if t != 'equal')
        rows.append(dict(a12=hex(a12), a10=hex(a10), kind=kind, d=d, n=len(B), ported=ported,
                         c=SRC.get(a10, ('-',))[0], area=area_of(a10)))
    import bisect
    for a12 in new12:
        k = bisect.bisect_left(paired12, a12)
        near = [paired12[j] for j in (k - 1, k) if 0 <= j < len(paired12)]
        nb = min(near, key=lambda x: abs(x - a12)) if near else None
        a10n = pairs.get(nb)
        rows.append(dict(a12=hex(a12), a10=None, kind='new', d=len(L12[a12]), n=len(L12[a12]),
                         ported=a12 in CITED12, c=SRC.get(a10n, ('-',))[0] if a10n else '-',
                         near12=hex(nb) if nb else None, area=area_of(a10n) if a10n else 'other'))
    json.dump(dict(rows=rows, gone10=[hex(a) for a in gone10], n10=len(L10), n12=len(L12),
                   same=sum(1 for a12, a10 in pairs.items() if L10[a10] == L12[a12])),
              open('tools/v12/survey.json', 'w'), indent=1)
    from collections import Counter
    c = Counter((r['area'], r['kind'], r['ported']) for r in rows)
    print('functions 1.0 %d, 1.2 %d, paired %d, same %d' % (len(L10), len(L12), len(pairs),
          sum(1 for a12, a10 in pairs.items() if L10[a10] == L12[a12])))
    for k in sorted(c):
        print(k, c[k])
    print('1.0 only:', len(gone10))


main()
