#include "w3o.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1008/w3o.h"
#include "../util/util.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x10017ad9
void __cdecl Sim_10017ad9(void *obj, float value)
{
    uint32_t state;

    if (obj == 0)
        return;
    state = U32(obj, 0x28);
    if (FL(state, 0x520) > value)
        FL(state, 0x520) = value;
    else if (!(FL(state, 0x520) >= value))
        FL(state, 0x520) = (float)((double)FL(state, 0x520) + GFL(0x10111108));
}

// FUNCTION: HGSIM 0x1001987a
int __cdecl Sim_1001987a(void *obj, uint32_t unused1, uint32_t unused2)
{
    if (obj == 0 || U32(obj, 0x50) == 0 || U32(obj, 0x28) == 0)
        return 0;
    if (U32(obj, 4) == G32(0x1017b8c8))
        *SIM_PTR(u16a, 0x101653f3) = (uint16_t)(*SIM_PTR(u16a, 0x101653f3) + 1);
    if (U32(PTRTAB(U32(obj, 4)), 8) == U32(PTRTAB(G32(0x1017b8c8)), 8))
        *SIM_PTR(u16a, 0x1016540c) = (uint16_t)(*SIM_PTR(u16a, 0x1016540c) + 1);
    Sim_10019920(obj);
    return 2;
}

// FUNCTION: HGSIM 0x10019920
int __cdecl Sim_10019920(void *obj)
{
    int owner = Sim_10052be7((int)U32(obj, 4));
    int i;

    for (i = 0; i < 0x200; i++) {
        uint8_t *rec = GBYTES(0x1096fc80u + (uint32_t)i * 0x156);
        uint32_t px, py, pz;
        float dx, dy, dz, distF;
        double dist;

        if (U32(rec, 0x48) == 0 || U32(rec, 0x3c) == 0)
            continue;
        if (U32(rec, 0) != 3 && U32(rec, 0) != 0xa && U32(rec, 0) != 0xc && U32(rec, 0) != 9)
            continue;
        if (owner == Sim_10052be7((int)U32(rec, 0x50)))
            continue;
        Sim_100a9cfb((void *)(uintptr_t)U32(rec, 0x3c), &px, &py, &pz);
        dx = (float)((double)FL(obj, 0x58) - Float_OfBits(px));
        dy = (float)((double)FL(obj, 0x5c) - Float_OfBits(py));
        dz = (float)((double)FL(obj, 0x60) - Float_OfBits(pz));
        dist = Sim_10084a94(dx, dy, dz);
        distF = (float)dist;
        if (dist > GFL(0x1011118c))
            continue;
        if (U32(rec, 0x64) & 0x80000000u) {
            double reduced = (double)distF - GFL(0x10111194);
            float reducedF = (float)reduced;

            if (reduced > GFL(0x10111198)) {
                double dot = ((double)FL(rec, 0x18) * dx + (double)FL(rec, 0x1c) * dy) + (double)FL(rec, 0x20) * dz;

                if (dot > GFL(0x10111198)) {
                    if (Sim_1009c929(0x64) > 0x4b) {
                        U32(rec, 0x64) = 0;
                    } else if (!(reducedF >= GFL(0x1011119c))) {
                        U32(rec, 0x64) = 0x451c4000;
                    } else {
                        int n = Sim_1009c929(Render_Ftol((double)reducedF / GFL(0x1011119c)));

                        FL(rec, 0x64) = (float)((double)n * GFL(0x1011119c) + GFL(0x10111194));
                    }
                }
            } else {
                U32(rec, 0x64) = 0;
            }
        }
        if (FL(rec, 0x64) > GFL(0x10111198) && !(distF > FL(rec, 0x64)))
            U32(rec, 0x48) |= 0x8000;
    }
    return 1;
}
