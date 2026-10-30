#include "w5b.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_1002/w3o.h"
#include "../view/view.h"
#include "../util/access.h"

typedef float HG_MAY_ALIAS w5b_f32;
#define W5B_F32(p, off) (*(w5b_f32 *)(BYTES(p) + (off)))
#define W5B_GF32(addr) (*SIM_PTR(w5b_f32, addr))

/* The C library's _ftol: truncation toward zero to 64 bits, of which the low 32 are used. */
static int32_t Ftol(double value)
{
    return (int32_t)(int64_t)value;
}

// FUNCTION: HGSIM 0x10061d67
void __cdecl Sim_10061d67(void *obj)
{
    uint32_t first, second, third;

    Sim_100aa39f(BYTES(U32(U32(obj, 0), 0x48)));
    third = U32(U32(obj, 0), 0x48);
    first = U32(obj, 0x490);
    second = U32(obj, 0x48c);
    Sim_10028c6f(SIM_CALL_10028b90(first, 1));
    Sim_10028c6f(SIM_CALL_10028b90(second, 1));
    Sim_10028c6f(SIM_CALL_10028b90(third, 1));
}

// FUNCTION: HGSIM 0x10062a3a
void __cdecl Sim_10062a3a(void)
{
    Rect_ScalePoint((const int32_t *)BYTES(U32(G32(0x109e893c), 0xec)), SIM_PTR(int32_t, 0x10144ec8), SIM_PTR(int32_t, 0x10144ec8));
    Rect_ScalePoint((const int32_t *)BYTES(U32(G32(0x109e8940), 0xec)), SIM_PTR(int32_t, 0x10144ed0), SIM_PTR(int32_t, 0x10144ed0));
    W5B_GF32(0x10144f38) = (float)((double)GS32(0x109e3e1c) / (double)W5B_GF32(0x1012b5a0) / (double)W5B_GF32(0x1012b5a4));
    G32(0x10144f4c) = 0xffffffffu;
    G32(0x1017694c) = (uint32_t)(uintptr_t)SIM_PTR(void, 0x10176938);
    SIM_CALL_10062386();
}

// FUNCTION: HGSIM 0x100631ef
void __cdecl Sim_100631ef(int *outA, int *outB, int x, int y, int angle)
{
    if (angle != 0) {
        float s = (float)SIM_CALL_10084766((float)angle);
        float c = (float)SIM_CALL_10084766((float)((double)angle + (double)W5B_GF32(0x1012b5a8)));

        *outA = Ftol((double)x * (double)c - (double)y * (double)s);
        *outB = Ftol((double)x * (double)s + (double)y * (double)c);
    } else {
        *outA = x;
        *outB = y;
    }
}

// FUNCTION: HGSIM 0x10068bb2
void __cdecl Sim_10068bb2(uint32_t x, float y, float z, float angle, uint32_t *outA, float *outB, float *outC)
{
    float s, c;

    if (outA == 0 || outB == 0 || outC == 0)
        return;
    if (!(SIM_FABS((double)angle) >= (double)W5B_GF32(0x1012b5f8)))
        return;
    c = (float)SIM_CALL_10084766((float)((double)angle + (double)W5B_GF32(0x1012b650)));
    s = (float)SIM_CALL_10084766(angle);
    *outA = x;
    *outB = (float)((double)y * (double)c - (double)z * (double)s);
    *outC = (float)((double)y * (double)s + (double)z * (double)c);
}

// FUNCTION: HGSIM 0x1006c9a0
void __cdecl Sim_1006c9a0(void *obj, int flag)
{
    uint32_t id;

    if (S32(obj, 0x4f0) == 4 || S32(obj, 0x4f0) == 5)
        return;
    id = U32(U32(obj, 0), 4);
    if (flag != 0 && (int32_t)id == GS32(0x1017b8c8) && G32(0x1015ebbc) == 0)
        U32(obj, 0x4f0) = 5;
    SIM_CALL_1006a12d(id, obj);
}
