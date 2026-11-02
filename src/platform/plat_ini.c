/*
 * Settings files (plat.h, "settings files"): our own reader with the rules of Windows' GetPrivateProfileStringA /
 * GetPrivateProfileIntA, the same on every system (DECISIONS.md, C runtime replacement, files). The file is opened
 * through the platform's file functions (name mapping included).
 *
 * Rules: lines end in LF or CR LF; "[section]" and "key=value" with spaces and tabs around names and values
 * dropped; names compared ignoring case; the first matching section and key win; lines starting with ';' are
 * comments; a value in a pair of double or single quotes loses them. A missing key gives the default (its trailing
 * spaces dropped). The section and key asked for lose their surrounding spaces too. Numbers: an optional sign, then
 * "0x" hexadecimal, "0o" octal, "0b" binary (lower-case prefixes only) or decimal, digits up to the first other
 * character, wrapping at 32 bits; no digits read as 0, a missing or empty value gives the default. (Checked against
 * Windows itself on random files.)
 */
#include <stdlib.h>
#include <string.h>
#include "plat.h"

#define PLAT_INI_MAX 0x40000    /* bigger settings files are not read past this */

static int plat_ini_icmp_n(const char *a, size_t na, const char *b)
{
    size_t i;

    for (i = 0; i < na; i++) {
        int x = (unsigned char)a[i], y = (unsigned char)b[i];

        if (y == 0)
            return 1;
        if (x >= 'A' && x <= 'Z')
            x += 'a' - 'A';
        if (y >= 'A' && y <= 'Z')
            y += 'a' - 'A';
        if (x != y)
            return 1;
    }
    return b[na] != 0;
}

static int plat_ini_space(char c)
{
    return c == ' ' || c == '\t';
}

/* name without its surrounding spaces and tabs into out (size bytes) */
static const char *plat_ini_trim(const char *name, char *out, size_t size)
{
    size_t n;

    while (plat_ini_space(*name))
        name++;
    n = strlen(name);
    while (n > 0 && plat_ini_space(name[n - 1]))
        n--;
    if (n >= size)
        n = size - 1;
    memcpy(out, name, n);
    out[n] = 0;
    return out;
}

/* The value of key in section, as a pointer into text and a length; 0 when there is none. */
static int plat_ini_find(const char *text, size_t size, const char *section_in, const char *key_in,
                         const char **value, size_t *len)
{
    const char *p = text, *end = text + size;
    char section[256], key[256];
    int in = 0;

    plat_ini_trim(section_in, section, sizeof section);
    plat_ini_trim(key_in, key, sizeof key);

    while (p < end) {
        const char *line = p, *le, *a, *b;

        while (p < end && *p != '\n')
            p++;
        le = p;
        if (p < end)
            p++;
        if (le > line && le[-1] == '\r')
            le--;
        for (a = line; a < le && plat_ini_space(*a); a++)
            ;
        if (a == le || *a == ';')
            continue;
        if (*a == '[') {
            const char *close = memchr(a, ']', (size_t)(le - a));

            if (close == NULL)
                close = le;
            a++;
            while (a < close && plat_ini_space(*a))
                a++;
            for (b = close; b > a && plat_ini_space(b[-1]); b--)
                ;
            if (in)
                return 0;       /* the section ended without the key */
            in = plat_ini_icmp_n(a, (size_t)(b - a), section) == 0;
            continue;
        }
        if (!in)
            continue;
        {
            const char *eq = memchr(a, '=', (size_t)(le - a));
            const char *v;

            if (eq == NULL)
                continue;
            for (b = eq; b > a && plat_ini_space(b[-1]); b--)
                ;
            if (plat_ini_icmp_n(a, (size_t)(b - a), key) != 0)
                continue;
            for (v = eq + 1; v < le && plat_ini_space(*v); v++)
                ;
            for (b = le; b > v && plat_ini_space(b[-1]); b--)
                ;
            if (b - v >= 2 && (*v == '"' || *v == '\'') && b[-1] == *v) {
                v++;
                b--;
            }
            *value = v;
            *len = (size_t)(b - v);
            return 1;
        }
    }
    return 0;
}

/* The whole settings file (at most PLAT_INI_MAX bytes) in a block to free; NULL when it cannot be read. */
static char *plat_ini_load(const char *path, size_t *size)
{
    uint32_t h = Plat_FileCreate(path, 0x80000000u, 1, 0, 3, 0x80, 0);
    uint32_t got = 0;
    char *text;

    if (h == PLAT_FILE_INVALID)
        return NULL;
    text = (char *)malloc(PLAT_INI_MAX);
    if (text != NULL && !Plat_FileRead(h, text, PLAT_INI_MAX, &got, 0))
        got = 0;
    Plat_FileClose(h);
    *size = got;
    return text;
}

uint32_t Plat_IniString(const char *path, const char *section, const char *key, const char *def, char *out,
                        uint32_t size)
{
    size_t total = 0, len, n;
    char *text = plat_ini_load(path, &total);
    const char *value = NULL;

    if (size == 0) {
        free(text);
        return 0;
    }
    if (text == NULL || !plat_ini_find(text, total, section, key, &value, &len)) {
        value = def != NULL ? def : "";
        len = strlen(value);
        while (len > 0 && value[len - 1] == ' ')
            len--;
    }
    n = len < size - 1 ? len : size - 1;
    memcpy(out, value, n);
    out[n] = 0;
    free(text);
    return (uint32_t)n;
}

uint32_t Plat_IniInt(const char *path, const char *section, const char *key, uint32_t def)
{
    size_t total = 0, len;
    char *text = plat_ini_load(path, &total);
    const char *v = NULL;
    uint32_t r = def;

    if (text != NULL && plat_ini_find(text, total, section, key, &v, &len) && len != 0) {
        size_t i = 0;
        int neg = 0;
        uint32_t x = 0, base = 10;

        if (v[i] == '-' || v[i] == '+')
            neg = v[i++] == '-';
        if (i + 1 < len && v[i] == '0' && (v[i + 1] == 'x' || v[i + 1] == 'o' || v[i + 1] == 'b')) {
            base = v[i + 1] == 'x' ? 16u : v[i + 1] == 'o' ? 8u : 2u;
            i += 2;
        }
        for (; i < len; i++) {
            char c = v[i];
            uint32_t d = c >= '0' && c <= '9' ? (uint32_t)(c - '0') : c >= 'a' && c <= 'f' ? (uint32_t)(c - 'a' + 10)
                       : c >= 'A' && c <= 'F' ? (uint32_t)(c - 'A' + 10) : 99u;

            if (d >= base)
                break;
            x = x * base + d;
        }
        r = neg ? 0u - x : x;
    }
    free(text);
    return r;
}
