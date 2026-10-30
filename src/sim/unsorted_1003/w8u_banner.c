#include <string.h>
#include "w8u.h"
#include "../unsorted_1004/w4u.h"
#include "../unsorted_1004/w8u.h"
#include "../unsorted_1008/w5c.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100355ad
void __cdecl Sim_100355ad(void)
{
    char name[0x100];       /* [ebp-0x11c] */
    int32_t rect[5];        /* [ebp-0x18] */
    void *file;
    void *image;
    void *palette;
    int ready;

    if (G32(0x1015e530) == 0 || *(const char *)(uintptr_t)G32(0x1015e530) == 0) {
        Str_Copy(name, SIM_PTR(const char, 0x1013a020));                    /* "sbannr" */
        Str_Cat(name, SIM_PTR(const char, 0x1014d1b0 + 2 * G32(0x101c64f0)));
        Str_Cat(name, SIM_PTR(const char, 0x1013a028));                     /* "." */
        Str_Cat(name, SIM_PTR(const char, 0x1013a02c));                     /* "gif" */
    } else {
        Str_Copy(name, (const char *)(uintptr_t)G32(0x1015e530));
        Str_Cat(name, SIM_PTR(const char, 0x1014d1b0 + 2 * G32(0x101c64f0)));
        Str_Cat(name, SIM_PTR(const char, 0x1013a030));                     /* "." */
        Str_Cat(name, SIM_PTR(const char, 0x1013a034));                     /* "gif" */
    }
    file = File_LoadWhole(name, 0);
    if (file == 0)
        return;
    image = SIM_API(W8u3HeapAllocFn, 0x10ac5658)(G32(0x1017b8f8), 1, 0x502e);
    if (image != 0) {
        palette = SIM_API(W8u3HeapAllocFn, 0x10ac5658)(G32(0x1017b8f8), 9, 0x300);
        if (palette != 0) {
            ((W8uDriverPaletteFn)(uintptr_t)U32(G32(0x1015e688), 0x14))(0, 0x100, palette, 1);
            memcpy(rect, GBYTES(0x109e3e00), sizeof(rect));
            Sim_1008d36c(rect, rect, (uint32_t)(uintptr_t)file);
            if (G32(0x1017b920) != 0)
                ready = ((W8uDriverQueryFn)(uintptr_t)U32(G32(0x1015e688), 0x20))();
            else
                ready = -1;
            if (ready == 0) {
                W8U_CALL_100ed491(rect, file, image);
                if (G32(0x1017b920) != 0)
                    ((W8uDriverCloseFn)(uintptr_t)U32(G32(0x1015e68c), 0x18))();
            }
            W8U_CALL_100ed6aa(file, palette);
            ((W8uDriverFadeFn)(uintptr_t)U32(G32(0x1015e688), 0x1c))(palette, 0x3c);
            SIM_API(W8u3HeapFreeFn, 0x10ac5634)(G32(0x1017b8f8), 1, palette);
        }
        SIM_API(W8u3HeapFreeFn, 0x10ac5634)(G32(0x1017b8f8), 1, image);
    }
    Heap_Free_100461d0(file);
}
