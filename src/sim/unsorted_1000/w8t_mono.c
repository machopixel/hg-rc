#include "w8t.h"
#include "w2e.h"
#include "w5a.h"
#include "w5b.h"
#include "w5c.h"
#include "w5y.h"
#include "w6d.h"
#include "w7l.h"
#include "../unsorted_1002/w2i.h"
#include "../unsorted_1003/w2e.h"
#include "../unsorted_1007/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10007890
void __cdecl Sim_10007890(void)
{
    if (G32(0x1015d01c) != 0)
        Sim_1000799a();
    if (G32(0x1012dc9c) != 0)
        Mono_TickSpinner();
    if (G32(0x1015d02c) != 0)
        Mono_ShowFrameRate();
    else if (G32(0x1015d028) != 0)
        Sim_10007c45();
    if (G32(0x1015d034) != 0)
        Sim_10007c9e();
    else if (G32(0x1015d03c) != 0)
        Sim_10007ed5();
    if (G32(0x1015d040) != 0)
        Mono_ShowPosition((const float *)(uintptr_t)G32(0x1014cae0));
    else if (G32(0x1015d048) != 0)
        Sim_10008123();
    if (G32(0x1012dc98) != 0)
        Mono_ShowMemory();
    else if (G32(0x1015d050) != 0)
        Sim_100082be();
    if (G32(0x1015d054) != 0)
        Cache_ShowItemCount();
    else if (G32(0x1015d058) != 0)
        Cache_ClearItemCount();
    if (G32(0x1015d068) != 0)
        G32(0x1015d064) = 1;
    if (G32(0x1015d060) != 0 && GS32(0x1015dc78) > GS32(0x1015d05c)) {
        G32(0x1015d05c) = G32(0x1015dc78) + 0xb5;
        /* the original also pushes [0x1014cf8c], which Sim_1002850e does not read */
        Sim_1002850e();
    }
}

// FUNCTION: HGSIM 0x10007ae3
void __cdecl Mono_ShowFrameRate(void)
{
    char text[0x100];

    if (G32(0x1015d030) == 0)
        return;
    G32(0x1015d07c) += 1;
    G32(0x1015d078) += G32(0x1015dc80);
    if (GS32(0x1015d07c) >= 10) {
        if (GS32(0x1015d078) > 0) {
            int32_t rate = (int32_t)(G32(0x1015d07c) * 0x712u) / GS32(0x1015d078);

            GS32(0x1015d070) = rate / 10;
            GS32(0x1015d074) = (int32_t)((uint32_t)rate - G32(0x1015d070) * 10u);
            W8T_SPRINTF_1000(text, (const char *)GBYTES(0x1012dcb0), G32(0x1015d070), G32(0x1015d074));
            if (G32(0x10a3ade8) != 0)
                Mono_PutText(text, 0x2f, 0, 0x78);
        }
        G32(0x1015d07c) = 0;
        G32(0x1015d078) = 0;
    }
    if (G32(0x1015d030) != 0 && GS32(0x1015d070) > 0) {
        W8T_SPRINTF_1000(text, (const char *)GBYTES(0x1012dcc8), G32(0x1015d070), G32(0x1015d074), G32(0x1015e54c),
                         G32(0x101c672c), G32(0x101c673c));
        W8T_CALL_1007e29a((const char *)GBYTES(0x1012dce8), G32(0x109b98c0), text, G32(0x1012dc80), G32(0x1012dc84));
    }
    G32(0x1015d028) = 1;
}

// FUNCTION: HGSIM 0x10007fba
void __cdecl Mono_ShowPosition(const float *pos)
{
    char text[0x84];

    if (G32(0x1015d044) == 0 && G32(0x10a3ade8) == 0)
        return;
    W8T_SPRINTF_1000(text, (const char *)GBYTES(0x1012dda0), (double)pos[0], (double)pos[1], (double)pos[2]);
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(text, 0x32, 1, 0xf);
    W8T_SPRINTF_1000(text, (const char *)GBYTES(0x1012ddbc), (double)pos[4], (double)pos[3], (double)pos[5]);
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(text, 0x32, 2, 0xf);
    if (G32(0x1015d044) != 0) {
        W8T_SPRINTF_1000(text, (const char *)GBYTES(0x1012ddd8), (double)pos[0], (double)pos[1], (double)pos[2],
                         (double)pos[4], (double)pos[3], (double)pos[5]);
        W8T_CALL_1007e29a((const char *)GBYTES(0x1012de04), G32(0x109b98c0), text, G32(0x1012dc90), G32(0x1012dc94));
    }
    G32(0x1015d048) = 1;
}

// FUNCTION: HGSIM 0x10008221
void __cdecl Mono_ShowMemory(void)
{
    char text[0x28];

    if (G32(0x1015d050) == 0) {
        if (G32(0x10a3ade8) != 0)
            Mono_PutText((const char *)GBYTES(0x1012de50), 0x40, 0, 0x78);
        G32(0x1015d050) = 1;
    }
    W8T_SPRINTF_1000(text, (const char *)GBYTES(0x1012de58), 0);
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(text, 0x44, 0, 0x78);
    if (G32(0x1015d04c) != 0)
        W8T_CALL_1007e29a((const char *)GBYTES(0x1012de60), G32(0x109b98c0), text, G32(0x1012dc88), G32(0x1012dc8c));
}

// FUNCTION: HGSIM 0x100085b7
void __cdecl Mono_ShowVersion(const char *name, uint32_t mem)
{
    char text[0x50];
    const uint32_t *res = (const uint32_t *)(uintptr_t)G32(0x109e3da4);

    W8T_SPRINTF_1000(text, (const char *)GBYTES(0x1012de74), name, res[0], res[1], mem);
    Sim_1007df6c(text, 0, 0x4f3, 0x50);
    if (G32(0x10a3ade8) == 0)
        return;
    W8T_SPRINTF_1000(text, (const char *)GBYTES(0x1012de8c), name, mem);
    Mono_PutTopLine(text);
}
