/* The fall check of a unit (game logic, bit exact: see w7p_states.c for the x87 compare rules). */
#include "w7p.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1005/w5b.h"

#ifdef __GNUC__
typedef float __attribute__((may_alias)) w7p_f32f;
#else
typedef float w7p_f32f;
#endif

#define F32(p, off) (*(w7p_f32f *)(BYTES(p) + (off)))
#define GF32(addr) (*SIM_PTR(w7p_f32f, addr))
#define GF64(addr) (*SIM_PTR(double, addr))
#define RECORD(unit) U32(unit, 0x54)
#define S8(p, off) (*(int8_t *)(BYTES(p) + (off)))

// FUNCTION: HGSIM 0x100c264a
uint32_t __cdecl Sim_100c264a(void *unit)
{
    uint32_t block = U32(unit, 0x28);
    int16_t flag;

    if (S32(block, 0x510) < 0 || !(F32(block, 0x540) < GF32(0x1012bb18) || F32(block, 0x540) > GF32(0x1012bb18)))
        return U32(unit, 0x80);
    flag = (int16_t)S8(RECORD(unit), 0x1e);
    if (!(F32(unit, 0x5c) >= GF32(0x1012bb68))) {
        if (!((double)F32(block, 0x54c) >= GF64(0x1012bb70)))
            flag = 1;
        else if ((double)F32(block, 0x54c) > GF64(0x1012bb78))
            flag = 0;
    }
    Sim_100c274a(unit, (unsigned char)flag);
    if (!(F32(block, 0x54c) >= GF32(0x1012bb80))) {
        char text[0x50];

        W7P_SPRINTF(text, SIM_PTR(const char, 0x10151b90), G32(0x1015dc78), U32(unit, 8), U32(unit, 4));
        Sim_1005f9aa(text);
    }
    return U32(unit, 0x80);
}
