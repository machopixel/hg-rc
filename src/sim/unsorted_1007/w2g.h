/*
 * Wave 2g: two empty stubs of HGSIM.DLL (0x1007cbb4 and 0x1007d941) that
 * ignore their arguments.
 */
#ifndef HG_SIM_W2G_1007_H
#define HG_SIM_W2G_1007_H

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Does nothing. */
void __cdecl Sim_1007cbb4(void);
/* Returns 0. */
int __cdecl Sim_1007d941(void);

#endif
