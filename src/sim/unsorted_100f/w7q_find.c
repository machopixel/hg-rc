#include <string.h>
#include "w7q.h"
#include "w6i.h"
#include "../util/access.h"
#include "../../platform/plat.h"



/* Maps the Windows error of a failed search to the error dword. */
static void W7q_FindError(void)
{
    uint32_t err = Plat_FileLastError();     /* C runtime replacement, files */

    if (err == 2 || err == 3 || err == 18)
        G32(0x1017e888) = 2;
    else if (err == 8)
        G32(0x1017e888) = 0xc;
    else
        G32(0x1017e888) = 0x16;
}

/*
 * Fills the result from WIN32_FIND_DATAA: attributes (0 for 0x80, normal file), the three file times, the low
 * size dword and the name.
 */
static void W7q_FindResult(const uint32_t *data, uint32_t *out)
{
    out[0] = data[0] == 0x80 ? 0 : data[0];
    out[1] = (uint32_t)Time_FromFileTime(data + 1);
    out[2] = (uint32_t)Time_FromFileTime(data + 3);
    out[3] = (uint32_t)Time_FromFileTime(data + 5);
    out[4] = data[8];
    memcpy(out + 5, data + 11, strlen((const char *)(data + 11)) + 1);
}

// FUNCTION: HGSIM 0x100fe470
int32_t __cdecl Crt_FindFirst(const char *spec, uint32_t *out)
{
    uint32_t data[0x50];
    int32_t handle = (int32_t)Plat_FileFindFirst(spec, data);

    if (handle == -1) {
        W7q_FindError();
        return -1;
    }
    W7q_FindResult(data, out);
    return handle;
}

// FUNCTION: HGSIM 0x100fe5a0
int __cdecl Crt_FindNext(uint32_t handle, uint32_t *out)
{
    uint32_t data[0x50];

    if (Plat_FileFindNext(handle, data) == 0) {
        W7q_FindError();
        return -1;
    }
    W7q_FindResult(data, out);
    return 0;
}
