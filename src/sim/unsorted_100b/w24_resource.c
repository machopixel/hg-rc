#include "w24.h"
#include "../util/access.h"
#include "../unsorted_1004/w5c.h"

#define W24R_LOAD SIM_FN(void *(__cdecl *)(const void *, uint32_t, uint32_t *), 0x100ba7d5)
#define W24R_LOAD_FILE SIM_FN(int(__cdecl *)(const char *, uint32_t *, void **, int), 0x10046ac4)
#define W24R_CLOSE SIM_FN(int(__cdecl *)(int), 0x101049c0)
#define W24R_ALLOC SIM_FN(void *(__cdecl *)(uint32_t, uint32_t), 0x100b659a)
#define W24R_MEMCPY SIM_FN(void *(__cdecl *)(void *, const void *, uint32_t), 0x100fcdc0)
#define W24R_RELEASE SIM_FN(int(__cdecl *)(const void *, uint32_t), 0x100bb2b0)
#define W24R_FORGET SIM_FN(int(__cdecl *)(const void *, uint32_t), 0x100b9203)

// FUNCTION: HGSIM 0x100b8d6c
void *__cdecl Sim_100b8d6c(int16_t *key, uint32_t table, const char *ext, int unused, uint32_t *size,
                           const uint32_t *copy)
{
    char name[9];
    int16_t loose = 0;      /* the original leaves this word unset on the cache path (DECISIONS.md, round 24) */
    void *data, *cached;
    int fd, i;

    (void)unused;
    data = W24R_LOAD(key + 1, table, size);
    if (data == 0) {
        loose = -1;
#ifdef W24R_MUTATE /* the checker's negative check only */
        loose = 0;
#endif
        /* the original appends the extension (strncat, 4) to 8 never-written bytes: taken as empty here */
        for (i = 0; i < 4 && ext[i] != 0; i++)
            name[i] = ext[i];
        name[i] = 0;
        fd = W24R_LOAD_FILE(Sim_100474db(name), size, &data, 0);
        if (fd == -1)
            data = 0;
        else
            W24R_CLOSE(fd);
    }
    if (data != 0 && copy != 0 && (int32_t)*size > 0) {
        cached = data;
        data = W24R_ALLOC(*size, *copy);
        if (data != 0)
            W24R_MEMCPY(data, cached, *size);
        W24R_RELEASE(key + 1, table);
        W24R_FORGET(key + 1, table);
    }
    key[0] = loose;
    return data;
}
