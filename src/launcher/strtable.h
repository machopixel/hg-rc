/*
 * Translation tables ("shell.txt", "lang.txt"): pairs of English and
 * translated text, looked up by the English original.
 */
#ifndef HG_LAUNCHER_STRTABLE_H
#define HG_LAUNCHER_STRTABLE_H

typedef struct StrEntry {
    char *english;
    char *translated;
} StrEntry;

typedef struct StrTable {
    int count;
    StrEntry *entries; /* sorted by english, for bsearch */
} StrTable;

StrTable *StrTable_Load(const char *path);
const char *StrTable_Lookup(StrTable *table, const char *english);
void StrTable_Free(StrTable *table);

#endif
