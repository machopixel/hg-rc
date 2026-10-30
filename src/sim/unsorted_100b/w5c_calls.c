#include "w5c.h"
#include "w2e.h"
#include "../win/w4v.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100ba5d1
void __cdecl Sim_100ba5d1(void)
{
    if (G32(0x1017d604) != 0) {
        SIM_CALL_100bd69c(G32(0x1017d604), SIM_PTR(uint8_t, 0x100ba68e));
        G32(0x1017d604) = 0;
    }
    if (G32(0x1017d600) != 0) {
        SIM_CALL_100bd69c(G32(0x1017d600), SIM_PTR(uint8_t, 0x100ba631));
        G32(0x1017d600) = 0;
    }
    if (G32(0x1017d5fc) != 0)
        SIM_CALL_100bd134();
}

// FUNCTION: HGSIM 0x100bcbcf
int __cdecl Sim_100bcbcf(char *path, uint32_t value, int byte)
{
    *GBYTES(0x1017e1b8) = (uint8_t)byte;
    G32(0x1017e1cc) = value;
    Path_ReplaceExtension(path, SIM_PTR(char, 0x10151554));
    if (SIM_CALL_100bcd61(path) == 0)
        return 0;
    Path_ReplaceExtension(path, SIM_PTR(char, 0x10151558));
    G32(0x1017e1c0) = SIM_CALL_100fd850(path, SIM_PTR(char, 0x1015155c));
    if (G32(0x1017e1c0) == 0)
        return 0;
    if (Sim_100bcc50() == 0)
        return 0;
    return 1;
}

// FUNCTION: HGSIM 0x100bee33
uint32_t __cdecl Sim_100bee33(void *obj)
{
    uint16_t word = 0xffff;
    uint32_t top = (uint32_t)(uintptr_t)obj;

    if (S32(obj, 0x1fc) != -1) {
        if (S32(obj, 0x1fc) == -2) {
            top = SIM_CALL_10015630(obj, 4, (int16_t)U16(obj, 0x1aa), 0);
            U32(obj, 0x1ae) = 1;
        } else {
            word = U16(obj, 0x1fc);
        }
        S32(obj, 0x1fc) = -1;
    }
    return (top & 0xffff0000u) | word;
}
