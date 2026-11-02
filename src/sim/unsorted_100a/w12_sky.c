#include "w12.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../unsorted_1005/w2f.h"
#include "../unsorted_1009/w5b.h"
#include "../unsorted_1009/w7o.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w8z.h"
#include "../unsorted_1010/w6k.h"
#include "../libm/hg_math.h"

#define W12_FOPEN SIM_FN(void *(__cdecl *)(const char *, const char *), 0x100fd850)
/* the DLL's C library sqrt (0x100fca04), on a double */
#define W12_SQRT hg_sqrt

/* Round 33, 1.2's coloured light (SKYGND.PAR "hambient", "l1", "l2", "gl"; 1.2 0x1008ceab): the ambient colour (1.2
   0x10138d20), light 1's colour (0x10138d38) and unit direction (0x10138d2c), light 2's (0x10138d58 / 0x10138d4c) and
   the global light's colour (0x10138d6c); colours 0..255 */
int32_t g_w33Ambient[3], g_w33Light1[3], g_w33Light2[3], g_w33Global[3];
float g_w33Light1Dir[3], g_w33Light2Dir[3], g_w33Light1Cam[3], g_w33Light2Cam[3];

/* 1.2's line format (0x1011fa6c): 1.0's and the four new keys */
static const char kW33SkyFormat[] =
    "mission=%4s globalintensity=%d hardwareintensity=%d hardwareambient=%d gintensity=%d gambient=%d "
    "skytexture=%s horizontexture=%s hardwarehorizon=%s horizontile=%f skycolor=%d,%d,%d,%d,%d,%d fogswitch=%d "
    "fogcolor=%d,%d,%d,%d skyspeed=%f density=%f,%f,%f hambient=%d,%d,%d l1=%d,%d,%d,%d,%d,%d "
    "l2=%d,%d,%d,%d,%d,%d gl=%d,%d,%d";

/* 1.2: a light's direction made a unit vector, kept as it was when its length is below 1e-7 */
static void W12_UnitDir(float *out, int32_t x, int32_t y, int32_t z)
{
    double s = W12_SQRT((double)(int32_t)((uint32_t)x * (uint32_t)x + (uint32_t)y * (uint32_t)y +
                                          (uint32_t)z * (uint32_t)z));
    float len = (float)s;

    if (!(hg_fabs(s) >= 1.0000000116860974e-07))  /* fcomp / test ah, 1: below or unordered skips */
        return;
    out[0] = (float)((double)x / (double)len);
    out[1] = (float)((double)y / (double)len);
    out[2] = (float)((double)z / (double)len);
}

static int32_t W12_Byte(int32_t c)
{
    if (c < 0)
        c = 0;
    if (c > 0xff)
        c = 0xff;
    return c;
}

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
    int32_t dir1[3] = {0, 0, 0}, dir2[3] = {0, 0, 0};
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
        /* 1.2: its own format, with the four new keys */
        Sim_100fd710(line, kW33SkyFormat, name, &intensity, &hwIntensity, &hwAmbient, &gIntensity,
                     &gAmbient, SIM_PTR(char, 0x1017d0b0), SIM_PTR(char, 0x1017d0d0), horizon, SIM_PTR(float, 0x1014c9f0),
                     &sky1, SIM_PTR(int32_t, 0x101cfc8c), &sky3, &sky4, &sky5, &sky6, SIM_PTR(int32_t, 0x1017d108),
                     SIM_PTR(int32_t, 0x1017d0f0), SIM_PTR(int32_t, 0x1017d0f4), SIM_PTR(int32_t, 0x1017d0f8),
                     SIM_PTR(int32_t, 0x1014c9ec), SIM_PTR(float, 0x1014c9f8), &density1, &density2, &density3,
                     &g_w33Ambient[0], &g_w33Ambient[1], &g_w33Ambient[2], &g_w33Light1[0], &g_w33Light1[1],
                     &g_w33Light1[2], &dir1[0], &dir1[1], &dir1[2], &g_w33Light2[0], &g_w33Light2[1], &g_w33Light2[2],
                     &dir2[0], &dir2[1], &dir2[2], &g_w33Global[0], &g_w33Global[1], &g_w33Global[2]);
        if (Sim_1010d240((const uint8_t *)key, (const uint8_t *)name, 4) == 0)
            break;
    }
    W12_UnitDir(g_w33Light1Dir, dir1[0], dir1[1], dir1[2]);
#ifndef W12S_MUTATE /* the checker's negative check only: light 2's direction left as it was */
    W12_UnitDir(g_w33Light2Dir, dir2[0], dir2[1], dir2[2]);
#endif
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
    {
        int i;

        for (i = 0; i < 3; i++) {
            g_w33Ambient[i] = W12_Byte(g_w33Ambient[i]);
            g_w33Light1[i] = W12_Byte(g_w33Light1[i]);
            g_w33Light2[i] = W12_Byte(g_w33Light2[i]);
            g_w33Global[i] = W12_Byte(g_w33Global[i]);
        }
    }
}