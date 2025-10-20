# Mod Manager UI (WBP_ModManager) — Quick instructions

This file is the Mod Manager UMG implementation spec. Follow these steps:

1. Ensure your PlayerModComponent exposes these Blueprint-callable helpers:
   - IsModInstalledByID(FName ModID) -> bool
   - GetModCooldownPercent(FName ModID) -> float (0..1)
   - GetModRemainingTime(FName ModID) -> float (seconds)

2. Create data assets for mods under Content/Data/Mods/ (Right-click → Misc → Data Asset → PlayerModDataAsset).

3. Create the widget WBP_ModListItem:
   - Variables: ModAsset (UPlayerModDataAsset), ParentManager (WBP_ModManager*)
   - Visuals: Icon, Name, Slot, Installed badge, small cooldown bar
   - OnClicked -> call ParentManager.SelectMod(ModAsset)

4. Create the main widget WBP_ModManager:
   - Variables: ModsDataSource (Array of PlayerModDataAsset), SelectedMod, OwnerCharacter
   - UI: Left scroll box (populate with WBP_ModListItem), Right detail panel (Icon, Name, Description, Stat chips, Install/Activate/Uninstall buttons, Cooldown progress)
   - Implement PopulateModList() to create list items and SelectMod() to populate details.

5. Hook buttons to PlayerModComponent methods:
   - InstallButton -> PlayerModComponent.InstallMod(SelectedMod)
   - ActivateButton -> PlayerModComponent.ActivateMod(SelectedMod.ModDefinition.ModID)
   - UninstallButton -> PlayerModComponent.UninstallMod(SelectedMod.ModDefinition.ModID)

6. Update cooldown UI every 0.25s (Timer) using GetModCooldownPercent/GetModRemainingTime.

7. Open the widget from your HUD or keybind:
   - CreateWidget(WBP_ModManager) -> Set OwnerCharacter -> AddToViewport()

8. Test in PIE with server-authoritative flow and verify replication.

Keep this file in the repo so designers and programmers can reference the implementation plan.