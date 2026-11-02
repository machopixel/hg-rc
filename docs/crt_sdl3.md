# C runtime routines our code calls, and SDL3

Analysis for Diego (2026-11-01, "analyse the exact list of functions that are called, and find if sdl3 offers that functionality"). Nothing was changed by it. SDL3 is the version we link, 3.2.30 (headers and sources under build/_deps/sdl3-src).

## How the list was made

- Every C file of src/sim that is not C-runtime code itself was preprocessed (gcc -E) and every use inside a function body counted: calls of our converted C-runtime functions by name, and calls through SIM_FN to a C-runtime address (0x100fa000 and up; through the hook these reach our converted copy, or the original when there is none). Prototypes are not counted. Call sites in macros count once per use.
- Result: **73 routines, 1370 call sites.** The other ~150 converted C-runtime routines are only called by these (the printf / scanf engines, the heap blocks, the stream buffers, locale, exit and error handling): they go when their callers are replaced.
- Maths (group 1) is not in this table: it is OpenLibm (sqrt, fmod, fabs, pow, asin, and sin / cos / acos of round 10), already switched.
- "Own" = SDL's own code, the same on every platform. "Libc" = in our Windows build SDL passes the call to the system C library (HAVE_* in SDL_build_config.h), on a platform without it SDL uses its own code.

## Totals

| verdict | routines | call sites |
|---|---|---|
| yes | 38 | 612 |
| partly | 25 | 731 |
| no | 7 | 23 |
| not used | 2 | 3 |
| n/a | 1 | 1 |

## The routines

| calls | ours | standard | group | SDL3 | verdict | SDL impl. | note |
|---|---|---|---|---|---|---|---|
| 187 | Str_Copy (0x100fc690) | strcpy | text | SDL_strlcpy | partly | own | no plain strcpy: SDL_strlcpy needs the buffer size |
| 152 | Crt_SPrintf (0x100fc780) | sprintf | format | SDL_snprintf | partly | own | needs the buffer size; format gaps (below) |
| 85 | Mem_Move (0x100fcdc0) | memmove | memory | SDL_memmove | yes | libc |  |
| 62 | Mem_Fill (0x100fcab0) | memset | memory | SDL_memset | yes | libc |  |
| 61 | Str_CopyN (0x100fc7f0) | strncpy | text | SDL_strlcpy | partly | own | different meaning: strncpy pads with zeros and does not terminate a full copy; SDL_strlcpy always terminates, never pads |
| 60 | Crt_FPrintf (0x100fd7e0) | fprintf | files | SDL_IOprintf | partly | own | on an SDL_IOStream; SDL's formatting (format gaps below) |
| 53 | Str_CompareNoCase (0x100ff1e0) | _stricmp | text | SDL_strcasecmp | partly | own | SDL folds case as UTF-8 (Unicode): bytes 0x80-0xff compare differently; ASCII the same |
| 48 | Str_Length (0x100fc8f0) | strlen | text | SDL_strlen | yes | libc |  |
| 48 | Sim_100fd850 (0x100fd850) | fopen | files | SDL_IOFromFile | partly | own | always binary: no CRLF translation for the text modes the game uses (r, w, a, rt, wt, at, a+, w+); no FILE* (an SDL_IOStream*) |
| 42 | 0x100ff860 | _assert (original, not converted) | rest | SDL_assert / SDL_assert_release | partly | own | a macro with SDL's own report (dialog / log, break, retry); the original prints and calls abort() |
| 41 | Heap_FreeBlock (0x100fefe0) | free | memory | SDL_free | yes | libc |  |
| 40 | Crt_FClose (0x100fd760) | fclose | files | SDL_CloseIO | yes | own | on an SDL_IOStream |
| 39 | Crt_StrStr (0x100fc970) | strstr | text | SDL_strstr | yes | libc |  |
| 33 | Str_Cat (0x100fc6a0) | strcat | text | SDL_strlcat | partly | own | no plain strcat: SDL_strlcat needs the buffer size |
| 33 | Heap_Malloc (0x100fef30) | malloc | memory | SDL_malloc | yes | libc | block contents differ (uninitialised memory) |
| 32 | Str_Compare (0x100fcd30) | strcmp | text | SDL_strcmp | yes | libc |  |
| 32 | Crt_FWrite (0x100febc0) | fwrite | files | SDL_WriteIO | yes | own | bytes, not (size, count): count = bytes / size |
| 24 | Str_CompareN (0x100fe7f0) | strncmp | text | SDL_strncmp | yes | libc |  |
| 23 | Crt_StrChr (0x100fd650) | strchr | text | SDL_strchr | yes | libc |  |
| 21 | Crt_FRead (0x100fe830) | fread | files | SDL_ReadIO | yes | own | bytes, not (size, count) |
| 18 | Crt_StrUpper (0x1010d180) | _strupr | text | SDL_strupr | yes | own | ASCII only, as the original's "C" locale |
| 16 | Str_NextToken_100fd560 (0x100fd560) | strtok | text | SDL_strtok_r | partly | own | re-entrant form: the caller keeps the position pointer |
| 14 | Sim_100fd710 (0x100fd710) | sscanf | format | SDL_sscanf | yes | libc | our Windows build passes to the C library's vsscanf; SDL's own scanner elsewhere |
| 13 | Crt_AToI (0x100fd1b0) | atoi | text | SDL_atoi | yes | libc |  |
| 13 | Crt_FSeek (0x100feb20) | fseek | files | SDL_SeekIO | yes | own |  |
| 13 | Crt_Close (0x101049c0) | _close | files | SDL_CloseIO | partly | own | low-level handle (int) I/O: no numbered handles in SDL, an SDL_IOStream* instead |
| 13 | Sim_1010d240 (0x1010d240) | _strnicmp | text | SDL_strncasecmp | partly | own | UTF-8 case folding, as SDL_strcasecmp |
| 12 | Crt_IsCType (0x100fd9a0) | _isctype (isalpha, isdigit, isspace ...) | text | SDL_isalpha, SDL_isdigit, SDL_isspace ... | yes | own | ASCII ("C" locale), as the original |
| 11 | Stream_ReadLine (0x100fe770) | fgets | files | - | no | - | no line reading in SDL_IOStream: a small loop over SDL_ReadIO (or SDL_LoadFile and split) |
| 8 | Str_NCat (0x100fd870) | strncat | text | SDL_strlcat | partly | own | different meaning: strncat counts the characters appended, SDL_strlcat the whole buffer |
| 8 | Math_AbsInt_100fd100 (0x100fd100) | abs | rest | SDL_abs | yes | libc |  |
| 7 | Crt_ToUpper (0x100fdb80) | toupper | text | SDL_toupper | yes | own | ASCII |
| 7 | Sim_10109980 (0x10109980) | _open | files | SDL_IOFromFile | partly | own | O_* flags and share mode map onto mode strings; no numbered handle |
| 7 | Crt_Write (0x10105750) | _write | files | SDL_WriteIO | partly | own | low-level handle I/O (text-mode translation of _O_TEXT handles not offered) |
| 6 | Crt_FTell (0x100fe970) | ftell | files | SDL_TellIO | yes | own |  |
| 6 | Crt_LSeek (0x10105fe0) | _lseek | files | SDL_SeekIO | partly | own | low-level handle I/O |
| 5 | Sim_1010c9f0 (0x1010c9f0) | _strdup | text | SDL_strdup | yes | own | free with SDL_free |
| 4 | Crt_Read (0x10105b50) | _read | files | SDL_ReadIO | partly | own | low-level handle I/O (text-mode translation not offered) |
| 4 | Heap_Realloc (0x101003e0) | realloc | memory | SDL_realloc | yes | libc |  |
| 3 | Crt_QSort (0x100fdcc0) | qsort | sort | SDL_qsort | yes | own | SDL's own sort: equal items may end in another order |
| 3 | Text_IntToRadix (0x100ffde0) | _itoa | text | SDL_itoa | yes | libc |  |
| 3 | Crt_Printf (0x100fdc80) | printf | format | SDL_Log | partly | own | to SDL's log instead of stdout (the game has no console) |
| 3 | Crt_FindFirst (0x100fe470) | _findfirst | files | SDL_GlobDirectory / SDL_EnumerateDirectory | partly | own | a list or a callback instead of a find handle; size / times through SDL_GetPathInfo |
| 3 | Crt_FindNext (0x100fe5a0) | _findnext | files | SDL_GlobDirectory / SDL_EnumerateDirectory | partly | own | as _findfirst |
| 3 | File_FindClose (0x100fe6c0) | _findclose | files | SDL_free (of the glob list) | partly | own | as _findfirst |
| 3 | Crt_FlushBuf (0x100ff030) | _flsbuf (putc) | files | SDL_WriteU8 / SDL_WriteIO | yes | own | putc of one byte |
| 3 | Proc_Exit (0x100ff3b0) | exit | rest | - | no | - | SDL has no exit: SDL_Quit, then return from main (or the platform's exit) |
| 3 | Crt_ToLower (0x100ff280) | tolower | text | SDL_tolower | yes | own | ASCII |
| 3 | Crt_StrSpn (0x100ff160) | strspn | text | - | no | - | not in SDL3: a few lines of our own |
| 3 | Str_FindLast (0x100ffdb0) | strrchr | text | SDL_strrchr | yes | libc |  |
| 2 | Crt_VSPrintf (0x101006c0) | vsprintf | format | SDL_vsnprintf | partly | own | needs the buffer size; format gaps |
| 2 | Crt_Remove (0x100fdb30) | remove | files | SDL_RemovePath | yes | own | also removes an empty directory |
| 2 | Mem_Copy (0x100fdf00) | memcpy | memory | SDL_memcpy | yes | libc |  |
| 2 | Crt_VSNPrintf (0x100fe3e0) | _vsnprintf | format | SDL_vsnprintf | partly | own | _vsnprintf returns -1 and does not terminate when the text is cut; SDL_vsnprintf always terminates and returns the full length; format gaps |
| 2 | File_GetLength (0x10100730) | _filelength | files | SDL_GetIOSize | yes | own | on an SDL_IOStream |
| 2 | Str_CSpan (0x100ff1a0) | strcspn | text | - | no | - | not in SDL3: a few lines of our own |
| 2 | Sim_100ff4e0 (0x100ff4e0) | ctime | time | SDL_TimeToDateTime | partly | own | gives the fields; the "Sun Nov 01 21:00:00 2026" text is ours to format |
| 2 | Crt_Time (0x100ff500) | time | time | SDL_GetCurrentTime | partly | own | nanoseconds since 1970: divide by 1e9 (SDL_NS_TO_SECONDS) |
| 2 | Crt_GetEnv (0x10100130) | getenv | rest | SDL_getenv | yes | own | SDL's copy of the environment, taken at start-up |
| 2 | Crt_Rand (0x101003a0) | rand (stays the exact copy) | rest | SDL_rand (not used) | not used | - | Diego: rand / srand stay exact (game logic); SDL_rand is another generator |
| 2 | Crt_StrNSet (0x1010d340) | _strnset | text | - | no | - | not in SDL3: a two-line loop |
| 2 | Math_AbsInt_101003d0 (0x101003d0) | labs | rest | SDL_abs | yes | libc |  |
| 2 | Search_Binary (0x10100620) | bsearch | sort | SDL_bsearch | yes | own |  |
| 1 | Sim_100fd540 (0x100fd540) | strtoul | text | SDL_strtoul | yes | libc |  |
| 1 | Sim_100fd290 (0x100fd290) | strtol | text | SDL_strtol | yes | libc |  |
| 1 | Sim_100fe450 (0x100fe450) | fscanf | format | - | no | - | not in SDL3: read the text (SDL_ReadIO / own fgets) and SDL_sscanf it |
| 1 | Crt_FPuts (0x100fe390) | fputs | files | SDL_WriteIO | yes | own | SDL_WriteIO(io, s, SDL_strlen(s)) |
| 1 | Crt_FFlush (0x100fe240) | fflush | files | SDL_FlushIO | yes | own |  |
| 1 | 0x1010026a | _abnormal_termination (original) | rest | - | n/a | - | the compiler's __try / __finally helper, not a library function: stays with MSVC's structured exceptions (or the __finally is rewritten) |
| 1 | Heap_Check (0x101000d0) | _heapchk | memory | - | no | - | no heap check in SDL (debug-only use) |
| 1 | Sim_1010d2f0 (0x1010d2f0) | _putch | rest | SDL_Log | partly | own | console character output: no console in SDL; to the log |
| 1 | Sim_10100390 (0x10100390) | srand (stays the exact copy) | rest | SDL_srand (not used) | not used | - | as rand |
| 1 | Heap_Calloc (0x10100580) | calloc | memory | SDL_calloc | yes | libc |  |

## printf formats our code uses, against SDL_snprintf

From every format string in our C and every DLL string our C refers to (by address) that holds a conversion. SDL_vsnprintf (SDL_string.c) is SDL's own code on every platform (only Watcom passes it to the C library). It knows the flags `-` `+` `#` `0`, widths and precisions (also `*`), the sizes `h` `l` `ll` `I64` `z`, and `%c %d %i %u %x %X %o %p %f %g %s %S %%`.

| conversion | uses | SDL | example |
|---|---|---|---|
| `%s` | 219 | yes | DLL 0x10150a34: %c:\%s\%s6.%s |
| `%d` | 155 | yes | w28_plugin.c: LoadPlugin(%s): kind=%s id=%d CheckFunc=%d; game wants id=%d |
| `%N.Nd` | 22 | yes | DLL 0x1012dd74: %4.4d |
| `%Nd` | 20 | yes | DLL 0x10131324: %6ld : %2d Mech %2d : state %8s %4s %2d, objective %4s %-12s |
| `%f` | 20 | yes | DLL 0x1013175c: %f,%f,%f,%f |
| `%N.Nf` | 19 | yes | DLL 0x1012dda0: txyz: %04.4f %04.4f %04.4f |
| `%%` | 17 | yes | w37_info.c: mov %%cs, %0 |
| `%c` | 14 | yes | DLL 0x10150a34: %c:\%s\%s6.%s |
| `%N.Ns` | 11 | yes | DLL 0x1013105c: %c%2.2d/%1.1d % 2.2d/%3.3s/%3.3d  %3.3s %1.1s% 3.3d %1.1s %3 |
| `%ld` | 9 | yes | w28_plugin.c: PluginChoice kind=%s name=%s index=%ld\n |
| `%u` | 9 | yes | DLL 0x1013a340: Refresh mode %d selected with profile time: %u  |
| `%Nx` | 8 | yes | DLL 0x10131904:         To %04x %04x %04x %04x  |
| `%Nf` | 6 | yes | DLL 0x1012ddd8: txyz: %07f %07f %07f rxyz: %07f %07f %07f  |
| `%Nld` | 6 | yes | DLL 0x10131324: %6ld : %2d Mech %2d : state %8s %4s %2d, objective %4s %-12s |
| `%Ns` | 6 | yes | DLL 0x10131324: %6ld : %2d Mech %2d : state %8s %4s %2d, objective %4s %-12s |
| `%i` | 5 | yes | DLL 0x10151e20: %s: %i |
| `%N.Nld` | 4 | yes | DLL 0x1012dcb0: Framerate %2.2ld.%1.1ld |
| `%hd` | 4 | yes | DLL 0x10149fd4: DisplayDibWindowDraw err: %hd  |
| `%x` | 2 | yes | DLL 0x1014b9bc: pe: %x  |
| `%NX` | 2 | yes | DLL 0x10151448: %02X |
| `% N.Nd` | 2 | **no**: the space flag | DLL 0x1013105c: %c%2.2d/%1.1d % 2.2d/%3.3s/%3.3d  %3.3s %1.1s% 3.3d %1.1s %3 |
| `%li` | 1 | yes | DLL 0x1012de18: Items in cache: %li      |
| `%-Ns` | 1 | yes | DLL 0x10131324: %6ld : %2d Mech %2d : state %8s %4s %2d, objective %4s %-12s |
| `%.Nlx` | 1 | yes | DLL 0x1013affc: TSFF_SIM_Effect() failed. Return code: %.8lx  |
| `%N.Nlf` | 1 | yes | DLL 0x1014bfe8: %s%3.1lf |
| `%o` | 1 | yes | DLL 0x10150970: %o |
| `%.Nf` | 1 | yes | DLL 0x10151e7c: %.2f m |

- **The space flag** (`% 2.2d`, `% 3.3d` in one DLL format string at 0x1013105c, 2 uses) is not known to SDL_snprintf: SDL stops at the space and prints the rest literally. Only that string is affected.
- `%f` / `%lf` with widths and precisions: SDL's own float printing (SDL_PrintFloat) builds the digits by repeated multiplication by 10; the last digit can differ from the Microsoft runtime's (and the original's) rounding for some values. Mostly debug and log texts (positions, frame rate); `%s%3.1lf` (0x1014bfe8) and `%.2f m` (0x10151e7c) may reach the screen.
- No `%e`, `%E`, `%G`, `%n` are used, which SDL lacks or treats differently.

## File modes in use

`r` 17, `rb` 17, `wb` 7, `w` 4, `wt` 4, `a` 2, `at` 2, `rt` 2, `a+` 2, `ab` 1, `r+b` 1, `w+b` 1, `w+` 1 (DLL strings our C refers to, and literals). SDL_IOFromFile takes `r` `w` `a` `r+` `w+` `a+` with an optional `b` and always works in binary: the **text modes** (`r` `w` `a` `rt` `wt` `at` `a+` `w+`, 34 places) lose the CRLF translation: files we write get LF line ends, files we read keep their CR before each LF (fgets / fscanf users must drop it). The `t` letter itself is not accepted (it must be dropped).

## Notable gaps

1. **Text-mode files** (above): SDL_IOStream is binary only.
2. **No fgets, fscanf, strspn, strcspn, _strnset, exit, _heapchk** in SDL3: small own code (fgets / fscanf over SDL_ReadIO and SDL_sscanf).
3. **Case-insensitive compares** (_stricmp 53 calls, _strnicmp 13): SDL_strcasecmp folds UTF-8; game texts with bytes 0x80-0xff (accented letters in Latin-1) would compare differently. ASCII is the same.
4. **strncpy / strncat** have no SDL twin with the same meaning (SDL_strlcpy / SDL_strlcat differ in padding, termination and the size counted): 69 calls to check one by one.
5. **sprintf / strcpy / strcat / vsprintf** need the destination size with SDL (snprintf / strlcpy / strlcat): 372 calls where the buffer size has to be found.
6. **printf formatting**: the space flag (one string) and float digits (above).
7. **_assert** (42 calls, original code): SDL_assert reports in its own way.
8. **Low-level handle I/O** (_open / _read / _write / _lseek / _close, 33 calls) and **_findfirst / _findnext** map onto SDL_IOStream and SDL_GlobDirectory with different shapes (no numbered handles, no find handle).
9. **_abnormal_termination** is part of MSVC's __try / __finally, not a library call (1 use).
10. **SDL passes to the system C library on Windows** for strlen, strcmp, strncmp, strstr, strchr, strrchr, strtol, strtoul, atoi, itoa, abs, sscanf, malloc / free / realloc / calloc, memcpy, memmove, memset: on our build these are the Microsoft runtime's own code, not SDL's (the results are the same; only "same code everywhere" does not hold for them).
