/*
 * Round 4, letter w: function pointer types and import slot addresses of the
 * Windows functions that the converted code calls through the DLL's import
 * table (see SIM_API in simmem.h). Slot addresses are those of the preferred base.
 */
#ifndef HG_SIM_WIN_W4W_API_H
#define HG_SIM_WIN_W4W_API_H

#include <stdint.h>
#include "../simmem.h"

typedef int (__stdcall *W4wFreeLibraryFn)(void *module);
typedef uint32_t (__stdcall *W4wGetLastErrorFn)(void);
typedef int (__stdcall *W4wWvsprintfFn)(char *out, const char *format, void *args);
typedef int (__stdcall *W4wBoolPtrFn)(void *a);
typedef int (__stdcall *W4wBool2Fn)(void *a, void *b);
typedef int (__stdcall *W4wIntFn)(int a);
typedef int (__stdcall *W4wInt2Fn)(int a, int b);
typedef void *(__stdcall *W4wPtrFn)(void);
typedef int (__stdcall *W4wSendMessageFn)(void *hwnd, uint32_t msg, uint32_t wparam, void *lparam);
typedef int (__stdcall *W4wPeekMessageFn)(void *msg, void *hwnd, uint32_t min, uint32_t max, uint32_t remove);
typedef int (__stdcall *W4wStretchBltFn)(void *dst, int xd, int yd, int wd, int hd, void *src, int xs, int ys, int ws,
                                         int hs, uint32_t rop);
typedef int (__stdcall *W4wSetDIBitsFn)(void *dc, int xd, int yd, uint32_t w, uint32_t h, int xs, int ys,
                                        uint32_t start, uint32_t lines, const void *bits, const void *info,
                                        uint32_t usage);
typedef void *(__stdcall *W4wSelectObjectFn)(void *dc, void *obj);
typedef int (__stdcall *W4wReleaseDCFn)(void *hwnd, void *dc);
typedef int (__stdcall *W4wMessageBoxFn)(void *hwnd, const char *text, const char *caption, uint32_t type);
typedef uint32_t (__stdcall *W4wU32IntFn)(int a);

#define W4W_IAT_STRETCHBLT 0x10ac5578u
#define W4W_IAT_SETDIBITSTODEVICE 0x10ac5570u
#define W4W_IAT_SELECTOBJECT 0x10ac5590u
#define W4W_IAT_DELETEOBJECT 0x10ac5594u
#define W4W_IAT_FREELIBRARY 0x10ac55d8u
#define W4W_IAT_GETLASTERROR 0x10ac5600u
#define W4W_IAT_GETFOCUS 0x10ac5778u
#define W4W_IAT_GETSYSTEMMETRICS 0x10ac577cu
#define W4W_IAT_PEEKMESSAGE 0x10ac5780u
#define W4W_IAT_TRANSLATEMESSAGE 0x10ac5784u
#define W4W_IAT_CLIPCURSOR 0x10ac5790u
#define W4W_IAT_SETCURSORPOS 0x10ac5794u
#define W4W_IAT_CLIENTTOSCREEN 0x10ac5798u
#define W4W_IAT_SCREENTOCLIENT 0x10ac579cu
#define W4W_IAT_GETCURSORPOS 0x10ac57a0u
#define W4W_IAT_GETASYNCKEYSTATE 0x10ac57a4u
#define W4W_IAT_RELEASEDC 0x10ac57a8u
#define W4W_IAT_SENDMESSAGE 0x10ac57b8u
#define W4W_IAT_DISPATCHMESSAGE 0x10ac57ccu
#define W4W_IAT_WVSPRINTF 0x10ac57d4u
#define W4W_IAT_MESSAGEBEEP 0x10ac57d8u
#define W4W_IAT_MESSAGEBOX 0x10ac57e4u
#define W4W_IAT_WAITMESSAGE 0x10ac57e8u
#define W4W_IAT_AUXSETVOLUME 0x10ac5848u
#define W4W_IAT_JOYGETNUMDEVS 0x10ac5864u

/* The game's heap handle. */
#endif
