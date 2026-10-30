/*
 * Round 24 diagnostic: who last wrote the two stack slots that the still-original AI line-of-fire check 0x1000aa9e
 * reads unset (DECISIONS.md, "Round 24"). Its call to Sim_10002180 at 0x1000ab09 passes [ebp-0x64] as dy and
 * [ebp-4] as dz, both never written by 0x1000aa9e.
 *
 * Only in a watcher run (hgrc.cfg "uninittrace") started with a mission on the command line: the launcher runs the
 * sim in a child copy of itself (HG_CHILD=1, _NO_DEBUG_HEAP=1 so the heap is the normal one) as its debugger. Debug
 * registers on the child's main thread, execute breakpoints only (below): the read at 0x1000ab09 and the two
 * candidate writers' stores. A read compares each slot with the last candidate store (same address, same value). Everything runs in the debugger process: nothing is written to the game's stack (an
 * exception handler in the game would be: the system puts the exception record there). Results in lastwriter.log.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#include "lastwriter.h"
#include "debug.h"

#define LW_READ_AT 0x1000ab09u
#define LW_SLOTS 2
#define LW_KINDS 64
#define LW_SAMPLES 300

typedef struct {
    DWORD next, ret, count, value;
} LwWriter;

typedef struct {
    DWORD addr;         /* the watched address (0: not armed) */
    int written;        /* a write was seen since it was armed */
    DWORD next, ret, value;
    LwWriter kinds[LW_KINDS];
    DWORD matched, unseen, unarmed, other;
    DWORD hAddr[LW_KINDS], hValue[LW_KINDS], hRet[LW_KINDS];    /* the last candidate store to each address */
    DWORD hUnit[LW_KINDS];      /* the unit id 0x100cada2 was checking (its [ebp+8]) for that store */
    DWORD unit, sameUnit, otherUnit;
} LwSlot;

static LwSlot g_slots[LW_SLOTS];
static DWORD g_reads, g_traps;
static FILE *g_log;
static const int kOffsets[LW_SLOTS] = {-4, -0x64};
static const char *const kNames[LW_SLOTS] = {"dz [ebp-4]", "dy [ebp-0x64]"};

/* hgrc.cfg "uninittrace" (the watcher entries) or "lastwriter" in hgrc.cfg or hgrc_extra.cfg (dev.cmd does not
   rewrite the latter: with "noconv lastwriter" there, entry 1 runs the original code through the menus) */
int LastWriter_Wanted(void)
{
    static const char *const kFiles[] = {"hgrc.cfg", "hgrc_extra.cfg"};
    char text[256], *tok;
    size_t n, f;
    FILE *fp;

    if (getenv("HG_CHILD") != NULL)
        return 0;
    for (f = 0; f < sizeof(kFiles) / sizeof(kFiles[0]); f++) {
        fp = fopen(kFiles[f], "r");
        if (fp == NULL)
            continue;
        n = fread(text, 1, sizeof(text) - 1, fp);
        fclose(fp);
        text[n] = '\0';
        for (tok = strtok(text, " \t\r\n"); tok != NULL; tok = strtok(NULL, " \t\r\n")) {
            if ((f == 0 && _stricmp(tok, "uninittrace") == 0) || _stricmp(tok, "lastwriter") == 0)
                return 1;
        }
    }
    return 0;
}

static DWORD ReadDword(HANDLE proc, DWORD addr)
{
    DWORD v = 0;
    SIZE_T got = 0;

    ReadProcessMemory(proc, (LPCVOID)(uintptr_t)addr, &v, sizeof(v), &got);
    return v;
}

/* DR0 / DR1 write watches (4 bytes) on the armed slots, DR2 the execute breakpoint at the read. */
/*
 * Execute breakpoints only (no data watches: those trapped on every stack write at the two addresses, hundreds of
 * thousands per frame in combat). DR0 / DR1 sit on the instruction after each candidate writer's store, found by the
 * first run's data watches (lastwriter.log of 2026-10-30, 6 of 6 reads): 0x100cb0ee after 0x100cada2's
 * fstp [ebp-0x3c] (dz) and 0x100cbc84 after 0x100cbc6b's fstp [ebp-0x10] (dy). DR2 is the read at 0x1000ab09.
 */
static const DWORD kSites[LW_SLOTS] = {0x100cb0eeu, 0x100cbc84u};
static const int kSiteOffsets[LW_SLOTS] = {-0x3c, -0x10};

/*
 * Menu mode (the default since round 24's second question; HG_LW=ai gives the line-of-fire mode above): the key code
 * [ebp-0x128] of the menu draw 0x1007581a, read at 0x10075bb0 (the call of Sim_10075753 after a separator was
 * skipped) and unset there on a menu's first frame. Its only stores are the menu draw's own: 0x100759a6 (ecx, the
 * key [0x1015dc18]) and 0x100759f3 / 0x10075a6a (0xd). DR0..DR2 on those three stores (before they run: the value
 * comes from ecx or the constant), DR3 on the read.
 */
static int g_menu = 1;
/*
 * Shots mode (the default since round 26; HG_LW=menu / ai give the other two): the target position [ebp-0x8c] /
 * [ebp-0x94] / [ebp-8] that the beam-shot update 0x10021e8c passes to Sim_100246b3 at 0x1002258a, never written by
 * itself. Candidate writers: the shot updates called just before it at the same depth by 0x10021d65's first loop,
 * 0x100225c7 (its 0x38-byte record [ebp-0xc0] covers -0x94 and -0x8c) and 0x100232d7. DR0 / DR1 on their single
 * epilogues (0x100232ad, 0x100240e9: the frame complete, ebp still theirs) record the three slots; DR2 is the read.
 */
static int g_shots;
static const DWORD kShotEnds[2] = {0x100232adu, 0x100240e9u};
static const int kShotSlots[3] = {-0x8c, -0x94, -8};
#define LW_SHOT_READ 0x1002258au
typedef struct {
    DWORD reads, match[2], other, none;
    DWORD addr[LW_KINDS], value[LW_KINDS], site[LW_KINDS];
    DWORD vals[LW_KINDS], valCount[LW_KINDS];
} LwShots;
static LwShots g_s;
static const DWORD kMenuStores[3] = {0x100759a6u, 0x100759f3u, 0x10075a6au};
#define LW_MENU_READ 0x10075bb0u

typedef struct {
    DWORD reads, firstFrame, match, other, none;
    DWORD addr[LW_KINDS], value[LW_KINDS], site[LW_KINDS];
    DWORD vals[LW_KINDS], valCount[LW_KINDS];
} LwMenu;
static LwMenu g_m;

static void SetWatches(CONTEXT *ctx)
{
    if (g_shots) {
        ctx->Dr0 = kShotEnds[0];
        ctx->Dr1 = kShotEnds[1];
        ctx->Dr2 = LW_SHOT_READ;
        ctx->Dr3 = 0;
        ctx->Dr7 = 1u | (1u << 2) | (1u << 4);
    } else if (g_menu) {
        ctx->Dr0 = kMenuStores[0];
        ctx->Dr1 = kMenuStores[1];
        ctx->Dr2 = kMenuStores[2];
        ctx->Dr3 = LW_MENU_READ;
        ctx->Dr7 = 1u | (1u << 2) | (1u << 4) | (1u << 6);
    } else {
        ctx->Dr0 = kSites[0];
        ctx->Dr1 = kSites[1];
        ctx->Dr2 = LW_READ_AT;
        ctx->Dr7 = 1u | (1u << 2) | (1u << 4);  /* L0 L1 L2, all execute (RW 00, LEN 00) */
    }
    ctx->Dr6 = 0;
}

/* A candidate's epilogue: the three slots at its frame's offsets, kept per address. */
static void ShotEnd(HANDLE proc, CONTEXT *ctx, int site)
{
    int i, k;

    g_traps++;
    for (i = 0; i < 3; i++) {
        DWORD addr = ctx->Ebp + (DWORD)kShotSlots[i];

        for (k = 0; k < LW_KINDS - 1 && g_s.addr[k] != addr && g_s.addr[k] != 0; k++)
            ;
        g_s.addr[k] = addr;
        g_s.value[k] = ReadDword(proc, addr);
        g_s.site[k] = (DWORD)site;
    }
}

/* The read: each slot against the last candidate epilogue's value at that address. */
static void ShotRead(HANDLE proc, CONTEXT *ctx)
{
    int i, k;

    g_s.reads++;
    for (i = 0; i < 3; i++) {
        DWORD addr = ctx->Ebp + (DWORD)kShotSlots[i];
        DWORD value = ReadDword(proc, addr);
        const char *verdict;

        for (k = 0; k < LW_KINDS && g_s.addr[k] != addr; k++)
            ;
        if (k == LW_KINDS) {
            g_s.none++;
            verdict = "no candidate epilogue at this address";
        } else if (g_s.value[k] == value) {
            g_s.match[g_s.site[k]]++;
            verdict = g_s.site[k] == 0 ? "match 0x100225c7" : "match 0x100232d7";
        } else {
            g_s.other++;
            verdict = "OTHER WRITER";
        }
        for (k = 0; k < LW_KINDS - 1 && g_s.valCount[k] != 0 && g_s.vals[k] != value; k++)
            ;
        g_s.vals[k] = value;
        g_s.valCount[k]++;
        if (g_log != NULL && g_s.reads <= LW_SAMPLES)
            fprintf(g_log, "read %lu [ebp%d] at %08lx = %08lx: %s\n", (unsigned long)g_s.reads, kShotSlots[i],
                    (unsigned long)addr, (unsigned long)value, verdict);
    }
}

static void MenuStore(CONTEXT *ctx, int i)
{
    DWORD addr = ctx->Ebp - 0x128;
    int k;

    g_traps++;
    for (k = 0; k < LW_KINDS - 1 && g_m.addr[k] != addr && g_m.addr[k] != 0; k++)
        ;
    g_m.addr[k] = addr;
    g_m.value[k] = i == 0 ? ctx->Ecx : 0xd;
    g_m.site[k] = kMenuStores[i];
}

static void MenuRead(HANDLE proc, CONTEXT *ctx)
{
    DWORD addr = ctx->Ebp - 0x128;
    DWORD value = ReadDword(proc, addr);
    DWORD rec = ReadDword(proc, ctx->Ebp - 0x14c);
    DWORD state = ReadDword(proc, rec);
    char title[33];
    SIZE_T got = 0;
    const char *verdict;
    int k;

    g_m.reads++;
    if (state != 1)
        return;                 /* written by this call on the way (a menu already shown) */
    g_m.firstFrame++;
    memset(title, 0, sizeof(title));
    ReadProcessMemory(proc, (LPCVOID)(uintptr_t)ReadDword(proc, rec + 4), title, sizeof(title) - 1, &got);
    for (k = 0; k < LW_KINDS && g_m.addr[k] != addr; k++)
        ;
    if (k == LW_KINDS) {
        g_m.none++;
        verdict = "no candidate store";
    } else if (g_m.value[k] == value) {
        g_m.match++;
        verdict = "match";
    } else {
        g_m.other++;
        verdict = "OTHER WRITER";
    }
    for (k = 0; k < LW_KINDS - 1 && g_m.valCount[k] != 0 && g_m.vals[k] != value; k++)
        ;
    g_m.vals[k] = value;
    g_m.valCount[k]++;
    if (g_log != NULL)
        fprintf(g_log, "first-frame read of '%s' at %08lx = %08lx: %s\n", title, (unsigned long)addr,
                (unsigned long)value, verdict);
}

static void Rearm(HANDLE thread)
{
    CONTEXT ctx;

    if (thread == NULL)
        return;
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    if (GetThreadContext(thread, &ctx)) {
        SetWatches(&ctx);
        SetThreadContext(thread, &ctx);
    }
}

static void CountWriter(LwSlot *s)
{
    int k;

    for (k = 0; k < LW_KINDS; k++) {
        LwWriter *w = &s->kinds[k];

        if (w->count != 0 && w->ret == s->ret) {
            w->count++;
            w->value = s->value;
            return;
        }
        if (w->count == 0) {
            w->next = s->next;
            w->ret = s->ret;
            w->value = s->value;
            w->count = 1;
            return;
        }
    }
    s->other++;
}

/* A read: does each slot hold what the candidate writer stored last, at that address? */
static void OnRead(HANDLE proc, CONTEXT *ctx)
{
    int i;
    /* 0x1000aa9e(target, unit, 1): the checking unit's id, its +4 */
    DWORD unit = ReadDword(proc, ReadDword(proc, ctx->Ebp + 0xc) + 4);

    g_reads++;
    for (i = 0; i < LW_SLOTS; i++) {
        LwSlot *s = &g_slots[i];
        DWORD addr = ctx->Ebp + (DWORD)kOffsets[i];
        DWORD value = ReadDword(proc, addr);
        int k, match = 0;

        for (k = 0; k < LW_KINDS && s->hAddr[k] != addr; k++)
            ;
        s->addr = addr;
        if (k == LW_KINDS) {
            s->written = 0;
            s->unarmed++;       /* no candidate store to this address yet */
        } else {
            s->written = 1;
            s->value = s->hValue[k];
            s->ret = s->hRet[k];
            s->unit = s->hUnit[k];
            match = s->value == value;
            if (match) {
                s->matched++;
                if (s->unit == unit)
                    s->sameUnit++;
                else
                    s->otherUnit++;
                CountWriter(s);
            } else {
                s->unseen++;    /* another writer came after the candidate's store */
            }
        }
        if (g_log != NULL && (g_reads <= LW_SAMPLES || !match))
            fprintf(g_log, "read %lu %s at %08lx = %08lx (unit %lu): %s; candidate store value %08lx (from %08lx, "
                    "unit %lu)\n", (unsigned long)g_reads, kNames[i], (unsigned long)addr, (unsigned long)value,
                    (unsigned long)unit, match ? "match" : s->written ? "NO MATCH" : "no candidate store",
                    (unsigned long)s->value, (unsigned long)s->ret, (unsigned long)s->unit);
    }
}

/* A candidate writer's store has just run: its address, value and the frame's return address. */
static void OnWrite(HANDLE proc, CONTEXT *ctx, int i)
{
    LwSlot *s = &g_slots[i];

    DWORD addr = ctx->Ebp + (DWORD)kSiteOffsets[i];
    int k;

    g_traps++;
    s->next = kSites[i];
    for (k = 0; k < LW_KINDS - 1 && s->hAddr[k] != addr && s->hAddr[k] != 0; k++)
        ;
    s->hAddr[k] = addr;
    s->hValue[k] = ReadDword(proc, addr);
    s->hRet[k] = ReadDword(proc, ctx->Ebp + 4);
    /* the unit 0x100cada2 checks: its own [ebp+8], or (in 0x100cbc6b) its caller's frame's */
    s->hUnit[k] = ReadDword(proc, (i == 0 ? ctx->Ebp : ReadDword(proc, ctx->Ebp)) + 8);
}

static void Report(void)
{
    int i, k;

    if (g_log == NULL)
        return;
    if (g_shots) {
        fprintf(g_log, "lastwriter shots: %lu reads at 0x1002258a, %lu candidate epilogues; slot reads: match "
                "0x100225c7 %lu, match 0x100232d7 %lu, other writer %lu, no candidate %lu\n", (unsigned long)g_s.reads,
                (unsigned long)g_traps, (unsigned long)g_s.match[0], (unsigned long)g_s.match[1],
                (unsigned long)g_s.other, (unsigned long)g_s.none);
        for (k = 0; k < LW_KINDS && g_s.valCount[k] != 0; k++)
            fprintf(g_log, "lastwriter shots:   value %08lx x%lu\n", (unsigned long)g_s.vals[k],
                    (unsigned long)g_s.valCount[k]);
        return;
    }
    if (g_menu) {
        fprintf(g_log, "lastwriter menu: %lu reads at 0x10075bb0, %lu on a menu's first frame (unset in the original); "
                "%lu key-code stores\n", (unsigned long)g_m.reads, (unsigned long)g_m.firstFrame,
                (unsigned long)g_traps);
        fprintf(g_log, "lastwriter menu: first-frame reads: match with the menu draw's last store there %lu, other "
                "writer after it %lu, no candidate store %lu\n", (unsigned long)g_m.match, (unsigned long)g_m.other,
                (unsigned long)g_m.none);
        for (k = 0; k < LW_KINDS && g_m.valCount[k] != 0; k++)
            fprintf(g_log, "lastwriter menu:   value %08lx x%lu\n", (unsigned long)g_m.vals[k],
                    (unsigned long)g_m.valCount[k]);
        return;
    }
    fprintf(g_log, "lastwriter: %lu reads of 0x1000aa9e at 0x1000ab09, %lu candidate stores\n",
            (unsigned long)g_reads, (unsigned long)g_traps);
    for (i = 0; i < LW_SLOTS; i++) {
        LwSlot *s = &g_slots[i];

        fprintf(g_log, "lastwriter: %s (candidate %08lx): match %lu, no candidate store there %lu, other writer after it %lu, "
                "callers beyond the table %lu\n", kNames[i], (unsigned long)kSites[i], (unsigned long)s->matched,
                (unsigned long)s->unarmed, (unsigned long)s->unseen, (unsigned long)s->other);
        fprintf(g_log, "lastwriter:   matches with the store made for the same unit %lu, for another unit %lu\n",
                (unsigned long)s->sameUnit, (unsigned long)s->otherUnit);
        for (k = 0; k < LW_KINDS && s->kinds[k].count != 0; k++)
            fprintf(g_log, "lastwriter:   x%lu stored by the call returning to %08lx, last value %08lx\n",
                    (unsigned long)s->kinds[k].count, (unsigned long)s->kinds[k].ret,
                    (unsigned long)s->kinds[k].value);
    }
}

/* The environment with HG_CHILD=1 and _NO_DEBUG_HEAP=1 added. */
static char *ChildEnvironment(void)
{
    char *env = GetEnvironmentStringsA(), *block, *dst;
    const char *src;
    size_t len = 0;
    static const char kAdd[] = "HG_CHILD=1\0_NO_DEBUG_HEAP=1\0";

    if (env == NULL)
        return NULL;
    for (src = env; *src != '\0'; src += strlen(src) + 1)
        len += strlen(src) + 1;
    block = (char *)malloc(len + sizeof(kAdd) + 1);
    if (block != NULL) {
        dst = block;
        for (src = env; *src != '\0'; src += strlen(src) + 1) {
            if (_strnicmp(src, "__COMPAT_LAYER=", 15) == 0)
                continue;       /* as RunSimIsolated: the child runs without the compatibility layer */
            strcpy(dst, src);
            dst += strlen(src) + 1;
        }
        memcpy(dst, kAdd, sizeof(kAdd));
        dst[sizeof(kAdd) - 1] = '\0';
    }
    FreeEnvironmentStringsA(env);
    return block;
}

int LastWriter_RunSim(const char *cmd, int *result)
{
    char exePath[MAX_PATH], *cmdLine, *env;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    DEBUG_EVENT ev;
    HANDLE mainThread = NULL;
    DWORD mainId = 0, code = 1;
    BOOL ok;

    if (GetModuleFileNameA(NULL, exePath, sizeof(exePath)) == 0)
        return 0;
    cmdLine = (char *)malloc(strlen(exePath) + strlen(cmd) + 8);
    env = ChildEnvironment();
    if (cmdLine == NULL || env == NULL) {
        free(cmdLine);
        free(env);
        return 0;
    }
    sprintf(cmdLine, "\"%s\" %s", exePath, cmd);
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    ok = CreateProcessA(exePath, cmdLine, NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS, env, NULL, &si, &pi);
    free(env);
    free(cmdLine);
    if (!ok) {
        DebugPrintf("lastwriter: starting the sim under the debugger failed, error %lu\n", GetLastError());
        return 0;
    }
    DebugPrintf("lastwriter: sim started under the debugger: %s\n", cmd);
    memset(g_slots, 0, sizeof(g_slots));
    memset(&g_m, 0, sizeof(g_m));
    memset(&g_s, 0, sizeof(g_s));
    g_shots = getenv("HG_LW") == NULL;
    g_menu = getenv("HG_LW") != NULL && _stricmp(getenv("HG_LW"), "menu") == 0;
    g_log = fopen("lastwriter.log", "w");
    for (;;) {
        DWORD cont = DBG_CONTINUE;

        if (!WaitForDebugEvent(&ev, INFINITE))
            break;
        switch (ev.dwDebugEventCode) {
        case CREATE_PROCESS_DEBUG_EVENT:
            mainThread = ev.u.CreateProcessInfo.hThread;
            mainId = ev.dwThreadId;
            if (ev.u.CreateProcessInfo.hFile != NULL)
                CloseHandle(ev.u.CreateProcessInfo.hFile);
            Rearm(mainThread);
            break;
        case LOAD_DLL_DEBUG_EVENT:
            if (ev.u.LoadDll.hFile != NULL)
                CloseHandle(ev.u.LoadDll.hFile);
            Rearm(mainThread);      /* the loader may have reset the thread's debug registers */
            break;
        case EXCEPTION_DEBUG_EVENT: {
            DWORD exc = ev.u.Exception.ExceptionRecord.ExceptionCode;

            if (exc == EXCEPTION_SINGLE_STEP && ev.dwThreadId == mainId) {
                CONTEXT ctx;

                ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS | CONTEXT_CONTROL | CONTEXT_INTEGER;
                if (GetThreadContext(mainThread, &ctx)) {
                    DWORD dr6 = ctx.Dr6;

                    if (g_shots) {
                        if (dr6 & 1u)
                            ShotEnd(pi.hProcess, &ctx, 0);
                        if (dr6 & 2u)
                            ShotEnd(pi.hProcess, &ctx, 1);
                        if (dr6 & 4u)
                            ShotRead(pi.hProcess, &ctx);
                    } else if (g_menu) {
                        if (dr6 & 1u)
                            MenuStore(&ctx, 0);
                        if (dr6 & 2u)
                            MenuStore(&ctx, 1);
                        if (dr6 & 4u)
                            MenuStore(&ctx, 2);
                        if (dr6 & 8u)
                            MenuRead(pi.hProcess, &ctx);
                    } else {
                        if (dr6 & 1u)
                            OnWrite(pi.hProcess, &ctx, 0);
                        if (dr6 & 2u)
                            OnWrite(pi.hProcess, &ctx, 1);
                        if (dr6 & 4u)
                            OnRead(pi.hProcess, &ctx);
                    }
                    ctx.EFlags |= 0x10000;      /* RF: run the instruction at the execute breakpoint */
                    ctx.Dr6 = 0;
                    SetThreadContext(mainThread, &ctx);
                }
            } else if (exc == EXCEPTION_BREAKPOINT) {
                Rearm(mainThread);  /* the loader's first breakpoint */
            } else {
                cont = DBG_EXCEPTION_NOT_HANDLED;
            }
            break;
        }
        case CREATE_THREAD_DEBUG_EVENT:
            break;
        default:
            break;
        }
        if (ev.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT && ev.dwProcessId == pi.dwProcessId) {
            code = ev.u.ExitProcess.dwExitCode;
            ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, cont);
            break;
        }
        ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, cont);
    }
    Report();
    if (g_log != NULL)
        fclose(g_log);
    g_log = NULL;
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    DebugPrintf("lastwriter: sim ended, code %lu (lastwriter.log)\n", (unsigned long)code);
    *result = (int)code;
    return 1;
}
