#include "w4u.h"
#include "w4u_api.h"

#define CD_DEVICE (*SIM_PTR(uint32_t, 0x1014c9b0))
#define CD_OPEN (*SIM_PTR(uint32_t, 0x1017d08c))
#define CD_RANGE SIM_PTR(uint32_t, 0x1017d080)
#define CD_SAVED SIM_PTR(uint32_t, 0x1017d068)

#define MCI_CLOSE 0x804u
#define MCI_PLAY 0x806u
#define MCI_STOP 0x808u
#define MCI_PAUSE 0x809u
#define MCI_SET 0x80du
#define MCI_STATUS 0x814u
#define MCI_RESUME 0x855u

static uint32_t Mci(uint32_t message, uint32_t flags, void *parms)
{
    return SIM_API(W4uMciSendFn, W4U_IAT_MCISENDCOMMAND)(CD_DEVICE, message, flags, parms);
}

// FUNCTION: HGSIM 0x1009dec0
int __cdecl Aux_FindCdDevice(void)
{
    uint32_t caps[12];
    int found = 0;
    int result = -1;
    int count = (int)SIM_API(W4uAuxGetNumDevsFn, W4U_IAT_AUXGETNUMDEVS)();
    int i;

    if (count > 0) {
        for (i = 0; i < count && !found; i++) {
            if (SIM_API(W4uAuxGetDevCapsFn, W4U_IAT_AUXGETDEVCAPS)((uint32_t)i, caps, 0x30))
                return -1;
            if ((caps[10] & 0xffff) == 1) {
                found = 1;
                result = i;
            }
        }
        if (!found) {
            for (i = 0; i < count && !found; i++) {
                if (SIM_API(W4uAuxGetDevCapsFn, W4U_IAT_AUXGETDEVCAPS)((uint32_t)i, caps, 0x30))
                    return -1;
                if ((caps[10] & 0xffff) == 2) {
                    found = 1;
                    result = i;
                }
            }
        }
    }
    return result;
}

// FUNCTION: HGSIM 0x1009e11d
int __cdecl Cd_Close(void)
{
    uint32_t parms[1];

    Mci(MCI_CLOSE, 0, parms);
    CD_DEVICE = 0xffffffffu;
    return 0;
}

// FUNCTION: HGSIM 0x1009e149
uint32_t __cdecl Cd_Pause(void)
{
    uint32_t parms[1];

    return Mci(MCI_PAUSE, 0, parms);
}

// FUNCTION: HGSIM 0x1009e169
uint32_t __cdecl Cd_Resume(void)
{
    uint32_t parms[1];

    return Mci(MCI_RESUME, 0, parms);
}

// FUNCTION: HGSIM 0x1009e189
uint32_t __cdecl Cd_Stop(void)
{
    uint32_t parms[1];

    return Mci(MCI_STOP, 0, parms);
}

// FUNCTION: HGSIM 0x1009e1a9
uint32_t __cdecl Cd_Play(uint32_t from, uint32_t to)
{
    uint32_t parms[3]; /* callback (not set), from, to */
    uint32_t flags = 4;

    parms[1] = from;
    if (to != 0) {
        flags |= 8;
        parms[2] = to;
    }
    return Mci(MCI_PLAY, flags, parms);
}

// FUNCTION: HGSIM 0x1009ea75
void __cdecl Cd_FreeTracks(uint32_t *range)
{
    if (range[2] != 0) {
        W4u_HeapFree(range[2]);
        range[2] = 0;
    }
}

// FUNCTION: HGSIM 0x1009e2b9
void __cdecl Cd_Shutdown(void)
{
    if (CD_OPEN != 0) {
        Cd_FreeTracks(CD_RANGE);
        Cd_Close();
        CD_OPEN = 0;
    }
}

// FUNCTION: HGSIM 0x1009e2e3
int __cdecl Cd_GetStatus(void)
{
    uint32_t parms[4]; /* callback (not set), return value, item, track (not set) */
    uint32_t mode;

    if (CD_OPEN == 0)
        return 5;
    parms[2] = 4;
    if (Mci(MCI_STATUS, 0x100, parms) != 0) {
        Cd_Shutdown();
        return 5;
    }
    mode = parms[1] - 0x20d;
    switch (mode) {
    case 0:
        return 2;
    case 1:
        return 3;
    case 4:
        return 4;
    case 5:
        return 1;
    default:
        return 5;
    }
}

// FUNCTION: HGSIM 0x1009e389
int __cdecl Cd_UpdateStatus(void)
{
    *SIM_PTR(int, 0x1017d078) = Cd_GetStatus();
    return *SIM_PTR(int, 0x1017d078);
}

// FUNCTION: HGSIM 0x1009eaa2
void __cdecl Cd_ReadPosition(uint32_t *out)
{
    uint32_t parms[4]; /* callback (not set), return value, item, track (not set) */

    if (CD_OPEN == 0)
        return;
    parms[2] = 2;
    if (Mci(MCI_STATUS, 0x100, parms) == 0) {
        out[0] = parms[1] & 0xff;
        out[1] = ((int32_t)(parms[1] & 0xffff) >> 8) & 0xff;
        out[2] = (parms[1] >> 16) & 0xff;
        out[3] = (parms[1] >> 24) & 0xff;
    }
}

// FUNCTION: HGSIM 0x1009e505
void __cdecl Cd_GetPosition(uint32_t *out)
{
    uint32_t state;

    if (CD_OPEN == 0)
        return;
    state = (uint32_t)Cd_GetStatus() - 1;
    if (state > 3)
        return;
    switch (state) {
    case 0:
    case 1:
        out[0] = 0;
        out[1] = 0;
        out[2] = 0;
        out[3] = 0;
        break;
    case 2:
        Cd_ReadPosition(out);
        break;
    case 3:
        out[0] = CD_SAVED[0];
        out[1] = CD_SAVED[1];
        out[2] = CD_SAVED[2];
        out[3] = CD_SAVED[3];
        break;
    }
}

// FUNCTION: HGSIM 0x1009e3cb
void __cdecl Cd_TogglePause(void)
{
    uint32_t state;

    if (CD_OPEN == 0)
        return;
    state = (uint32_t)Cd_GetStatus() - 1;
    if (state > 3)
        return;
    switch (state) {
    case 1:
        Cd_Play(*SIM_PTR(uint32_t, 0x1017d090), *SIM_PTR(uint32_t, 0x1017d094));
        break;
    case 2:
        Cd_GetPosition(CD_SAVED);
        Cd_Pause();
        break;
    case 3:
        Cd_Resume();
        break;
    }
}

// FUNCTION: HGSIM 0x1009e441
void __cdecl Sim_1009e441(void)
{
    uint32_t state;

    if (CD_OPEN == 0)
        return;
    state = (uint32_t)Cd_GetStatus() - 1;
    if (state > 3)
        return;
    switch (state) {
    case 1:
        Cd_Play(**SIM_PTR(uint32_t *, 0x1017d088), 0);
        break;
    case 2:
        Cd_GetPosition(CD_SAVED);
        break;
    case 3:
        Cd_Resume();
        break;
    }
}

// FUNCTION: HGSIM 0x1009e4ae
void __cdecl Cd_StopWait(void)
{
    if (CD_OPEN != 0) {
        do {
            Cd_Stop();
        } while (Cd_GetStatus() == 3);
    }
}

// FUNCTION: HGSIM 0x1009e680
void __cdecl Cd_Skip(const uint32_t *range, int delta)
{
    uint32_t position[4];
    int status;
    int target;

    if (CD_OPEN == 0)
        return;
    status = Cd_GetStatus();
    if (status == 1 || status <= 2 || status > 4)
        return;
    Cd_GetPosition(position);
    target = (int)position[0] + delta;
    if (target < (int)range[0] || target > (int)range[1])
        return;
    if (status == 3) {
        Cd_Stop();
        Cd_Play((*SIM_PTR(uint32_t *, 0x1017d088))[target - (int)CD_RANGE[0]], 0);
    } else if (status == 4) {
        Cd_Play((*SIM_PTR(uint32_t *, 0x1017d088))[target - (int)CD_RANGE[0]], 0);
        Cd_Stop();
        CD_SAVED[0] = (uint32_t)target;
        CD_SAVED[1] = 0;
        CD_SAVED[2] = 0;
    }
}

// FUNCTION: HGSIM 0x1009e4cb
void __cdecl Cd_PrevTrack(void)
{
    if (CD_OPEN != 0)
        Cd_Skip(CD_RANGE, -1);
}

// FUNCTION: HGSIM 0x1009e4e8
void __cdecl Cd_NextTrack(void)
{
    if (CD_OPEN != 0)
        Cd_Skip(CD_RANGE, 1);
}

// FUNCTION: HGSIM 0x1009e768
void __cdecl Cd_PlayTrack(int track)
{
    uint32_t position[4];
    const uint32_t *range = CD_RANGE;
    int status;

    if (CD_OPEN == 0)
        return;
    *SIM_PTR(uint32_t, 0x1017d090) = (*SIM_PTR(uint32_t *, 0x1017d088))[track - (int)range[0]];
    *SIM_PTR(uint32_t, 0x1017d094) = (*SIM_PTR(uint32_t *, 0x1017d088))[track + 1 - (int)range[0]];
    status = Cd_GetStatus();
    if (status == 1 || status <= 1 || status > 4)
        return;
    Cd_GetPosition(position);
    if (track < (int)range[0] || track > (int)range[1])
        return;
    if (status == 3) {
        Cd_Stop();
        Cd_Play(*SIM_PTR(uint32_t, 0x1017d090), *SIM_PTR(uint32_t, 0x1017d094));
    } else if (status == 4) {
        Cd_Play(*SIM_PTR(uint32_t, 0x1017d090), *SIM_PTR(uint32_t, 0x1017d094));
        CD_SAVED[0] = (uint32_t)track;
        CD_SAVED[1] = 0;
        CD_SAVED[2] = 0;
    } else {
        Cd_Play(*SIM_PTR(uint32_t, 0x1017d090), *SIM_PTR(uint32_t, 0x1017d094));
    }
}

// FUNCTION: HGSIM 0x1009e886
void __cdecl Cd_PlayRange(uint32_t from, uint32_t to)
{
    uint32_t position[4];
    int status;

    if (CD_OPEN == 0)
        return;
    status = Cd_GetStatus();
    if (status == 1 || status <= 1 || status > 4)
        return;
    Cd_GetPosition(position);
    if (to <= from)
        return;
    if (status == 3) {
        Cd_Stop();
        Cd_Play(from, to);
    } else if (status == 4) {
        Cd_Play(from, to);
        CD_SAVED[0] = 0;
        CD_SAVED[1] = 0;
        CD_SAVED[2] = 0;
    } else {
        Cd_Play(from, to);
    }
}

// FUNCTION: HGSIM 0x1009e947
int __cdecl Cd_LoadTracks(uint32_t *range)
{
    uint32_t parms[4]; /* callback (not set), return value, item, track */
    int count;
    int i;

    if (CD_OPEN == 0)
        return 1;
    parms[2] = 3;
    if (Mci(MCI_STATUS, 0x100, parms) != 0) {
        Cd_Shutdown();
        return 0;
    }
    count = (int)parms[1];
    range[0] = 1;
    range[1] = (uint32_t)count;
    range[2] = W4u_HeapAlloc((uint32_t)count * 4 + 4);
    if (range[2] == 0) {
        CD_OPEN = 0;
        return 1;
    }
    for (i = (int)range[0]; i <= (int)range[1]; i++) {
        parms[2] = 2;
        parms[3] = (uint32_t)i;
        if (Mci(MCI_STATUS, 0x110, parms) != 0) {
            Cd_Shutdown();
            return 0;
        }
        ((uint32_t *)(uintptr_t)range[2])[i - (int)range[0]] = parms[1];
    }
    ((uint32_t *)(uintptr_t)range[2])[i - (int)range[0]] = (range[1] + 1) & 0xff;
    return 0;
}
