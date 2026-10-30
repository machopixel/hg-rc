# Round 10 wiring, idempotent: python -I tools/round10/wire.py <sim .c file> <sim .h file> "<CONV line>"
# Adds conv_w10.inc to the three include lists of src/hooks/hooks_conv.c (once), the header to its header
# list, the .c file to src/launcher/CMakeLists.txt, and the CONV line to src/hooks/conv_w10.inc.
import os, sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
cfile, hfile, conv = sys.argv[1].replace('\\', '/'), sys.argv[2].replace('\\', '/'), sys.argv[3].strip()


def rw(path, fn):
    t = open(path, newline='').read()
    nl = '\r\n' if '\r\n' in t else '\n'
    n = fn(t, nl)
    if n != t:
        open(path, 'w', newline='').write(n)
        return True
    return False


def hooks(t, nl):
    a = '#include "conv_w9h.inc"' + nl
    if '#include "conv_w10.inc"' not in t:
        assert t.count(a) == 3
        t = t.replace(a, a + '#include "conv_w10.inc"' + nl)
    inc = '#include "../%s"' % hfile[4:] if hfile.startswith('src/') else '#include "../%s"' % hfile
    if inc not in t:
        anchor = '#include "hook.h"' + nl
        assert t.count(anchor) == 1
        t = t.replace(anchor, inc + nl + anchor)
    return t


def cmake(t, nl):
    line = '    ../%s' % (cfile[4:] if cfile.startswith('src/') else cfile)
    if line + nl in t:
        return t
    anchor = '    ../hooks/hooks_conv.c' + nl
    assert t.count(anchor) == 1
    return t.replace(anchor, line + nl + anchor)


def convinc(path):
    t = open(path).read() if os.path.exists(path) else ''
    rva = conv.split(',')[0]
    if rva in t:
        return False
    open(path, 'a').write(conv + '\n')
    return True


print('hooks_conv.c', rw(os.path.join(ROOT, 'src/hooks/hooks_conv.c'), hooks))
print('CMakeLists', rw(os.path.join(ROOT, 'src/launcher/CMakeLists.txt'), cmake))
print('conv_w10.inc', convinc(os.path.join(ROOT, 'src/hooks/conv_w10.inc')))
