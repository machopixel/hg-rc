#include "strtable.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_MAX 256
#define WHITESPACE " \n\r\t\v\f"

/* :OPTIONS: flags, set by keywords and applied to every later translation. */
#define OPT_ELBONIAN 0x1 /* add accents after vowels, for testing */
#define OPT_STARS    0x2 /* replace text with stars, framed by $...$ */
#define OPT_LINENUM  0x4 /* prefix with the source line number */
#define OPT_XTDELBON 0x8 /* same as OPT_ELBONIAN */

/* Files without a .txt extension are encrypted, byte by byte, by position. */
typedef struct EncFile {
    FILE *fp;
    unsigned long pos;
    unsigned long reserved;
} EncFile;

// FUNCTION: HG 0x401370
static void Decrypt(unsigned char *buf, unsigned long len, unsigned long pos)
{
    unsigned char key = (unsigned char)(pos * 0x29);
    unsigned char base = (unsigned char)(pos / 70 + 0x35);
    unsigned char run = (unsigned char)(pos % 70);

    while (len-- != 0) {
        unsigned char c = (unsigned char)((unsigned char)(*buf - base) ^ key);

        if (c & 1)
            c -= 0x6c;
        *buf++ = c;
        key += 0x29;
        if (++run == 70) {
            run = 0;
            base++;
        }
    }
}

// FUNCTION: HG 0x4014b0
static EncFile *EncOpen(const char *path)
{
    EncFile *f = malloc(sizeof(*f));

    if (f == NULL)
        return NULL;
    f->fp = fopen(path, "rb");
    if (f->fp == NULL) {
        free(f);
        return NULL;
    }
    f->pos = 0;
    f->reserved = 0;
    return f;
}

// FUNCTION: HG 0x401500
static size_t EncRead(EncFile *f, unsigned char *buf, size_t count)
{
    size_t n = fread(buf, 1, count, f->fp);

    Decrypt(buf, (unsigned long)n, f->pos);
    f->pos += (unsigned long)n;
    return n;
}

// FUNCTION: HG 0x401540
static int EncClose(EncFile *f)
{
    int result = fclose(f->fp);

    if (result == 0)
        free(f);
    return result;
}

/* Returns 0 on success, 1 to 5 for the step that failed. */
// FUNCTION: HG 0x401400
static int DecryptFile(const char *dstPath, const char *srcPath)
{
    EncFile *src;
    FILE *dst;
    unsigned char c;

    src = EncOpen(srcPath);
    if (src == NULL)
        return 1;
    dst = fopen(dstPath, "wb");
    if (dst == NULL)
        return 2;
    while (EncRead(src, &c, 1) == 1) {
        if (fwrite(&c, 1, 1, dst) != 1)
            return 3;
    }
    if (fclose(dst) != 0)
        return 4;
    return EncClose(src) != 0 ? 5 : 0;
}

/*
 * Resolves C escapes in place. Raw line breaks are dropped, so a block's
 * line structure only survives through \n.
 */
// FUNCTION: HG 0x401ae0
static char *Unescape(char *s)
{
    int in = 0;
    int out = 0;
    char digits[4];
    int value;
    int n;

    if (s == NULL)
        return NULL;
    while (s[in] != '\0') {
        char c = s[in];

        if (c >= '\n' && c <= '\r') {
            in++;
            continue;
        }
        if (c != '\\') {
            s[out++] = c;
            in++;
            continue;
        }
        in++;
        switch (s[in]) {
        case 'a':  s[out++] = '\a'; in++; break;
        case 'b':  s[out++] = '\b'; in++; break;
        case 'f':  s[out++] = '\f'; in++; break;
        case 'n':  s[out++] = '\n'; in++; break;
        case 'r':  s[out++] = '\r'; in++; break;
        case 't':  s[out++] = '\t'; in++; break;
        case 'v':  s[out++] = '\v'; in++; break;
        case '\'': s[out++] = '\''; in++; break;
        case '"':  s[out++] = '"';  in++; break;
        case '\\': s[out++] = '\\'; in++; break;
        case '?':  s[out++] = '?';  in++; break;
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
            n = 0;
            while (isdigit((unsigned char)s[in]) && s[in] != '8' && s[in] != '9' && n < 3)
                digits[n++] = s[in++];
            digits[n] = '\0';
            sscanf(digits, "%o", &value);
            s[out++] = (char)value;
            break;
        case 'x':
            in++;
            n = 0;
            while (isxdigit((unsigned char)s[in]) && n < 3)
                digits[n++] = s[in++];
            digits[n] = '\0';
            if (n != 0) {
                sscanf(digits, "%x", &value);
                s[out++] = (char)value;
            } else {
                s[out++] = '\\';
                s[out++] = 'x';
            }
            break;
        case '\0':
            /* A trailing backslash is dropped. */
            break;
        default:
            s[out++] = '\\';
            s[out++] = s[in++];
            break;
        }
    }
    s[out] = '\0';
    return s;
}

/* Reads lines up to the next blank line and joins them with spaces. */
// FUNCTION: HG 0x401990
static char *ReadBlock(FILE *fp, int *lineNo)
{
    char line[LINE_MAX];
    char *text = NULL;

    while (fgets(line, sizeof(line), fp) != NULL) {
        size_t len = strlen(line);

        ++*lineNo;
        if (strspn(line, WHITESPACE) == len)
            return Unescape(text);
        if (text != NULL) {
            text = realloc(text, strlen(text) + len + 2);
            if (text == NULL)
                return NULL;
            strcat(text, " ");
        } else {
            text = malloc(len + 1);
            if (text == NULL)
                return NULL;
            text[0] = '\0';
        }
        strcat(text, line);
    }
    return text;
}

// FUNCTION: HG 0x401d80
static void ApplyOptions(char **english, char **text, unsigned flags, int lineNo)
{
    (void)english;

    if (flags & (OPT_ELBONIAN | OPT_XTDELBON)) {
        const char *src = *text;
        size_t len = strlen(src);
        /* The original allocated len * 2 + 1, one byte short for the '&'. */
        char *dst = malloc(len * 2 + 2);
        size_t i;
        size_t o = 0;

        if (dst == NULL)
            return;
        for (i = 0; src[i] != '\0'; i++) {
            dst[o++] = src[i];
            switch (src[i]) {
            case 'A': dst[o++] = '\xc4'; break;
            case 'a': dst[o++] = '\xe4'; break;
            case 'E': dst[o++] = '\xcb'; break;
            case 'e': dst[o++] = '\xeb'; break;
            case 'I': dst[o++] = '\xcf'; break;
            case 'i': dst[o++] = '\xef'; break;
            case 'O': dst[o++] = '\xd6'; break;
            case 'o': dst[o++] = '\xf6'; break;
            case 'U': dst[o++] = '\xdc'; break;
            case 'u': dst[o++] = '\xfc'; break;
            case 'y': dst[o++] = '\xff'; break;
            case '?': dst[o++] = '\xbf'; break;
            case '!': dst[o++] = '\xa1'; break;
            }
        }
        dst[o] = '&';
        dst[o + 1] = '\0';
        free(*text);
        *text = dst;
    }

    if (flags & OPT_STARS) {
        char *s = *text;
        int len = (int)strlen(s);
        int i;

        for (i = 0; i < len; i++) {
            if (strchr(WHITESPACE, s[i]) == NULL)
                s[i] = '*';
        }
        s[0] = '$';
        s[len - 1] = '$';
    }

    if (flags & OPT_LINENUM) {
        char *numbered = malloc(strlen(*text) + 9);

        if (numbered == NULL)
            return;
        sprintf(numbered, "%d: %s", lineNo, *text);
        free(*text);
        *text = numbered;
    }
}

/* Reads the next :ENGLISH: / :TRANSLATED: pair. Returns 0 at end of file. */
// FUNCTION: HG 0x401740
static int ReadEntry(FILE *fp, char **english, char **text, unsigned *flags, int *lineNo)
{
    char line[LINE_MAX];

    while (fgets(line, sizeof(line), fp) != NULL) {
        ++*lineNo;
        if (_strnicmp(line, ":ENGLISH:", 9) == 0) {
            *english = ReadBlock(fp, lineNo);
        } else if (_strnicmp(line, ":TRANSLATED:", 12) == 0) {
            *text = ReadBlock(fp, lineNo);
            if (*english != NULL) {
                if (*text == NULL) {
                    *text = malloc(strlen(*english) + 1);
                    if (*text == NULL)
                        return 0;
                    strcpy(*text, *english);
                }
                if (*flags != 0)
                    ApplyOptions(english, text, *flags, *lineNo);
                return 1;
            }
        } else if (_strnicmp(line, ":OPTIONS:", 9) == 0) {
            char *options = ReadBlock(fp, lineNo);
            int i;

            if (options == NULL)
                continue;
            for (i = 0; i < (int)strlen(options); i++) {
                if (isupper((unsigned char)options[i]))
                    options[i] += 0x20;
            }
            if (strstr(options, "elbonian"))
                *flags |= OPT_ELBONIAN;
            if (strstr(options, "stars"))
                *flags |= OPT_STARS;
            if (strstr(options, "linenum"))
                *flags |= OPT_LINENUM;
            if (strstr(options, "xtdelbon"))
                *flags |= OPT_XTDELBON;
            free(options);
        }
    }
    return 0;
}

// FUNCTION: HG 0x401700
static int CompareEntries(const void *a, const void *b)
{
    return strcmp(((const StrEntry *)a)->english, ((const StrEntry *)b)->english);
}

// FUNCTION: HG 0x401570
StrTable *StrTable_Load(const char *path)
{
    StrTable *table;
    FILE *fp;
    char *english = NULL;
    char *text = NULL;
    int lineNo = 0;
    unsigned flags = 0;
    int capacity = 0;
    int usedTemp = 0;

    if (path == NULL)
        path = "lang.txt";
    if (strstr(path, ".txt") == NULL && DecryptFile("temp.txt", path) == 0) {
        path = "temp.txt";
        usedTemp = 1;
    }

    fp = fopen(path, "r");
    if (fp == NULL) {
        table = NULL;
    } else {
        table = calloc(sizeof(StrTable), 1);
        if (table != NULL) {
            while (ReadEntry(fp, &english, &text, &flags, &lineNo)) {
                if (table->count >= capacity) {
                    StrEntry *grown;

                    capacity = capacity != 0 ? capacity * 2 : 100;
                    grown = realloc(table->entries, capacity * sizeof(StrEntry));
                    if (grown == NULL) {
                        free(english);
                        free(text);
                        break;
                    }
                    table->entries = grown;
                }
                table->entries[table->count].english = english;
                table->entries[table->count].translated = text;
                table->count++;
                english = NULL;
                text = NULL;
            }
            qsort(table->entries, table->count, sizeof(StrEntry), CompareEntries);
        }
        fclose(fp);
    }

    if (usedTemp)
        remove(path);
    return table;
}

// FUNCTION: HG 0x401f90
void StrTable_Free(StrTable *table)
{
    int i;

    if (table == NULL)
        return;
    if (table->entries != NULL) {
        for (i = 0; i < table->count; i++) {
            free(table->entries[i].english);
            free(table->entries[i].translated);
        }
        free(table->entries);
    }
    free(table);
}

/* Returns the translation, or the English text itself when there is none. */
// FUNCTION: HG 0x401ff0
const char *StrTable_Lookup(StrTable *table, const char *english)
{
    StrEntry key;
    const StrEntry *found;

    if (table == NULL)
        return english;
    key.english = (char *)english;
    found = bsearch(&key, table->entries, table->count, sizeof(StrEntry), CompareEntries);
    return found != NULL ? found->translated : english;
}
