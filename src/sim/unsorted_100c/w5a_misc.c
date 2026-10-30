#include "w5a.h"
#include "w3o.h"
#include "../unsorted_1001/w3t.h"
#include "../util/util.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x100bf9a1
void __cdecl Sim_100bf9a1(void *obj)
{
    Sim_100c274a(obj, 0);
    U16(obj, 0x1fa) = U16(obj, 0x1f8);
    U16(obj, 0x1f8) = 0xffff;
    U32(obj, 0x200) = 0;
    U32(obj, 0x204) = 0;
    U32(obj, 0x218) = 0;
    U32(obj, 0x21c) = 0;
    U16(obj, 0x1a8) = U16(obj, 0x1aa);
    W5A_CALL_1001403a(obj, U16(obj, 0x1aa));
}

// FUNCTION: HGSIM 0x100c1853
int __cdecl Sim_100c1853(void *obj)
{
    int result;
    int code;

    if (Sim_100c476c(obj) == 0) {
        result = 1;
        code = 0x16;
    } else if (W5A_CALL_100c4d63(obj) != 0) {
        result = -1;
        code = 0x17;
    } else {
        result = 0;
        code = -1;
    }
    S32(obj, 0x1fc) = code;
    return result;
}

// FUNCTION: HGSIM 0x100c34b7
float __cdecl Sim_100c34b7(void *obj, int16_t key)
{
    uint32_t old = U32(obj, 0xf0);
    float result;

    W5A_CALL_1001403a(obj, (uint16_t)key);
    result = (float)W5A_FMOD((double)Sim_10015266(obj) + GF64(0x1012bbb0), 360.0);
    W5A_CALL_1001403a(obj, (uint16_t)old);
    return result;
}

// FUNCTION: HGSIM 0x100c4da3
int __cdecl Sim_100c4da3(const void *obj)
{
    uint32_t out = 0;
    int count;

    if (obj == 0)
        return -1;
    count = W5A_CALL_10033eb3(U32(obj, 0x58), U32(obj, 0x5c), U32(obj, 0x60), 0x46dac000u, &out);
    if (count != 0 && out != 0)
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x100c6eae
int __cdecl Entry_FindByName(const void *obj, const char *name)
{
    int i;

    for (i = 0; i < S32(obj, 0x4b4); i++) {
        if (W5A_STRICMP(name, (const char *)(uintptr_t)U32(obj, i * 12 + 4)) == 0)
            break;
    }
    if (i == S32(obj, 0x4b4))
        return -1;
    return i;
}
