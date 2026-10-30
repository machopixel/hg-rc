#include "w8u.h"
#include "w5x.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w7q.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100347d5
void __cdecl Debug_AppendLog(const char *text)
{
    if (G32(0x1015e500) == 0)
        G32(0x1015e500) = (uint32_t)(uintptr_t)Sim_100fd850(SIM_PTR(const char, 0x10139e58),         /* "debug.log" */
                                                           SIM_PTR(const char, 0x10139fbc));        /* "at" */
    if (G32(0x1015e500) != 0) {
        Crt_FPuts(text, (void *)(uintptr_t)G32(0x1015e500));
        Crt_FFlush((void *)(uintptr_t)G32(0x1015e500));
        W8U_FCLOSE_1003((void *)(uintptr_t)G32(0x1015e500));
        G32(0x1015e500) = 0;
    }
}

// FUNCTION: HGSIM 0x100348d8
void __cdecl Debug_Printf(const char *format, ...)
{
    char text[0x100];   /* the original's buffer at [ebp-0x100]; Windows only reads it */
    va_list args;

    va_start(args, format);
    W8U_VSNPRINTF_1003(text, 0x100, format, args);
    va_end(args);
    switch (G32(0x1015e4f8)) {
    case 1:
        Sim_10034777(text);
        break;
    case 2:
        SIM_API(W8uOutputDebugStringFn, 0x10ac55e0)(text);
        break;
    case 3:
        if (G32(0x1015e4fc) != 0)
            SIM_API(W8uMessageBoxFn, 0x10ac57e4)(G32(0x1015e4fc), text, SIM_PTR(const char, 0x10139f58),  /* "DEBUG Message" */
                                                 G32(0x10139e50));
        break;
    case 4:
        Debug_AppendLog(text);
        break;
    }
}
