---
name: claude-play
description: Play and test Adastrea in PIE as the player through UClaudeDriverSubsystem and Tools/claude_play.py, which fly the ship, fight, dock, take screenshots and read a JSON state snapshot. Use whenever a change needs checking in the running game (combat, flight, docking, AI reactions, distress calls) instead of writing a one-off remote-exec PIE script with slate tick callbacks.
---

# Playing the game in PIE

`UClaudeDriverSubsystem` (Source/Adastrea/.../Testing/) is a world subsystem that only exists in PIE and game worlds of non-shipping builds. It runs queued text commands in C++ every frame through the player's own controller and ship. `Tools/claude_play.py` reaches it from the shell, with one short remote-exec call per step.

## Loop

```bash
python Tools/claude_play.py pie start
python Tools/claude_play.py run "cmd SpawnHostiles 2; wait 2; attack hostile 60; stop"
python Tools/claude_play.py wait 120
python Tools/claude_play.py state
python Tools/claude_play.py pie stop
```

`wait` polls until the queue is empty, then prints each command's result, the ship's status and the log's error/warning counts. `state` prints the full JSON: ship, locked target, the 12 nearest contacts (distance, angle off the nose, hull, pilot class), queue, history and log.

With several editors open, `--root <checkout>` picks the editor by project_root (the default is the checkout the script is in). Add `--pid <pid>` when two editors share a root.

## Commands (separate with `;`)

`<ref>` is `target` (the locked target), `hostile` (the nearest hostile ship), `station` (the nearest station), `x,y,z` in metres, or a name fragment (the actor name or display name, nearest match).

| command | effect |
|---|---|
| `fly <ref> [arrive_m=500]` | turn toward and cruise to it, throttle 0 on arrival |
| `face <ref> [tol_deg=3]` | turn the nose onto it |
| `throttle <0-100>` / `stop` | set the throttle / throttle 0 and wait until still |
| `lock <ref>` | lock it as the target (fails if it isn't lockable in range) |
| `fire <s>` | hold the fire input action |
| `attack <ref> [timeout_s=60]` | lock, chase, lead and fire until it is a wreck |
| `dock [ref]` / `undock` | fly to the station's bay and request docking / leave the dock |
| `wait <s>` | do nothing |
| `cmd <console command>` | e.g. `cmd SpawnHostiles 2`, `cmd adastrea.Security 3` |
| `shot <name>` | screenshot of the PIE viewport to `Saved/Screenshots/WindowsEditor/Claude/<name>.png` (the path is in the result; read it with Read) |

Console equivalents from the PIE console: `claude.Run <commands>`, `claude.State`, `claude.Cancel`.

## Notes

- Turning is applied directly to the ship (and the control rotation). Throttle, fire (the Enhanced Input fire action), lock-on and docking go through the player's real paths.
- A backgrounded editor can drop to about 3 fps, so commands still finish but take longer in real time. Raise the `wait` timeout rather than shortening commands.
- Ships never explode: `attack` succeeds when the target `IsWrecked()`.
- To add a command, extend `StartCurrent`/`TickCurrent` in ClaudeDriverSubsystem.cpp and the table above.
