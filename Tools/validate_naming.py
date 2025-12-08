#!/usr/bin/env python3
"""
Asset Naming Convention Validator

Validates that Unreal Engine assets follow the project's naming conventions.
Enforces prefixes like BP_, DA_, M_, MI_, T_, etc.

Usage:
    python Tools/validate_naming.py
    
Exit codes:
    0: All checks passed or no issues found
    1: Found naming violations
"""

import os
import sys
from pathlib import Path
import re

# Expected prefixes for different asset types
ASSET_PREFIXES = {
    '.uasset': {
        'Blueprint': ['BP_'],
        'DataAsset': ['DA_'],
        'Material': ['M_', 'MI_', 'MF_'],  # Material, Material Instance, Material Function
        'Texture': ['T_'],
        'StaticMesh': ['SM_'],
        'SkeletalMesh': ['SK_'],
        'Animation': ['A_', 'AM_', 'AS_'],  # Animation, AnimMontage, AnimSequence
        'Sound': ['S_'],
        'Particle': ['P_'],
        'Widget': ['WBP_', 'W_'],
        'Level': ['L_'],
    }
}

# Folders that should contain specific asset types
FOLDER_REQUIREMENTS = {
    'Blueprints': ['BP_', 'WBP_', 'W_'],
    'DataAssets': ['DA_'],
    'Materials': ['M_', 'MI_', 'MF_'],
    'Textures': ['T_'],
    'Meshes': ['SM_', 'SK_'],
    'Audio': ['S_'],
}

def check_asset_name(filepath, content_dir):
    """
    Check if an asset follows naming conventions.
    
    Args:
        filepath: Path to the asset file
        content_dir: Root Content directory path
        
    Returns:
        List of issues (empty if valid)
    """
    issues = []
    
    filename = filepath.name
    rel_path = filepath.relative_to(content_dir)
    parent_folder = filepath.parent.name
    
    # Skip some special files/folders
    if filename.startswith('_') or filename.startswith('.'):
        return issues
    
    # Check if in Content directory
    if 'Content' not in str(filepath):
        return issues
    
    # Get file extension
    ext = filepath.suffix.lower()
    
    # Check .uasset files
    if ext == '.uasset':
        # Remove the extension to get asset name
        asset_name = filepath.stem
        
        # Check if it has a known prefix
        has_valid_prefix = False
        for asset_type, prefixes in ASSET_PREFIXES['.uasset'].items():
            if any(asset_name.startswith(prefix) for prefix in prefixes):
                has_valid_prefix = True
                break
        
        if not has_valid_prefix:
            # Check if it's in a folder that has specific requirements
            if parent_folder in FOLDER_REQUIREMENTS:
                expected_prefixes = FOLDER_REQUIREMENTS[parent_folder]
                issues.append({
                    'file': rel_path,
                    'message': f'Asset in {parent_folder}/ should start with one of: {", ".join(expected_prefixes)}',
                    'severity': 'warning'
                })
            else:
                issues.append({
                    'file': rel_path,
                    'message': f'Asset name does not follow naming convention (no recognized prefix)',
                    'severity': 'info'
                })
        
        # Check for spaces in name (not allowed)
        if ' ' in asset_name:
            issues.append({
                'file': rel_path,
                'message': 'Asset name contains spaces (use underscores instead)',
                'severity': 'error'
            })
        
        # Check for PascalCase after prefix
        if '_' in asset_name:
            parts = asset_name.split('_', 1)
            if len(parts) > 1:
                name_part = parts[1]
                # Check if it starts with lowercase (bad)
                if len(name_part) > 0 and name_part[0].islower():
                    issues.append({
                        'file': rel_path,
                        'message': f'Asset name should use PascalCase after prefix (found: {name_part})',
                        'severity': 'warning'
                    })
    
    return issues

def scan_content_directory(repo_root):
    """
    Scan the Content directory for assets.
    
    Args:
        repo_root: Repository root path
        
    Returns:
        List of all issues found
    """
    all_issues = []
    content_dir = Path(repo_root) / 'Content'
    
    if not content_dir.exists():
        print(f"Info: Content directory not found: {content_dir}")
        print("This check is optional if assets are not in the repository.")
        return all_issues
    
    # Find all .uasset files
    asset_files = list(content_dir.rglob('*.uasset'))
    
    if not asset_files:
        print("Info: No .uasset files found in Content directory.")
        print("Assets may be stored in a separate repository or not committed.")
        return all_issues
    
    print(f"Scanning {len(asset_files)} asset files...")
    
    for asset_file in asset_files:
        issues = check_asset_name(asset_file, content_dir)
        all_issues.extend(issues)
    
    return all_issues

def main():
    """Main entry point."""
    repo_root = Path(__file__).parent.parent
    
    print("=" * 80)
    print("Asset Naming Convention Validation")
    print("=" * 80)
    print()
    
    issues = scan_content_directory(repo_root)
    
    if not issues:
        print("✅ All assets follow naming conventions (or no assets found)")
        print()
        return 0
    
    # Categorize by severity
    errors = [i for i in issues if i['severity'] == 'error']
    warnings = [i for i in issues if i['severity'] == 'warning']
    infos = [i for i in issues if i['severity'] == 'info']
    
    if errors:
        print(f"❌ Found {len(errors)} errors:")
        print()
        for issue in errors:
            print(f"   {issue['file']}: {issue['message']}")
        print()
    
    if warnings:
        print(f"⚠️  Found {len(warnings)} warnings:")
        print()
        for issue in warnings:
            print(f"   {issue['file']}: {issue['message']}")
        print()
    
    if infos:
        print(f"ℹ️  Found {len(infos)} suggestions:")
        print()
        for issue in infos[:10]:  # Limit to first 10
            print(f"   {issue['file']}: {issue['message']}")
        if len(infos) > 10:
            print(f"   ... and {len(infos) - 10} more")
        print()
    
    print("=" * 80)
    print("Naming Convention Reference:")
    print("  BP_  = Blueprint")
    print("  DA_  = Data Asset")
    print("  M_   = Material")
    print("  MI_  = Material Instance")
    print("  T_   = Texture")
    print("  SM_  = Static Mesh")
    print("  WBP_ = Widget Blueprint")
    print("=" * 80)
    
    # Return 1 only if there are errors, not warnings/infos
    return 1 if errors else 0

if __name__ == '__main__':
    sys.exit(main())
