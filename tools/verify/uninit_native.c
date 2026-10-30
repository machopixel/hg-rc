/*
 * Native test of the uninittrace stubs (src/hooks/hooks_uninit.c): a fake function that returns the leftover in its
 * local [ebp-8] and stores [ebp-4] is called with known leftovers before and after the watchers are installed. The
 * results must not change (the watchers do not touch the game's stack), and the summary must show the leftovers.
 * Build (32-bit), run in a folder whose hgrc.cfg has the word uninittrace:
 *     i686-w64-mingw32-gcc -O1 -I src tools/verify/uninit_native.c src/hooks/hook.c -o uninit_native.exe
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <windows.h>

#define UNINIT_TABLE "../../tools/verify/uninit_native_table.inc"
#include "../../src/hooks/hooks_uninit.c"

void DebugPrintf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

/* The parent-key watcher of 0x1005ab44 looks keys up with these sim functions; not reached here. */
int __cdecl Sim_100b3839(uint32_t key)
{
    (void)key;
    return -1;
}

int __cdecl Sim_100db770(int value)
{
    return value;
}

static volatile DWORD g_ecx;

static DWORD __attribute__((noinline)) CallWithLeftovers(void *fn, DWORD a, DWORD b)
{
    DWORD r;

    __asm__ volatile("movl %2, -16(%%esp)\n\t"
                     "movl %3, -12(%%esp)\n\t"
                     "call *%1\n\t"
                     : "=a"(r)
                     : "r"(fn), "r"(a), "r"(b)
                     : "ecx", "edx", "memory", "cc");
    return r;
}

int main(void)
{
    static const unsigned char kFake[] = {
        0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x10,     /* push ebp; mov ebp, esp; sub esp, 0x10 */
        0x8B, 0x45, 0xF8, 0x8B, 0x4D, 0xFC,     /* mov eax, [ebp-8]; mov ecx, [ebp-4] */
        0x89, 0x0D, 0, 0, 0, 0,                 /* mov [g_ecx], ecx */
        0x8B, 0xE5, 0x5D, 0xC3,                 /* mov esp, ebp; pop ebp; ret */
    };
    unsigned char *code = VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    DWORD addr = (DWORD)(UINT_PTR)&g_ecx, r;
    int bad = 0, i;

    memcpy(code, kFake, sizeof(kFake));
    memcpy(code + 14, &addr, 4);
    r = CallWithLeftovers(code, 0x11111111, 0x22222222);
    printf("before: eax %08lx ecx %08lx\n", r, g_ecx);
    bad |= r != 0x11111111 || g_ecx != 0x22222222;
    HooksUninit_OnSimLoaded((HMODULE)code);
    for (i = 0; i < 3; i++) {
        DWORD a = 0xA0000000u + (DWORD)i, b = 0xB0000000u + (DWORD)(i & 1);

        r = CallWithLeftovers(code, a, b);
        printf("after %d: eax %08lx ecx %08lx\n", i, r, g_ecx);
        bad |= r != a || g_ecx != b;
    }
    HooksUninit_OnSimExit();
    bad |= g_fns[0].calls != 3 || g_sites[0].runs != 3 || g_sites[0].unset[0] != 3 || g_sites[0].unset[1] != 3;
    printf("%s\n", bad ? "FAIL" : "PASS");
    return bad;
}