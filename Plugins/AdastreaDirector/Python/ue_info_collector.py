#!/usr/bin/env python3
"""
Unreal Engine Comprehensive Information Collector

This script collects extensive information about a game project from inside
the Unreal Engine editor. It provides agents with detailed context about:
- Project configuration and metadata
- Assets (counts, types, sizes, locations)
- Actors and level content
- Blueprints (complexity, structure, dependencies)
- Materials and textures
- Performance metrics
- Editor settings and plugins
- Build configuration
- Source code structure

USAGE:
    This script must be run from within Unreal Engine's Python environment.
    
    In UE Editor:
    1. Enable Python Editor Script Plugin
    2. Window > Developer Tools > Python Console
    3. Run: execfile("Plugins/AdastreaDirector/Python/ue_info_collector.py")
    
    Or programmatically:
    import ue_info_collector
    info = ue_info_collector.collect_all_info()
    ue_info_collector.print_report(info)
    ue_info_collector.save_to_json(info, "ue_project_info.json")
"""

import os
import json
from typing import Dict, Any, List
from datetime import datetime
from collections import defaultdict

# Check if running in Unreal Engine
try:
    import unreal
    UNREAL_AVAILABLE = True
except ImportError:
    UNREAL_AVAILABLE = False
    print("WARNING: Not running inside Unreal Engine. Some features will be unavailable.")
    

class UEInfoCollector:
    """Collects comprehensive information about the UE project."""
    
    def __init__(self):
        """Initialize the collector."""
        if not UNREAL_AVAILABLE:
            print("⚠ WARNING: Unreal Python API not available")
            print("This script must be run inside Unreal Engine's Python environment")
            self.available = False
            return
        
        self.available = True
        self._init_subsystems()
    
    def _init_subsystems(self):
        """Initialize UE subsystems."""
        try:
            self.asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
            self.editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
            self.editor_asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
            self.unreal_editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
            self.level_editor_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
            print("✓ UE subsystems initialized")
        except Exception as e:
            print(f"✗ Failed to initialize subsystems: {e}")
            self.available = False
    
    # ============================================================================
    # Project Information
    # ============================================================================
    
    def collect_project_info(self) -> Dict[str, Any]:
        """Collect basic project information."""
        if not self.available:
            return {"error": "UE not available"}
        
        info = {
            "collection_time": datetime.now().isoformat(),
            "engine_version": {},
            "project": {},
            "paths": {}
        }
        
        try:
            # Engine version
            info["engine_version"] = {
                "major": unreal.SystemLibrary.get_engine_version(),
                "full": str(unreal.SystemLibrary.get_engine_version())
            }
            
            # Project directory
            project_dir = unreal.SystemLibrary.get_project_directory()
            info["paths"]["project_directory"] = project_dir
            info["paths"]["project_content_directory"] = unreal.SystemLibrary.get_project_content_directory()
            info["paths"]["project_saved_directory"] = unreal.SystemLibrary.get_project_saved_directory()
            
            # Current level
            world = self.unreal_editor_subsystem.get_editor_world()
            if world:
                info["project"]["current_level"] = world.get_name()
                info["project"]["current_level_path"] = world.get_path_name()
            
            # Project name (extract from path)
            info["project"]["name"] = os.path.basename(project_dir.rstrip('/\\'))
            
            print("✓ Collected project info")
            
        except Exception as e:
            print(f"✗ Error collecting project info: {e}")
            info["error"] = str(e)
        
        return info
    
    # ============================================================================
    # Asset Information
    # ============================================================================
    
    def collect_asset_info(self) -> Dict[str, Any]:
        """Collect comprehensive asset information."""
        if not self.available:
            return {"error": "UE not available"}
        
        info = {
            "total_assets": 0,
            "by_type": {},
            "by_path": {},
            "largest_assets": [],
            "recently_modified": [],
            "naming_conventions": {}
        }
        
        try:
            # Get all assets
            all_assets = self.asset_registry.get_all_assets()
            info["total_assets"] = len(all_assets)
            
            asset_details = []
            type_counts = defaultdict(int)
            path_counts = defaultdict(int)
            
            for asset_data in all_assets:
                asset_class = str(asset_data.asset_class_path.asset_name)
                asset_path = str(asset_data.package_name)
                asset_name = str(asset_data.asset_name)
                
                # Count by type
                type_counts[asset_class] += 1
                
                # Count by top-level path
                path_parts = asset_path.split('/')
                if len(path_parts) > 1:
                    top_path = f"/{path_parts[1]}" if path_parts[1] else "/Game"
                    path_counts[top_path] += 1
                
                # Collect asset details
                asset_details.append({
                    "name": asset_name,
                    "class": asset_class,
                    "path": asset_path
                })
            
            # Sort and store by type
            info["by_type"] = dict(sorted(type_counts.items(), key=lambda x: x[1], reverse=True))
            info["by_path"] = dict(sorted(path_counts.items(), key=lambda x: x[1], reverse=True))
            
            # Analyze naming conventions
            info["naming_conventions"] = self._analyze_naming_conventions(asset_details)
            
            print(f"✓ Collected info on {info['total_assets']} assets")
            
        except Exception as e:
            print(f"✗ Error collecting asset info: {e}")
            info["error"] = str(e)
        
        return info
    
    def _analyze_naming_conventions(self, assets: List[Dict]) -> Dict[str, Any]:
        """
        Analyze asset naming conventions.
        
        Uses standard Unreal Engine naming conventions:
        - Prefixes: BP_ (Blueprint), M_ (Material), MI_ (Material Instance), 
          T_ (Texture), SM_ (Static Mesh), SK_ (Skeletal Mesh), A_ (Animation),
          S_ (Sound), P_ (Particle), E_ (Enum), W_ (Widget)
        - Suffixes: _C (Compiled), _Inst (Instance), _Mat (Material), _Tex (Texture)
        
        These can be customized by modifying the lists below.
        """
        conventions = {
            "prefixes": defaultdict(int),
            "suffixes": defaultdict(int),
            "has_prefix": 0,
            "has_suffix": 0
        }
        
        # Standard UE naming conventions - modify these for custom conventions
        common_prefixes = ['BP_', 'M_', 'MI_', 'T_', 'SM_', 'SK_', 'A_', 'S_', 'P_', 'E_', 'W_']
        common_suffixes = ['_C', '_Inst', '_Mat', '_Tex']
        
        for asset in assets:
            name = asset["name"]
            
            # Check prefixes
            for prefix in common_prefixes:
                if name.startswith(prefix):
                    conventions["prefixes"][prefix] += 1
                    conventions["has_prefix"] += 1
                    break
            
            # Check suffixes
            for suffix in common_suffixes:
                if name.endswith(suffix):
                    conventions["suffixes"][suffix] += 1
                    conventions["has_suffix"] += 1
                    break
        
        # Convert defaultdict to regular dict for JSON serialization
        conventions["prefixes"] = dict(conventions["prefixes"])
        conventions["suffixes"] = dict(conventions["suffixes"])
        
        return conventions
    
    # ============================================================================
    # Blueprint Information
    # ============================================================================
    
    def collect_blueprint_info(self) -> Dict[str, Any]:
        """Collect detailed blueprint information."""
        if not self.available:
            return {"error": "UE not available"}
        
        info = {
            "total_blueprints": 0,
            "by_parent_class": {},
            "actor_blueprints": 0,
            "component_blueprints": 0,
            "interface_blueprints": 0,
            "function_libraries": 0,
            "widget_blueprints": 0,
            "animation_blueprints": 0,
            "largest_blueprints": [],
            "detailed_analysis": {
                "total_variables": 0,
                "total_functions": 0,
                "total_components": 0,
                "avg_variables_per_bp": 0,
                "avg_functions_per_bp": 0
            }
        }
        
        try:
            # Get all blueprint assets
            bp_filter = unreal.ARFilter(
                class_names=["Blueprint"],
                recursive_paths=True
            )
            blueprints = self.asset_registry.get_assets(bp_filter)
            info["total_blueprints"] = len(blueprints)
            
            parent_class_counts = defaultdict(int)
            bp_details = []
            total_vars = 0
            total_funcs = 0
            total_comps = 0
            analyzed_count = 0
            
            for bp_data in blueprints:
                bp_path = str(bp_data.package_name)
                bp_name = str(bp_data.asset_name)
                
                try:
                    # Try to load and analyze blueprint
                    bp = unreal.load_asset(bp_path)
                    if bp:
                        bp_detail = {
                            "name": bp_name,
                            "path": bp_path,
                            "variables": 0,
                            "functions": 0,
                            "components": 0,
                            "graphs": 0
                        }
                        
                        # Get parent class
                        bp_class = bp.get_class()
                        if bp_class:
                            parent = bp_class.get_super_class()
                            if parent:
                                parent_name = parent.get_name()
                                parent_class_counts[parent_name] += 1
                                bp_detail["parent_class"] = parent_name
                                
                                # Categorize by type
                                if 'Actor' in parent_name:
                                    info["actor_blueprints"] += 1
                                elif 'Component' in parent_name:
                                    info["component_blueprints"] += 1
                                elif 'Interface' in parent_name:
                                    info["interface_blueprints"] += 1
                                elif 'FunctionLibrary' in parent_name or 'Library' in parent_name:
                                    info["function_libraries"] += 1
                                elif 'Widget' in parent_name or 'UserWidget' in parent_name:
                                    info["widget_blueprints"] += 1
                                elif 'Anim' in parent_name:
                                    info["animation_blueprints"] += 1
                        
                        # Try to get blueprint-specific details
                        try:
                            # Get variables (member variables)
                            if hasattr(bp, 'get_editor_property'):
                                try:
                                    vars_prop = bp.get_editor_property('new_variables')
                                    if vars_prop:
                                        bp_detail["variables"] = len(vars_prop)
                                        total_vars += len(vars_prop)
                                except (AttributeError, RuntimeError):
                                    # Property may not exist for all blueprint types
                                    pass
                                
                                # Try to get function graphs
                                try:
                                    func_graphs = bp.get_editor_property('function_graphs')
                                    if func_graphs:
                                        bp_detail["functions"] = len(func_graphs)
                                        total_funcs += len(func_graphs)
                                except (AttributeError, RuntimeError):
                                    # Property may not exist for all blueprint types
                                    pass
                                
                                # Try to get event graphs
                                try:
                                    uber_graphs = bp.get_editor_property('ubergraph_pages')
                                    if uber_graphs:
                                        bp_detail["graphs"] = len(uber_graphs)
                                except (AttributeError, RuntimeError):
                                    # Property may not exist for all blueprint types
                                    pass
                                
                                # Try to get components (for Actor blueprints)
                                try:
                                    components = bp.get_editor_property('simple_construction_script')
                                    if components:
                                        # Get all nodes in construction script
                                        nodes = components.get_editor_property('all_nodes')
                                        if nodes:
                                            bp_detail["components"] = len(nodes)
                                            total_comps += len(nodes)
                                except (AttributeError, RuntimeError):
                                    # Only Actor blueprints have construction scripts
                                    pass
                            
                            analyzed_count += 1
                            bp_details.append(bp_detail)
                            
                        except Exception as e:
                            # Could not get detailed info for this blueprint
                            pass
                
                except (RuntimeError, AttributeError) as e:
                    # Skip blueprints that fail to load or have missing attributes
                    # This is expected for some blueprint types
                    pass
            
            info["by_parent_class"] = dict(sorted(parent_class_counts.items(), key=lambda x: x[1], reverse=True))
            
            # Calculate averages
            if analyzed_count > 0:
                info["detailed_analysis"]["total_variables"] = total_vars
                info["detailed_analysis"]["total_functions"] = total_funcs
                info["detailed_analysis"]["total_components"] = total_comps
                info["detailed_analysis"]["avg_variables_per_bp"] = round(total_vars / analyzed_count, 2)
                info["detailed_analysis"]["avg_functions_per_bp"] = round(total_funcs / analyzed_count, 2)
                info["detailed_analysis"]["analyzed_blueprints"] = analyzed_count
            
            # Sort and get largest blueprints (by total complexity)
            bp_details_sorted = sorted(
                bp_details,
                key=lambda x: x.get("variables", 0) + x.get("functions", 0) * 2 + x.get("components", 0),
                reverse=True
            )
            info["largest_blueprints"] = bp_details_sorted[:10]  # Top 10 most complex
            
            print(f"✓ Collected info on {info['total_blueprints']} blueprints ({analyzed_count} analyzed in detail)")
            
        except Exception as e:
            print(f"✗ Error collecting blueprint info: {e}")
            info["error"] = str(e)
        
        return info
    
    # ============================================================================
    # Deep Blueprint Analysis
    # ============================================================================
    
    def analyze_blueprint_detailed(self, blueprint_path: str) -> Dict[str, Any]:
        """
        Perform deep analysis on a specific blueprint.
        
        Args:
            blueprint_path: Full path to the blueprint asset
            
        Returns:
            Detailed analysis including variables, functions, components, and complexity metrics
        """
        if not self.available:
            return {"error": "UE not available"}
        
        analysis = {
            "path": blueprint_path,
            "name": "",
            "parent_class": "",
            "variables": [],
            "functions": [],
            "components": [],
            "graphs": [],
            "complexity_score": 0,
            "metadata": {}
        }
        
        try:
            bp = unreal.load_asset(blueprint_path)
            if not bp:
                analysis["error"] = "Failed to load blueprint"
                return analysis
            
            analysis["name"] = bp.get_name()
            
            # Get parent class
            try:
                bp_class = bp.get_class()
                if bp_class:
                    parent = bp_class.get_super_class()
                    if parent:
                        analysis["parent_class"] = parent.get_name()
            except Exception as e:
                analysis["metadata"]["parent_class_error"] = str(e)
            
            # Analyze variables
            try:
                if hasattr(bp, 'get_editor_property'):
                    vars_prop = bp.get_editor_property('new_variables')
                    if vars_prop:
                        for var in vars_prop:
                            var_info = {
                                "name": str(var.get_editor_property('var_name')) if hasattr(var, 'get_editor_property') else "unknown",
                                "type": str(var.get_editor_property('var_type')) if hasattr(var, 'get_editor_property') else "unknown"
                            }
                            analysis["variables"].append(var_info)
            except Exception as e:
                analysis["metadata"]["variables_error"] = str(e)
            
            # Analyze functions
            try:
                if hasattr(bp, 'get_editor_property'):
                    func_graphs = bp.get_editor_property('function_graphs')
                    if func_graphs:
                        for func in func_graphs:
                            func_name = func.get_name() if hasattr(func, 'get_name') else "unknown"
                            analysis["functions"].append(func_name)
            except Exception as e:
                analysis["metadata"]["functions_error"] = str(e)
            
            # Analyze components
            try:
                if hasattr(bp, 'get_editor_property'):
                    scs = bp.get_editor_property('simple_construction_script')
                    if scs:
                        nodes = scs.get_editor_property('all_nodes')
                        if nodes:
                            for node in nodes:
                                comp_class = node.get_editor_property('component_class')
                                if comp_class:
                                    analysis["components"].append(comp_class.get_name())
            except Exception as e:
                analysis["metadata"]["components_error"] = str(e)
            
            # Analyze graphs
            try:
                if hasattr(bp, 'get_editor_property'):
                    # Event graphs
                    uber_graphs = bp.get_editor_property('ubergraph_pages')
                    if uber_graphs:
                        for graph in uber_graphs:
                            graph_info = {
                                "name": graph.get_name() if hasattr(graph, 'get_name') else "unknown",
                                "type": "event"
                            }
                            analysis["graphs"].append(graph_info)
                    
                    # Function graphs
                    func_graphs = bp.get_editor_property('function_graphs')
                    if func_graphs:
                        for graph in func_graphs:
                            graph_info = {
                                "name": graph.get_name() if hasattr(graph, 'get_name') else "unknown",
                                "type": "function"
                            }
                            analysis["graphs"].append(graph_info)
            except Exception as e:
                analysis["metadata"]["graphs_error"] = str(e)
            
            # Calculate complexity score
            # Variables: 1 point each
            # Functions: 3 points each (more complex)
            # Components: 2 points each
            # Graphs: 2 points each
            analysis["complexity_score"] = (
                len(analysis["variables"]) +
                len(analysis["functions"]) * 3 +
                len(analysis["components"]) * 2 +
                len(analysis["graphs"]) * 2
            )
            
            print(f"✓ Deep analysis completed for {analysis['name']}")
            print(f"  Variables: {len(analysis['variables'])}, Functions: {len(analysis['functions'])}")
            print(f"  Components: {len(analysis['components'])}, Graphs: {len(analysis['graphs'])}")
            print(f"  Complexity Score: {analysis['complexity_score']}")
            
        except Exception as e:
            print(f"✗ Error analyzing blueprint: {e}")
            analysis["error"] = str(e)
        
        return analysis
    
    # ============================================================================
    # Blueprint Screenshot (Documentation)
    # ============================================================================
    
    def get_blueprint_screenshot_info(self) -> Dict[str, Any]:
        """
        Get information about blueprint screenshot capabilities.
        
        Note: Direct blueprint graph screenshots are NOT available via Python API.
        This function returns information about alternative methods.
        
        Returns:
            Dictionary with screenshot capability information and recommendations
        """
        return {
            "direct_screenshot_available": False,
            "python_api_limitation": "UE Python API does not expose blueprint graph screenshot functionality",
            "alternatives": [
                {
                    "method": "Blueprint Screenshot Tool Plugin",
                    "description": "Third-party plugin that adds screenshot buttons to Blueprint editor",
                    "url": "https://github.com/Gradess2019/BlueprintScreenshotTool",
                    "features": [
                        "Full graph capture regardless of size",
                        "Hotkeys (Ctrl+F7 for screenshot)",
                        "Configurable export settings",
                        "Automatic directory opening"
                    ]
                },
                {
                    "method": "Manual Editor Screenshot",
                    "description": "Use UE Editor's built-in screenshot functionality",
                    "command": "HighResShot 2x",
                    "limitation": "Only captures visible portion of editor"
                },
                {
                    "method": "Copy Blueprint as Text",
                    "description": "Export blueprint logic as text for analysis",
                    "tools": ["BlueprintUE online visualizer"],
                    "limitation": "Not a visual screenshot, but good for logic analysis"
                },
                {
                    "method": "Custom C++ Solution",
                    "description": "Implement graph rendering in C++ plugin",
                    "complexity": "High",
                    "requires": ["C++ development", "UE graph rendering knowledge"]
                }
            ],
            "what_we_can_analyze": [
                "Blueprint variables (names, types, count)",
                "Blueprint functions (names, count)",
                "Blueprint components (types, count)",
                "Blueprint graphs (names, types, count)",
                "Parent classes and hierarchy",
                "Complexity metrics (calculated from above)",
                "Blueprint metadata"
            ],
            "recommendation": "Use the Blueprint Screenshot Tool plugin for visual captures, and use this Python script for detailed data analysis"
        }
    
    # ============================================================================
    # Level and Actor Information
    # ============================================================================
    
    def collect_level_info(self) -> Dict[str, Any]:
        """Collect information about levels and actors."""
        if not self.available:
            return {"error": "UE not available"}
        
        info = {
            "current_level": {},
            "all_levels": [],
            "actors": {},
            "lighting": {},
            "volumes": {}
        }
        
        try:
            # Get current world
            world = self.unreal_editor_subsystem.get_editor_world()
            if not world:
                info["error"] = "No world loaded"
                return info
            
            info["current_level"]["name"] = world.get_name()
            info["current_level"]["path"] = world.get_path_name()
            
            # Get all actors in current level
            all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
            info["actors"]["total"] = len(all_actors)
            
            # Categorize actors
            actor_types = defaultdict(int)
            light_actors = []
            volume_actors = []
            
            for actor in all_actors:
                actor_class = actor.get_class().get_name()
                actor_types[actor_class] += 1
                
                # Collect lighting info
                if 'Light' in actor_class:
                    light_actors.append({
                        "name": actor.get_name(),
                        "class": actor_class,
                        "location": str(actor.get_actor_location())
                    })
                
                # Collect volume info
                if 'Volume' in actor_class:
                    volume_actors.append({
                        "name": actor.get_name(),
                        "class": actor_class
                    })
            
            info["actors"]["by_type"] = dict(sorted(actor_types.items(), key=lambda x: x[1], reverse=True))
            info["lighting"]["total_lights"] = len(light_actors)
            info["lighting"]["lights"] = light_actors[:20]  # Limit to first 20
            info["volumes"]["total_volumes"] = len(volume_actors)
            info["volumes"]["types"] = volume_actors[:20]  # Limit to first 20
            
            # Find all level assets
            level_filter = unreal.ARFilter(
                class_names=["World"],
                recursive_paths=True
            )
            levels = self.asset_registry.get_assets(level_filter)
            info["all_levels"] = [str(level.package_name) for level in levels]
            
            print(f"✓ Collected info on {info['actors']['total']} actors in current level")
            
        except Exception as e:
            print(f"✗ Error collecting level info: {e}")
            info["error"] = str(e)
        
        return info
    
    # ============================================================================
    # Material Information
    # ============================================================================
    
    def collect_material_info(self) -> Dict[str, Any]:
        """Collect information about materials and textures."""
        if not self.available:
            return {"error": "UE not available"}
        
        info = {
            "materials": {"total": 0, "instances": 0, "functions": 0},
            "textures": {"total": 0, "by_type": {}},
            "shader_complexity": {}
        }
        
        try:
            # Count materials
            material_filter = unreal.ARFilter(
                class_names=["Material"],
                recursive_paths=True
            )
            materials = self.asset_registry.get_assets(material_filter)
            info["materials"]["total"] = len(materials)
            
            # Count material instances
            mi_filter = unreal.ARFilter(
                class_names=["MaterialInstanceConstant"],
                recursive_paths=True
            )
            material_instances = self.asset_registry.get_assets(mi_filter)
            info["materials"]["instances"] = len(material_instances)
            
            # Count material functions
            mf_filter = unreal.ARFilter(
                class_names=["MaterialFunction"],
                recursive_paths=True
            )
            material_functions = self.asset_registry.get_assets(mf_filter)
            info["materials"]["functions"] = len(material_functions)
            
            # Count textures
            texture_filter = unreal.ARFilter(
                class_names=["Texture2D", "TextureCube", "TextureRenderTarget2D"],
                recursive_paths=True
            )
            textures = self.asset_registry.get_assets(texture_filter)
            info["textures"]["total"] = len(textures)
            
            # Categorize textures
            texture_types = defaultdict(int)
            for tex in textures:
                tex_class = str(tex.asset_class_path.asset_name)
                texture_types[tex_class] += 1
            
            info["textures"]["by_type"] = dict(texture_types)
            
            print(f"✓ Collected info on {info['materials']['total']} materials and {info['textures']['total']} textures")
            
        except Exception as e:
            print(f"✗ Error collecting material info: {e}")
            info["error"] = str(e)
        
        return info
    
    # ============================================================================
    # Plugin Information
    # ============================================================================
    
    def collect_plugin_info(self) -> Dict[str, Any]:
        """Collect information about installed plugins."""
        if not self.available:
            return {"error": "UE not available"}
        
        info = {
            "total_plugins": 0,
            "enabled_plugins": [],
            "disabled_plugins": [],
            "plugin_categories": {}
        }
        
        try:
            # Note: Full plugin enumeration requires file system access
            # PluginBlueprintLibrary exists but has limited Python exposure
            info["note"] = "Plugin enumeration requires file system access"
            
            # Try to get some plugin info from project directory
            project_dir = unreal.SystemLibrary.get_project_directory()
            plugins_dir = os.path.join(project_dir, "Plugins")
            
            if os.path.exists(plugins_dir):
                plugins = []
                for item in os.listdir(plugins_dir):
                    plugin_path = os.path.join(plugins_dir, item)
                    if os.path.isdir(plugin_path):
                        # Check for .uplugin file
                        uplugin_files = [f for f in os.listdir(plugin_path) if f.endswith('.uplugin')]
                        if uplugin_files:
                            plugins.append(item)
                
                info["project_plugins"] = plugins
                info["total_plugins"] = len(plugins)
            
            print(f"✓ Found {info['total_plugins']} project plugins")
            
        except Exception as e:
            print(f"✗ Error collecting plugin info: {e}")
            info["error"] = str(e)
        
        return info
    
    # ============================================================================
    # Performance Information
    # ============================================================================
    
    def collect_performance_info(self) -> Dict[str, Any]:
        """Collect performance-related information."""
        if not self.available:
            return {"error": "UE not available"}
        
        info = {
            "editor_stats": {},
            "rendering": {},
            "memory": {}
        }
        
        try:
            # Note: Actual performance metrics require PIE or game running
            # This collects configuration and available info
            
            info["note"] = "Detailed performance metrics available during PIE session"
            info["instructions"] = "Run 'stat fps', 'stat unit', 'stat memory' in console during PIE"
            
            # Get some basic info
            world = self.unreal_editor_subsystem.get_editor_world()
            if world:
                info["editor_stats"]["world_loaded"] = True
                info["editor_stats"]["world_name"] = world.get_name()
            
            print("✓ Collected performance info (limited in editor mode)")
            
        except Exception as e:
            print(f"✗ Error collecting performance info: {e}")
            info["error"] = str(e)
        
        return info
    
    # ============================================================================
    # Source Code Information
    # ============================================================================
    
    def collect_source_info(self) -> Dict[str, Any]:
        """Collect information about C++ source code."""
        if not self.available:
            return {"error": "UE not available"}
        
        info = {
            "has_source": False,
            "modules": [],
            "classes": 0
        }
        
        try:
            project_dir = unreal.SystemLibrary.get_project_directory()
            source_dir = os.path.join(project_dir, "Source")
            
            if os.path.exists(source_dir):
                info["has_source"] = True
                
                # Count modules
                modules = []
                for item in os.listdir(source_dir):
                    module_path = os.path.join(source_dir, item)
                    if os.path.isdir(module_path) and not item.startswith('.'):
                        modules.append(item)
                
                info["modules"] = modules
                
                # Count C++ files
                cpp_files = 0
                header_files = 0
                for root, dirs, files in os.walk(source_dir):
                    cpp_files += len([f for f in files if f.endswith('.cpp')])
                    header_files += len([f for f in files if f.endswith('.h')])
                
                info["cpp_files"] = cpp_files
                info["header_files"] = header_files
                info["total_source_files"] = cpp_files + header_files
            
            print(f"✓ Collected source code info: {info['total_source_files'] if info['has_source'] else 0} files")
            
        except Exception as e:
            print(f"✗ Error collecting source info: {e}")
            info["error"] = str(e)
        
        return info
    
    # ============================================================================
    # Animation Information
    # ============================================================================
    
    def collect_animation_info(self) -> Dict[str, Any]:
        """Collect information about animations and skeletal assets."""
        if not self.available:
            return {"error": "UE not available"}
        
        info = {
            "skeletal_meshes": 0,
            "animations": 0,
            "animation_blueprints": 0,
            "skeletons": 0
        }
        
        try:
            # Skeletal meshes
            sk_filter = unreal.ARFilter(
                class_names=["SkeletalMesh"],
                recursive_paths=True
            )
            skeletal_meshes = self.asset_registry.get_assets(sk_filter)
            info["skeletal_meshes"] = len(skeletal_meshes)
            
            # Animation sequences
            anim_filter = unreal.ARFilter(
                class_names=["AnimSequence"],
                recursive_paths=True
            )
            animations = self.asset_registry.get_assets(anim_filter)
            info["animations"] = len(animations)
            
            # Animation blueprints
            anim_bp_filter = unreal.ARFilter(
                class_names=["AnimBlueprint"],
                recursive_paths=True
            )
            anim_bps = self.asset_registry.get_assets(anim_bp_filter)
            info["animation_blueprints"] = len(anim_bps)
            
            # Skeletons
            skel_filter = unreal.ARFilter(
                class_names=["Skeleton"],
                recursive_paths=True
            )
            skeletons = self.asset_registry.get_assets(skel_filter)
            info["skeletons"] = len(skeletons)
            
            print(f"✓ Collected animation info: {info['animations']} animations")
            
        except Exception as e:
            print(f"✗ Error collecting animation info: {e}")
            info["error"] = str(e)
        
        return info
    
    # ============================================================================
    # Audio Information
    # ============================================================================
    
    def collect_audio_info(self) -> Dict[str, Any]:
        """Collect information about audio assets."""
        if not self.available:
            return {"error": "UE not available"}
        
        info = {
            "sound_waves": 0,
            "sound_cues": 0,
            "sound_classes": 0
        }
        
        try:
            # Sound waves
            sw_filter = unreal.ARFilter(
                class_names=["SoundWave"],
                recursive_paths=True
            )
            sound_waves = self.asset_registry.get_assets(sw_filter)
            info["sound_waves"] = len(sound_waves)
            
            # Sound cues
            sc_filter = unreal.ARFilter(
                class_names=["SoundCue"],
                recursive_paths=True
            )
            sound_cues = self.asset_registry.get_assets(sc_filter)
            info["sound_cues"] = len(sound_cues)
            
            # Sound classes
            scl_filter = unreal.ARFilter(
                class_names=["SoundClass"],
                recursive_paths=True
            )
            sound_classes = self.asset_registry.get_assets(scl_filter)
            info["sound_classes"] = len(sound_classes)
            
            print(f"✓ Collected audio info: {info['sound_waves']} sound waves")
            
        except Exception as e:
            print(f"✗ Error collecting audio info: {e}")
            info["error"] = str(e)
        
        return info


# ============================================================================
# Main Collection Function
# ============================================================================

def collect_all_info() -> Dict[str, Any]:
    """
    Collect all available information about the UE project.
    
    Returns:
        Dictionary containing all collected information
    """
    print("\n" + "="*60)
    print("Unreal Engine Project Information Collector")
    print("="*60 + "\n")
    
    if not UNREAL_AVAILABLE:
        print("✗ ERROR: Not running inside Unreal Engine!")
        print("This script must be run from UE's Python environment.")
        return {"error": "Not running in Unreal Engine"}
    
    collector = UEInfoCollector()
    
    if not collector.available:
        return {"error": "Failed to initialize collector"}
    
    # Collect all information
    all_info = {}
    
    print("Collecting project information...")
    all_info["project_info"] = collector.collect_project_info()
    
    print("\nCollecting asset information...")
    all_info["assets"] = collector.collect_asset_info()
    
    print("\nCollecting blueprint information...")
    all_info["blueprints"] = collector.collect_blueprint_info()
    
    print("\nCollecting blueprint screenshot capabilities...")
    all_info["blueprint_screenshots"] = collector.get_blueprint_screenshot_info()
    
    print("\nCollecting level information...")
    all_info["levels"] = collector.collect_level_info()
    
    print("\nCollecting material information...")
    all_info["materials"] = collector.collect_material_info()
    
    print("\nCollecting plugin information...")
    all_info["plugins"] = collector.collect_plugin_info()
    
    print("\nCollecting performance information...")
    all_info["performance"] = collector.collect_performance_info()
    
    print("\nCollecting source code information...")
    all_info["source"] = collector.collect_source_info()
    
    print("\nCollecting animation information...")
    all_info["animation"] = collector.collect_animation_info()
    
    print("\nCollecting audio information...")
    all_info["audio"] = collector.collect_audio_info()
    
    print("\n" + "="*60)
    print("Collection Complete!")
    print("="*60 + "\n")
    
    return all_info


# ============================================================================
# Output Functions
# ============================================================================

def print_report(info: Dict[str, Any], detailed: bool = False):
    """
    Print a human-readable report of the collected information.
    
    Args:
        info: Information dictionary from collect_all_info()
        detailed: Whether to print detailed information
    """
    print("\n" + "="*60)
    print("PROJECT INFORMATION REPORT")
    print("="*60 + "\n")
    
    # Project Info
    if "project_info" in info:
        proj = info["project_info"]
        print("PROJECT")
        print("-" * 60)
        if "project" in proj:
            print(f"  Name: {proj['project'].get('name', 'Unknown')}")
            print(f"  Current Level: {proj['project'].get('current_level', 'Unknown')}")
        if "engine_version" in proj:
            print(f"  Engine Version: {proj['engine_version'].get('full', 'Unknown')}")
        print()
    
    # Assets
    if "assets" in info:
        assets = info["assets"]
        print("ASSETS")
        print("-" * 60)
        print(f"  Total Assets: {assets.get('total_assets', 0)}")
        if "by_type" in assets and assets["by_type"]:
            print("  Top Asset Types:")
            for asset_type, count in list(assets["by_type"].items())[:10]:
                print(f"    - {asset_type}: {count}")
        print()
    
    # Blueprints
    if "blueprints" in info:
        bp = info["blueprints"]
        print("BLUEPRINTS")
        print("-" * 60)
        print(f"  Total Blueprints: {bp.get('total_blueprints', 0)}")
        print(f"  Actor Blueprints: {bp.get('actor_blueprints', 0)}")
        print(f"  Widget Blueprints: {bp.get('widget_blueprints', 0)}")
        print(f"  Animation Blueprints: {bp.get('animation_blueprints', 0)}")
        
        if "detailed_analysis" in bp:
            details = bp["detailed_analysis"]
            print(f"\n  Detailed Analysis:")
            print(f"    Total Variables: {details.get('total_variables', 0)}")
            print(f"    Total Functions: {details.get('total_functions', 0)}")
            print(f"    Total Components: {details.get('total_components', 0)}")
            print(f"    Avg Variables/BP: {details.get('avg_variables_per_bp', 0)}")
            print(f"    Avg Functions/BP: {details.get('avg_functions_per_bp', 0)}")
        
        if "largest_blueprints" in bp and bp["largest_blueprints"]:
            print(f"\n  Most Complex Blueprints:")
            for i, large_bp in enumerate(bp["largest_blueprints"][:5], 1):
                name = large_bp.get("name", "Unknown")
                vars_count = large_bp.get("variables", 0)
                funcs = large_bp.get("functions", 0)
                comps = large_bp.get("components", 0)
                print(f"    {i}. {name}: {vars_count} vars, {funcs} funcs, {comps} comps")
        print()
    
    # Levels
    if "levels" in info:
        levels = info["levels"]
        if "actors" in levels:
            print("LEVEL ACTORS")
            print("-" * 60)
            print(f"  Total Actors: {levels['actors'].get('total', 0)}")
            if "by_type" in levels["actors"] and levels["actors"]["by_type"]:
                print("  Top Actor Types:")
                for actor_type, count in list(levels["actors"]["by_type"].items())[:10]:
                    print(f"    - {actor_type}: {count}")
        print()
    
    # Materials
    if "materials" in info:
        mats = info["materials"]
        print("MATERIALS & TEXTURES")
        print("-" * 60)
        if "materials" in mats:
            print(f"  Materials: {mats['materials'].get('total', 0)}")
            print(f"  Material Instances: {mats['materials'].get('instances', 0)}")
            print(f"  Material Functions: {mats['materials'].get('functions', 0)}")
        if "textures" in mats:
            print(f"  Textures: {mats['textures'].get('total', 0)}")
        print()
    
    # Animation
    if "animation" in info:
        anim = info["animation"]
        print("ANIMATION")
        print("-" * 60)
        print(f"  Skeletal Meshes: {anim.get('skeletal_meshes', 0)}")
        print(f"  Animations: {anim.get('animations', 0)}")
        print(f"  Animation Blueprints: {anim.get('animation_blueprints', 0)}")
        print()
    
    # Audio
    if "audio" in info:
        audio = info["audio"]
        print("AUDIO")
        print("-" * 60)
        print(f"  Sound Waves: {audio.get('sound_waves', 0)}")
        print(f"  Sound Cues: {audio.get('sound_cues', 0)}")
        print()
    
    # Source Code
    if "source" in info:
        source = info["source"]
        print("SOURCE CODE")
        print("-" * 60)
        print(f"  Has C++ Source: {source.get('has_source', False)}")
        if source.get('has_source'):
            print(f"  Modules: {', '.join(source.get('modules', []))}")
            print(f"  Total Source Files: {source.get('total_source_files', 0)}")
        print()
    
    # Plugins
    if "plugins" in info:
        plugins = info["plugins"]
        print("PLUGINS")
        print("-" * 60)
        print(f"  Project Plugins: {plugins.get('total_plugins', 0)}")
        if "project_plugins" in plugins:
            for plugin in plugins["project_plugins"]:
                print(f"    - {plugin}")
        print()
    
    print("="*60)


def save_to_json(info: Dict[str, Any], filename: str = "ue_project_info.json"):
    """
    Save collected information to a JSON file.
    
    Args:
        info: Information dictionary from collect_all_info()
        filename: Output filename (must not contain path separators)
    """
    try:
        # Validate filename to prevent path traversal
        if os.path.sep in filename or (os.path.altsep and os.path.altsep in filename):
            raise ValueError(f"Filename must not contain path separators: {filename}")
        if filename.startswith('.'):
            raise ValueError(f"Filename must not start with '.': {filename}")
        
        # Get project saved directory
        if UNREAL_AVAILABLE:
            saved_dir = unreal.SystemLibrary.get_project_saved_directory()
            filepath = os.path.join(saved_dir, filename)
        else:
            filepath = filename
        
        with open(filepath, 'w') as f:
            json.dump(info, f, indent=2)
        
        print(f"✓ Information saved to: {filepath}")
        return filepath
        
    except Exception as e:
        print(f"✗ Error saving to JSON: {e}")
        return None


def save_to_markdown(info: Dict[str, Any], filename: str = "ue_project_info.md"):
    """
    Save collected information to a Markdown file.
    
    Args:
        info: Information dictionary from collect_all_info()
        filename: Output filename (must not contain path separators)
    """
    try:
        # Validate filename to prevent path traversal
        if os.path.sep in filename or (os.path.altsep and os.path.altsep in filename):
            raise ValueError(f"Filename must not contain path separators: {filename}")
        if filename.startswith('.'):
            raise ValueError(f"Filename must not start with '.': {filename}")
        
        # Get project saved directory
        if UNREAL_AVAILABLE:
            saved_dir = unreal.SystemLibrary.get_project_saved_directory()
            filepath = os.path.join(saved_dir, filename)
        else:
            filepath = filename
        
        with open(filepath, 'w') as f:
            f.write("# Unreal Engine Project Information\n\n")
            f.write(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n")
            
            # Project Info
            if "project_info" in info:
                f.write("## Project\n\n")
                proj = info["project_info"]
                if "project" in proj:
                    f.write(f"- **Name:** {proj['project'].get('name', 'Unknown')}\n")
                    f.write(f"- **Current Level:** {proj['project'].get('current_level', 'Unknown')}\n")
                if "engine_version" in proj:
                    f.write(f"- **Engine Version:** {proj['engine_version'].get('full', 'Unknown')}\n")
                f.write("\n")
            
            # Assets
            if "assets" in info:
                f.write("## Assets\n\n")
                assets = info["assets"]
                f.write(f"- **Total Assets:** {assets.get('total_assets', 0)}\n\n")
                if "by_type" in assets and assets["by_type"]:
                    f.write("### By Type\n\n")
                    for asset_type, count in list(assets["by_type"].items())[:20]:
                        f.write(f"- {asset_type}: {count}\n")
                    f.write("\n")
            
            # Blueprints
            if "blueprints" in info:
                f.write("## Blueprints\n\n")
                bp = info["blueprints"]
                f.write(f"- **Total Blueprints:** {bp.get('total_blueprints', 0)}\n")
                f.write(f"- **Actor Blueprints:** {bp.get('actor_blueprints', 0)}\n")
                f.write(f"- **Widget Blueprints:** {bp.get('widget_blueprints', 0)}\n")
                f.write(f"- **Animation Blueprints:** {bp.get('animation_blueprints', 0)}\n")
                f.write("\n")
            
            # Levels
            if "levels" in info and "actors" in info["levels"]:
                f.write("## Level Content\n\n")
                actors = info["levels"]["actors"]
                f.write(f"- **Total Actors:** {actors.get('total', 0)}\n\n")
                if "by_type" in actors and actors["by_type"]:
                    f.write("### Actor Types\n\n")
                    for actor_type, count in list(actors["by_type"].items())[:10]:
                        f.write(f"- {actor_type}: {count}\n")
                    f.write("\n")
            
            # Materials
            if "materials" in info:
                f.write("## Materials & Textures\n\n")
                mats = info["materials"]
                if "materials" in mats:
                    f.write(f"- **Materials:** {mats['materials'].get('total', 0)}\n")
                    f.write(f"- **Material Instances:** {mats['materials'].get('instances', 0)}\n")
                if "textures" in mats:
                    f.write(f"- **Textures:** {mats['textures'].get('total', 0)}\n")
                f.write("\n")
            
            # Animation
            if "animation" in info:
                f.write("## Animation\n\n")
                anim = info["animation"]
                f.write(f"- **Skeletal Meshes:** {anim.get('skeletal_meshes', 0)}\n")
                f.write(f"- **Animations:** {anim.get('animations', 0)}\n")
                f.write(f"- **Animation Blueprints:** {anim.get('animation_blueprints', 0)}\n")
                f.write("\n")
            
            # Audio
            if "audio" in info:
                f.write("## Audio\n\n")
                audio = info["audio"]
                f.write(f"- **Sound Waves:** {audio.get('sound_waves', 0)}\n")
                f.write(f"- **Sound Cues:** {audio.get('sound_cues', 0)}\n")
                f.write("\n")
            
            # Source Code
            if "source" in info:
                f.write("## Source Code\n\n")
                source = info["source"]
                f.write(f"- **Has C++ Source:** {source.get('has_source', False)}\n")
                if source.get('has_source'):
                    f.write(f"- **Modules:** {', '.join(source.get('modules', []))}\n")
                    f.write(f"- **Total Source Files:** {source.get('total_source_files', 0)}\n")
                f.write("\n")
        
        print(f"✓ Information saved to: {filepath}")
        return filepath
        
    except Exception as e:
        print(f"✗ Error saving to Markdown: {e}")
        return None


# ============================================================================
# Main Execution
# ============================================================================

if __name__ == "__main__":
    """Main execution when run directly."""
    
    # Collect all information
    info = collect_all_info()
    
    # Print report
    print_report(info)
    
    # Save to files
    json_file = save_to_json(info)
    md_file = save_to_markdown(info)
    
    print("\n" + "="*60)
    print("USAGE NOTES FOR AGENTS")
    print("="*60)
    print("\nThis information can help agents to:")
    print("  • Understand project structure and organization")
    print("  • Identify asset naming conventions")
    print("  • Analyze blueprint complexity and patterns")
    print("  • Detect potential performance issues")
    print("  • Find optimization opportunities")
    print("  • Understand the technology stack")
    print("  • Make informed decisions about code changes")
    print("\nFiles saved:")
    if json_file:
        print(f"  • JSON: {json_file}")
    if md_file:
        print(f"  • Markdown: {md_file}")
    print("\n" + "="*60)
