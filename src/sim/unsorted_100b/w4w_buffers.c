#include "w4w.h"
#include "../win/w4w_api.h"
#include "../win/w4a.h"
#include "../util/access.h"
#include "../unsorted_100f/w2e.h"

static void release(uint32_t addr)
{
    if (G32(addr) != 0) {
        SIM_API(W4wHeapFreeFn, W4W_IAT_HEAPFREE)(W4W_GAME_HEAP, 1, BYTES(G32(addr)));
        G32(addr) = 0;
    }
}

// FUNCTION: HGSIM 0x100b372b
void __cdecl Sim_100b372b(void)
{
    release(0x1017d214);
    release(0x1017d204);
    release(0x1017d1f4);
    release(0x1017d208);
    release(0x1017d1f8);
    G32(0x1017d20c) = 0;
    G32(0x1017d210) = 0;
    G32(0x101a1160) = 0;
    G32(0x101c42a4) = G32(0x101a1160);
    G32(0x101c42a0) = 0;
    G32(0x101a1168) = G32(0x101c42a0);
}

// FUNCTION: HGSIM 0x100b35b0
int __cdecl Sim_100b35b0(void)
{
    int ok = 0;
    uint32_t size = 0x9000;

    Sim_100b372b();
    G32(0x1017d210) = size >> 3;
    G32(0x1017d214) = (uint32_t)(uintptr_t)Heap_AllocZero(size);
    if (G32(0x1017d214) != 0) {
        G32(0x101a1160) = size >> 2;
        G32(0x101c42a4) = G32(0x101a1160);
        G32(0x1017d204) = (uint32_t)(uintptr_t)Heap_AllocZero(size);
        if (G32(0x1017d204) != 0) {
            G32(0x1017d1f4) = (uint32_t)(uintptr_t)Heap_AllocZero(size);
            if (G32(0x1017d1f4) != 0) {
                G32(0x1017d208) = (uint32_t)(uintptr_t)Heap_AllocZero(size);
                if (G32(0x1017d208) != 0) {
                    G32(0x1017d1f8) = (uint32_t)(uintptr_t)Heap_AllocZero(size);
                    if (G32(0x1017d1f8) != 0)
                        ok = 1;
                }
            }
        }
    }
    return ok;
}

// FUNCTION: HGSIM 0x100ba285
void __cdecl Sim_100ba285(char *s)
{
    int n = (int)Str_Length(s);

    while (n > 0 && (s[n] == ' ' || s[n] == 0 || s[n] == '\r' || s[n] == '\n' || s[n] == '\t'))
        n--;
    n++;
    s[n] = 0;
}

// FUNCTION: HGSIM 0x100bd1b6
void __cdecl Sim_100bd1b6(void)
{
    uint32_t i;

    for (i = 1; i < G32(0x1017e1bc) + 1u; i++)
        SIM_API(W4wFreeLibraryFn, W4W_IAT_FREELIBRARY)((void *)(uintptr_t)G32(0x1017dfd0 + i * 32u));
}
