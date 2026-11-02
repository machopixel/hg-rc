/* C runtime replacement, the rest: see hg_rest.h. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hg_rest.h"
#include "../../platform/plat.h"
#include "../unsorted_100f/w6i.h"

int32_t hg_time(void)
{
    return (int32_t)time(NULL);
}

int32_t *hg_localtime(const int32_t *t)
{
    static int32_t out[9];
    time_t tt;
    struct tm *tm;

    if (t == NULL || *t < 0)
        return NULL;            /* the original: no local time before 1970 */
    tt = (time_t)*t;
    tm = localtime(&tt);
    if (tm == NULL)
        return NULL;
    out[0] = tm->tm_sec;
    out[1] = tm->tm_min;
    out[2] = tm->tm_hour;
    out[3] = tm->tm_mday;
    out[4] = tm->tm_mon;
    out[5] = tm->tm_year;
    out[6] = tm->tm_wday;
    out[7] = tm->tm_yday;
    out[8] = tm->tm_isdst > 0;
    return out;
}

char *hg_getenv(const char *name)
{
    return getenv(name);
}

void hg_exit_process(uint32_t code)
{
    exit((int)code);
}

void hg_terminate_process(uint32_t code)
{
    _Exit((int)code);
}

/* ---- assert ---------------------------------------------------------------------------------------------- */

#define HG_ASSERT_LINE 60       /* longer program / file names: "..." and their last 60 characters */
#define HG_ASSERT_SIZE 0x21c    /* the original's text buffer */

static void hg_assert_cat_name(char *buf, const char *name)
{
    size_t n = strlen(name);

    if (n + 1 > HG_ASSERT_LINE) {
        strcat(buf, "...");
        strcat(buf, name + n - HG_ASSERT_LINE);
    } else {
        strcat(buf, name);
    }
}

void __cdecl Crt_Assert(const char *expr, const char *file, uint32_t line)
{
    char text[HG_ASSERT_SIZE + 64], program[260], num[16];
    size_t room;

    strcpy(text, "Assertion failed!\n\nProgram: ");
    if (Plat_ProgramPath(program, sizeof program) == 0)
        strcpy(program, "<program name unknown>");
    hg_assert_cat_name(text, program);
    strcat(text, "\nFile: ");
    hg_assert_cat_name(text, file);
    strcat(text, "\nLine: ");
    snprintf(num, sizeof num, "%u", line);
    strcat(text, num);
    strcat(text, "\n\nExpression: ");
    if (strlen(text) + strlen(expr) + 2 * 2 > HG_ASSERT_SIZE) {
        room = HG_ASSERT_SIZE - (strlen(text) + 3 + 2 * 2);
        strncat(text, expr, room);
        strcat(text, "...");
    } else {
        strcat(text, expr);
    }
    strcat(text, "\n\n");
    strcat(text, "For information on how your program can cause an assertion\n"
                 "failure, see the Visual C++ documentation on asserts");
    strcat(text, "\n\n");
    strcat(text, "(Press Retry to debug the application - JIT must be enabled)");
    switch (Plat_AssertBox(text, "Microsoft Visual C++ Runtime Library")) {
    case PLAT_ASSERT_ABORT:
        Proc_ExitQuick(3);      /* the original: raise(SIGABRT), with no handler _exit(3) */
        break;
    case PLAT_ASSERT_RETRY:
        Plat_DebugBreak();
        break;
    default:
        break;
    }
}
