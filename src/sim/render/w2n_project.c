#include <string.h>
#include "w2n.h"

#define F32(addr) (*SIM_PTR(float, addr))
#define I32(addr) (*SIM_PTR(int32_t, addr))
#define U32(addr) (*SIM_PTR(uint32_t, addr))

/* Pool vertices, list of projected vertices and its state (see w2n.h). */
#define LIST_OK U32(0x101c9ea4)
#define LIST_COUNT I32(0x101c9ea8)
#define LIST ((unsigned char **)SIM_PTR(unsigned char *, 0x101c76a0))

/*
 * The edge-clipping helpers and ProjectVertex are called through their jump
 * thunks like the original does: hooks_view.c patches ProjectVertex (sub-pixel
 * positions) and must keep seeing these calls.
 */
typedef void *(__cdecl *VertexFn)(void *);
typedef void *(__cdecl *EdgeFn)(void *, void *);
#define PROJECT_VERTEX SIM_FN(VertexFn, 0x1000318e)
#define CLIP_NEAR_EDGE SIM_FN(EdgeFn, 0x1000110e)
#define CLIP_FAR_EDGE SIM_FN(EdgeFn, 0x10001a64)
#define TRANSFORM_VERTEX SIM_FN(VertexFn, 0x1000222f)

// FUNCTION: HGSIM 0x100a62c4
void *__cdecl Render_ProjectVertex(unsigned char *vertex)
{
    unsigned char clip = 0;

    if ((vertex[0x29] & 1) == 0) {
        float scale = F32(0x1012ba78) / *(float *)(vertex + 8);
        int32_t x = (int32_t)((uint32_t)Render_Ftol((double)*(float *)vertex * (double)scale) + U32(0x10185bd4));
        int32_t y;

        *(int32_t *)(vertex + 0x18) = x;
        if (x > I32(0x10185bec))
            clip |= 2;
        if (x < I32(0x10185bd0))
            clip |= 1;
        y = (int32_t)(U32(0x10185bf4) - (uint32_t)Render_Ftol((double)*(float *)(vertex + 4) * (double)scale));
        *(int32_t *)(vertex + 0x1c) = y;
        if (y > I32(0x10185be0))
            clip |= 8;
        if (y < I32(0x10185bf0))
            clip |= 4;
        vertex[0x28] = clip;
        vertex[0x29] |= 1;
        U32(0x101c673c)++;
    } else {
        clip = vertex[0x28];
    }
    *SIM_PTR(unsigned char, 0x101c7680) |= clip;
    *SIM_PTR(unsigned char, 0x101cc2a0) &= clip;
    if (LIST_COUNT >= 0xff) {
        LIST_OK = 0;
    } else {
        LIST[LIST_COUNT] = vertex;
        LIST_COUNT++;
    }
    return vertex;
}

/* Projects the vertex the edge clipper returned. */
static void ProjectResult(void *v)
{
    PROJECT_VERTEX(v);
}

// FUNCTION: HGSIM 0x100a0512
unsigned __cdecl Render_ClipPolygon(unsigned char *verts, unsigned count, uint32_t nearBits, uint32_t farBits)
{
    const float *m = SIM_PTR(float, 0x10185ba0);
    uint32_t savedNear, savedFar;
    unsigned char *cur, *last;
    unsigned i;
    float far;

    memcpy(&far, &farBits, sizeof far);
    if (LIST_OK == 0)
        return 0;
    LIST_COUNT = 0;
    savedNear = U32(0x10185c50);
    savedFar = U32(0x10185bf8);
    U32(0x10185c50) = nearBits;
    U32(0x10185bf8) = farBits;

    for (i = 0, cur = verts; i < count; i++, cur += 0x58) {
        float *x = (float *)(cur + 0x20), *y = (float *)(cur + 0x24), *z = (float *)(cur + 0x28);
        double dx, dy, dz;

        *x = *(float *)(cur + 0xc) - F32(0x10185c48);
        *y = *(float *)(cur + 0x10) - F32(0x10185c44);
        *z = *(float *)(cur + 0x14) - F32(0x10185c4c);
        dx = *x;
        dy = *y;
        dz = *z;
        *x = (float)((dx * m[0] + dy * m[1]) + dz * m[2]);
        *y = (float)((dx * m[3] + dy * m[4]) + dz * m[5]);
        *z = (float)((dx * m[6] + dy * m[7]) + dz * m[8]);

        if (!(*z > F32(0x10185c50)))
            cur[0x30] = 1;
        else if (far > F32(0x1012ba28) && *z >= F32(0x10185bf8))
            cur[0x30] = 2;
        else
            cur[0x30] = 0;
    }

    cur = verts;
    last = verts + (count - 1) * 0x58;
    for (i = 0; i < count && LIST_OK != 0; i++) {
        if (last[0x30] != cur[0x30]) {
            if (cur[0x30] == 0) {
                ProjectResult(last[0x30] == 1 ? CLIP_NEAR_EDGE(last, cur) : CLIP_FAR_EDGE(last, cur));
            } else if (last[0x30] == 0) {
                ProjectResult(cur[0x30] == 1 ? CLIP_NEAR_EDGE(last, cur) : CLIP_FAR_EDGE(last, cur));
            } else if (*(float *)(last + 0x28) > *(float *)(cur + 0x28)) {
                ProjectResult(CLIP_FAR_EDGE(last, cur));
                ProjectResult(CLIP_NEAR_EDGE(last, cur));
            } else {
                ProjectResult(CLIP_NEAR_EDGE(last, cur));
                ProjectResult(CLIP_FAR_EDGE(last, cur));
            }
        }
        if (cur[0x30] == 0) {
            void *r = TRANSFORM_VERTEX(cur);

            if (LIST_OK != 0)
                ProjectResult(r);
        }
        last = cur;
        cur += 0x58;
    }
    U32(0x10185c50) = savedNear;
    U32(0x10185bf8) = savedFar;
    return LIST_OK != 0 ? (unsigned)LIST_COUNT : 0;
}

// FUNCTION: HGSIM 0x10058c3e
int __cdecl Render_FogLevel(float z, float arg)
{
    float fog = SIM_FN(float(__cdecl *)(float, float), 0x10002414)(z, arg);

    return Render_Ftol((double)fog * (double)F32(0x10143fcc));
}
