#include "w23.h"
#include "w4u.h"
#include "../win/w4w.h"
#include "../util/access.h"

#define W23A_REC BYTES(G32(0x1015d1b8))

// FUNCTION: HGSIM 0x1001efbe
void __cdecl Sim_1001efbe(int32_t which, uint32_t value)
{
    switch (which) {
    case 0:
        G32(0x101321b4) = value;
        U32(W23A_REC, 4) = G32(0x101321b4);
        return;
    case 1:
        G32(0x101321b8) = value;
        U32(W23A_REC, 8) = G32(0x101321b8);
        if (value != 0)
            G32(0x101321c0) |= 2;
        else
            G32(0x101321c0) &= ~2u;
        return;
    case 2:
        G32(0x101321bc) = value;
        U32(W23A_REC, 0xc) = G32(0x101321bc);
        if (value != 0) {
            U32(W23A_REC, 0x10) |= 8;
            U32(W23A_REC, 0x10) |= 4;
            if (G32(0x1015ec58) == 0)
                Sim_1001f335();
        } else {
            U32(W23A_REC, 0x10) &= ~8u;
            U32(W23A_REC, 0x10) &= ~4u;
            Sim_1001f399();
        }
        G32(0x101321c0) = U32(W23A_REC, 0x10);
        return;
    case 3:
        G32(0x101321b0) = value;
        U32(W23A_REC, 0) = G32(0x101321b0);
        return;
    case 4:
#ifdef W23A_MUTATE /* the checker's negative check only */
        if (value != 0)
            U32(W23A_REC, 0x10) |= 4;
#else
        if (value != 0)
            U32(W23A_REC, 0x10) |= 2;
#endif
        else
            U32(W23A_REC, 0x10) &= ~2u;
        G32(0x101321c0) = U32(W23A_REC, 0x10);
        return;
    default:
        return;
    }
}

// FUNCTION: HGSIM 0x1001f14f
void __cdecl Sim_1001f14f(int32_t which)
{
    switch (which) {
    case 0:
        G32(0x101321b4) = U32(W23A_REC, 4);
        return;
    case 1:
        G32(0x101321b8) = U32(W23A_REC, 8);
        if (G32(0x101321b8) != 0)
            G32(0x101321c0) |= 2;
        else
            G32(0x101321c0) &= ~2u;
        return;
    case 2:
        G32(0x101321bc) = U32(W23A_REC, 0xc);
        if (G32(0x101321bc) != 0) {
            G32(0x101321c0) |= 8;
            G32(0x101321c0) |= 4;
            Sim_1009ebeb();
            if (G32(0x1015ec58) == 0)
                Sim_1001f335();
        } else {
            G32(0x101321c0) &= ~8u;
            G32(0x101321c0) &= ~4u;
            Sim_1001f399();
        }
        return;
    case 3:
#ifdef W23A_MUTATE /* the checker's negative check only */
        G32(0x101321b0) = U32(W23A_REC, 4);
#else
        G32(0x101321b0) = U32(W23A_REC, 0);
#endif
        return;
    case 4:
        G32(0x101321c0) = U32(W23A_REC, 0x10);
        return;
    default:
        return;
    }
}
