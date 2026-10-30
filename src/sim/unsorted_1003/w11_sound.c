#include "w11.h"
#include "../simmem.h"
#include "../util/access.h"
#include "w7m.h"
#include "../unsorted_1008/w8w.h"

typedef uint32_t(__stdcall *W11SndMethod1Fn)(uint32_t *self, uint32_t a);
typedef uint32_t(__stdcall *W11SndMethod0Fn)(uint32_t *self);
typedef void(__stdcall *W11CoUninitializeFn)(void);

/* method n (byte offset into the object's table) of the COM object obj */
#define W11_METHOD(type, obj, off) ((type)(uintptr_t)U32(BYTES((obj)[0]), (off)))

// FUNCTION: HGSIM 0x10039708
void __cdecl Sim_10039708(uint32_t *obj, uint32_t value)
{
    if (obj != 0)
        W11_METHOD(W11SndMethod1Fn, obj, 0x44)(obj, value);
}

// FUNCTION: HGSIM 0x1003a548
int __cdecl Sim_1003a548(uint32_t **slot)
{
    if (*slot == 0)
        return 1;
    W11_METHOD(W11SndMethod0Fn, *slot, 8)(*slot);
    *slot = 0;
    SIM_API(W11CoUninitializeFn, 0x10ac58a4)();
    return 1;
}

typedef int32_t(__stdcall *W11SndSetBlockFn)(uint32_t self, void *block, uint32_t apply);

// FUNCTION: HGSIM 0x10039931
void __cdecl Sim_10039931(int idx, const uint8_t *unit)
{
    const uint8_t *rec;
    uint32_t obj;
    int32_t hr;

    if (G32(0x1015d0a4) == 0 || U32(BYTES(G32(0x1015d0a4)), 0xc) == 0 || unit == 0)
        return;
    rec = BYTES(U32(unit, 0x28));
    G32(0x109e3724) = U32(unit, 0x58);
    G32(0x109e3728) = U32(unit, 0x5c);
    G32(0x109e372c) = U32(unit, 0x60);
    G32(0x109e3730) = U32(rec, 0x548);
    G32(0x109e3734) = U32(rec, 0x54c);
    G32(0x109e3738) = U32(rec, 0x550);
    obj = U32(BYTES(G32(0x1015d0a4)), 0x114 + (uint32_t)idx * 4);
    hr = ((W11SndSetBlockFn)(uintptr_t)U32(BYTES(U32(BYTES(obj), 0)), 0x30))(obj, SIM_PTR(void, 0x109e3720), 1);
    if (hr < 0)
        Sound_ErrorName(hr);
}

typedef int32_t(__stdcall *W11SndCommitFn)(uint32_t self);

// FUNCTION: HGSIM 0x100397e7
void __cdecl Sim_100397e7(void)
{
    const uint8_t *rec = BYTES(U32(PTRTAB(G32(0x1017b8c8)), 0x28));
    const uint8_t *cam;
    uint32_t lis;
    int32_t hr;

    if (G32(0x1015d0a4) == 0 || U32(BYTES(G32(0x1015d0a4)), 0xc) == 0)
        return;
    cam = BYTES(G32(0x1014cae0));
    G32(0x109e36e4) = U32(cam, 0);
    G32(0x109e36e8) = U32(cam, 4);
    G32(0x109e36ec) = U32(cam, 8);
    G32(0x109e36f0) = U32(rec, 0x548);
    G32(0x109e36f4) = U32(rec, 0x54c);
    G32(0x109e36f8) = U32(rec, 0x550);
    *SIM_PTR(float, 0x109e36fc) = Sim_10084766(*(const float *)(cam + 0xc));
    G32(0x109e3700) = 0;
    *SIM_PTR(float, 0x109e3704) =
        Sim_10084766((float)((double)*(const float *)(BYTES(G32(0x1014cae0)) + 0xc) +
                             (double)*SIM_PTR(const float, 0x10111498)));
    G32(0x109e3708) = 0;
    G32(0x109e370c) = 0x3f800000u;
    G32(0x109e3710) = 0;
    lis = U32(BYTES(G32(0x1015d0a4)), 0xc);
    hr = ((W11SndSetBlockFn)(uintptr_t)U32(BYTES(U32(BYTES(lis), 0)), 0x28))(lis, SIM_PTR(void, 0x109e36e0), 1);
    if (hr < 0)
        Sound_ErrorName(hr);
    lis = U32(BYTES(G32(0x1015d0a4)), 0xc);
    hr = ((W11SndCommitFn)(uintptr_t)U32(BYTES(U32(BYTES(lis), 0)), 0x44))(lis);
    if (hr < 0)
        Sound_ErrorName(hr);
}

typedef uint32_t(__stdcall *W11GetProcAddressFn)(uint32_t module, const char *name);

/* the names of the library functions, in the order of their slots 0x1015ea8c.. */
static const uint32_t g_w11NetNames[55] = {
    0x1013a794, 0x1013a7a4, 0x1013a7b8, 0x1013a7cc, 0x1013a7d8, 0x1013a7e4, 0x1013a7f4, 0x1013a808, 0x1013a81c,
    0x1013a828, 0x1013a838, 0x1013a840, 0x1013a84c, 0x1013a854, 0x1013a85c, 0x1013a870, 0x1013a880, 0x1013a890,
    0x1013a8a0, 0x1013a8b0, 0x1013a8c0, 0x1013a8d0, 0x1013a8e0, 0x1013a8f4, 0x1013a900, 0x1013a910, 0x1013a920,
    0x1013a930, 0x1013a944, 0x1013a95c, 0x1013a970, 0x1013a980, 0x1013a990, 0x1013a9a0, 0x1013a9ac, 0x1013a9bc,
    0x1013a9c8, 0x1013a9d4, 0x1013a9e4, 0x1013a9f8, 0x1013aa0c, 0x1013aa18, 0x1013aa24, 0x1013aa30, 0x1013aa40,
    0x1013aa50, 0x1013aa60, 0x1013aa6c, 0x1013aa7c, 0x1013aa8c, 0x1013aaa0, 0x1013aab4, 0x1013aac0, 0x1013aad4,
    0x1013aae4,
};

// FUNCTION: HGSIM 0x1003be30
void __cdecl Sim_1003be30(void)
{
    int k;

    for (k = 0; k < 55; k++)
        G32(0x1015ea8c + 4 * (uint32_t)k) =
            SIM_API(W11GetProcAddressFn, 0x10ac55dc)(G32(0x1015eb68), SIM_PTR(const char, g_w11NetNames[k]));
}
