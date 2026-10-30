#include "w6i.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../unsorted_100f/w6i.h"
#include <string.h>

typedef int(__stdcall *W6iHeapValidateFn)(uint32_t heap, uint32_t flags, const void *block);
typedef uint32_t(__stdcall *W6iGetLastErrorFn)(void);

// FUNCTION: HGSIM 0x101000d0
int __cdecl Heap_Check(void)
{
    int result = -2;
    uint32_t error;

    if (W6I_FN_10106E50() < 0)
        result = -4;
    if (SIM_API(W6iHeapValidateFn, 0x10ac56a4)(G32(0x10ac3714), 0, 0) != 0)
        return result;
    error = SIM_API(W6iGetLastErrorFn, 0x10ac5600)();
    if (error != 0x78)
        return -4;
    G32(0x1017e88c) = error;
    G32(0x1017e888) = 0x28;
    return result;
}

// FUNCTION: HGSIM 0x10100620
void *__cdecl Search_Binary(const void *key, uint8_t *base, uint32_t num, uint32_t width,
                            int(__cdecl *compare)(const void *, const void *))
{
    uint32_t lo = (uint32_t)(uintptr_t)base;
    uint32_t hi = lo + (num - 1) * width;
    uint32_t n = num;

    while (lo <= hi) {
        uint32_t half = n >> 1;
        uint32_t odd;
        uint32_t mid;
        int r;

        if (half == 0) {
            if (n == 0)
                return 0;
            return compare(key, BYTES(lo)) == 0 ? BYTES(lo) : 0;
        }
        odd = n & 1;
        mid = lo + width * (odd != 0 ? half : half - 1);
        r = compare(key, BYTES(mid));
        if (r == 0)
            return BYTES(mid);
        if (r < 0) {
            hi = mid - width;
            n = odd != 0 ? half : half - 1;
        } else {
            lo = mid + width;
            n = half;
        }
    }
    return 0;
}

// FUNCTION: HGSIM 0x10100730
int32_t __cdecl File_GetLength(uint32_t fd)
{
    if (fd < G32(0x10ac385c)) {
        uint32_t table = SIM_PTR(u32a, 0x10ac3720)[(int32_t)fd >> 5];

        if ((U8(table, (fd & 0x1f) * 8 + 4) & 1) != 0) {
            int32_t here = W6I_FN_10105FE0(fd, 0, 1);
            int32_t end;

            if (here == -1)
                return -1;
            end = W6I_FN_10105FE0(fd, 0, 2);
            if (here != end)
                W6I_FN_10105FE0(fd, here, 0);
            return end;
        }
    }
    G32(0x1017e888) = 9;
    G32(0x1017e88c) = 0;
    return -1;
}

// FUNCTION: HGSIM 0x101009f0
uint32_t __cdecl Sim_101009f0(void)
{
    return W6I_FN_101086C0(0x10000, 0x30000);
}

// FUNCTION: HGSIM 0x10100ed0
uint32_t __cdecl Sim_10100ed0(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    uint32_t result;

    *GBYTES(0x1017e908) = 1;
    result = W6I_FN_10100BE0(a, b, c, d);
    *GBYTES(0x1017e908) = 0;
    return result;
}

// FUNCTION: HGSIM 0x10100fa0
void __cdecl Str_MakeRoom(char *str, uint32_t count)
{
    if (count != 0)
        Mem_Copy(str + count, str, (uint32_t)strlen(str) + 1);
}

// FUNCTION: HGSIM 0x10101960
void __cdecl Stream_PutCharCounted(int c, uint8_t *f, int *count)
{
    uint32_t room = U32(f, 4) - 1;
    int result;

    U32(f, 4) = room;
    if ((int32_t)room >= 0) {
        *BYTES(U32(f, 0)) = (uint8_t)c;
        result = c & 0xff;
        U32(f, 0) = U32(f, 0) + 1;
    } else {
        result = W6I_FN_100FF030(c, f);
    }
    if (result == -1)
        *count = -1;
    else
        *count = (int)((uint32_t)*count + 1);
}
