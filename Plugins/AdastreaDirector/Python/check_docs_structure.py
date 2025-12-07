"""Check current documentation structure"""
from pathlib import Path

root = Path("c:/Unreal Projects/Adastrea")

print("📚 Current Documentation Structure:\n")

dirs_to_check = [
    "wiki",
    ".github",
    "Assets", 
    "Source",
    "Plugins/AdastreaDirector",
    "Config",
    "Content/DataAssets"
]

for dir_name in dirs_to_check:
    path = root / dir_name
    if path.exists():
        md_count = len(list(path.rglob("*.md")))
        txt_count = len(list(path.rglob("*.txt")))
        py_count = len(list(path.rglob("*.py")))
        print(f"✅ {dir_name}:")
        print(f"   {md_count} .md, {txt_count} .txt, {py_count} .py files")
    else:
        print(f"❌ {dir_name}: Not found")

print("\n📊 Summary:")
total_md = len(list(root.rglob("*.md")))
print(f"Total .md files in project: {total_md}")
