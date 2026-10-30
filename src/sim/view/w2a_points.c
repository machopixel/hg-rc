#include "w2a.h"
#include "../simmem.h"

// FUNCTION: HGSIM 0x1000856d
void __cdecl Sim_1000856d(void)
{
    void *view = SIM_PTR(void, 0x109e3e20);
    int32_t *a = SIM_PTR(int32_t, 0x1012dc80);
    int32_t *b = SIM_PTR(int32_t, 0x1012dc90);
    int32_t *c = SIM_PTR(int32_t, 0x1012dc88);

    View_ScalePoint(view, a, a);
    View_ScalePoint(view, b, b);
    View_ScalePoint(view, c, c);
}
