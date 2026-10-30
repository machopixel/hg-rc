/* Wave 2e, original 0x10060209. */
#ifndef HG_SIM_UNSORTED_1006_W2E_H
#define HG_SIM_UNSORTED_1006_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Mem_Move(dst, src, 0x30) through Sim_10028571; returns dst. */
void *__cdecl Sim_10060209(const void *src, void *dst);

#endif
