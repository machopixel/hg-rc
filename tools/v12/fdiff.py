# Round 33: the logic differences between a 1.0 function and its 1.2 version, for planning a port. Read-only.
#     python -I tools/v12/fdiff.py 0x1006e22c 0x1005d75c [out.txt]
# The listings are aligned as v12_common's v12_pairs does (absolute addresses and stack offsets ignored), with
# register names blanked too. Replaced blocks that agree once record displacements are blanked as well (layout only:
# 1.2's 4-byte record shift) are counted, not printed; the rest is printed with both sides' real addresses and
# instructions.
import difflib, os, re, sys
os.chdir(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
exec(open('tools/verify/v12_common.py').read())
DISP = re.compile(r'([+-] )(0x[0-9a-f]+|\d+)\]')
REG = re.compile(r'\b(e[abcd]x|e[sd]i|[abcd][xlh])\b')


def raw(path, a):
    b, img, _ = _img(path)
    out, far = [], a
    for i in _md.disasm(img[a - b:a - b + 0x10000], a):
        if (i.mnemonic.startswith('j') or i.mnemonic == 'loop') and i.op_str.startswith('0x'):
            t = int(i.op_str, 16)
            if a <= t < a + 0x10000:
                far = max(far, t)
        out.append('%x  %s %s' % (i.address, i.mnemonic, i.op_str))
        if i.mnemonic == 'ret' and i.address >= far:
            break
    return out


def main():
    a10, a12 = int(sys.argv[1], 16), int(sys.argv[2], 16)
    A, B = _listing(V10_DLL, a10), _listing(V12_DLL, a12)
    RA, RB = raw(V10_DLL, a10), raw(V12_DLL, a12)
    ka = [REG.sub('R', x[0]) for x in A]
    kb = [REG.sub('R', x[0]) for x in B]
    sm = difflib.SequenceMatcher(None, ka, kb, autojunk=False)
    out, layout, logic = [], 0, 0
    for t, i1, i2, j1, j2 in sm.get_opcodes():
        if t == 'equal':
            continue
        if t == 'replace' and [DISP.sub(r'\1D]', x) for x in ka[i1:i2]] == [DISP.sub(r'\1D]', x) for x in kb[j1:j2]]:
            layout += i2 - i1
            continue
        logic += max(i2 - i1, j2 - j1)
        out.append('---- %s' % t)
        out += ['  1.0 ' + RA[k] for k in range(i1, i2)]
        out += ['  1.2 ' + RB[k] for k in range(j1, j2)]
    head = '1.0 %x (%d) / 1.2 %x (%d): %d layout-only instructions, %d in logic blocks' % (
        a10, len(A), a12, len(B), layout, logic)
    text = head + '\n' + '\n'.join(out) + '\n'
    if len(sys.argv) > 3:
        open(sys.argv[3], 'w').write(text)
        print(head)
    else:
        print(text)


main()
