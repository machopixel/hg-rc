#include "w8v.h"
#include "w7n.h"
#include "../util/access.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"

/* The analog control slots: 0x80 bytes each from 0x1015fe00, count at 0x1015fde8 (at most 0x32). */
#define ANALOG(i, off) (*SIM_PTR(u32a, 0x1015fe00 + (uint32_t)(i) * 0x80 + (off)))
#define LINE_NO G32(0x1015fdf8)

/*
 * Finds or adds the analog control for the source src (its name at +0, its id at +4): an existing slot with the
 * same source is returned (and bound to device/index when it has no binding yet; "torso_pan"/"torso_tilt" also
 * set 0x1013f148/0x1013f14c), else a new slot is made (device NULL: unbound). Returns the slot or -1 when full.
 */
// FUNCTION: HGSIM 0x10053fe1
int __cdecl Input_AddAnalog(const uint32_t *src, const char *device, int32_t index)
{
    int32_t i, n, dev;

    for (i = 0; i < GS32(0x1015fde8); i++) {
        if (ANALOG(i, 0x14) == 0)
            continue;
        if (U32(ANALOG(i, 0x14), 0) != src[1])
            continue;
        if (device == 0)
            return i;
        if (ANALOG(i, 8) != 0)
            continue;
        dev = Input_GetDevice(device);
        ANALOG(i, 8) = (uint32_t)dev * 0x50 + (uint32_t)index * 4 + 0x1015fac8;
        ANALOG(i, 0) = (uint32_t)dev;
        ANALOG(i, 4) = (uint32_t)index;
        if (Str_CompareN((const char *)(uintptr_t)src[0], SIM_PTR(char, 0x10141a5c), 0xf) == 0)
            G32(0x1013f148) = (uint32_t)index;
        else if (Str_CompareN((const char *)(uintptr_t)src[0], SIM_PTR(char, 0x10141a68), 0xf) == 0)
            G32(0x1013f14c) = (uint32_t)index;
        return i;
    }
    n = GS32(0x1015fde8);
    G32(0x1015fde8) = (uint32_t)n + 1;
    if (GS32(0x1015fde8) > 0x32) {
        W8V_SYSERR(0x11, SIM_PTR(char, 0x10141a74), LINE_NO, 0);
        return -1;
    }
    dev = device != 0 ? Input_GetDevice(device) : -1;
    ANALOG(n, 0) = (uint32_t)dev;
    ANALOG(n, 4) = (uint32_t)index;
    ANALOG(n, 0x14) = (uint32_t)(uintptr_t)Input_AddAnalogSink(src);
    ANALOG(n, 8) = dev < 0 ? 0 : (uint32_t)dev * 0x50 + (uint32_t)index * 4 + 0x1015fac8;
    ANALOG(n, 0x7c) = Str_CompareN((const char *)(uintptr_t)src[0], SIM_PTR(char, 0x10141a98), 5) == 0;
    return n;
}

/*
 * Reads the keys of one chord up to the line "}": each line is " [+-] device key"; the entries (pointer to the
 * device's key bit word, the bit, 1 for '-') are 12 bytes each in out, at most 8. Returns 1 when the last key
 * was found, 0 otherwise or after a syntax error (reported with SystemError).
 * The locals are laid out in one block like the original's frame, because sscanf may write a long word past
 * the end of its buffer into the next one; the error reports pass the bottom of that block (never written) for
 * their "%s".
 */
// FUNCTION: HGSIM 0x100541e9
int __cdecl Input_ReadChord(void *stream, int32_t *count, uint8_t *out)
{
    char frame[0x16c];
    char *const sign = frame + 0x2c, *const device = frame + 0x30, *const key = frame + 0x58, *const line = frame + 0x6c;
    int32_t found = 0, dev;
    uint8_t *e;

    *count = 0;
    for (;;) {
        if (Input_ReadLine(line, 0xff, stream) == 0)
            break;
        if (line[0] == '}')
            break;
        if (Sim_100fd710(line, SIM_PTR(char, 0x10141aa0), sign, device, key) != 3) {
            W8V_SYSERR(0x11, SIM_PTR(char, 0x10141ab0), LINE_NO, frame, 0);
            return 0;
        }
        if (*count >= 8) {
            W8V_SYSERR(0x11, SIM_PTR(char, 0x10141ae0), LINE_NO, frame, 0);
            return 0;
        }
        dev = Input_GetDevice(device);
        found = Sim_10053a4f(dev, key);
        if (found == -1)
            continue;
        e = out + *count * 0xc;
        U32(e, 8) = sign[0] == '-';
        U32(e, 0) = (uint32_t)dev * 0x50 + (uint32_t)(found >> 5) * 4 + 0x1015fb08;
        U32(e, 4) = 1u << (found & 0x1f);
        *count = *count + 1;
    }
    return found != -1;
}

/*
 * Reads inputw.map, then for every device (0x10164bc0 of them) that is present (0x1015f868) its own map
 * "giddiwin\<device name without trailing digits>.std" when it exists.
 */
// FUNCTION: HGSIM 0x100548aa
int __cdecl Input_LoadMaps(void)
{
    char name[0x100];
    int32_t i, c, digit;
    void *f;

    f = Sim_100fd850(SIM_PTR(char, 0x10141c58), SIM_PTR(char, 0x10141c54));
    if (f == 0) {
        W8V_SYSERR(0x11, SIM_PTR(char, 0x10141c64), 0);
        return 0;
    }
    W8V_CALL_100543ee(f);
    W8V_FCLOSE5(f);
    for (i = 0; i < GS32(0x10164bc0); i++) {
        if (SIM_PTR(u32a, 0x1015f868)[i] == 0)
            continue;
        Str_Copy(name, SIM_PTR(char, 0x10141c84));
        Str_Cat(name, SIM_PTR(char, 0x10141c90));
        Str_Cat(name, SIM_PTR(char, 0x1015fc58 + (uint32_t)i * 0x28));
        for (;;) {
            if (GS32(0x1015965c) > 1) {
                digit = Crt_IsCType(name[Str_Length(name) - 1], 4);
            } else {
                c = name[Str_Length(name) - 1];
                digit = U16(G32(0x10159450), c * 2) & 4;
            }
            if (digit == 0)
                break;
            name[Str_Length(name) - 1] = 0;
        }
        Str_Cat(name, SIM_PTR(char, 0x10141c94));
        f = Sim_100fd850(name, SIM_PTR(char, 0x10141c9c));
        if (f != 0) {
            W8V_CALL_100543ee(f);
            W8V_FCLOSE5(f);
        }
    }
    return 1;
}
