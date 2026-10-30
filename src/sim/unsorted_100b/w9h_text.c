#include "w9h.h"
#include "../util/access.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2k.h"
#include "../unsorted_100f/w6h.h"
#include "../unsorted_100f/w6i.h"
#include "../unsorted_100f/w8z.h"

/* Character class of c (a signed char, like the original indexes the table) under `mask`. */
static int W9h_Class(char c, int mask)
{
    if (GS32(0x1015965c) > 1)
        return Crt_IsCType((int)(int8_t)c, mask);
    return U16(G32(0x10159450), (int32_t)(int8_t)c * 2) & mask;
}

// FUNCTION: HGSIM 0x100b6f6a
char *__cdecl Text_Unescape(char *text)
{
    char digits[4];
    int32_t value = 0;
    uint32_t i = 0, j = 0;
    int32_t k;
    char c;

    if (text == 0)
        return 0;
    while (text[i] != 0) {
        c = text[i];
        if (c >= 10 && c <= 13) {
            i++;
            continue;
        }
        if (c != '\\' || c < 10) {
            text[j++] = text[i++];
            continue;
        }
        i++;
        switch (text[i]) {
        case 0:
            break;
        case 'a':
            text[j++] = 7;
            i++;
            break;
        case 'b':
            text[j++] = 8;
            i++;
            break;
        case 'f':
            text[j++] = 0xc;
            i++;
            break;
        case 'n':
            text[j++] = 0xa;
            i++;
            break;
        case 'r':
            text[j++] = 0xd;
            i++;
            break;
        case 't':
            text[j++] = 9;
            i++;
            break;
        case 'v':
            text[j++] = 0xb;
            i++;
            break;
        case '\'':
            text[j++] = '\'';
            i++;
            break;
        case '"':
            text[j++] = '"';
            i++;
            break;
        case '\\':
            text[j++] = '\\';
            i++;
            break;
        case '?':
            text[j++] = '?';
            i++;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            k = 0;
            while (W9h_Class(text[i], 4) != 0 && text[i] != '8' && text[i] != '9' && k < 3) {
                digits[k++] = text[i];
                i++;
            }
            digits[k] = 0;
            Sim_100fd710(digits, SIM_PTR(const char, 0x10150970), &value);
            text[j++] = (char)value;
            break;
        case 'x':
            k = 0;
            i++;
            while (W9h_Class(text[i], 0x80) != 0 && k < 3) {
                digits[k++] = text[i];
                i++;
            }
            digits[k] = 0;
            if (k == 0) {
                text[j++] = '\\';
                text[j++] = 'x';
            } else {
                Sim_100fd710(digits, SIM_PTR(const char, 0x10150974), &value);
                text[j++] = (char)value;
            }
            break;
        default:
            text[j++] = '\\';
            text[j++] = text[i];
            i++;
            break;
        }
    }
    text[j] = 0;
    return text;
}

// FUNCTION: HGSIM 0x100b7430
void __cdecl Sim_100b7430(uint32_t unused, char **text, uint32_t flags, uint32_t number)
{
    char *out;
    uint32_t len, i, j;
    int32_t k;

    (void)unused;
    if ((flags & 1) != 0 || (flags & 8) != 0) {
        len = Str_Length(*text);
        i = 0;
        j = 0;
        out = (char *)Heap_Malloc(len * 2 + 1);
        if (out == 0)
            return;
        while ((*text)[i] != 0) {
            out[j++] = (*text)[i];
            switch ((*text)[i]) {
            case '!':
                out[j++] = (char)0xa1;
                break;
            case '?':
                out[j++] = (char)0xbf;
                break;
            case 'A':
                out[j++] = (char)0xc4;
                break;
            case 'E':
                out[j++] = (char)0xcb;
                break;
            case 'I':
                out[j++] = (char)0xcf;
                break;
            case 'O':
                out[j++] = (char)0xd6;
                break;
            case 'U':
                out[j++] = (char)0xdc;
                break;
            case 'a':
                out[j++] = (char)0xe4;
                break;
            case 'e':
                out[j++] = (char)0xeb;
                break;
            case 'i':
                out[j++] = (char)0xef;
                break;
            case 'o':
                out[j++] = (char)0xf6;
                break;
            case 'u':
                out[j++] = (char)0xfc;
                break;
            case 'y':
                out[j++] = (char)0xff;
                break;
            default:
                break;
            }
            i++;
        }
        out[j++] = '&';
        out[j] = 0;
        Heap_FreeBlock((uint32_t)(uintptr_t)*text);
        *text = out;
    }
    if ((flags & 2) != 0) {
        len = Str_Length(*text);
        for (k = 0; k < (int32_t)len; k++) {
            if (Crt_StrChr(SIM_PTR(const char, 0x10150978), (int)(int8_t)(*text)[k]) == 0)
                (*text)[k] = '*';
        }
        (*text)[0] = '$';
        (*text)[(int32_t)len - 1] = '$';
    }
    if ((flags & 4) != 0) {
        len = Str_Length(*text);
        out = (char *)Heap_Malloc(len + 10);
        if (out == 0)
            return;
        Crt_SPrintf(out, SIM_PTR(const char, 0x10150980), number, *text);
        Heap_FreeBlock((uint32_t)(uintptr_t)*text);
        *text = out;
    }
}
