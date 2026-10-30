#include "w5c.h"
#include "w3t.h"
#include "../util/access.h"

typedef float HG_MAY_ALIAS w5c_f32;
#define W5C_GF32(addr) (*SIM_PTR(w5c_f32, addr))

// FUNCTION: HGSIM 0x100144e9
float __cdecl Sim_100144e9(const void *obj)
{
    float value = Sim_10015266(obj);
    double r = SIM_FMOD((double)value, 360.0);
    uint8_t *record = BYTES(U32(obj, 0x54));

    if (r > (double)W5C_GF32(0x10111140)) {
        U32(record, 0xc) = 0x444ccccd;
    } else if (r >= (double)W5C_GF32(0x10111154)) {
        float scaled = (float)((double)(float)r / (double)W5C_GF32(0x10111140));

        *(w5c_f32 *)(record + 0xc) = (float)((double)W5C_GF32(0x10111158) * (double)scaled);
    } else {
        U32(record, 0xc) = 0xc44ccccd;
    }
    return value;
}
