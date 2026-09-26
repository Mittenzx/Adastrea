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

## Known blockers / status

See the section below, which the last packaging run updates.
