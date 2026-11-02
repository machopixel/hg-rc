#include <string.h>
#include "w8z.h"
#include "../unsorted_1004/w2l.h"
#include "../unsorted_1008/w4v.h"
#include "../unsorted_100b/w6g.h"

/* The 16-byte palette name number i at 0x10185620 (the original adds i * 16 to the address). */
static uint32_t W8z_PaletteName(int32_t i)
{
    return (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x10185620) + ((uint32_t)i << 4);
}

#define W8Z_EXT ((const char *)(uintptr_t)G32(0x1014ced4))

// FUNCTION: HGSIM 0x100d3000
void __cdecl Sim_100d3000(void *unused)
{
    uint32_t pal;
    uint8_t in[3];
    uint8_t out[3];
    uint32_t colour;
    uint32_t word;
    uint32_t i;

    (void)unused;
    if (G32(0x1017b940) == 0)
        return;
    pal = W8Z_CALL_100ba7d5_D((const char *)SIM_PTR(char, 0x10185620), W8Z_EXT, 0);
    if (pal == 0)
        return;
    ((W8zDrv4Fn)(uintptr_t)W8Z_DRIVER(0x14))(0, 0x100, (void *)(uintptr_t)pal, 1);
    for (i = 0; i < 0x100; i++) {
        in[0] = U8(pal, i * 3);
        in[1] = U8(pal, i * 3 + 1);
        in[2] = U8(pal, i * 3 + 2);
        Sim_1004b804(in, out);
        if (i == 0xff)
            colour = 0x808080;
        else
            colour = ((uint32_t)out[0] << 2 << 16) | ((uint32_t)out[1] << 2 << 8) | ((uint32_t)out[2] << 2);
        SIM_PTR(u32a, 0x10185760)[i] = colour;
        if (G32(0x1017b940) != 0)
            word = W8Z_CALL_100f01ea(i);
        else
            word = i;
        SIM_PTR(u16a, 0x101659e0)[i] = (uint16_t)word;
    }
    (*SIM_PTR(W8zSlotFn, 0x109e3d44))(0, SIM_PTR(uint8_t, 0x10185760));
    W8Z_CALL_10062386();
}

// FUNCTION: HGSIM 0x100d3427
void __cdecl Sim_100d3427(int32_t index)
{
    uint32_t name = W8z_PaletteName(index);
    uint32_t pal;

    if (name == 0)
        return;
    pal = W8Z_CALL_100ba7d5_D((const char *)(uintptr_t)name, W8Z_EXT, 0);
    if (pal == 0)
        return;
    Sim_100d3000((void *)(uintptr_t)pal);
    ((W8zDrv1Fn)(uintptr_t)W8Z_DRIVER(0x18))((void *)(uintptr_t)pal);
    Sim_100bb2b0((const uint32_t *)(uintptr_t)name, W8Z_EXT);
}

// FUNCTION: HGSIM 0x100d3510
int __cdecl Sim_100d3510(int32_t index, int32_t time, int mode)
{
    uint32_t from = 0;
    int32_t previous = -1;
    int done = 0;
    uint32_t to;
    int32_t steps;
    int32_t count;

    if (GS32(0x1017e65c) > 0 && mode == 0) {
        from = G32(0x1017e620);
    } else if (GS32(0x1017e65c) <= 0) {
        previous = GS32(0x10152420);
        from = W8Z_CALL_100ba7d5_D((const char *)(uintptr_t)W8z_PaletteName(previous), W8Z_EXT, 0);
    }
    if (from == 0)
        return 0;
    to = W8Z_CALL_100ba7d5_D((const char *)(uintptr_t)W8z_PaletteName(index), W8Z_EXT, 0);
    if (to != 0) {
        G32(0x10152428) = (uint32_t)index;
        G32(0x1015242c) = (uint32_t)previous;
        if (G32(0x1015dc80) != 0) {
            if (mode == 1)
                time >>= 1;
            if (GS32(0x1015dc80) > 0)
                count = W8z_Ftol((double)time / (double)GS32(0x1015dc80) + (double)W8Z_GF(0x1012bcd0));
            else
                count = 10;
            steps = count;
            if (steps == 0)
                steps = 1;
        } else {
            steps = 0x14;
        }
        G32(0x1017e65c) = mode == 2 ? 1u : (uint32_t)steps;
        G32(0x1017e658) = (mode == 1 || mode == 2) ? (uint32_t)steps : 0u;
        Sim_1008c6f5(SIM_PTR(uint8_t, 0x1017e620), (const uint8_t *)(uintptr_t)from, (const uint8_t *)(uintptr_t)to, 0,
                     0x100, (uint32_t)steps);
        done = 1;
        Sim_100bb2b0((const uint32_t *)(uintptr_t)W8z_PaletteName(index), W8Z_EXT);
    }
    if (previous == -1)
        Hg_HeapFree(G32(0x1017b8f8), 1, (void *)(uintptr_t)from);
    else
        Sim_100bb2b0((const uint32_t *)(uintptr_t)W8z_PaletteName(previous), W8Z_EXT);
    return done;
}

// FUNCTION: HGSIM 0x100d3877
void __cdecl Sim_100d3877(int flag)
{
    uint32_t name = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x10185720);
    uint32_t pal;
    void *mem;
    uint32_t first[5];
    uint32_t second[5];
    void *record;
    int32_t step;
    int timer;
    int elapsed = 0;
    int last = -1;
    uint32_t value = 0;

    if (name != 0 && (pal = W8Z_CALL_100ba7d5_D((const char *)(uintptr_t)name, W8Z_EXT, 0)) != 0) {
        Sim_100d3000((void *)(uintptr_t)pal);
        if (flag == 0) {
            ((W8zDrv2Fn)(uintptr_t)W8Z_DRIVER(0x1c))((void *)(uintptr_t)pal, 0x3c);
            Sim_100bb2b0((const uint32_t *)(uintptr_t)name, W8Z_EXT);
            ((W8zDrv1Fn)(uintptr_t)W8Z_DRIVER(0x18))((void *)(uintptr_t)pal);
        } else if (G32(0x1017b940) == 0) {
            mem = Hg_HeapAlloc(G32(0x1017b8f8), 9, G32(0x109e3ce8));
            if (mem != 0) {
                memcpy(first, SIM_PTR(uint8_t, 0x109e3e00), sizeof(first));
                first[0] = (uint32_t)(uintptr_t)second;
                memcpy(second, SIM_PTR(uint8_t, 0x109e3e20), sizeof(second));
                second[0] = (uint32_t)(uintptr_t)mem;
                record = SIM_PTR(uint8_t, 0x109e3e00);
                step = (int32_t)(G32(0x109e3ce8) << 2) / 0xb5;
                timer = (int16_t)W8Z_TIMER_ALLOC(0x100);
                W8Z_TIMER_RESET((unsigned)timer);
                while (elapsed < 0x10f) {
                    elapsed = W8Z_TIMER_ELAPSED((unsigned)timer);
                    if (elapsed > last) {
                        last = elapsed + 1;
                        value = W8Z_CALL_100eda16(first, record, step, value);
                        if (G32(0x1017b920) != 0)
                            ((W8zVoidFn)(uintptr_t)U32(G32(0x1015e68c), 0x18))();
                    }
                }
                W8Z_CALL_100eb070(record, 0xf0);
                if (G32(0x1017b920) != 0)
                    ((W8zVoidFn)(uintptr_t)U32(G32(0x1015e68c), 0x18))();
                ((W8zDrv1Fn)(uintptr_t)W8Z_DRIVER(0x18))((void *)(uintptr_t)pal);
                Hg_HeapFree(G32(0x1017b8f8), 1, mem);
                W8Z_TIMER_FREE((unsigned)timer);
            }
        }
    }
    G32(0x10152420) = 0x10;
    G32(0x10152424) = 0x10;
}
