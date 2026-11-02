/*
 * Files for the game's C runtime (plat.h, "files"): standard C files, always binary, and SDL3 to list folders. The
 * same code on Windows, Linux and Mac; the only system difference is the name mapping (plat_file_path).
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>
#include "plat.h"

#define PLAT_FILES 128
#define PLAT_FINDS 16
#define PLAT_PATH 1024
#define PLAT_HANDLE_BASE 0x48470000u    /* "GH..": handle = base + slot index, never 0 or PLAT_FILE_INVALID */
#define PLAT_FIND_BASE 0x46440000u

enum { PLAT_LAST_NONE, PLAT_LAST_READ, PLAT_LAST_WRITE };

typedef struct PlatFile {
    FILE *f;
    int last;               /* the last transfer: C needs a seek between a read and a write */
    int remove_on_close;    /* CreateFile's FILE_FLAG_DELETE_ON_CLOSE (the C runtime's _O_TEMPORARY) */
    uint32_t access;        /* CreateFile's access: a read or write it does not allow fails as on Windows */
    char path[PLAT_PATH];
} PlatFile;

typedef struct PlatFind {
    char **names;           /* sorted, the matching entries of the folder */
    int count, next;
    char dir[PLAT_PATH];
} PlatFind;

static PlatFile g_platFiles[PLAT_FILES];
static PlatFind g_platFinds[PLAT_FINDS];
static uint32_t g_platFileError;

uint32_t Plat_FileLastError(void)
{
    return g_platFileError;
}

static uint32_t plat_file_fail(uint32_t code)
{
    g_platFileError = code;
    return 0;
}

/* Windows' error number for the C library's errno after a failed fopen / remove */
static uint32_t plat_file_errno(void)
{
    switch (errno) {
    case ENOENT: return 2;
    case EMFILE: return 4;
    case EEXIST: return 80;
    case ENOSPC: return 112;
    default: return 5;
    }
}

/* ---- names ----------------------------------------------------------------------------------------------- */

static int plat_file_icmp(const char *a, const char *b)
{
    for (;; a++, b++) {
        int x = (unsigned char)*a, y = (unsigned char)*b;

        if (x >= 'A' && x <= 'Z')
            x += 'a' - 'A';
        if (y >= 'A' && y <= 'Z')
            y += 'a' - 'A';
        if (x != y || x == 0)
            return x - y;
    }
}

typedef struct PlatFileList {
    char **names;
    int count, size;
} PlatFileList;

static SDL_EnumerationResult SDLCALL plat_file_collect(void *ud, const char *dir, const char *name)
{
    PlatFileList *list = (PlatFileList *)ud;
    char *copy;

    (void)dir;
    if (list->count == list->size) {
        int size = list->size ? list->size * 2 : 32;
        char **names = (char **)realloc(list->names, sizeof(char *) * (size_t)size);

        if (names == NULL)
            return SDL_ENUM_FAILURE;
        list->names = names;
        list->size = size;
    }
    copy = (char *)malloc(strlen(name) + 1);
    if (copy == NULL)
        return SDL_ENUM_FAILURE;
    strcpy(copy, name);
    list->names[list->count++] = copy;
    return SDL_ENUM_CONTINUE;
}

static int plat_file_sortcmp(const void *a, const void *b)
{
    int d = plat_file_icmp(*(char *const *)a, *(char *const *)b);

    return d != 0 ? d : strcmp(*(char *const *)a, *(char *const *)b);
}

/* The entries of folder dir, sorted ignoring case (the same order on every system); count < 0 when it cannot be read. */
static PlatFileList plat_file_list(const char *dir)
{
    PlatFileList list = {NULL, 0, 0};

    if (!SDL_EnumerateDirectory(dir[0] != 0 ? dir : ".", plat_file_collect, &list)) {
        int i;

        for (i = 0; i < list.count; i++)
            free(list.names[i]);
        free(list.names);
        list.names = NULL;
        list.count = -1;
        return list;
    }
    if (list.count > 1)
        qsort(list.names, (size_t)list.count, sizeof(char *), plat_file_sortcmp);
    return list;
}

static void plat_file_unlist(PlatFileList *list)
{
    int i;

    for (i = 0; i < list->count; i++)
        free(list->names[i]);
    free(list->names);
}

static int plat_file_exists(const char *path)
{
    return SDL_GetPathInfo(path, NULL);
}

/*
 * The game's file name as a path of this system into out: "\" becomes "/"; on other systems than Windows the drive
 * ("C:") goes, and a name that does not exist as written is looked up folder by folder ignoring letter case (the
 * first match in sorted order); parts that match nothing are kept as written (a new file is created so).
 */
static void plat_file_path(const char *name, char *out, size_t size)
{
    size_t n = 0;

#ifndef _WIN32
    if (((name[0] | 0x20) >= 'a' && (name[0] | 0x20) <= 'z') && name[1] == ':')
        name += 2;
#endif
    for (; *name != 0 && n + 1 < size; name++)
        out[n++] = *name == '\\' ? '/' : *name;
    out[n] = 0;
#ifndef _WIN32
    if (out[0] != 0 && !plat_file_exists(out)) {
        char fixed[PLAT_PATH];
        const char *p = out;
        size_t m = 0;

        if (*p == '/') {
            fixed[m++] = '/';
            p++;
        }
        fixed[m] = 0;
        while (*p != 0) {
            const char *end = strchr(p, '/');
            size_t len = end != NULL ? (size_t)(end - p) : strlen(p);
            char part[PLAT_PATH];

            if (len >= sizeof part || m + len + 2 >= sizeof fixed)
                return;
            memcpy(part, p, len);
            part[len] = 0;
            memcpy(fixed + m, part, len + 1);
            if (!plat_file_exists(fixed)) {
                char dir[PLAT_PATH];
                PlatFileList list;
                int i;

                memcpy(dir, fixed, m);
                dir[m] = 0;
                list = plat_file_list(dir);
                for (i = 0; i < list.count; i++) {
                    if (plat_file_icmp(list.names[i], part) == 0 && strlen(list.names[i]) == len) {
                        memcpy(fixed + m, list.names[i], len + 1);
                        break;
                    }
                }
                plat_file_unlist(&list);
            }
            m += len;
            p += len;
            if (*p == '/') {
                fixed[m++] = '/';
                fixed[m] = 0;
                p++;
            }
        }
        if (m < size)
            memcpy(out, fixed, m + 1);
    }
#endif
}

/* ---- open files ------------------------------------------------------------------------------------------ */

static PlatFile *plat_file_get(uint32_t handle)
{
    uint32_t i = handle - PLAT_HANDLE_BASE - 1;

    if (i >= PLAT_FILES || g_platFiles[i].f == NULL) {
        g_platFileError = 6;    /* ERROR_INVALID_HANDLE */
        return NULL;
    }
    return &g_platFiles[i];
}

uint32_t Plat_FileCreate(const char *name, uint32_t access, uint32_t share, void *security, uint32_t disposition,
                         uint32_t flags, uint32_t tmpl)
{
    char path[PLAT_PATH];
    SDL_PathInfo info;
    int exists, writing = (access & 0x40000000u) != 0;
    const char *mode;
    FILE *f;
    uint32_t i;

    (void)share, (void)security, (void)tmpl;
    plat_file_path(name, path, sizeof path);
    for (i = 0; i < PLAT_FILES && g_platFiles[i].f != NULL; i++)
        ;
    if (i == PLAT_FILES) {
        g_platFileError = 4;    /* ERROR_TOO_MANY_OPEN_FILES */
        return PLAT_FILE_INVALID;
    }
    if (disposition == 5 && !writing) {
        g_platFileError = 87;   /* truncating needs write access: Windows refuses it before looking at the file */
        return PLAT_FILE_INVALID;
    }
    exists = SDL_GetPathInfo(path, &info);
    if (exists && info.type == SDL_PATHTYPE_DIRECTORY) {
        g_platFileError = 5;    /* a folder: Windows answers "access denied" */
        return PLAT_FILE_INVALID;
    }
    switch (disposition) {
    case 1:                     /* CREATE_NEW */
        if (exists) {
            g_platFileError = 80;
            return PLAT_FILE_INVALID;
        }
        mode = "w+b";
        break;
    case 2:                     /* CREATE_ALWAYS */
        mode = "w+b";
        break;
    case 4:                     /* OPEN_ALWAYS */
        mode = exists ? (writing ? "r+b" : "rb") : "w+b";
        break;
    case 3:                     /* OPEN_EXISTING */
    case 5:                     /* TRUNCATE_EXISTING */
        if (!exists) {
            char dir[PLAT_PATH];
            char *slash;

            strcpy(dir, path);
            slash = strrchr(dir, '/');
            if (slash != NULL && slash != dir) {
                *slash = 0;
                g_platFileError = plat_file_exists(dir) ? 2 : 3;
            } else {
                g_platFileError = 2;
            }
            return PLAT_FILE_INVALID;
        }
        mode = disposition == 5 ? "w+b" : writing ? "r+b" : "rb";
        break;
    default:
        g_platFileError = 87;   /* ERROR_INVALID_PARAMETER */
        return PLAT_FILE_INVALID;
    }
    errno = 0;
    f = fopen(path, mode);
    if (f == NULL) {
        g_platFileError = plat_file_errno();
        return PLAT_FILE_INVALID;
    }
    g_platFiles[i].f = f;
    g_platFiles[i].last = PLAT_LAST_NONE;
    g_platFiles[i].remove_on_close = (flags & 0x04000000u) != 0;
    g_platFiles[i].access = access;
    strcpy(g_platFiles[i].path, path);
    g_platFileError = 0;
    return PLAT_HANDLE_BASE + i + 1;
}

int32_t Plat_FileRead(uint32_t handle, void *data, uint32_t size, uint32_t *done, void *overlapped)
{
    PlatFile *pf = plat_file_get(handle);
    size_t got;

    (void)overlapped;
    if (done != NULL)
        *done = 0;
    if (pf == NULL)
        return 0;
    if (!(pf->access & 0x80000000u))
        return (int32_t)plat_file_fail(5);
    if (pf->last == PLAT_LAST_WRITE)
        fseek(pf->f, 0, SEEK_CUR);
    pf->last = PLAT_LAST_READ;
    got = fread(data, 1, size, pf->f);
    if (done != NULL)
        *done = (uint32_t)got;
    if (got < size && ferror(pf->f)) {
        clearerr(pf->f);
        return (int32_t)plat_file_fail(5);
    }
    clearerr(pf->f);            /* the end of the file is not an error for ReadFile */
    return 1;
}

int32_t Plat_FileWrite(uint32_t handle, const void *data, uint32_t size, uint32_t *done, void *overlapped)
{
    PlatFile *pf = plat_file_get(handle);
    size_t put;

    (void)overlapped;
    if (done != NULL)
        *done = 0;
    if (pf == NULL)
        return 0;
    if (!(pf->access & 0x40000000u))
        return (int32_t)plat_file_fail(5);
    if (pf->last == PLAT_LAST_READ)
        fseek(pf->f, 0, SEEK_CUR);
    pf->last = PLAT_LAST_WRITE;
    put = fwrite(data, 1, size, pf->f);
    if (done != NULL)
        *done = (uint32_t)put;
    if (put < size) {
        clearerr(pf->f);
        return (int32_t)plat_file_fail(errno == ENOSPC ? 112 : 5);
    }
    return 1;
}

uint32_t Plat_FileSeek(uint32_t handle, int32_t distance, int32_t *high, uint32_t method)
{
    PlatFile *pf = plat_file_get(handle);
    long base, pos, was;

    if (pf == NULL)
        return PLAT_FILE_INVALID;
    if (high != NULL)
        *high = 0;
    if (method > 2) {
        g_platFileError = 87;
        return PLAT_FILE_INVALID;
    }
    if (method == 0) {
        base = 0;
    } else if (method == 1) {
        base = ftell(pf->f);
    } else {
        was = ftell(pf->f);
        if (fseek(pf->f, 0, SEEK_END) != 0) {
            g_platFileError = 5;
            return PLAT_FILE_INVALID;
        }
        base = ftell(pf->f);
        fseek(pf->f, was, SEEK_SET);
    }
    pos = base + distance;
    if (base < 0 || pos < 0) {
        g_platFileError = 131;  /* ERROR_NEGATIVE_SEEK: the position stays */
        return PLAT_FILE_INVALID;
    }
    if (fseek(pf->f, pos, SEEK_SET) != 0) {
        g_platFileError = 5;
        return PLAT_FILE_INVALID;
    }
    pf->last = PLAT_LAST_NONE;
    g_platFileError = 0;
    return (uint32_t)pos;
}

/* SetEndOfFile: standard C cannot cut a file, so the part before the position is read and the file written anew. */
int32_t Plat_FileTruncate(uint32_t handle)
{
    PlatFile *pf = plat_file_get(handle);
    long pos, size;
    char *keep;
    FILE *f;

    if (pf == NULL)
        return 0;
    if (!(pf->access & 0x40000000u))
        return (int32_t)plat_file_fail(5);
    pos = ftell(pf->f);
    if (pos < 0 || fseek(pf->f, 0, SEEK_END) != 0)
        return (int32_t)plat_file_fail(5);
    size = ftell(pf->f);
    if (pos >= size) {          /* nothing after the position: Windows would grow the file to it */
        fseek(pf->f, pos, SEEK_SET);
        if (pos > size && fseek(pf->f, pos - 1, SEEK_SET) == 0)
            fputc(0, pf->f), fseek(pf->f, pos, SEEK_SET);
        pf->last = PLAT_LAST_NONE;
        return 1;
    }
    keep = (char *)malloc(pos > 0 ? (size_t)pos : 1);
    if (keep == NULL)
        return (int32_t)plat_file_fail(8);
    fseek(pf->f, 0, SEEK_SET);
    if (pos > 0 && fread(keep, 1, (size_t)pos, pf->f) != (size_t)pos) {
        free(keep);
        return (int32_t)plat_file_fail(5);
    }
    f = freopen(pf->path, "w+b", pf->f);
    if (f == NULL) {
        free(keep);
        pf->f = NULL;
        return (int32_t)plat_file_fail(5);
    }
    pf->f = f;
    if (pos > 0)
        fwrite(keep, 1, (size_t)pos, f);
    free(keep);
    pf->last = PLAT_LAST_WRITE;
    return 1;
}

int32_t Plat_FileFlush(uint32_t handle)
{
    PlatFile *pf = plat_file_get(handle);

    if (pf == NULL)
        return 0;
    return fflush(pf->f) == 0 ? 1 : (int32_t)plat_file_fail(5);
}

int32_t Plat_FileClose(uint32_t handle)
{
    PlatFile *pf = plat_file_get(handle);
    int bad;

    if (pf == NULL)
        return 0;
    bad = fclose(pf->f) != 0;
    pf->f = NULL;
    if (pf->remove_on_close)
        remove(pf->path);
    return bad ? (int32_t)plat_file_fail(5) : 1;
}

uint32_t Plat_FileType(uint32_t handle)
{
    return plat_file_get(handle) != NULL ? 1u : 0u;
}

int32_t Plat_FileDelete(const char *name)
{
    char path[PLAT_PATH];
    SDL_PathInfo info;

    plat_file_path(name, path, sizeof path);
    if (!SDL_GetPathInfo(path, &info))
        return (int32_t)plat_file_fail(2);
    if (info.type != SDL_PATHTYPE_FILE)
        return (int32_t)plat_file_fail(5);
    errno = 0;
    if (remove(path) != 0)
        return (int32_t)plat_file_fail(plat_file_errno());
    return 1;
}

/* ---- folder listing ------------------------------------------------------------------------------------- */

/* Windows' match of a name against a pattern of '*' and '?', ignoring case. */
static int plat_file_match(const char *pat, const char *name)
{
    while (*pat != 0) {
        if (*pat == '*') {
            while (*pat == '*')
                pat++;
            if (*pat == 0)
                return 1;
            for (; *name != 0; name++)
                if (plat_file_match(pat, name))
                    return 1;
            return plat_file_match(pat, name);
        }
        if (*name == 0)
            return 0;
        if (*pat != '?') {
            int x = (unsigned char)*pat, y = (unsigned char)*name;

            if (x >= 'A' && x <= 'Z')
                x += 'a' - 'A';
            if (y >= 'A' && y <= 'Z')
                y += 'a' - 'A';
            if (x != y)
                return 0;
        }
        pat++;
        name++;
    }
    return *name == 0;
}

/* Windows' rule on top: a pattern ending in ".*" also matches the name without an extension ("*.*" is "*"). */
static int plat_file_match_dos(const char *pat, const char *name)
{
    size_t n = strlen(pat);

    if (plat_file_match(pat, name))
        return 1;
    if (n >= 2 && pat[n - 2] == '.' && pat[n - 1] == '*' && strchr(name, '.') == NULL) {
        char cut[PLAT_PATH];

        if (n - 2 >= sizeof cut)
            return 0;
        memcpy(cut, pat, n - 2);
        cut[n - 2] = 0;
        return plat_file_match(cut, name);
    }
    return 0;
}

static void plat_file_put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v, p[1] = (uint8_t)(v >> 8), p[2] = (uint8_t)(v >> 16), p[3] = (uint8_t)(v >> 24);
}

/* SDL_Time (nanoseconds since 1970) as a Windows FILETIME (100 ns since 1601) */
static void plat_file_time(uint8_t *p, SDL_Time t)
{
    uint64_t ft = (uint64_t)(t / 100) + 116444736000000000ull;

    plat_file_put32(p, (uint32_t)ft);
    plat_file_put32(p + 4, (uint32_t)(ft >> 32));
}

static int plat_file_fill(PlatFind *fd, void *data)
{
    while (fd->next < fd->count) {
        const char *name = fd->names[fd->next++];
        char path[PLAT_PATH * 2];
        SDL_PathInfo info;
        uint8_t *d = (uint8_t *)data;

        if (snprintf(path, sizeof path, "%s%s%s", fd->dir, fd->dir[0] != 0 ? "/" : "", name) >= (int)sizeof path)
            continue;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            SDL_GetPathInfo(fd->dir[0] != 0 ? fd->dir : ".", &info);
        } else if (!SDL_GetPathInfo(path, &info)) {
            continue;
        }
        memset(d, 0, 0x140);
        plat_file_put32(d, info.type == SDL_PATHTYPE_DIRECTORY ? 0x10u : 0x20u);
        plat_file_time(d + 4, info.create_time);
        plat_file_time(d + 0xc, info.access_time);
        plat_file_time(d + 0x14, info.modify_time);
        plat_file_put32(d + 0x1c, (uint32_t)(info.size >> 32));
        plat_file_put32(d + 0x20, (uint32_t)info.size);
        strncpy((char *)d + 0x2c, name, 259);
        return 1;
    }
    return 0;
}

uint32_t Plat_FileFindFirst(const char *pattern, void *data)
{
    char path[PLAT_PATH];
    const char *mask;
    PlatFileList list;
    PlatFind *fd;
    int i, n, k;

    for (k = 0; k < PLAT_FINDS && g_platFinds[k].names != NULL; k++)
        ;
    if (k == PLAT_FINDS) {
        g_platFileError = 4;
        return PLAT_FILE_INVALID;
    }
    fd = &g_platFinds[k];
    /* the folder part is mapped like a file name, the mask is matched against the folder's entries */
    {
        char game[PLAT_PATH];
        const char *cut = strrchr(pattern, '\\');
        const char *cut2 = strrchr(pattern, '/');
        size_t len;

        if (cut2 != NULL && (cut == NULL || cut2 > cut))
            cut = cut2;
        mask = cut != NULL ? cut + 1 : pattern;
        len = cut != NULL ? (size_t)(cut - pattern) : 0;
        if (len >= sizeof game)
            len = sizeof game - 1;
        memcpy(game, pattern, len);
        game[len] = 0;
        if (len == 0 && cut != NULL)
            strcpy(game, "/");
        plat_file_path(game, path, sizeof path);
    }
    list = plat_file_list(path);
    if (list.count < 0) {
        g_platFileError = 3;
        return PLAT_FILE_INVALID;
    }
    /* Windows lists "." and ".." in every folder but a drive's root */
    n = 0;
    fd->names = (char **)malloc(sizeof(char *) * (size_t)(list.count + 2));
    if (fd->names == NULL) {
        plat_file_unlist(&list);
        g_platFileError = 8;
        return PLAT_FILE_INVALID;
    }
    if (plat_file_match_dos(mask, ".")) {
        fd->names[n] = (char *)malloc(2);
        strcpy(fd->names[n++], ".");
    }
    if (plat_file_match_dos(mask, "..")) {
        fd->names[n] = (char *)malloc(3);
        strcpy(fd->names[n++], "..");
    }
    for (i = 0; i < list.count; i++) {
        if (plat_file_match_dos(mask, list.names[i])) {
            fd->names[n++] = list.names[i];
            list.names[i] = NULL;
        }
    }
    plat_file_unlist(&list);
    fd->count = n;
    fd->next = 0;
    strcpy(fd->dir, path);
    if (!plat_file_fill(fd, data)) {
        Plat_FileFindClose(PLAT_FIND_BASE + (uint32_t)k + 1);
        g_platFileError = 2;    /* ERROR_FILE_NOT_FOUND */
        return PLAT_FILE_INVALID;
    }
    g_platFileError = 0;
    return PLAT_FIND_BASE + (uint32_t)k + 1;
}

static PlatFind *plat_find_get(uint32_t find)
{
    uint32_t k = find - PLAT_FIND_BASE - 1;

    if (k >= PLAT_FINDS || g_platFinds[k].names == NULL) {
        g_platFileError = 6;
        return NULL;
    }
    return &g_platFinds[k];
}

int32_t Plat_FileFindNext(uint32_t find, void *data)
{
    PlatFind *fd = plat_find_get(find);

    if (fd == NULL)
        return 0;
    if (!plat_file_fill(fd, data))
        return (int32_t)plat_file_fail(18);    /* ERROR_NO_MORE_FILES */
    return 1;
}

int32_t Plat_FileFindClose(uint32_t find)
{
    PlatFind *fd = plat_find_get(find);
    int i;

    if (fd == NULL)
        return 0;
    for (i = 0; i < fd->count; i++)
        free(fd->names[i]);
    free(fd->names);
    fd->names = NULL;
    fd->count = fd->next = 0;
    return 1;
}
