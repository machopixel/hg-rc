#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"
#include "../unsorted_100b/w6g.h"

typedef uint32_t(__cdecl *W10_7Fn1)(uint32_t arg);
typedef void(__cdecl *W10_7Fn2)(uint32_t a, uint32_t b);
typedef const char *(__cdecl *W10_7TextFn)(uint8_t *menu, uint8_t *item, int32_t row, uint32_t x, uint32_t y,
                                           uint8_t *state);
typedef void(__cdecl *W10_7DrawFn)(uint32_t target, uint32_t x, uint32_t y, uint32_t font, const char *text,
                                   void *colours);

#define FN(off) U32(item, off)

// FUNCTION: HGSIM 0x10077272
void __cdecl Sim_10077272(uint8_t *menu, uint8_t *item, int32_t row, uint32_t x, uint32_t y, uint8_t *state)
{
    char text[0x80];      /* the original's 0x80-byte buffer at ebp-0xa4 */
    int keep = 1;
    int hot, selected;
    uint32_t target, list;
    int32_t cur, count;
    const char *more;

    if (menu == 0 || U32(menu, 0x48) == 0 || state == 0)
        return;
    target = U32(menu, 0);
    if (target == 0 || item == 0 || U32(item, 8) == 0)
        return;
    hot = (uint32_t)row == G32(0x1015dc18) - 0x31u;
    if ((U32(item, 0) & 1) != 0 && FN(0x14) != 0)
        cur = (int32_t)((W10_7Fn1)(uintptr_t)FN(0x14))(U32(item, 0xc));
    else
        cur = S32(item, 4);
    selected = (uint32_t)row == U32(state, 0x10);
    switch (U32(state, 0)) {
    case 1:
        if (FN(0x10) != 0)
            ((W10_7Fn2)(uintptr_t)FN(0x10))((uint32_t)(uintptr_t)state, (uint32_t)(uintptr_t)item);
        if (FN(0x14) != 0) {
            U32(item, 4) = ((W10_7Fn1)(uintptr_t)FN(0x14))(U32(item, 0xc));
            cur = S32(item, 4);
        } else {
            U32(item, 4) = 0;
            cur = 0;
        }
        break;
    case 2:
        if (FN(0x10) != 0)
            ((W10_7Fn2)(uintptr_t)FN(0x10))((uint32_t)(uintptr_t)state, (uint32_t)(uintptr_t)item);
        break;
    case 4:
        if (FN(0x1c) != 0)
            ((W10_7Fn2)(uintptr_t)FN(0x1c))(U32(item, 0xc), (uint32_t)cur);
        if (FN(0x10) != 0)
            ((W10_7Fn2)(uintptr_t)FN(0x10))((uint32_t)(uintptr_t)state, (uint32_t)(uintptr_t)item);
        keep = 0;
        break;
    case 5:
        if (FN(0x20) != 0)
            ((W10_7Fn1)(uintptr_t)FN(0x20))(U32(item, 0xc));
        keep = 0;
        break;
    default:
        break;
    }
    if (S32(U32(item, 8), 4) > 0) {
        count = S32(U32(item, 8), 4);
        cur = (int32_t)((uint32_t)cur + (uint32_t)count) % count;
        list = U32(item, 8);
        Str_CopyN(text, (const char *)(uintptr_t)U32(list, 8 + (uint32_t)cur * 4u), 0x7f);
        text[0x7f] = 0;
        if (U32(U32(item, 8), 0) != 0) {
            more = ((W10_7TextFn)(uintptr_t)U32(U32(item, 8), 0))(menu, item, row, x, y, state);
            if (more != 0) {
                Str_NCat(text, more, 0x7f - Str_Length(text));
                text[0x7f] = 0;
            }
        }
        if (G32(0x101668e8) != 0)
            SIM_FN(W10_7DrawFn, 0x100f2f01)(target, x, y, U32(menu, 0x48), text, SIM_PTR(void, 0x109b9920));
        else if (G32(0x1017b940) != 0)
            SIM_FN(W10_7DrawFn, 0x100f1e10)(target, x, y, U32(menu, 0x48), text, SIM_PTR(void, 0x109b9920));
        else
            SIM_FN(W10_7DrawFn, 0x100ecea6)(target, x, y, U32(menu, 0x48), text, SIM_PTR(void, 0x109b9920));
    }
    if (selected) {
        if (hot && U32(state, (uint32_t)row * 0x14u + 0x18u) == 5 && FN(0x1c) != 0)
            ((W10_7Fn2)(uintptr_t)FN(0x1c))(U32(item, 0xc), (uint32_t)cur);
        if (FN(0x18) != 0 && keep)
            ((W10_7Fn2)(uintptr_t)FN(0x18))(U32(item, 0xc), (uint32_t)cur);
    }
    U32(item, 4) = (uint32_t)cur;
}


typedef uint32_t(__cdecl *W10_7LoadFn)(const char *name, uint32_t ext, void *out);
typedef uint32_t(__cdecl *W10_7SizeFn)(uint32_t shape, uint32_t zero);
typedef int32_t(__cdecl *W10_7HeightFn)(uint32_t font);
typedef void(__cdecl *W10_7BlitFn)(uint32_t target, uint32_t shape, int32_t zero, int32_t x, int32_t y);

/* The shape is drawn by 0x100f0205 when [0x1017b940] is set, else by 0x100e91dc. */
static void W10_7Blit(uint32_t target, uint32_t shape, int32_t x, int32_t y)
{
    if (G32(0x1017b940) != 0)
        SIM_FN(W10_7BlitFn, 0x100f0205)(target, shape, 0, x, y);
    else
        SIM_FN(W10_7BlitFn, 0x100e91dc)(target, shape, 0, x, y);
}

// FUNCTION: HGSIM 0x10076de0
void __cdecl Sim_10076de0(uint8_t *menu, uint8_t *item, int32_t row, int32_t x, int32_t y, uint8_t *state)
{
    int32_t delta = 0, cur, fill, w;
    int keep = 1;
    int hot, selected;
    uint32_t target, font, names, bar, knob;
    int64_t p;

    if (menu == 0 || U32(menu, 0x48) == 0 || state == 0)
        return;
    font = U32(menu, 0x48);
    target = U32(menu, 0);
    if (target == 0 || item == 0 || U32(item, 8) == 0)
        return;
    names = U32(item, 8);
    hot = (uint32_t)row == G32(0x1015dc18) - 0x31u;
    if ((U32(item, 0) & 1) != 0 && FN(0x14) != 0)
        cur = (int32_t)((W10_7Fn1)(uintptr_t)FN(0x14))(U32(item, 0xc));
    else
        cur = S32(item, 4);
    selected = (uint32_t)row == U32(state, 0x10);
    switch (U32(state, 0)) {
    case 1:
        if (FN(0x10) != 0)
            ((W10_7Fn2)(uintptr_t)FN(0x10))((uint32_t)(uintptr_t)state, (uint32_t)(uintptr_t)item);
        if (FN(0x14) != 0) {
            U32(item, 4) = ((W10_7Fn1)(uintptr_t)FN(0x14))(U32(item, 0xc));
            cur = S32(item, 4);
        } else {
            U32(item, 4) = 0;
            cur = 0;
        }
        break;
    case 2:
        if (selected) {
            switch (G32(0x1015dc18)) {
            case 0x20:
            case 0xc8:
                delta = 0x199a;
                break;
            case 0xc9:
                delta = -0x199a;
                break;
            default:
                if (hot)
                    delta = 0x199a;
                break;
            }
        }
        break;
    case 4:
        if (FN(0x1c) != 0)
            ((W10_7Fn2)(uintptr_t)FN(0x1c))(U32(item, 0xc), (uint32_t)cur);
        if (FN(0x10) != 0)
            ((W10_7Fn2)(uintptr_t)FN(0x10))((uint32_t)(uintptr_t)state, (uint32_t)(uintptr_t)item);
        keep = 0;
        break;
    case 5:
        if (FN(0x20) != 0)
            ((W10_7Fn1)(uintptr_t)FN(0x20))(U32(item, 0xc));
        keep = 0;
        break;
    default:
        break;
    }
    if (G32(0x1014cb38) != 0) {
        U8(U32(names, 8), 7) = 0x47;
        U8(U32(names, 0x18), 7) = 0x47;
        U8(U32(names, 0), 7) = 0x47;
        U8(U32(names, 0x10), 7) = 0x47;
    }
    if (G32(0x101c64f0) != 0) {
        bar = SIM_FN(W10_7LoadFn, 0x100ba7d5)((const char *)(uintptr_t)U32(names, 8), G32(0x1014cec0), 0);
        knob = SIM_FN(W10_7LoadFn, 0x100ba7d5)((const char *)(uintptr_t)U32(names, 0x18), G32(0x1014cec0), 0);
    } else {
        bar = SIM_FN(W10_7LoadFn, 0x100ba7d5)((const char *)(uintptr_t)U32(names, 0), G32(0x1014cec0), 0);
        knob = SIM_FN(W10_7LoadFn, 0x100ba7d5)((const char *)(uintptr_t)U32(names, 0x10), G32(0x1014cec0), 0);
    }
    if (bar != 0 && knob != 0) {
        w = (int32_t)SIM_FN(W10_7SizeFn, 0x100ed743)(bar, 0) >> 16;
        cur = (int32_t)((uint32_t)cur + (uint32_t)delta);
        if (cur < 0)
            cur = 0;
        if (cur > 0x10000)
            cur = 0x10000;
        /* 16.16 product, rounded by the bit shifted out last (the original's shrd / adc) */
        p = (int64_t)cur * w;
        fill = (int32_t)(uint32_t)((uint64_t)(p >> 16) + (uint64_t)((p >> 15) & 1));
        y = (int32_t)((uint32_t)y + (uint32_t)SIM_FN(W10_7HeightFn, 0x100ecce0)(font));
        x /= 3;
        W10_7Blit(target, bar, x, y);
        x = (int32_t)((uint32_t)x + (uint32_t)fill);
        x -= ((int32_t)SIM_FN(W10_7SizeFn, 0x100ed743)(knob, 0) >> 16) / 2;
        W10_7Blit(target, knob, x, y);
        if (selected) {
            if (((delta != 0 && U32(state, (uint32_t)row * 0x14u + 0x18u) == 4) ||
                 (hot && U32(state, (uint32_t)row * 0x14u + 0x18u) == 5)) &&
                FN(0x1c) != 0)
                ((W10_7Fn2)(uintptr_t)FN(0x1c))(U32(item, 0xc), (uint32_t)cur);
            if (FN(0x18) != 0 && keep)
                ((W10_7Fn2)(uintptr_t)FN(0x18))(U32(item, 0xc), (uint32_t)cur);
        }
    }
    U32(item, 4) = (uint32_t)cur;
    Sim_100bb2b0((const uint32_t *)(uintptr_t)U32(names, 8), (const char *)(uintptr_t)G32(0x1014cec0));
    Sim_100bb2b0((const uint32_t *)(uintptr_t)U32(names, 0x18), (const char *)(uintptr_t)G32(0x1014cec0));
}