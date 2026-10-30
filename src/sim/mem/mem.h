/*
 * Converted functions of HGSIM.DLL's memory cache module (original source
 * C:\projx\Code\sim\Mem\syscache.c). Plain C with no knowledge of the hook
 * layer: src/hooks/hooks_mem.c puts each one in place of the original.
 * Every function keeps the original calling convention (cdecl).
 */
#ifndef HG_SIM_MEM_H
#define HG_SIM_MEM_H

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Functions of the original DLL that the converted code still calls (its C
 * runtime heap, its assert, its list helper). The hook layer fills this in
 * with their addresses; tests fill it in with stand-ins.
 */
typedef struct MemImports {
    void(__cdecl *free)(void *block);
    void(__cdecl *assertFail)(const char *expr, const char *file, int line);
    /* Frees every item with freeItem (or free when NULL), the array and the list. */
    void(__cdecl *listFree)(void *list, void(__cdecl *freeItem)(void *));
    const char *recordAssertExpr, *recordAssertFile;
    const char *tableAssertExpr, *tableAssertFile;
} MemImports;

void Mem_SetImports(const MemImports *imports);

/* A 64-bit value stored as two 32-bit halves: low word at +0x10, high word at +0x14. */
int Cache_StampNewer(const unsigned char *a, const unsigned char *b);

/* Name table: count at +4, array of C strings at +8. */
char *Cache_FindName(const unsigned char *table, const char *name);

/* Frees a record that owns a list at +8. */
void Cache_FreeRecord(unsigned char *record);

/*
 * Frees a hash table: bucket count at +0, bucket array at +0x10, list at +0x14,
 * item destructor at +0x20. Nodes hold the item at +0x18 and the next node at +0x1c.
 */
void Cache_FreeHashTable(int *table);

/* In-place text helpers used when reading cache description lines. */
void Str_CutAtBlank(char *s);
char *Str_SkipBlanks(char *s);
void Str_TrimEnd(char *s);

#endif
