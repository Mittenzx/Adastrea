# Packaging Adastrea (Win64)

## TL;DR

```bat
REM Close every UnrealEditor on this project first (the cook uses UnrealEditor-Cmd).
Tools\package_win64.bat              REM Development (default)
Tools\package_win64.bat Shipping     REM or DebugGame / Shipping
```

- Output: `Saved\Packaged\Windows\Adastrea.exe`
- Log: `Saved\Logs\Package_Win64_<Config>.log`. UAT also writes its per-step logs
  (UBT, cook) to `%UE_ROOT%\Engine\Programs\AutomationTool\Saved\Logs`.
- Overrides: `UE_ROOT` (default `C:\Program Files\Epic Games\UE_5.8`), `ARCHIVE_DIR`.
  Any extra arguments are passed straight to UAT, e.g. `Tools\package_win64.bat Development -iterate`.

The script is a thin wrapper around:

```bat
RunUAT.bat BuildCookRun -project=<repo>\Adastrea.uproject -noP4 -platform=Win64 ^
  -clientconfig=Development -build -cook -stage -pak -archive ^
  -archivedirectory=<repo>\Saved\Packaged -unattended -utf8output
```

## What gets cooked

Configured in `Config/DefaultGame.ini` under `[/Script/UnrealEd.ProjectPackagingSettings]`:

- **Maps**: `TestLevel` (the entry map) and `MiningTest`. To ship another level, add a
  `+MapsToCook=(FilePath="/Game/Maps/<Name>")` line.
- **Always-cooked folders**: C++ loads a number of assets by string path at runtime
  (`LoadObject`, `StaticLoadClass` or soft paths built from strings). Examples are the
  ship-select roster in `AdastreaHUD.cpp`, hull materials in `Spaceship.cpp`, the station
  editor widget and module catalog, interior meshes and materials, and the mining laser
  VFX. The cooker can't follow those references, so their folders are listed in
  `DirectoriesToAlwaysCook`. **If you add a new `LoadObject(TEXT("/Game/..."))`, make
  sure its folder is covered, or the packaged game gets a null asset.**
- **Raw JSON**: `Content/Data/*.json` (crafting tree, station module catalog, example
  station layout) is read with `FFileHelper` from `FPaths::ProjectContentDir()`, so it is
  staged into the pak through `DirectoriesToAlwaysStageAsUFS=(Path="Data")`.
- There is no "cook everything". `Content/Fab` and `Content/Textures` only ship the
  assets that something references.

## Entry flow

`GameDefaultMap=/Game/Maps/TestLevel.TestLevel` is set in `Config/DefaultEngine.ini`.
`GameMapsSettings` is a config=Engine class, so the old copy in DefaultGame.ini (which
pointed at a `/Game/Maps/MainMenu` map that never existed) was ignored and has been
removed. There is no front-end menu map. The "main menu" is an overlay toggled by
`AAdastreaPlayerController::ToggleMainMenu`. TestLevel's World Settings override the
game mode to `BP_SpaceGameMode`.

## Plugins

`ModelContextProtocol` (Unreal MCP, NoRedist, has Runtime modules) and `AllToolsets` are
denied for Game/Client/Server targets in `Adastrea.uproject` through `TargetDenyList`.
They stay enabled in the editor, and the packaged game doesn't start an MCP server.

## Status (first run, 2026-09-26)

A Win64 Development BuildCookRun **succeeds**: the build, cook, stage, pak (IoStore) and
archive steps take about 2 minutes on a warm DDC, and the output is about 1.6 GB. The
packaged `Adastrea.exe` boots straight into TestLevel with `BP_SpaceGameMode`. It spawns
and possesses the player ship, loads the crafting-tree JSON from the pak, populates the
markets, and the AI miners mine. The HUD renders.

Fixed on the way (Game-target compile blockers; the editor build hid both):
- `AActor::GetActorLabel()` is WITH_EDITOR-only. `AdastreaHUD.cpp` now uses
  `HudActorName()` (the label in the editor, `GetName()` in packaged builds). **Don't call
  GetActorLabel() in runtime modules.**
- `AdastreaFunctionLibrary.cpp` switched on the forward-declared `EDamageType`. It now
  includes `Interfaces/IDamageable.h`. The editor build only compiled because of the shared PCH.

## Known issues / remaining blockers

1. **Orphaned DataAssets**: `DA_Weapon_*`, `DA_WeaponTemplates`, `DA_Quest_*`,
   `DA_WeaponVFX_*`, `DA_ImpactEffects_Standard` and `DA_Council_CoreSystems` have classes
   that were removed from C++, so the cooker warns "class ... does not exist". Their
   folders are in `DirectoriesToNeverCook`. `DA_Council_CoreSystems` still warns because it
   shares `/Game/DataAssets/Sectors` with live assets. Fix: delete these assets in the
   editor (Content Browser, with reference check).
2. **Station and ship names in the packaged HUD** show object names (e.g.
   `BP_SpaceStation_C_1`), because actor labels don't exist at runtime. The HUD should
   read `ASpaceStation::StationName` (currently protected, so it needs a public getter)
   and `ASpaceship::GetShipName()`.
3. **Unverified visuals**: the only screenshot was taken on frame 0 (`-ExecCmds=HighResShot`)
   and shows the HUD over a black scene. Confirm that the stations, star dome and asteroids
   render by running the exe interactively.
4. **Performance**: the frame counter averaged about 9 fps over the first 85 s, with the
   window unfocused and PSO caches cold. Profile before drawing conclusions (`stat unit`,
   and a bundled PSO cache).
5. **Live Coding mutex**: if any UnrealEditor is open (on *any* checkout), building the
   Editor target fails with "Unable to build while Live Coding is active". The mutex is
   keyed on the engine's UnrealEditor.exe path. When that editor has a different
   project/checkout open, pass `-ubtargs=-NoHotReloadFromIDE` (the script accepts extra
   arguments). Otherwise close it.
6. The `Mac`/`Linux` entries in `TargetPlatforms` are untested. Only Win64 has been packaged.
