#include "w3s.h"
#include "fns.h"
#include "../render/w3s_fp.h"
#include "../unsorted_100c/w2l.h"
#include "../util/util.h"
#include "../view/view.h"

// FUNCTION: HGSIM 0x100bd76b
int __cdecl PtrArray_Insert(void *arr, uint32_t item, int(__cdecl *compare)(uint32_t, uint32_t))
{
    int i;
    int j;

    if (item == 0)
        return 0;
    if (S32(arr, 4) >= S32(arr, 0))
        return 0;
    if (compare != 0) {
        for (i = 0; i < S32(arr, 4); i++) {
            if (compare(U32(U32(arr, 8), i * 4), item) > 0)
                break;
        }
        for (j = S32(arr, 4); j > i; j--)
            U32(U32(arr, 8), j * 4) = U32(U32(arr, 8), j * 4 - 4);
        U32(U32(arr, 8), i * 4) = item;
        S32(arr, 4) = S32(arr, 4) + 1;
    } else {
        U32(U32(arr, 8), S32(arr, 4) * 4) = item;
        S32(arr, 4) = S32(arr, 4) + 1;
    }
    return 1;
}

// FUNCTION: HGSIM 0x100beee4
int16_t __cdecl Sim_100beee4(void *obj)
{
    int16_t result = -1;
    uint32_t partner = 0;
    uint32_t partnerObj = 0;
    uint32_t own = U32(obj, 0x28);

    if ((U16(obj, 0x1aa) & 0x200) != 0) {
        partner = PTRTAB(U16(obj, 0x1aa) & 0xff);
        partnerObj = U32(partner, 0x28);
    }
    if (S32(obj, 0) == 1) {
        if (Sim_100c3eb5(BYTES(own)) != 0) {
            result = 10;
        } else if (partner == 0) {
            result = 0;
        } else if (Sim_10079feb(S32(partner, 0)) != 0) {
            result = 0;
        } else if (S32(partner, 0) == 1 && (U16(partnerObj, 0x560) & 0x20) != 0) {
            result = 0;
        } else {
            if (!(F32(obj, 0xd4) > GF32(0x1012bb10)) && Sim_1009c929(0x13) == 3)
                return 0x15;
            if (!(F32(obj, 0xd4) > GF32(0x1012bb10)) && Sim_100beea2(BYTES(own)) != 0)
                result = 0x14;
        }
    }
    return result;
}
