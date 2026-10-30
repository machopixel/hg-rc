#include <stdarg.h>
#include "w9f.h"
#include "w5y.h"
#include "../util/access.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2g.h"
#include "../unsorted_100f/w2j.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_1010/w7r.h"
#include "../unsorted_1010/w8A.h"

// FUNCTION: HGSIM 0x100b78d0
int32_t __cdecl Text_FormatNumbered(char *out, uint32_t key, ...)
{
    char **slots = 0;
    int32_t nslots = 0;
    int32_t extra = 0;
    const char *src;
    char *fmt;
    char *all;
    char *pct;
    char *piece;
    uint32_t span;
    int32_t pos;
    int32_t i;
    va_list args;

    key = Sim_100b7834((const uint32_t *)(uintptr_t)G32(0x1017d594), key);
    if (key == 0)
        return -1;
    src = (const char *)(uintptr_t)key;
    fmt = (char *)Heap_Malloc(Str_Length(src) + 1u);
    if (fmt == 0)
        return -1;
    fmt[0] = 0;
    while ((pct = Crt_StrChr(src, '%')) != 0) {
        if (pct - src != 0)
            Str_NCat(fmt, src, (unsigned)(pct - src));
        if (pct[1] == '%') {
            Str_Cat(fmt, (const char *)GBYTES(0x10150988));                         /* "%%" */
            src = pct + 2;
            continue;
        }
        if (Sim_100fd710(pct, (const char *)GBYTES(0x1015098c), &pos) == 0)        /* "%%%d:" */
            return -1;
        src = pct;
        pct = Crt_StrChr(pct, ':');
        Str_NCat(fmt, src, (unsigned)(pct - src + 1));
        span = Str_CSpan(pct, (const char *)GBYTES(0x10150994));                     /* "diouxXeEfgGcCsSnp" */
        extra = extra + (int32_t)span + 1;
        if (pos > nslots) {
            slots = (char **)Heap_Realloc(slots, (uint32_t)pos << 2);
            if (slots == 0)
                return -1;
            Mem_Fill(slots + nslots, 0, (uint32_t)(pos - nslots) << 2);
            nslots = pos;
        }
        piece = (char *)Heap_Malloc(span + 2u);
        pos = pos - 1;
        slots[pos] = piece;
        if (slots[pos] == 0)
            return -1;
        slots[pos][0] = '%';
        pct = pct + 1;
        Str_CopyN(slots[pos] + 1, pct, span);
        slots[pos][1 + span] = 0;
        src = pct + span;
    }
    Str_Cat(fmt, src);
    all = (char *)Heap_Malloc((uint32_t)(extra + 1 + nslots));
    if (all == 0)
        return -1;
    all[0] = 0;
    for (i = 0; i < nslots; i++) {
        if (slots[i] != 0) {
            Str_Cat(all, slots[i]);
            Str_Cat(all, (const char *)GBYTES(0x101509a8));                          /* "\x7f" */
        }
    }
    va_start(args, key);
    if (Crt_VSPrintf(out, all, (void *)args) == -1) {
        va_end(args);
        return -1;
    }
    va_end(args);
    Heap_FreeBlock((uint32_t)(uintptr_t)all);
    src = out;
    for (i = 0; i < nslots; i++) {
        if (slots[i] != 0) {
            src = Str_NextToken_100fd560((char *)(uintptr_t)src, (const char *)GBYTES(0x101509ac));   /* "\x7f" */
            if (src == 0)
                return -1;
            slots[i] = (char *)Heap_Realloc(slots[i], Str_Length(src) + 1u);
            if (slots[i] == 0)
                return -1;
            Str_Copy(slots[i], src);
            src = 0;
        }
    }
    out[0] = 0;
    src = fmt;
    while ((pct = Crt_StrChr(src, '%')) != 0) {
        if (pct - src != 0)
            Str_NCat(out, src, (unsigned)(pct - src));
        if (pct[1] == '%') {
            Str_Cat(out, (const char *)GBYTES(0x101509b0));                          /* "%" */
            src = pct + 2;
            continue;
        }
        if (Sim_100fd710(pct, (const char *)GBYTES(0x101509b4), &pos) == 0)        /* "%%%d:" */
            return -1;
        pos = pos - 1;
        Str_Cat(out, slots[pos]);
        src = Crt_StrChr(pct, ':') + 1;
    }
    Str_Cat(out, src);
    for (i = 0; i < nslots; i++) {
        if (slots[i] != 0)
            Heap_FreeBlock((uint32_t)(uintptr_t)slots[i]);
    }
    Heap_FreeBlock((uint32_t)(uintptr_t)slots);
    Heap_FreeBlock((uint32_t)(uintptr_t)fmt);
    return (int32_t)Str_Length(out);
}
