#include "w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1001083a
void __cdecl Sim_1001083a(int index)
{
    uint32_t record;

    if (index != 0) {
        record = G32(0x1013BB40) * 0x4FC3Au + (uint32_t)index * 0x13Fu + 0x1047383Au;
        U32(GBYTES(record), 2) = 0x10000;
    }
}

// FUNCTION: HGSIM 0x10013fe1
unsigned __cdecl Sim_10013fe1(unsigned sel, unsigned b, unsigned c, unsigned d)
{
    unsigned short result = (unsigned short)sel;

    if ((sel & 0xFFFF) == 0x40)
        result = (unsigned short)b;
    else if ((sel & 0xFFFF) == 0x25)
        result = (unsigned short)c;
    else if ((sel & 0xFFFF) == 0x2a)
        result = (unsigned short)d;
    return result;
}

// FUNCTION: HGSIM 0x100155b7
int __cdecl Sim_100155b7(const void *obj, int key)
{
    int i, j;

    for (i = 0; i < 3; i++) {
        uint32_t list = U32(obj, 0x174 + i * 4);

        if (list == 0)
            continue;
        for (j = 0; j < S16(list, 0); j++) {
            if (S16(U32(list, 2), j * 4) == (int16_t)key)
                return 1;
        }
    }
    return 0;
}

// FUNCTION: HGSIM 0x10015cd2
int __cdecl Sim_10015cd2(unsigned index)
{
    uint32_t record = PTRTAB(index);

    if (record == 0)
        return -1;
    return S32(record, 0x10);
}

// FUNCTION: HGSIM 0x1001c102
void __cdecl Sim_1001c102(void)
{
}
