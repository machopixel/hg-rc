#include "w11.h"
#include "../simmem.h"
#include "../util/access.h"
#include "w9g.h"
#include "../unsorted_1008/w2i.h"
#include "../unsorted_100a/w9a.h"
#include "../unsorted_100b/w8y.h"

// FUNCTION: HGSIM 0x1005f6c6
void __cdecl Sim_1005f6c6(void)
{
    char *dll = SIM_PTR(char, 0x10144260);
    char *sim = SIM_PTR(char, 0x10144270);

    Lang_ReadIni(dll, sim);
    G32(0x1017d594) = (uint32_t)(uintptr_t)Sim_100b69d0(sim);
    Text_InitStrings();
    /* KEYBOARD.DLL is dropped (DECISIONS.md, Diego 2026-11-02): the original's path for a DLL that does not load.
     * The key table [0x10143a00] keeps its built-in contents (no language remap) and [0x101668e8] (multi-byte
     * project text) is left as it is. */
    G32(0x101668ec) = 0;
    Sim_10083495();
}
