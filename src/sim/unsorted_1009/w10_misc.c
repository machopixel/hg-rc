#include "w10.h"
#include "../simmem.h"
#include "../render/w2n.h"
#include "../util/access.h"
#include "../util/util.h"
#include "w2g.h"
#include "../fx/w32.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100f/w8z.h"
#include "../libm/hg_math.h"

#define GD(addr) (*SIM_PTR(double, addr))

// FUNCTION: HGSIM 0x10098f8c
int __cdecl Sim_10098f8c(int index, int count, uint8_t *verts, uint32_t unused)
{
    uint8_t *v = verts;
    int i;

    (void)unused;
    if (count < 2)
        return 0;
    for (i = 0; i < count; i++, v += 0x24) {
        /* fcomp against the double: not above (or unordered) gives 0.01f */
        U32(v, 0x1c) = (double)*(float *)(v + 0x1c) > GD(0x1012b9b0) ? 0x3f7d70a4u : 0x3c23d70au;
        U32(v, 0x20) = (double)*(float *)(v + 0x20) > GD(0x1012b9b0) ? 0x3f7d70a4u : 0x3c23d70au;
        U32(v, 0x18) = 0xff000000u;
    }
    return Render_SubmitVerts_100068d9(index, count, verts);
}

#define GF(addr) (*SIM_PTR(float, addr))
#define SIM_ADDR(addr) ((uint32_t)(uintptr_t)SIM_PTR(void, addr))

// FUNCTION: HGSIM 0x10096ac3
void __cdecl Sim_10096ac3(uint32_t x, uint32_t y, uint32_t z, float vx, float vy, float vz)
{
    uint8_t *obj;
    uint32_t rec;
    float k = GF(0x1012b994);
    float sx, sy, sz, px, py, pz;
    int i;

    /* 1.2 (round 32): the tracer particle explosion instead */
    if (W32_PARTICLES()) {
        Sim12_10084362(*(float *)&x, *(float *)&y, *(float *)&z, vx, vy, vz);
        return;
    }
    if (!((int32_t)(G32(0x1017cc24) + 1u) < GS32(0x1017cc20)))     /* wraps like the original */
        return;
    obj = Sim_10090c7d(SIM_ADDR(0x100027fc), SIM_ADDR(0x10001785), 0, 0);
    if (obj == 0)
        return;
    U16(obj, 0x10) = 1;
    rec = U32(U32(obj, 0x552), 0x8c);
    *(float *)(BYTES(rec) + 0x50) = (float)Sim_10084a94(vx * k, vy * k, vz * k);
    U32(rec, 0x44) = x;
    U32(rec, 0x48) = y;
    U32(rec, 0x4c) = z;
    U32(rec, 0x38) = 0;
    U32(rec, 0x3c) = 0;
    U32(rec, 0x40) = 0;
    *(float *)(obj + 0x56e) = vx / GF(0x1012b968);
    *(float *)(obj + 0x572) = vy / GF(0x1012b968);
    *(float *)(obj + 0x576) = vz / GF(0x1012b968);
    sx = vx / GF(0x1012b958);
    sy = vy / GF(0x1012b958);
    sz = vz / GF(0x1012b958);
    px = sx;
    py = sy;
    pz = sz;
    Sim_100aa39f((void *)(uintptr_t)U32(obj, 0x552));
    Sim_100a9d47((void *)(uintptr_t)U32(obj, 0x552), x, y, z);
    Sim_100aa799((void *)(uintptr_t)U32(obj, 0x552));
    G32(0x1017cc24)++;
    U32(obj, 0x55e) = G32(0x1015dc78);
    for (i = 0; i < 10; i++) {
        uint8_t *p = obj + 0x12 + i * 0x2a;

        U16(p, 0x18) = 1;
        px = px + sx;
        py = py + sy;
        pz = pz + sz;
        *(float *)p = px;
        *(float *)(p + 4) = py;
        *(float *)(p + 8) = pz;
    }
    U32(obj, 0xc) = 1;
}

typedef void *(__cdecl *W10_9LoadFn)(void *name, void *key, uint32_t zero);
typedef void(__cdecl *W10_9TextFn)(uint32_t target, uint32_t x, uint32_t y, void *font, const char *text,
                                     void *colours);
typedef double(__cdecl *W10_9FmodFn)(double, double);

static void W10_9Text(uint32_t target, uint32_t x, uint32_t y, void *font, const char *text)
{
    if (G32(0x101668e8) != 0)
        SIM_FN(W10_9TextFn, 0x100f2f01)(target, x, y, font, text, SIM_PTR(void, 0x109b9920));
    else
        SIM_FN(W10_9TextFn, 0x100f1e10)(target, x, y, font, text, SIM_PTR(void, 0x109b9920));
}

// FUNCTION: HGSIM 0x1009b03d
void __cdecl Sim_1009b03d(uint8_t *obj)
{
    uint32_t target = U32(obj, 0);
    uint32_t unit = PTRTAB(G32(0x1017b8c8));
    void *font;
    float h;

    font = SIM_FN(W10_9LoadFn, 0x100ba7d5)((void *)(uintptr_t)U32(obj, 0x30), (void *)(uintptr_t)G32(0x1014cec4), 0);
    if (font == 0)
        return;
    /* the original also computes a float from +0x18 here that it never uses */
    if (U32(obj, 0x18) != U32(obj, 0x1c))
        U32(obj, 0x1c) = U32(obj, 0x18);
    W10_9Text(target, U32(obj, 0x60), U32(obj, 0x64), font, (const char *)(uintptr_t)U32(obj, 0x40));
    if (G32(0x1015d1ac) == 4) {
        h = (float)hg_fmod(
            hg_fmod((double)*(float *)(BYTES(unit) + 0x68), 360.0) +
                *SIM_PTR(double, 0x1012b9c8),
            360.0);
        /* fcomp: equal or unordered leaves the text alone */
        if (h < *(float *)(obj + 0x4c) || h > *(float *)(obj + 0x4c)) {
            Crt_SPrintf((char *)(uintptr_t)U32(obj, 0x48), SIM_PTR(const char, 0x1014bfe8),
                        (const char *)(uintptr_t)U32(obj, 0x44), (double)h);
            *(float *)(obj + 0x4c) = h;
        }
        W10_9Text(target, U32(obj, 0x68), U32(obj, 0x6c), font, (const char *)(uintptr_t)U32(obj, 0x48));
    }
    Sim_100bb2b0((const uint32_t *)(uintptr_t)U32(obj, 0x30), (const char *)(uintptr_t)G32(0x1014cec4));
}
