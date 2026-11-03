#include "w9d.h"
#include "../unsorted_1000/w2e.h"
#include "../unsorted_1003/fns.h"
#include "../unsorted_1003/w7m.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2g.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w7q.h"
#include "../unsorted_100f/w8A.h"
#include "../util/access.h"

#define STR(addr) SIM_PTR(char, addr)

/* The driver name at 0x109e89e4 (12 bytes, then a terminator). */
static void W9d_SetDriver(const char *name)
{
    G32(0x109e89e0) = G32(0x109e89e0) | 1;
    Str_CopyN(STR(0x109e89e4), name, 0xc);
    U8(GBYTES(0x109e89f0), 0) = 0;
}

static void W9d_Switch(char *tok, uint32_t *opts)
{
    char *p;
    int c;

    switch (Crt_ToUpper((int8_t)tok[1])) {
    case 'B':
        p = Crt_StrChr(tok, '=');
        if (p != 0)
            G32(0x1017d5dc) = (uint32_t)(uintptr_t)(p + 1);
        break;
    case 'C':
        opts[0] = 1;
        break;
    case 'D':
        G32(0x1015f1e8) = 1;
        break;
    case 'E':
        Crt_Remove(STR(0x10135574));
        G32(0x1015dc28) = 1;
        break;
    case 'F': {
        int32_t n = 10;

        if (tok[2] == '=')
            n = Sim_100fd290(tok + 3, 0, 10);
        G32(0x1015dc94) = (uint32_t)(0xb5 / n);
        break;
    }
    case 'G':
        p = Crt_StrChr(tok, '=');
        if (p != 0)
            G32(0x1017d5e0) = (uint32_t)(uintptr_t)(p + 1);
        break;
    case 'J':
        if (tok[2] == '=')
            G32(0x1017d154) = (uint32_t)(uintptr_t)(tok + 3);
        /* falls through */
    case 'L':
        G32(0x101668f4) = 1;
        break;
    case 'M':
        Mono_Enable();
        G32(0x1015f1e8) = 1;
        break;
    case 'N':
        G32(0x1015ec58) = 1;
        G32(0x109e89e0) = G32(0x109e89e0) | 4;
        p = Crt_StrChr(tok, '=');
        if (p == 0) {
            U8(GBYTES(0x109e89fe), 0) = 0;
        } else {
            Str_CopyN(STR(0x109e89fe), p + 1, 0x100);
            U8(GBYTES(0x109e8afe), 0) = 0;
        }
        Sim_1003c5a0(STR(0x10135580));
        break;
    case 'O':
        if (tok[3] == '=') {
            c = Crt_ToUpper((int8_t)tok[2]);
            if (c == 'E')
                G32(0x1017e6b4) = (uint32_t)(uintptr_t)(tok + 4);
            else if (c == 'F')
                G32(0x1017e6b0) = (uint32_t)(uintptr_t)(tok + 4);
        }
        break;
    case 'P':
        G32(0x1015dc24) = 1;
        break;
    case 'Q':
        Sim_1003217c();
        break;
    case 'R':
        G32(0x1017b8d4) = 1;
        break;
    case 'S':
        opts[1] = 0;
        break;
    case 'T':
        p = Crt_StrChr(tok, '=');
        if (p != 0)
            Sim_100a7880(p + 1);
        break;
    case 'V':
        if (Crt_ToUpper((int8_t)tok[2]) == 'G') {
            G32(0x109e89e0) = G32(0x109e89e0) | 1;
            p = Crt_StrChr(tok, '=');
            if (p == 0) {
                U8(GBYTES(0x109e89e4), 0) = 0;
            } else {
                Str_CopyN(STR(0x109e89e4), p + 1, 0xc);
                U8(GBYTES(0x109e89f0), 0) = 0;
            }
        } else if (tok[2] == '=') {
            uint32_t digit;

            if (GS32(0x1015965c) > 1)
                digit = (uint32_t)Crt_IsCType((int8_t)tok[3], 4);
            else
                digit = U16(G32(0x10159450), (int32_t)(int8_t)tok[3] * 2) & 4;
            if (digit != 0)
                G32(0x1013a00c) = (uint32_t)Crt_AToI(tok + 3);
        }
        break;
    case 'X':
        if (tok[2] == '=') {
            switch (Crt_ToUpper((int8_t)tok[3])) {
            case 'F':
                Sim_10034855(4);
                break;
            case 'G':
                G32(0x1017b3bc) = 0;
                G32(0x1015d06c) = 1;
                Sim_10034855(1);
                break;
            case 'M':
            case 'P':
                G32(0x1017b3bc) = 1;
                G32(0x1015d06c) = 0;
                Sim_10034855(1);
                break;
            case 'S':
                Sim_10034855(2);
                break;
            default:
                break;
            }
        }
        Mono_Enable();
        G32(0x1015f1e8) = 1;
        opts[1] = 0;
        break;
    case 'Z':
        /* the display choice ("Z=H": the renderer, else software): ignored, the renderer is always there
           (round 42, DECISIONS.md) */
        break;
    default:
        W9D_CALL_100bc280(10, STR(0x1013558c), tok);
        break;
    }
}

// FUNCTION: HGSIM 0x1002ad50
int __cdecl CmdLine_Parse(char *cmdline, uint32_t *opts, char *rest)
{
    int result = 1;
    char *tok;

    if (cmdline == 0) {
        Crt_Printf(STR(0x101354e8));
        result = 0;
    }
    Str_Copy(rest, STR(0x10135514));
    tok = Str_NextToken_100fd560(cmdline, STR(0x101354e4));
    while (tok != 0) {
        if (tok[0] == '/' || tok[0] == '-') {
            char *sw = tok + 1;

            /* round 42: "/d3d" is ignored; "/3dfx" (our Vulkan renderer's slot) also sets the renderer flag
               [0x1017b940], which the original set for "/d3d" or later in SimMain (DECISIONS.md) */
            if (Str_CompareNoCase(sw, STR(0x10135518)) == 0) {
            } else if (Str_CompareNoCase(sw, STR(0x1013551c)) == 0) {
                G32(0x1017b944) = 1;
                G32(0x1017b940) = 1;
            } else if (Str_CompareNoCase(sw, STR(0x10135524)) == 0)
                G32(0x1017c8c4) = 1;
            else if (Str_CompareNoCase(sw, STR(0x1013552c)) == 0)
                G32(0x1017c8c4) = 0;
            else if (Str_CompareNoCase(sw, STR(0x10135534)) == 0)
                G32(0x1017c8c0) = 1;
            else if (Str_CompareNoCase(sw, STR(0x10135540)) == 0)
                G32(0x1017c8e8) = 1;
            else if (Str_CompareNoCase(sw, STR(0x10135548)) == 0)
                W9d_SetDriver(STR(0x10135550));
            else if (Str_CompareNoCase(sw, STR(0x1013555c)) == 0)
                W9d_SetDriver(STR(0x10135560));
            else if (Str_CompareNoCase(sw, STR(0x1013556c)) == 0)
                G32(0x1017b954) = 1;
            else
                W9d_Switch(tok, opts);
        } else {
            Str_Copy(rest, tok);
        }
        tok = Str_NextToken_100fd560(0, STR(0x101354e4));
    }
    Mono_PutTopLine(rest);
    return result;
}
