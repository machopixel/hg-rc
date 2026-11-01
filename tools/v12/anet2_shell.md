# How 1.2's menus (HGSHELL.DLL) use ANET2 - step 1 of the transport plan (read-only, 2026-11-01)

Read from 1.2's hgshell.dll (Ghidra through rea), not from memory. All ANET2 calls go through one wrapper object
(a C++ class, `this` in ecx: +4 the session handle `dp`, +0x3c the last result) at 0x1002a32f .. 0x1002dcd1, plus
the lobby's receive loop 0x1011f0bf (about 3900 lines) and 12 senders at 0x10128e3a .. 0x10129f2b.
Signatures are as the shell calls them (cdecl; argument meaning from use).

## Start-up (0x1002a32f)
- Reads `shellnet.ini` for the game server's address (default `206.79.108.85`), then `dpini_SetFile("gtest.ini")`,
  `dp_enableDebugPrint(1)`.
- `dpEnumTransports(path[0x80], callback, this)`. The callback (0x1002e06e) keeps four transports by name: "Internet"
  (slot 1), "Null Modem" (3), "Modem" (2), a fourth name at 0x10203c44 (slot 0). Each is the 0x80-byte path plus
  0x32 bytes of description.

## Create / destroy (0x1002a794, 0x1002abb8)
- `dpCreate(&dp, transport, commInitReq, 0)`. commInitReq is 0x2c bytes: +0 size 0x2c, +4 a random session id
  (rand / time / clock); for modem / null modem also a phone number / port block (+0x1c / +0x18 / +0x20.. from the
  object).
- Transport 1 (Internet) then lists the game servers (`dpEnumServers(dp, 2000, callback, this)`), picks the
  lowest-latency one (`dpSetGameServer(dp, address)`) and asks `dpGetAppVersion(dp, buf)`.
- `dpDestroy(dp, 0)`.

## Sessions (games)
- List: `dpEnumSessions(dp, template, 0, 0, callback, this)`. The template is a session record whose +0xd word is
  the game type. The callback gets each 0x5f-byte session record (the shell keeps them at this+0x1d2,
  0x5f apart, with the session id word at +0xf).
- Host: `dpOpen(dp, newSession, callback, this)`. newSession is 0x5f bytes (0x1002bf91):
  - +0xd: the game type word;
  - +0x11: a word (maximum players);
  - +0x15: the flags word, 1;
  - +0x17: the name;
  - +0x4f: a flags dword (bits 0..2 read back by the shell).
- Join: `dpOpen(dp, &sessions[i], callback, this)`.
- `dpClose(dp)`.
- `dpGetSessionDesc(dp, buf, &size = 0x5f)` / `dpSetSessionDesc(dp, session, 0)`. The host keeps the game's
  settings in the description: 10 bytes at +0x37, and the flags dword at +0x4f.
- The session record as the list callback shows it: +0xf the session id word, +0x13 a word (current players: the
  list keeps only sessions with +0x13 > 0 or +0x15 flag 8), +0x15 the flags word.

## Players and teams
- `dpCreatePlayer(dp, callback, this, name)`; the callback gets the new id.
- `dpDestroyPlayer(dp, id)`, `dpSetPlayerName(dp, id, name)`, `dpGetPlayerName(dp, id, buf, len)`.
- `dpGetPlayerData(dp, id, 0, buf, &size, 0)`.
- `dpEnumPlayers(dp, session or 0, callback(id, name, flags, ctx), ctx, 0x6d6 ms)`. The list ends with id 0xfa01.
- Teams are ANET2 groups:
  - `dpCreateGroup(dp, &id, name)`, `dpDestroyGroup(dp, id)`;
  - `dpAddPlayerToGroup(dp, group, player)`, `dpDeletePlayerFromGroup(dp, group, player)`;
  - `dpEnumGroups(dp, 0, callback, ctx, 1000)`, `dpEnumGroupPlayers(dp, group, 0, callback, ctx, 1000)`.

## Messages
- `dpSend(dp, from, to (0 = everyone), flags (1 = guaranteed), buf, len)`.
- `dpReceive(dp, &from, &to, 0, buf, &size)`. It returns 2 when nothing is waiting; the shell tries 3 times.
- System notices come through `dpReceive` as 2-letter tags (little-endian words):
  - "d1": a player was added;
  - "d2": a player was deleted, with the player's id at +2 and the name at +0x10 (0x18 bytes). This is the
    layout the sim's NETLEFT_FIX reads; it confirms Diego's option B;
  - "d5" / "d6": a player was added to / removed from a group, with the group at +2 and the player at +4;
  - "d7": the session description changed.
- The shell's own messages, by tag:
  - "S0": chat, 0xf9 / 0xf4 bytes;
  - "S1": 8 bytes: id, a dword;
  - "S2";
  - "H0" .. "H8": host lobby control;
  - "GO", "DA", "DM", "CH", "PU", "SU", "KA", "HA", "KI".
- `dpReadyToFreeze(dp)` is polled while it answers 6, before the game starts.
- `dpDownloadUpdate(dp, buf)`, `dpGetAppVersion(dp, buf)`: the online update check.

## Direct sockets (WSOCK32)
- 0x101199c0 .. 0x10119d41: a small TCP client (WSAStartup, socket, bind, gethostbyname / inet_addr, connect,
  select, send / recv, shutdown, closesocket). It is outside ANET2: probably the server / update check. Its use is
  still to read.

## The callbacks (all cdecl; the last argument is the caller's context)
The wrapper's methods are a table at 0x101a7dd0, used as +0 create, +4 destroy, +8 receive pump, +0xc close,
+0x14 create player, +0x18 player list. The callbacks forward to its +0x20 .. +0x40 methods:
- `dpEnumServers`: (hostname, roundtrip ms, info*, flags, ctx). Returns non-zero to go on. The shell keeps the
  hostname, the roundtrip, the info's first dword and the flags; a NULL hostname ends the list.
- `dpEnumSessions`: (session*, timeout*, flags, ctx). Returns non-zero to go on; a NULL session ends the list.
- `dpOpen` when hosting: (session*, timeout*, flags, ctx). The shell copies the new session record and takes its id
  (+0xf).
- `dpOpen` when joining: (session*, timeout*, flags, ctx). A NULL session means failure (the shell sets result
  0x15); otherwise the shell reads flag 8 of +0x4f.
- `dpCreatePlayer`: (id, name, flags, ctx). The shell keeps its own id (wrapper +0x12c) and name.
- `dpEnumPlayers`: (id, name, flags, ctx). Id 0xfa01 ends the list.
- `dpEnumGroups`: (id, name, flags, ctx). A NULL name ends the list.
- `dpEnumGroupPlayers`: (id, name, flags, ctx). The ctx is a 6-byte block: the wrapper and the group id.
- `dpEnumTransports`: (path[0x80], info[0x32], ctx).

## The direct TCP client
- 0x1011ee52: only in the "Internet with game servers" mode, and only when `cookie.ini` exists.
- It reads a host name from that file and connects to TCP port 3333 (0xd05) there (0x101199c0). The send / receive
  helpers are at 0x10119c6f / 0x10119d41.
- It was an Activision online service. Without cookie.ini it is skipped (result 0xb), so the stand-in needs nothing
  for it.

## What the sim takes from the shell
- The shell builds the game record and the sim keeps it at [0x1015d004]: +0xc the session handle `dp`, +0x10 the
  own player's key, +0x12 the player count, +0x16 a pointer to the keys (words), +0x126 a pointer to the 0x1e-byte
  player records (id word at +0x1a, side byte at +0x1c), and the two setting bytes at +0x137 / +0x138.
- The sim then calls `dp` through its own table of ANET2 functions (w11_sound.c, 55 names from the ANET2 module
  [0x1015eb68]). It only calls 8 of them; see DECISIONS.md, network round.
- So the stand-in's session handle must be the same object for the shell and the sim, as ANET2's is today.

## Not needed for the stand-in
- The shell's own message layouts (H0..H8, S0..S2, GO, ...): the stand-in only carries those bytes.
