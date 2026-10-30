#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_1002/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10033c24
int __cdecl Sim_10033c24(void)
{
    Mem_Fill(GBYTES(0x109E4CC0), 0, 0x3524);
    return 1;
}

// FUNCTION: HGSIM 0x100348bd
char *__cdecl Sim_100348bd(const char *s)
{
    return Str_CopyN((char *)GBYTES(0x10139E58), s, 0x100);
}

// FUNCTION: HGSIM 0x10036334
void __cdecl Path_StripDirectory(char *path)
{
    char tail[0x100];
    int len = (int)Str_Length(path);
    char *p = path + len;

    while (len != 0) {
        if (*p == '\\') {
            p++;
            Str_Copy(tail, p);
            Str_Copy(path, tail);
            return;
        }
        p--;
        len--;
    }
}

// FUNCTION: HGSIM 0x10038ad5
void __cdecl Sim_10038ad5(void *obj, void *rec)
{
    int n = 0;
    int i;
    int found = 0;
    uint8_t *slot = 0;

    for (i = 0; i < 8; i++) {
        slot = BYTES(U32(obj, 0x84)) + (uint32_t)i * 0x128u;
        if (S32(slot, 4) == -1) {
            found = 1;
            break;
        }
    }
    if (!found)
        return;

    U32(rec, 0x128) = U32(obj, 0x5fb);
    Mem_Move(slot, BYTES(rec) + 0xc, 0x128);
    U32(slot, 0x104) = (uint32_t)i;
    if ((U32(slot, 0x38) == U32(slot, 0x108) + 1 && S16(rec, U32(slot, 0x108) * 0x24u + 0x136) == 0)
        || S32(slot, 8) == -1 || S32(slot, 0x20) == -2)
        U32(slot, 8) = 0xFFFFFFFFu;
    else
        U32(slot, 8) = 1;

    if (U32(U32(obj, 0), 4) == G32(0x1017b8c8))
        Sim_10028981(BYTES(G32(0x109e88ec + (uint32_t)i * 4)),
                     (const char *)(GBYTES(0x10145592) + U32(slot, 4) * 0xd6u));

    for (i = 0; i < 0x32; i++) {
        uint8_t *entry = BYTES(U32(obj, 0x8c)) + (uint32_t)i * 0x24u;

        if (S16(entry, 2) == 0) {
            Mem_Move(entry, BYTES(rec) + (uint32_t)n * 0x24u + 0x134, 0x24);
            U32(slot, 0x3c + (uint32_t)n * 4) = (uint32_t)i;
            n++;
        }
        if (!(i < S32(slot, 0x38)))
            break;
    }
    U32(slot, 0x110) = U32(obj, 0x8c) + U32(slot, 0x3c) * 0x24u;
    U32(obj, U32(rec, 0x128) * 4u + 0x498) = U32(rec, 4);
    U32(obj, 0x5ff) = U32(rec, 0x68c);
}

// FUNCTION: HGSIM 0x10039725
void __cdecl Sim_10039725(void)
{
    Mem_Fill(GBYTES(0x109e36e0), 0, 0x40);
    Mem_Fill(GBYTES(0x109e3720), 0, 0x40);
    if (U32(G32(0x1015d0a4), 0xc) != 0) {
        G32(0x109e36e0) = 0x40;
        G32(0x109e3714) = 0x3c23d70a;
        G32(0x109e3718) = 0x3f800000;
        G32(0x109e371c) = 0x3f800000;
        G32(0x109e3720) = 0x40;
        G32(0x109e373c) = 0x168;
        G32(0x109e3740) = 0x168;
        G32(0x109e3744) = 0;
        G32(0x109e3748) = 0;
        G32(0x109e374c) = 0x3f800000;
        G32(0x109e3750) = 0;
        G32(0x109e3754) = 0x451c4000;
        G32(0x109e3758) = 0x46ea6000;
        G32(0x109e375c) = 0;
    }
}
