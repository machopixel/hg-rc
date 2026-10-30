# Input

The sim reads input with plain Win32 calls (no DirectInput). Each kind of input is a device driver table in HGSIM.DLL: eight function slots followed by the device name. A new device, such as a gamepad, is one more table of the same shape.

| Device | Table (VA) | Slots we care about |
|---|---|---|
| mouse | 0x10149cc0 | [3] 0x1007d948 release cursor clip, [4] 0x1007d961 re-centre one axis, [5] 0x1007d9d8 poll |
| keyboard | 0x10143d70 | [3] 0x10057a58 copy key state, [4] 0x10057aa9 next typed key, [5] 0x10057afc flush |
| joystick | 0x101426f0 | [2] 0x100567cb setup, [5] 0x10056dfa poll |

## How the sim gets keys

Windows key messages go to the sim's window procedure (0x100867a2), which calls 0x10057b4e. For each message that function calls:

- 0x10057d2d(vk, lParam, down): sets or clears bit (scancode - 1) in a 128-bit state table at 0x1015e650. The scancode is lParam bits 16-23, bit 24 is the extended-key flag, remapped through the table at 0x10143c00 for keys below 0x59.
- 0x10057c1a(vk), on key up: pushes the typed key onto a 64-entry queue (0x10165ee0).

The message loop is 0x10086b99. It drops mouse messages (0x200-0x209) while the cursor is hidden (0x1017b93c = 0) and ends on WM_QUIT.

## How the sim gets the mouse

Poll 0x1007d9d8(?, int *pos, uint *buttons) reads GetCursorPos and GetAsyncKeyState. It treats the window client area as the game's screen mode (0x1017b918 x 0x1017b91c, 640x480 in missions), clips the cursor to that rectangle, and returns the position as 16.16 fixed point from -1 to +1 around the window centre. The sim re-centres the cursor itself through slot [4]. With the Vulkan window at 2560x1440 the clip box covers only the top-left 640x480.

## Plan

`src/platform/input.h` is the only input interface the hook layer uses. `input_sdl3.c` is the SDL3 backend. A console port or a gamepad adds files behind the same interface.

1. **Message loop (done and tested; `sdlloop` in hgrc.cfg):** SDL3 wraps the game window and runs the loop. SDL passes every message on to the sim's window procedure (SDL_windowsevents.c: "if there's a window proc, assume it's going to handle messages"), so nothing changes for the sim. Tested here with a stand-in window under Wine: keys and WM_CLOSE reach the original procedure, mouse messages can be dropped, WM_QUIT reaches the hook with no window.
2. **Keyboard (written; `sdlinput` in hgrc.cfg, dev.cmd option A):** SDL key events are turned into calls of 0x10057d2d and 0x10057c1a (tables from USB scancodes to PC scancode, extended flag and VK in `input_sdl3.c`). The sim's own Windows-message handler 0x10057b4e is replaced by nothing, because it would also pull key messages out of the queue before SDL saw them; the keyboard flush (0x10057afc) and the typed-key read (0x10057f83) pump SDL instead of peeking messages.
3. **Mouse (written; part of `sdlinput`, the extra word `sdlnomouse` leaves it out):** slots [3], [4], [5] are replaced. SDL3 captures the pointer (relative mode, scaled by the Windows pointer speed settings so it feels like the cursor did) and its movement is added to a position inside the game's screen-mode box, 640x480 in a mission. The sim gets the same numbers as before from the same formula, and re-centring just resets that position. There is no clip box on screen any more. Buttons come from `SDL_GetGlobalMouseState`, like the sim's `GetAsyncKeyState` calls. The capture is released when the window is inactive and when the sim returns (`HooksInput_OnSimExit`, which also closes SDL3 so the menus run as before).
4. **Gamepad (later):** a new driver table, or the existing joystick slots, fed from SDL3 gamepad state.
