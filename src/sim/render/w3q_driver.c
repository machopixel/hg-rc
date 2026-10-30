#include "w3q.h"
#include "../util/access.h"

/* Plugin vertex as the display driver callback takes it (36 bytes, see w2n.h). */
typedef struct DriverVertex {
    float x, y;
    uint32_t unset08, unset0c;
    uint32_t color;
    uint32_t unset14;
    uint32_t fog;
    uint32_t unset1c, unset20;
} DriverVertex;

typedef int(__cdecl *DriverPolyFn)(int count, void *vertices, uint32_t color);
#define DRIVER_POLY (*SIM_PTR(DriverPolyFn, 0x1014cb4c))

static void SetVertex(DriverVertex *v, int x, int y, uint32_t color)
{
    v->x = (float)x;
    v->y = (float)y;
    v->unset08 = 0;
    v->unset0c = 0;
    v->color = color;
    v->unset14 = 0;
    v->fog = 0xff000000u;
    v->unset1c = 0;
    v->unset20 = 0;
}

// FUNCTION: HGSIM 0x100a291d
int __cdecl Sim_100a291d(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color)
{
    DriverVertex v[4];

    SetVertex(&v[0], x0, y0, color);
    SetVertex(&v[1], x1, y1, color);
    SetVertex(&v[2], x2, y2, color);
    SetVertex(&v[3], x3, y3, color);
    return DRIVER_POLY(4, v, color);
}

// FUNCTION: HGSIM 0x100a29da
int __cdecl Sim_100a29da(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    DriverVertex v[3];

    SetVertex(&v[0], x0, y0, color);
    SetVertex(&v[1], x1, y1, color);
    SetVertex(&v[2], x2, y2, color);
    return DRIVER_POLY(3, v, color);
}
