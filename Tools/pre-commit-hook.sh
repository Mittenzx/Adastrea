#!/bin/bash
#
# Pre-commit hook for Adastrea
#
# This hook runs code quality checks before allowing a commit.
# It helps catch common issues early in the development process.
#
# To install this hook:
#   cp Tools/pre-commit-hook.sh .git/hooks/pre-commit
#   chmod +x .git/hooks/pre-commit
#
# To bypass the hook (not recommended):
#   git commit --no-verify
#

set -e

echo "Running pre-commit code quality checks..."
echo ""

# Track if any checks failed
FAILED=0

# Check 1: UPROPERTY usage
echo "1. Checking UPROPERTY usage..."
if python3 Tools/check_uproperty.py; then
    echo "   ✅ UPROPERTY check passed"
else
    echo "   ⚠️  UPROPERTY check found issues (continuing)"
    # Don't fail commit on UPROPERTY warnings
fi
echo ""

# Check 2: Asset naming conventions
echo "2. Checking asset naming conventions..."
if python3 Tools/validate_naming.py; then
    echo "   ✅ Naming convention check passed"
else
    echo "   ⚠️  Naming convention check found issues (continuing)"
    # Don't fail commit on naming warnings
fi
echo ""

# Check 3: Check for secrets
echo "3. Checking for potential hardcoded secrets..."
if git diff --cached --name-only | grep -E '\.(cpp|h)$' | xargs -r grep -n "password\s*=\|apiKey\s*=\|api_key\s*=" 2>/dev/null; then
    echo "   ❌ Found potential hardcoded secrets!"
    echo "   Please remove sensitive data before committing."
    FAILED=1
else
    echo "   ✅ No obvious secrets found"
fi
echo ""

# Check 4: Check for debug code
echo "4. Checking for debug statements..."
if git diff --cached --name-only | grep -E '\.(cpp|h)$' | xargs -r grep -n "std::cout\|printf(" 2>/dev/null; then
    echo "   ⚠️  Found debug print statements (consider using UE_LOG instead)"
    # Don't fail commit on debug statements
else
    echo "   ✅ No debug statements found"
fi
echo ""

# Summary
echo "================================"
if [ $FAILED -eq 0 ]; then
    echo "✅ All checks passed!"
    echo "Proceeding with commit..."
    exit 0
else
    echo "❌ Some checks failed!"
    echo ""
    echo "To commit anyway, use: git commit --no-verify"
    echo "(Not recommended)"
    exit 1
fi
