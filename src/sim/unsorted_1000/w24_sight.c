#include "w24.h"
#include "w8t.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_1002/w7l.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_100c/w6h.h"
#include "../unsorted_100c/w8z.h"
#include "../unsorted_100d/w5c.h"
#include "../util/util.h"
#include "../util/w3o.h"

/* The segment record of Sim_100d6f16 / Sim_1009cd36 (0x38 bytes, as in the original frame). */
typedef struct {
    uint32_t w[14];
} W24Segment;

// FUNCTION: HGSIM 0x1000aa9e
int __cdecl Sim_1000aa9e(void *target, void *unit, int check)
{
    W24Segment seg;
    float yaw, range, unused18, unusedc, a;
    uint32_t hit, id;
    uint16_t kind;

    /* the original stores all three differences to one local: the z difference is left */
#ifdef W24S_MUTATE /* the checker's negative check only: the two handed-on values swapped */
    Sim_100cbc6b((float)((double)FL(target, 0x60) - (double)FL(unit, 0x60)), g_w24AimDz, g_w24AimDy, &yaw, &range,
                 &unused18, &unusedc);
#else
    Sim_100cbc6b((float)((double)FL(target, 0x60) - (double)FL(unit, 0x60)), g_w24AimDy, g_w24AimDz, &yaw, &range,
                 &unused18, &unusedc);
#endif
    a = (float)W8T_FMOD_1000((double)FL(target, 0x68) + (double)FL(target, 0x74) - (double)yaw +
                             (double)GFL(0x10111030), 360.0);
    if (check != 0 && !((double)a >= (double)GFL(0x10111034)) && (double)a > (double)GFL(0x10111038))
        return 0;
    Sim_100d6f16(target, &seg);
    Sim_1009cd36(&seg, range);
    hit = 0;
    if (Sim_1002dda9(&seg, &hit, (int)U32(target, 4)) != 0 || Sim_100c2851(&seg, &hit, U32(target, 4), 1) != 0) {
        if (hit != 0) {
            kind = U16(hit, 2);
            id = U32(unit, 4) | 0x200u;
            if ((kind & 0x100) != 0)
                return U16(hit, 0x24) == (id & 0xff);
            if ((kind & 0x200) != 0 && (id & 0x400) != 0)
                return U16(hit, 0x24) == (id & 0xff);
            return 0;
        }
    }
    return U32(target, 0xf0) == (U32(unit, 4) | 0x200u);
}
