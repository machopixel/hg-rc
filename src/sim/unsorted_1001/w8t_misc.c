#include "w8t.h"
#include "fns.h"
#include "w2e.h"
#include "w3t.h"
#include "w7l.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w7q.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x1001349a
void __cdecl Sim_1001349a(void)
{
    if (G32(0x1015d06c) == 0)
        return;
    if (G32(0x101303dc) != G32(0x101303d0)) {
        G32(0x101303dc) = G32(0x101303d0);
        G32(0x101303d4) = 0xffffffffu;
        G32(0x101303d8) = G32(0x101303d4);
        G32(0x1015d108) = 0;
        Mono_BlankRows();
    }
    if (GS32(0x101303d0) == -1)
        W8T_CALL_10011c28();
    else if (GS32(0x101303d0) == -2)
        Sim_10012970();
    else
        W8T_CALL_10012af0(G32(0x101303d0));
}

/* A float given by its bits (the original moves these constants as dwords). */
static float W8t_Float(uint32_t bits)
{
    union { uint32_t u; float f; } v;

    v.u = bits;
    return v.f;
}

// FUNCTION: HGSIM 0x1001412d
float __cdecl Sim_1001412d(void *obj, float limit)
{
    int chosen = 0;
    uint8_t *block = BYTES(U32(obj, 0x28));
    float step = FL(U32(obj, 0x54), 8);
    float turn = (float)W8T_FABS_1001((double)Sim_10015266(obj));
    double t;

    if (S32(obj, 0) == 1 && S16(obj, 0x1a6) == 5 && Sim_100140df(obj) != 0)
        chosen = 1;
    if (S32(obj, 0) == 1 && S16(obj, 0x1a6) == 8 && U32(obj, 0x224) == 0x100 && U32(obj, 0x228) == 1)
        chosen = 1;
    if (S16(obj, 0x1a6) != 3 && S16(obj, 0x1a6) != 2 && S32(obj, 0) == 1 && (U16(block, 0x562) & 1) == 0 &&
        U32(block, 0x607) == 0 && chosen != 0 && (double)FL(obj, 0xd4) > (double)limit) {
        step = W8t_Float(0x3acccccd);
        if (!((double)turn >= (double)GFL(0x10111138)))
            U8(U32(obj, 0x54), 0x56) = 1;
        return step;
    }
    if (S32(obj, 0) == 1 && (U16(block, 0x562) & 1) != 0 && chosen == 0)
        return W8t_Float(0);
    if ((double)limit * (double)GFL(0x1011113c) < (double)FL(obj, 0xd4))
        step = W8t_Float(0x3c800000);
    else if ((double)limit * (double)GFL(0x10111100) < (double)FL(obj, 0xd4))
        step = W8t_Float((U16(block, 0x562) & 1) != 0 ? 0x3c4ccccd : 0x3c800000);
    else if ((double)FL(obj, 0xd4) < (double)limit)
        step = W8t_Float(0);
    else
        step = W8t_Float((U16(block, 0x562) & 1) != 0 ? 0x3b800000 : 0x3c4ccccd);
    if ((U16(block, 0x562) & 1) != 0 && (double)turn > (double)GFL(0x10111138))
        step = W8t_Float(0);
    if (S32(obj, 0) == 2 || S32(obj, 0) == 8)
        step = W8t_Float(0x3c800000);
    t = W8T_FABS_1001((double)step - (double)GFL(0x10111110));
    if (t >= (double)GFL(0x10111120) &&
        W8T_FABS_1001((double)FL(U32(obj, 0x54), 0xc)) > (double)GFL(0x10111140)) {
        double v = (double)GFL(0x10111118) -
                   W8T_FABS_1001((double)FL(U32(obj, 0x54), 0xc)) / (double)GFL(0x101110fc) / (double)GFL(0x10111144);

        step = (float)v;
        if (!(v >= (double)GFL(0x10111148)))
            step = W8t_Float(0x3acccccd);
    }
    if ((double)FL(U32(obj, 0x28), 0x4e8) > (double)GFL(0x1011110c))
        step = (float)((double)step / (double)GFL(0x10111100));
    if (!((double)step >= (double)GFL(0x10111148)))
        step = W8t_Float(0);
    if (S32(obj, 0) == 2 || S32(obj, 0) == 8) {
        t = W8T_FABS_1001((double)step - (double)GFL(0x10111110));
        if (!(t >= (double)GFL(0x10111120)))
            step = W8t_Float(0x3c4ccccd);
    }
    return step;
}

// FUNCTION: HGSIM 0x1001b36c
void __cdecl Anim_WriteText(const char *name, const uint8_t *model)
{
    void *file;
    int32_t i, j;

    if (name == 0 || model == 0)
        return;
    file = Sim_100fd850(name, (const char *)GBYTES(0x101317ac));
    if (file == 0)
        return;
    Crt_FPrintf(file, (const char *)GBYTES(0x101317b0));
    Crt_FPrintf(file, (const char *)GBYTES(0x101317bc), model + 4);
    Crt_FPrintf(file, (const char *)GBYTES(0x101317cc), U32(U32(model, 0x14), 0));
    Crt_FPrintf(file, (const char *)GBYTES(0x101317d8), U32(U32(model, 0x14), 4));
    Crt_FPrintf(file, (const char *)GBYTES(0x101317e8), U32(U32(model, 0x14), 8));
    Crt_FPrintf(file, (const char *)GBYTES(0x101317f0), U32(U32(model, 0x14), 0xc));
    for (i = 0; i < S32(U32(model, 0x14), 0xc); i++) {
        const uint8_t *track = model + i * 0xc + 0x1c;
        const uint8_t *keys;

        Crt_FPrintf(file, (const char *)GBYTES(0x10131800), U32(track, 0));
        keys = BYTES(U32(track, 4));
        Crt_FPrintf(file, (const char *)GBYTES(0x10131810), U32(keys, 0));
        for (j = 0; j < S32(keys, 0); j++) {
            const uint8_t *k = keys + j * 0x10 + 4;

            Crt_FPrintf(file, (const char *)GBYTES(0x10131824), U32(k, 0), (double)FL(k, 4), (double)FL(k, 8),
                        (double)FL(k, 0xc));
        }
        keys = BYTES(U32(track, 8));
        Crt_FPrintf(file, (const char *)GBYTES(0x10131844), U32(keys, 0));
        for (j = 0; j < S32(keys, 0); j++) {
            const uint8_t *k = keys + j * 0x14 + 4;

            Crt_FPrintf(file, (const char *)GBYTES(0x10131858), U32(k, 0), (double)FL(k, 4), (double)FL(k, 8),
                        (double)FL(k, 0xc), (double)FL(k, 0x10));
        }
    }
    Crt_FPrintf(file, (const char *)GBYTES(0x1013187c));
    {
        const uint8_t *control = BYTES(U32(model, 0x31c));

        for (i = 0; i < S32(control, 0); i++) {
            const uint8_t *e = control + i * 0x39 + 4;

            Crt_FPrintf(file, (const char *)GBYTES(0x1013188c), U32(e, 0));
            if ((int8_t)U8(e, 4) != 0)
                Crt_FPrintf(file, (const char *)GBYTES(0x101318a0), e + 4);
            else
                Crt_FPrintf(file, (const char *)GBYTES(0x101318b4));
            Crt_FPrintf(file, (const char *)GBYTES(0x101318c8), U32(e, 0xd));
            Crt_FPrintf(file, (const char *)GBYTES(0x101318dc), U32(e, 0x11));
            Crt_FPrintf(file, (const char *)GBYTES(0x101318f0), U32(e, 0x15));
            Crt_FPrintf(file, (const char *)GBYTES(0x10131904), U32(e, 0x29), U32(e, 0x2d), U32(e, 0x31), U32(e, 0x35));
            Crt_FPrintf(file, (const char *)GBYTES(0x10131924), U32(e, 0x19), U32(e, 0x1d), U32(e, 0x21), U32(e, 0x25));
        }
    }
    W8T_FCLOSE(file);
}

// FUNCTION: HGSIM 0x1001db6f
void __cdecl Sim_1001db6f(void)
{
    int i;

    Str_Copy((char *)(uintptr_t)G32(0x1013b380), (const char *)GBYTES(0x10131f60));
    Str_Copy((char *)(uintptr_t)G32(0x1013b38c), (const char *)GBYTES(0x10131f70));
    Str_Copy((char *)(uintptr_t)G32(0x1013b398), (const char *)GBYTES(0x10131f88));
    Str_Copy((char *)(uintptr_t)G32(0x1013b434), (const char *)GBYTES(0x10131f9c));
    Str_Copy((char *)(uintptr_t)G32(0x1013b3d4), (const char *)GBYTES(0x10131fb0));
    Str_Copy((char *)(uintptr_t)G32(0x1013b404), (const char *)GBYTES(0x10131fc0));
    G32(0x1015d190) = G32(0x101316bc);
    G32(0x1015d194) = G32(0x101316dc);
    G32(0x1015d198) = G32(0x101316fc);
    G32(0x1015d19c) = G32(0x1013171c);
    Mem_Fill(GBYTES(0x10a1eaa0), 0, 0x19000);
    Mem_Fill(GBYTES(0x10a1d140), 0, 0x578);
    Mem_Fill(GBYTES(0x109f4240), 0, 0x750);
    Mem_Fill(GBYTES(0x10a37b80), 0, 0x750);
    Mem_Fill(GBYTES(0x10a1e340), 0, 0x750);
    Mem_Fill(GBYTES(0x10a382e0), 0, 0x750);
    W8T_CALL_1001d0fa();
    for (i = 0; i < 0x19; i++) {
        uint32_t rec = W8T_CALL_1001b902(0);

        if (rec != 0)
            G32(0x10a1d154 + i * 0x38) = U32(rec, 0);
        G32(0x10a1d174 + i * 0x38) = 0;
    }
    G32(0x10a1d140 + G32(0x1017b8c8) * 0x38u) = 1;
}
