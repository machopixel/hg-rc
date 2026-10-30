/*
 * Round 7, letter n: input device / channel set-up (0x1005366e..0x10057c1a) and the entry loader at 0x1005a91a.
 * Helpers that are not converted yet (or that another hook file patches) are called through their original
 * address with SIM_FN and the prototypes below.
 *
 * Input device records: 0x74 bytes at 0x1015f880 (at most 5, count [0x10164bc0]); the device name given by the
 * caller is kept in the 0x28-byte slots at 0x1015fc58, the driver that opened it at 0x1015fd20 and an "opened"
 * flag at 0x1015f868 (one dword per record). A driver (table of three at 0x1013f150) is a block of function
 * pointers: +0 count(), +4 name(index, record) (0 = the record now holds that device's name), +8 open(record)
 * (0 = success), +0xc close(record).
 */
#ifndef HG_SIM_UNSORTED_1005_W7N_H
#define HG_SIM_UNSORTED_1005_W7N_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Patched by hooks_sim.c (system error report): (code, format, ...). */
#define W7N_CALL_100bc280 SIM_FN(int(__cdecl *)(int, const char *, ...), 0x100bc280)
/* Not converted: error report (code, a, b) used by the input set-up. */
#define W7N_CALL_1005573d SIM_FN(int(__cdecl *)(int, const void *, const void *), 0x1005573d)
/* Not converted: the C library sprintf (destination, format, ...). */
#define W7N_CALL_100fc780 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: one-argument character mapping of the C library (in the w7r list). */
#define W7N_CALL_100ff280 SIM_FN(int(__cdecl *)(int), 0x100ff280)
/* Not converted: (value, &next value, entry +0x168, 0, entry +0x78, index), see Sim_1005a91a. */
#define W7N_CALL_1005ab44 SIM_FN(int(__cdecl *)(int, int *, void *, int, uint32_t, int), 0x1005ab44)

/*
 * Looks for the device `name` in every driver of the table 0x1013f150: asks each driver for its device names one
 * by one into the next free record; a name that is not the wanted one is closed again (+0xc). On a match the name
 * and driver are stored, "joystick1" also sets [0x1013f144], the count goes up and the driver opens the record:
 * returns the record index, or -1 (error 0x70 reported) when opening fails. With five records in use it reports
 * "Too many input devices specified" and returns -1. A name no driver knows takes a record anyway (flag 0, error
 * 0x6e reported) and gives -1.
 */
int __cdecl Input_AddDevice(const char *name);
/*
 * The record of device `name` if it is already listed (its index, or -1 when it did not open), else
 * Input_AddDevice(name). For "keyboard" the result is also kept in [0x1015fdf4] ("Keyboard initialization error"
 * reported when it is -1).
 */
int __cdecl Input_GetDevice(const char *name);
/*
 * Index of the channel `name` in the name list at record +0x64 (count +0x58) of device `device`; a name that
 * starts with a digit is a number (atoi). -1 when the device is -1, when it has no list ("No input channel"
 * reported with the line number [0x1015fdf8]) or when the name is not in it (error 0x6f reported).
 */
int __cdecl Sim_1005393a(int device, const char *name);
/* The same with the name list at record +0x6c (count +0x5c). */
int __cdecl Sim_10053a4f(int device, const char *name);
/*
 * Reads lines from the stream into buf (at most n bytes, counting them in [0x1015fdf8]) and cuts each at the first
 * '\n' or '#', until a line has a character that is not white space: returns 1, or 0 at the end of the stream.
 */
int __cdecl Input_ReadLine(char *buf, int n, uint8_t *stream);
/*
 * The 0x32-byte record at 0x10161700 (count [0x1015fdec], at most 0x32) for the source whose id (+4) matches, or a
 * new one filled from the source (+0 name, +0xc, +0x10, +0x14 range, +0x18, +0x1c byte) and the channel table
 * entries name_plus, name_minus, name_set and name_reset ("_delta" cut off the name for the first two). Returns 0
 * ("Too many analog sinks" reported) when the table is full.
 */
uint8_t *__cdecl Input_AddAnalogSink(const uint32_t *src);
/*
 * Reads the OEM name of joystick `index` (0-based) from the registry key of `driver`
 * (...\MediaResources\Joystick\<driver>\CurrentJoystickSettings, value Joystick<n>OEMName, then
 * ...\MediaProperties\PrivateProperties\Joystick\OEM\<that>, value OEMName) and copies at most n characters to
 * out. Returns 1 on success, else 0. Uses the buffers 0x10166720 (key name) and 0x101666e0 (value).
 */
int __cdecl Joy_GetOemName(int index, const char *driver, char *out, uint32_t n);
/*
 * Queues the character of virtual key `key` (table 0x10143a00, else MapVirtualKeyA type 2 passed through
 * 0x100ff280) with 0x100 / 0x400 / 0x200 added while GetKeyState(0x11 / 0x12 / 0x10) has bit 0x1000 set, in the
 * 64-entry ring at 0x10165ee0 (write index [0x1016686c], read index [0x10166868]). Nothing for character 0.
 */
void __cdecl Input_QueueKey(int key);
/*
 * For entry `index` of the 0x20-byte table at [obj+0x88]+0x7c (the next entry when the name starts with 'g' or
 * 'G'): Sim_1005aa45(entry name, value, -1, -1, word [obj+0x88]+0x10, 0x100, transform, index) with the transform
 * (1, 1, 1, 0, 0, 0, -x, -y, -z) of the floats at [obj+0x88]+0x15c, then 0x1005ab44.
 */
void __cdecl Sim_1005a91a(void *obj, int index, int value);

#endif
