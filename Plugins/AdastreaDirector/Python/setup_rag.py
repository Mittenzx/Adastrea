"""
Complete RAG setup script with ingestion
"""
import os
import sys
import subprocess
from pathlib import Path

def run_ingestion(docs_dir=None):
    """Run the document ingestion process"""
    print("\n" + "=" * 60)
    print("Starting Document Ingestion")
    print("=" * 60)
    
    project_root = Path(__file__).parent.parent.parent.parent
    
    # Default docs directories - prioritized and organized
    if docs_dir is None:
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
            
            # Root-level essential docs only (exclude build/temp/obsolete files)
            (project_root, "Root Documentation", [
                "README.md", "ARCHITECTURE.md", "CODE_STYLE.md", "CONTRIBUTING.md",
                "CHANGELOG.md", "QUICKSTART.md", "ROADMAP.md", "DOCUMENTATION_INDEX.md",
                "CHEATSHEET.md", "BLUEPRINT_*.md", "ENHANCED_INPUT_GUIDE.md",
                "PLUGIN_DEVELOPMENT_GUIDE.md", "PROCEDURAL_GENERATION_*.md",
                "TESTING_*.md", "OPTIMIZATION_*.md", "DATA_MANAGEMENT_*.md",
                "CONTENT_CREATION_QUICKSTART.md", "EDITOR_IMPLEMENTATION_GUIDE.md",
                "SCHEMA_GUIDE.md", "YAML_*.md", "GOOGLE_SHEETS_*.md",
                "EXAMPLE_DATA_ASSETS.md", "SCENE_POPULATION_*.md",
                "GENERATOR_*.md", "SCRIPTS_README.md", "INPUT_SYSTEM_CLARIFICATION.md",
                "GAME_MOCKUPS.md", "INTEGRATION_SUMMARY.md", "PERFORMANCE_IMPROVEMENTS.md",
                "SETUP_CHECKLIST.md"
            ])
        ]
    else:
        docs_dirs = [(Path(docs_dir), "Custom Directory", None)]
    
    # Files/patterns to exclude (obsolete, build artifacts, temporary files)
    exclude_patterns = [
        "OBSOLETE", "ZZ_", "_OLD", "_BACKUP", "DEPRECATED",
        "check_docker", "DOCKER_", "CONTAINER_", "MCP_REMOTE",
        "STUB_AND_PLACEHOLDER", "AUDIT_SUMMARY", "WIKI_IMPLEMENTATION",
        "CLOUD_BUILD", "GITHUB_TOKEN", "UE_PLUGIN_FEASIBILITY"
    ]
    
    def should_exclude(file_path):
        """Check if file should be excluded based on patterns"""
        name = file_path.name
        return any(pattern in name for pattern in exclude_patterns)
    
    # Collect all markdown files with filtering
    all_docs = []
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
            md_files = [f for f in md_files if not should_exclude(f)]
            
            all_docs.extend(md_files)
            print(f"📁 {label}: {len(md_files)} files")
    
    print(f"\n📚 Total: {len(all_docs)} markdown files")
    
    if len(all_docs) == 0:
        print("❌ No markdown files found to ingest!")
        return False
    
    # Run ingestion for each directory
    print("\n🔄 Starting ingestion process...")
    print("   This may take a few minutes...")
    
    try:
        from rag_ingestion import RAGIngestionAgent, ProgressWriter
        
        print("\n📥 Processing documents...")
        
        # Initialize the ingestion agent once
        progress_writer = ProgressWriter()
        agent = RAGIngestionAgent(
            collection_name="adastrea_docs",
            persist_directory=str(project_root / "Plugins" / "AdastreaDirector" / "Chromadb"),
            progress_writer=progress_writer,
        )
        
        # Aggregate stats
        total_stats = {
            "total_files": 0,
            "skipped": 0,
            "updated": 0,
            "added": 0,
            "errors": 0,
        }
        
        # Ingest each directory
        for entry in docs_dirs:
            docs_path = entry[0] if isinstance(entry, tuple) else entry
            
            if docs_path.exists():
                print(f"\n📂 Processing: {docs_path.name}")
                stats = agent.ingest_directory_incremental(
                    str(docs_path),
                    force_reingest=False
                )
                
                # Aggregate stats
                for key in total_stats:
                    total_stats[key] += stats.get(key, 0)
        
        print("\n✅ Ingestion Complete!")
        print(f"   Total files: {total_stats.get('total_files', 0)}")
        print(f"   Added: {total_stats.get('added', 0)} files")
        print(f"   Updated: {total_stats.get('updated', 0)} files")
        print(f"   Skipped: {total_stats.get('skipped', 0)} files (unchanged)")
        print(f"   Errors: {total_stats.get('errors', 0)} files")
        
        return True
        
    except Exception as e:
        print(f"\n❌ Ingestion failed: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description="Setup AdastreaDirector RAG system")
    parser.add_argument('--ingest', action='store_true', help='Run document ingestion')
    parser.add_argument('--reingest', action='store_true', help='Force re-ingestion of all documents')
    parser.add_argument('--docs-dir', type=str, help='Custom documentation directory')
    
    args = parser.parse_args()
    
    # Always run wizard first
    print("Running setup wizard...")
    result = subprocess.run([sys.executable, 'setup_wizard.py'])
    
    if result.returncode != 0:
        print("\n⚠️  Setup wizard found issues. Please fix them before ingesting.")
        return False
    
    # If --reingest, delete existing ChromaDB
    if args.reingest:
        chromadb_path = Path(__file__).parent.parent / "Chromadb"
        if chromadb_path.exists():
            print("\n🗑️  Removing existing ChromaDB...")
            import shutil
            shutil.rmtree(chromadb_path)
            print("✅ ChromaDB removed")
    
    # Run ingestion if requested
    if args.ingest or args.reingest:
        success = run_ingestion(args.docs_dir)
        return success
    
    print("\n💡 Tip: Run with --ingest to start document ingestion")
    return True

if __name__ == '__main__':
    success = main()
    sys.exit(0 if success else 1)
