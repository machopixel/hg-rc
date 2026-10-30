#include "w8u.h"
#include "w6d.h"
#include "../unsorted_1002/w7l.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_1009/w7o.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x10031ce0
int __cdecl Sim_10031ce0(void *box, uint8_t *data, void *seg)
{
    uint32_t nearV, farV, n2, f2;       /* [ebp-4], [ebp-0x1c], [ebp-0xc], [ebp-0x18] */
    float len;
    uint8_t *faces;
    int i;

    if (box == 0)
        return 0;
    if (Sim_1009cfd4(FL(seg, 0), FL(seg, 0x24), FL(box, 0), FL(box, 4), &nearV, &farV) != 0)
        return 0;
    if (Sim_1009cfd4(FL(seg, 4), FL(seg, 0x28), FL(box, 8), FL(box, 0xc), &n2, &f2) != 0)
        return 0;
    if ((double)Float_OfBits(n2) > (double)Float_OfBits(nearV))
        nearV = n2;
    if (!((double)Float_OfBits(f2) >= (double)Float_OfBits(farV)))
        farV = f2;
    if (Sim_1009cfd4(FL(seg, 8), FL(seg, 0x2c), FL(box, 0x10), FL(box, 0x14), &n2, &f2) != 0)
        return 0;
    if ((double)Float_OfBits(n2) > (double)Float_OfBits(nearV))
        nearV = n2;
    if (!((double)Float_OfBits(f2) >= (double)Float_OfBits(farV)))
        farV = f2;
    if ((double)Float_OfBits(nearV) > (double)Float_OfBits(farV))
        return 0;
    if ((nearV & 0x80000000u) != 0)
        nearV = 0;
    len = Sim_1009cbde(seg);
    if ((double)Float_OfBits(farV) > (double)len)
        farV = Bits_OfFloat(len);
    if ((double)Float_OfBits(nearV) > (double)Float_OfBits(farV))
        return 0;
    if (S32(box, 0x18) == 0)
        return Sim_10031ed3(box, (uint32_t)(uintptr_t)data, seg) != 0 ? 1 : 0;
    faces = BYTES(box) + 0x2c;
    for (i = 0; i < S32(box, 0x18); i++) {
        void *face = (void *)(uintptr_t)U32(faces, 0);

        faces += 4;
        if (Sim_1002e790(face, data + 0x18, seg) != 0)
            return 1;
    }
    return 0;
}
