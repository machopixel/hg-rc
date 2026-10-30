#include "w3q.h"
#include "fns.h"
#include "../render/w2n.h"
#include "../util/w3q.h"

typedef int(__cdecl *PixelFn)(uint32_t, uint32_t);

#define REC(i) (0x10124eb8 + (uint32_t)(i) * 0x98)

// FUNCTION: HGSIM 0x1004c5c1
int __cdecl Sim_1004c5c1(int a, unsigned word)
{
    int n0 = Sim_1004c565(word);
    int n1 = Sim_1004c573((int)word);
    int n2 = Sim_1004c584((int)word);
    int n3 = Sim_1004c595((int)word);
    int n4 = Sim_1004c5a6((int)word);
    uint32_t sum = G32(0x101116b2 + (uint32_t)a * 0xaa);
    float f;

    sum += G32(REC(n0));
    sum += G32(REC(n1));
    sum += G32(REC(n2));
    sum += G32(REC(n3));
    sum += G32(REC(n4));
    f = (float)(int32_t)sum;
    f = (float)((double)f * (double)GF32(REC(n0) + 4));
    f = (float)((double)f * (double)GF32(REC(n1) + 4));
    f = (float)((double)f * (double)GF32(REC(n2) + 4));
    f = (float)((double)f * (double)GF32(REC(n3) + 4));
    return Render_Ftol((double)f * (double)GF32(REC(n4) + 4));
}

// FUNCTION: HGSIM 0x1004e6a2
int __cdecl Sim_1004e6a2(void)
{
    int result;

    if (G32(0x1015f224) >= G32(0x1015f330))
        return -1;
    result = (*SIM_PTR(PixelFn, 0x1015f334))(G32(0x1015f338) + G32(0x1015f228),
                                                                        G32(0x1015f220) + G32(0x1015f224));
    G32(0x1015f228) = G32(0x1015f228) + 1;
    if (G32(0x1015f228) >= G32(0x1015f33c)) {
        G32(0x1015f228) = 0;
        G32(0x1015f224) = G32(0x1015f224) + 1;
    }
    return result;
}
