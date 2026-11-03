/*
 * Test-only bodies (spec_w9e.py) for callees of the round-9 letter-e functions that the checker replaces by
 * logging stubs at their C symbol as well as at their original address: the bodies below never run, they only
 * give the symbols an address to hook. They stand in for the real files (w3s_array.c, w7p_ptrarray.c,
 * w7p_misc.c, w6g_misc.c, w8x_scene.c, w8y_scene.c, unsorted_100b/fields.c, w8x_hud.c, w7q_objects.c,
 * w8z_crt.c, w7n_input.c, w8v_input.c), whose other functions would pull most of the DLL into the test image.
 * Not part of the game build.
 */
#include <stdint.h>
#include "sim/simmem.h"

#ifndef _WIN32
#define __cdecl
#endif

/* weak: when the checker links a real file that defines the same function (it adds files for missing symbols),
   the real one is used; either body is hooked, never run */
#define W9E_SHIM __attribute__((weak))

static volatile int w9e_shim_sink; /* keeps every body distinct, so no two symbols share an address */

W9E_SHIM void *__cdecl PtrArray_CreateSized(int n)
{
    w9e_shim_sink = n + 1;
    return 0;
}

W9E_SHIM int __cdecl PtrArray_Insert(void *arr, uint32_t item, int(__cdecl *compare)(uint32_t, uint32_t))
{
    (void)arr;
    (void)compare;
    w9e_shim_sink = (int)item + 2;
    return 0;
}

W9E_SHIM void __cdecl Sim_100b9675(void *obj, void(__cdecl *fn)(uint32_t))
{
    (void)obj;
    (void)fn;
    w9e_shim_sink = 3;
}

W9E_SHIM void __cdecl Sim_100bd69c(void *obj, void(__cdecl *fn)(uint32_t))
{
    (void)obj;
    (void)fn;
    w9e_shim_sink = 4;
}

W9E_SHIM int __cdecl Sim_100bb2b0(const uint32_t *key, const char *name)
{
    (void)key;
    (void)name;
    w9e_shim_sink = 5;
    return 0;
}

W9E_SHIM void __cdecl Sim_100a4d8e(void)
{
    w9e_shim_sink = 6;
}

W9E_SHIM void __cdecl Sim_100a4ff0(void *node)
{
    (void)node;
    w9e_shim_sink = 17;
}

W9E_SHIM void __cdecl Sim_100a512a(void *node)
{
    (void)node;
    w9e_shim_sink = 18;
}

W9E_SHIM void __cdecl Sim_100b53f0(int index, uint32_t *a, uint32_t *b, uint32_t *c)
{
    (void)a;
    (void)b;
    (void)c;
    w9e_shim_sink = index + 7;
}

W9E_SHIM int __cdecl Sim_1008d6f7(void *pane, const uint8_t *text, int32_t x, int32_t y, void *font, uint32_t colour)
{
    (void)pane;
    (void)text;
    (void)font;
    w9e_shim_sink = x + y + (int)colour + 8;
    return 0;
}

W9E_SHIM int __cdecl Sim_1008d8c0(void *pane, const uint8_t *text, int32_t x, int32_t y, void *font, uint32_t colour)
{
    (void)pane;
    (void)text;
    (void)font;
    w9e_shim_sink = x + y + (int)colour + 9;
    return 0;
}

W9E_SHIM int __cdecl Sim_100ccb0d(const char *s, void *table)
{
    (void)s;
    (void)table;
    w9e_shim_sink = 10;
    return 0;
}

W9E_SHIM int __cdecl Crt_SPrintf(char *buf, const char *format, ...)
{
    (void)buf;
    (void)format;
    w9e_shim_sink = 11;
    return 0;
}

W9E_SHIM int __cdecl Input_GetDevice(const char *name)
{
    (void)name;
    w9e_shim_sink = 12;
    return 0;
}

W9E_SHIM int __cdecl Sim_1005393a(int device, const char *name)
{
    (void)name;
    w9e_shim_sink = device + 13;
    return 0;
}

W9E_SHIM int __cdecl Input_ReadLine(char *buf, int n, uint8_t *stream)
{
    (void)buf;
    (void)stream;
    w9e_shim_sink = n + 14;
    return 0;
}

W9E_SHIM int __cdecl Input_AddAnalog(const uint32_t *src, const char *device, int32_t index)
{
    (void)src;
    (void)device;
    w9e_shim_sink = index + 15;
    return 0;
}

W9E_SHIM int __cdecl Input_ReadChord(void *stream, int32_t *count, uint8_t *out)
{
    (void)stream;
    (void)count;
    (void)out;
    w9e_shim_sink = 16;
    return 0;
}
