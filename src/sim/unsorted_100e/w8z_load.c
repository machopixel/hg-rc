#include "w8z.h"
#include "w3p.h"
#include "../unsorted_1002/w5x.h"
#include "../unsorted_1005/w2h.h"
#include "../unsorted_100b/w2j.h"
#include "../unsorted_100b/w4w.h"
#include "../unsorted_100d/fns.h"
#include "../unsorted_100d/w4v.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100e122f
int __cdecl Sim_100e122f(const char *s)
{
    int result = 0;
    uint8_t record[0x10];
    uint8_t out[0x20];
    uint32_t found;
    int digit;
    int i;

    Sim_10052530();
    for (i = 0; i < 0x96; i++)
        SIM_PTR(u32a, 0x1017eec0)[i] = 0xffffffffu;
    G32(0x1017e6a0) = 0;
    G32(0x1017e6a4) = 0;
    Sim_100e2756(0, 0, 0);
    if (GS32(0x1015965c) > 1)
        digit = Crt_IsCType((int)(signed char)s[0], 4);
    else
        digit = U16(G32(0x10159450) + (uint32_t)((int32_t)(signed char)s[0] * 2), 0) & 4;
    if (digit != 0)
        U16(record, 0) = (uint16_t)Crt_AToI(s);
    else
        U16(record, 0) = 0xffff;
    Str_CopyN((char *)record + 2, s, 0xc);
    record[0xe] = 0;
    Sim_100db77e(0);
    found = W8Z_CALL_100278c0(record, out);
    if (found != 0 && Sim_100b35b0() != 0) {
        result = W8Z_CALL_100dcc12(found);
        Sim_10027a8f((void *)(uintptr_t)found);
        Sim_100db351();
        Sim_100b372b();
        if (Sim_100b487e() == 0)
            W8Z_CALL_100bc280(0x4b, 0);
    }
    if (result == 0) {
        W8Z_CALL_100bc280(0xb, SIM_PTR(char, 0x10152614), (char *)record + 2);
        return 0;
    }
    return 1;
}
