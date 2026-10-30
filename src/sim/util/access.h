/* Typed access at a byte offset, for the structures of the original DLL whose layout is only known as offsets. */
#ifndef HG_SIM_UTIL_ACCESS_H
#define HG_SIM_UTIL_ACCESS_H

#include <stdint.h>
#include "../simmem.h"

#ifdef __GNUC__
#define HG_MAY_ALIAS __attribute__((may_alias))
#else
#define HG_MAY_ALIAS
#endif

/* The original never reorders its accesses, so these types may alias anything. */
typedef uint32_t HG_MAY_ALIAS u32a;
typedef int32_t HG_MAY_ALIAS s32a;
typedef uint16_t HG_MAY_ALIAS u16a;
typedef int16_t HG_MAY_ALIAS s16a;

#define BYTES(p) ((uint8_t *)(uintptr_t)(p))
#define U32(p, off) (*(u32a *)(BYTES(p) + (off)))
#define S32(p, off) (*(s32a *)(BYTES(p) + (off)))
#define U16(p, off) (*(u16a *)(BYTES(p) + (off)))
#define S16(p, off) (*(s16a *)(BYTES(p) + (off)))
#define U8(p, off) (*(BYTES(p) + (off)))

/* Dword / pointer of the DLL's data at its preferred-base address. */
#define G32(addr) (*SIM_PTR(u32a, addr))
#define GS32(addr) (*SIM_PTR(s32a, addr))
#define GBYTES(addr) SIM_PTR(uint8_t, addr)

/* The pointer table at 0x109e8b40. */
#define PTRTAB(i) (SIM_PTR(u32a, 0x109e8b40)[(i)])

#endif
