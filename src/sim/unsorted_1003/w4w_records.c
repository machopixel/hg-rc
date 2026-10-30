#include "w4w.h"
#include "../win/w4w_api.h"
#include "../util/access.h"
#include "../unsorted_1000/w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_1004/w3p.h"

#define HEAP_ALLOC SIM_API(W4wHeapAllocFn, W4W_IAT_HEAPALLOC)
#define HEAP_FREE SIM_API(W4wHeapFreeFn, W4W_IAT_HEAPFREE)

// FUNCTION: HGSIM 0x1003184b
void *__cdecl Sim_1003184b(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f,
                           int count)
{
    uint8_t *block = (uint8_t *)HEAP_ALLOC(W4W_GAME_HEAP, 1, (uint32_t)count * 4u + 0x2cu);
    int i;

    if (block == 0) {
        Mono_PrintText((const char *)GBYTES(0x10138c44));
        return block;
    }
    U32(block, 0) = a;
    U32(block, 4) = b;
    U32(block, 8) = c;
    U32(block, 0xc) = d;
    U32(block, 0x10) = e;
    U32(block, 0x14) = f;
    U32(block, 0x18) = (uint32_t)count;
    for (i = 3; i >= 0; i--)
        U32(block, 0x1c + i * 4) = 0;
    if (count > 0)
        Mem_Fill(block + 0x2c, 0, (uint32_t)count * 4u);
    return block;
}

// FUNCTION: HGSIM 0x10038ccd
void __cdecl Sim_10038ccd(void)
{
    uint8_t *rec = (uint8_t *)HEAP_ALLOC(W4W_GAME_HEAP, 1, 0x698);

    G32(0x109e3770) = (uint32_t)(uintptr_t)rec;
    G32(0x109e3760) = G32(0x109e3770);
    U32(G32(0x109e3770), 0) = 0;
    U32(G32(0x109e3770), 8) = 0;
    U32(G32(0x109e3770), 0x690) = 0;
    U32(G32(0x109e3770), 0x694) = 0;
    U32(G32(0x109e3760), 0) = 0;
    U32(G32(0x109e3760), 8) = 0;
    U32(G32(0x109e3760), 0x690) = 0;
    U32(G32(0x109e3760), 0x694) = 0;
}

// FUNCTION: HGSIM 0x10039202
void __cdecl Sim_10039202(void)
{
    uint32_t rec = G32(0x109e3760);

    while (rec != 0) {
        uint32_t next = U32(rec, 0x690);

        HEAP_FREE(W4W_GAME_HEAP, 1, BYTES(rec));
        rec = next;
    }
}

// FUNCTION: HGSIM 0x10039241
void __cdecl Sim_10039241(void)
{
    Sim_10038ccd();
}

// FUNCTION: HGSIM 0x1003924b
void __cdecl Sim_1003924b(void)
{
    Sim_10039202();
}

// FUNCTION: HGSIM 0x1003c313
void __cdecl Sim_1003c313(void)
{
    uint32_t addr;
    int again = 1;

    for (addr = 0x1015ea8c; addr <= 0x1015eb64; addr += 4)
        G32(addr) = 0;
    while (again != 0)
        again = SIM_API(W4wFreeLibraryFn, W4W_IAT_FREELIBRARY)((void *)(uintptr_t)G32(0x1015eb68));
    G32(0x1015eb68) = 0;
}

static void free_block(uint32_t block)
{
    HEAP_FREE(W4W_GAME_HEAP, 1, BYTES(block));
}

// FUNCTION: HGSIM 0x1003edda
void __cdecl Sim_1003edda(void)
{
    Sim_100420a0();
    if (G32(0x1015eccc) == 0)
        return;
    if (G32(0x1015ec88) != 0)
        free_block(G32(0x1015ec88));
    if (G32(0x1015ecc4) != 0)
        free_block(G32(0x1015ecc4));
    if (G32(0x1015ec90) != 0)
        free_block(G32(0x1015ec90));
    if (G32(0x1015ec94) != 0)
        free_block(G32(0x1015ec94));
    if (G32(0x1015d084) != 0)
        free_block(G32(0x1015d084));
    if (G32(0x1015ecc8) != 0)
        free_block(G32(0x1015ecc8));
    free_block(G32(0x1015d008));
}
