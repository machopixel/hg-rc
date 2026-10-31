# Round 32: the 1.0 functions closest to a 1.2 function (for the ones funcmap.py could not pair): the listings
# compared as funcmap.py lists them (difflib ratio), candidates of similar length only. Prints the best five.
#     python -I tools/v12/funcfind.py 0x1001b3a1 [...]
import difflib, sys
exec(open('tools/v12/funcmap.py').read().split('t10, t12 = ')[0])

C10 = []
for a in starts(V10_DLL):
    try:
        C10.append((a, [x[0] for x in _listing(V10_DLL, a)]))
    except Exception:
        pass
for arg in sys.argv[1:]:
    a12 = int(arg, 16)
    L = [x[0] for x in _listing(V12_DLL, a12)]
    best = []
    for a10, M in C10:
        if not (0.7 * len(L) <= len(M) <= 1.3 * len(L) + 4):
            continue
        r = difflib.SequenceMatcher(None, L, M, autojunk=False).ratio()
        best.append((r, a10, len(M)))
    best.sort(reverse=True)
    print(hex(a12), len(L), ['%s %.3f (%d)' % (hex(a), r, n) for r, a, n in best[:5]])
