#include "w8y.h"
#include "w2e.h"
#include "w2l.h"
#include "w3o.h"
#include "w5a.h"
#include "w7o.h"
#include "../util/access.h"
#include "../render/w3r.h"
#include "../unsorted_1002/w4u.h"
#include "../unsorted_1004/w5c.h"
#include "../unsorted_100c/w2h.h"
#include "../unsorted_100c/w5y.h"
#include "../unsorted_100c/w7p.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_1010/w6k.h"

/* Archive slot `index` of the table 0x101c65e0 (0x14a bytes each). */
#define SLOT(index) (BYTES(SIM_PTR(uint8_t, 0x101c65e0)) + (uint32_t)(index) * 0x14au)
/* The loaded item table of directory entry e (+0x2a + e * 8), its items start 0x16 bytes in. */
#define ITEMS(index, e) (U32(SLOT(index), 0x2a + (uint32_t)(e) * 8u) + 0x16u)
/* The directory word +0x14 of entry e (entries of 0x18 bytes from the directory block's +0xe). */
#define DIRWORD(index, e) U16(U32(SLOT(index), 6) + 0xeu, (uint32_t)(e) * 0x18u + 0x14u)

// FUNCTION: HGSIM 0x100a76a0
int __cdecl Sim_100a76a0(void)
{
    int result = 1;

    Sim_100a78c3((uint32_t)(uintptr_t)SIM_PTR(void, 0x100a77ef), (uint32_t)(uintptr_t)SIM_PTR(void, 0x100a7800));
    Sim_10027f06();
    if (G32(0x1017d154) == 0)
        G32(0x1017d154) = (uint32_t)(uintptr_t)Sim_1010c9f0(Sim_100474db(SIM_PTR(const char, 0x1014d0f8)));
    G32(0x1014cf8c) = (uint32_t)Sim_100a79db((const char *)(uintptr_t)G32(0x1017d154), 0);
    if (G32(0x1014cf8c) != 0xffffffffu) {
        Sim_100a7e55((int32_t)G32(0x1014cf8c));
    } else {
        W8Y_CALL_100bc280(3, SIM_PTR(const char, 0x1014d104), G32(0x1017d154), 0);
        result = 0;
    }
    return result;
}

// FUNCTION: HGSIM 0x100a79db
int __cdecl Sim_100a79db(const char *name, int mode)
{
    uint8_t header[0xc];
    uint32_t size;
    uint32_t handle;
    int block;
    uint16_t slot;

    for (slot = 0; slot < 1; slot++)
        if (U16(SLOT(slot), 4) == 0)
            break;
    if (slot >= 1)
        return -1;
    if ((int8_t)mode == 2) {
        handle = Sim_10109980((uint32_t)(uintptr_t)name, 0x8002, 0);
        if (handle == 0xffffffffu)
            return -1;
        W8Y_CALL_10105b50(handle, header, 0xc);
        if (Sim_1010d240(header, SIM_PTR(const uint8_t, 0x1014d194), 4) != 0)
            return -1;
        if (header[8] == 0xfe) {
            W8Y_CALL_101049c0(handle);
            return -2;
        }
    } else if ((int8_t)mode == 0) {
        handle = Sim_10109980((uint32_t)(uintptr_t)name, 0x8000, 0);
        if (handle == 0xffffffffu)
            return -1;
        W8Y_CALL_10105b50(handle, header, 0xc);
        if (Sim_1010d240(header, SIM_PTR(const uint8_t, 0x1014d19c), 4) != 0)
            return -1;
    } else {
        return -1;
    }
    W8Y_CALL_10105fe0(handle, 0x10, 0);
    if (File_ReadChunked(handle, (uint32_t)(uintptr_t)&size, 4) != 4)
        return -1;
    size += 8;
    block = Sim_100a78d9((int)size);
    if (block == 0)
        return -1;
    W8Y_CALL_10105fe0(handle, 0xc, 0);
    if ((uint32_t)File_ReadChunked(handle, (uint32_t)block, size) != size) {
        Sim_100a78eb((uint32_t)block);
        return -1;
    }
    U32(SLOT(slot), 0) = handle;
    U32(SLOT(slot), 6) = (uint32_t)block;
    U16(SLOT(slot), 4) = 1;
    Crt_StrNSet((char *)SLOT(slot) + 0x12a, 0, 0x20);
    Str_CopyN((char *)SLOT(slot) + 0x12a, name, 0x20);
    return slot;
}

// FUNCTION: HGSIM 0x100a80d2
int __cdecl Sim_100a80d2(int index, const char *key, uint32_t sub, uint32_t *outPos, uint32_t *outEnd)
{
    uint32_t e = (uint16_t)Sim_100a7da3(index, key);
    uint32_t pos;

    if (e == 0xffffffffu)       /* never true: the index was zero-extended (kept from the original) */
        return -1;
    if (U32(SLOT(index), 0x2a + e * 8) == 0 && Sim_100a7e55(index) == -1)
        return -1;
    pos = U32(ITEMS(index, e), (sub & 0xffff) * 8) + DIRWORD(index, e);
    if (outPos != 0)
        *outPos = pos;
    if (outEnd != 0)
        *outEnd = U32(ITEMS(index, e), (sub & 0xffff) * 8 + 4) - DIRWORD(index, e);
    W8Y_CALL_10105fe0(U32(SLOT(index), 0), pos, 0);
    return 0;
}

// FUNCTION: HGSIM 0x100a8238
int __cdecl Sim_100a8238(int index, const char *key, uint32_t sub, void *buffer)
{
    int16_t e = (int16_t)Sim_100a7da3(index, key);
    uint32_t pos;
    uint32_t len;
    uint32_t s;

    if (e == -1)
        return -1;
    if (U32(SLOT(index), 0x2a + (int32_t)e * 8) == 0 && Sim_100a7e55(index) == -1)
        return -1;
    s = sub & 0xffff;
    pos = U32(ITEMS(index, (int32_t)e), s * 8) + DIRWORD(index, (int32_t)e);
    len = U32(ITEMS(index, (int32_t)e), s * 8 + 4) - DIRWORD(index, (int32_t)e);
    if (W8Y_CALL_10105fe0(U32(SLOT(index), 0), pos, 0) == -1)
        return -1;
    if ((uint32_t)File_ReadChunked(U32(SLOT(index), 0), (uint32_t)(uintptr_t)buffer, len) != len)
        return -1;
    return 0;
}

// FUNCTION: HGSIM 0x100a836e
int __cdecl Sim_100a836e(int index, const char *key, uint32_t sub, void *buffer)
{
    int n;
    int got;

    if ((sub & 0xffff) < 0xf000)
        return Sim_100a8238(index, key, sub, buffer);
    n = Sim_100c6f39((void *)(uintptr_t)G32(0x1017d158), (int)((sub & 0xffff) - 0xf000));
    if (n == -1)
        return -1;
    got = Sim_100c6fbd((void *)(uintptr_t)G32(0x1017d158), buffer, (uint32_t)n);
    Sim_100c6fb8();
    if (got != n)
        return -1;
    return 0;
}

// FUNCTION: HGSIM 0x100a8405
uint32_t __cdecl Sim_100a8405(int index, const char *key, uint32_t sub, uint32_t *outHandle)
{
    int16_t e = (int16_t)Sim_100a7da3(index, key);
    uint32_t pos;

    if (e == -1)
        return 0;
    if (U32(SLOT(index), 0x2a + (int32_t)e * 8) == 0 && Sim_100a7e55(index) == -1)
        return 0;
    pos = U32(ITEMS(index, (int32_t)e), (sub & 0xffff) * 8) + DIRWORD(index, (int32_t)e);
    *outHandle = U32(SLOT(index), 0);
    return pos;
}
