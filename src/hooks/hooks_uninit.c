/*
 * Uninitialised-stack watchers ("uninittrace" in hgrc.cfg or hgrc_extra.cfg; off by default). Diagnostic only: the
 * original code runs unchanged, the watchers only read memory.
 *
 * The functions in tools/round10/uninit_stack_list.md read locals they never write, so they see whatever the previous
 * function left on the stack. Each watched function gets an entry watcher on its prologue that notes the leftover
 * value in each such local. A normal tap stub would push registers below the stack pointer, which is exactly where
 * those locals are about to be, and so would change the leftovers; the entry stub therefore switches to a private
 * stack before it saves anything. Where a clean tap point exists, a second watcher sits on the instruction that does
 * the read, on the same private stack (round 18: a normal tap stub there still wrote below the frame, into the frame
 * the next call would use, and so changed leftovers that other watched reads then saw). A read counts as
 * "unset" when the local still holds the leftover noted at entry for the same call (matched by the frame pointer, so
 * recursion is fine).
 *
 * The private stack is shared, so this assumes the watched functions run on one thread (the sim's), as they do.
 *
 * At sim exit one debug.log line per function: calls, the leftover values at entry (most frequent first), and for
 * every read site how often it ran, how often the value was still the leftover, and those values.
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <windows.h>

#include "../launcher/debug.h"
#include "../sim/util/util.h"
#include "../sim/unsorted_100d/fns.h"
#include "hook.h"

#define MAX_OFF 8         /* round 22: 0x100a0def reads eight */
#define MAX_VALUES 6
#define RECENT 64           /* entry records kept per function, to match reads to their call */
#define TAP_STACK_DWORDS 4096

typedef struct ValueCount {
    DWORD value[MAX_VALUES];
    DWORD count[MAX_VALUES];
    DWORD other;
} ValueCount;

typedef struct UninitFn {
    DWORD addr;             /* at the preferred base 0x10000000 */
    int stolen;
    unsigned char prologue[9];
    int nOff;
    int off[MAX_OFF];       /* [ebp - off]; a negative entry is a 16-bit local at [ebp - -off] */
    const char *what;
    /* run time */
    DWORD calls;
    ValueCount entry[MAX_OFF];
    DWORD recentEbp[RECENT];
    DWORD recentValue[RECENT][MAX_OFF];
    DWORD recentNext;
    void *tramp;
} UninitFn;

typedef struct UninitSite {
    int fn;
    DWORD addr;
    int stolen;
    unsigned char expect[10];
    DWORD readAt;
    int nOff;
    int off[MAX_OFF];
    /* run time */
    DWORD runs;
    DWORD unset[MAX_OFF];
    ValueCount unsetValues[MAX_OFF];
    void *tramp;
} UninitSite;

#ifdef UNINIT_TABLE /* tools/verify/uninit_native.c tests the stubs with its own table */
#include UNINIT_TABLE
#else
#include "uninit_table.inc"
#endif

#define NFN ((int)(sizeof(g_fns) / sizeof(g_fns[0])))
#ifdef UNINIT_TABLE
#define NPARENT 0 /* the native test has no 0x1005ab44 */
#else
#define NPARENT 1 /* the parent-key watcher of 0x1005ab44 */
#endif
#define NSITE ((int)(sizeof(g_sites) / sizeof(g_sites[0])))

static int g_enabled;
static DWORD g_saveEsp;
static DWORD g_tapStack[TAP_STACK_DWORDS];

static DWORD ReadLocal(DWORD ebp, int off)
{
    if (off < 0)
        return *(const WORD *)(UINT_PTR)(ebp - (DWORD)-off);
    return *(const DWORD *)(UINT_PTR)(ebp - (DWORD)off);
}

static void Count(ValueCount *vc, DWORD value)
{
    int i;

    for (i = 0; i < MAX_VALUES; i++) {
        if (vc->count[i] != 0 && vc->value[i] == value) {
            vc->count[i]++;
            return;
        }
    }
    for (i = 0; i < MAX_VALUES; i++) {
        if (vc->count[i] == 0) {
            vc->value[i] = value;
            vc->count[i] = 1;
            return;
        }
    }
    vc->other++;
}

/* Runs on the private stack. `esp` is the stack pointer at the function's first instruction, so the frame pointer of
 * this call will be esp - 4. */
/*
 * Round 19: the node-tree walks 0x10089066 (walk A) and 0x1008950e (walk B), both started by 0x100897c7 (A, then
 * 0x10088e37, then B). Every call at depth d of a walk has the frame pointer root - 0x44 * d, so [ebp-4] is one
 * slot per depth. A leaf (a call that does not reach the after-loop read 0x1008926a / 0x10089712) returns its
 * never-written [ebp-4] compared with the marker 0x7f7fc99e. Each leaf read is classed by where that slot was last
 * written: (a) a non-leaf at that depth earlier in the same walk, (b) a non-leaf at that depth in the walk A run
 * just before this walk B (value still the same, or changed in between, by 0x10088e37), (c) neither (outside).
 */
#define WALK_DEPTHS 64
#define WALK_MARKER 0x7f7fc99eu

typedef struct WalkState {
    DWORD rootEbp;
    int active;
    DWORD final[WALK_DEPTHS];
    unsigned char written[WALK_DEPTHS];
    unsigned char nonLeaf[WALK_DEPTHS];
    DWORD leaves, cls[4], marker[4], mismatch;
} WalkState;

static WalkState g_walk[2];
static int g_walkBPaired, g_walkLastRootA;
static DWORD g_simBase;

static int WalkDepth(const WalkState *w, DWORD ebp)
{
    DWORD d;

    if (!w->active || ebp > w->rootEbp || (w->rootEbp - ebp) % 0x44 != 0)
        return -1;
    d = (w->rootEbp - ebp) / 0x44;
    return d < WALK_DEPTHS ? (int)d : -1;
}

/* the entry of walk `which` (0 = A, 1 = B): a root when called from 0x100897c7 */
static void WalkEntry(int which, DWORD esp)
{
    WalkState *w = &g_walk[which];
    DWORD ret = *(const DWORD *)(UINT_PTR)esp, ebp = esp - 4;
    int d;

    if (ret == g_simBase + (which == 0 ? 0x89818u : 0x89864u)) {
        if (which == 1)     /* walk A just before, from the same 0x100897c7 frame */
            g_walkBPaired = g_walkLastRootA && g_walk[0].rootEbp == ebp;
        else
            g_walk[1].active = 0;
        g_walkLastRootA = which == 0;
        w->active = 1;
        w->rootEbp = ebp;
        memset(w->written, 0, sizeof(w->written));
    }
    d = WalkDepth(w, ebp);
    if (d >= 0)
        w->nonLeaf[d] = 0;
}

/* a read in walk `which`: after the child loop (after = 1) or at the final compare (after = 0) */
static void WalkRead(int which, DWORD ebp, int after)
{
    WalkState *w = &g_walk[which];
    DWORD v = *(const DWORD *)(UINT_PTR)(ebp - 4);
    int d = WalkDepth(w, ebp), c;

    if (d < 0)
        return;
    if (after) {
        w->nonLeaf[d] = 1;
        return;
    }
    if (w->nonLeaf[d]) {
        w->written[d] = 1;
        w->final[d] = v;
        return;
    }
    w->leaves++;
    if (w->written[d]) {
        c = 0;
        if (v != w->final[d])
            w->mismatch++;
    } else if (which == 1 && g_walkBPaired && g_walk[0].written[d]) {
        c = v == g_walk[0].final[d] ? 1 : 2;
    } else {
        c = 3;
    }
    w->cls[c]++;
    if (v == WALK_MARKER)
        w->marker[c]++;
}

static void __cdecl UninitEntry(DWORD index, DWORD esp)
{
    UninitFn *f = &g_fns[index];
    DWORD ebp = esp - 4;
    DWORD slot = f->recentNext++ % RECENT;
    int k;

    f->calls++;
    if (f->addr == 0x10089066 || f->addr == 0x1008950E)
        WalkEntry(f->addr == 0x1008950E, esp);
    f->recentEbp[slot] = ebp;
    for (k = 0; k < f->nOff; k++) {
        DWORD v = ReadLocal(ebp, f->off[k]);

        f->recentValue[slot][k] = v;
        Count(&f->entry[k], v);
    }
}

/* Called by the read stub of site `index` (MakeSiteStub) with the game's frame pointer at the read. */
static void __cdecl UninitRead(int index, DWORD ebp)
{
    UninitSite *s = &g_sites[index];
    UninitFn *f = &g_fns[s->fn];
    int r, k, j;

    if (s->addr == 0x1008926A || s->addr == 0x10089712 || s->addr == 0x1008930F || s->addr == 0x100897B1)
        WalkRead(s->addr == 0x10089712 || s->addr == 0x100897B1, ebp, s->addr == 0x1008926A || s->addr == 0x10089712);

    s->runs++;
    for (r = 0; r < RECENT; r++) {
        DWORD slot = (f->recentNext - 1 - (DWORD)r) % RECENT;

        if ((DWORD)r >= f->recentNext || f->recentEbp[slot] != ebp)
            continue;
        for (k = 0; k < s->nOff; k++) {
            DWORD v = ReadLocal(ebp, s->off[k]);

            for (j = 0; j < f->nOff; j++) {
                if (f->off[j] == s->off[k] && f->recentValue[slot][j] == v) {
                    s->unset[k]++;
                    Count(&s->unsetValues[k], v);
                    if (s->unset[k] <= 4) /* the first few, as they happen */
                        DebugPrintf("uninittrace: read 0x%08lx [ebp-0x%lx] unset: %08lx\n", s->readAt,
                                    (DWORD)(s->off[k] < 0 ? -s->off[k] : s->off[k]), v);
                }
            }
        }
        return;
    }
}


/*
 * Round 18: the parent key of each sub-unit the POF node walk 0x1005ab44 creates. A watcher at 0x1005acbf (the call
 * of 0x1005aa45 is being set up: [esp] = the node index, [esp + 4] = the parent key, the caller's possibly
 * never-written [ebp-0x10]), on the private stack (MakeParentStub), looks the key up the way 0x1005aa45 does
 * (Sim_100b3839(Sim_100db770(key))) and counts whether it names an existing entry, so the E and D runs show whether
 * the leftover changes any parent.
 */
static DWORD g_parentCalls, g_parentFound, g_parentSmall;
static ValueCount g_parentValues, g_parentFoundValues;

static void *g_parentTramp;

/* `esp` is the game's stack pointer at 0x1005acbf: [esp] = the node index, [esp + 4] = the parent key. */
static void __cdecl ParentAt(DWORD esp)
{
    DWORD key = ((const DWORD *)(UINT_PTR)esp)[1];

    g_parentCalls++;
    Count(&g_parentValues, key);
    if (key < 0x10000)
        g_parentSmall++;
    if (Sim_100b3839((uint32_t)Sim_100db770((int)key)) != -1) {
        g_parentFound++;
        Count(&g_parentFoundValues, key);
    }
}

static const unsigned char kParentTapBytes[] = {0xC1, 0xE1, 0x05, 0x8B, 0x55, 0x18};

static void Put32(unsigned char **p, DWORD v)
{
    memcpy(*p, &v, 4);
    *p += 4;
}

/*
 * The entry stub of function `index`:
 *     mov [g_saveEsp], esp ; mov esp, private top ; pushad ; pushfd ; push [g_saveEsp] ; push index
 *     mov eax, UninitEntry ; call eax ; add esp, 8 ; popfd ; popad ; mov esp, [g_saveEsp] ; jmp [tramp]
 * Nothing is written to the game's stack; `mov` leaves the flags alone.
 */
static void *MakeEntryStub(int index)
{
    unsigned char *stub = VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    unsigned char *p = stub;

    if (stub == NULL)
        return NULL;
    *p++ = 0x89; *p++ = 0x25; Put32(&p, (DWORD)(UINT_PTR)&g_saveEsp);
    *p++ = 0xBC; Put32(&p, (DWORD)(UINT_PTR)&g_tapStack[TAP_STACK_DWORDS]);
    *p++ = 0x60;
    *p++ = 0x9C;
    *p++ = 0xFF; *p++ = 0x35; Put32(&p, (DWORD)(UINT_PTR)&g_saveEsp);
    *p++ = 0x68; Put32(&p, (DWORD)index);
    *p++ = 0xB8; Put32(&p, (DWORD)(UINT_PTR)UninitEntry);
    *p++ = 0xFF; *p++ = 0xD0;
    *p++ = 0x83; *p++ = 0xC4; *p++ = 0x08;
    *p++ = 0x9D;
    *p++ = 0x61;
    *p++ = 0x8B; *p++ = 0x25; Put32(&p, (DWORD)(UINT_PTR)&g_saveEsp);
    *p++ = 0xFF; *p++ = 0x25; Put32(&p, (DWORD)(UINT_PTR)&g_fns[index].tramp);
    FlushInstructionCache(GetCurrentProcess(), stub, (SIZE_T)(p - stub));
    return stub;
}

/* The parent-key watcher's stub, on the private stack like the entry stubs (a normal tap stub would push onto the
 * frame the next recursive call is about to use, the very local being watched). */
static void *MakeParentStub(void)
{
    unsigned char *stub = VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    unsigned char *p = stub;

    if (stub == NULL)
        return NULL;
    *p++ = 0x89; *p++ = 0x25; Put32(&p, (DWORD)(UINT_PTR)&g_saveEsp);
    *p++ = 0xBC; Put32(&p, (DWORD)(UINT_PTR)&g_tapStack[TAP_STACK_DWORDS]);
    *p++ = 0x60;
    *p++ = 0x9C;
    *p++ = 0xFF; *p++ = 0x35; Put32(&p, (DWORD)(UINT_PTR)&g_saveEsp);
    *p++ = 0xB8; Put32(&p, (DWORD)(UINT_PTR)ParentAt);
    *p++ = 0xFF; *p++ = 0xD0;
    *p++ = 0x83; *p++ = 0xC4; *p++ = 0x04;
    *p++ = 0x9D;
    *p++ = 0x61;
    *p++ = 0x8B; *p++ = 0x25; Put32(&p, (DWORD)(UINT_PTR)&g_saveEsp);
    *p++ = 0xFF; *p++ = 0x25; Put32(&p, (DWORD)(UINT_PTR)&g_parentTramp);
    FlushInstructionCache(GetCurrentProcess(), stub, (SIZE_T)(p - stub));
    return stub;
}

/*
 * The read stub of site `index`, on the private stack like the entry stubs (round 18): a normal tap stub pushes the
 * registers and runs its C code on the game's stack below the tap, which is where the next call's frame will be, so it
 * changed the very leftovers being watched (DECISIONS.md, round 18).
 *     mov [g_saveEsp], esp ; mov esp, private top ; pushad ; pushfd ; push ebp ; push index
 *     mov eax, UninitRead ; call eax ; add esp, 8 ; popfd ; popad ; mov esp, [g_saveEsp] ; jmp [tramp]
 */
static void *MakeSiteStub(int index)
{
    unsigned char *stub = VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    unsigned char *p = stub;

    if (stub == NULL)
        return NULL;
    *p++ = 0x89; *p++ = 0x25; Put32(&p, (DWORD)(UINT_PTR)&g_saveEsp);
    *p++ = 0xBC; Put32(&p, (DWORD)(UINT_PTR)&g_tapStack[TAP_STACK_DWORDS]);
    *p++ = 0x60;
    *p++ = 0x9C;
    *p++ = 0x55;
    *p++ = 0x68; Put32(&p, (DWORD)index);
    *p++ = 0xB8; Put32(&p, (DWORD)(UINT_PTR)UninitRead);
    *p++ = 0xFF; *p++ = 0xD0;
    *p++ = 0x83; *p++ = 0xC4; *p++ = 0x08;
    *p++ = 0x9D;
    *p++ = 0x61;
    *p++ = 0x8B; *p++ = 0x25; Put32(&p, (DWORD)(UINT_PTR)&g_saveEsp);
    *p++ = 0xFF; *p++ = 0x25; Put32(&p, (DWORD)(UINT_PTR)&g_sites[index].tramp);
    FlushInstructionCache(GetCurrentProcess(), stub, (SIZE_T)(p - stub));
    return stub;
}

static int ConfigHasWord(const char *wanted)
{
    static const char *const kFiles[] = {"hgrc.cfg", "hgrc_extra.cfg"};
    char word[64];
    int found = 0;
    size_t i;

    for (i = 0; i < sizeof(kFiles) / sizeof(kFiles[0]) && !found; i++) {
        FILE *fp = fopen(kFiles[i], "r");

        while (fp != NULL && !found && fscanf(fp, "%63s", word) == 1)
            found = _stricmp(word, wanted) == 0;
        if (fp != NULL)
            fclose(fp);
    }
    return found;
}

void HooksUninit_OnSimLoaded(HMODULE sim)
{
    HookDesc d;
    int i, fnCount = 0, siteCount = 0;

    g_enabled = ConfigHasWord("uninittrace");
    if (!g_enabled)
        return;
    g_simBase = (DWORD)(UINT_PTR)sim;
    for (i = 0; i < NFN; i++) {
        void *stub = MakeEntryStub(i);

        if (stub == NULL)
            continue;
        memset(&d, 0, sizeof(d));
        d.module = "HGSIM.DLL";
        d.rva = g_fns[i].addr - 0x10000000u;
        d.name = "uninittrace entry";
        d.expect = g_fns[i].prologue;
        d.stolen = (size_t)g_fns[i].stolen;
        d.replacement = stub;
        d.original = &g_fns[i].tramp;
        fnCount += Hook_InstallModule(sim, "HGSIM.DLL", &d, 1);
    }
    for (i = 0; i < NSITE; i++) {
        memset(&d, 0, sizeof(d));
        d.module = "HGSIM.DLL";
        d.rva = g_sites[i].addr - 0x10000000u;
        d.name = "uninittrace read";
        d.expect = g_sites[i].expect;
        d.stolen = (size_t)g_sites[i].stolen;
        d.replacement = MakeSiteStub(i);
        d.original = &g_sites[i].tramp;
        if (d.replacement != NULL)
            siteCount += Hook_InstallModule(sim, "HGSIM.DLL", &d, 1);
    }
#ifndef UNINIT_TABLE /* not in the native test, whose fake module is one page */
    memset(&d, 0, sizeof(d));
    d.module = "HGSIM.DLL";
    d.rva = 0x5acbf;
    d.name = "uninittrace parent key";
    d.expect = kParentTapBytes;
    d.stolen = sizeof(kParentTapBytes);
    d.replacement = MakeParentStub();
    d.original = &g_parentTramp;
    if (d.replacement != NULL)
        siteCount += Hook_InstallModule(sim, "HGSIM.DLL", &d, 1);
#endif
    DebugPrintf("HGSIM.DLL: uninittrace %d of %d entry watcher(s) and %d of %d read watcher(s) installed\n", fnCount,
                NFN, siteCount, NSITE + NPARENT);
}

static int Append(char *buf, int at, int size, const char *fmt, ...)
{
    va_list ap;
    int n;

    if (at >= size - 1)
        return at;
    va_start(ap, fmt);
    n = _vsnprintf(buf + at, (size_t)(size - at), fmt, ap);
    va_end(ap);
    if (n < 0 || n >= size - at) {
        buf[size - 1] = 0;
        return size - 1;
    }
    return at + n;
}

static int AppendValues(char *buf, int at, int size, const ValueCount *vc)
{
    int i, j, order[MAX_VALUES], n = 0;

    for (i = 0; i < MAX_VALUES; i++)
        if (vc->count[i] != 0)
            order[n++] = i;
    for (i = 1; i < n; i++) /* most frequent first */
        for (j = i; j > 0 && vc->count[order[j]] > vc->count[order[j - 1]]; j--) {
            int t = order[j];

            order[j] = order[j - 1];
            order[j - 1] = t;
        }
    for (i = 0; i < n; i++)
        at = Append(buf, at, size, " %08lx x%lu", vc->value[order[i]], vc->count[order[i]]);
    if (vc->other != 0)
        at = Append(buf, at, size, " +%lu other", vc->other);
    return at;
}

/* Called when the sim has returned: the summary, a few short lines per watched function (DebugPrintf keeps at most
 * 255 characters of a line). */
void HooksUninit_OnSimExit(void)
{
    char line[240];
    int i, k, s, at;

    if (!g_enabled)
        return;
    for (i = 0; i < 2; i++) {
        const WalkState *w = &g_walk[i];

        DebugPrintf("uninittrace: walk %s leaves %lu: same walk %lu (marker %lu, differs %lu), previous walk A kept %lu "
                    "(marker %lu), changed %lu (marker %lu), outside %lu (marker %lu)\n", i == 0 ? "0x10089066" : "0x1008950e",
                    w->leaves, w->cls[0], w->marker[0], w->mismatch, w->cls[1], w->marker[1], w->cls[2], w->marker[2],
                    w->cls[3], w->marker[3]);
    }
    at = Append(line, 0, (int)sizeof(line), "uninittrace: 0x1005ab44 parent keys %lu, found %lu, below 0x10000 %lu:",
                g_parentCalls, g_parentFound, g_parentSmall);
    AppendValues(line, at, (int)sizeof(line), &g_parentValues);
    DebugPrintf("%s\n", line);
    at = Append(line, 0, (int)sizeof(line), "uninittrace:   found keys:");
    AppendValues(line, at, (int)sizeof(line), &g_parentFoundValues);
    DebugPrintf("%s\n", line);
    for (i = 0; i < NFN; i++) {
        const UninitFn *f = &g_fns[i];

        DebugPrintf("uninittrace: 0x%08lx (%s) calls %lu\n", f->addr, f->what, f->calls);
        for (k = 0; k < f->nOff; k++) {
            at = Append(line, 0, (int)sizeof(line), f->off[k] < 0 ? "uninittrace:   [ebp-0x%lx] word at entry:" :
                                                                    "uninittrace:   [ebp-0x%lx] at entry:",
                        (DWORD)(f->off[k] < 0 ? -f->off[k] : f->off[k]));
            AppendValues(line, at, (int)sizeof(line), &f->entry[k]);
            DebugPrintf("%s\n", line);
        }
        for (s = 0; s < NSITE; s++) {
            const UninitSite *site = &g_sites[s];

            if (site->fn != i)
                continue;
            DebugPrintf("uninittrace:   read 0x%08lx ran %lu\n", site->readAt, site->runs);
            for (k = 0; k < site->nOff; k++) {
                at = Append(line, 0, (int)sizeof(line), "uninittrace:     [ebp-0x%lx] unset %lu",
                            (DWORD)(site->off[k] < 0 ? -site->off[k] : site->off[k]), site->unset[k]);
                if (site->unset[k] != 0) {
                    at = Append(line, at, (int)sizeof(line), ":");
                    AppendValues(line, at, (int)sizeof(line), &site->unsetValues[k]);
                }
                DebugPrintf("%s\n", line);
            }
        }
    }
}