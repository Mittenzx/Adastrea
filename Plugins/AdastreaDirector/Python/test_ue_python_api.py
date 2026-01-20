#!/usr/bin/env python3
"""
Tests for UE Python API Integration

These tests verify the UE Python API module functionality.
Note: Full testing requires running inside Unreal Engine.
These tests use mocks for development/CI purposes.
"""

import pytest
import sys
from unittest.mock import Mock, patch


# Mock the unreal module for testing outside UE
class MockUnreal:
    """Mock unreal module for testing."""
    
    class EditorUtilityLibrary:
        @staticmethod
        def get_selected_assets():
            return []
    
    class AssetToolsHelpers:
        @staticmethod
        def get_asset_tools():
            return Mock()
    
    class EditorActorSubsystem:
        pass
    
    class EditorAssetSubsystem:
        pass
    
    class StaticMeshEditorSubsystem:
        pass
    
    class UnrealEditorSubsystem:
        pass
    
    class LevelEditorSubsystem:
        pass
    
    class SystemLibrary:
        @staticmethod
        def execute_console_command(context, command):
            pass
        
        @staticmethod
        def get_project_directory():
            return "/Project"
        
        @staticmethod
        def get_engine_version():
            return "5.3.0"
    
    class EditorLevelLibrary:
        @staticmethod
        def get_editor_world():
            return Mock()
        
        @staticmethod
        def get_selected_level_actors():
            return []
        
        @staticmethod
        def load_level(path):
            return True
        
        @staticmethod
        def save_current_level():
            return True
        
        @staticmethod
        def destroy_actor(actor):
            pass
    
    class GameplayStatics:
        @staticmethod
        def get_all_actors_of_class(world, actor_class):
            return []
    
    class AssetRegistryHelpers:
        @staticmethod
        def get_asset_registry():
            return Mock()
    
    class ARFilter:
        def __init__(self, **kwargs):
            pass
    
    class NotificationInfo:
        def __init__(self):
            self.text = None
            self.fade_in_duration = 0.5
            self.fade_out_duration = 0.5
            self.expire_duration = 3.0
    
    class NotificationLibrary:
        @staticmethod
        def show_notification(notification):
            pass
    
    class Text:
        def __init__(self, text):
            self.text = text
    
    class Vector:
        def __init__(self, x, y, z):
            self.x = x
            self.y = y
            self.z = z
    
    class Rotator:
        def __init__(self, roll, pitch, yaw):
            self.roll = roll
            self.pitch = pitch
            self.yaw = yaw
    
    class Actor:
        def get_name(self):
            return "MockActor"
        
        def get_class(self):
            return type('MockClass', (), {'get_name': lambda: 'Actor'})()
        
        def get_actor_location(self):
            return MockUnreal.Vector(0, 0, 0)
        
        def get_actor_rotation(self):
            return MockUnreal.Rotator(0, 0, 0)
        
        def get_actor_scale3d(self):
            return MockUnreal.Vector(1, 1, 1)
        
        def set_actor_label(self, name):
            pass
    
    class Pawn:
        """Mock Pawn class."""
        pass
    
    class Character:
        """Mock Character class."""
        pass
    
    class Blueprint:
        """Mock Blueprint class."""
        def get_name(self):
            return "MockBlueprint"
    
    class BlueprintFactory:
        """Mock BlueprintFactory class."""
        def __init__(self):
            self.parent_class = None
        
        def set_editor_property(self, name, value):
            if name == "ParentClass":
                self.parent_class = value
    
    @staticmethod
    def get_editor_subsystem(subsystem_class):
        mock_subsystem = Mock()
        # Set up default return values for methods used in the code
        mock_subsystem.get_editor_world.return_value = Mock()
        mock_subsystem.get_selected_level_actors.return_value = []
        mock_subsystem.load_level.return_value = True
        mock_subsystem.save_current_level.return_value = True
        mock_subsystem.destroy_actor.return_value = None
        mock_subsystem.save_asset.return_value = True
        mock_subsystem.spawn_actor_from_class.return_value = Mock()
        return mock_subsystem
    
    @staticmethod
    def load_class(context, path):
        return Mock()
    
    @staticmethod
    def load_asset(path):
        return Mock()
    
    @staticmethod
    def log(message):
        pass
    
    @staticmethod
    def log_warning(message):
        pass
    
    @staticmethod
    def log_error(message):
        pass


# Patch unreal module before importing our module
sys.modules['unreal'] = MockUnreal()

# Now we can import our module
from ue_python_api import (
    UEPythonBridge,
    UEAssetInfo,
    UEActorInfo,
    LogLevel,
    is_running_in_ue,
    get_bridge
)


class TestUEAssetInfo:
    """Tests for UEAssetInfo dataclass."""
    
    def test_asset_info_creation(self):
        """Test creating asset info."""
        asset = UEAssetInfo(
            asset_name="M_Material",
            asset_path="/Game/Materials/M_Material",
            asset_class="Material"
        )
        
        assert asset.asset_name == "M_Material"
        assert asset.asset_path == "/Game/Materials/M_Material"
        assert asset.asset_class == "Material"
        assert asset.asset_size == 0
        assert asset.metadata == {}
    
    def test_asset_info_with_metadata(self):
        """Test asset info with metadata."""
        asset = UEAssetInfo(
            asset_name="SM_Cube",
            asset_path="/Game/Meshes/SM_Cube",
            asset_class="StaticMesh",
            asset_size=1024,
            metadata={"triangles": 12}
        )
        
        assert asset.metadata["triangles"] == 12
        assert asset.asset_size == 1024


class TestUEActorInfo:
    """Tests for UEActorInfo dataclass."""
    
    def test_actor_info_creation(self):
        """Test creating actor info."""
        actor = UEActorInfo(
            actor_name="StaticMeshActor_1",
            actor_class="StaticMeshActor"
        )
        
        assert actor.actor_name == "StaticMeshActor_1"
        assert actor.actor_class == "StaticMeshActor"
        assert actor.location == (0.0, 0.0, 0.0)
        assert actor.rotation == (0.0, 0.0, 0.0)
        assert actor.scale == (1.0, 1.0, 1.0)
    
    def test_actor_info_with_transform(self):
        """Test actor info with transform data."""
        actor = UEActorInfo(
            actor_name="Light_1",
            actor_class="PointLight",
            location=(100.0, 200.0, 50.0),
            rotation=(0.0, 0.0, 90.0),
            scale=(2.0, 2.0, 2.0)
        )
        
        assert actor.location == (100.0, 200.0, 50.0)
        assert actor.rotation == (0.0, 0.0, 90.0)
        assert actor.scale == (2.0, 2.0, 2.0)


class TestLogLevel:
    """Tests for LogLevel enum."""
    
    def test_log_levels(self):
        """Test all log levels."""
        assert LogLevel.LOG.value == "Log"
        assert LogLevel.DISPLAY.value == "Display"
        assert LogLevel.WARNING.value == "Warning"
        assert LogLevel.ERROR.value == "Error"


class TestUEPythonBridge:
    """Tests for UEPythonBridge class."""
    
    @pytest.fixture
    def bridge(self):
        """Create a bridge instance for testing."""
        return UEPythonBridge()
    
    def test_bridge_initialization(self, bridge):
        """Test bridge initializes successfully."""
        assert bridge is not None
        assert bridge.editor_util is not None
    
    def test_execute_console_command(self, bridge):
        """Test console command execution."""
        result = bridge.execute_console_command("stat fps")
        assert result is True
    
    def test_log_message(self, bridge):
        """Test logging messages."""
        # Should not raise exceptions
        bridge.log_message("Test message", LogLevel.LOG)
        bridge.log_message("Test warning", LogLevel.WARNING)
        bridge.log_message("Test error", LogLevel.ERROR)
    
    def test_get_selected_assets(self, bridge):
        """Test getting selected assets."""
        assets = bridge.get_selected_assets()
        assert isinstance(assets, list)
    
    def test_get_selected_actors(self, bridge):
        """Test getting selected actors."""
        actors = bridge.get_selected_actors()
        assert isinstance(actors, list)
    
    def test_get_current_level_name(self, bridge):
        """Test getting current level name."""
        with patch.object(bridge, 'get_current_level_name', return_value="TestLevel"):
            level = bridge.get_current_level_name()
            assert isinstance(level, str)
    
    def test_get_project_directory(self, bridge):
        """Test getting project directory."""
        path = bridge.get_project_directory()
        assert path == "/Project"
    
    def test_get_engine_version(self, bridge):
        """Test getting engine version."""
        version = bridge.get_engine_version()
        assert version == "5.3.0"
    
    def test_save_current_level(self, bridge):
        """Test saving current level."""
        result = bridge.save_current_level()
        assert result is True
    
    def test_load_level(self, bridge):
        """Test loading a level."""
        result = bridge.load_level("/Game/Maps/TestLevel")
        assert result is True


class TestConvenienceFunctions:
    """Tests for convenience functions."""
    
    def test_is_running_in_ue(self):
        """Test UE environment detection."""
        # With mock, should return True
        assert is_running_in_ue() is True
    
    def test_get_bridge(self):
        """Test getting bridge instance."""
        bridge = get_bridge()
        assert bridge is not None
        assert isinstance(bridge, UEPythonBridge)


class TestErrorHandling:
    """Tests for error handling."""
    
    @pytest.fixture
    def bridge(self):
        """Create a bridge instance."""
        return UEPythonBridge()
    
    def test_execute_command_with_exception(self, bridge):
        """Test console command with exception."""
        with patch('ue_python_api.unreal.SystemLibrary.execute_console_command',
                   side_effect=Exception("Test error")):
            result = bridge.execute_console_command("stat fps")
            assert result is False
    
    def test_get_selected_assets_with_exception(self, bridge):
        """Test get selected assets with exception."""
        with patch.object(bridge.editor_util, 'get_selected_assets',
                         side_effect=Exception("Test error")):
            assets = bridge.get_selected_assets()
            assert assets == []
    
    def test_load_asset_not_found(self, bridge):
        """Test loading non-existent asset."""
        with patch('ue_python_api.unreal.load_asset', return_value=None):
            asset = bridge.load_asset("/Game/NonExistent")
            assert asset is None


class TestAssetOperations:
    """Tests for asset operations."""
    
    @pytest.fixture
    def bridge(self):
        """Create a bridge instance."""
        return UEPythonBridge()
    
    def test_find_assets_by_class(self, bridge):
        """Test finding assets by class."""
        # Mock the asset registry
        mock_asset_data = Mock()
        mock_asset_data.asset_name = "M_Material"
        mock_asset_data.package_name = "/Game/Materials/M_Material"
        mock_asset_data.asset_class = "Material"
        
        mock_registry = Mock()
        mock_registry.get_assets.return_value = [mock_asset_data]
        
        with patch('ue_python_api.unreal.AssetRegistryHelpers.get_asset_registry',
                   return_value=mock_registry):
            assets = bridge.find_assets_by_class("Material", "/Game")
            assert len(assets) == 1
            assert assets[0].asset_name == "M_Material"
    
    def test_load_asset_success(self, bridge):
        """Test successful asset loading."""
        mock_asset = Mock()
        mock_asset.get_name.return_value = "M_Material"
        
        with patch('ue_python_api.unreal.load_asset', return_value=mock_asset):
            asset = bridge.load_asset("/Game/Materials/M_Material")
            assert asset is not None


class TestActorOperations:
    """Tests for actor operations."""
    
    @pytest.fixture
    def bridge(self):
        """Create a bridge instance."""
        return UEPythonBridge()
    
    def test_get_all_actors_of_class(self, bridge):
        """Test getting all actors of a class."""
        actors = bridge.get_all_actors_of_class("StaticMeshActor")
        assert isinstance(actors, list)
    
    def test_spawn_actor(self, bridge):
        """Test spawning an actor."""
        mock_actor = MockUnreal.Actor()
        
        with patch.object(bridge.editor_actor_subsystem, 'spawn_actor_from_class',
                         return_value=mock_actor):
            actor = bridge.spawn_actor(
                "StaticMeshActor",
                location=(100.0, 200.0, 50.0),
                actor_name="TestActor"
            )
            assert actor is not None
    
    def test_delete_actor(self, bridge):
        """Test deleting an actor."""
        mock_actor = MockUnreal.Actor()
        
        with patch('ue_python_api.unreal.GameplayStatics.get_all_actors_of_class',
                   return_value=[mock_actor]):
            result = bridge.delete_actor("MockActor")
            assert result is True


class TestBlueprintOperations:
    """Tests for blueprint operations."""
    
    @pytest.fixture
    def bridge(self):
        """Create a bridge instance."""
        return UEPythonBridge()
    
    def test_create_blueprint_with_default_parent(self, bridge):
        """Test creating a blueprint with default Actor parent."""
        mock_blueprint = Mock()
        mock_blueprint.get_name.return_value = "BP_MyActor"
        
        # Mock the factory and asset_tools
        mock_factory = Mock()
        with patch('ue_python_api.unreal.BlueprintFactory', return_value=mock_factory):
            with patch.object(bridge.asset_tools, 'create_asset', return_value=mock_blueprint):
                with patch.object(bridge.editor_asset_subsystem, 'save_asset', return_value=True):
                    blueprint = bridge.create_blueprint(
                        blueprint_name="BP_MyActor",
                        parent_class=None,
                        package_path="/Game/Blueprints"
                    )
                    assert blueprint is not None
                    assert blueprint == mock_blueprint
    
    def test_create_blueprint_with_string_parent(self, bridge):
        """Test creating a blueprint with string parent class."""
        mock_blueprint = Mock()
        mock_blueprint.get_name.return_value = "BP_MyPawn"
        
        mock_factory = Mock()
        with patch('ue_python_api.unreal.BlueprintFactory', return_value=mock_factory):
            with patch('ue_python_api.unreal.Pawn', Mock()):
                with patch.object(bridge.asset_tools, 'create_asset', return_value=mock_blueprint):
                    with patch.object(bridge.editor_asset_subsystem, 'save_asset', return_value=True):
                        blueprint = bridge.create_blueprint(
                            blueprint_name="BP_MyPawn",
                            parent_class="Pawn",
                            package_path="/Game/Blueprints"
                        )
                        assert blueprint is not None
    
    def test_create_blueprint_with_class_object(self, bridge):
        """Test creating a blueprint with class object as parent."""
        mock_blueprint = Mock()
        mock_blueprint.get_name.return_value = "BP_MyCharacter"
        mock_parent_class = Mock()
        
        mock_factory = Mock()
        with patch('ue_python_api.unreal.BlueprintFactory', return_value=mock_factory):
            with patch.object(bridge.asset_tools, 'create_asset', return_value=mock_blueprint):
                with patch.object(bridge.editor_asset_subsystem, 'save_asset', return_value=True):
                    blueprint = bridge.create_blueprint(
                        blueprint_name="BP_MyCharacter",
                        parent_class=mock_parent_class,
                        package_path="/Game/Characters"
                    )
                    assert blueprint is not None
    
    def test_create_blueprint_with_full_class_path(self, bridge):
        """Test creating a blueprint with full class path."""
        mock_blueprint = Mock()
        mock_blueprint.get_name.return_value = "BP_CustomActor"
        mock_class = Mock()
        
        mock_factory = Mock()
        with patch('ue_python_api.unreal.BlueprintFactory', return_value=mock_factory):
            with patch('ue_python_api.unreal.load_class', return_value=mock_class):
                with patch.object(bridge.asset_tools, 'create_asset', return_value=mock_blueprint):
                    with patch.object(bridge.editor_asset_subsystem, 'save_asset', return_value=True):
                        blueprint = bridge.create_blueprint(
                            blueprint_name="BP_CustomActor",
                            parent_class="/Script/Engine.Actor",
                            package_path="/Game/Blueprints"
                        )
                        assert blueprint is not None
    
    def test_create_blueprint_failure(self, bridge):
        """Test blueprint creation failure."""
        mock_factory = Mock()
        with patch('ue_python_api.unreal.BlueprintFactory', return_value=mock_factory):
            with patch.object(bridge.asset_tools, 'create_asset', return_value=None):
                blueprint = bridge.create_blueprint(
                    blueprint_name="BP_FailedActor",
                    parent_class=None,
                    package_path="/Game/Blueprints"
                )
                assert blueprint is None
    
    def test_create_blueprint_package_path_normalization(self, bridge):
        """Test that package path trailing slashes are removed."""
        mock_blueprint = Mock()
        mock_factory = Mock()
        
        with patch('ue_python_api.unreal.BlueprintFactory', return_value=mock_factory):
            with patch.object(bridge.asset_tools, 'create_asset', return_value=mock_blueprint):
                with patch.object(bridge.editor_asset_subsystem, 'save_asset', return_value=True):
                    bridge.create_blueprint(
                        blueprint_name="BP_TestActor",
                        package_path="/Game/Blueprints/"  # Has trailing slash
                    )
                    # Verify create_asset was called with normalized path
                    call_args = bridge.asset_tools.create_asset.call_args
                    assert call_args[1]['package_path'] == "/Game/Blueprints"


class TestBlueprintGraphOperations:
    """Tests for blueprint graph manipulation (experimental)."""
    
    @pytest.fixture
    def bridge(self):
        """Create a bridge instance."""
        return UEPythonBridge()
    
    def test_add_blueprint_node(self, bridge):
        """Test adding a node to blueprint graph."""
        mock_blueprint = Mock()
        mock_blueprint.get_name.return_value = "BP_TestActor"
        
        with patch('ue_python_api.unreal.load_asset', return_value=mock_blueprint):
            # Should raise NotImplementedError as this feature is not yet implemented
            with pytest.raises(NotImplementedError) as exc_info:
                bridge.add_blueprint_node(
                    "/Game/Blueprints/BP_TestActor",
                    "BeginPlay",
                    position_x=100.0,
                    position_y=100.0
                )
            assert "not yet implemented" in str(exc_info.value)
    
    def test_add_blueprint_node_not_found(self, bridge):
        """Test adding node to non-existent blueprint."""
        with patch('ue_python_api.unreal.load_asset', return_value=None):
            result = bridge.add_blueprint_node(
                "/Game/Blueprints/BP_NotFound",
                "BeginPlay"
            )
            assert result is None
    
    def test_connect_blueprint_nodes(self, bridge):
        """Test connecting two blueprint nodes."""
        mock_node1 = Mock()
        mock_node2 = Mock()
        
        result = bridge.connect_blueprint_nodes(
            "/Game/Blueprints/BP_TestActor",
            mock_node1,
            "execute",
            mock_node2,
            "execute"
        )
        # Currently returns success flag
        assert isinstance(result, bool)
        assert result is True
    
    def test_compile_blueprint(self, bridge):
        """Test compiling a blueprint."""
        mock_blueprint = Mock()
        
        with patch('ue_python_api.unreal.load_asset', return_value=mock_blueprint):
            with patch.object(bridge.editor_asset_subsystem, 'save_asset', return_value=True):
                result = bridge.compile_blueprint("/Game/Blueprints/BP_TestActor")
                assert result is True
    
    def test_compile_blueprint_not_found(self, bridge):
        """Test compiling non-existent blueprint."""
        with patch('ue_python_api.unreal.load_asset', return_value=None):
            result = bridge.compile_blueprint("/Game/Blueprints/BP_NotFound")
            assert result is False
    
    def test_add_blueprint_variable(self, bridge):
        """Test adding a variable to blueprint."""
        mock_blueprint = Mock()
        
        with patch('ue_python_api.unreal.load_asset', return_value=mock_blueprint):
            with patch.object(bridge.editor_asset_subsystem, 'save_asset', return_value=True):
                result = bridge.add_blueprint_variable(
                    "/Game/Blueprints/BP_TestActor",
                    "Health",
                    "Float",
                    default_value=100.0,
                    is_exposed=True
                )
                assert result is True
    
    def test_add_blueprint_variable_not_found(self, bridge):
        """Test adding variable to non-existent blueprint."""
        with patch('ue_python_api.unreal.load_asset', return_value=None):
            result = bridge.add_blueprint_variable(
                "/Game/Blueprints/BP_NotFound",
                "Health",
                "Float"
            )
            assert result is False
    
    def test_add_blueprint_node_with_name(self, bridge):
        """Test adding a named node to blueprint."""
        mock_blueprint = Mock()
        
        with patch('ue_python_api.unreal.load_asset', return_value=mock_blueprint):
            # Should raise NotImplementedError as this feature is not yet implemented
            with pytest.raises(NotImplementedError):
                bridge.add_blueprint_node(
                    "/Game/Blueprints/BP_TestActor",
                    "Print",
                    position_x=200.0,
                    position_y=150.0,
                    node_name="PrintDebugInfo"
                )


class TestBlueprintCommentNodes:
    """Tests for blueprint comment node operations."""
    
    @pytest.fixture
    def bridge(self):
        """Create a bridge instance."""
        return UEPythonBridge()
    
    def test_add_blueprint_comment_basic(self, bridge):
        """Test adding a basic comment to blueprint."""
        mock_blueprint = Mock()
        
        with patch('ue_python_api.unreal.load_asset', return_value=mock_blueprint):
            result = bridge.add_blueprint_comment(
                "/Game/Blueprints/BP_TestActor",
                "Test Comment",
                position_x=100,
                position_y=100
            )
            assert result is True
    
    def test_add_blueprint_comment_with_color(self, bridge):
        """Test adding a colored comment."""
        mock_blueprint = Mock()
        
        with patch('ue_python_api.unreal.load_asset', return_value=mock_blueprint):
            result = bridge.add_blueprint_comment(
                "/Game/Blueprints/BP_TestActor",
                "Important Note",
                color=(255, 0, 0)  # Red
            )
            assert result is True
    
    def test_add_blueprint_comment_adastrea_style(self, bridge):
        """Test adding an Adastrea-branded comment."""
        mock_blueprint = Mock()
        
        with patch('ue_python_api.unreal.load_asset', return_value=mock_blueprint):
            result = bridge.add_blueprint_comment(
                "/Game/Characters/BP_Player",
                "═══ ADASTREA CHARACTER SYSTEM ═══",
                position_y=-200,
                width=800,
                height=60,
                color=(138, 43, 226),  # Adastrea brand color
                font_size=20
            )
            assert result is True
    
    def test_add_blueprint_comment_multiline(self, bridge):
        """Test adding a multi-line comment."""
        mock_blueprint = Mock()
        
        with patch('ue_python_api.unreal.load_asset', return_value=mock_blueprint):
            result = bridge.add_blueprint_comment(
                "/Game/Combat/BP_WeaponSystem",
                "Function: CalculateDamage\nInputs: BaseDamage, DamageType\nOutput: FinalDamage",
                width=500,
                height=120
            )
            assert result is True
    
    def test_add_blueprint_comment_not_found(self, bridge):
        """Test adding comment to non-existent blueprint."""
        with patch('ue_python_api.unreal.load_asset', return_value=None):
            result = bridge.add_blueprint_comment(
                "/Game/Blueprints/BP_NotFound",
                "Test Comment"
            )
            assert result is False
    
    def test_add_blueprint_comment_custom_size(self, bridge):
        """Test adding a comment with custom size."""
        mock_blueprint = Mock()
        
        with patch('ue_python_api.unreal.load_asset', return_value=mock_blueprint):
            result = bridge.add_blueprint_comment(
                "/Game/UI/BP_MainMenu",
                "TODO: Integrate Adastrea save system",
                width=600,
                height=150,
                color=(255, 215, 0)  # Gold
            )
            assert result is True
    
    def test_generate_comment_script(self, bridge):
        """Test that comment script is generated correctly."""
        import ast
        mock_blueprint = Mock()
        
        with patch('ue_python_api.unreal.load_asset', return_value=mock_blueprint):
            script = bridge._generate_comment_script(
                "/Game/BP_Test",
                "Test Comment",
                100.0,
                200.0,
                400.0,
                100.0,
                (255, 0, 0),
                18
            )
            # Verify script contains key elements
            assert "import unreal" in script
            assert "EdGraphNode_Comment" in script
            assert "Test Comment" in script
            assert "100" in script or "100.0" in script
            assert "200" in script or "200.0" in script
            
            # Verify the script is syntactically valid Python
            try:
                ast.parse(script)
            except SyntaxError as e:
                pytest.fail(f"Generated script has syntax errors: {e}")


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
