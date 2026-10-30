#include "w3p.h"
#include "../util/access.h"
#include "../util/w3p.h"

// FUNCTION: HGSIM 0x100b3fd5
void __cdecl Sim_100b3fd5(int a, int b, int c, int d, int e, int f, int g, int h, int i)
{
    GF32(0x10150800) = (float)a;
    GF32(0x10150804) = (float)b;
    GF32(0x10150808) = (float)c;
    GF32(0x1015080c) = (float)((double)d * (double)GF32(0x1012babc));
    GF32(0x10150810) = (float)((double)e * (double)GF32(0x1012babc));
    GF32(0x10150814) = (float)((double)f * (double)GF32(0x1012babc));
    GF32(0x10150818) = (float)g;
    GF32(0x1015081c) = (float)h;
    GF32(0x10150820) = (float)i;
}

// FUNCTION: HGSIM 0x100bdc5d
int __cdecl Sim_100bdc5d(SortedList100b *list, int32_t value, int(__cdecl *compare)(int32_t item, int32_t value))
{
    int32_t low;
    int32_t high;
    int32_t mid;
    int32_t result;
    int32_t k;

    if (list->count >= list->capacity)
        return 0;
    if (list->count == 0) {
        list->items[0] = value;
        list->count++;
        return 1;
    }
    low = 0;
    high = list->count - 1;
    result = 0;
    mid = 0;
    while (high >= low) {
        mid = (high + low) >> 1;
        result = compare(list->items[mid], value);
        if (result == 0)
            break;
        if (result < 0)
            low = mid + 1;
        else
            high = mid - 1;
    }
    if (result < 0)
        mid++;
    for (k = list->count; k > mid; k--)
        list->items[k] = list->items[k - 1];
    list->items[mid] = value;
    list->count++;
    return 1;
}
