#include "w11.h"
#include "../simmem.h"
#include "../util/access.h"
#include "w9g.h"
#include "../unsorted_1008/w2i.h"
#include "../unsorted_100a/w9a.h"
#include "../unsorted_100b/w8y.h"

typedef uint32_t(__stdcall *W11LoadLibraryFn)(const char *name);
typedef uint32_t(__stdcall *W11GetProcAddressFn)(uint32_t module, const char *name);
typedef int(__stdcall *W11FreeLibraryFn)(uint32_t module);
typedef void(__cdecl *W11LangSetFn)(uint32_t record);
typedef int(__cdecl *W11LangTestFn)(void);
typedef void(__cdecl *W11LangInitFn)(uint32_t language, void *a, void *b, void *c);

#define W11_LANG_RECORD() BYTES(G32(0x10144230))

// FUNCTION: HGSIM 0x1005f6c6
void __cdecl Sim_1005f6c6(void)
{
    char *dll = SIM_PTR(char, 0x10144260);
    char *sim = SIM_PTR(char, 0x10144270);
    uint32_t module;

    Lang_ReadIni(dll, sim);
    G32(0x1017d594) = (uint32_t)(uintptr_t)Sim_100b69d0(sim);
    Text_InitStrings();
    G32(0x101668ec) = SIM_API(W11LoadLibraryFn, 0x10ac55e4)(dll);
    if (G32(0x101668ec) == 0) {
        Sim_10083495();
        return;
    }
    module = G32(0x101668ec);
    G32(0x1015dc60) = SIM_API(W11GetProcAddressFn, 0x10ac55dc)(module, SIM_PTR(const char, 0x1014427c));
    G32(0x1015dc3c) = SIM_API(W11GetProcAddressFn, 0x10ac55dc)(G32(0x101668ec), SIM_PTR(const char, 0x1014428c));
    G32(0x1015dc5c) = SIM_API(W11GetProcAddressFn, 0x10ac55dc)(G32(0x101668ec), SIM_PTR(const char, 0x101442a0));
    ((W11LangSetFn)(uintptr_t)G32(0x1015dc60))(G32(0x10144230));
    G32(0x10157590) = U32(W11_LANG_RECORD(), 0xa);
    G32(0x101668e8) = U32(W11_LANG_RECORD(), 0xe) != 0 && ((W11LangTestFn)(uintptr_t)G32(0x1015dc3c))() != 0;
    /* the original pushes ecx with only its low word loaded (the upper half is what the call above left);
     * the DLL function takes the 16-bit language id */
    ((W11LangInitFn)(uintptr_t)G32(0x1015dc5c))(U16(W11_LANG_RECORD(), 0), SIM_PTR(void, 0x10143a00),
                                                 SIM_PTR(void, 0x10144208), SIM_PTR(void, 0x10144220));
    if (SIM_API(W11FreeLibraryFn, 0x10ac55d8)(G32(0x101668ec)) != 0)
        G32(0x101668ec) = 0;
}
