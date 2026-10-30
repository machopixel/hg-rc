#include <string.h>

#include "mem.h"

// FUNCTION: HGSIM 0x100ba21c
void Str_CutAtBlank(char *s)
{
    while (*s != '\0' && *s != ' ' && *s != '\t')
        s++;
    *s = '\0';
}

// FUNCTION: HGSIM 0x100ba252
char *Str_SkipBlanks(char *s)
{
    while (*s != '\0' && (*s == ' ' || *s == '\t'))
        s++;
    return s;
}

/*
 * Starts at the terminating NUL, which counts as blank, so the first character
 * kept is the last non-blank one, and the string ends right after it.
 */
// FUNCTION: HGSIM 0x100ba285
void Str_TrimEnd(char *s)
{
    int n = (int)strlen(s);

    while (n > 0 && (s[n] == ' ' || s[n] == '\0' || s[n] == '\r' || s[n] == '\n' || s[n] == '\t'))
        n--;
    s[n + 1] = '\0';
}
