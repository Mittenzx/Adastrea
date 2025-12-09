#!/usr/bin/env python3
"""
Module Dependency Checker for Adastrea
Detects circular dependencies between Unreal Engine modules
"""

import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Set, Tuple

def find_build_cs_files(source_dir: Path) -> List[Path]:
    """Find all .Build.cs files in the source directory"""
    build_files = []
    for root, dirs, files in os.walk(source_dir):
        for file in files:
            if file.endswith('.Build.cs'):
                build_files.append(Path(root) / file)
    return build_files

def extract_module_name(build_file: Path) -> str:
    """Extract module name from Build.cs file path"""
    return build_file.stem.replace('.Build', '')

def parse_dependencies(build_file: Path) -> Tuple[str, Set[str]]:
    """
    Parse a .Build.cs file to extract module dependencies
    Returns: (module_name, set of dependency module names)
    """
    module_name = extract_module_name(build_file)
    dependencies = set()
    
    try:
        content = build_file.read_text(encoding='utf-8')
        
        # Find PublicDependencyModuleNames.AddRange
        public_deps_pattern = r'PublicDependencyModuleNames\.AddRange\s*\(\s*new\s+string\[\]\s*\{([^}]+)\}'
        public_matches = re.finditer(public_deps_pattern, content, re.DOTALL)
        
        for match in public_matches:
            deps_str = match.group(1)
            # Extract quoted strings
            dep_names = re.findall(r'"([^"]+)"', deps_str)
            dependencies.update(dep_names)
        
        # Find PrivateDependencyModuleNames.AddRange
        private_deps_pattern = r'PrivateDependencyModuleNames\.AddRange\s*\(\s*new\s+string\[\]\s*\{([^}]+)\}'
        private_matches = re.finditer(private_deps_pattern, content, re.DOTALL)
        
        for match in private_matches:
            deps_str = match.group(1)
            dep_names = re.findall(r'"([^"]+)"', deps_str)
            dependencies.update(dep_names)
        
        # Find individual Add statements
        add_pattern = r'(?:Public|Private)DependencyModuleNames\.Add\s*\(\s*"([^"]+)"\s*\)'
        add_matches = re.finditer(add_pattern, content)
        
        for match in add_matches:
            dependencies.add(match.group(1))
            
    except Exception as e:
        print(f"Warning: Could not parse {build_file}: {e}", file=sys.stderr)
    
    return module_name, dependencies

def build_dependency_graph(source_dir: Path) -> Dict[str, Set[str]]:
    """
    Build a dependency graph for all modules
    Returns: dict mapping module_name -> set of dependencies
    """
    build_files = find_build_cs_files(source_dir)
    graph = {}
    
    for build_file in build_files:
        module_name, deps = parse_dependencies(build_file)
        graph[module_name] = deps
    
    return graph

def find_circular_dependencies(graph: Dict[str, Set[str]]) -> List[List[str]]:
    """
    Find circular dependencies in the module graph
    Returns: list of cycles (each cycle is a list of module names)
    """
    cycles = []
    
    def dfs(node: str, path: List[str], visited: Set[str], rec_stack: Set[str]):
        visited.add(node)
        rec_stack.add(node)
        path.append(node)
        
        # Get dependencies for this module
        dependencies = graph.get(node, set())
        
        for dep in dependencies:
            # Skip Unreal Engine core modules
            if dep in ['Core', 'CoreUObject', 'Engine', 'InputCore', 'EnhancedInput',
                       'UMG', 'Slate', 'SlateCore', 'AIModule', 'NavigationSystem',
                       'Niagara', 'PhysicsCore', 'OnlineSubsystem']:
                continue
            
            if dep not in graph:
                # Dependency module not in our project, skip
                continue
            
            if dep in rec_stack:
                # Found a cycle
                cycle_start = path.index(dep)
                cycle = path[cycle_start:] + [dep]
                cycles.append(cycle)
            elif dep not in visited:
                dfs(dep, path[:], visited, rec_stack)
        
        rec_stack.remove(node)
    
    visited = set()
    for module in graph.keys():
        if module not in visited:
            dfs(module, [], visited, set())
    
    return cycles

def check_includes_for_circular_deps(source_dir: Path, graph: Dict[str, Set[str]]) -> List[str]:
    """
    Check for potential circular dependencies in include statements
    Returns: list of warning messages
    """
    warnings = []
    
    # Map module names to their source directories
    module_dirs = {}
    for build_file in find_build_cs_files(source_dir):
        module_name = extract_module_name(build_file)
        module_dirs[module_name] = build_file.parent
    
    # Check each module's source files
    for module_name, module_dir in module_dirs.items():
        if not module_dir.exists():
            continue
        
        # Find all .h and .cpp files
        for src_file in module_dir.rglob('*.h'):
            try:
                content = src_file.read_text(encoding='utf-8')
                
                # Check for includes of other modules
                for other_module in module_dirs.keys():
                    if other_module == module_name:
                        continue
                    
                    # Check if this module includes the other module
                    include_pattern = rf'#include\s+"[^"]*{other_module}/[^"]*"'
                    if re.search(include_pattern, content):
                        # Check if there's a direct dependency declared
                        if other_module not in graph.get(module_name, set()):
                            warnings.append(
                                f"Warning: {module_name} includes {other_module} headers "
                                f"but doesn't declare dependency in {module_name}.Build.cs"
                            )
                        
                        # Check for potential circular dependency
                        if module_name in graph.get(other_module, set()):
                            warnings.append(
                                f"Potential circular dependency: {module_name} includes {other_module}, "
                                f"and {other_module} depends on {module_name}"
                            )
                            
            except Exception as e:
                pass  # Skip files that can't be read
    
    return warnings

def main():
    """Main entry point"""
    # Get source directory
    script_dir = Path(__file__).parent
    repo_root = script_dir.parent.parent
    source_dir = repo_root / 'Source'
    
    if not source_dir.exists():
        print(f"Error: Source directory not found: {source_dir}", file=sys.stderr)
        sys.exit(1)
    
    print("Checking module dependencies...")
    print(f"Source directory: {source_dir}")
    print()
    
    # Build dependency graph
    graph = build_dependency_graph(source_dir)
    
    print(f"Found {len(graph)} modules:")
    for module in sorted(graph.keys()):
        print(f"  - {module}")
    print()
    
    # Find circular dependencies
    cycles = find_circular_dependencies(graph)
    
    if cycles:
        print("❌ CIRCULAR DEPENDENCIES DETECTED:")
        print()
        for i, cycle in enumerate(cycles, 1):
            print(f"  Cycle {i}: {' -> '.join(cycle)}")
        print()
        print("Circular dependencies must be fixed before the project can build correctly.")
        print("See .github/MODULE_DEPENDENCY_FIX.md for resolution strategies.")
        sys.exit(1)
    else:
        print("✅ No circular dependencies detected in module declarations")
        print()
    
    # Check for potential issues in include statements
    warnings = check_includes_for_circular_deps(source_dir, graph)
    
    if warnings:
        print("⚠️  WARNINGS:")
        print()
        for warning in warnings:
            print(f"  {warning}")
        print()
        print("These warnings may indicate potential circular dependency issues.")
        print("Review the code to ensure proper module separation.")
    else:
        print("✅ No include-based circular dependency warnings")
    
    print()
    print("Module dependency check complete!")
    
    # Exit with success if no cycles found
    sys.exit(0)

if __name__ == '__main__':
    main()
