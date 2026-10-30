/*
 * Test entries for the checker only (spec_w3s.py): functions that return in st(0) are wrapped so
 * that the value is stored at a fixed address (the spec's trampolines do the same for the original),
 * and the 16-bit result of Sim_100beee4 is sign-extended (its callers store ax only).
 */
#include "sim/unsorted_1000/w3s.h"
#include "sim/unsorted_1002/w3s.h"
#include "sim/unsorted_1003/w3s.h"
#include "sim/unsorted_1004/w3s.h"
#include "sim/unsorted_100b/w3s.h"

#define OUT (*(volatile double *)0xAF0F00)

int __cdecl Test_10008eac(const void *obj)
{
    OUT = (double)Sim_10008eac(obj);
    return 0;
}

int __cdecl Test_1002db2d(float x, float y, float z)
{
    OUT = (double)Sim_1002db2d(x, y, z);
    return 0;
}

int __cdecl Test_100335fd(const void *entry, float x, float y, float z)
{
    OUT = Entry_DistSq(entry, x, y, z);
    return 0;
}

int __cdecl Test_1004487c(const float *p)
{
    OUT = (double)Sim_1004487c(p);
    return 0;
}

int __cdecl Test_100beee4(void *obj)
{
    return (int)Sim_100beee4(obj);
}
