#include "w7p.h"
#include "w5x.h"
#include "../util/access.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"

static void *open_record(const char *name, uint32_t modeText, uint32_t mode)
{
    uint8_t *rec = Heap_Malloc(0xc);

    if (rec == 0)
        return 0;
    U32(rec, 0) = (uint32_t)(uintptr_t)Sim_100fd850(name, SIM_PTR(const char, modeText));
    if (U32(rec, 0) == 0) {
        Heap_FreeBlock((uint32_t)(uintptr_t)rec);
        return 0;
    }
    U32(rec, 4) = 0;
    U32(rec, 8) = mode;
    return rec;
}

// FUNCTION: HGSIM 0x100ae344
void *__cdecl FileRec_OpenWrite(const char *name)
{
    return open_record(name, 0x1014d1e8, 1);
}

// FUNCTION: HGSIM 0x100ae3a8
void *__cdecl FileRec_OpenAppend(const char *name)
{
    return open_record(name, 0x1014d1ec, 2);
}

// FUNCTION: HGSIM 0x100ae40c
void *__cdecl FileRec_OpenUpdate(const char *name)
{
    return open_record(name, 0x1014d1f0, 3);
}

// FUNCTION: HGSIM 0x100ae4bc
uint32_t __cdecl FileRec_WriteScrambled(void *rec, const uint8_t *src, uint32_t count)
{
    uint8_t buffer[0x200];
    uint32_t done = 0;
    uint32_t blocks = count >> 9;
    uint32_t rest = count % 0x200;

    if (U32(rec, 8) == 2 && Sim_100ae64d(rec, 0, 2) != 0)
        return 0;
    for (; blocks != 0; blocks--) {
        Mem_Move(buffer, src, 0x200);
        Sim_1003c7b0(buffer, 0x200, U32(rec, 4));
        if (W7P_CALL_100febc0(buffer, 1, 0x200, U32(rec, 0)) != 0x200)
            return done;
        U32(rec, 4) += 0x200;
        done += 0x200;
        src += 0x200;
    }
    if (rest != 0) {
        Mem_Move(buffer, src, rest);
        Sim_1003c7b0(buffer, (int)rest, U32(rec, 4));
        if (W7P_CALL_100febc0(buffer, 1, rest, U32(rec, 0)) != rest)
            return done;
        U32(rec, 4) += rest;
        done += rest;
    }
    return done;
}

// FUNCTION: HGSIM 0x100ae68f
int __cdecl FileRec_Close(void *rec)
{
    int result = W7P_CALL_100fd760(U32(rec, 0));

    if (result != 0)
        return result;
    Heap_FreeBlock((uint32_t)(uintptr_t)rec);
    return 0;
}
