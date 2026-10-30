#include "w12.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../render/w2n.h"
#include "../unsorted_1000/w2j.h"
#include "../unsorted_1000/w2k.h"
#include "../unsorted_1000/w4w.h"
#include "../unsorted_1000/w5y.h"
#include "../unsorted_100d/w7q.h"
#include "../unsorted_100e/w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w7q.h"
#include "../unsorted_1010/w7s.h"

/* The C library fopen / fclose and the error report of the DLL. */
#define W12W_FOPEN SIM_FN(uint32_t(__cdecl *)(const char *, const char *), 0x100fd850)
#define W12W_FCLOSE SIM_FN(int(__cdecl *)(uint32_t), 0x100fd760)
#define W12W_REPORT2 SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100bc280)
#define W12W_REPORT3 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100bc280)

typedef union {
    float f;
    uint32_t u;
} W12wBits;

static uint32_t W12W_U(float f)
{
    W12wBits b;

    b.f = f;
    return b.u;
}

/* the checksum keeps its sign and the low 20 bits of its size */
static int32_t W12W_Fold(int32_t v)
{
    uint32_t a = v < 0 ? 0u - (uint32_t)v : (uint32_t)v;

    a &= 0xfffff;
    return v < 0 ? (int32_t)(0u - a) : (int32_t)a;
}

// FUNCTION: HGSIM 0x100e2aa3
int __cdecl Sim_100e2aa3(uint32_t base, uint32_t *offset, uint32_t *out)
{
    char name[17];
    uint32_t hdr, verts, polys, poly, tail = 0, key, total = 0, mat, look;
    int32_t sum = 0, solid = 1, special = 0, i, k, kind, skip = 0;
    uint32_t matCache = G32(0x1017e6bc);

    name[16] = 0; /* start value (DECISIONS.md): a 16-character name ends there */
    G32(0x1017e86c) = G32(0x1017e6c0);
    G32(0x1017e83c) = 0;
    G32(0x1017e840) = 0;
    hdr = base + *offset;
    if (U32(hdr, 0) != G32(0x1012bdc0)) {
        G32(0x1017e838) = 0xfffffffeu;
        return -1;
    }
    kind = (int32_t)S16(hdr, 0x1c) & ~0x3000;
    if (kind == 1) {
        G32(0x1017e83c) = 0;
        G32(0x1017e840) = 1;
    } else if (kind == 2) {
        G32(0x1017e840) = 0;
        G32(0x1017e83c) = 1;
    } else {
        G32(0x1017e840) = 0;
        G32(0x1017e83c) = 0;
    }
    if (U8(hdr, 8) > 0x80) {
        for (i = 0; i < 0x10; i++)
            U8(hdr, 8 + (uint32_t)i) = (uint8_t)(0x100 - (int32_t)(int8_t)U8(hdr, 8 + (uint32_t)i));
    }
    Str_CopyN(name, (const char *)(uintptr_t)(hdr + 8), 0x10);
    Crt_StrUpper(name);
    if (U32(G32(0x1015d1b8), 0x14) == 0) {
        if (name[0] == 'T' || name[0] == 'G' || name[0] == 'D' || name[0] == 'V' || name[0] == 'O' || name[0] == 'S')
            solid = 0;
        if (name[0] == 'W' && name[1] != 'X')
            solid = 0;
    }
    if (name[3] == '_' &&
        (Str_CompareN(name, SIM_PTR(const char, 0x10152680), 4) == 0 ||
         Str_CompareN(name, SIM_PTR(const char, 0x10152688), 4) == 0 ||
         Str_CompareN(name, SIM_PTR(const char, 0x10152690), 4) == 0 ||
         Str_CompareN(name, SIM_PTR(const char, 0x10152698), 4) == 0))
        special = 1;
    key = G32(0x1017e858);
    verts = hdr + 0x20;
    polys = verts + (uint32_t)(int32_t)S16(hdr, 0x18) * 0x2c;
    poly = polys;
    for (k = S16(hdr, 0x1a); k-- != 0;) {
        uint32_t n = U16(poly, 0x2e);

        total += n * 4;
        poly += n <= 4 ? 0x38u : 0x3eu;
    }
    if (*out == 0) {
        *out = (uint32_t)(uintptr_t)Sim_10008f1c(S16(hdr, 0x18), S16(hdr, 0x1a), total, &tail);
        if (*out == 0) {
            G32(0x1017e838) = 0xfffffffeu;
            return -1;
        }
        Str_CopyN((char *)(uintptr_t)(*out + 0xa0), name, 0x10);
        Sim_10008e8b((void *)(uintptr_t)*out, key);
    } else if (Sim_10008cbc((void *)(uintptr_t)*out, key, S16(hdr, 0x18), S16(hdr, 0x1a), total, &tail) == 0) {
        G32(0x1017e838) = 0xfffffffeu;
        return -1;
    }
    for (i = 0; i < S16(hdr, 0x18); i++) {
        uint32_t v = verts + (uint32_t)i * 0x2c;

        Sim_10009136((void *)(uintptr_t)*out,
                     W12W_U((float)((double)FL(v, 0) * (double)GFL(0x1015264c) + (double)GFL(0x1017e844))),
                     W12W_U((float)((double)FL(v, 4) * (double)GFL(0x10152650) + (double)GFL(0x1017e848))),
                     W12W_U((float)((double)FL(v, 8) * (double)GFL(0x10152654) + (double)GFL(0x1017e84c))),
                     U32(v, 0x24), U32(v, 0x28), W12W_U((float)U8(v, 0x25)), W12W_U((float)U8(v, 0x26)),
                     W12W_U((float)U8(v, 0x27)), W12W_U((float)-*(const double *)(uintptr_t)(v + 0xc)),
                     W12W_U((float)-*(const double *)(uintptr_t)(v + 0x14)),
                     W12W_U((float)-*(const double *)(uintptr_t)(v + 0x1c)));
        sum = W12W_Fold((int32_t)((uint32_t)sum + (uint32_t)Render_Ftol(
            (((((double)FL(v, 0) + (double)FL(v, 4)) + (double)FL(v, 8)) + (double)FL(v, 0x24)) + (double)FL(v, 0x28)))));
    }
    poly = polys;
    if (G32(0x1017b954) != 0) {
        if (G32(0x10152648) != 0) {
            G32(0x1017ed8c) = W12W_FOPEN(SIM_PTR(const char, 0x1017ed20), SIM_PTR(const char, 0x101526a0));
            G32(0x10152648) = 0;
        } else {
            G32(0x1017ed8c) = W12W_FOPEN(SIM_PTR(const char, 0x1017ed20), SIM_PTR(const char, 0x101526a4));
        }
    }
    for (i = 0; i < S16(hdr, 0x1a); i++) {
        uint32_t n = U16(poly, 0x2e), step = n <= 4 ? 0x38u : 0x3eu, flags = 0, made, rec, corner;
        uint32_t lookName;
        int found;

        if ((int8_t)U8(poly, 0) != 0)
            Crt_StrUpper((char *)(uintptr_t)poly);
        if (G32(0x1017e6c8) == 0 || G32(0x1017e6c4) == 0)
            W12W_REPORT2(3, 0);
        mat = 0;
        if (special != 0) {
            lookName = G32(0x1017e6c0) + 0x20;
            found = 1;
            mat = G32(0x1017e6c0);
        } else {
            lookName = poly;
            found = 0;
            for (k = 0; k < 3; k++) {
                mat = G32(0x1017e860 + (uint32_t)k * 4);
                if (mat == 0)
                    break;
                if (U32(lookName, 0) == U32(mat, 0x20) &&
                    Str_Compare((const char *)(uintptr_t)lookName, (const char *)(uintptr_t)(mat + 0x20)) == 0) {
                    G32(0x1017e860 + (uint32_t)k * 4) = G32(0x1017e860);
                    G32(0x1017e860) = mat;
                    found = 1;
                    break;
                }
            }
        }
        if (found == 0) {
            look = (uint32_t)(uintptr_t)Sim_100e2837((const char *)(uintptr_t)lookName, SIM_PTR(void, 0x1017e740));
            if (look == 0) {
                mat = G32(0x1017e6c0);
            } else {
                mat = U32(look, 0x34);
                G32(0x1017e868) = G32(0x1017e864);
                G32(0x1017e864) = G32(0x1017e860);
                G32(0x1017e860) = mat;
            }
            if (mat == G32(0x1017e6c0) && G32(0x1017b954) != 0) {
                Crt_FPrintf((void *)(uintptr_t)G32(0x1017ed8c), SIM_PTR(const char, 0x101526a8));
                Crt_FPrintf((void *)(uintptr_t)G32(0x1017ed8c), SIM_PTR(const char, 0x101526c8), name,
                            (const char *)(uintptr_t)poly);
            }
        }
        if (U32(mat, 0x44) == 0) {
            look = (uint32_t)(uintptr_t)Sim_100e2837((const char *)(uintptr_t)mat, SIM_PTR(void, 0x1017e6d0));
            if (look != 0)
                matCache = U32(look, 0x34);
            if (matCache == G32(0x1017e6bc) && G32(0x1017b954) != 0) {
                Crt_FPrintf((void *)(uintptr_t)G32(0x1017ed8c), SIM_PTR(const char, 0x101526e4));
                Crt_FPrintf((void *)(uintptr_t)G32(0x1017ed8c), SIM_PTR(const char, 0x10152708), name,
                            (const char *)(uintptr_t)mat);
            }
            U32(mat, 0x44) = matCache;
        } else {
            matCache = U32(mat, 0x44);
        }
        if (U32(mat, 0x40) == 0xffffffffu) {
            if (solid != 0)
                Sim_100da14c((void *)(uintptr_t)matCache, (void *)(uintptr_t)mat);
            else
                U32(mat, 0x40) = 0x100;
            if (G32(0x1012da38) == 0x1ff &&
                Str_CompareNoCase((const char *)(uintptr_t)poly, SIM_PTR(const char, 0x10152724)) == 0)
                G32(0x1012da38) = U32(mat, 0x40) - 0x100;
            if (G32(0x1012da3c) == 0x1ff &&
                Str_CompareNoCase((const char *)(uintptr_t)poly, SIM_PTR(const char, 0x10152730)) == 0)
                G32(0x1012da3c) = U32(mat, 0x40) - 0x100;
        }
        switch (U32(matCache, 0x40)) {
        case 0:
        case 1:
        case 4:
            flags |= U32(matCache, 0x40) << 12 | U32(matCache, 0x44) << 8 | U32(matCache, 0x48) << 4;
            break;
        case 3:
            flags |= U32(matCache, 0x40) << 12 | U32(mat, 0x40) << 4;
            break;
        case 2:
        case 5:
        case 6:
        case 7:
            flags |= U32(matCache, 0x40) << 12 | U32(matCache, 0x44) << 8 | (U32(mat, 0x40) - 0x100);
            break;
        case 8:
            flags |= 0xb000u | U32(matCache, 0x44) << 8 | (U32(mat, 0x40) - 0x100);
            break;
        case 9:
            flags |= 0xc000u | U32(matCache, 0x44) << 8 | (U32(mat, 0x40) - 0x100);
            break;
        case 10:
            flags |= 0xd000u | U32(matCache, 0x44) << 8 | (U32(mat, 0x40) - 0x100);
            break;
        case 11:
            flags |= 0xe000u | U32(matCache, 0x44) << 8 | (U32(mat, 0x40) - 0x100);
            break;
        }
        made = (uint32_t)(uintptr_t)Sim_100091f7((void *)(uintptr_t)*out, (unsigned short)flags, tail);
        if (made == 0) {
            G32(0x1017e838) = 0xfffffff7u;
            return -1;
        }
        U32(made, 0x18) = W12W_U((float)-*(const double *)(uintptr_t)(poly + 0x16));
        U32(made, 0xc) = U32(made, 0x18);
        U32(made, 0x1c) = W12W_U((float)-*(const double *)(uintptr_t)(poly + 0x1e));
        U32(made, 0x10) = U32(made, 0x1c);
        U32(made, 0x20) = W12W_U((float)-*(const double *)(uintptr_t)(poly + 0x26));
        U32(made, 0x14) = U32(made, 0x20);
        tail += n * 4;
        corner = poly + 0x30;
        for (k = 0; k < (int32_t)n; k++) {
            Sim_10009278((void *)(uintptr_t)*out, (void *)(uintptr_t)made, (unsigned char)U16(corner, (uint32_t)k * 2));
            sum = (int32_t)((uint32_t)sum + U16(corner, (uint32_t)k * 2));
        }
        for (k = (int32_t)n; k < 4; k++)
            sum = (int32_t)((uint32_t)sum + U16(corner, (uint32_t)k * 2));
        sum = (int32_t)((uint32_t)sum + U16(poly, 0x10));
        sum = (int32_t)((uint32_t)sum + n);
        sum = W12W_Fold(sum);
        rec = U32(*out, 0x30) + U32(U32(*out, 0x30), 8) + (uint32_t)i * 0x28;
        if (U32(matCache, 0x40) == 4) {
            uint32_t base18 = U32(*out, 0x30) + 0x18;

            for (k = 0; k < (int32_t)U16(rec, 2); k++) {
                uint32_t v = base18 + (uint32_t)U8(rec, U32(rec, 4) + (uint32_t)k) * 0x58;

                FL(v, 0x34) = (float)((double)U8(matCache, 0x5d) / (double)GFL(0x1012bdc8));
                U8(poly, 0x13) = (uint8_t)Render_Ftol((double)FL(v, 0x34));
                FL(v, 0x38) = (float)((double)U8(matCache, 0x5e) / (double)GFL(0x1012bdc8));
                U8(poly, 0x14) = (uint8_t)Render_Ftol((double)FL(v, 0x38));
                FL(v, 0x3c) = (float)((double)U8(matCache, 0x5f) / (double)GFL(0x1012bdc8));
                U8(poly, 0x15) = (uint8_t)Render_Ftol((double)FL(v, 0x3c));
            }
        } else {
            uint32_t hi = flags & 0xf000;

            if ((hi == 0xb000 || hi == 0xc000 || hi == 0xd000 || hi == 0xe000) && U16(rec, 2) == 3) {
                uint32_t base18 = U32(*out, 0x30) + 0x18, v[3];
                int32_t lo = 0x7fffffff, up = (int32_t)0x80000001, atLo = 0, atUp = 0;

                for (k = 0; k < 3; k++)
                    v[k] = base18 + (uint32_t)U8(rec, U32(rec, 4) + (uint32_t)k) * 0x58;
                for (k = 0; k < 3; k++) {
                    if ((double)lo > (double)FL(v[k], 8))
                        lo = Render_Ftol((double)FL(v[k], 8) + (double)GFL(0x1012bdcc));
                    if (!((double)up >= (double)FL(v[k], 8)))
                        up = Render_Ftol((double)FL(v[k], 8) + (double)GFL(0x1012bdcc));
                }
                for (k = 0; k < 3; k++) {
                    int32_t t = Render_Ftol((double)FL(v[k], 8) + (double)GFL(0x1012bdcc));

                    if (t == lo)
                        atLo++;
                    else if (t == up)
                        atUp++;
                }
                if (atLo != atUp && atLo < atUp)
                    U32(rec, 8) |= 1;
            }
        }
        skip += (int32_t)step;
        poly += step;
    }
    if (G32(0x1017b954) != 0) {
        W12W_FCLOSE(G32(0x1017ed8c));
        G32(0x1017ed8c) = 0;
    }
    *offset += 0x20;
    *offset += (uint32_t)(int32_t)S16(hdr, 0x18) * 0x2c;
    *offset += (uint32_t)skip;
    if (sum != S32(hdr, 4)) {
        G32(0x1017e838) = 0xfffffffeu;
        W12W_REPORT3(0x58, 0x1015273c, 0);
        return -1;
    }
    Sim_100094ba((void *)(uintptr_t)*out);
    G32(0x1017e838) = 0;
    return 0;
}