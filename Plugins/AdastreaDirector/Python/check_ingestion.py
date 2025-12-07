"""
Check ingestion progress
"""
import sys
from pathlib import Path

chromadb_path = Path(__file__).parent.parent / "Chromadb"

if not chromadb_path.exists():
    print("⏳ Ingestion not started yet")
    sys.exit(1)

# Count files in ChromaDB
file_count = len(list(chromadb_path.rglob("*")))
print(f"✅ ChromaDB created: {file_count} files")

# Check collection
collection_path = chromadb_path / "chroma.sqlite3"
if collection_path.exists():
    print(f"✅ Database file exists: {collection_path}")
    size_mb = collection_path.stat().st_size / (1024 * 1024)
    print(f"   Size: {size_mb:.2f} MB")
else:
    print("⏳ Database file not yet created")

print("\n💡 Ingestion is running in the background.")
print("   This process may take 5-10 minutes for 352 files.")
print("   You can continue using VS Code - ingestion will complete automatically.")
