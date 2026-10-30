/*
 * Points one entry of a module's import table at another function, so calls the
 * module makes to a DLL function go to ours instead.
 */
#include <string.h>

#include "hook.h"

int Hook_PatchImport(HMODULE module, const char *dllName, const char *functionName,
                     void *replacement)
{
    unsigned char *base = (unsigned char *)module;
    const IMAGE_DOS_HEADER *dos = (const IMAGE_DOS_HEADER *)base;
    const IMAGE_NT_HEADERS *nt = (const IMAGE_NT_HEADERS *)(base + dos->e_lfanew);
    const IMAGE_DATA_DIRECTORY *dir =
        &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    const IMAGE_IMPORT_DESCRIPTOR *imp;

    if (dir->VirtualAddress == 0)
        return 0;
    for (imp = (const IMAGE_IMPORT_DESCRIPTOR *)(base + dir->VirtualAddress); imp->Name != 0;
         imp++) {
        const IMAGE_THUNK_DATA *names;
        IMAGE_THUNK_DATA *slots;

        if (_stricmp((const char *)(base + imp->Name), dllName) != 0)
            continue;
        names = (const IMAGE_THUNK_DATA *)(base + (imp->OriginalFirstThunk != 0
                                                       ? imp->OriginalFirstThunk
                                                       : imp->FirstThunk));
        slots = (IMAGE_THUNK_DATA *)(base + imp->FirstThunk);
        for (; names->u1.AddressOfData != 0; names++, slots++) {
            const IMAGE_IMPORT_BY_NAME *by;
            DWORD old;

            if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal))
                continue;
            by = (const IMAGE_IMPORT_BY_NAME *)(base + names->u1.AddressOfData);
            if (strcmp((const char *)by->Name, functionName) != 0)
                continue;
            if (!VirtualProtect(&slots->u1.Function, sizeof(slots->u1.Function),
                                PAGE_READWRITE, &old))
                return 0;
            slots->u1.Function = (DWORD_PTR)replacement;
            VirtualProtect(&slots->u1.Function, sizeof(slots->u1.Function), old, &old);
            return 1;
        }
    }
    return 0;
}
