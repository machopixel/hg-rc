#include "w8w.h"
#include "w2l.h"
#include "w3o.h"
#include "w7n.h"
#include "../unsorted_1004/w2h.h"
#include "../unsorted_1008/w2i.h"
#include "../unsorted_1008/w6f.h"
#include "../unsorted_1008/w7o.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../util/access.h"
#include "../view/view.h"
#include "../../platform/plat.h"


/*
 * (value << 16) / divisor with the original's 64-bit signed division (cdq, shld, shl, idiv), truncated toward
 * zero. The original raises a divide error for a zero divisor or a quotient that does not fit 32 bits; this
 * version returns the low 32 bits of the quotient (0 for a zero divisor).
 */
static int32_t DivShifted16(int32_t value, int32_t divisor)
{
    uint32_t nmag = value < 0 ? 0u - (uint32_t)value : (uint32_t)value;
    uint32_t dmag = divisor < 0 ? 0u - (uint32_t)divisor : (uint32_t)divisor;
    uint64_t n = (uint64_t)nmag << 16;
    uint64_t rem = 0, quot = 0;
    int i;

    if (dmag == 0)
        return 0;
    for (i = 47; i >= 0; i--) {
        rem = (rem << 1) | ((n >> i) & 1);
        if (rem >= dmag) {
            rem -= dmag;
            quot |= (uint64_t)1 << i;
        }
    }
    if ((value < 0) != (divisor < 0))
        quot = 0 - quot;
    return (int32_t)(uint32_t)quot;
}

// FUNCTION: HGSIM 0x10074cea
void __cdecl Sim_10074cea(uint8_t *s, int32_t code)
{
    int32_t *frame = (int32_t *)(uintptr_t)U32(s, 0);
    int32_t *text = (int32_t *)(uintptr_t)U32(s, 0x18);
    int32_t point[2];
    uint32_t data, size;
    int32_t sx, sy, step, shift;

    if (frame == 0 || text == 0 || frame[0] == 0 || text[0] == 0)
        return;
    if (U32(s, 0x10) != 0) {
        if (G32(0x101c64f0) == 0)
            U8(U32(s, 0x10), 2) = 0x4c;
        if (G32(0x1014cb38) != 0)
            U8(U32(s, 0x10), 7) = 0x47;
        data = W8W7_CALL_100ba7d5((char *)(uintptr_t)U32(s, 0x10), G32(0x1014cec0), 0);
        if (data != 0) {
            size = W8W7_CALL_100ed743(data, 0);
            Sim_100bb2b0((const uint32_t *)(uintptr_t)U32(s, 0x10), (const char *)(uintptr_t)G32(0x1014cec0));
            sx = (int32_t)size >> 16;
            sy = (int32_t)(size & 0xffff);
            sx = DivShifted16(sx, (int32_t)(G32(0x109e3e14) + 1));
            sy = DivShifted16(sy, (int32_t)(G32(0x109e3de0) + 1));
            sx = DivShifted16(sx, (int32_t)((uint32_t)text[3] - (uint32_t)text[1] + 1));
            sy = DivShifted16(sy, (int32_t)((uint32_t)text[4] - (uint32_t)text[2] + 1));
            Rect_ScaleAroundCenter(text, text, sx, sy);
        }
    }
    point[0] = 0;
    step = DivShifted16(1, (int32_t)(U32(s, 0x54) + 2));
    point[1] = step;
    U32(s, 0x64) = (uint32_t)DivShifted16(step, 0x20000);
    U32(s, 0x7c) = (uint32_t)step + (uint32_t)step;
    U32(s, 0x74) = U32(s, 0x7c);
    U32(s, 0x6c) = U32(s, 0x74);
    if (code == 5) {
        View_ScaleRect((void *)(uintptr_t)frame[0], frame, frame);
        View_ScaleRect((void *)(uintptr_t)text[0], text, text);
    }
    /* the point {0, step} lies in the original's frame like this array: the scaling writes both values */
    Rect_ScalePoint(frame, point, point);
    Rect_ScalePoint(frame, (int32_t *)(s + 0x60), (int32_t *)(s + 0x60));
    Rect_ScalePoint(frame, (int32_t *)(s + 0x68), (int32_t *)(s + 0x68));
    Rect_ScalePoint(frame, (int32_t *)(s + 0x70), (int32_t *)(s + 0x70));
    Rect_ScalePoint(frame, (int32_t *)(s + 0x78), (int32_t *)(s + 0x78));
    if (U32(s, 4) & 0x10) {
        shift = (int32_t)((uint32_t)text[1] - (uint32_t)frame[1]);
        text[1] = (int32_t)((uint32_t)text[1] - (uint32_t)shift);
        text[3] = (int32_t)((uint32_t)text[3] - (uint32_t)shift);
    }
    U32(s, 0x58) = (uint32_t)point[0];
    U32(s, 0x5c) = (uint32_t)point[1];
}

// FUNCTION: HGSIM 0x10075667
int32_t __cdecl Sim_10075667(uint8_t *s)
{
    uint32_t frame = U32(s, 0);
    uint32_t data;

    if (frame == 0)
        return 0;
    data = U32(s, 0x18);
    if (data == 0)
        return 0;
    Sim_10074fce(s);
    if (U32(s, 4) & 0x20)
        W8W7_CALL_100eb070(frame, 0xf0);
    if (U32(s, 0x14) != 0) {
        if (G32(0x1017b940) != 0)
            W8W7_CALL_100f0205(data, U32(s, 0x14), 0, 0, 0);
        else
            W8W7_CALL_100e91dc(data, U32(s, 0x14), 0, 0, 0);
    }
    if (U32(s, 4) & 4)
        Sim_1008d4f1((int32_t *)(uintptr_t)frame, 0x1c);
    W8W7_CALL_1007581a(s);
    return Sim_10076676(s) == 0;
}

// FUNCTION: HGSIM 0x10077d50
void __cdecl Sim_10077d50(void)
{
    void *file;
    int i;

    G32(0x10179d20) = 1;
    Sim_10078177();
    for (i = 0; i < 7; i++)
        SIM_PTR(u32a, 0x1096fc40)[i] = SIM_PTR(u32a, 0x1096fc00)[i];
    G32(0x1096fbec) = G32(0x101355b8);
    G32(0x1096fc1c) = G32(0x10144fac);
    G32(0x1096fc20) = G32(0x1015d1ac);
    file = Sim_100fd850((const char *)GBYTES(0x10149ae8), (const char *)GBYTES(0x10149ae4));
    if (file == 0)
        return;
    W8W7_FWRITE(GBYTES(0x1096fc40), 4, 7, file);
    W8W7_FWRITE(GBYTES(0x1096fbec), 4, 1, file);
    W8W7_FWRITE(GBYTES(0x1096fc1c), 4, 1, file);
    W8W7_FWRITE(GBYTES(0x1096fc20), 4, 1, file);
    W8W7_FCLOSE(file);
}

// FUNCTION: HGSIM 0x10077e34
void __cdecl Sim_10077e34(void)
{
    void *file = Sim_100fd850((const char *)GBYTES(0x10149af8), (const char *)GBYTES(0x10149af4));

    if (file == 0) {
        G32(0x10179d20) = 0;
        return;
    }
    G32(0x10179d20) = 1;
    W8W7_FREAD(GBYTES(0x1096fc40), 4, 7, file);
    W8W7_FREAD(GBYTES(0x1096fbec), 4, 1, file);
    W8W7_FREAD(GBYTES(0x1096fc1c), 4, 1, file);
    W8W7_FREAD(GBYTES(0x1096fc20), 4, 1, file);
    W8W7_FCLOSE(file);
}

// FUNCTION: HGSIM 0x1007c89b
int32_t __cdecl Sim_1007c89b(void)
{
    uint8_t block[0x15014];
    int32_t count = 0;
    int32_t i;
    uint8_t *side = GBYTES(0x10473800) + G32(0x1013bb40) * 0x4fc3a;
    uint8_t *member, *entry;
    void *file;

    Mem_Fill(block, 0, 0x15014);
    Mem_Move(block, GBYTES(0x10149c8c), 4);
    U32(block, 8) = U32(side, 4);
    U32(block, 0xc) = U32(side, 8);
    U32(block, 0x10) = U8(side, 0x38);
    for (i = 0; i < (int32_t)U32(side, 0); i++) {
        member = side + i * 0x13f;
        if (U8(member, 0xae) == 0)
            continue;
        entry = block + 0x14 + count * 0x54;
        U32(entry, 0) = U8(member, 0x3a) == 5;
        U32(entry, 4) = U8(member, 0x3b);
        U32(entry, 8) = U32(member, 0xa1);
        U32(entry, 0xc) = U32(member, 0xa5);
        U32(entry, 0x10) = U32(member, 0xaf);
        Str_Copy((char *)entry + 0x14, (const char *)member + 0xe7);
        count++;
    }
    U32(block, 4) = (uint32_t)count;
    if (G32(0x1015ec58) != 0)
        U32(block, 0x10) = Sim_100429d5() & 0xff;
    file = Sim_100fd850((const char *)GBYTES(0x10149c98), (const char *)GBYTES(0x10149c94));
    if (file == 0)
        return 0;
    W8W7_FWRITE(block, 0x15014, 1, file);
    W8W7_FCLOSE(file);
    return 1;
}

// FUNCTION: HGSIM 0x1007dd90
void __cdecl Sim_1007dd90(void)
{
    int32_t i;
    uint8_t *panel;
    int32_t *rect;
    uint32_t data;
    int32_t sign, dx, dy, height;

    for (i = 0; i < 2; i++) {
        panel = GBYTES(0x10149d28) + i * 0x3c;
        rect = (int32_t *)(uintptr_t)U32(panel, 0x38);
        if (G32(0x101c64f0) == 0)
            U8(panel, 0x26) = 0x4c;
        View_ScaleRect(GBYTES(0x109e3e20), rect, rect);
        data = W8W7_CALL_100ba7d5((char *)panel + 0x24, G32(0x1014cec0), 0);
        if (data != 0)
            Sim_1008d2d0(rect, rect, (void *)(uintptr_t)data, 0);
        sign = i == 0 ? -1 : 1;
        dx = -rect[1];
        dy = (int32_t)((uint32_t)sign * (uint32_t)rect[2]);
        rect[1] = (int32_t)((uint32_t)rect[1] + (uint32_t)dx);
        rect[2] = (int32_t)((uint32_t)rect[2] + (uint32_t)dy);
        rect[3] = (int32_t)((uint32_t)rect[3] + (uint32_t)dx);
        rect[4] = (int32_t)((uint32_t)rect[4] + (uint32_t)dy);
        Rect_ScalePoint(rect, (int32_t *)(panel + 4), (int32_t *)(panel + 4));
        if (Str_CompareNoCase((const char *)(uintptr_t)G32(0x1014cec4), (const char *)GBYTES(0x10149da0)) == 0)
            Str_Copy((char *)(uintptr_t)G32(0x1014cec4), (const char *)GBYTES(0x10149da8));
        Str_CopyN((char *)panel + 0x14, (const char *)GBYTES(0x101380a0) + G32(0x101c64f0) * 0x12, 8);
        data = W8W7_CALL_100ba7d5((char *)panel + 0x14, G32(0x1014cec4), 0);
        if (data != 0) {
            height = W8W7_CALL_100ecce0(data);
            U32(panel, 8) = (uint32_t)(((int32_t)((uint32_t)rect[4] - (uint32_t)rect[2] + 1 - (uint32_t)height)) / 2);
            Sim_100bb2b0((const uint32_t *)(panel + 0x14), (const char *)(uintptr_t)G32(0x1014cec4));
        }
    }
}

// FUNCTION: HGSIM 0x1007e29a
void __cdecl Sim_1007e29a(char *name1, char *name2, const char *text, int32_t dx, int32_t dy)
{
    uint32_t back = 0, font;
    int32_t rect[5], centred[5];
    int i;

    if (text == 0)
        return;
    if (name1[0] != 0)
        back = W8W7_CALL_100ba7d5(name1, G32(0x1014cec0), 0);
    font = W8W7_CALL_100ba7d5(name2, G32(0x1014cec4), 0);
    if (font == 0)
        return;
    rect[0] = 0x109e3e20;
    W8W7_CALL_1008de0a(text, font, rect);
    if (dx >= 0) {
        rect[1] = (int32_t)((uint32_t)rect[1] + (uint32_t)dx);
        rect[3] = (int32_t)((uint32_t)rect[3] + (uint32_t)dx);
    } else {
        for (i = 0; i < 5; i++)
            centred[i] = rect[i];
        View_CenterRect(GBYTES(0x109e3e20), centred, centred);
        rect[1] = centred[1];
        rect[3] = centred[3];
    }
    if (dy >= 0) {
        rect[2] = (int32_t)((uint32_t)rect[2] + (uint32_t)dy);
        rect[4] = (int32_t)((uint32_t)rect[4] + (uint32_t)dy);
    } else {
        for (i = 0; i < 5; i++)
            centred[i] = rect[i];
        View_CenterRect(GBYTES(0x109e3e20), centred, centred);
        rect[2] = centred[2];
        rect[4] = centred[4];
    }
    if (back != 0)
        W8W7_CALL_1008e0fe(rect, back, 0);
    W8W7_CALL_1008dba2(rect, text, font);
}

// FUNCTION: HGSIM 0x1007e66c
void __cdecl Sim_1007e66c(void)
{
    /* a WIN32_FIND_DATAA (0x140 bytes, as the original's frame), from the platform's folder listing (C runtime
     * replacement, files: no short name, so the long one is used) */
    uint8_t found[0x140];
    char *path = (char *)GBYTES(0x1017aeb0);
    signed char drive = 0;
    uint32_t handle;

    W8W7_SPRINTF(path, (const char *)GBYTES(0x10149df8), GBYTES(0x10149df0));
    handle = Plat_FileFindFirst(path, found);
    if (handle == 0xffffffffu) {
        drive = (signed char)W8W7_CALL_10099510();
        if (drive == 0)
            return;
        W8W7_SPRINTF(path, (const char *)GBYTES(0x10149e0c), (int)drive, GBYTES(0x10149e04));
        handle = Plat_FileFindFirst(path, found);
        if (handle == 0xffffffffu)
            return;
    }
    do {
        if (found[0x130] == 0)
            ProjFile_FindEntry((const char *)found + 0x2c, 1);
        else
            ProjFile_FindEntry((const char *)found + 0x130, 1);
    } while (Plat_FileFindNext(handle, found) != 0);
    Plat_FileFindClose(handle);
    if (drive != 0)
        W8W7_SPRINTF(path, (const char *)GBYTES(0x10149e24), (int)drive, GBYTES(0x10149e1c));
    else
        W8W7_SPRINTF(path, (const char *)GBYTES(0x10149e34), GBYTES(0x10149e2c));
}

// FUNCTION: HGSIM 0x1007e7b6
uint32_t __cdecl Sim_1007e7b6(const char *name)
{
    char file[0x100];
    char path[0x100];

    /* the original first tests the address 0x1017aeb0 itself for null (never true) */
    Str_Copy(file, name);
    Str_Cat(file, (const char *)GBYTES(0x10149e38));
    if (ProjFile_FindEntry(file, 0) == 0)
        return 0;
    Str_Copy(path, (const char *)GBYTES(0x1017aeb0));
    Str_Cat(path, (const char *)GBYTES(0x10149e40));
    Str_Cat(path, file);
    return W8W7_CALL_100461e8(path, 0);
}
