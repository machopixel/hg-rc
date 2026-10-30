#include "w8w.h"
#include "w2f.h"
#include "../unsorted_1000/w5x.h"
#include "../unsorted_1003/w7m.h"
#include "../unsorted_100f/w2e.h"
#include "../util/util.h"
#include "../util/w3o.h"

void(__cdecl *g_simDestroyObserver)(uint32_t killer, const uint8_t *s, int32_t code, const uint8_t *shot);
const uint8_t *g_simDestroyShot;

// FUNCTION: HGSIM 0x1006abf6
void __cdecl Sim_1006abf6(uint32_t a, uint8_t *s, int32_t code)
{
    uint32_t block = U32(U32(s, 0), 0x48);
    uint32_t obj, camera;
    int digit;
    char name[12];

    if (g_simDestroyObserver != 0)
        g_simDestroyObserver(a, s, code, g_simDestroyShot);
    if (U32(U32(s, 0), 0) == 1 && U32(s, 0xbdb) != 5 && (code == 2 || code == 3) && U32(block, 4) != 0 &&
        U32(U32(block, 4), 4) != 0) {
        Sim_100386d5((uint32_t *)s, -1, 0);
        if (code == 3) {
            Sim_1006ab0c(s);
            U32(s, 0xc45) = 1;
        }
    }
    if (code == 0xa || code == 9 || code == 0xb || code == 0xd || code == 0xe || code == 0xf || code == 0x10)
        W8W6_CALL_1006bf33(a, code, s);
    if (U32(U32(s, 0), 0) == 9) {
        U16(s, 0x560) |= 0x400;
        U32(U32(s, 0), 0x150) |= a & 0xffff;
        digit = Sim_1009c929(3);
        Str_Copy(name, (const char *)GBYTES(0x10145314));
        name[7] = (char)(digit + 0x31);
        obj = U32(s, 0);
        camera = G32(0x1014cae0);
        Sim_1000ed52(Bits_OfFloat((float)((double)FL(obj, 0x58) - (double)FL(camera, 0))),
                     Bits_OfFloat((float)((double)FL(obj, 0x5c) - (double)FL(camera, 4))),
                     Bits_OfFloat((float)((double)FL(obj, 0x60) - (double)FL(camera, 8))),
                     (uint32_t)(uintptr_t)name, G32(0x1015ef0c), U32(s, 0));
    } else {
        W8W6_CALL_1006ad98(a, s, code, 0);
    }
}
