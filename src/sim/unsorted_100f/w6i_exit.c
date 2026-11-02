#include "w6i.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../crt/hg_rest.h"


// FUNCTION: HGSIM 0x100ff4c0
void __cdecl Crt_CallTable(W6iVoidFn *begin, W6iVoidFn *end)
{
    uintptr_t p;

    for (p = (uintptr_t)begin; p < (uintptr_t)end; p += 4) {
        W6iVoidFn fn = *(W6iVoidFn *)p;

        if (fn != 0)
            fn();
    }
}

// FUNCTION: HGSIM 0x100ff380
void __cdecl Sim_100ff380(void)
{
    W6iVoidFn first = *SIM_PTR(W6iVoidFn, 0x101593cc);

    if (first != 0)
        first();
    Crt_CallTable(SIM_PTR(W6iVoidFn, 0x1012d208), SIM_PTR(W6iVoidFn, 0x1012d410));
    Crt_CallTable(SIM_PTR(W6iVoidFn, 0x1012d000), SIM_PTR(W6iVoidFn, 0x1012d104));
}

// FUNCTION: HGSIM 0x100ff410
void __cdecl Proc_DoExit(uint32_t code, int quick, int returnToCaller)
{
    if (G32(0x1017e8d0) == 1)
        hg_terminate_process(code);     /* C runtime replacement, the rest (crt/hg_rest.h) */
    G32(0x1017e8cc) = 1;
    *GBYTES(0x1017e8c8) = (uint8_t)returnToCaller;
    if (quick == 0) {
        uint32_t start = G32(0x10ac4bc8);

        if (start != 0) {
            uint32_t p;

            for (p = G32(0x10ac4bb8) - 4; p >= start; p -= 4) {
                W6iVoidFn fn = *(W6iVoidFn *)BYTES(p);

                if (fn != 0) {
                    fn();
                    start = G32(0x10ac4bc8);
                }
            }
        }
        Crt_CallTable(SIM_PTR(W6iVoidFn, 0x1012d514), SIM_PTR(W6iVoidFn, 0x1012d724));
    }
    Crt_CallTable(SIM_PTR(W6iVoidFn, 0x1012d828), SIM_PTR(W6iVoidFn, 0x1012d92c));
    if (returnToCaller == 0) {
        G32(0x1017e8d0) = 1;
        hg_exit_process(code);          /* C runtime replacement, the rest (crt/hg_rest.h) */
    }
}

// FUNCTION: HGSIM 0x100ff3b0
void __cdecl Proc_Exit(uint32_t code)
{
    Proc_DoExit(code, 0, 0);
}

// FUNCTION: HGSIM 0x100ff3d0
void __cdecl Proc_ExitQuick(uint32_t code)
{
    Proc_DoExit(code, 1, 0);
}

// FUNCTION: HGSIM 0x100ff3f0
void __cdecl Sim_100ff3f0(void)
{
    Proc_DoExit(0, 0, 1);
}

// FUNCTION: HGSIM 0x100ff400
void __cdecl Sim_100ff400(void)
{
    Proc_DoExit(0, 1, 1);
}
