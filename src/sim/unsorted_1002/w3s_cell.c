#include "w3s.h"
#include "w3q.h"
#include "w5c.h"
#include "w5x.h"
#include "../render/w3s_fp.h"
#include "../unsorted_1008/w3s.h"

/* Per-type tests of an object: f(object, x, y, z, &result), non-zero when they produced a result. */
typedef int(__cdecl *ObjectTest)(void *, float, float, float, float *);
/* the original calls them through the DLL's jump table (0x100035df, 0x10001e1f, 0x10002775) */
#define TEST_TYPE_5 Sim_1002d3ca
#define TEST_TYPE_8 ((ObjectTest)Sim_1002d479)
#define TEST_TYPE_0 Sim_1002cde1

// FUNCTION: HGSIM 0x1002db2d
float __cdecl Sim_1002db2d(float x, float y, float z)
{
    uint32_t cell;
    uint32_t object;
    float best;
    float result;

    if (G32(0x1017c8c4) != 0)
        cell = (uint32_t)Sim_10088775(x, z);
    else
        cell = G32(0x1017c8cc);
    if (cell == 0)
        return GF32(0x101112c4);

    best = -3.4e38f; /* bits 0xff7fc99e, the value of the float at 0x101112cc */
    for (object = U32(cell, 0x10); object != 0; object = U32(object, 0x10)) {
        ObjectTest test;

        switch (S32(object, 0x34)) {
        case 5:
            test = TEST_TYPE_5;
            break;
        case 8:
            test = TEST_TYPE_8;
            break;
        case 0:
            test = TEST_TYPE_0;
            break;
        default:
            continue;
        }
        if (test(BYTES(object), x, y, z, &result) != 0 && result > best) {
            best = result;
            G32(0x1015dd4c) = G32(0x1015dd40);
            G32(0x1015dd50) = G32(0x1015dd44);
            G32(0x1015dd54) = G32(0x1015dd48);
        }
    }
    if (best > GF32(0x101112cc))
        return best;
    return 0.0f;
}
