#!/usr/bin/env python3
"""
Adastrea - Content Organization Tool

This script organizes Unreal Engine content files into proper folder structures
according to Adastrea's naming conventions and organization guidelines.

Features:
- Detects asset types (Materials, Meshes, Textures, Blueprints)
- Creates appropriate folder structure
- Moves files to correct locations
- Supports dry-run mode for safety
- Generates detailed report of changes
- Handles Fab imports and third-party content

Usage:
    # Dry run (preview changes without moving files)
    python Tools/organize_content.py --dry-run
    
    # Actually organize files
    python Tools/organize_content.py
    
    # Organize specific folder
    python Tools/organize_content.py --path Content/Imported
    
    # Generate report only
    python Tools/organize_content.py --report-only
"""

import sys
import shutil
import argparse
from pathlib import Path
from typing import Dict, List, Tuple, Optional
from collections import defaultdict


class ContentOrganizer:
    """Organizes Unreal Engine content files"""
    
    # Asset type detection based on common naming patterns
    ASSET_PATTERNS = {
        'Material': {
            'prefixes': ['M_', 'MI_', 'Material', 'default'],
            'folder': 'Materials'
        },
        'StaticMesh': {
            'prefixes': ['SM_', 'Cube', 'Cylinder', 'Sphere', 'Plane', 'Icosphere', 'mesh'],
            'folder': 'Meshes'
        },
        'Texture': {
            'prefixes': ['T_', 'TEX_', 'Texture'],
            'suffixes': ['_D', '_N', '_R', '_M', '_AO', '_E', '_Albedo', '_Normal', '_Roughness'],
            'folder': 'Textures'
        },
        'Blueprint': {
            'prefixes': ['BP_', 'WBP_'],
            'folder': 'Blueprints'
        },
        'DataAsset': {
            'prefixes': ['DA_'],
            'folder': 'DataAssets'
        },
        'Animation': {
            'prefixes': ['A_', 'Anim_'],
            'folder': 'Animations'
        },
        'Sound': {
            'prefixes': ['S_', 'Sound_'],
            'folder': 'Audio'
        },
        'ParticleSystem': {
            'prefixes': ['P_', 'PS_', 'NS_'],
            'folder': 'VFX'
        },
        'Null': {
            'prefixes': ['Null'],
            'folder': 'Uncategorized'
        }
    }
    
    # Folders that should not be reorganized
    PROTECTED_FOLDERS = [
        'Blueprints',
        'DataAssets',
        'UI',
        'VFX',
        'Input',
        'Maps',
        'Python',
        'Docs'
    ]
    
    # Subfolders within Meshes based on type
    MESH_CATEGORIES = {
        'Cube': 'Props',
        'Cylinder': 'Props',
        'Sphere': 'Props',
        'Plane': 'Props',
        'Icosphere': 'Props',
        'BezierCurve': 'Splines',
        'mesh': 'Imported'
    }
    
    def __init__(self, content_path: str, dry_run: bool = False, organize_unrecognized: bool = False):
        self.content_path = Path(content_path)
        self.dry_run = dry_run
        self.organize_unrecognized = organize_unrecognized
        self.moves: List[Tuple[Path, Path]] = []
        self.stats: Dict[str, int] = defaultdict(int)
        self.errors: List[str] = []
        self.unrecognized_files: List[Path] = []
    
    def detect_asset_type(self, filename: str) -> Optional[str]:
        """Detect asset type from filename"""
        name_without_ext = Path(filename).stem
        
        # Check each asset type
        for asset_type, patterns in self.ASSET_PATTERNS.items():
            # Check prefixes
            if 'prefixes' in patterns:
                for prefix in patterns['prefixes']:
                    if name_without_ext.startswith(prefix):
                        return asset_type
            
            # Check suffixes (mainly for textures)
            if 'suffixes' in patterns:
                for suffix in patterns['suffixes']:
                    if name_without_ext.endswith(suffix):
                        return asset_type
        
        # Default to StaticMesh for numbered variants (check against mesh categories)
        mesh_prefixes = list(self.MESH_CATEGORIES.keys()) + ['Cube_', 'Cylinder_', 'Sphere_']
        if any(name_without_ext.startswith(prefix) for prefix in mesh_prefixes):
            return 'StaticMesh'
        
        return None
    
    def get_target_folder(self, asset_type: str, filename: str) -> str:
        """Get target folder for an asset type"""
        base_folder = self.ASSET_PATTERNS.get(asset_type, {}).get('folder', 'Uncategorized')
        
        # Special handling for meshes - create subcategories
        if asset_type == 'StaticMesh':
            for mesh_prefix, category in self.MESH_CATEGORIES.items():
                if filename.startswith(mesh_prefix):
                    return f"{base_folder}/{category}"
        
        return base_folder
    
    def should_skip_folder(self, folder_path: Path) -> bool:
        """Check if folder should be skipped"""
        folder_name = folder_path.name
        
        # Skip protected folders
        if folder_name in self.PROTECTED_FOLDERS:
            return True
        
        # Skip if already organized (has appropriate subfolders)
        if folder_name in ['Materials', 'Meshes', 'Textures', 'Blueprints', 'DataAssets']:
            return True
        
        return False
    
    def scan_folder(self, folder_path: Path) -> List[Tuple[Path, str, str]]:
        """Scan folder and identify files to move"""
        files_to_organize = []
        
        if not folder_path.exists():
            self.errors.append(f"Folder does not exist: {folder_path}")
            return files_to_organize
        
        # Recursively scan folder
        for item in folder_path.iterdir():
            if item.is_dir():
                # Skip protected folders
                if not self.should_skip_folder(item):
                    files_to_organize.extend(self.scan_folder(item))
            elif item.is_file() and item.suffix in ['.uasset', '.umap']:
                # Detect asset type
                asset_type = self.detect_asset_type(item.name)
                if asset_type:
                    target_folder = self.get_target_folder(asset_type, item.name)
                    files_to_organize.append((item, asset_type, target_folder))
                    self.stats[asset_type] += 1
                else:
                    self.stats['Unrecognized'] += 1
                    self.unrecognized_files.append(item)
                    # Optionally move unrecognized files to a special folder
                    if self.organize_unrecognized:
                        files_to_organize.append((item, 'Unrecognized', 'Uncategorized'))
        
        return files_to_organize
    
    def organize_files(self, files: List[Tuple[Path, str, str]]) -> None:
        """Organize files by moving them to target folders"""
        for source_path, asset_type, target_folder in files:
            # Create target path
            target_path = self.content_path / target_folder / source_path.name
            
            # Create target directory if it doesn't exist
            target_dir = target_path.parent
            
            # Check if file already exists at target
            if target_path.exists():
                self.errors.append(f"Target already exists: {target_path}")
                continue
            
            # Record the move
            self.moves.append((source_path, target_path))
            
            if not self.dry_run:
                try:
                    # Create directory
                    target_dir.mkdir(parents=True, exist_ok=True)
                    
                    # Move file
                    shutil.move(str(source_path), str(target_path))
                    print(f"✓ Moved: {source_path.relative_to(self.content_path)} → {target_path.relative_to(self.content_path)}")
                except Exception as e:
                    self.errors.append(f"Failed to move {source_path}: {str(e)}")
    
    def generate_report(self) -> str:
        """Generate organization report"""
        report = []
        report.append("=" * 80)
        report.append("CONTENT ORGANIZATION REPORT")
        report.append("=" * 80)
        report.append("")
        
        # Summary statistics
        report.append("STATISTICS:")
        report.append("-" * 80)
        for asset_type, count in sorted(self.stats.items()):
            report.append(f"  {asset_type:20} : {count:4} files")
        report.append(f"  {'Total':20} : {sum(self.stats.values()):4} files")
        report.append("")
        
        # Moves
        if self.moves:
            report.append(f"PLANNED MOVES: {len(self.moves)}")
            report.append("-" * 80)
            
            # Group by target folder
            moves_by_folder = defaultdict(list)
            for source, target in self.moves:
                folder = target.parent.relative_to(self.content_path)
                moves_by_folder[str(folder)].append((source, target))
            
            for folder, moves in sorted(moves_by_folder.items()):
                report.append(f"\n  → {folder}/ ({len(moves)} files)")
                for source, target in moves[:5]:  # Show first 5
                    report.append(f"      • {source.name}")
                if len(moves) > 5:
                    report.append(f"      ... and {len(moves) - 5} more")
        
        report.append("")
        
        # Unrecognized files
        if self.unrecognized_files:
            report.append(f"UNRECOGNIZED FILES: {len(self.unrecognized_files)}")
            report.append("-" * 80)
            report.append("These files could not be automatically categorized:")
            for file in self.unrecognized_files[:20]:  # Show first 20
                report.append(f"  ? {file.name}")
            if len(self.unrecognized_files) > 20:
                report.append(f"  ... and {len(self.unrecognized_files) - 20} more")
            report.append("")
            if not self.organize_unrecognized:
                report.append("Tip: Use --organize-unrecognized to move these to Uncategorized/ folder")
                report.append("")
        
        # Errors
        if self.errors:
            report.append(f"ERRORS: {len(self.errors)}")
            report.append("-" * 80)
            for error in self.errors[:10]:  # Show first 10
                report.append(f"  ! {error}")
            if len(self.errors) > 10:
                report.append(f"  ... and {len(self.errors) - 10} more errors")
            report.append("")
        
        # Mode indicator
        if self.dry_run:
            report.append("=" * 80)
            report.append("DRY RUN - No files were actually moved")
            report.append("Run without --dry-run to perform the organization")
            report.append("=" * 80)
        
        return "\n".join(report)
    
    def run(self, target_folder: Optional[str] = None) -> None:
        """Run the organization process"""
        print("=" * 80)
        print("ADASTREA CONTENT ORGANIZER")
        print("=" * 80)
        print(f"Content Path: {self.content_path}")
        print(f"Mode: {'DRY RUN' if self.dry_run else 'LIVE'}")
        print("")
        
        # Determine scan path
        if target_folder:
            scan_path = self.content_path / target_folder
        else:
            scan_path = self.content_path
        
        print(f"Scanning: {scan_path}")
        print("")
        
        # Scan for files
        files_to_organize = self.scan_folder(scan_path)
        
        # Organize files
        if files_to_organize:
            print(f"\nFound {len(files_to_organize)} files to organize")
            self.organize_files(files_to_organize)
        else:
            print("No files found to organize")
        
        # Generate report once and reuse
        report = self.generate_report()
        
        # Print report
        print("\n")
        print(report)
        
        # Save report to file
        report_path = self.content_path.parent / "content_organization_report.txt"
        with open(report_path, 'w') as f:
            f.write(report)
        print(f"\nReport saved to: {report_path}")


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description='Organize Adastrea content files into proper folder structure',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Preview changes without moving files
  python Tools/organize_content.py --dry-run
  
  # Organize all files in Content directory
  python Tools/organize_content.py
  
  # Organize only Imported folder
  python Tools/organize_content.py --path Imported
  
  # Generate report without organizing
  python Tools/organize_content.py --report-only
        """
    )
    
    parser.add_argument(
        '--content-dir',
        type=str,
        default='Content',
        help='Path to Content directory (default: Content)'
    )
    
    parser.add_argument(
        '--path',
        type=str,
        help='Specific subfolder to organize (relative to Content)'
    )
    
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='Preview changes without actually moving files'
    )
    
    parser.add_argument(
        '--report-only',
        action='store_true',
        help='Generate report only, do not move files'
    )
    
    parser.add_argument(
        '--organize-unrecognized',
        action='store_true',
        help='Move unrecognized files to Uncategorized/ folder'
    )
    
    args = parser.parse_args()
    
    # Determine content path
    script_dir = Path(__file__).parent.parent
    content_path = script_dir / args.content_dir
    
    if not content_path.exists():
        print(f"ERROR: Content directory not found: {content_path}")
        sys.exit(1)
    
    # Create organizer
    dry_run = args.dry_run or args.report_only
    organizer = ContentOrganizer(str(content_path), dry_run=dry_run, 
                                organize_unrecognized=args.organize_unrecognized)
    
    # Run organization
    try:
        organizer.run(target_folder=args.path)
    except KeyboardInterrupt:
        print("\n\nOrganization cancelled by user")
        sys.exit(1)
    except Exception as e:
        print(f"\nERROR: {str(e)}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
