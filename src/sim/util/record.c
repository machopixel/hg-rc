#include "util.h"
#include "access.h"

// FUNCTION: HGSIM 0x1000711c
void __cdecl Sim_1000711c(short index, unsigned short value)
{
    uint8_t *rec = GBYTES(0x10ac1b00) + (uint32_t)(int)index * 0xeu;

    if (S16(rec, 8) != -2)
        U16(rec, 6) = value;
    if (U16(rec, 4) == 0)
        U16(rec, 4) = 0x2d;
}

// FUNCTION: HGSIM 0x10009136
void __cdecl Sim_10009136(void *obj, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4,
                          uint32_t a5, uint32_t a6, uint32_t a7, uint32_t a8, uint32_t a9,
                          uint32_t a10, uint32_t a11)
{
    uint32_t list = U32(obj, 0x30);
    uint8_t *e;

    if (list == 0)
        return;
    /* entry count is the short at list+4; entries (0x58 bytes) start at list+0x18 */
    e = BYTES(list) + (uint32_t)(int)S16(list, 4) * 0x58u + 0x18;
    U32(e, 0x00) = a1;
    U32(e, 0x0c) = a1;
    U32(e, 0x04) = a2;
    U32(e, 0x10) = a2;
    U32(e, 0x08) = a3;
    U32(e, 0x14) = a3;
    U32(e, 0x18) = a4;
    U32(e, 0x1c) = a5;
    U32(e, 0x34) = a6;
    U32(e, 0x38) = a7;
    U32(e, 0x3c) = a8;
    U32(e, 0x40) = a9;
    U32(e, 0x44) = a10;
    U32(e, 0x48) = a11;
    U16(list, 4) = (uint16_t)(U16(list, 4) + 1);
}

// FUNCTION: HGSIM 0x10009278
void __cdecl Sim_10009278(void *obj, void *buf, unsigned char byte)
{
    if (U32(obj, 0x30) == 0)
        return;
    /* byte buffer: data pointer-offset at buf+4, fill count (ushort) at buf+2 */
    U8(buf, U32(buf, 4) + U16(buf, 2)) = byte;
    U16(buf, 2) = (uint16_t)(U16(buf, 2) + 1);
}

// FUNCTION: HGSIM 0x100a88db
void __cdecl Sim_100a88db(void)
{
    G32(0x1017d178) = G32(0x1017d170);
    G32(0x1017d174) = G32(0x1017d170) + G32(0x1014d1a4) - 0x30;
}

// FUNCTION: HGSIM 0x100ca350
int __cdecl Sim_100ca350(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    int result = -1;

    if (GS32(0x1017e5a8) < 0x80 && GS32(0x1017e5a8) != -1) {
        uint8_t *e = GBYTES(0x109a8640) + (uint32_t)GS32(0x1017e5a8) * 0x54u;

        U32(e, 0x18) = b;
        U32(e, 0x1c) = c;
        U32(e, 0x20) = d;
        U32(e, 0) = 1;
        U16(e, 0x24) = 0x401;
        U32(e, 8) = a | 0x200;
        U32(e, 0xc) = U32(PTRTAB(a), 8);
        U32(e, 0x10) = 0x453b8000;
        U32(e, 4) = 0;
        U8(e, 0x28) = 0x21;
        U8(e, 0x29) = 0;
        result = GS32(0x1017e5a8);
        GS32(0x1017e5a8) = GS32(0x1017e5a8) + 1;
    }
    return result;
}
