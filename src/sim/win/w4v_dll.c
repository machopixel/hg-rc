#include "w4v.h"
#include "../util/access.h"
#include "../../platform/force.h"
#include "../../platform/plat.h"
#include "../util/unpack.h"

/* The archive's unpackers, in the shape the pack reader calls them (Sim_100bc70d): ours instead of the DLLs' Uncompress
 * (src/sim/util/unpack.c; DECISIONS.md, "Windows calls: the decompressors"). */
static int __cdecl W4v_UnpackPkware(void *out, uint32_t *size, uint32_t packed, uint32_t packedSize)
{
    return Unpack_Pkware((uint8_t *)out, size, (const uint8_t *)(uintptr_t)packed, packedSize);
}

static int __cdecl W4v_UnpackZlib(void *out, uint32_t *size, uint32_t packed, uint32_t packedSize)
{
    return Unpack_Zlib((uint8_t *)out, size, (const uint8_t *)(uintptr_t)packed, packedSize);
}

/* a method the archive names that is neither: every entry packed with it fails to load (the original waited for its
 * DLL for ever) */
static int __cdecl W4v_UnpackUnknown(void *out, uint32_t *size, uint32_t packed, uint32_t packedSize)
{
    (void)out, (void)size, (void)packed, (void)packedSize;
    return 1;
}

/* a method name against a DLL name, letter case aside (as Windows matched the file name) */
static int W4v_SameName(const char *a, const char *b)
{
    for (; *a != 0 || *b != 0; a++, b++) {
        char x = *a >= 'A' && *a <= 'Z' ? (char)(*a + 32) : *a, y = *b >= 'A' && *b <= 'Z' ? (char)(*b + 32) : *b;

        if (x != y)
            return 0;
    }
    return 1;
}

// FUNCTION: HGSIM 0x100bcc50
int __cdecl Sim_100bcc50(void)
{
    uint32_t i;

    for (i = 1; i < G32(0x1017e1bc) + 1u; i++) {
        uint32_t entry = (i << 5) + 0x1017dfb8u;
        const char *name = (const char *)GBYTES(entry);
        void *unpack = (void *)W4v_UnpackUnknown;

        if (W4v_SameName(name, "pkware.dll"))
            unpack = (void *)W4v_UnpackPkware;
        else if (W4v_SameName(name, "zlib.dll"))
            unpack = (void *)W4v_UnpackZlib;
        G32(entry + 0x18) = 0;                  /* no library handle */
        G32(entry + 0x1c) = (uint32_t)(uintptr_t)unpack;
    }
    return 1;
}

// FUNCTION: HGSIM 0x10045e19
int __cdecl Force_ExitSystem(void)
{
    int result;

    if (G32(0x1015ef3c) == 0)
        return 0;
    if (!Force_Present())          /* FFORCE.DLL's three functions are force.h's now */
        return (int)0x8004050f;
    result = Force_Exit();
    if (result < 0)
        Plat_DebugText((const char *)GBYTES(0x1013b02c));     /* "Unable to Exit from SWForce system!" */
    return result;
}
