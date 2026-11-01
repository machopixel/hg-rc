#include "w9f.h"
#include "w2h.h"
#include "w2i.h"
#include "w2k.h"
#include "w6e.h"
#include "w8w.h"
#include "../util/access.h"
#include "../view/view.h"
#include "../unsorted_1001/w5z.h"
#include "../unsorted_1002/w5y.h"
#include "../unsorted_1004/w2b.h"
#include "../unsorted_1004/w3p.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w8z.h"

#define NOW G32(0x1037305c)
#define SIDE_COUNT GS32(0x104737e0)
#define OWN_SIDE GS32(0x1013bb40)
#define NETWORKED G32(0x1015ec58)
#define ROW_FLAG(j) (SIM_PTR(u32a, 0x104727e0)[(uint32_t)(j)])
#define SIDE_CURRENT(a) (SIM_PTR(u32a, 0x104727a0)[(uint32_t)(a)])

/* The 24-byte record 0x10020441 passes to 0x1001f790, which reads all of it: the name, the message text at +0x10
 * and the sound result at +0x14. The original keeps these as adjacent locals (ebp-0x1c / ebp-0x18). */
typedef struct W9fQueueRec {
    char name[16];
    char *text;
    uint32_t sound;
} W9fQueueRec;

/* Marks a name: 's' / 'n' into mark[0] when the text `name` contains "avon" / "avos" (the literals of the call
 * site, avon then avos) and the current block's dword +0x613 is 1 / 0. The mark is byte 3 of the FIRST name of
 * the row or side even when another text was tested (Sim_1007adfc, code 6). */
static void W9f_MarkName(const char *name, char *mark, const uint8_t *block, uint32_t avon, uint32_t avos)
{
    if (Crt_StrStr(name, (const char *)GBYTES(avon)) != 0 && U32(block, 0x613) == 1)
        mark[0] = 's';
    else if (Crt_StrStr(name, (const char *)GBYTES(avos)) != 0 && U32(block, 0x613) == 0)
        mark[0] = 'n';
}

// FUNCTION: HGSIM 0x1007adfc
int __cdecl Sim_1007adfc(int side, int j, int code)
{
    char text[100];
    uint8_t *block = BYTES(U32(PTRTAB(G32(0x1017b8c8)), 0x28));
    uint8_t *row = W9F_ROW(side, j);
    W9fQueueRec rec;

    if (side != OWN_SIDE || ROW_FLAG(j) != 0)
        return 1;
    if (code == 5) {
        Crt_SPrintf(text, (const char *)GBYTES(0x10149c2c), (const char *)row + 0xad, BYTES(G32(0x101c469c)));
        W9f_MarkName((const char *)row + 0x89, (char *)row + 0x8c, block, 0x10149c34, 0x10149c3c);
        Str_Copy(rec.name, (const char *)row + 0x89);
        rec.sound = Sim_1007e7b6((const char *)row + 0x89);
    } else if (code == 6) {
        Crt_SPrintf(text, (const char *)GBYTES(0x10149c44), (const char *)row + 0xad, BYTES(G32(0x101c468c)));
        W9f_MarkName((const char *)row + 0x99, (char *)row + 0x8c, block, 0x10149c4c, 0x10149c54);
        Str_Copy(rec.name, (const char *)row + 0x99);
        rec.sound = Sim_1007e7b6((const char *)row + 0x99);
    }
    if (text[0] == ' ' || row[0x74] == 0)
        rec.text = (char *)GBYTES(0x10179d34);
    else
        rec.text = text;
    Str_CollapseSpaces(rec.text);
    Sim_10020441(&rec);
    ROW_FLAG(j) = 1;
    return 1;
}

// FUNCTION: HGSIM 0x1007b1d0
int __cdecl Sim_1007b1d0(int side, int code)
{
    char text[100];
    uint8_t *block = BYTES(U32(PTRTAB(G32(0x1017b8c8)), 0x28));
    uint8_t *own;
    W9fQueueRec rec;

    if (G32(0x10179d30) != 0 && NETWORKED != 0)
        return 1;
    if (side != OWN_SIDE)
        return 1;
    own = W9F_SIDE(side);
    if (code == 2) {
        Crt_SPrintf(text, (const char *)BYTES(G32(0x101c46f0)));
        W9f_MarkName((const char *)own + 0x18, (char *)own + 0x1b, block, 0x10149c5c, 0x10149c64);
        Str_Copy(rec.name, (const char *)own + 0x18);
        rec.sound = Sim_1007e7b6((const char *)own + 0x18);
        G32(0x10179d30) = 1;
        if (NETWORKED != 0)
            Sim_10041c00();
    } else if (code == 3) {
        Crt_SPrintf(text, (const char *)BYTES(G32(0x101c4688)));
        W9f_MarkName((const char *)own + 0x28, (char *)own + 0x2b, block, 0x10149c6c, 0x10149c74);
        Str_Copy(rec.name, (const char *)own + 0x28);
        rec.sound = Sim_1007e7b6((const char *)own + 0x28);
    } else if (code == 4) {
        Crt_SPrintf(text, (const char *)BYTES(G32(0x101c4684)));
        Str_Copy(rec.name, (const char *)GBYTES(0x10149c7c));                  /* "BET68" */
        rec.sound = Sim_1007e7b6((const char *)GBYTES(0x10149c84));            /* "BET68" */
    }
    rec.text = text;
    Sim_10020441(&rec);
    return 1;
}

/* Resets row b of side a: state 3, no times, and the sound flag of b cleared unless its dword +2 is 0x10. */
static void W9f_ResetRow(uint8_t *row, int flagIndex)
{
    row[0] = 3;
    U32(row, 0x67) = 0xffffffffu;
    U32(row, 0x6b) = 0xffffffffu;
    if (U32(row, 2) != 0x10)
        ROW_FLAG(flagIndex) = 0;
}

// FUNCTION: HGSIM 0x1007b5db
void __cdecl Sim_1007b5db(int side, int j)
{
    int ok = 1;
    int all = 1;
    uint8_t state = 3;
    uint8_t *row = W9F_ROW(side, j);
    int32_t type;
    int expired;
    int32_t i = 0;
    int32_t k;
    int found;
    int hit;
    int any;
    int32_t other;
    uint8_t *otherSide;
    uint8_t *otherRow;

    if (row[0] == 5 || row[0] == 6 || Sim_1007b4d6(side, j) == 0) {
        row[0x73] = 0;
        return;
    }
    if (S32(row, 0x67) == -1) {
        U32(row, 0x67) = NOW;
        row[0x73] = 1;
    }
    type = S32(row, 2);
    switch (type) {
    case 1:
    case 2:
        for (i = 0; i < (int32_t)row[0xee]; i++) {
            ok = ok != 0 && Sim_10079e3b(row + 0xef + i * 2) != 0;
            if (NETWORKED != 0 && ok != 0)
                ok = ok != 0 && Sim_10042908(row + 0xef + i * 2) != 0;
        }
        break;
    case 4:
        for (i = 0; i < (int32_t)row[0xee]; i++) {
            hit = Sim_10079e3b(row + 0xef + i * 2);
            if (NETWORKED != 0 && hit != 0)
                hit = hit != 0 && Sim_10042908(row + 0xef + i * 2) != 0;
            all = all != 0 && hit != 0;
            if (NETWORKED == 0)
                ok = ok != 0 && hit == 0;
            else
                ok = ok != 0 && all == 0;
        }
        break;
    case 8:
        found = 0;
        for (i = 0; i < (int32_t)row[0xee] && found == 0; i++)
            found = Sim_10079ecd(row + 0xef + i * 2, side);
        ok = ok != 0 && found != 0;
        break;
    case 0x20:
        found = 0;
        for (i = 0; i < (int32_t)row[0xee] && found == 0; i++)
            found = Sim_10079ecd(row + 0xef + i * 2, side);
        ok = ok != 0 && found != 0;
        for (i = 0; i < S32(W9F_SIDE(side), 0); i++) {
            if (U32(W9F_ROW(side, i), 0x75) != 0 && j != i)
                ok = ok != 0 && W9F_ROW(side, i)[0] == 5;
        }
        break;
    case 0x100:
        if (side != OWN_SIDE && NETWORKED == 0)
            ok = ok != 0 && SIDE_CURRENT(side) == (uint32_t)j;
        for (i = 0; i < (int32_t)row[0xee]; i++) {
            if (NETWORKED != 0)
                ok = ok != 0 && W9F7_CALL_1007a4af(row + 0xef + i * 2, side) != 0;
            else
                ok = ok != 0 && W9F7_CALL_1007a005(row + 0xef + i * 2, side) != 0;
        }
        break;
    case 0x2000:
        for (i = 0; i < (int32_t)row[0xee]; i++) {
            if (NETWORKED != 0)
                ok = ok != 0 && W9F7_CALL_1007a955(row + 0xef + i * 2, side) == 0;
            else
                ok = ok != 0 && W9F7_CALL_1007a005(row + 0xef + i * 2, side) == 0;
        }
        break;
    case 0x4000:
        if (row[0x73] != 0) {
            any = 0;
            for (i = 0; i < (int32_t)row[0xee]; i++) {
                hit = Sim_10079c86(row + 0xef + i * 2, side);
                any = any != 0 || hit != 0;
            }
            ok = ok != 0 && any != 0;
        } else {
            ok = 0;
        }
        break;
    case 0x8000:
    case 0x8001:
    case 0x8002:
        ok = 0;
        break;
    default:
        break;
    }
    if (S32(row, 0x6f) >= 0 && S32(row, 0x67) >= 0)
        expired = (int32_t)NOW - S32(row, 0x67) > S32(row, 0x6f);
    else
        expired = 0;
    switch (type) {
    case 0x10000:
        other = S16(row, 0xa9);
        if (other > SIDE_COUNT)
            break;
        otherSide = W9F_SIDE(other);
        if (NETWORKED == 0 || GS32(0x101655ac) < 0)
            otherSide[0x38] = 2;
        Sim_1007b1d0(other, otherSide[0x38]);
        U32(otherSide, 8) = NOW;
        state = 5;
        Sim_1007adfc(side, j, 5);
        break;
    case 0x20000:
        other = S16(row, 0xa9);
        if (other > SIDE_COUNT)
            break;
        otherSide = W9F_SIDE(other);
        otherSide[0x38] = 3;
        Sim_1007b1d0(other, otherSide[0x38]);
        U32(otherSide, 8) = NOW;
        state = 5;
        Sim_1007adfc(side, j, 5);
        break;
    case 0x40000:
        other = S16(row, 0xa9);
        if (other > SIDE_COUNT || S16(row, 0xab) > S32(W9F_SIDE(other), 0))
            break;
        otherRow = W9F_ROW(other, S16(row, 0xab));
        otherRow[0x74] = otherRow[0x74] == 0;
        state = 5;
        Sim_1007adfc(side, j, 5);
        break;
    case 0x80000:
        other = S16(row, 0xa9);
        if (other > SIDE_COUNT || S16(row, 0xab) > S32(W9F_SIDE(other), 0))
            break;
        otherRow = W9F_ROW(other, S16(row, 0xab));
        if (otherRow[0] == 6 || otherRow[0] == 5)
            break;
        otherRow[0] = 6;
        U32(otherRow, 0x6b) = NOW;
        Sim_1007adfc(other, S16(row, 0xab), otherRow[0]);
        state = 5;
        Sim_1007adfc(side, j, 5);
        break;
    case 0x100000:
        other = S16(row, 0xa9);
        if (other > SIDE_COUNT || S16(row, 0xab) > S32(W9F_SIDE(other), 0))
            break;
        otherRow = W9F_ROW(other, S16(row, 0xab));
        if (otherRow[0] == 6 || otherRow[0] == 5)
            break;
        otherRow[0] = 5;
        U32(otherRow, 0x6b) = NOW;
        Sim_1007adfc(other, S16(row, 0xab), otherRow[0]);
        state = 5;
        Sim_1007adfc(side, j, 5);
        break;
    case 0x200000:
        other = S16(row, 0xa9);
        if (other > SIDE_COUNT)
            break;
        otherSide = W9F_SIDE(other);
        for (k = 0; k < S32(otherSide, 0); k++)
            W9f_ResetRow(W9F_ROW(other, k), k);
        otherSide[0x38] = 0;
        U32(otherSide, 8) = 0xffffffffu;
        state = 5;
        Sim_1007adfc(side, j, 5);
        break;
    case 0x400000:
        other = S16(row, 0xa9);
        if (other > SIDE_COUNT || S16(row, 0xab) > S32(W9F_SIDE(other), 0))
            break;
        /* the original clears the flag of its stale loop counter here (no loop above runs for this type) */
        W9f_ResetRow(W9F_ROW(other, S16(row, 0xab)), i);
        state = 5;
        Sim_1007adfc(side, j, 5);
        break;
    case 0:
    case 0x200:
    case 0x400:
    case 0x800:
        if (expired != 0) {
            uint32_t group = U32(GBYTES(0x109b7d40), (uint32_t)side * 0x6cu);

            if (PTRTAB(group) != 0 && (S16(PTRTAB(group), 0x1c) & 2) != 0 && NETWORKED != 0)
                return;
            state = 5;
            Sim_1007adfc(side, j, 5);
        }
        break;
    case 4:
        if (expired != 0) {
            if (ok != 0) {
                state = 5;
                Sim_1007adfc(side, j, 5);
            } else {
                state = 6;
            }
        } else {
            if (ok == 0) {
                state = side == OWN_SIDE ? 6 : 8;
                Sim_1007adfc(side, j, 6);
            }
            if (all != 0)
                state = 6;
        }
        break;
    default:
        if (ok != 0) {
            state = 5;
            Sim_1007adfc(side, j, 5);
        } else if (expired != 0) {
            state = 6;
            Sim_1007adfc(side, j, 6);
        }
        break;
    }
    row[0] = state;
    if (state != 3)
        U32(row, 0x6b) = NOW;
}

// FUNCTION: HGSIM 0x1007c42f
void __cdecl Sim_1007c42f(void)
{
    int32_t side, j, k;

    NOW = (uint32_t)(GS32(0x1015dc78) / 0xb5);
    for (side = 0; side < SIDE_COUNT; side++) {
        uint8_t *block = W9F_SIDE(side);
        int32_t current;

        for (j = 0; j < S32(block, 0); j++)
            Sim_1007b5db(side, j);
        if (block[0x38] == 0) {
            int none = 1;
            int failed = 0;

            for (j = 0; j < S32(block, 0); j++) {
                uint8_t *row = W9F_ROW(side, j);

                if (U32(row, 0x75) != 0) {
                    if (row[0] == 5) {
                        none = none != 0;
                    } else if (row[0] == 6 || row[0] == 8) {
                        none = 0;
                        failed = 1;
                    } else {
                        none = 0;
                    }
                }
            }
            if (none != 0 || G32(0x10179d2c) != 0) {
                if (NETWORKED == 0 || GS32(0x101655ac) < 0)
                    block[0x38] = 2;
                Sim_1007b1d0(side, block[0x38]);
                U32(block, 8) = NOW;
            } else if (failed != 0) {
                block[0x38] = 3;
                Sim_1007b1d0(side, block[0x38]);
                U32(block, 8) = NOW;
            } else if (S32(block, 0xc) > 0 && (int32_t)NOW - S32(block, 4) >= S32(block, 0xc)) {
                block[0x38] = 4;
                Sim_1007b1d0(side, block[0x38]);
                U32(block, 8) = NOW;
            }
        /* 1.2 (0x1006932f): the own side wins once its objectives are done, with no [0x1015f1e8] test in a single
           game and no wait for pending script events ([0x1015d528]) */
#ifdef W9FW_MUTATE /* the checker's negative check only: 1.0's tests */
        } else if (side == OWN_SIDE && (NETWORKED != 0 || G32(0x1015f1e8) == 0)) {
#else
        } else if (side == OWN_SIDE) {
#endif
            int allDone = 1;

            if (NETWORKED != 0 && block[0x38] != 2) {
                for (k = 0; k < SIDE_COUNT; k++)
                    allDone &= W9F_SIDE(k)[0x38] != 0;
            }
#ifdef W9FW_MUTATE
            if (allDone != 0 && G32(0x1015d528) == 0 && G32(0x1015f1fc) == 0) {
#else
            if (allDone != 0 && G32(0x1015f1fc) == 0) {
#endif
                G32(0x1015f1fc) = 1;
                G32(0x1015f1e4) = 0;
                G32(0x1015f1d0) = 0;
                if (NETWORKED != 0)
                    Sim_100419e9();
            }
        }
        current = -1;
        for (j = 0; j < S32(block, 0); j++) {
            if (Sim_1007b4d6(side, j) != 0 && (U32(W9F_ROW(side, j), 2) & 0xffff0000u) == 0) {
                current = j;
                break;
            }
        }
        if (SIDE_CURRENT(side) != (uint32_t)current) {
            uint8_t *row = W9F_ROW(side, current);

            SIDE_CURRENT(side) = (uint32_t)current;
            if (U32(row, 2) == 0x4000) {
                for (k = 0; k < (int32_t)row[0xee]; k++) {
                    uint32_t index = row[0xef + k * 2];
                    uint32_t kind = (uint32_t)row[0xf0 + k * 2] << 8;

                    if (kind == 0x200)
                        U32(PTRTAB(index), 0x150) = 0x80000000u;
                    else if (kind == 0x400)
                        U32(GBYTES(0x109e9bec), index * 0x50u) = 0x80000000u;
                }
            }
            Sim_1001661f((uint32_t)side);
        }
    }
}
