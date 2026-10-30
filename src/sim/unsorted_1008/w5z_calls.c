#include "w5z.h"
#include "../simmem.h"
#include "../util/access.h"

typedef int(__cdecl *Fn1010026a)(void);
typedef uint32_t(__stdcall *SendMessageAFn)(uint32_t hwnd, uint32_t msg, uint32_t wparam, uint32_t lparam);
typedef int(__stdcall *MessageBoxAFn)(uint32_t hwnd, const char *text, const char *caption, uint32_t type);

// FUNCTION: HGSIM 0x1008663f
void __cdecl Sim_1008663f(void)
{
    if (SIM_FN(Fn1010026a, 0x1010026a)() == 0 || G32(0x1015dca0) == 0)
        return;
    SIM_API(SendMessageAFn, 0x10ac57b8)(G32(0x1017b8f0), 0x41e, 0, 0);
    SIM_API(MessageBoxAFn, 0x10ac57e4)(0, SIM_PTR(const char, 0x1014b658), SIM_PTR(const char, 0x1014b64c), 0x10);
}
