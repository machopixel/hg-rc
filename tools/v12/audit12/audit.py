# Independent 1.2 audit: every 1.2 function vs its 1.0 twin, addresses mapped (callees by pairing, data by votes).
# python -I audit.py  (cwd = repo root). Writes audit.json next to this script.
import bisect, difflib, hashlib, json, os, re, struct, sys
from collections import Counter, defaultdict
import pefile, capstone

HERE = os.path.dirname(os.path.abspath(__file__))
V10, V12 = '../GameRip/HGSIM.DLL', '../GameRip_12/hgsim.dll'
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = False


class Img:
    def __init__(s, path):
        pe = pefile.PE(path)
        s.base = pe.OPTIONAL_HEADER.ImageBase
        s.img = pe.get_memory_mapped_image()
        t = next(x for x in pe.sections if x.Name.startswith(b'.text'))
        s.tlo, s.thi = s.base + t.VirtualAddress, s.base + t.VirtualAddress + t.Misc_VirtualSize
        s.relocs = set()
        for blk in getattr(pe, 'DIRECTORY_ENTRY_BASERELOC', []):
            for e in blk.entries:
                if e.type == 3:
                    s.relocs.add(s.base + e.rva)
        s.imports = {}
        for d in getattr(pe, 'DIRECTORY_ENTRY_IMPORT', []):
            for i in d.imports:
                s.imports[i.address] = (d.dll.decode() + '!' + (i.name.decode() if i.name else str(i.ordinal)))
        s.cache = {}
        # data inside .text: dword jump tables (>= 2 consecutive relocated dwords pointing into .text) and the
        # byte index tables that follow them (small bytes up to the next prologue / int3 / nop padding)
        s.jt = set()
        tr = sorted(r for r in s.relocs if s.text(r) and s.text(s.d32(r)))
        trs = set(tr)
        for r in tr:
            if r + 4 in trs or r - 4 in trs:
                s.jt.update(range(r, r + 4))
                if r + 4 not in trs:
                    p = r + 4
                    while p < s.thi and s.img[p - s.base] < 0x40 and s.img[p - s.base:p - s.base + 3] != b'\x55\x8b\xec' \
                            and p < r + 0x400:
                        s.jt.add(p)
                        p += 1

    def d32(s, a):
        return struct.unpack_from('<I', s.img, a - s.base)[0]

    def text(s, a):
        return s.tlo <= a < s.thi

    def insn(s, a):
        if a not in s.cache:
            o = a - s.base
            r = next(md.disasm(s.img[o:o + 16], a), None)
            s.cache[a] = r
        return s.cache[a]

    def body(s, start):
        """recursive descent: {addr: insn}; jump tables followed through relocated dwords"""
        out, work = {}, [start]
        while work:
            a = work.pop()
            while a not in out and s.text(a) and a not in s.jt:
                i = s.insn(a)
                if i is None:
                    break
                out[a] = i
                m, op = i.mnemonic, i.op_str
                nxt = a + i.size
                if m in ('ret', 'retf', 'int3', 'hlt'):
                    break
                if m == 'jmp':
                    if op.startswith('0x'):
                        t = int(op, 16)
                        # tail jump to another known function start ends here
                        if t in s.starts and t != start:
                            break
                        work.append(t)
                    else:
                        mt = re.search(r'\*\s*4\s*\+\s*(0x[0-9a-f]+)\]', op)
                        if mt:
                            tb = int(mt.group(1), 16)
                            k = 0
                            while tb + 4 * k in s.relocs and s.text(s.d32(tb + 4 * k)) and k < 512:
                                work.append(s.d32(tb + 4 * k))
                                k += 1
                    break
                if (m.startswith('j') or m.startswith('loop')) and op.startswith('0x'):
                    work.append(int(op, 16))
                a = nxt
        return out


def load_ghidra10():
    out = set()
    for ln in open('build/ghidra/HGSIM.idx.txt'):
        p = ln.split('\t')
        if len(p) >= 2:
            out.add(int(p[0], 16))
    return out


def load_ghidra12():
    out = set()
    for r in json.load(open(os.path.join(HERE, 'rea12.json'))):
        a = r['address']
        if a.startswith('EXTERNAL'):
            continue
        out.add(int(a, 16))
    return out


def scan_starts(I):
    s = set()
    img, b = I.img, I.base
    for o in range(I.tlo - b, I.thi - b - 3):
        if img[o:o + 3] == b'\x55\x8b\xec' and (img[o - 1] in (0xc3, 0xcc, 0x90) or img[o - 3] == 0xc2):
            s.add(b + o)
    calls = set()
    for o in range(I.tlo - b, I.thi - b - 5):
        if img[o] == 0xe8:
            t = b + o + 5 + struct.unpack_from('<i', img, o + 1)[0]
            if I.text(t):
                calls.add(t)
    ptrs = set()
    for r in I.relocs:
        if I.text(r) and False:
            pass
        t = I.d32(r)
        if I.text(t):
            ptrs.add(t)
    return s, calls, ptrs


def thunk(I, a):
    i = I.insn(a)
    if i is not None and i.mnemonic == 'jmp' and i.op_str.startswith('0x'):
        return int(i.op_str, 16)
    return a


PRO3 = bytes([0x55, 0x8b, 0xec])
ABS = re.compile(r'0x1[0-9a-f]{7}')


def canon(I, f, body):
    """[(key, [abs operands], addr)] in address order"""
    rows = []
    for a in sorted(body):
        i = body[a]
        op = i.op_str
        nums = [int(x, 16) for x in ABS.findall(op)]
        m = i.mnemonic
        if (m == 'call' or m == 'jmp' or m.startswith('j') or m.startswith('loop')) and op.startswith('0x'):
            t = int(op, 16)
            if m == 'call':
                nums = [thunk(I, t)]
                key = 'call F'
            elif m == 'jmp' and (t < min(body) or t > max(body)) and t not in body:
                nums = [thunk(I, t)]
                key = 'jmp F'
            else:
                nums = []
                key = m + ' J'
        else:
            imp = [x for x in nums if x in I.imports]
            key = m + ' ' + ABS.sub(lambda mm: ('IMP:' + I.imports[int(mm.group(0), 16)]) if int(mm.group(0), 16) in I.imports else 'G', op)
            nums = [x for x in nums if x not in I.imports]
        rows.append((key, nums, a))
    return rows


def build(I, gh):
    pro, calls, ptrs = scan_starts(I)
    I.starts = set()
    calls = {thunk(I, c) for c in calls}
    ptrs = {x for x in (thunk(I, t) for t in ptrs) if I.text(x)}
    # pointer targets that open with push ebp / mov ebp, esp are function starts whatever precedes them; so are
    # the targets of the incremental-link jmp thunks (1.0)
    pro = set(pro) | {t for t in ptrs if I.img[t - I.base:t - I.base + 3] == PRO3}
    for a0 in sorted(gh):
        i = I.insn(a0)
        if i is not None and i.mnemonic == 'jmp' and i.op_str.startswith('0x') and i.size == 5:
            t = int(i.op_str, 16)
            if I.text(t) and I.img[t - I.base:t - I.base + 3] == PRO3:
                pro.add(t)
    # call targets only from decoded code (a raw e8-byte scan finds targets inside data and operands)
    I.starts = set(pro) | set(gh)
    seen, work, calls = set(), sorted((set(pro) | set(gh)) - I.jt), set()
    while work:
        f = work.pop()
        if f in seen:
            continue
        seen.add(f)
        for a, i in I.body(f).items():
            if i.mnemonic == 'call' and i.op_str.startswith('0x'):
                t = thunk(I, int(i.op_str, 16))
                if I.text(t) and t not in I.jt and t not in seen:
                    calls.add(t)
                    work.append(t)
    cand = {c for c in (gh | pro | calls) if c not in I.jt}
    # pass 1: prologue functions, tail jumps only stop at other prologue starts; a start without a prologue that
    # lies inside one of these bodies is a point inside that function
    I.starts = set(pro)
    powner = {}
    for f in sorted(pro):
        if f in I.jt:
            continue
        for a in I.body(f):
            powner.setdefault(a, f)
    I.starts = {c for c in cand if c in pro or powner.get(c, c) == c}
    bodies = {}
    for f in sorted(cand):
        try:
            bodies[f] = I.body(f)
        except Exception:
            pass
    # ownership: a candidate lying inside a Ghidra function's body (not its start) is internal
    owner = {}
    for f in sorted(gh):
        if f in I.starts:
            for a in bodies.get(f, {}):
                owner.setdefault(a, f)
    funcs = {}
    internal = {}
    for f in sorted(cand):
        if f not in bodies or not bodies[f]:
            continue
        o = owner.get(f)
        if f not in pro and powner.get(f, f) != f:
            o = powner[f]
        if o is not None and o != f:
            internal[f] = o
            continue
        b = bodies[f]
        if len(b) == 1 and next(iter(b.values())).mnemonic == 'jmp':
            continue   # incremental-link thunk
        funcs[f] = b
    # pointer targets not covered by any function body and looking like a start (prev byte ret/int3/nop/pad)
    covered = set()
    for f, b in funcs.items():
        covered.update(b)
    extra = []
    for t in sorted(ptrs):
        if t in funcs or t in covered or t in I.jt:
            continue
        pb = I.img[t - I.base - 1]
        if pb in (0xc3, 0xcc, 0x90) or I.img[t - I.base - 3] == 0xc2:
            extra.append(t)
            funcs[t] = I.body(t)
            covered.update(funcs[t])
    return funcs, internal, extra, ptrs


def main():
    I10, I12 = Img(V10), Img(V12)
    F10, int10, ex10, P10 = build(I10, load_ghidra10())
    F12, int12, ex12, P12 = build(I12, load_ghidra12())
    print('functions 1.0 %d (internal %d, ptr-only extra %d); 1.2 %d (internal %d, extra %d)' % (
        len(F10), len(int10), len(ex10), len(F12), len(int12), len(ex12)))
    C10 = {f: canon(I10, f, b) for f, b in F10.items()}
    C12 = {f: canon(I12, f, b) for f, b in F12.items()}

    def blank_stack(k):
        k = re.sub(r'ebp - (0x[0-9a-f]+|\d+)', 'ebp-L', k)
        return k

    K10 = {f: [blank_stack(r[0]) for r in C10[f]] for f in C10}
    K12 = {f: [blank_stack(r[0]) for r in C12[f]] for f in C12}

    # ---- pairing ----
    pairs = {}      # 1.2 -> 1.0
    used = set()

    def h(k):
        return hashlib.sha1('\n'.join(k).encode()).hexdigest()
    H10 = defaultdict(list)
    for f, k in K10.items():
        H10[h(k)].append(f)
    H12 = defaultdict(list)
    for f, k in K12.items():
        H12[h(k)].append(f)
    for hh, l12 in H12.items():
        l10 = H10.get(hh, [])
        if len(l12) == 1 and len(l10) == 1:
            pairs[l12[0]] = l10[0]
            used.add(l10[0])

    def align(f12, f10):
        sm = difflib.SequenceMatcher(None, K10[f10], K12[f12], autojunk=False)
        return sm

    # propagate through calls and data pointers of aligned code
    changed = True
    rounds = 0
    while changed and rounds < 20:
        changed = False
        rounds += 1
        votes = defaultdict(Counter)
        for f12, f10 in list(pairs.items()):
            sm = align(f12, f10)
            for t, i1, i2, j1, j2 in sm.get_opcodes():
                if t != 'equal':
                    continue
                for k in range(i2 - i1):
                    r10, r12 = C10[f10][i1 + k], C12[f12][j1 + k]
                    for u, v in zip(r10[1], r12[1]):
                        if u in F10 and v in F12:
                            votes[v][u] += 1
        for v, c in votes.items():
            if v in pairs:
                continue
            u, n = c.most_common(1)[0]
            if u in used:
                continue
            pairs[v] = u
            used.add(u)
            changed = True
        # equal listings among the rest, nearest address
    # similarity for the rest
    rest12 = [f for f in F12 if f not in pairs and len(K12[f]) >= 3]
    rest10 = [f for f in F10 if f not in used and len(K10[f]) >= 3]
    cand = []
    sm = difflib.SequenceMatcher(autojunk=False)
    for a10 in rest10:
        sm.set_seq2(K10[a10])
        n10 = len(K10[a10])
        for a12 in rest12:
            n12 = len(K12[a12])
            if min(n10, n12) / max(n10, n12) < 0.4:
                continue
            sm.set_seq1(K12[a12])
            if sm.real_quick_ratio() < 0.5 or sm.quick_ratio() < 0.5:
                continue
            r = sm.ratio()
            if r >= 0.5:
                cand.append((r, a12, a10))
    cand.sort(reverse=True)
    simpair = {}
    for r, a12, a10 in cand:
        if a12 in pairs or a10 in used:
            continue
        pairs[a12] = a10
        used.add(a10)
        simpair[a12] = r

    # ---- data map by votes over equal aligned instructions ----
    dvotes = defaultdict(Counter)
    for f12, f10 in pairs.items():
        sm = align(f12, f10)
        for t, i1, i2, j1, j2 in sm.get_opcodes():
            if t != 'equal':
                continue
            for k in range(i2 - i1):
                r10, r12 = C10[f10][i1 + k], C12[f12][j1 + k]
                if r10[0].startswith(('call', 'jmp F')):
                    continue
                for u, v in zip(r10[1], r12[1]):
                    dvotes[u][v] += 1
    dmap = {u: c.most_common(1)[0][0] for u, c in dvotes.items()}

    # repair: a 1.2 function whose pair is not an exact listing gets the 1.0 function of identical listing whose
    # data operands all map through dmap (and whose callees are paired with its callees), when there is exactly one
    def mapped_equal(f12, f10):
        if K10[f10] != K12[f12]:
            return False
        for r10, r12 in zip(C10[f10], C12[f12]):
            for u, v in zip(r10[1], r12[1]):
                if r10[0] in ('call F', 'jmp F'):
                    if v in pairs and pairs[v] != u:
                        return False
                elif dmap.get(u, v) != v:
                    return False
        return True
    byk = defaultdict(list)
    for f, k in K10.items():
        byk[tuple(k)].append(f)
    exact = {v for v, u in pairs.items() if mapped_equal(v, u)}
    nrep = 0
    for _ in range(3):
        inv = {u: v for v, u in pairs.items()}
        for v in sorted(F12):
            if v in exact:
                continue
            c = [u for u in byk.get(tuple(K12[v]), []) if mapped_equal(v, u) and not (u in inv and inv[u] in exact)]
            if len(c) == 1:
                u = c[0]
                if u in inv:
                    del pairs[inv[u]]
                pairs[v] = u
                used.add(u)
                exact.add(v)
                inv = {uu: vv for vv, uu in pairs.items()}
                nrep += 1
    used = set(pairs.values())
    print('repaired pairs', nrep)
    inv12 = {v: u for u, v in pairs.items()}

    # ---- verdict per pair ----
    rows = []
    DISP = re.compile(r'([+-] )(0x[0-9a-f]+|\d+)\]')
    for f12, f10 in sorted(pairs.items()):
        sm = align(f12, f10)
        issues = []
        nlogic = 0
        nlayout = 0
        for t, i1, i2, j1, j2 in sm.get_opcodes():
            if t == 'equal':
                for k in range(i2 - i1):
                    r10, r12 = C10[f10][i1 + k], C12[f12][j1 + k]
                    for u, v in zip(r10[1], r12[1]):
                        if r10[0] in ('call F', 'jmp F'):
                            if u in F10 or v in F12:
                                if pairs.get(v) != u:
                                    issues.append(('callee', hex(r10[2]), hex(u), hex(r12[2]), hex(v),
                                                   hex(pairs[v]) if v in pairs else None))
                        else:
                            if dmap.get(u) != v:
                                # data operand mismatch (maybe import / string / table entry)
                                issues.append(('data', hex(r10[2]), hex(u), hex(r12[2]), hex(v),
                                               hex(dmap[u]) if u in dmap else None))
                continue
            a = [DISP.sub(r'\1D]', x) for x in K10[f10][i1:i2]]
            b = [DISP.sub(r'\1D]', x) for x in K12[f12][j1:j2]]
            if t == 'replace' and a == b:
                nlayout += i2 - i1
                continue
            # immediates 4 lower (record shift as arithmetic)
            if t == 'replace' and i2 - i1 == j2 - j1:
                ok = True
                for x, y in zip(K10[f10][i1:i2], K12[f12][j1:j2]):
                    if x == y:
                        continue
                    nx = re.findall(r'0x[0-9a-f]+|\b\d+\b', x)
                    ny = re.findall(r'0x[0-9a-f]+|\b\d+\b', y)
                    if re.sub(r'0x[0-9a-f]+|\b\d+\b', 'N', x) != re.sub(r'0x[0-9a-f]+|\b\d+\b', 'N', y) or len(nx) != len(ny):
                        ok = False
                        break
                    for p, q in zip(nx, ny):
                        p, q = int(p, 0), int(q, 0)
                        if p != q and p - q not in (4, 8):
                            ok = False
                    if not ok:
                        break
                if ok:
                    nlayout += i2 - i1
                    issues.append(('imm4', hex(C10[f10][i1][2]), hex(C12[f12][j1][2])))
                    continue
            nlogic += max(i2 - i1, j2 - j1)
            issues.append(('logic', t, hex(C10[f10][i1][2]) if i1 < len(C10[f10]) else None,
                           hex(C12[f12][j1][2]) if j1 < len(C12[f12]) else None, i2 - i1, j2 - j1))
        rows.append(dict(a12=hex(f12), a10=hex(f10), n12=len(K12[f12]), n10=len(K10[f10]), logic=nlogic,
                         layout=nlayout, sim=simpair.get(f12), issues=issues))
    new12 = [hex(f) for f in sorted(F12) if f not in pairs]
    gone10 = [hex(f) for f in sorted(F10) if f not in used]
    json.dump(dict(rows=rows, new12=new12, gone10=gone10, int12={hex(k): hex(v) for k, v in int12.items()},
                   ex12=[hex(x) for x in ex12], ex10=[hex(x) for x in ex10],
                   n12={hex(f): len(K12[f]) for f in F12}, n10={hex(f): len(K10[f]) for f in F10},
                   dmap={hex(u): hex(v) for u, v in dmap.items()},
                   dsplit={hex(u): {hex(v): n for v, n in c.items()} for u, c in dvotes.items() if len(c) > 1}),
              open(os.path.join(HERE, 'audit.json'), 'w'))
    same = sum(1 for r in rows if not r['issues'] and not r['logic'] and not r['layout'])
    print('pairs %d (similarity %d), exact-with-mapping %d, new12 %d, gone10 %d' % (
        len(pairs), len(simpair), same, len(new12), len(gone10)))
    print('with logic', sum(1 for r in rows if r['logic']), 'callee/data issues only',
          sum(1 for r in rows if not r['logic'] and any(i[0] in ('callee', 'data') for i in r['issues'])))


main()
