#include "w5b.h"
#include "fns.h"
#include "../unsorted_1000/w5b.h"
#include "../unsorted_1001/w3t.h"
#include "../util/access.h"

typedef float HG_MAY_ALIAS w5b_f32;
#define W5B_F32(p, off) (*(w5b_f32 *)(BYTES(p) + (off)))
#define W5B_GF32(addr) (*SIM_PTR(w5b_f32, addr))

/* The C library's _ftol: truncation toward zero to 64 bits, of which the low 32 are used. */
static int32_t Ftol(double value)
{
    return (int32_t)(int64_t)value;
}

/* The 32 bits of the float result of Sim_10015266, which the original stores and passes on as a dword. */
static uint32_t Angle(const void *obj)
{
    float value = Sim_10015266(obj);

    return *(u32a *)&value;
}

// FUNCTION: HGSIM 0x100c0558
void __cdecl Sim_100c0558(void *obj)
{
    Sim_100c48fa(obj, S32(obj, 0x218));
    SIM_CALL_1001403a(obj, (int16_t)U16(obj, 0x1aa));
    Sim_1000ae00(obj, Angle(obj));
    if (S32(obj, 0x204) < GS32(0x1015dc78))
        Sim_100c48fa(obj, 2);
}

// FUNCTION: HGSIM 0x100c0d40
int __cdecl Sim_100c0d40(void *obj, int16_t value)
{
    SIM_CALL_1001403a(obj, value);
    Sim_1000ae00(obj, Angle(obj));
    return !((double)W5B_F32(obj, 0xe0) + (double)W5B_GF32(0x1012bb48) > (double)W5B_F32(obj, 0x5c));
}

// FUNCTION: HGSIM 0x100c3892
int __cdecl Sim_100c3892(void *obj, int limit)
{
    uint32_t block = U32(obj, 0x28);

    if (S32(block, 0x510) >= 6 && SIM_FABS((double)W5B_F32(block, 0x540)) >= (double)W5B_GF32(0x1012bb34)
        && Ftol((double)W5B_F32(block, 0x4e8)) < limit)
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x100ccaa0
char *__cdecl Sim_100ccaa0(int seconds)
{
    int32_t hours = seconds / 3600;
    int32_t minutes = (int32_t)((uint32_t)seconds - (uint32_t)hours * 3600u) / 60;
    int32_t rest = (int32_t)((uint32_t)seconds - (uint32_t)hours * 3600u - (uint32_t)minutes * 60u);

    SIM_SPRINTF(SIM_PTR(char, 0x1017e5d0), SIM_PTR(const char, 0x10151ddc), hours, minutes, rest);
    return SIM_PTR(char, 0x1017e5d0);
}
