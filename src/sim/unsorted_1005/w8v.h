/*
 * Round 8, letter v: functions at 0x10050e7e..0x1005f949: HUD gauges (compass needle and ring, bars, frame),
 * the HG.INI reader, the input mapping files (inputw.map, chords, analog controls), small record files
 * and the hg.log file. Helpers that are not converted yet (or that another round-8 helper converts at the same
 * time) are called through their original address with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1005_W8V_H
#define HG_SIM_UNSORTED_1005_W8V_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* A line on a surface (surface, x1, y1, x2, y2, 0, colour): 0x100ef727 takes a 16-bit colour (patched by
 * hooks_hud.c), 0x100e86a1 a palette index. */
#define W8V_LINE16 SIM_FN(void(__cdecl *)(uint32_t, int32_t, int32_t, int32_t, int32_t, int, uint32_t), 0x100ef727)
#define W8V_LINE8 SIM_FN(void(__cdecl *)(uint32_t, int32_t, int32_t, int32_t, int32_t, int, uint32_t), 0x100e86a1)
/* A filled box (surface, x, y, width, height, colour). */
#define W8V_CALL_10051c90 SIM_FN(void(__cdecl *)(uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t), 0x10051c90)
/* Draws an image (0, 0, image, surface). */
#define W8V_CALL_100652e8 SIM_FN(void(__cdecl *)(int, int, uint32_t, uint32_t), 0x100652e8)
/* A table lookup of an angle in degrees; result in st(0). */
#define W8V_CALL_10084766 SIM_FN(float(__cdecl *)(float), 0x10084766)
/* Clips the point pt against the rectangle and writes the result to out (rect, pt, out). */
#define W8V_CALL_1008e9af SIM_FN(void(__cdecl *)(const void *, const int32_t *, int32_t *), 0x1008e9af)
/* C library streams: fwrite, fread, fseek(stream, offset, origin), fclose, sprintf, time. */
#define W8V_FWRITE5 SIM_FN(uint32_t(__cdecl *)(const void *, uint32_t, uint32_t, void *), 0x100febc0)
#define W8V_FREAD5 SIM_FN(uint32_t(__cdecl *)(void *, uint32_t, uint32_t, void *), 0x100fe830)
#define W8V_FSEEK5 SIM_FN(int(__cdecl *)(void *, int32_t, int), 0x100feb20)
#define W8V_FCLOSE5 SIM_FN(int(__cdecl *)(void *), 0x100fd760)
#define W8V_SPRINTF5 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
#define W8V_TIME5 SIM_FN(uint32_t(__cdecl *)(uint32_t *), 0x100ff500)
/* The game's fatal error report SystemError(code, format, ...) (hooks_sim.c taps it). */
#define W8V_SYSERR SIM_FN(int(__cdecl *)(int, const char *, ...), 0x100bc280)
/* Not converted: reads one input mapping file (stream). */
#define W8V_CALL_100543ee SIM_FN(void(__cdecl *)(void *), 0x100543ee)
/* Not converted: finds a resource (name, extension, out size); upper-cases both texts in place. */
#define W8V_CALL_100ba7d5 SIM_FN(void *(__cdecl *)(char *, char *, uint32_t *), 0x100ba7d5)
/* Not converted: loads the GDF data (data, size). */
#define W8V_CALL_10059a34 SIM_FN(uint32_t(__cdecl *)(void *, uint32_t), 0x10059a34)
/* Not converted: (name, extension). */
#define W8V_CALL_100b9203 SIM_FN(void(__cdecl *)(const char *, const char *), 0x100b9203)

/* w8v_hud.c */
void __cdecl Sim_10050e7e(uint8_t *rect, float angle, int32_t colour, uint8_t *rect2);
void __cdecl Sim_100510d9(uint8_t *gauge);
void __cdecl Sim_10051372(uint8_t *gauge);
void __cdecl Sim_10051532(uint8_t *gauge);
void __cdecl Sim_100516c8(uint8_t *gauge);
void __cdecl Sim_100517dc(uint8_t *gauge);

/* w8v_ini.c */
int __cdecl Ini_FindSection(const char *section);
char *__cdecl Ini_GetValue(const char *key, int raw);

/* w8v_input.c */
int __cdecl Input_AddAnalog(const uint32_t *src, const char *device, int32_t index);
int __cdecl Input_ReadChord(void *stream, int32_t *count, uint8_t *out);
int __cdecl Input_LoadMaps(void);

/* w8v_misc.c */
uint32_t __cdecl Sim_100599a0(const char *name);
int __cdecl Sim_10059e5a(const char *name, const uint8_t *head, const uint8_t *items, const uint8_t *tail,
                         const void *data, int32_t size, const void *extra);
int __cdecl Sim_1005a86f(const char *name, char *outName, uint32_t *outValue);
int __cdecl Sim_1005af72(const uint8_t *rec, const char *dir, const char *name);
int __cdecl Log_Open(void);
int __cdecl Log_Close(void);

#endif
