# python -I show.py a10 a12 [...pairs]  -> logic blocks of each pair, with the audit's canon (imports by name)
import sys, os, re, difflib
HERE = os.path.dirname(os.path.abspath(__file__))
src = open(os.path.join(HERE, 'audit.py')).read().split('\ndef build(')[0]
exec(src)
I10, I12 = Img(V10), Img(V12)
I10.starts = set(); I12.starts = set()
DISP = re.compile(r'([+-] )(0x[0-9a-f]+|\d+)\]')
NUM = re.compile(r'0x[0-9a-f]+|\b\d+\b')


def keys(I, f):
    b = I.body(f)
    rows = canon(I, f, b)
    return rows, [re.sub(r'ebp - (0x[0-9a-f]+|\d+)', 'ebp-L', r[0]) for r in rows], b


args = sys.argv[1:]
maxl = 2000
for k in range(0, len(args), 2):
    a10, a12 = int(args[k], 16), int(args[k + 1], 16)
    R10, K10, B10 = keys(I10, a10)
    R12, K12, B12 = keys(I12, a12)
    sm = difflib.SequenceMatcher(None, K10, K12, autojunk=False)
    out = []
    for t, i1, i2, j1, j2 in sm.get_opcodes():
        if t == 'equal':
            continue
        a = [DISP.sub(r'\1D]', x) for x in K10[i1:i2]]
        b = [DISP.sub(r'\1D]', x) for x in K12[j1:j2]]
        if a == b:
            continue
        if i2 - i1 == j2 - j1 and all(NUM.sub('N', x) == NUM.sub('N', y) and all(
                int(p, 0) - int(q, 0) in (0, 4, 8) for p, q in zip(NUM.findall(x), NUM.findall(y))) for x, y in zip(K10[i1:i2], K12[j1:j2])):
            continue
        out.append('  -- %s' % t)
        for i in range(i1, i2):
            r = R10[i]; ins = B10[r[2]]
            out.append('   1.0 %x  %s %s' % (r[2], ins.mnemonic, ins.op_str))
        for j in range(j1, j2):
            r = R12[j]; ins = B12[r[2]]
            out.append('   1.2 %x  %s %s' % (r[2], ins.mnemonic, ins.op_str))
    print('=== 1.0 %x / 1.2 %x  (%d / %d insns), %d lines' % (a10, a12, len(K10), len(K12), len(out)))
    print('\n'.join(out[:maxl]))
    if len(out) > maxl:
        print('   ... %d more' % (len(out) - maxl))
