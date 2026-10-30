#include "hook.h"

#include <stdlib.h>
#include <string.h>

#include "../launcher/debug.h"

#define JMP_REL32_SIZE 5

static void WriteJmp(unsigned char *at, const void *target)
{
    at[0] = 0xE9;
    *(INT32 *)(at + 1) = (INT32)((const unsigned char *)target - (at + JMP_REL32_SIZE));
}

/*
 * A copy of the bytes the patch overwrites, followed by a jump back into the
 * original function. The copied bytes must not contain relative jumps or
 * calls, which is why each hook lists its prologue explicitly.
 */
static void *MakeTrampoline(const unsigned char *code, size_t stolen)
{
    unsigned char *tramp = VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (tramp == NULL)
        return NULL;
    memcpy(tramp, code, stolen);
    WriteJmp(tramp + stolen, code + stolen);
    FlushInstructionCache(GetCurrentProcess(), tramp, stolen + JMP_REL32_SIZE);
    return tramp;
}

/*
 * A stub for a tap: save every register, hand the caller's arguments to
 * `tap`, restore, then run the stolen bytes and continue in the function.
 */
static void *MakeTapStub(const unsigned char *code, size_t stolen, HookTapFn tap)
{
    unsigned char *stub = VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    unsigned char *p = stub;

    if (stub == NULL)
        return NULL;
    *p++ = 0x60;                                      /* pushad */
    *p++ = 0x9C;                                      /* pushfd */
    *p++ = 0x8D; *p++ = 0x44; *p++ = 0x24; *p++ = 0x28; /* lea eax,[esp+0x28]: the arguments */
    *p++ = 0x50;                                      /* push eax */
    *p++ = 0xB8;                                      /* mov eax, tap */
    *(DWORD *)p = (DWORD)(UINT_PTR)tap;
    p += 4;
    *p++ = 0xFF; *p++ = 0xD0;                         /* call eax */
    *p++ = 0x83; *p++ = 0xC4; *p++ = 0x04;            /* add esp,4 */
    *p++ = 0x9D;                                      /* popfd */
    *p++ = 0x61;                                      /* popad */
    memcpy(p, code, stolen);
    p += stolen;
    WriteJmp(p, code + stolen);
    p += JMP_REL32_SIZE;
    FlushInstructionCache(GetCurrentProcess(), stub, (SIZE_T)(p - stub));
    return stub;
}

static int InstallOne(HMODULE module, const HookDesc *hook)
{
    unsigned char *code = (unsigned char *)module + hook->rva;
    DWORD oldProtect;
    void *tramp = NULL;
    void *entry = hook->replacement;
    size_t i;

    if (hook->stolen < JMP_REL32_SIZE || hook->stolen > HOOK_MAX_PROLOGUE) {
        DebugPrintf("hook %s: bad prologue length %u\n", hook->name, (unsigned)hook->stolen);
        return 0;
    }
    if (memcmp(code, hook->expect, hook->stolen) != 0) {
        DebugPrintf("hook %s: bytes at %s+0x%lx differ from the expected build, skipped\n",
                    hook->name, hook->module, hook->rva);
        return 0;
    }
    if ((hook->replacement == NULL) == (hook->tap == NULL)) {
        DebugPrintf("hook %s: needs exactly one of replacement and tap\n", hook->name);
        return 0;
    }
    if (hook->tap != NULL) {
        tramp = MakeTapStub(code, hook->stolen, hook->tap);
        entry = tramp;
    } else if (hook->original != NULL) {
        tramp = MakeTrampoline(code, hook->stolen);
    }
    if ((hook->tap != NULL || hook->original != NULL) && tramp == NULL)
        return 0;
    if (!VirtualProtect(code, hook->stolen, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        DebugPrintf("hook %s: VirtualProtect failed, error %lu\n", hook->name, GetLastError());
        return 0;
    }
    WriteJmp(code, entry);
    for (i = JMP_REL32_SIZE; i < hook->stolen; i++)
        code[i] = 0x90; /* nop, so a disassembler stays in step */
    VirtualProtect(code, hook->stolen, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), code, hook->stolen);

    if (hook->original != NULL && hook->tap == NULL)
        *hook->original = tramp;
    DebugPrintf("hook %s: installed at %s+0x%lx\n", hook->name, hook->module, hook->rva);
    return 1;
}

int Hook_InstallModule(HMODULE module, const char *moduleName, const HookDesc *hooks,
                       size_t count)
{
    int installed = 0;
    size_t i;

    for (i = 0; i < count; i++) {
        if (_stricmp(hooks[i].module, moduleName) == 0)
            installed += InstallOne(module, &hooks[i]);
    }
    return installed;
}
