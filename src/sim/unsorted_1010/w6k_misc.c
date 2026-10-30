#include <string.h>
#include "w6k.h"
#include "w2f.h"
#include "../util/access.h"

typedef uint32_t (__stdcall *W6kLoadLibraryAFn)(const char *name);
typedef uint32_t (__stdcall *W6kGetProcAddressFn)(uint32_t module, const char *name);
typedef uint32_t (__stdcall *W6kNoArgFn)(void);
typedef uint32_t (__stdcall *W6kOneArgFn)(uint32_t);
typedef int32_t (__stdcall *W6kMessageBoxFn)(uint32_t owner, const char *text, const char *caption, uint32_t type);

#define IAT_LOADLIBRARYA 0x10ac55e4u
#define IAT_GETPROCADDRESS 0x10ac55dcu
#define IAT_GETACP 0x10ac56b8u
#define IAT_GETOEMCP 0x10ac56bcu

/* Looked up from user32.dll on first use. */
#define FN_MESSAGEBOX G32(0x1017e9a8)
#define FN_GETACTIVEWINDOW G32(0x1017e9ac)
#define FN_GETLASTACTIVEPOPUP G32(0x1017e9b0)

// FUNCTION: HGSIM 0x10107840
int32_t __cdecl Sim_10107840(const char *text, const char *caption, uint32_t type)
{
    uint32_t owner = 0;
    uint32_t lib;

    if (FN_MESSAGEBOX == 0) {
        lib = SIM_API(W6kLoadLibraryAFn, IAT_LOADLIBRARYA)(SIM_PTR(const char, 0x1012c188));   /* "user32.dll" */
        if (lib == 0)
            return 0;
        FN_MESSAGEBOX = SIM_API(W6kGetProcAddressFn, IAT_GETPROCADDRESS)(lib, SIM_PTR(const char, 0x1012c17c));
        if (FN_MESSAGEBOX == 0)
            return 0;
        FN_GETACTIVEWINDOW = SIM_API(W6kGetProcAddressFn, IAT_GETPROCADDRESS)(lib, SIM_PTR(const char, 0x1012c16c));
        FN_GETLASTACTIVEPOPUP = SIM_API(W6kGetProcAddressFn, IAT_GETPROCADDRESS)(lib, SIM_PTR(const char, 0x1012c158));
    }
    if (FN_GETACTIVEWINDOW != 0)
        owner = ((W6kNoArgFn)(uintptr_t)FN_GETACTIVEWINDOW)();
    if (owner != 0 && FN_GETLASTACTIVEPOPUP != 0)
        owner = ((W6kOneArgFn)(uintptr_t)FN_GETLASTACTIVEPOPUP)(owner);
    return ((W6kMessageBoxFn)(uintptr_t)FN_MESSAGEBOX)(owner, text, caption, type);
}

// FUNCTION: HGSIM 0x10107f20
uint32_t __cdecl Sim_10107f20(uint32_t cp)
{
    G32(0x1017ebdc) = 0;
    if (cp == (uint32_t)-2) {
        G32(0x1017ebdc) = 1;
        return SIM_API(W6kNoArgFn, IAT_GETOEMCP)();
    }
    if (cp == (uint32_t)-3) {
        G32(0x1017ebdc) = 1;
        return SIM_API(W6kNoArgFn, IAT_GETACP)();
    }
    if (cp == (uint32_t)-4) {
        cp = G32(0x1017e960);
        G32(0x1017ebdc) = 1;
    }
    return cp;
}

// FUNCTION: HGSIM 0x101083f0
void __cdecl Sim_101083f0(void)
{
    uint32_t mode = G32(0x1017e8fc);
    uint32_t callback;

    if (mode == 1 || (mode == 0 && G32(0x1017e900) == 1)) {
        W6K_FN_10108430(0xfc);
        callback = G32(0x1017ebe8);
        if (callback != 0)
            ((void (__cdecl *)(void))(uintptr_t)callback)();
        W6K_FN_10108430(0xff);
    }
}

// FUNCTION: HGSIM 0x10108e20
int __cdecl Sim_10108e20(uint32_t *out, const char *text)
{
    uint16_t value[6];

    /* the original lets 0x1010b7c0 store its end pointer over its own `text` argument; it is not used after */
    W6K_FN_1010b7c0(value, &text, text, 0, 0, 0, 0);
    return Sim_10108d00(value, out);
}

// FUNCTION: HGSIM 0x10109040
void __cdecl Sim_10109040(void)
{
    W6K_FN_101009b0(2);
}

// FUNCTION: HGSIM 0x10109dd0
uint32_t __cdecl Sim_10109dd0(void)
{
    uint32_t r = G32(0x1017ece0);

    if (r == 0) {
        r = W6K_FN_10109df0();
        G32(0x1017ece0)++;
    }
    return r;
}

#define DAYS_BEFORE_MONTH(leap, m) ((leap) == 0 ? GS32(0x1015c0d4 + 4 * (m)) : GS32(0x1015c10c + 4 * (m)))

// FUNCTION: HGSIM 0x1010a310
void __cdecl Sim_1010a310(int32_t which, int32_t type, int32_t year, int32_t month, int32_t week, int32_t dow,
                          int32_t day, int32_t hour, int32_t min, int32_t sec, int32_t msec)
{
    int32_t leap = year & 3;
    int32_t yday;
    int32_t wd;
    uint32_t t;

    if (type == 1) {
        /* the day of the given weekday in the given week of the month (week 5: the last one) */
        yday = DAYS_BEFORE_MONTH(leap, month) + 1;
        wd = (int32_t)((uint32_t)year * 365u + (uint32_t)yday + (uint32_t)(((int32_t)((uint32_t)year - 1u)) >> 2) -
                       0x63dbu) % 7;
        if (wd < dow)
            yday = (int32_t)((uint32_t)yday + (uint32_t)week * 7u - (uint32_t)wd + (uint32_t)dow - 7u);
        else
            yday = (int32_t)((uint32_t)yday + (uint32_t)week * 7u - (uint32_t)wd + (uint32_t)dow);
        if (week == 5 && yday > DAYS_BEFORE_MONTH(leap, month + 1))
            yday -= 7;
    } else {
        yday = (int32_t)((uint32_t)DAYS_BEFORE_MONTH(leap, month) + (uint32_t)day);
    }
    t = (uint32_t)sec + 60u * ((uint32_t)min + 60u * (uint32_t)hour);
    if (which == 1) {
        G32(0x1015c0bc) = (uint32_t)yday;
        G32(0x1015c0b8) = (uint32_t)year;
        G32(0x1015c0c0) = (uint32_t)msec + 1000u * t;
        return;
    }
    G32(0x1015c0cc) = (uint32_t)yday;
    t = (uint32_t)msec + 1000u * (t + G32(0x1015c028));
    G32(0x1015c0d0) = t;
    if ((int32_t)t < 0) {
        G32(0x1015c0c8) = (uint32_t)year;
        G32(0x1015c0d0) = t + 0x5265bffu;
        return;
    }
    if ((int32_t)t >= 0x5265c00)
        G32(0x1015c0d0) = t - 0x5265bffu;
    G32(0x1015c0c8) = (uint32_t)year;
}

// FUNCTION: HGSIM 0x1010a9c0
int __cdecl Sim_1010a9c0(double x)
{
    uint32_t lo, hi;
    int neg;

    memcpy(&lo, (const char *)&x, 4);
    memcpy(&hi, (const char *)&x + 4, 4);
    neg = (hi & 0x80000000u) != 0;
    if ((hi & 0x7ff00000u) == 0x7ff00000u) {
        switch (W6K_FN_10103940(lo, hi)) {
        case 1:
            return 0x200;
        case 2:
            return 4;
        case 3:
            return 2;
        default:
            return 1;
        }
    }
    if ((hi & 0x7ff00000u) == 0 && ((hi & 0xfffffu) != 0 || lo != 0))
        return neg ? 0x10 : 0x80;
    /* the original compares with the constant 0.0 at 0x1012c490 */
    if (x == 0.0)
        return neg ? 0x20 : 0x40;
    return neg ? 8 : 0x100;
}
