#include "w3t.h"
#include "../util/w3t.h"
#include "../render/w2n.h"
#include "../unsorted_100d/w2f.h"

// FUNCTION: HGSIM 0x1001139e
void __cdecl Sim_1001139e(void *obj)
{
    uint32_t rec;

    switch (U8(obj, 0x1b8)) {
    case 0:
        U32(obj, 0x1dc) = 0;
        U32(obj, 0x1ec) = 0;
        U32(obj, 0x1e0) = 0x21;
        break;
    case 1:
        U32(obj, 0x1dc) = 0;
        U32(obj, 0x1ec) = 2;
        U32(obj, 0x1e0) = 0x21;
        break;
    case 2:
        U32(obj, 0x1dc) = 0;
        U32(obj, 0x1ec) = 2;
        U32(obj, 0x1e0) = 0x64;
        break;
    default:
        break;
    }
    rec = G32(0x10130cd4) + (uint32_t)((int32_t)(int8_t)U8(obj, 0x1b9) * 0x1c);
    U32(obj, 0x27c) = S32(obj, 0) != 1 ? 0 : U32(rec, 0);
    U32(obj, 0x278) = U32(rec, 4);
    U32(obj, 0x1c2) = U32(rec, 8);
    S32(obj, 0x1e4) = Render_Ftol((double)S32(rec, 0xc) / 100.0 * 65.0);
    U32(obj, 0x1e8) = U32(rec, 0x10);
    U32(obj, 0x1f0) = U32(rec, 0x14);
    U32(obj, 0x1f4) = U32(rec, 0x18);
    if (S32(obj, 0x1c2) != 0)
        Sim_100d60b6(BYTES(U32(obj, 0x28)), 0);
}

// FUNCTION: HGSIM 0x10014061
float __cdecl Sim_10014061(int16_t key)
{
    int group = key & 0xf00;
    float v;

    if (group == 0x100)
        v = BitsToFloat(U32(GBYTES(0x109a8650), (key & 0xff) * 0x54));
    else if (group == 0x200)
        v = 100.0f;
    else if (group == 0x400)
        v = 10000.0f;
    else
        v = 100.0f;
    if (!(v >= 100.0f))
        v = 100.0f;
    return v;
}

// FUNCTION: HGSIM 0x10015266
float __cdecl Sim_10015266(const void *obj)
{
    float x;

    if (S32(obj, 4) == GS32(0x1017b8c8) && S32(U32(obj, 0x28), 0x50c) != 0)
        x = (float)((double)FL32(obj, 0x110) - (double)FL32(obj, 0x68));
    else
        x = (float)((double)FL32(obj, 0xe8) - (double)FL32(obj, 0x68));
    if (x > 180.0f)
        x = (float)((double)x - 360.0);
    else if (!(x >= -180.0f))
        x = (float)((double)x + 360.0);
    return x;
}

// FUNCTION: HGSIM 0x1001be08
int __cdecl Sim_1001be08(int t, int period, const int32_t *table, float *out)
{
    int n = table[0];
    int k;
    int prevKey = 0, nextKey = 0;
    const uint8_t *prev = BYTES(table) + 8, *next = BYTES(table) + 8;
    int a, b;
    float s, uf;
    double u;

    if (out == 0 || n < 1)
        return 1;
    t = t % period;
    for (k = 0; k < n; k++) {
        const uint8_t *e = BYTES(table) + 4 + (uint32_t)k * 16u;
        int key = S32(e, 0);

        if (t < key) {
            const uint8_t *before = BYTES(table) + 4 + (uint32_t)(k - 1 < 0 ? n - 1 : k - 1) * 16u;

            prevKey = S32(before, 0);
            prev = before + 4;
            nextKey = k - 1 < 0 ? (int)((uint32_t)key + (uint32_t)period) : key;
            next = e + 4;
            break;
        }
        if (t == key) {
            U32(out, 0) = U32(e, 4);
            U32(out, 4) = U32(e, 8);
            U32(out, 8) = U32(e, 0xc);
            return 0;
        }
        if (k + 1 == n) {
            const uint8_t *first = BYTES(table) + 4;

            prevKey = key;
            prev = e + 4;
            nextKey = (int)((uint32_t)S32(first, 0) + (uint32_t)period);
            next = first + 4;
            break;
        }
    }
    prevKey = (int)((uint32_t)prevKey + 1u);
    a = (int)((uint32_t)t - (uint32_t)prevKey);
    b = (int)((uint32_t)nextKey - (uint32_t)prevKey);
    s = (float)((double)a / (double)b);
    u = 1.0 - (double)s;
    uf = (float)u;
    out[0] = (float)(u * (double)FL32(prev, 0) + (double)s * (double)FL32(next, 0));
    out[1] = (float)((double)uf * (double)FL32(prev, 4) + (double)s * (double)FL32(next, 4));
    out[2] = (float)((double)uf * (double)FL32(prev, 8) + (double)s * (double)FL32(next, 8));
    return 0;
}
