# residual instructions (numbers and registers blanked, order ignored) of given pairs, with their addresses
import sys, os, re
from collections import Counter, defaultdict
HERE = os.path.dirname(os.path.abspath(__file__))
exec(open(os.path.join(HERE, 'audit.py')).read().split('\ndef build(')[0])
I10, I12 = Img(V10), Img(V12)
I10.starts = set(); I12.starts = set()
NUM = re.compile(r'0x[0-9a-f]+|\b\d+\b')
REG = re.compile(r'\b(e[abcd]x|e[sd]i|[abcd][xlh])\b')
def rows(I, f):
    b = I.body(f)
    return [(REG.sub('R', NUM.sub('N', r[0])), r[2], b[r[2]]) for r in canon(I, f, b)]
a = sys.argv[1:]
for k in range(0, len(a), 2):
    A, B = rows(I10, int(a[k], 16)), rows(I12, int(a[k + 1], 16))
    ca, cb = Counter(x[0] for x in A), Counter(x[0] for x in B)
    ea, eb = ca - cb, cb - ca
    print('=== 1.0 %s / 1.2 %s' % (a[k], a[k + 1]))
    for x in A:
        if ea[x[0]] > 0:
            ea[x[0]] -= 1; print('   1.0 %x  %s %s' % (x[1], x[2].mnemonic, x[2].op_str))
    for x in B:
        if eb[x[0]] > 0:
            eb[x[0]] -= 1; print('   1.2 %x  %s %s' % (x[1], x[2].mnemonic, x[2].op_str))
