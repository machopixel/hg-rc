#include "util.h"
#include "access.h"

// FUNCTION: HGSIM 0x1000c749
int __cdecl Sim_1000c749(void *obj, int index, unsigned mask)
{
    int result = 0;

    if (obj != 0) {
        uint8_t *entry = BYTES(U32(obj, 0x84)) + (uint32_t)index * 0x128u;
        uint8_t *rec = GBYTES(0x10145508) + U32(entry, 4) * 0xd6u;

        if ((U32(rec, 0x86) & mask) != 0)
            result = 1;
    }
    return result;
}

// FUNCTION: HGSIM 0x10011b9e
uint32_t __cdecl Sim_10011b9e(short key, const void *pairs, short count)
{
    short i;

    for (i = 0; i < count; i++) {
        if (U32(pairs, i * 8 + 4) == (uint32_t)(int)key)
            return U32(pairs, i * 8);
    }
    return 0;
}

// FUNCTION: HGSIM 0x10015c61
int __cdecl Sim_10015c61(int value)
{
    uint32_t current = PTRTAB(G32(0x1017b8c8));
    int i;

    if (current == 0)
        return -1;
    for (i = 0; i < GS32(0x1015dc64); i++) {
        uint32_t rec = PTRTAB(i);

        if (U32(rec, 8) == U32(current, 8) && U32(rec, 0x10) == (uint32_t)value)
            return i;
    }
    return -1;
}

// FUNCTION: HGSIM 0x10016cb7
int __cdecl Sim_10016cb7(int index, uint32_t *out, int flag)
{
    int mask = 6;
    uint8_t *group = GBYTES(0x109b7d40) + (uint32_t)index * 0x6cu;
    int n = 0;
    int i;

    if (flag == 0)
        mask |= 0x10;
    for (i = 0; i < S32(group, 4); i++) {
        uint32_t rec = PTRTAB(U32(group, 0x1c + i * 4));

        if ((S16(rec, 0x1c) & mask) == 0)
            out[n++] = rec;
    }
    if (U32(group, 8) != 0) {
        uint32_t rec = PTRTAB(U32(group, 0x1c + 4 * 4));

        if ((S16(rec, 0x1c) & mask) == 0)
            out[4] = rec;
    }
    return n;
}

// FUNCTION: HGSIM 0x100b36d8
uint32_t __cdecl Sim_100b36d8(uint32_t key)
{
    uint32_t result = 0;
    int i;

    for (i = GS32(0x1017d20c) - 1; i >= 0; i--) {
        if (U32(G32(0x1017d214), i * 8) == key) {
            result = U32(G32(0x1017d214), i * 8 + 4);
            break;
        }
    }
    return result;
}

// FUNCTION: HGSIM 0x100b3839
int __cdecl Sim_100b3839(uint32_t key)
{
    int result = -1;
    int i;

    for (i = 0; i < GS32(0x101c42a0); i++) {
        if (U32(G32(0x1017d1f8), i * 4) == key)
            result = (int)U32(G32(0x1017d1f4), i * 4);
    }
    return result;
}

// FUNCTION: HGSIM 0x100b38c5
int __cdecl Sim_100b38c5(int key)
{
    int result = -1;
    int i;

    if (key >= 0) {
        for (i = 0; i < GS32(0x101a1168); i++) {
            if ((int)U32(G32(0x1017d208), i * 4) == key)
                result = (int)U32(G32(0x1017d204), i * 4);
        }
    }
    return result;
}

// FUNCTION: HGSIM 0x100b399c
void *__cdecl Sim_100b399c(int index)
{
    return GBYTES(0x101a2128) + (uint32_t)index * 0x88u;
}

// FUNCTION: HGSIM 0x100c3668
int __cdecl Sim_100c3668(void *obj)
{
    int found = 0;

    if (obj != 0) {
        short i;

        for (i = 0; i < S32(obj, 0x4f8) && !found; i++) {
            uint8_t *entry = BYTES(U32(obj, 0x84)) + (uint32_t)(int)i * 0x128u;

            if (S32(entry, 8) != -1 && U32(entry, 0x20) != 0)
                found = 1;
        }
    }
    return !found && S32(obj, 0x4f8) != 0;
}

// FUNCTION: HGSIM 0x100f2cec
uint32_t __cdecl Sim_100f2cec(void *base, uint32_t value)
{
    uint32_t b = (uint32_t)(uintptr_t)base;
    /* the low 16 bits of value are cut to a byte, bits 16 and up are kept */
    uint32_t hi = ((value & 0xffff0000u) | (value & 0xffu)) << 1;
    uint16_t entry = U16(G32(0x10157590), hi);
    uint32_t pick = 2;
    uint16_t low;
    uint32_t idx;

    if (entry == 0) {
        low = (uint8_t)value;
        pick = 1;
    } else {
        low = (uint16_t)(entry + ((value & 0xff00u) >> 8)); /* 16-bit sum */
    }
    G32(0x10157594) = pick;
    idx = (hi & 0xffff0000u) | low;
    return U32(U32(b + (idx << 2) + 0x10, 0) + b, 0);
}
