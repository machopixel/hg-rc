#include "w12.h"
#include "../simmem.h"
#include "../util/access.h"

/* The C library of the DLL: fopen, fgets, fclose, strstr, strlen, toupper. */
#define W12_FOPEN SIM_FN(void *(__cdecl *)(const char *, const char *), 0x100fd850)
#define W12_FGETS SIM_FN(char *(__cdecl *)(char *, int, void *), 0x100fe770)
#define W12_FCLOSE SIM_FN(int(__cdecl *)(void *), 0x100fd760)
#define W12_STRSTR SIM_FN(char *(__cdecl *)(const char *, const char *), 0x100fc970)
#define W12_STRLEN SIM_FN(uint32_t(__cdecl *)(const char *), 0x100fc8f0)
#define W12_TOUPPER SIM_FN(int(__cdecl *)(int), 0x100fdb80)

// FUNCTION: HGSIM 0x100b9dde
int __cdecl Sim_100b9dde(void)
{
    char line[0x100];
    const char *p;
    void *fp = W12_FOPEN(SIM_PTR(const char, 0x10150e88), SIM_PTR(const char, 0x10150e84));

    if (fp == 0)
        return 0x58;
    line[0] = 0;    /* start value (DECISIONS.md): an empty line when fgets reads nothing */
    p = line;       /* start value (DECISIONS.md): the line's start when it has no "language =" */
    W12_FGETS(line, 0x100, fp);
    W12_FCLOSE(fp);
    if (W12_STRSTR(line, SIM_PTR(const char, 0x10150e94)) != 0) {
        for (p = line + W12_STRLEN(line) - 1; *p == '\n' || *p == ' '; p--)
            ;
    }
    return W12_TOUPPER((int)(signed char)*p);
}