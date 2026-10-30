#include "w5c.h"
#include "w3t.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_1004/w4u.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1001555c
void __cdecl Sim_1001555c(void *obj, int16_t a, int16_t b)
{
    if (U16(obj, 0x198) == 0) {
        SIM_CALL_10015630(obj, a, b, 0);
    } else {
        U32(obj, 0x180 + 4u * U16(obj, 0x198)) = (uint32_t)(uint16_t)a | ((uint32_t)(uint16_t)b << 16);
    }
}

// FUNCTION: HGSIM 0x100159cf
void __cdecl Sim_100159cf(void *obj, int16_t value)
{
    int has_bit8 = 0;

    if ((value & 0x100) != 0)
        has_bit8 = 1;
    S32(obj, 0xf0) = value;
    SIM_CALL_100cbdb8(obj, 1, 1);
    if (S32(obj, 0xf0) == S32(obj, 0x1d6))
        SIM_CALL_100cbdb8(obj, 1, 1);
    if (has_bit8 == 1)
        U16(obj, 0x1a8) = U16(obj, 0x118);
    else
        U16(obj, 0x1a8) = U16(obj, 0xf0);
}

// FUNCTION: HGSIM 0x1001bb1f
void __cdecl Sim_1001bb1f(const char *name, int id, int reload)
{
    uint8_t *rec;
    void *data;

    if (name == 0)
        return;
    rec = SIM_CALL_1001b902(id);
    if (rec == 0 || U32(rec, 4) == 1)
        return;
    data = Sim_1001b880(name);
    if (data == 0)
        return;
    U32(rec, 4) = 1;
    U32(rec, 0) = (uint32_t)(uintptr_t)data;
    Str_Copy((char *)rec + 8, name);
    if (reload != 0)
        SIM_CALL_1001cedf(id);
}

// FUNCTION: HGSIM 0x1001dcff
void __cdecl Sim_1001dcff(void)
{
    int i;

    for (i = 0; i < 0x80; i++) {
        if (G32(0x10a1eaa0 + (uint32_t)i * 0x320) != 0) {
            SIM_CALL_100bb2b0(GBYTES(0x10a1eaa4 + (uint32_t)i * 0x320), SIM_PTR(char, 0x10131fd0));
            G32(0x10a1eaa0 + (uint32_t)i * 0x320) = 0;
        }
    }
}

// FUNCTION: HGSIM 0x1001ffc8
uint32_t __cdecl Sim_1001ffc8(void *obj)
{
    uint32_t old = U32(obj, 0x67);

    U32(obj, 0) = 0;
    U32(obj, 0x67) = 0;
    if (U32(obj, 0x24) != 0)
        Heap_Free_100461d0((void *)(uintptr_t)U32(obj, 0x24));
    if (U32(obj, 0x28) != 0)
        SIM_CALL_100bc280(0x20, SIM_PTR(char, 0x10134258));
    return old;
}
