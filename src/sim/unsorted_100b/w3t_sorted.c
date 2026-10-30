#include "w3t.h"

// FUNCTION: HGSIM 0x100bdb28
int __cdecl Sim_100bdb28(SortedArray *array, uint32_t value, double(__cdecl *compare)(uint32_t item, uint32_t value))
{
    int32_t low, high, mid = 0, i;
    float last = 0.0f;

    if (array->count >= array->capacity)
        return 0;
    if (array->count == 0) {
        array->items[0] = value;
        array->count++;
        return 1;
    }
    low = 0;
    high = array->count - 1;
    while (high >= low) {
        double r;

        mid = (high + low) >> 1;
        r = compare(array->items[mid], value);
        last = (float)r;
        if (r == 0.0 || r != r)
            break;
        if (last < 0.0f)
            low = mid + 1;
        else
            high = mid - 1;
    }
    if (!(last >= 0.0f))
        mid++;
    for (i = array->count; i > mid; i--)
        array->items[i] = array->items[i - 1];
    array->items[mid] = value;
    array->count++;
    return 1;
}

// FUNCTION: HGSIM 0x100bdd77
uint32_t __cdecl Sim_100bdd77(SortedArray *array, uint32_t value, int(__cdecl *compare)(uint32_t item, uint32_t value))
{
    int32_t low, high, mid = 0, i;

    if (array->count >= array->capacity)
        return 0;
    if (array->count == 0) {
        array->items[0] = value;
        array->count++;
        return value;
    }
    low = 0;
    high = array->count - 1;
    while (high >= low) {
        int r;

        mid = (high + low) >> 1;
        r = compare(array->items[mid], value);
        if (r == 0)
            return array->items[mid];
        if (r < 0)
            low = mid + 1;
        else
            high = mid - 1;
    }
    for (i = array->count; i > mid; i--)
        array->items[i] = array->items[i - 1];
    array->items[mid] = value;
    array->count++;
    return value;
}
