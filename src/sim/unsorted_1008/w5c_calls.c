#include "w5c.h"
#include "w2i.h"

// FUNCTION: HGSIM 0x1008d36c
int32_t *__cdecl Sim_1008d36c(const int32_t *rect, int32_t *out, uint32_t arg)
{
    int32_t packed = SIM_CALL_100ed70b(arg);
    int32_t hi = packed >> 16;
    uint32_t lo = (uint32_t)packed & 0xffff;
    int32_t width = (int32_t)((uint32_t)rect[3] - (uint32_t)rect[1] + 1u);
    int32_t height = (int32_t)((uint32_t)rect[4] - (uint32_t)rect[2] + 1u);
    uint32_t q;

    /* hi * 65536 fits in 32 bits, so a 32-bit idiv gives the quotient of the original 64-bit one */
    hi = (int32_t)((uint32_t)hi << 16) / width;
    /* lo * 65536 is positive but may not fit in a signed dword: divide by the magnitude, then apply the sign */
    if (height < 0) {
        q = (lo << 16) / (uint32_t)(-height);
        lo = (uint32_t)(-(int32_t)q);
    } else {
        lo = (lo << 16) / (uint32_t)height;
    }
    Rect_ScaleAroundCenter(rect, out, hi, (int32_t)lo);
    return out;
}
