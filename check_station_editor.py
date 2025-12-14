import unreal

def check_station_editor_components():
    print("=== Station Editor Component Check ===")

    # Check WBP_StationEditor parent class
    widget_path = '/Game/UI/Stations/WBP_StationEditor'
    widget = unreal.load_asset(widget_path)
    if widget:
        parent_class = widget.get_class().get_super_class()
        parent_name = parent_class.get_name() if parent_class else "None"
        print(f"WBP_StationEditor parent class: {parent_name}")
        expected_parent = "StationEditorWidgetCpp"
        if parent_name == expected_parent:
            print("✓ Correct parent class")
        else:
            print(f"✗ Expected {expected_parent}, got {parent_name}")
    else:
        print("✗ WBP_StationEditor not found")

    # Check WBP_ModuleListItem parent class
    item_path = '/Game/UI/Stations/WBP_ModuleListItem'
    item_widget = unreal.load_asset(item_path)
    if item_widget:
        parent_class = item_widget.get_class().get_super_class()
        parent_name = parent_class.get_name() if parent_class else "None"
        print(f"WBP_ModuleListItem parent class: {parent_name}")
        expected_parent = "ModuleListItemWidget"
        if parent_name == expected_parent:
            print("✓ Correct parent class")
        else:
            print(f"✗ Expected {expected_parent}, got {parent_name}")
    else:
        print("✗ WBP_ModuleListItem not found")

    # Check WBP_ConstructionQueueItem parent class
    queue_path = '/Game/UI/Stations/WBP_ConstructionQueueItem'
    queue_widget = unreal.load_asset(queue_path)
    if queue_widget:
        parent_class = queue_widget.get_class().get_super_class()
        parent_name = parent_class.get_name() if parent_class else "None"
        print(f"WBP_ConstructionQueueItem parent class: {parent_name}")
        expected_parent = "ConstructionQueueItemWidget"
        if parent_name == expected_parent:
            print("✓ Correct parent class")
        else:
            print(f"✗ Expected {expected_parent}, got {parent_name}")
    else:
        print("✗ WBP_ConstructionQueueItem not found")

    # Check DA_StationModuleCatalog
    catalog_path = '/Game/DataAssets/Stations/DA_StationModuleCatalog'
    catalog = unreal.load_asset(catalog_path)
    if catalog:
        print("✓ DA_StationModuleCatalog found")
        # Try to get module count if possible
        try:
            modules = getattr(catalog, 'Modules', None)
            if modules:
                module_count = len(modules)
                print(f"Module count: {module_count}")
                if module_count > 0:
                    print("✓ Has modules")
                else:
                    print("✗ No modules defined")
            else:
                print("? Could not access Modules property")
        except:
            print("? Could not check module count")
    else:
        print("✗ DA_StationModuleCatalog not found")

if __name__ == "__main__":
    check_station_editor_components()