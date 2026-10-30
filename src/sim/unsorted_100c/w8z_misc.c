#include "w8z.h"
#include "w2f.h"
#include "w5x.h"
#include "w6h.h"
#include "../unsorted_1000/w3o.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w7q.h"
#include "../unsorted_1010/w6k.h"
#include "../util/util.h"

// FUNCTION: HGSIM 0x100c6cf5
void __cdecl Sim_100c6cf5(void *idx)
{
    uint32_t gnu;
    uint32_t entry;
    int32_t pad;

    for (;;) {
        if (W8Z_CALL_100fe830((void *)(uintptr_t)G32(0x10151ce4), 1, 0x200, (void *)(uintptr_t)U32(idx, 0)) !=
            0x200)
            return;
        Tar_ReadHeader((const char *)(uintptr_t)G32(0x10151ce4), SIM_PTR(uint8_t, 0x1017e4d0), &gnu);
        if (G32(0x1017e4ec) == 0)
            return;
        if (S32(idx, 0x4b4) >= 100)
            return;
        entry = (uint32_t)(uintptr_t)idx + U32(idx, 0x4b4) * 0xcu + 4;
        U32(idx, 0x4b4) += 1;
        U32(entry, 8) = G32(0x1017e4ec);
        U32(entry, 4) = (uint32_t)Crt_FTell((void *)(uintptr_t)U32(idx, 0));
        U32(entry, 0) = (uint32_t)(uintptr_t)Sim_1010c9f0((const char *)(uintptr_t)G32(0x10151ce4));
        if (U32(entry, 0) == 0)
            return;
        W8Z_CALL_100feb20((void *)(uintptr_t)U32(idx, 0), GS32(0x1017e4ec), 1);
        pad = 0x200 - GS32(0x1017e4ec) % 512;
        if (pad == 0x200)
            pad = 0;
        W8Z_CALL_100feb20((void *)(uintptr_t)U32(idx, 0), pad, 1);
    }
}

// FUNCTION: HGSIM 0x100c7deb
void __cdecl Sim_100c7deb(void *obj)
{
    uint32_t block;
    uint32_t unit;
    uint32_t kind;
    uint32_t index;
    uint32_t rec;
    int32_t x, y;
    char label[0x24];
    uint32_t view[8];
    uint8_t saved[0x68];
    uint32_t px, py, pz;
    uint32_t aim;

    if (S16(obj, 4) == 0 || G32(0x10151cf8) == 0)
        return;
    U32(obj, 0xf8) = G32(0x109e89d8);
    if (S16(obj, 6) == 1) {
        if (G32(0x1017e590) != 0) {
            if (Sim_1009c929(10) < 7)
                G32(0x1017e590) = 0;
            Sim_100c81b5(obj);
            return;
        }
        if (Sim_1009c929(10) < 3)
            G32(0x1017e590) = 1;
    } else if (S16(obj, 6) > 2) {
        Sim_100c81b5(obj);
        return;
    }
    block = U32(PTRTAB(G32(0x1017b8c8)), 0x28);
    unit = U32(block, 0);
    kind = U32(unit, 0xf0) & 0xf00;
    index = U32(unit, 0xf0) & 0xff;
    if (kind == 0 || (U32(unit, 0xf0) & 0x1000) != 0)
        return;
    if (kind == 0x100) {
        rec = U32(obj, 0xec);
        x = (int32_t)(U32(rec, 0xc) - U32(rec, 4)) / 4;
        y = (int32_t)(U32(rec, 0x10) - U32(rec, 8)) / 4;
        if ((S16((uintptr_t)SIM_PTR(uint8_t, 0x109a8664) + index * 0x54u, 0) & 0x20) == 0)
            Str_Copy(label, (const char *)SIM_PTR(char, 0x10151d0c));
        else
            Str_Copy(label, (const char *)SIM_PTR(char, 0x10151d14));
        W8Z_CALL_100652e8(x, y, label, (void *)(uintptr_t)U32(obj, 0xec));
        return;
    }
    if (kind == 0x400 &&
        (S16((uintptr_t)SIM_PTR(uint8_t, 0x109e9ba0) + (U32(U32(block, 0), 0xf0) & 0xff) * 0x50u, 0) & 4) != 0)
        return;
    Sim_10044992((const uint32_t *)(uintptr_t)G32(0x1014cae0), view);
    Sim_100787f4(saved);
    px = U32(unit, 0xdc);
    py = U32(unit, 0xe0);
    pz = U32(unit, 0xe4);
    aim = Sim_100cba68();
    if (kind == 0x400) {
        if (aim != 0)
            Sim_1000946e((void *)(uintptr_t)U32(aim, 0x8c), &px, &py, &pz);
    } else {
        /* the original works out a float from this field and never uses it; the read is kept */
        (void)*(volatile u32a *)(BYTES(U32(PTRTAB(U32(unit, 0xf0) & 0xff), 0x28)) + 0x53c);
    }
    if (G32(0x10151cf8) == 1) {
        G32(0x1014caec) = 0;
        G32(0x1014cb24) = 0;
        G32(0x1014cb1c) = 4;
        G32(0x1014cb20) = 0;
    }
    G32(0x1014cb08) = 0;
    G32(0x1014cb04) = G32(0x1014cb08);
    if (G32(0x109e89d8) == 2)
        W8Z_CALL_100619a0(0, U32(G32(0x1014cae0), 0x18), view, aim, 0);
    Mem_Move(SIM_PTR(uint8_t, 0x1014cae8), saved, 0x68);
}

// FUNCTION: HGSIM 0x100cfd28
void __cdecl Sim_100cfd28(void *obj)
{
    uint32_t res;
    uint32_t a, b, c;

    if (S16(obj, 4) == 0 || G32(0x10179cc4) == 0)
        return;
    ((W8zCallbackFn)(uintptr_t)U32(obj, 0x114))(obj, G32(0x101c4c54));
    res = W8Z_CALL_100ba7d5((const char *)(uintptr_t)G32(0x109b9900), (const char *)(uintptr_t)G32(0x1014cec4), 0);
    if (res == 0)
        return;
    c = U32(U32(obj, 0xf0), 4);
    b = U32(U32(obj, 0xf0), 0);
    a = U32(obj, 0xec);
    if (G32(0x101668e8) != 0)
        W8Z_CALL_100f2f01(a, b, c, res, BYTES(obj) + 0x10, SIM_PTR(uint8_t, 0x109b9920));
    else if (G32(0x1017b940) != 0)
        W8Z_CALL_100f1e10(a, b, c, res, BYTES(obj) + 0x10, SIM_PTR(uint8_t, 0x109b9920));
    else
        W8Z_CALL_100ecea6(a, b, c, res, BYTES(obj) + 0x10, SIM_PTR(uint8_t, 0x109b9920));
    Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109b9900), (const char *)(uintptr_t)G32(0x1014cec4));
}
