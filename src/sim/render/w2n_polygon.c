#include "w2n.h"
#include "w33.h"

#define F32(addr) (*SIM_PTR(float, addr))
#define U32(addr) (*SIM_PTR(uint32_t, addr))
#define RENDER_PLUGIN SIM_PTR(void, 0x109e3dc0)
#define RENDER_CALL RENDER_SLOT(int(__cdecl *)(int, void *, void *, int, void *), 0x109e3d30)
#define CAMERA (*SIM_PTR(unsigned char *, 0x1014cae0))
#define PROJECTED_LIST ((unsigned char **)SIM_PTR(unsigned char *, 0x101c76a0))
#define MAX_POLY 255 /* the projected list holds at most 0xff vertices */
#define ONE_BITS 0x3f800000u
/* the renderer's state call [0x109e3d40], 1.0's numbering */
#define RENDER_STATE RENDER_SLOT(int(__cdecl *)(unsigned, unsigned), 0x109e3d40)

/* Fields of a 36-byte plugin vertex taken from a projected vertex: screen x, y as floats, camera z. */
static void ScreenPosition(unsigned char *v, const unsigned char *p)
{
    *(float *)v = (float)*(const int32_t *)(p + 0x18);
    *(float *)(v + 4) = (float)*(const int32_t *)(p + 0x1c);
    *(uint32_t *)(v + 8) = *(const uint32_t *)(p + 8);
}

// FUNCTION: HGSIM 0x100a0339
int __cdecl Render_SubmitVertexPtrs_100a0339(void *texture, void **projected, int count)
{
    unsigned char buf[MAX_POLY * 0x24];
    unsigned char *v = buf;
    void **next = projected;
    int left = count;

    if (texture == NULL)
        return 0;
    while (left-- != 0) {
        const unsigned char *p = *next++;

        ScreenPosition(v, p);
        *(float *)(v + 0xc) = *(float *)(CAMERA + 0x94) / *(const float *)(p + 8);
        *(uint32_t *)(v + 0x1c) = *(const uint32_t *)(p + 0x20);
        *(uint32_t *)(v + 0x20) = *(const uint32_t *)(p + 0x24);
        *(uint32_t *)(v + 0x10) = 0xff;
        v[0x14] = (unsigned char)U32(0x10143fdc);
        v[0x15] = (unsigned char)U32(0x10143fe0);
        v[0x16] = (unsigned char)U32(0x10143fe4);
        v[0x17] = *SIM_PTR(unsigned char, 0x101cfc8c);
        if (left < count - 2)
            *(uint32_t *)(v + 0x18) = 0;
        else
            *(uint32_t *)(v + 0x18) = 0xff000000u;
        v += 0x24;
    }
    /* 1.2 (0x1008d51a): its 3DFX / PowerVR flag here, where 1.0 tested its own "3dfx" one [0x1017d0ac] */
    if (g_w33Voodoo != 0)
        RENDER_CALL(2, RENDER_PLUGIN, buf, count, texture);
    else
        RENDER_CALL(6, RENDER_PLUGIN, buf, count, texture);
    return 1;
}

// FUNCTION: HGSIM 0x100ac738
void __cdecl Render_SubmitClipped_100ac738(unsigned char *verts, unsigned count)
{
    unsigned n = Render_ClipPolygon(verts, count, *(uint32_t *)(CAMERA + 0x3c), *(uint32_t *)(CAMERA + 0x40));
    unsigned char buf[MAX_POLY * 0x24];
    int32_t desc[2];
    unsigned char *out;
    unsigned char **next;
    unsigned i;

    if (n < 3)
        return;
    next = PROJECTED_LIST;
    out = buf + (n - 1) * 0x24;
    for (i = 0; i < n; i++, out -= 0x24) {
        const unsigned char *p = *next++;
        float u;

        ScreenPosition(out, p);
        /* 1.2 (0x100974e8): rhw 1 / z (1.0: 1) */
        *(float *)(out + 0xc) = (float)((double)F32(0x10111000) / (double)*(const float *)(p + 8));
        *(uint32_t *)(out + 0x1c) = ONE_BITS;
        *(uint32_t *)(out + 0x20) = ONE_BITS;
        *(uint32_t *)(out + 0x10) = 0xfe;
        out[0x14] = 0xf2;
        out[0x16] = 0x46;
        out[0x15] = 0xae;
        *(uint32_t *)(out + 0x18) =
            (uint32_t)Render_FogLevel(*(const float *)(p + 8), *(const float *)(verts + 0x10)) << 24;
        u = *(const float *)(p + 0x20);
        if (!(u < F32(0x1012baa4)) && !(u > F32(0x1012baa4))) {
            out[0x17] = 0x40;
            if (g_w33Voodoo != 0)                   /* 1.2's 3DFX / PowerVR mode: the fog byte 0xbf */
#ifdef W2NS_MUTATE /* the checker's negative check only: 0xff instead */
                *(uint32_t *)(out + 0x18) = 0xff000000u;
#else
                *(uint32_t *)(out + 0x18) = 0xbf000000u;
#endif
        } else {
            out[0x17] = 0;
            if (g_w33Voodoo != 0)
                *(uint32_t *)(out + 0x18) = 0xff000000u;
        }
    }
    /* 1.2: its 3DFX / PowerVR mode with fog on turns the renderer's fog off here (1.2's state 1 = our 2) */
    if (U32(0x1017d108) != 0 && g_w33Voodoo != 0)
        RENDER_STATE(2, 0);
    desc[1] = 0xfe;
    RENDER_CALL(0xf, RENDER_PLUGIN, buf, (int)n, desc);
}

// FUNCTION: HGSIM 0x100ad1f3
void __cdecl Sim_100ad1f3(unsigned char *obj, unsigned char *part)
{
    unsigned char recs[MAX_POLY * 0x58];
    float fogParam = *(float *)(obj + 0x18);
    unsigned char *table;
    unsigned char *rec;
    unsigned n, i;
    unsigned char buf[MAX_POLY * 0x24];
    int32_t desc[2];
    unsigned char *out;
    unsigned char **next = PROJECTED_LIST;

    if (part == NULL || (*(uint32_t *)(part + 0x1c) & 0x80000000u) != 0)
        return;
    U32(0x101c9ea8) = 0;
    n = *(uint16_t *)(part + 2);
    table = *(unsigned char **)(*(unsigned char **)(part + 0x24) + 0x30) + 0x18;
    for (i = 0, rec = recs; i < n; i++, rec += 0x58) {
        const unsigned char *src = table + part[*(int32_t *)(part + 4) + i] * 0x58;

        *(uint32_t *)(rec + 0xc) = *(const uint32_t *)(src + 0xc);
        *(uint32_t *)(rec + 0x14) = *(const uint32_t *)(src + 0x14);
        if (!(fogParam < F32(0x1012baa8)) && !(fogParam > F32(0x1012baa8)))
            *(float *)(rec + 0x10) = 0.0f;
        else
            *(float *)(rec + 0x10) = fogParam + F32(0x1012baac);
        *(uint32_t *)(rec + 0x18) = 0;
        *(uint32_t *)(rec + 0x1c) = 0;
        *(uint32_t *)(rec + 0x2c) = 0;
    }
    n = Render_ClipPolygon(recs, n, *(uint32_t *)(CAMERA + 0x3c), *(uint32_t *)(CAMERA + 0x40));
    if (n < 3)
        return;
    out = buf + (n - 1) * 0x24;
    for (i = 0; i < n; i++, out -= 0x24) {
        const unsigned char *p = *next++;

        ScreenPosition(out, p);
        /* 1.2 (0x10097ff0): rhw 1 / z (1.0: 1), light 0xff (1.0: 0xfe) */
        *(float *)(out + 0xc) = (float)((double)F32(0x10111000) / (double)*(const float *)(p + 8));
        *(uint32_t *)(out + 0x1c) = ONE_BITS;
        *(uint32_t *)(out + 0x20) = ONE_BITS;
#ifdef W2NS_MUTATE /* the checker's negative check only: 1.0's light */
        *(uint32_t *)(out + 0x10) = 0xfe;
#else
        *(uint32_t *)(out + 0x10) = 0xff;
#endif
        out[0x14] = 0;
        out[0x16] = 0;
        out[0x15] = 0;
        out[0x17] = 0x80;
        if (g_w33Voodoo == 0)
            *(uint32_t *)(out + 0x18) = (uint32_t)Render_FogLevel(*(const float *)(p + 8), fogParam) << 24;
        else                                        /* 1.2's 3DFX / PowerVR mode: the fog byte 0x80 */
            *(uint32_t *)(out + 0x18) = 0x80000000u;
    }
    desc[1] = 0xff;                                 /* 1.2: 0xff (1.0: 0xfe) */
    if (U32(0x1017d108) != 0 && g_w33Voodoo != 0)
        RENDER_STATE(2, 0);
    RENDER_CALL(0xf, RENDER_PLUGIN, buf, (int)n, desc);
}
