#include "w5y.h"
#include "w2e.h"
#include "fns.h"
#include "../unsorted_1003/w2e.h"
#include "../util/access.h"

typedef int(__cdecl *SprintfFn)(char *dst, const char *format, ...);
typedef int(__cdecl *Call12Fn)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t,
                               uint32_t, uint32_t, uint32_t, uint32_t);

// FUNCTION: HGSIM 0x10008197
void __cdecl Cache_ShowItemCount(void)
{
    char text[0x1c];

    SIM_FN(SprintfFn, 0x100fc780)(text, (const char *)GBYTES(0x1012de18), G32(0x109f13e0));
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(text, 0x23, 0x18, 0xf);
    G32(0x1015d058) = 1;
}

// FUNCTION: HGSIM 0x100081dd
void __cdecl Cache_ClearItemCount(void)
{
    char text[0x1c];

    SIM_FN(SprintfFn, 0x100fc780)(text, (const char *)GBYTES(0x1012de34), 0x19, 0x19);
    if (G32(0x10a3ade8) != 0)
        Mono_PutText(text, 0x23, 0x18, 0xf);
    G32(0x1015d058) = 0;
}

// FUNCTION: HGSIM 0x100094ba
void __cdecl Sim_100094ba(void *obj)
{
    if (U32(obj, 0x30) == 0)
        return;
    SIM_FN(void(__cdecl *)(void *), 0x100089c6)(obj);
}

// FUNCTION: HGSIM 0x1000e93e
int __cdecl Sim_1000e93e(void)
{
    uint32_t name = (uint32_t)Sim_1000df9c((const char *)GBYTES(0x1012df60));
    uint32_t id = Sim_1000edce();

    return (int)SIM_FN(Call12Fn, 0x1000e737)(0, 0, name, 0, 0, G32(0x101321b4), 0, id, (uint32_t)-1, 0x250, 0, 0);
}
