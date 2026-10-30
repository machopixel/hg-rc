#include "w5b.h"
#include "w2g.h"
#include "../unsorted_1003/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10008123
void __cdecl Sim_10008123(void)
{
    char format[0x34];
    char line[0x34];

    SIM_SPRINTF(format, SIM_PTR(const char, 0x1012de0c), 0x32, 0x32);
    SIM_SPRINTF(line, format, SIM_PTR(const char, 0x1012dc50));
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(line, 0x32, 1, 0xf);
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(line, 0x32, 2, 0xf);
    G32(0x1015d048) = 0;
}

// FUNCTION: HGSIM 0x100082be
void __cdecl Sim_100082be(void)
{
    char format[0x10];
    char line[0x10];

    SIM_SPRINTF(format, SIM_PTR(const char, 0x1012de68), 0xc, 0xc);
    SIM_SPRINTF(line, format, SIM_PTR(const char, 0x1012dc50));
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(line, 0x40, 0, 0x78);
    G32(0x1015d050) = 0;
}

// FUNCTION: HGSIM 0x1000ae00
int __cdecl Sim_1000ae00(void *obj, uint32_t arg)
{
    int result;

    if (U32(obj, 0x1f4) != 0)
        result = SIM_CALL_1000b070(obj, arg);
    else
        result = SIM_CALL_1000b587(obj, arg);
    if (Sim_1000afe7((int)U16(obj, 0x1aa)) != 0 && result != 0) {
        u16a *counter = SIM_PTR(u16a, 0x101654f2);

        *counter = (uint16_t)(*counter + 1);
    }
    return result;
}

// FUNCTION: HGSIM 0x1000c7b4
int __cdecl Sim_1000c7b4(uint32_t value)
{
    switch (value) {
    case 0:
    case 1:
    case 9:
    case 0xc:
        return 1;
    default:
        return 0;
    }
}
