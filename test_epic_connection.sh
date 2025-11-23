#!/usr/bin/env bash
# Epic Games Container Connection Test Script
# 
# This script helps diagnose why Unreal Engine container pulls are failing
# by testing various connection points and authorization levels.

set -e

# Configuration
EPIC_CONTAINER_REGISTRY="ghcr.io/epicgames/unreal-engine"
CONTAINER_TAG="dev-slim-5.6"
TEST_TIMEOUT=30

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Results tracking
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_SKIPPED=0

echo -e "${BLUE}==================================================${NC}"
echo -e "${BLUE}Epic Games Container Connection Diagnostic Tool${NC}"
echo -e "${BLUE}==================================================${NC}"
echo ""
echo "This script tests the connection and authorization chain"
echo "required to pull Epic Games Unreal Engine containers."
echo ""

# Function to print status
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓ PASS${NC} $2"
        ((TESTS_PASSED++))
    elif [ $1 -eq 2 ]; then
        echo -e "${YELLOW}⊘ SKIP${NC} $2"
        ((TESTS_SKIPPED++))
    else
        echo -e "${RED}✗ FAIL${NC} $2"
        ((TESTS_FAILED++))
    fi
}

# Test 1: Check if Docker is installed and running
echo -e "\n${BLUE}[Test 1/10] Docker Installation${NC}"
echo "---------------------------------------"
if command -v docker &> /dev/null; then
    DOCKER_VERSION=$(docker --version 2>&1)
    print_status 0 "Docker is installed: $DOCKER_VERSION"
    
    if docker info &> /dev/null; then
        print_status 0 "Docker daemon is running"
    else
        print_status 1 "Docker daemon is not running"
        echo -e "${YELLOW}   → Start Docker daemon and try again${NC}"
        exit 1
    fi
else
    print_status 1 "Docker is not installed"
    echo -e "${YELLOW}   → Install Docker from: https://docs.docker.com/get-docker/${NC}"
    exit 1
fi

# Test 2: Check GitHub Container Registry connectivity
echo -e "\n${BLUE}[Test 2/10] GitHub Container Registry Connectivity${NC}"
echo "---------------------------------------"
if timeout $TEST_TIMEOUT curl -s -o /dev/null -w "%{http_code}" https://ghcr.io | grep -q "200\|301\|302"; then
    print_status 0 "ghcr.io is reachable"
else
    print_status 1 "Cannot connect to ghcr.io"
    echo -e "${YELLOW}   → Check your internet connection${NC}"
    echo -e "${YELLOW}   → Check if ghcr.io is accessible from your network${NC}"
fi

# Test 3: Check Epic Games website connectivity
echo -e "\n${BLUE}[Test 3/10] Epic Games Services Connectivity${NC}"
echo "---------------------------------------"
if timeout $TEST_TIMEOUT curl -s -o /dev/null -w "%{http_code}" https://www.epicgames.com | grep -q "200\|301\|302"; then
    print_status 0 "epicgames.com is reachable"
else
    print_status 1 "Cannot connect to epicgames.com"
    echo -e "${YELLOW}   → Check your internet connection${NC}"
fi

if timeout $TEST_TIMEOUT curl -s -o /dev/null -w "%{http_code}" https://dev.epicgames.com | grep -q "200\|301\|302"; then
    print_status 0 "dev.epicgames.com is reachable"
else
    print_status 1 "Cannot connect to dev.epicgames.com"
fi

# Test 4: Check GitHub authentication
echo -e "\n${BLUE}[Test 4/10] GitHub Authentication${NC}"
echo "---------------------------------------"
if [ -z "$GITHUB_TOKEN" ]; then
    print_status 1 "GITHUB_TOKEN environment variable not set"
    echo -e "${YELLOW}   → For local testing: export GITHUB_TOKEN='your_token'${NC}"
    echo -e "${YELLOW}   → Create token at: https://github.com/settings/tokens${NC}"
    echo -e "${YELLOW}   → Required scope: read:packages${NC}"
    echo ""
    echo -e "${BLUE}Would you like to enter a GitHub token now? (y/n)${NC}"
    if [ -t 0 ]; then
        read -r -n 1 USE_TOKEN
        echo ""
        if [ "$USE_TOKEN" = "y" ] || [ "$USE_TOKEN" = "Y" ]; then
            echo -e "${BLUE}Enter your GitHub Personal Access Token:${NC}"
            read -r -s GITHUB_TOKEN
            export GITHUB_TOKEN
            echo ""
        fi
    fi
fi

if [ -n "$GITHUB_TOKEN" ]; then
    print_status 0 "GITHUB_TOKEN is set"
    
    # Validate token format
    if [[ $GITHUB_TOKEN =~ ^(ghp_|gho_|ghu_|ghs_|ghr_) ]]; then
        print_status 0 "Token format appears valid"
    else
        print_status 1 "Token format looks invalid (should start with ghp_, gho_, etc.)"
    fi
fi

# Test 5: Check GitHub username
echo -e "\n${BLUE}[Test 5/10] GitHub Username Configuration${NC}"
echo "---------------------------------------"
if [ -z "$GITHUB_USERNAME" ]; then
    print_status 1 "GITHUB_USERNAME environment variable not set"
    
    if [ -t 0 ]; then
        echo -e "${BLUE}Enter your GitHub username:${NC}"
        read -r GITHUB_USERNAME
        export GITHUB_USERNAME
    fi
fi

if [ -n "$GITHUB_USERNAME" ]; then
    print_status 0 "Using GitHub username: $GITHUB_USERNAME"
else
    print_status 2 "No GitHub username provided (skipping auth tests)"
fi

# Test 6: Test Docker login to GHCR
echo -e "\n${BLUE}[Test 6/10] Docker Login to GitHub Container Registry${NC}"
echo "---------------------------------------"
if [ -n "$GITHUB_TOKEN" ] && [ -n "$GITHUB_USERNAME" ]; then
    if echo "$GITHUB_TOKEN" | docker login ghcr.io -u "$GITHUB_USERNAME" --password-stdin &> /dev/null; then
        print_status 0 "Successfully authenticated with ghcr.io"
    else
        print_status 1 "Failed to authenticate with ghcr.io"
        echo -e "${YELLOW}   → Verify token has 'read:packages' scope${NC}"
        echo -e "${YELLOW}   → Check token hasn't expired${NC}"
        echo -e "${YELLOW}   → Verify GitHub username is correct${NC}"
    fi
else
    print_status 2 "Skipping (no credentials provided)"
fi

# Test 7: Check if user can access public Epic registry metadata
echo -e "\n${BLUE}[Test 7/10] Epic Games Container Registry Access${NC}"
echo "---------------------------------------"
# Try to get manifest without authentication (should fail but tells us about access)
MANIFEST_RESPONSE=$(docker manifest inspect ${EPIC_CONTAINER_REGISTRY}:${CONTAINER_TAG} 2>&1 || true)

if echo "$MANIFEST_RESPONSE" | grep -q "manifest unknown"; then
    print_status 1 "Container manifest not found or access denied"
    echo -e "${YELLOW}   → This usually means Epic Games organization access is not granted${NC}"
elif echo "$MANIFEST_RESPONSE" | grep -q "unauthorized"; then
    print_status 1 "Unauthorized to access Epic container metadata"
    echo -e "${YELLOW}   → Confirm you're logged in with Docker${NC}"
elif echo "$MANIFEST_RESPONSE" | grep -q "denied"; then
    print_status 1 "Access explicitly denied to Epic containers"
    echo -e "${YELLOW}   → Complete Epic Games account setup steps${NC}"
elif echo "$MANIFEST_RESPONSE" | grep -q "schemaVersion"; then
    print_status 0 "Can access Epic container metadata"
else
    print_status 1 "Unknown response from registry"
    echo -e "${YELLOW}   Response: $MANIFEST_RESPONSE${NC}"
fi

# Test 8: Attempt to pull container (main test)
echo -e "\n${BLUE}[Test 8/10] Container Pull Test${NC}"
echo "---------------------------------------"
if [ -n "$GITHUB_TOKEN" ] && [ -n "$GITHUB_USERNAME" ]; then
    echo "Attempting to pull ${EPIC_CONTAINER_REGISTRY}:${CONTAINER_TAG}..."
    echo "(This may take several minutes on first run)"
    
    PULL_OUTPUT=$(docker pull ${EPIC_CONTAINER_REGISTRY}:${CONTAINER_TAG} 2>&1 || true)
    
    if echo "$PULL_OUTPUT" | grep -q "Downloaded newer image\|Image is up to date"; then
        print_status 0 "Successfully pulled Epic Games container!"
        echo -e "${GREEN}   🎉 Your setup is complete and working!${NC}"
    elif echo "$PULL_OUTPUT" | grep -q "denied"; then
        print_status 1 "Access denied when pulling container"
        echo ""
        echo -e "${YELLOW}This means you're authenticated to GitHub, but not authorized for Epic containers.${NC}"
        echo ""
        echo -e "${YELLOW}Required setup steps:${NC}"
        echo -e "  1. Link Epic account to GitHub at:"
        echo -e "     ${BLUE}https://www.epicgames.com/account/connections${NC}"
        echo -e "  2. Accept Epic Games organization invitation:"
        echo -e "     ${BLUE}https://github.com/EpicGames${NC}"
        echo -e "  3. Accept EULA for containers at:"
        echo -e "     ${BLUE}https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-container-images${NC}"
        echo -e "  4. Wait 2-24 hours for permissions to propagate"
    elif echo "$PULL_OUTPUT" | grep -q "unauthorized"; then
        print_status 1 "Not authenticated to pull container"
        echo -e "${YELLOW}   → Check Docker login was successful${NC}"
        echo -e "${YELLOW}   → Verify GITHUB_TOKEN is valid${NC}"
    else
        print_status 1 "Container pull failed with unknown error"
        echo -e "${YELLOW}   Last 5 lines of output:${NC}"
        echo "$PULL_OUTPUT" | tail -5 | sed 's/^/   /'
    fi
else
    print_status 2 "Skipping (no credentials provided)"
fi

# Test 9: Check Docker config for credentials
echo -e "\n${BLUE}[Test 9/10] Docker Credentials Storage${NC}"
echo "---------------------------------------"
if [ -f "$HOME/.docker/config.json" ]; then
    print_status 0 "Docker config file exists"
    
    if grep -q "ghcr.io" "$HOME/.docker/config.json" 2>/dev/null; then
        print_status 0 "ghcr.io credentials are stored"
    else
        print_status 1 "No ghcr.io credentials found in config"
    fi
else
    print_status 1 "Docker config file not found"
fi

# Test 10: Network connectivity summary
echo -e "\n${BLUE}[Test 10/10] Network Diagnostics Summary${NC}"
echo "---------------------------------------"

# DNS resolution test
if nslookup ghcr.io > /dev/null 2>&1; then
    print_status 0 "DNS resolution for ghcr.io works"
else
    print_status 1 "Cannot resolve ghcr.io DNS"
fi

# Check if behind proxy
if [ -n "$HTTP_PROXY" ] || [ -n "$HTTPS_PROXY" ] || [ -n "$http_proxy" ] || [ -n "$https_proxy" ]; then
    echo -e "${YELLOW}   ⓘ Proxy environment variables detected${NC}"
    echo -e "${YELLOW}   → Make sure Docker is configured to use the proxy${NC}"
fi

# Summary
echo ""
echo -e "${BLUE}==================================================${NC}"
echo -e "${BLUE}Test Summary${NC}"
echo -e "${BLUE}==================================================${NC}"
echo ""
echo -e "Tests Passed:  ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests Failed:  ${RED}$TESTS_FAILED${NC}"
echo -e "Tests Skipped: ${YELLOW}$TESTS_SKIPPED${NC}"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
    echo ""
    echo "Your Epic Games container setup appears to be working correctly."
    echo "You should be able to use the Unreal Engine build workflows."
    exit 0
elif [ $TESTS_FAILED -le 3 ]; then
    echo -e "${YELLOW}⚠ Some tests failed${NC}"
    echo ""
    echo "Review the failed tests above and follow the recommendations."
    echo "Most issues can be resolved by completing the Epic Games setup steps."
    exit 1
else
    echo -e "${RED}✗ Multiple tests failed${NC}"
    echo ""
    echo "There appear to be significant configuration issues."
    echo "Please review all failed tests and complete the setup steps."
    echo ""
    echo "For detailed setup instructions, see:"
    echo "  - GITHUB_TOKEN_DOCKER_SETUP.md"
    echo "  - CLOUD_BUILD_QUICK_START.md"
    exit 1
fi
