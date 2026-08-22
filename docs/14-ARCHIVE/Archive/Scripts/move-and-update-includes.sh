#!/usr/bin/env bash
# Move MyFirstSpaceshipDataAsset.h to Private/Ships and update includes across the repo.
# Usage: ./move-and-update-includes.sh
set -euo pipefail

old="Source/Adastrea/Ships/MyFirstSpaceshipDataAsset.h"
new="Source/Adastrea/Private/Ships/MyFirstSpaceshipDataAsset.h"
branch="move/myfirstspaceshipdataasset-to-private"

if [ ! -f "$old" ]; then
  echo "ERROR: $old not found. Please run this from the repository root."
  exit 1
fi

# Create branch
git checkout -b "$branch"

# Ensure destination dir exists and move file
mkdir -p "$(dirname "$new")"
git mv "$old" "$new"

# Search for files referencing the header (excluding Binaries/Intermediate) and update include lines
echo "Searching for references to MyFirstSpaceshipDataAsset.h..."
matches=$(git grep -I --line-number -n -e 'MyFirstSpaceshipDataAsset.h' -- ':!Binaries' ':!Intermediate' || true)

if [ -z "$matches" ]; then
  echo "No references found outside Binaries/Intermediate. Only the file move was committed."
else
  echo "$matches" | cut -d: -f1 | sort -u | while read -r file; do
    echo "Updating includes in: $file"
    # Replace common include patterns:
    # - "Ships/MyFirstSpaceshipDataAsset.h" -> "Private/Ships/MyFirstSpaceshipDataAsset.h"
    # - <Ships/MyFirstSpaceshipDataAsset.h> -> <Private/Ships/MyFirstSpaceshipDataAsset.h>
    # - If project uses different include layout you may need to adjust the replacement manually.
    sed -i.bak -E 's#(["<])([^">]*Ships/)?MyFirstSpaceshipDataAsset.h([">])#\1Private/Ships/MyFirstSpaceshipDataAsset.h\3#g' "$file"
    rm -f "${file}.bak"
    git add "$file"
  done
fi

# Stage and commit the moved header
git add "$new"
git commit -m "Move MyFirstSpaceshipDataAsset.h to Private/Ships and update includes"

echo "Done. Created branch: $branch"
echo "Please review changes (git show --name-only HEAD or git diff origin/$branch..$branch)."