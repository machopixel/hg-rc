#include "w7l.h"
#include "w3q.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_1003/w2e.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../view/view.h"
#include "../util/util.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x100071fb
void __cdecl Sim_100071fb(void)
{
    int i, j;

    for (i = 0; i < 0x200; i++) {
        for (j = 0; j < 0x20; j++) {
            uint8_t *entry = GBYTES(0x10a51ac0) + i * 0x300 + j * 0x18;

            if (U32(entry, 0x14) != 0) {
                Sim_100bb2b0((const uint32_t *)entry, (const char *)(uintptr_t)G32(0x1014ceb8));
                U32(entry, 0x14) = 0;
            }
            U8(entry, 0) = 0;
            U16(entry, 0x10) = 0;
        }
    }
    Sim_100bb2b0((const uint32_t *)GBYTES(0x10a3ae80), (const char *)(uintptr_t)G32(0x1014cf0c));
    for (i = 0; i < 0x200; i++) {
        uint8_t *rec = GBYTES(0x10ac1b00) + i * 0xe;

        U16(rec, 0) = 0xffff;
        U16(rec, 2) = 0;
        U16(rec, 4) = 0;
        U16(rec, 6) = 0;
        U16(rec, 8) = 0xfffe;
        U32(rec, 0xa) = 0xffffffffu;
    }
}

// FUNCTION: HGSIM 0x10007c9e
void __cdecl Sim_10007c9e(void)
{
    char text[0x50];
    int i = 0;
    uint32_t list;

    if (G32(0x10a3ade8) == 0 && G32(0x1015d038) == 0)
        return;
    G32(0x1015d010) = 0;
    G32(0x1015d018) = G32(0x1015d010);
    G32(0x1015d00c) = G32(0x1015d018);
    G32(0x1015d014) = G32(0x1015d00c);
    Sim_10087c86();
    while ((list = G32(0x1036f2a0 + 4 * i)) != 0) {
        Sim_1000951c((const void *)(uintptr_t)list, W7L_FN_100021b7);
        i++;
    }
    Sim_1000951c((const void *)(uintptr_t)G32(0x1017c8d0), W7L_FN_100021b7);
    if (G32(0x10a3ade8) == 0)
        return;
    if (G32(0x1015d03c) == 0) {
        if (G32(0x10a3ade8) != 0) {
            if (G32(0x10a3ade8) != 0)
                Mono_PutText((const char *)GBYTES(0x1012dcfc), 0, 1, 0xf);
            if (G32(0x10a3ade8) != 0)
                Mono_PutText((const char *)GBYTES(0x1012dd1c), 0, 2, 0xf);
        }
        if (G32(0x1015d038) != 0) {
            W7L_SPRINTF_1000(text, (const char *)GBYTES(0x1012dd3c), G32(0x1015d014), G32(0x1015d018), G32(0x1015d00c),
                             G32(0x1015e54c));
            Sim_1007df6c(text, (const char *)(uintptr_t)G32(0x109b98c0), 0xb5, 0x50);
        }
        G32(0x1015d03c) = 1;
    }
    if (G32(0x10a3ade8) == 0)
        return;
    W7L_SPRINTF_1000(text, (const char *)GBYTES(0x1012dd74), G32(0x1015d014));
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(text, 10, 1, 0xf);
    W7L_SPRINTF_1000(text, (const char *)GBYTES(0x1012dd7c), G32(0x1015d00c));
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(text, 0x1a, 1, 0xf);
    W7L_SPRINTF_1000(text, (const char *)GBYTES(0x1012dd84), G32(0x1015d018));
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(text, 10, 2, 0xf);
    W7L_SPRINTF_1000(text, (const char *)GBYTES(0x1012dd8c), G32(0x1015e54c));
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(text, 0x1a, 2, 0xf);
}

// FUNCTION: HGSIM 0x100089c6
void __cdecl Sim_100089c6(void *obj)
{
    uint8_t *model = BYTES(U32(obj, 0x2c));
    uint32_t k;
    uint8_t *p;
    float minx, miny, minz, maxx, maxy, maxz;
    double best = 0.0;

    if (model == 0)
        return;
    k = (uint32_t)(S16(model, 4) - 1);
    p = model + k * 0x58 + 0x18;
    maxx = minx = FL(p, 0xc);
    maxy = miny = FL(p, 0x10);
    maxz = minz = FL(p, 0x14);
    while (k-- != 0) {
        p = model + k * 0x58 + 0x18;
        if (!(FL(p, 0xc) >= minx))
            minx = FL(p, 0xc);
        if (!(FL(p, 0x10) >= miny))
            miny = FL(p, 0x10);
        if (!(FL(p, 0x14) >= minz))
            minz = FL(p, 0x14);
        if (FL(p, 0xc) > maxx)
            maxx = FL(p, 0xc);
        if (FL(p, 0x10) > maxy)
            maxy = FL(p, 0x10);
        if (FL(p, 0x14) > maxz)
            maxz = FL(p, 0x14);
    }
    FL(obj, 0x44) = (float)(((double)maxx + minx) * GFL(0x10111008));
    U32(obj, 0x38) = U32(obj, 0x44);
    FL(obj, 0x48) = (float)(((double)maxy + miny) * GFL(0x10111008));
    U32(obj, 0x3c) = U32(obj, 0x48);
    FL(obj, 0x4c) = (float)(((double)maxz + minz) * GFL(0x10111008));
    U32(obj, 0x40) = U32(obj, 0x4c);
    k = (uint32_t)(int32_t)S16(model, 4);
    while (k-- != 0) {
        double dx, dy, dz, d;

        p = model + k * 0x58 + 0x18;
        dx = (double)FL(p, 0xc) - FL(obj, 0x44);
        dy = (double)FL(p, 0x10) - FL(obj, 0x48);
        dz = (double)FL(p, 0x14) - FL(obj, 0x4c);
        d = dx * dx + dy * dy + dz * dz;
        if (d > best)
            best = d;
    }
    FL(obj, 0x50) = (float)W7L_SQRT_1000(best);
    if ((char)U8(obj, 0xa0) == 'w' || (char)U8(obj, 0xa0) == 'W')
        FL(obj, 0x50) = (float)((double)FL(obj, 0x50) / GFL(0x1011100c));
    else if (Crt_StrStr((const char *)BYTES(obj) + 0xa0, (const char *)GBYTES(0x1012df24)) != 0)
        FL(obj, 0x50) = (float)((double)FL(obj, 0x50) / GF64(0x10111010));
}

// FUNCTION: HGSIM 0x1000ae6e
void __cdecl Sim_1000ae6e(void *obj, float *a, float *b)
{
    uint8_t *head = BYTES(U32(obj, 0));
    uint32_t flags = U32(head, 0xf0);
    uint8_t *rec = 0;
    float first, second;

    if (flags & 0x200)
        rec = BYTES(PTRTAB(flags & 0xff));
    if (rec != 0) {
        float x = FL(U32(rec, 0x28), 0x53c);
        float y = FL(head, 0xd8);
        float t = (float)W7L_CALL_1008486e_1000(x, y);

        if (!(y >= GFL(0x10111048)) && !(t >= GFL(0x1011104c)))
            t = Float_OfBits(0x41200000u);          /* 10.0 */
        else if (t > GFL(0x1011104c))
            t = Float_OfBits(0x41200000u);
        else if (!(t >= GFL(0x10111050)))
            t = Float_OfBits(0x40400000u);          /* 3.0 */
        first = t;
        second = first;
    } else {
        first = Float_OfBits(0x41200000u);
        second = first;
    }
    if (U32(head, 0) == 1 && U32(obj, 0x4fc) == U32(obj, 0x5fb)) {
        *a = (float)((double)first * GFL(0x10111054));
        *b = (float)((double)second * GFL(0x10111054));
    } else if (Sim_1000c749(obj, (int)U32(obj, 0x4fc), 1) != 0) {
        *a = (float)((double)first * GFL(0x10111058));
        *b = (float)((double)second * GFL(0x10111058));
    } else {
        *(u32a *)a = Bits_OfFloat(first);
        *(u32a *)b = Bits_OfFloat(second);
    }
}

// FUNCTION: HGSIM 0x1000c894
void __cdecl Sim_1000c894(void)
{
    const float base = Float_OfBits(0x42340000u);       /* 45.0 */
    float step = (float)((double)base / GFL(0x10111088));
    uint8_t *entry = GBYTES(0x10145508);
    uint8_t *out = GBYTES(0x10a3a600);
    int i, j;

    for (i = 0; i < 0x2e; i++, entry += 0xd6) {
        float angle, scale, amp;
        uint8_t *values, *angles;

        if ((U32(entry, 0x86) & 0x40) == 0)
            continue;
        scale = (float)((double)S32(entry, 0x38) * GFL(0x1013abc0));
        amp = FL(entry, 0x3c);
        values = out + 8;
        angles = out + 0x58;
        U32(out, 0) = (uint32_t)i;
        if (U32(entry, 0x10) == 0x45) {
            step = Float_OfBits(0x40900000u);           /* 4.5 */
            angle = Float_OfBits(0xc2340000u);          /* -45.0 */
        } else {
            step = (float)((double)base / GFL(0x10111088));
            angle = 0.0f;
        }
        for (j = 0; j < 20; j++) {
            float s, c, q;

            angle = (float)((double)angle + step);
            FL(angles, 0) = angle;
            s = (float)(W7L_CALL_10084766_1000(FL(angles, 0)) * amp);
            c = (float)(W7L_CALL_10084766_1000((float)((double)FL(angles, 0) + GFL(0x1011108c))) * amp);
            q = (float)((double)s / scale);
            FL(values, 0) = (float)((double)GFL(0x1011105c) * c * q);
            if (U32(entry, 0x10) == 0x45) {
                FL(values, 0) = (float)((double)FL(values, 0) + GFL(0x10111090));
                FL(values, 0) = (float)((double)FL(values, 0) * GFL(0x10111094));
            }
            values += 4;
            angles += 4;
        }
        out += 0xa8;
        G32(0x1015d0a0) = G32(0x1015d0a0) + 1;
    }
}

// FUNCTION: HGSIM 0x1000cfe2
float __cdecl Sim_1000cfe2(void *obj)
{
    uint32_t sel = U16(obj, 0x1aa) & 0xf00;
    uint32_t index = U16(obj, 0x1aa) & 0xff;
    uint8_t *block = BYTES(U32(obj, 0x28));
    uint32_t x, y, z;

    if (U32(block, 0x494) != 0)
        Sim_100a9cfb(BYTES(U32(block, 0x494)), &x, &y, &z);
    else
        Sim_100a9cfb(BYTES(U32(obj, 0x48)), &x, &y, &z);
    if (sel == 0x200) {
        float d = (float)((double)FL(PTRTAB(index), 0x5c) - Float_OfBits(y));

        return (float)W7L_CALL_1008486e_1000(d, FL(obj, 0xd4));
    }
    return 0.0f;
}
