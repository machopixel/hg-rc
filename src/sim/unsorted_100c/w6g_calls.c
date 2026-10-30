#include "w6g.h"
#include "w2l.h"
#include "w5c.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "../unsorted_1000/w5b.h"
#include "../unsorted_1001/w3t.h"
#include "../unsorted_1001/w5c.h"
#include "../unsorted_1001/w5y.h"

// FUNCTION: HGSIM 0x100c0423
int __cdecl Sim_100c0423(void *obj)
{
    float f;

    Sim_1001403a(obj, S16(obj, 0x1aa));
    f = Sim_100144e9(obj);
    Sim_100c0379(obj, f);
    return Sim_1000ae00(obj, Bits_OfFloat(f));
}

// FUNCTION: HGSIM 0x100c0471
int __cdecl Sim_100c0471(void *obj)
{
    int result = 0;
    float f;

    Sim_1001403a(obj, S16(obj, 0x1aa));
    f = Sim_10015266(obj);
    Sim_1000ae00(obj, Bits_OfFloat(f));
    if (W6G_CALL_100fcb70((double)f) > GFL(0x1012bb20))
        (void)Sim_100144e9(obj);
    if (FL(obj, 0xd4) > GFL(0x1012bb24) && !(f > GFL(0x1012bb20))) {
        Sim_100c4898(obj);
        U32(U32(obj, 0x54), 8) = 0;
        U8(U32(obj, 0x54), 0x26) = 1;
    } else {
        result = 1;
    }
    if (result == 1)
        Sim_100c48c9(obj);
    return result;
}

// FUNCTION: HGSIM 0x100c0ac4
uint32_t __cdecl Sim_100c0ac4(void *obj, int16_t value)
{
    uint32_t r;

    Sim_1001403a(obj, value);
    Sim_1000ae00(obj, Bits_OfFloat(Sim_100144e9(obj)));
    r = U32(U32(obj, 0x28), 0x4f4);
    U32(U32(obj, 0x54), 8) = 0x3c800000u;
    return r;
}

// FUNCTION: HGSIM 0x100c0fc4
uint32_t __cdecl Sim_100c0fc4(void *obj, int16_t value)
{
    float f;
    uint32_t block;

    Sim_1001403a(obj, value);
    if ((int8_t)U8(U32(obj, 0x54), 0x59) == 0) {
        if (W6G_CALL_100c2ae5(obj) == 0)
            f = Sim_100144e9(obj);
        else
            f = Sim_10015266(obj);
    } else {
        f = Sim_100144e9(obj);
    }
    Sim_1000ae00(obj, Bits_OfFloat(f));
    block = U32(obj, 0x54);
    U32(block, 8) = 0x3c800000u;
    return block;
}
