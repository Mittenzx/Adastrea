"""
Test the new ingestion configuration without actually ingesting
"""
from pathlib import Path

project_root = Path(__file__).parent.parent.parent.parent

# Same logic as setup_rag.py
docs_dirs = [
    # Core project documentation
    (project_root / "wiki", "Wiki Documentation"),
    (project_root / ".github", "GitHub Configuration & Instructions"),
    
    # System guides and templates
    (project_root / "Assets", "Asset Templates & Guides"),
    
    # Plugin documentation
    (project_root / "Plugins" / "AdastreaDirector", "Director Plugin Docs"),
    
    # Source code documentation
    (project_root / "Source", "Source Code Docs"),
    
    # Root-level essential docs only (no build artifacts)
    (project_root, "Root Documentation", [
        "README.md", "ARCHITECTURE.md", "CODE_STYLE.md", "CONTRIBUTING.md",
        "CHANGELOG.md", "QUICKSTART.md", "ROADMAP.md", "DOCUMENTATION_INDEX.md",
        "CHEATSHEET.md", "BLUEPRINT_*.md", "ENHANCED_INPUT_GUIDE.md",
        "PLUGIN_DEVELOPMENT_GUIDE.md", "PROCEDURAL_GENERATION_*.md",
        "TESTING_*.md", "OPTIMIZATION_*.md", "DATA_MANAGEMENT_*.md",
        "CONTENT_CREATION_QUICKSTART.md", "EDITOR_IMPLEMENTATION_GUIDE.md",
        "SCHEMA_GUIDE.md", "YAML_*.md", "GOOGLE_SHEETS_*.md",
        "EXAMPLE_DATA_ASSETS.md", "SCENE_POPULATION_*.md",
        "GENERATOR_*.md", "SCRIPTS_README.md", "INPUT_SYSTEM_CLARIFICATION.md"
    ])
]

print("📚 Documentation to be ingested:\n")
print("=" * 60)

# Files/patterns to exclude
exclude_patterns = [
    "OBSOLETE", "ZZ_", "_OLD", "_BACKUP", "DEPRECATED",
    "check_docker", "DOCKER_", "CONTAINER_", "MCP_REMOTE",
    "STUB_AND_PLACEHOLDER", "AUDIT_SUMMARY", "WIKI_IMPLEMENTATION",
    "CLOUD_BUILD", "GITHUB_TOKEN", "UE_PLUGIN_FEASIBILITY"
]

def should_exclude(file_path):
    """Check if file should be excluded"""
    name = file_path.name
    return any(pattern in name for pattern in exclude_patterns)

total_files = 0
excluded_count = 0

for entry in docs_dirs:
    if len(entry) == 2:
        docs_path, label = entry
        patterns = None
    else:
        docs_path, label, patterns = entry
        
    if docs_path.exists():
        if patterns:
            # Filter by specific patterns
            md_files = []
            for pattern in patterns:
                if "*" in pattern:
                    md_files.extend(docs_path.glob(pattern))
                else:
                    file_path = docs_path / pattern
                    if file_path.exists():
                        md_files.append(file_path)
        else:
            # Get all markdown files recursively
            md_files = list(docs_path.rglob("*.md"))
        
        # Apply exclusion filter
        original_count = len(md_files)
        md_files = [f for f in md_files if not should_exclude(f)]
        excluded_count += (original_count - len(md_files))
        
        count = len(md_files)
        total_files += count
        print(f"\n📁 {label}")
        print(f"   Path: {docs_path}")
        print(f"   Files: {count}")
        
        if patterns and count > 0:
            print(f"   Sample files:")
            for f in sorted(md_files)[:5]:
                print(f"     - {f.name}")
            if count > 5:
                print(f"     ... and {count - 5} more")

print("\n" + "=" * 60)
print(f"\n📊 Total: {total_files} markdown files will be ingested")
if excluded_count > 0:
    print(f"🚫 Excluded: {excluded_count} files (obsolete/build artifacts)")
print("\n✨ This is more focused than before!")
print("   Previous: ~352 files (including build artifacts)")
print(f"   Updated: {total_files} files (curated documentation only)")
print("\n📋 Excluded patterns:")
for pattern in exclude_patterns:
    print(f"   - {pattern}")
