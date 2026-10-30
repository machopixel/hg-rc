#include "w5b.h"
#include "w3q.h"
#include "../unsorted_100a/w2f.h"
#include "../unsorted_100a/w2l.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10027df4
void __cdecl Sim_10027df4(void)
{
    int bucket;

    if (G32(0x1015dc30) == 0)
        return;
    for (bucket = 0; bucket < 0x3f1; bucket++) {
        uint32_t node = U32(G32(0x1015dc30), (uint32_t)bucket * 4u);

        while (node != 0) {
            uint32_t next = U32(node, 8);

            if (U16(node, 0) >= 0xf000)
                SIM_CALL_10027fb8(BYTES(node));
            node = next;
        }
    }
}

// FUNCTION: HGSIM 0x10028f38
void __cdecl Sim_10028f38(void *obj, int a, int b)
{
    uint32_t other;

    if (obj == 0)
        return;
    other = Sim_100aa7f1(obj);
    if (other != 0)
        Sim_10028f38(BYTES(other), a, b);
    other = Sim_100aa7fc(obj);
    if (other != 0)
        Sim_10028f38(BYTES(other), a, b);
    SIM_CALL_10028d9f(obj, a, b);
}

// FUNCTION: HGSIM 0x1002b600
void __cdecl Sim_1002b600(void)
{
    int i;

    for (i = 0; i < 2; i++)
        SIM_CALL_100a77a7(SIM_PTR(char, 0x10138254u + (uint32_t)i * 0x18u), G32(0x1014ceb4));
    for (i = 0; i < 0x2e; i++)
        SIM_CALL_100a77a7(SIM_PTR(char, 0x101455baU + (uint32_t)i * 0xd6u), G32(0x1014ceb4));
}

// FUNCTION: HGSIM 0x1002d349
int __cdecl Sim_1002d349(void *obj, float a, float b, float c)
{
    if (U32(obj, 0x98) != 0)
        return SIM_CALL_10031acd(BYTES(U32(obj, 0x98)), (int)U32(obj, 0x2c), a, b, c) & 1;
    return Sim_1002d276(obj, a, b, c);
}
