/* Native check of src/sim/crt/hg_winheap.c against Windows' own heap (DECISIONS.md, "Windows calls: memory").
 * Build and run (32-bit mingw, from the repository root):
 *   i686-w64-mingw32-gcc -O2 -I src -o winheap_native.exe tools/verify/winheap_native.c src/sim/crt/hg_winheap.c
 */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "sim/crt/hg_winheap.h"

#define N 600
static unsigned rnd(void) { static unsigned s = 12345; s = s * 1103515245u + 12345u; return s >> 8; }

int main(void)
{
    int round, bad = 0;
    for (round = 0; round < 40; round++) {
        HANDLE wh = HeapCreate(1, 0x1000, 0);
        uint32_t hh = Hg_HeapCreate(1, 0x1000, 0);
        unsigned char *w[N] = {0}, *h[N] = {0};
        unsigned sz[N] = {0};
        long step;
        for (step = 0; step < 20000; step++) {
            int i = rnd() % N, op = rnd() % 10;
            unsigned n = rnd() % (op == 9 ? 70000 : 600), flags = (rnd() % 2) ? 8 : 0, k;
            if (w[i] == NULL || op < 4) {
                if (w[i] != NULL) {
                    int a = HeapFree(wh, 1, w[i]) != 0, b = Hg_HeapFree(hh, 1, h[i]);
                    if (a != b) { printf("free differs\n"); bad++; }
                }
                w[i] = HeapAlloc(wh, 1 | flags, n);
                h[i] = Hg_HeapAlloc(hh, 1 | flags, n);
                if ((w[i] == NULL) != (h[i] == NULL)) { printf("alloc null differs round %d n %u\n", round, n); bad++; }
                if (w[i] == NULL || h[i] == NULL) { if (w[i]) HeapFree(wh, 1, w[i]); if (h[i]) Hg_HeapFree(hh, 1, h[i]); w[i] = h[i] = NULL; continue; }
                if (flags) for (k = 0; k < n; k++) if (h[i][k] != 0 || w[i][k] != 0) { printf("zero fill differs\n"); bad++; break; }
                for (k = 0; k < n; k++) w[i][k] = h[i][k] = (unsigned char)rnd();
                sz[i] = n;
            } else if (op < 8) {
                unsigned char *a = HeapReAlloc(wh, 1 | flags | (op == 7 ? 0x10 : 0), w[i], n);
                unsigned char *b = Hg_HeapReAlloc(hh, 1 | flags | (op == 7 ? 0x10 : 0), h[i], n);
                /* in place: Windows may grow a block when it has room after it, ours never does (both valid) */
                int grow = op == 7 && n > sz[i];
                if ((a == NULL) != (b == NULL) && !(grow && b == NULL)) { printf("realloc null differs n %u old %u inplace %d\n", n, sz[i], op == 7); bad++; }
                if (grow && b != NULL) { printf("ours grew in place\n"); bad++; }
                if (a == NULL || b == NULL) {
                    if (a != NULL) {   /* Windows grew it, ours did not: start that slot again */
                        HeapFree(wh, 1, a); Hg_HeapFree(hh, 1, h[i]); w[i] = h[i] = NULL;
                    }
                    continue;
                }
                w[i] = a; h[i] = b;
                for (k = 0; k < n; k++) if (a[k] != b[k] && (k < sz[i] || flags)) { printf("realloc data differs at %u\n", k); bad++; break; }
                for (k = 0; k < n; k++) w[i][k] = h[i][k] = (unsigned char)rnd();
                sz[i] = n;
            } else {
                SIZE_T a = HeapSize(wh, 1, w[i]); uint32_t b = Hg_HeapSize(hh, 1, h[i]);
                if (a != b) { printf("size differs %u %u\n", (unsigned)a, b); bad++; }
                if ((HeapValidate(wh, 1, w[i]) != 0) != (Hg_HeapValidate(hh, 1, h[i]) != 0)) { printf("validate differs\n"); bad++; }
            }
            if (bad > 10) return 1;
        }
        if (HeapFree(wh, 1, NULL) == 0 || Hg_HeapFree(hh, 1, NULL) == 0) { printf("free null\n"); bad++; }
        if ((HeapDestroy(wh) != 0) != Hg_HeapDestroy(hh)) { printf("destroy differs\n"); bad++; }
        if (Hg_HeapOwns(hh) || Hg_HeapAlloc(hh, 0, 4) != NULL) { printf("destroyed heap still works\n"); bad++; }
    }
    printf("%s: 40 heaps, 800000 steps, %d differences\n", bad ? "FAIL" : "PASS", bad);
    return bad != 0;
}
