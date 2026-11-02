#include "w11.h"
#include "../../platform/picture.h"
#include "../simmem.h"
#include "../util/access.h"
#include "w2i.h"
#include "../util/util.h"

#define W11_DDERR_SURFACELOST 0x887601c2u

typedef uint32_t(__stdcall *W11SurfMethod0Fn)(uint32_t self);
typedef uint32_t(__stdcall *W11SurfMethod1Fn)(uint32_t self, uint32_t a);
typedef uint32_t(__stdcall *W11SurfMethod2Fn)(uint32_t self, uint32_t a, uint32_t b);

/* method at byte offset off of the COM object whose pointer is the dword at address slot */
#define W11_SURF_METHOD(type, slot, off) ((type)(uintptr_t)U32(BYTES(U32(BYTES(G32(slot)), 0)), (off)))
#define W11_RESULT G32(0x10372b94)

typedef void *(__cdecl *W11MemcpyFn)(void *dst, const void *src, uint32_t n);

/* the window blit of the whole picture, starting at scan line start, placed at y: the original's
   SetDIBitsToDevice, now the loading-screen picture (platform/picture.h), the rows from start on drawn from y down */
static uint32_t W11_Blit(int y, uint32_t start)
{
    return (uint32_t)Pic_Blit(BYTES(G32(0x1015e9a0)), GS32(0x109e3d14), GS32(0x109e3d18), 0, y, 0, (int)start,
                              GS32(0x109e3d14), GS32(0x109e3d18));
}

/* the jitter: each row of the effect's area shifted right by a running offset clamped to 0..width-1 */
static void W11_Jitter(uint32_t fxAddr, int32_t *offset)
{
    uint32_t base = G32(0x1015e9a0);
    int32_t i;

    for (i = 0; i < S32(BYTES(G32(fxAddr)), 0xc); i++) {
        const uint8_t *fx;
        uint32_t row;
        int32_t r = Sim_1009c929(S32(BYTES(G32(fxAddr)), 0x14));

        fx = BYTES(G32(fxAddr));
        *offset += r - (S32(fx, 0x14) >> 1);
        if (*offset < 0)
            *offset = 0;
        else if (*offset > S32(fx, 8) - 1)
            *offset = S32(fx, 8) - 1;
        row = base + U32(fx, 0) + (uint32_t)(i + S32(fx, 4)) * 0x280;
        SIM_FN(W11MemcpyFn, 0x100fcdc0)((void *)(uintptr_t)(row + (uint32_t)*offset), (const void *)(uintptr_t)row,
                                        U32(fx, 8) - (uint32_t)*offset);
    }
}

// FUNCTION: HGSIM 0x10083801
void __cdecl Sim_10083801(void)
{
    int32_t offset = 0;
    int on1 = 0, on2 = 0;

    if (G32(0x10179ca0) != 0)
        on1 = Sim_1009c929(100) < S32(BYTES(G32(0x10179ca0)), 0x10);
    if (G32(0x10179ca4) != 0)
        on2 = Sim_1009c929(100) < S32(BYTES(G32(0x10179ca4)), 0x10);
    if (G32(0x10179ca0) != 0 && (on1 || G32(0x1017b7e0) != 0)) {
        if ((Sim_1009c929(100) < S32(BYTES(G32(0x10179ca0)), 0x18) && on1) || G32(0x1017b7e0) != 0) {
            if (G32(0x1017b7e0) == 0)
                G32(0x1017b7dc) = 0;
            G32(0x103729c4) = W11_Blit(0, G32(0x1017b7dc));
            G32(0x103729c4) = W11_Blit(0x1ea - GS32(0x1017b7dc), 0);
            G32(0x1017b7dc) = (uint32_t)((GS32(0x1017b7dc) + 0x10) % S32(BYTES(G32(0x10179ca0)), 0xc));
            G32(0x1017b7e0) = Sim_1009c929(100) < S32(BYTES(G32(0x10179ca0)), 0x1c) ? 0 : 1;
        } else {
            W11_Jitter(0x10179ca0, &offset);
            G32(0x103729c4) = W11_Blit(0, 0);
        }
    } else if (G32(0x10179ca4) != 0 && on2 && G32(0x10144fa0) != 0) {
        W11_Jitter(0x10179ca4, &offset);
        G32(0x103729c4) = W11_Blit(0, 0);
    } else {
        G32(0x103729c4) = W11_Blit(0, 0);
    }
}
