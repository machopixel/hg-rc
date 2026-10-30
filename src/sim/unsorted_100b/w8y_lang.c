#include "w8y.h"
#include "../util/access.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w7q.h"
#include "../unsorted_1010/w6k.h"
#include "../unsorted_1010/w7r.h"

// FUNCTION: HGSIM 0x100b69d0
void *__cdecl Sim_100b69d0(const char *name)
{
    int32_t room = 0;
    char *eng = 0;
    char *tr = 0;
    uint32_t opt = 0;
    int lines = 0;
    int temp = 0;
    void *stream;
    uint8_t *table;
    uint32_t items;

    if (name == 0)
        name = SIM_PTR(const char, 0x101508e4);
    if (Crt_StrStr(name, SIM_PTR(const char, 0x101508f0)) == 0
        && W8Y_CALL_10048613(SIM_PTR(const char, 0x101508f8), name) == 0) {
        name = SIM_PTR(const char, 0x10150904);
        temp = 1;
    }
    stream = Sim_100fd850(name, SIM_PTR(const char, 0x10150910));
    if (stream == 0) {
        table = 0;
    } else {
        table = (uint8_t *)Heap_Calloc(8, 1);
        if (table != 0) {
            while (Sim_100b6ba0(stream, &eng, &tr, &opt, &lines) != 0) {
                if (!(room > S32(table, 0))) {
#ifdef W27L_MUTATE /* the checker's negative check only: the table starts with room for 99 */
                    room = room != 0 ? (int32_t)((uint32_t)room << 1) : 99;
#else
                    room = room != 0 ? (int32_t)((uint32_t)room << 1) : 100;
#endif
                    items = (uint32_t)(uintptr_t)Heap_Realloc((void *)(uintptr_t)U32(table, 4), (uint32_t)room << 3);
                    if (items == 0) {
                        Heap_FreeBlock((uint32_t)(uintptr_t)eng);
                        Heap_FreeBlock((uint32_t)(uintptr_t)tr);
                        break;
                    }
                    U32(table, 4) = items;
                }
                U32(U32(table, 4), U32(table, 0) * 8) = (uint32_t)(uintptr_t)eng;
                U32(U32(table, 4), U32(table, 0) * 8 + 4) = (uint32_t)(uintptr_t)tr;
                U32(table, 0) = U32(table, 0) + 1;
                tr = 0;
                eng = 0;
            }
            Crt_QSort((uint8_t *)(uintptr_t)U32(table, 4), U32(table, 0), 8, SIM_FN(W7qCompareFn, 0x100b6b87));
        }
        W8Y_CALL_100fd760(stream);
    }
    if (temp != 0)
        W8Y_CALL_100fdb30(name);
    return table;
}

// FUNCTION: HGSIM 0x100b6ba0
int __cdecl Sim_100b6ba0(void *stream, char **eng, char **tr, uint32_t *opt, int *lines)
{
    char line[0x100];
    char *o;
    int32_t i;
    int upper;

    for (;;) {
        if (Stream_ReadLine(line, 0x100, (uint8_t *)stream) == 0)
            return 0;
        *lines = *lines + 1;
        if (Sim_1010d240((const uint8_t *)line, SIM_PTR(const uint8_t, 0x10150914), 9) == 0) {
            *eng = Sim_100b6e5e(stream, lines);
            continue;
        }
        if (Sim_1010d240((const uint8_t *)line, SIM_PTR(const uint8_t, 0x10150920), 0xc) == 0) {
            *tr = Sim_100b6e5e(stream, lines);
            if (*eng == 0)
                continue;
            if (*tr == 0) {
                *tr = (char *)Heap_Malloc(Str_Length(*eng) + 1);
                if (*tr == 0)
                    return 0;
                Str_Copy(*tr, *eng);
            }
            if (*opt != 0)
                W8Y_CALL_100b7430(eng, tr, *opt, *lines);
            return 1;
        }
        if (Sim_1010d240((const uint8_t *)line, SIM_PTR(const uint8_t, 0x10150930), 9) != 0)
            continue;
        o = Sim_100b6e5e(stream, lines);
        if (o == 0)
            continue;
        for (i = 0; i < (int32_t)Str_Length(o); i++) {
            if (GS32(0x1015965c) > 1)
                upper = Crt_IsCType((int8_t)o[i], 1);
            else
                upper = U16(G32(0x10159450), (int32_t)(int8_t)o[i] * 2) & 1;
            if (upper != 0)
                o[i] = (char)((int8_t)o[i] + 0x20);
        }
        if (Crt_StrStr(o, SIM_PTR(const char, 0x1015093c)) != 0)
            *opt |= 1;
        if (Crt_StrStr(o, SIM_PTR(const char, 0x10150948)) != 0)
            *opt |= 2;
        if (Crt_StrStr(o, SIM_PTR(const char, 0x10150950)) != 0)
            *opt |= 4;
        if (Crt_StrStr(o, SIM_PTR(const char, 0x10150958)) != 0)
            *opt |= 8;
        Heap_FreeBlock((uint32_t)(uintptr_t)o);
    }
}

// FUNCTION: HGSIM 0x100b6e5e
char *__cdecl Sim_100b6e5e(void *stream, int *lines)
{
    char line[0x100];
    char *text = 0;
    uint32_t len;

    while (Stream_ReadLine(line, 0x100, (uint8_t *)stream) != 0) {
        len = Str_Length(line);
        *lines = *lines + 1;
        if (Crt_StrSpn(line, SIM_PTR(const char, 0x10150964)) == len)
            return W8Y_CALL_100b6f6a(text);
        if (text != 0) {
            text = (char *)Heap_Realloc(text, Str_Length(text) + len + 2);
            if (text == 0)
                return 0;
            Str_Cat(text, SIM_PTR(const char, 0x1015096c));
        } else {
            text = (char *)Heap_Malloc(len + 1);
            if (text == 0)
                return 0;
            *text = 0;
        }
        Str_Cat(text, line);
    }
    return text;
}
