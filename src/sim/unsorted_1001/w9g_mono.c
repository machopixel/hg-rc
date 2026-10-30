#include "w9g.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1001/w2e.h"
#include "../unsorted_1003/w2e.h"
#include "../unsorted_100f/w8z.h"

#define STR(a) ((const char *)GBYTES(a))

// FUNCTION: HGSIM 0x10012af0
void __cdecl Mono_ShowMissions(int star)
{
    char line[0x100];
    char head[0x110];
    uint8_t *p = GBYTES(0x10473800) + (uint32_t)star * 0x4fc3au;
    int attr = 7;
    int32_t a, b, i;

    switch (p[0x38]) {
    case 0:
        if (S32(p, 0xc) > 0)
            Crt_SPrintf(line, STR(0x1013117c), star, (int)(U32(p, 0xc) - (G32(0x1037305c) - U32(p, 4))));
        else
            Crt_SPrintf(line, STR(0x1013119c), star);
        break;
    case 2:
        Crt_SPrintf(line, STR(0x101311bc), star, S32(p, 8));
        break;
    case 3:
        Crt_SPrintf(line, STR(0x101311e4), star, S32(p, 8));
        break;
    case 4:
        Crt_SPrintf(line, STR(0x10131208), star, S32(p, 8));
        break;
    default:
        break;      /* 1 and above 4: the line is printed as the stack had it (kept) */
    }
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(line, 0, 3, (short)attr);
    if (GS32(0x101303d8) == -1) {
        G32(0x101303d8) = 0;
        a = S32(p, 0) - 1;
        G32(0x101303d4) = a < 0x14 ? a : 0x14;
    }
    if (GS32(0x1015d108) < GS32(0x101303d8)) {
        if (GS32(0x1015d108) > 0) {
            G32(0x101303d8) = G32(0x1015d108);
            a = GS32(0x101303d8) + 0x14;
            b = S32(p, 0) - 1;
            G32(0x101303d4) = a < b ? a : b;
            Mono_BlankRows();
        } else {
            G32(0x1015d108) = G32(0x101303d8);
        }
    } else if (GS32(0x1015d108) > GS32(0x101303d4)) {
        if (GS32(0x1015d108) < S32(p, 0)) {
            G32(0x101303d4) = G32(0x1015d108);
            a = GS32(0x101303d4) - 0x14;
            G32(0x101303d8) = a > 0 ? a : 0;
            Mono_BlankRows();
        } else {
            G32(0x1015d108) = G32(0x101303d4);
        }
    }
    for (i = GS32(0x101303d8); i <= GS32(0x101303d4); i++) {
        uint8_t *e = GBYTES(0x10473800) + (uint32_t)star * 0x4fc3au + (uint32_t)i * 0x13fu + 0x3a;
        const char *kind, *mark, *name, *state;

        attr = 7;
        if (e[0x73] != 0)
            attr |= 8;
        switch ((uint32_t)e[1] - 1u) {
        case 0:
            kind = STR(0x10131230);
            break;
        case 1:
            kind = STR(0x1013123c);
            break;
        case 3:
            kind = STR(0x10131248);
            break;
        case 7:
            kind = STR(0x10131254);
            break;
        default:
            kind = STR(0x10131260);
            break;
        }
        name = (const char *)(uintptr_t)Sim_10011b9e((short)U16(e, 2), GBYTES(0x10130678), 0x13);
        mark = GS32(0x1015d108) == i ? STR(0x1013126c) : STR(0x10131270);
        Crt_SPrintf(head, STR(0x10131274), mark, i, kind, name);
        switch (e[0]) {
        case 5:
            Crt_SPrintf(line, STR(0x1013128c), head, S32(e, 0x6b), e + 0xad);
            break;
        case 6:
            Crt_SPrintf(line, STR(0x101312ac), head, S32(e, 0x6b), e + 0xad);
            break;
        default:
            state = e[0x73] != 0 ? STR(0x101312cc) : STR(0x101312d8);
            if (S32(e, 0x6f) > 0 && e[0x73] != 0)
                Crt_SPrintf(line, STR(0x101312e4), head, state,
                            (int)(U32(e, 0x6f) - (G32(0x1037305c) - U32(e, 0x67))), e + 0xad);
            else
                Crt_SPrintf(line, STR(0x101312fc), head, state, e + 0xad);
            break;
        }
        if (G32(0x10a3ade8) != 0)
            Mono_PutText(line, 0, (short)(i - GS32(0x101303d8) + 4), (short)attr);
    }
}
