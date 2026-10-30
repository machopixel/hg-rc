#include "w12.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../unsorted_1005/w2f.h"
#include "../unsorted_1009/w5b.h"
#include "../unsorted_1009/w7o.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_1010/w6k.h"

#define W12_FOPEN SIM_FN(void *(__cdecl *)(const char *, const char *), 0x100fd850)

typedef union {
    float f;
    uint32_t u;
} W12SkyBits;

// FUNCTION: HGSIM 0x100a0063
void __cdecl Sim_100a0063(const char *mission)
{
    char key[32];
    char line[0x200];
    /* the sscanf targets on the stack; start values (DECISIONS.md): 0 / empty unless the original sets them */
    char name[32] = {0}, horizon[32] = {0};
    int32_t intensity = 0x80, hwIntensity = 0x80, hwAmbient = 0, gIntensity = 0, gAmbient = 0;
    int32_t sky1 = 0, sky3 = 0, sky4 = 0, sky5 = 0, sky6 = 0;
    uint32_t density1 = 0, density2 = 0, density3 = 0;
    void *fp;
    W12SkyBits v;

    G32(0x1017d108) = 0;
    Str_CopyUpper(key, mission);
    fp = W12_FOPEN(SIM_PTR(const char, 0x1014cb58), SIM_PTR(const char, 0x1014cb54));
    if (fp == 0)
        return;
    for (;;) {
        char *hash;

        if (Sim_1009ff33(fp, line, 0x200) == 0)
            return;
        hash = Crt_StrChr(line, 0x23);
        if (hash != 0)
            *hash = 0;
        if (line[0] == 0)
            continue;
        Sim_100fd710(line, (const char *)GBYTES(0x1014cb64), name, &intensity, &hwIntensity, &hwAmbient, &gIntensity,
                     &gAmbient, SIM_PTR(char, 0x1017d0b0), SIM_PTR(char, 0x1017d0d0), horizon, SIM_PTR(float, 0x1014c9f0),
                     &sky1, SIM_PTR(int32_t, 0x101cfc8c), &sky3, &sky4, &sky5, &sky6, SIM_PTR(int32_t, 0x1017d108),
                     SIM_PTR(int32_t, 0x1017d0f0), SIM_PTR(int32_t, 0x1017d0f4), SIM_PTR(int32_t, 0x1017d0f8),
                     SIM_PTR(int32_t, 0x1014c9ec), SIM_PTR(float, 0x1014c9f8), &density1, &density2, &density3);
        if (Sim_1010d240((const uint8_t *)key, (const uint8_t *)name, 4) == 0)
            break;
    }
    if (G32(0x1017b940) != 0 || G32(0x1017b944) != 0) {
        intensity = hwIntensity;
        Str_Copy(SIM_PTR(char, 0x1017d0d0), horizon);
        U16(G32(0x1014cae0), 0x2a) = (uint16_t)hwAmbient;
        G32(0x10138c74) = (uint32_t)sky5;
        G32(0x10138c78) = (uint32_t)sky6;
    } else {
        G32(0x10138c74) = (uint32_t)sky1;
        G32(0x10138c78) = (uint32_t)sky3;
        if ((int32_t)G32(0x1017d108) < 0)
            G32(0x1017d108) = 0;
    }
    if (G32(0x1014cb38) != 0) {
        G32(0x10138c74) = 0xc0;
        G32(0x10138c78) = 0xdf;
        intensity = gIntensity;
        U16(G32(0x1014cae0), 0x2a) = (uint16_t)gAmbient;
    }
    if (sky4 > 0x100)
        sky4 = 0x100;
    v.f = (float)sky4;
    G32(0x1014c9c0) = v.u;
    G32(0x1014c9f4) = G32(0x1014c9c0);
    v.f = (float)((double)*SIM_PTR(float, 0x1014c9c8) / (double)*SIM_PTR(float, 0x1014c9c0));
    G32(0x1014c9e8) = v.u;
    Sim_10058549(intensity);
    G32(0x10143fc4) = density1;
    G32(0x10143fc8) = density3;
    G32(0x101668cc) = density2;
}