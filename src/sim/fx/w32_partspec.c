#include "w32.h"
#include "../render/w3q.h"
#include "../render/w31.h"
#include "../render/w33.h"
#include "../unsorted_1000/w2f.h"
#include "../util/access.h"
#include "../../platform/plat.h"

/* 1.2's table as its image has it: no record, no textures, white, intensity 0xff, size 8 (accel / laser 1) */
#define W32_T(name, size) {0, 0, 0xff, 0xff, 0xff, 0xff, 0xff, (size), (name), 0, 0, 0}
W32Type g_w32_1011e700[W32_TYPES] = {
    W32_T("railgun", 8),   W32_T("haywire", 8),     W32_T("accel", 1),        W32_T("laser", 1),
    W32_T("shockwave", 8), W32_T("skate", 8),       W32_T("gun", 8),          W32_T("smoke", 8),
    W32_T("grenade", 8),   W32_T("mortar", 8),      W32_T("missile", 8),      W32_T("chunket", 8),
    W32_T("tracer", 8),    W32_T("spark", 8),       W32_T("missilespark", 8), W32_T("laserspark", 8),
    W32_T("pweaponspark", 8), W32_T("projpuff", 8), W32_T("laserpuff", 8),    W32_T("pweaponpuff", 8),
    W32_T("missilepuff", 8),  W32_T("footstep", 8),
};
char g_w32_1014b5a0[4];
int32_t g_w32_1014b58c;

/* "Texture" and the number in two digits (1.2: sprintf "Texture%02d") */
static void W32_TextureKey(char *out, int32_t n)
{
    static const char kKey[] = "Texture";
    int i;

    for (i = 0; kKey[i] != 0; i++)
        out[i] = kKey[i];
    out[i++] = (char)('0' + n / 10 % 10);
    out[i++] = (char)('0' + n % 10);
    out[i] = 0;
}

// FUNCTION: HGSIM12 0x10078e34
int32_t __cdecl Sim12_10078e34(uint8_t *type, char *names, int32_t max)
{
    W32Type *t = (W32Type *)type;
    char path[0x100], text[0x100], key[16];
    uint32_t n;
    int32_t i, k;

    path[0] = 0;
    n = Plat_CurrentDir(path, 0x100);
    (void)n;
    for (k = 0; path[k] != 0; k++)
        ;
    path[k++] = '\\';
    {
        static const char kFile[] = "PartSpec.ini";
        for (i = 0; kFile[i] != 0; i++)
            path[k++] = kFile[i];
        path[k] = 0;
    }
    if (max > 0x20)
        max = 0x20;
    t->red = (uint8_t)Plat_IniInt(path, t->section, "Red", t->red);
    t->green = (uint8_t)Plat_IniInt(path, t->section, "Green", t->green);
    t->blue = (uint8_t)Plat_IniInt(path, t->section, "Blue", t->blue);
    t->intensity = (uint8_t)Plat_IniInt(path, t->section, "Intensity", t->intensity);
    t->size = (uint8_t)Plat_IniInt(path, t->section, "Size", t->size);
    t->lock = (int32_t)Plat_IniInt(path, t->section, "Lock", (uint32_t)t->lock);
    t->order = (int32_t)Plat_IniInt(path, t->section, "Order", (uint32_t)t->order);
    t->loop = Plat_IniInt(path, t->section, "Loop", t->loop);
    for (i = 0; i < max; i++) {
        W32_TextureKey(key, i);
        if (Plat_IniString(path, t->section, key, g_w32_1014b5a0, text, 0x100) != 0 && text[0] != 0) {
            /* strncpy of 8 (the rest of the 8 padded with zeros), then the end */
            for (k = 0; k < 8 && text[k] != 0; k++)
                names[i * 9 + k] = text[k];
            for (; k < 8; k++)
                names[i * 9 + k] = 0;
            names[i * 9 + 8] = 0;
        }
    }
    for (i = 0; i < max; i++)
        if (names[i * 9] == 0)
            break;
#ifdef W32C_MUTATE /* the checker's negative check only: one texture short */
    if (i > 0)
        i--;
#endif
    return i;
}

static void W32_ResetType(W32Type *t)
{
    t->group = -1;
    t->count = 0;
    t->red = 0xf;
    t->green = 0xf;
    t->blue = 0xf;
    t->intensity = 0xf;
    t->size = 1;
    t->lock = 0;
    t->order = 0;
    t->loop = 0;
}

// FUNCTION: HGSIM12 0x100790be
void __cdecl Sim12_100790be(void)
{
    char names[0x20 * 9];
    int32_t slot, rec, n, k;

    /* 1.2: the types are reset only without the 3D mode and its 3DFX / PowerVR mode */
#ifdef W32PV_MUTATE /* the checker's negative check only: the 3DFX mode not tested */
    if (!W31_TexMode()) {
#else
    if (!W31_TexMode() && g_w33Voodoo == 0) {
#endif
        for (slot = 0; slot < W32_TYPES; slot++)
            W32_ResetType(&g_w32_1011e700[slot]);
        return;
    }
    slot = 0;
    for (rec = 0x100; rec < 0x200 && slot < W32_TYPES; rec++) {
        if (S16(SIM_PTR(uint8_t, 0x10ac1b00u + (uint32_t)rec * 14), 8) != -2)
            continue;
        while (slot < W32_TYPES && g_w32_1011e700[slot].group >= 1)
            slot++;
        if (slot >= W32_TYPES)
            continue;
        for (k = 0; k < (int32_t)sizeof(names); k++)
            names[k] = 0;
        g_w32_1011e700[slot].group = rec;
        n = g_w32_1011e700[slot].section == 0 ? 0 : Sim12_10078e34(W32_TYPE(slot), names, 0x20);
        if (n != 0) {
            for (k = 0; k < n; k++)
                if (Sim_10006ba7(names + k * 9, -1, 0) != 0)
                    break;
            if (k != 0) {
                Sim_10006db8(g_w32_1011e700[slot].group, -1);
                g_w32_1011e700[slot].count = (uint32_t)k;
            } else {
                n = 0;
            }
        }
        if (n == 0)
            W32_ResetType(&g_w32_1011e700[slot]);
        slot++;
    }
}
