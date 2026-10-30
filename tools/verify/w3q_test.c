/* Test entry for the checker only (spec_w3q.py): Sim_100d6e99 returns in st(0), which the checker cannot see,
 * so this wrapper stores the double at a fixed address of the argument memory (the spec does the same
 * for the original with a small trampoline). */
#include "sim/unsorted_100d/w3q.h"

void __cdecl Test_100d6e99(const void *obj)
{
    *(volatile double *)0x00AFF000 = Sim_100d6e99(obj);
}

/*
 * The linked files unsorted_100b/fields.c and unsorted_100a/w2h_misc.c also hold functions that call
 * these three converted functions; none of those is reached by the tests, so empty stand-ins keep the
 * image small instead of pulling their whole call chains in.
 */
void __cdecl Sim_1003a52b(unsigned v) {}
int __cdecl Sim_10079feb(int v) { return 0; }
int __cdecl Sim_10015bc9(void) { return 0; }
