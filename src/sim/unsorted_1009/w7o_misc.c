#include "w7o.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"

typedef uint32_t(__stdcall *W7oGetLogicalDriveStringsAFn)(uint32_t size, char *buffer);
typedef uint32_t(__stdcall *W7oGetDriveTypeAFn)(const char *root);
typedef uint32_t(__stdcall *W7oFindCloseFn)(uint32_t handle);

// FUNCTION: HGSIM 0x1009ffad
char *__cdecl Str_CopyUpper(char *dst, const char *src)
{
    int32_t n = (int32_t)Str_Length(src);
    int32_t i;

    for (i = 0; i < n; i++) {
        int c = (signed char)src[i];
        int letter;

        if (GS32(0x1015965c) > 1)
            letter = Crt_IsCType(c, 0x103);
        else
            letter = U16(G32(0x10159450), c * 2) & 0x103;
        if (letter != 0)
            dst[i] = (char)W7O_CALL_100ff280(c);
        else
            dst[i] = src[i];
    }
    dst[i] = 0;
    return dst + i;
}

// FUNCTION: HGSIM 0x1009cfd4
int __cdecl Sim_1009cfd4(float p, float d, float lo, float hi, uint32_t *outNear, uint32_t *outFar)
{
    float t1, t2;

    if (!(W7O_CALL_100fcb70((double)d) >= (double)GFL(0x1012ba08))) {
        if (!(p >= lo))
            return 1;
        if (p > hi)
            return 1;
        t1 = Float_OfBits(0xff7fc99eu);     /* -3.4e38 */
        t2 = Float_OfBits(0x7f7fc99eu);     /* 3.4e38 */
    } else {
        float a = (float)((double)lo - (double)p);
        float b = (float)((double)hi - (double)p);

        t1 = (float)((double)a / (double)d);
        t2 = (float)((double)b / (double)d);
    }
    if (t1 > t2) {
        *(u32a *)outNear = Bits_OfFloat(t2);
        *(u32a *)outFar = Bits_OfFloat(t1);
    } else {
        *(u32a *)outNear = Bits_OfFloat(t1);
        *(u32a *)outFar = Bits_OfFloat(t2);
    }
    return 0;
}
