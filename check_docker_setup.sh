#!/bin/bash
# Docker Setup Validation Script for Adastrea UE5 Builds
# This script helps verify that your Docker and Epic Games setup is correct

set -e

# Cleanup function
cleanup() {
    # Clean up temporary files
    rm -f /tmp/docker_pull_output.txt
}

# Set trap to call cleanup on exit
trap cleanup EXIT

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Adastrea Docker Setup Validator ===${NC}"
echo ""
echo "This script checks your Docker and Epic Games container access setup."
echo "For complete setup instructions, see: GITHUB_TOKEN_DOCKER_SETUP.md"
echo ""

# Function to print status
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓${NC} $2"
    else
        echo -e "${RED}✗${NC} $2"
    fi
}

# Check 1: Docker installed
echo -e "${BLUE}[1/6] Checking Docker installation...${NC}"
if command -v docker &> /dev/null; then
    DOCKER_VERSION=$(docker --version)
    print_status 0 "Docker is installed: $DOCKER_VERSION"
else
    print_status 1 "Docker is not installed"
    echo -e "${YELLOW}    → Install Docker from: https://docs.docker.com/get-docker/${NC}"
    exit 1
fi
echo ""

# Check 2: Docker daemon running
echo -e "${BLUE}[2/6] Checking Docker daemon...${NC}"
if docker info &> /dev/null; then
    print_status 0 "Docker daemon is running"
else
    print_status 1 "Docker daemon is not running"
    echo -e "${YELLOW}    → Start Docker daemon and try again${NC}"
    exit 1
fi
echo ""

# Check 3: GitHub Container Registry authentication
echo -e "${BLUE}[3/6] Checking GitHub Container Registry authentication...${NC}"
if [ -z "$GITHUB_TOKEN" ]; then
    print_status 1 "GITHUB_TOKEN environment variable not set"
    echo -e "${YELLOW}    → For local testing, create a token at: https://github.com/settings/tokens${NC}"
    echo -e "${YELLOW}    → Grant it 'read:packages' scope${NC}"
    echo -e "${YELLOW}    → Set it with: export GITHUB_TOKEN=\"your_token_here\"${NC}"
    echo -e "${YELLOW}    → For GitHub Actions, GITHUB_TOKEN is automatic (no action needed)${NC}"
    NEEDS_AUTH=1
else
    print_status 0 "GITHUB_TOKEN is set"
    
    # Try to login
    echo -e "${BLUE}    Attempting to login to ghcr.io...${NC}"
    
    # Prompt for GitHub username if not set
    if [ -z "$GITHUB_USERNAME" ]; then
        echo -e "${YELLOW}    → Enter your GitHub username: ${NC}"
        read -r GITHUB_USERNAME
        
        # Validate username is not empty
        if [ -z "$GITHUB_USERNAME" ]; then
            print_status 1 "GitHub username cannot be empty"
            NEEDS_AUTH=1
            echo ""
            continue
        fi
    fi
    
    if echo "$GITHUB_TOKEN" | docker login ghcr.io -u "$GITHUB_USERNAME" --password-stdin &> /dev/null; then
        print_status 0 "Successfully authenticated with GitHub Container Registry"
    else
        print_status 1 "Failed to authenticate with GitHub Container Registry"
        echo -e "${YELLOW}    → Verify your token has 'read:packages' scope${NC}"
        echo -e "${YELLOW}    → Check token hasn't expired${NC}"
        echo -e "${YELLOW}    → Verify GitHub username is correct${NC}"
        NEEDS_AUTH=1
    fi
fi
echo ""

# Check 4: Epic Games account setup (informational)
echo -e "${BLUE}[4/6] Epic Games Account Setup (Manual Verification)${NC}"
echo -e "${YELLOW}You need to manually verify the following steps:${NC}"
echo ""
echo -e "  1. Link Epic Games account to GitHub"
echo -e "     Visit: ${BLUE}https://www.epicgames.com/account/connections${NC}"
echo -e "     Status: [ ] Connected"
echo ""
echo -e "  2. Accept Epic Games organization invitation"
echo -e "     Visit: ${BLUE}https://github.com/EpicGames${NC}"
echo -e "     Status: [ ] Member of EpicGames organization"
echo ""
echo -e "  3. Accept Epic container EULA"
echo -e "     Visit: ${BLUE}https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-container-images${NC}"
echo -e "     Status: [ ] EULA accepted"
echo ""
echo -e "${YELLOW}Note: Without completing all 3 steps above, container pull will fail!${NC}"
echo ""

# Check 5: Try to pull Epic container (if authenticated)
if [ -z "$NEEDS_AUTH" ]; then
    echo -e "${BLUE}[5/6] Testing Epic Games container access...${NC}"
    echo -e "${YELLOW}This may take a few minutes on first run...${NC}"
    
    if docker pull ghcr.io/epicgames/unreal-engine:dev-slim-5.6 &> /tmp/docker_pull_output.txt; then
        print_status 0 "Successfully pulled Epic Games UE5.6 container"
        echo -e "${GREEN}    ✓ Your setup is complete and working!${NC}"
    else
        print_status 1 "Failed to pull Epic Games container"
        echo -e "${YELLOW}    Error output:${NC}"
        tail -5 /tmp/docker_pull_output.txt | sed 's/^/    /'
        echo ""
        echo -e "${YELLOW}    Common causes:${NC}"
        echo -e "${YELLOW}    → Epic account not linked (Step 1 above)${NC}"
        echo -e "${YELLOW}    → Organization invitation not accepted (Step 2 above)${NC}"
        echo -e "${YELLOW}    → EULA not accepted (Step 3 above)${NC}"
        echo -e "${YELLOW}    → Permissions still propagating (wait a few hours)${NC}"
        echo ""
        echo -e "${YELLOW}    See GITHUB_TOKEN_DOCKER_SETUP.md for detailed troubleshooting${NC}"
    fi
else
    echo -e "${BLUE}[5/6] Skipping container pull test${NC}"
    print_status 1 "Cannot test without GITHUB_TOKEN authentication"
fi
echo ""

# Check 6: Summary
echo -e "${BLUE}[6/6] Summary${NC}"
echo ""
if [ -z "$NEEDS_AUTH" ] && docker image inspect ghcr.io/epicgames/unreal-engine:dev-slim-5.6 &> /dev/null; then
    echo -e "${GREEN}✓ Setup Complete!${NC}"
    echo -e "  Your Docker environment is ready for Unreal Engine builds."
    echo ""
    echo -e "Next steps:"
    echo -e "  1. Enable the workflow: .github/workflows/ue-build-example.yml"
    echo -e "  2. Push to GitHub or trigger workflow manually"
    echo -e "  3. Monitor build in Actions tab"
else
    echo -e "${YELLOW}⚠ Setup Incomplete${NC}"
    echo -e "  Please complete the steps marked with ${RED}✗${NC} above."
    echo ""
    echo -e "For detailed setup instructions, see:"
    echo -e "  📖 ${BLUE}GITHUB_TOKEN_DOCKER_SETUP.md${NC} (comprehensive guide)"
    echo -e "  📖 ${BLUE}DOCKER_SETUP_QUICK_REFERENCE.md${NC} (quick reference)"
    echo -e "  📖 ${BLUE}CLOUD_BUILD_QUICK_START.md${NC} (cloud build setup)"
    echo ""
fi

echo -e "${BLUE}=== Validation Complete ===${NC}"
