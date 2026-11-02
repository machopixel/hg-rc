#include "w7n.h"
#include "w5y.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_1008/w2i.h"
#include "../util/access.h"

typedef int(__cdecl *W7nCountFn)(void);
typedef int(__cdecl *W7nNameFn)(int index, uint8_t *record);
typedef int(__cdecl *W7nRecordFn)(uint8_t *record);

typedef uint32_t(__stdcall *W7nMapVirtualKeyAFn)(uint32_t code, uint32_t type);
typedef int16_t(__stdcall *W7nGetKeyStateFn)(int key);

#define DRIVER(i) G32(0x1013f150u + 4u * (uint32_t)(i))
#define DEVICE(i) (GBYTES(0x1015f880) + (i) * 0x74)
#define DEVICE_NAME(i) ((char *)GBYTES(0x1015fc58) + (i) * 0x28)
#define DEVICE_DRIVER(i) (SIM_PTR(u32a, 0x1015fd20)[(i)])
#define DEVICE_OPEN(i) (SIM_PTR(u32a, 0x1015f868)[(i)])

/* The class bits of a character as the C library's isctype tests them (the table at [0x10159450] unless the
 * code page has multibyte characters). */
static uint32_t W7n_CharClass(char c, int mask)
{
    if (GS32(0x1015965c) > 1)
        return (uint32_t)Crt_IsCType(c, mask);
    return (uint32_t)*(u16a *)(BYTES(G32(0x10159450)) + (int)c * 2) & (uint32_t)mask;
}

// FUNCTION: HGSIM 0x1005366e
int __cdecl Input_AddDevice(const char *name)
{
    int d, i, count, slot;

    for (d = 0; d < 3; d++) {
        if (DRIVER(d) == 0)
            continue;
        count = ((W7nCountFn)(uintptr_t)U32(DRIVER(d), 0))();
        if (count == 0)
            continue;
        for (i = 0; i < count; i++) {
            slot = GS32(0x10164bc0);
            if (GS32(0x10164bc0) >= 5) {
                W7N_CALL_100bc280(0x11, SIM_PTR(char, 0x10141968), 0);
                return -1;
            }
            if (((W7nNameFn)(uintptr_t)U32(DRIVER(d), 4))(i, DEVICE(slot)) != 0)
                continue;
            if (Str_CompareNoCase((const char *)DEVICE(slot), name) != 0) {
                ((W7nRecordFn)(uintptr_t)U32(DRIVER(d), 0xc))(DEVICE(slot));
                continue;
            }
            Str_Copy(DEVICE_NAME(slot), name);
            DEVICE_DRIVER(slot) = DRIVER(d);
            if (Str_CompareNoCase((const char *)DEVICE(slot), SIM_PTR(char, 0x1014198c)) == 0)
                G32(0x1013f144) = G32(0x10164bc0);
            G32(0x10164bc0) = G32(0x10164bc0) + 1;
            if (((W7nRecordFn)(uintptr_t)U32(DEVICE_DRIVER(slot), 8))(DEVICE(slot)) != 0) {
                W7N_CALL_1005573d(0x70, 0, DEVICE(slot) + 0xc);
                DEVICE_OPEN(slot) = 0;
                return -1;
            }
            DEVICE_OPEN(slot) = 1;
            return slot;
        }
    }
    slot = GS32(0x10164bc0);
    G32(0x10164bc0) = G32(0x10164bc0) + 1;
    Str_Copy(DEVICE_NAME(slot), name);
    DEVICE_OPEN(slot) = 0;
    W7N_CALL_1005573d(0x6e, 0, name);
    return -1;
}

// FUNCTION: HGSIM 0x1005388b
int __cdecl Input_GetDevice(const char *name)
{
    int i, result;

    for (i = 0; i < GS32(0x10164bc0); i++) {
        if (Str_CompareNoCase(DEVICE_NAME(i), name) == 0)
            return DEVICE_OPEN(i) != 0 ? i : -1;
    }
    result = Input_AddDevice(name);
    if (Str_CompareNoCase(name, SIM_PTR(char, 0x10141998)) == 0) {
        if (result != -1)
            GS32(0x1015fdf4) = result;
        else
            W7N_CALL_100bc280(0x11, SIM_PTR(char, 0x101419a4), 0);
    }
    return result;
}

/* Sim_1005393a and Sim_10053a4f: the channel list at device +listOff with its count at +countOff. */
static int W7n_FindChannel(int device, const char *name, uint32_t listOff, uint32_t countOff, uint32_t format)
{
    int i;

    if (W7n_CharClass(*name, 4) != 0)
        return Crt_AToI(name);
    if (device == -1)
        return -1;
    if (U32(DEVICE(device), listOff) == 0) {
        W7N_CALL_100bc280(0x11, SIM_PTR(char, format), name, G32(0x1015fdf8), 0);
        return -1;
    }
    for (i = 0; i < S32(DEVICE(device), countOff); i++) {
        uint32_t entry = U32(U32(DEVICE(device), listOff), 4u * (uint32_t)i);

        if (entry != 0 && Str_CompareNoCase((const char *)(uintptr_t)entry, name) == 0)
            return i;
    }
    W7N_CALL_1005573d(0x6f, name, DEVICE(device) + 0xc);
    return -1;
}

// FUNCTION: HGSIM 0x1005393a
int __cdecl Sim_1005393a(int device, const char *name)
{
    return W7n_FindChannel(device, name, 0x64, 0x58, 0x101419c4);
}

// FUNCTION: HGSIM 0x10053a4f
int __cdecl Sim_10053a4f(int device, const char *name)
{
    return W7n_FindChannel(device, name, 0x6c, 0x5c, 0x101419e4);
}

// FUNCTION: HGSIM 0x10053b64
int __cdecl Input_ReadLine(char *buf, int n, uint8_t *stream)
{
    int content = 0;
    char *p;

    for (;;) {
        if (Stream_ReadLine(buf, n, stream) == 0)
            return 0;
        G32(0x1015fdf8) = G32(0x1015fdf8) + 1;
        for (p = buf; *p != 0; p++) {
            if (*p == '\n' || *p == '#') {
                *p = 0;
                break;
            }
            if (W7n_CharClass(*p, 8) == 0)
                content = 1;
        }
        if (content != 0)
            return 1;
    }
}

/* The dword at +4 of the channel table entry for `name` (Sim_10053620), 0 when there is none. */
static uint32_t W7n_ChannelValue(const char *name)
{
    void *entry = Sim_10053620(name);

    return entry != 0 ? U32(entry, 4) : 0;
}

// FUNCTION: HGSIM 0x10053c40
uint8_t *__cdecl Input_AddAnalogSink(const uint32_t *src)
{
    char text[0x28];
    char *cut;
    uint8_t *rec;
    int i;

    for (i = 0; i < GS32(0x1015fdec); i++) {
        if (U32(GBYTES(0x10161700), i * 0x32) == src[1])
            return GBYTES(0x10161700) + i * 0x32;
    }
    i = GS32(0x1015fdec);
    G32(0x1015fdec) = G32(0x1015fdec) + 1;
    if (GS32(0x1015fdec) > 0x32) {
        W7N_CALL_100bc280(0x11, SIM_PTR(char, 0x10141a04), G32(0x1015fdf8), 0);
        return 0;
    }
    rec = GBYTES(0x10161700) + i * 0x32;
    if (Crt_StrStr((const char *)(uintptr_t)src[0], SIM_PTR(char, 0x10141a24)) != 0)
        U32(rec, 4) = (uint32_t)(uintptr_t)(rec + 0x1a);
    else
        U32(rec, 4) = (uint32_t)(uintptr_t)(rec + 0x16);
    U32(rec, 0) = src[1];
    U32(rec, 8) = src[5] - src[4];
    U32(rec, 0xc) = src[4] << 16;
    U8(rec, 0x14) = (uint8_t)src[7];
    U32(rec, 0x10) = src[3] << 16;
    U32(rec, 0x10) = U32(rec, 0x10) - U32(rec, 0xc);
    U32(rec, 0x10) = U32(rec, 0x10) << 1;
    S32(rec, 0x10) = S32(rec, 0x10) / S32(rec, 8);
    U32(rec, 0x10) = U32(rec, 0x10) - 0x10000u;
    U32(rec, 0x1e) = src[6];
    U32(rec, 0x1a) = U32(rec, 0x10);

    Str_Copy(text, (const char *)(uintptr_t)src[0]);
    cut = Crt_StrStr(text, SIM_PTR(char, 0x10141a2c));
    if (cut != 0)
        *cut = 0;
    Str_Cat(text, SIM_PTR(char, 0x10141a34));
    U32(rec, 0x22) = W7n_ChannelValue(text);

    Str_Copy(text, (const char *)(uintptr_t)src[0]);
    cut = Crt_StrStr(text, SIM_PTR(char, 0x10141a3c));
    if (cut != 0)
        *cut = 0;
    Str_Cat(text, SIM_PTR(char, 0x10141a44));
    U32(rec, 0x26) = W7n_ChannelValue(text);

    Str_Copy(text, (const char *)(uintptr_t)src[0]);
    Str_Cat(text, SIM_PTR(char, 0x10141a4c));
    U32(rec, 0x2e) = W7n_ChannelValue(text);

    Str_Copy(text, (const char *)(uintptr_t)src[0]);
    Str_Cat(text, SIM_PTR(char, 0x10141a54));
    U32(rec, 0x2a) = W7n_ChannelValue(text);
    return rec;
}

// FUNCTION: HGSIM 0x10057186
int __cdecl Joy_GetOemName(int index, const char *driver, char *out, uint32_t n)
{
    /* C runtime replacement, the rest (DECISIONS.md): the original read the joystick's maker name from the Windows
     * registry: "Joystick<n>OEMName" under HKEY_LOCAL_MACHINE System\CurrentControlSet\Control\MediaResources
     * \Joystick\<driver>\CurrentJoystickSettings, then "OEMName" under ...\Control\MediaProperties
     * \PrivateProperties\Joystick\OEM\<that name>. Joysticks are not supported (Diego, 2026-11-02): no name, as the
     * original answers when a key is missing (it also printed an empty debug line then). */
    (void)index, (void)driver, (void)out, (void)n;
    return 0;
}

// FUNCTION: HGSIM 0x10057c1a
void __cdecl Input_QueueKey(int key)
{
    uint16_t c = U16(GBYTES(0x10143a00), key * 2);

    if ((int16_t)c == 0) {
        c = (uint16_t)(int16_t)(int8_t)SIM_API(W7nMapVirtualKeyAFn, 0x10ac578c)((uint32_t)key, 2);
        c = (uint16_t)W7N_CALL_100ff280((int16_t)c);
    }
    if ((int16_t)c == 0)
        return;
    if ((SIM_API(W7nGetKeyStateFn, 0x10ac5788)(0x11) & 0x1000) != 0)
        c |= 0x100;
    if ((SIM_API(W7nGetKeyStateFn, 0x10ac5788)(0x12) & 0x1000) != 0)
        c |= 0x400;
    if ((SIM_API(W7nGetKeyStateFn, 0x10ac5788)(0x10) & 0x1000) != 0)
        c |= 0x200;
    U16(GBYTES(0x10165ee0), GS32(0x1016686c) * 2) = c;
    G32(0x1016686c) = G32(0x1016686c) + 1;
    if (G32(0x1016686c) == 0x40) {
        G32(0x1016686c) = 0;
    } else if (G32(0x1016686c) == G32(0x10166868)) {
        G32(0x10166868) = G32(0x10166868) + 1;
        if (G32(0x10166868) == 0x40)
            G32(0x10166868) = 0;
    }
}
