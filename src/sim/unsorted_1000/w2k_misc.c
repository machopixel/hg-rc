#include "w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10008e8b
void __cdecl Sim_10008e8b(void *obj, uint32_t value)
{
    uint32_t target = U32(obj, 0x30);

    if (target != 0)
        U32(target, 0) = value;
}

// FUNCTION: HGSIM 0x100093ea
uint32_t __cdecl Sim_100093ea(const void *obj)
{
    if (obj == NULL)
        return 0;
    return U16(obj, 0) & 0x7ef0;
}

// FUNCTION: HGSIM 0x1000c627
int __cdecl Sim_1000c627(const void *obj)
{
    uint32_t flags;

    if (obj == NULL)
        return -1;
    flags = U32(obj, 0xf0);
    if ((flags & 0x200) != 0 && U32(PTRTAB(flags & 0xff), 0) == 9)
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x1000e060
void __cdecl Sim_1000e060(const uint8_t **cursor, int *count)
{
    const uint8_t *p = *cursor;
    uint32_t value = 0;
    int i;

    for (i = 0; i < *count; i++) {
        if (U32(p, 0) == G32(0x1012DF58)) {
            p += 4;
            value = U32(p, 0);
            p += 4;
            break;
        }
        p++;
    }
    *cursor = p;
    *count = (int)value;
}
