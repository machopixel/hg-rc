#include "w11.h"
#include "../unsorted_100f/w6i.h"
#include "../../platform/plat.h"
#include "../simmem.h"
#include "../util/access.h"
#include "w5b.h"
#include "../unsorted_100b/w5y.h"
#include "../unsorted_100b/w7p.h"

typedef int(__cdecl *W11StrcmpFn)(const char *a, const char *b);
typedef int(__cdecl *W11SprintfFn)(char *out, const char *fmt, ...);
typedef int(__stdcall *W11ShowCursorFn)(int show);
typedef uint32_t(__stdcall *W11GetFocusFn)(void);
typedef int(__stdcall *W11MessageBoxFn)(uint32_t window, const char *text, const char *caption, uint32_t flags);

#define W11_BUF SIM_PTR(char, 0x1015f468)
#define W11_FMT(addr) SIM_PTR(const char, addr)
#define W11_TEXT(key) Sim_100b7834((const uint32_t *)(uintptr_t)G32(0x1017d594), (uint32_t)(uintptr_t)(key))
#define W11_SPRINTF SIM_FN(W11SprintfFn, 0x100fc780)

// FUNCTION: HGSIM 0x1005573d
void __cdecl Sim_1005573d(int code, uint32_t value, const char *key)
{
    int hidden = 0;
    uint32_t t1, t2;

    if (G32(0x101653c8) != 0)
        return;
    G32(0x101653c8) = 1;
    if (SIM_FN(W11StrcmpFn, 0x100fcd30)(SIM_PTR(const char, 0x10141da4), key) == 0)
        return;
    /* the texts are looked up in the order the original pushes them (the later argument first) */
    switch (code) {
    case 0x6e:
        t1 = W11_TEXT(key);
        t2 = W11_TEXT(key);
        W11_SPRINTF(W11_BUF, W11_FMT(0x10141dac), G32(0x101c5104), t2, G32(0x101c50fc), G32(0x101c50f8),
                    G32(0x101c50f0), t1, G32(0x101c50e8));
        break;
    case 0x6f:
        W11_SPRINTF(W11_BUF, W11_FMT(0x10141dc4), G32(0x101c50e0), value, G32(0x101c50d8), key, G32(0x101c5030),
                    G32(0x101c502c), G32(0x101c4bbc), G32(0x101c4bb8));
        break;
    case 0x70:
        t1 = W11_TEXT(key);
        t2 = W11_TEXT(key);
        W11_SPRINTF(W11_BUF, W11_FMT(0x10141ddc), G32(0x101c4bc4), t2, G32(0x101c4bc0), G32(0x101c4be4),
                    G32(0x101c4bdc), G32(0x101c4bec), t1, G32(0x101c4be8));
        break;
    default:
        W11_SPRINTF(W11_BUF, W11_FMT(0x10141df8), G32(0x101c4bd4), G32(0x101c4bd0));
        break;
    }
    Sim_1005f9aa(W11_BUF);
    /* the original's ShowCursor loops (shown, remembering it was hidden) and MessageBoxA over the focus window
       (GetFocus): SDL3, over the game window */
    if (!Plat_CursorShown()) {
        hidden = 1;
        Plat_CursorVisible(1);
    }
    if (Plat_MessageBox((void *)(uintptr_t)G32(0x1017b8f0), W11_BUF, (const char *)(uintptr_t)G32(0x101c4be0), 0x12041) == 2) {
        Sim_100bc451();
        Proc_Exit((uint32_t)code);      /* exit, 0x100ff3b0 (converted) */
    }
    if (hidden != 0)
        Plat_CursorVisible(0);
}
