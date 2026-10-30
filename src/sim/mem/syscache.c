#include <string.h>

#include "mem.h"

// FUNCTION: HGSIM 0x100b9748
int Cache_StampNewer(const unsigned char *a, const unsigned char *b)
{
    unsigned lowA = *(const unsigned *)(a + 0x10), highA = *(const unsigned *)(a + 0x14);
    unsigned lowB = *(const unsigned *)(b + 0x10), highB = *(const unsigned *)(b + 0x14);

    if (highA < highB || (highA == highB && lowA <= lowB))
        return 0;
    return 1;
}

// FUNCTION: HGSIM 0x100ba1c9
char *Cache_FindName(const unsigned char *table, const char *name)
{
    char **names = *(char ***)(table + 8);
    int i;

    for (i = 0; i < *(const int *)(table + 4); i++) {
        char *entry = names[i];

        if (_stricmp(entry, name) == 0)
            return entry;
    }
    return NULL;
}

static MemImports g_imports;

void Mem_SetImports(const MemImports *imports)
{
    g_imports = *imports;
}

// FUNCTION: HGSIM 0x100ba631
void Cache_FreeRecord(unsigned char *record)
{
    if (record == NULL)
        g_imports.assertFail(g_imports.recordAssertExpr, g_imports.recordAssertFile, 0xA5);
    if (*(void **)(record + 8) != NULL)
        g_imports.listFree(*(void **)(record + 8), NULL);
    if (record != NULL)
        g_imports.free(record);
}

// FUNCTION: HGSIM 0x100ba68e
void Cache_FreeHashTable(int *table)
{
    int bucket;
    void(__cdecl *freeItem)(void *);

    if (table == NULL)
        g_imports.assertFail(g_imports.tableAssertExpr, g_imports.tableAssertFile, 0xB3);
    freeItem = (void(__cdecl *)(void *))(size_t)table[8];
    if (table[4] != 0) {
        for (bucket = 0; bucket < table[0]; bucket++) {
            unsigned char *node = ((unsigned char **)(size_t)table[4])[bucket];

            while (node != NULL) {
                unsigned char *next = *(unsigned char **)(node + 0x1C);
                void *item = *(void **)(node + 0x18);

                if (freeItem == NULL) {
                    if (item != NULL)
                        g_imports.free(item);
                } else {
                    freeItem(item);
                }
                g_imports.free(node);
                node = next;
            }
        }
        g_imports.free((void *)(size_t)table[4]);
    }
    if (table[5] != 0) {
        *(int *)((size_t)table[5] + 4) = 0;
        g_imports.listFree((void *)(size_t)table[5], NULL);
    }
    g_imports.free(table);
}
