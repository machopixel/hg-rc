#include "w9b.h"
#include "../unsorted_1004/w7m.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100500ab
uint8_t *__cdecl Sim_100500ab(uint8_t *entry)
{
    uint32_t index = U32(entry, 4);
    uint32_t kind;

    U32(entry, 0x94) = 0;
    U32(entry, 0x90) = 0;
    U32(entry, 0x8c) = 0;
    U32(entry, 0x88) |= 0x10;
    SIM_PTR(u32a, 0x109b8b60)[index] = 0xffffffffu;
    kind = Sim_1004ee50((uint32_t *)entry);
    U32(entry, 0x8c) |= kind;
    return entry;
}

#define TEXT(addr) SIM_PTR(const char, addr)

// FUNCTION: HGSIM 0x100572c9
char *__cdecl Joy_PickProfileName(uint8_t *dev, const uint8_t *caps)
{
    uint16_t flags = U16(caps, 0x60);
    const char *name = (const char *)dev + 0xc;
    char *out = (char *)dev + 0x4c;

    if (Crt_StrStr(name, TEXT(0x1014282c)) != 0)                                 /* "Tracker" */
        return Str_Copy(out, TEXT(0x10142834));                                   /* "tracker" */
    if (Crt_StrStr(name, TEXT(0x1014283c)) != 0 ||                               /* "SideWinder" */
        ((flags & 1) && (flags & 2) && (flags & 0x10) && (flags & 0x20) && U32(caps, 0x3c) == 8))
        return Str_Copy(out, TEXT(0x10142848));                                   /* "sidewind" */
    if (Crt_StrStr(name, TEXT(0x10142854)) != 0 ||                               /* "Flightstick Pro" */
        ((flags & 1) && (flags & 0x10) && (flags & 0x20) && U32(caps, 0x3c) == 4))
        return Str_Copy(out, TEXT(0x10142864));                                   /* "fltstick" */
    if (Crt_StrStr(name, TEXT(0x10142870)) != 0 ||                               /* "Thrustmaster" */
        ((flags & 2) && (flags & 0x10) && (flags & 0x20) && U32(caps, 0x3c) == 4))
        return Str_Copy(out, TEXT(0x10142880));                                   /* "tmaster" */
    return Str_Copy(out, TEXT(0x10142888));                                       /* "joystick" */
}
