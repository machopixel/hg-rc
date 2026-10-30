#include "w2g.h"
#include "../simmem.h"

#define U16(p, off) (*(uint16_t *)((uint8_t *)(p) + (off)))
#define U32(p, off) (*(uint32_t *)((uint8_t *)(p) + (off)))
#define PTR(v) ((uint8_t *)(uintptr_t)(v))

// FUNCTION: HGSIM 0x100071da
void __cdecl Sim_100071da(int index, int value)
{
    uint32_t addr = 0x10AC1B04u + (uint32_t)((int32_t)(int16_t)index * 14);

    *SIM_PTR(uint16_t, addr) = (uint16_t)value;
}

// FUNCTION: HGSIM 0x10008ef8
uint32_t __cdecl Sim_10008ef8(const void *obj)
{
    const uint8_t *rec = PTR(U32(obj, 0x30));

    if (rec == 0)
        return 0;
    return U16(rec, 0x14);
}

// FUNCTION: HGSIM 0x1000941e
void __cdecl Sim_1000941e(void *obj, int value)
{
    U16(obj, 0x26) = (uint16_t)value;
}

// FUNCTION: HGSIM 0x10009613
void __cdecl Sim_10009613(const void *obj, int index, uint32_t *id, uint32_t *count, uint32_t *out, int max)
{
    const uint8_t *block = PTR(U32(obj, 0x30));
    const uint8_t *rec;
    int n;

    if (block == 0)
        return;
    rec = block + U32(block, 8) + (uint32_t)index * 0x28u;
    if (id != 0)
        *id = U16(rec, 0);
    n = U16(rec, 2);
    if (count != 0)
        *count = (uint32_t)n;
    if (out != 0) {
        uint32_t left;

        if (n > max)
            n = max;
        left = (uint32_t)n;
        while (left-- != 0)
            out[left] = rec[U32(rec, 4) + left];
    }
}

// FUNCTION: HGSIM 0x1000971b
void __cdecl Sim_1000971b(void *word, uint32_t mask)
{
    U16(word, 0) = (uint16_t)((mask & 0x10F) | (U16(word, 0) & 0xFEF0u) | 0x8000u);
}

// FUNCTION: HGSIM 0x1000afe7
int __cdecl Sim_1000afe7(int value)
{
    if (((int32_t)(int16_t)value & 0x200) != 0 && ((int32_t)(int16_t)value & 0xFF) == *SIM_PTR(int32_t, 0x1017B8C8))
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x1000df50
void __cdecl Sim_1000df50(void)
{
    int i;

    *SIM_PTR(uint32_t, 0x10A38DE0) = 0;
    for (i = 0; i < 0x100; i++) {
        *SIM_PTR(uint32_t, 0x10A39200u + (uint32_t)i * 0x14u) = 0;
        *SIM_PTR(uint8_t, 0x10A39204u + (uint32_t)i * 0x14u) = 0;
    }
}

// FUNCTION: HGSIM 0x1000e510
uint8_t *__cdecl Sim_1000e510(int index)
{
    return SIM_PTR(uint8_t, 0x10A39204u + (uint32_t)index * 0x14u);
}
