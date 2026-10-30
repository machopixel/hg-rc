#include "w3s.h"
#include "../util/access.h"

typedef void(__cdecl *RecordCallback)(uint32_t);

// FUNCTION: HGSIM 0x10029f50
void __cdecl Sim_10029f50(void)
{
    int i;

    for (i = 0; i < GS32(0x1015dc64); i++) {
        if (U32(PTRTAB(i), 0x30) != 0 && U32(PTRTAB(i), 0x28) != 0) {
            RecordCallback callback = (RecordCallback)(uintptr_t)U32(PTRTAB(i), 0x30);

            callback(PTRTAB(i));
        }
    }
}

// FUNCTION: HGSIM 0x1002a019
void __cdecl Sim_1002a019(void)
{
    int i;

    for (i = 0; i < GS32(0x1015dc64); i++) {
        if (U32(PTRTAB(i), 0x38) != 0 && U32(PTRTAB(i), 0x28) != 0) {
            RecordCallback callback = (RecordCallback)(uintptr_t)U32(PTRTAB(i), 0x38);

            callback(U32(PTRTAB(i), 0x28));
        }
    }
}

// FUNCTION: HGSIM 0x1002a086
void __cdecl Sim_1002a086(void)
{
    uint32_t rec = PTRTAB(GS32(0x1017b8c8));

    if (rec != 0) {
        RecordCallback callback = (RecordCallback)(uintptr_t)U32(rec, 0x3c);

        if (callback != 0 && U32(rec, 0x28) != 0)
            callback(U32(rec, 0x28));
    }
}

// FUNCTION: HGSIM 0x1002a10e
void __cdecl Sim_1002a10e(void)
{
    int i;

    for (i = 0; i < GS32(0x1015dc64); i++) {
        if (U32(PTRTAB(i), 0x44) != 0) {
            RecordCallback callback = (RecordCallback)(uintptr_t)U32(PTRTAB(i), 0x44);

            callback(U32(PTRTAB(i), 0x28));
        }
    }
}
