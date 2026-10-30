#include "w2e.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10041cc6
void __cdecl Sim_10041cc6(uint32_t id, const char *s, uint32_t unused, void *dst)
{
    (void)unused;
    if (Str_CompareN((const char *)GBYTES(0x1015d160), s, 0x18) == 0) {
        Str_CopyN((char *)BYTES(dst) + 0xe, s, 0x18);
        U16(dst, 0) = (uint16_t)id;
    }
}

// FUNCTION: HGSIM 0x10041d01
void __cdecl Sim_10041d01(const void *src, void *dst)
{
    if (src != 0) {
        if (Str_CompareN((const char *)GBYTES(0x1015db60), (const char *)BYTES(src) + 0x17, 0x20) == 0) {
            Mem_Move(dst, src, 0x5f);
            G32(0x1015ec50) = 0;
        }
    } else {
        if (G32(0x1015ec50) == 0)
            G32(0x1015ec50) = 1;
        Mem_Fill(dst, 0, 0x5f);
    }
}

// FUNCTION: HGSIM 0x10044255
void __cdecl Sim_10044255(void)
{
    Mem_Fill(GBYTES(0x1015ed70), 0, 0x118);
    G32(0x1013aec4) = 0;
    G32(0x1015ed20) = 0;
    G32(0x1015ed38) = 0;
    G32(0x1015ee98) = 0;
}

// FUNCTION: HGSIM 0x10043f10
void __cdecl Sim_10043f10(void)
{
    Sim_10044255();
}

// FUNCTION: HGSIM 0x10048762
int __cdecl Sim_10048762(const char *s)
{
    int len = (int)Str_Length(s);
    const char *table = (const char *)GBYTES(0x109b94be);
    const char *p = s + len - 1;

    while (len != 0) {
        if ((int)(int8_t)*table != ((int)(int8_t)*p ^ 9))
            return 0;
        len--;
        p--;
        table--;
    }
    return 1;
}

// FUNCTION: HGSIM 0x100487d0
void __cdecl Sim_100487d0(void)
{
    int i;

    for (i = 0; i < 0x10; i++) {
        uint8_t *record = GBYTES(0x1013b384) + (uint32_t)i * 0xcu;
        int field;

        for (field = 0; field < 2; field++) {
            char *str = (char *)BYTES(U32(record, (uint32_t)field * 4));
            int len, j;

            if (str == 0)
                continue;
            len = (int)Str_Length(str);
            for (j = 0; j < len; j++)
                str[j] = (char)(str[j] ^ 0xa);
        }
    }
}
