# Cloud Build Services for Adastrea

This document provides information about external applications and websites that can be used to test build the Adastrea Unreal Engine 5.6 game project in cloud or CI/CD environments.

## Overview

Building Unreal Engine projects requires significant computational resources and proper environment setup. This guide outlines various cloud-based build services and platforms that can be integrated with the Adastrea project for automated building, testing, and deployment.

## Available Cloud Build Services

### 1. Epic Games Horde Build Platform

**Description:** Native CI/CD automation and distributed build tool for Unreal Engine, released with UE5.4+.

**Key Features:**
- Open-source and purpose-built for Unreal Engine
- Distributed compilation using Unreal Build Accelerator (UBA)
- Comprehensive build dashboards and analytics
- Powers Epic's internal workflows (including Fortnite)
- Can be deployed on AWS with containerization

**Best For:**
- Studios with scale requirements
- Teams already using Perforce
- Projects needing robust distributed builds

**Getting Started:**
- Official Documentation: [Horde Documentation](https://docs.unrealengine.com/5.0/en-US/horde-build-automation-tool-in-unreal-engine/)
- Deployment: Self-hosted on AWS or use managed service (RUNREAL × Hathora)
- Requirements: UE5.4+ recommended

**Implementation Steps:**
1. Set up AWS infrastructure or use managed Horde service
2. Configure Perforce or Git integration
3. Set up build agents with UE5 installed
4. Configure build jobs in Horde dashboard
5. Integrate with source control webhooks

**Resources:**
- [Building Games with Unreal Engine and Horde on AWS](https://www.youtube.com/watch?v=kIP4wsVprYY)
- [Hathora Managed Horde Service](https://blog.hathora.dev/accelerate-unreal-engine-development-with-horde/)

---

### 2. Unreal Cloud Build

**Description:** Managed service for leveraging Unreal Build Accelerator with cloud or Kubernetes clusters.

**Key Features:**
- No need to self-host Horde infrastructure
- Utilizes Unreal Build Accelerator (UBA)
- Simplified setup for smaller teams
- Works without Perforce requirement

**Best For:**
- Small to medium teams
- Teams not using Perforce
- Teams wanting managed infrastructure

**Getting Started:**
- Website: [https://unreal.build/](https://unreal.build/)
- Sign up for service and follow onboarding
- Configure with your Git repository

**Implementation Steps:**
1. Sign up at unreal.build
2. Connect your Git repository
3. Configure build settings for UE5.6
4. Set up automated triggers
5. Monitor builds through web dashboard

---

### 3. GitHub Actions with Unreal Engine Docker Containers

**Description:** Use Docker containers with Unreal Engine in GitHub Actions workflows for automated builds.

**Key Features:**
- Integrated directly with GitHub repository
- Uses official Epic Games UE containers
- Free for public repositories (limited minutes for private)
- Highly customizable workflows
- Can run on GitHub-hosted or self-hosted runners

**Best For:**
- Projects hosted on GitHub
- Teams familiar with Docker
- Open-source or public projects
- Teams wanting full workflow control

**Getting Started:**

#### Prerequisites:
- Docker installed
- GitHub account with UE container access
- (For local Docker authentication only) GitHub Personal Access Token with `read:packages` scope  
  *Note: For GitHub Actions workflows, the built-in `secrets.GITHUB_TOKEN` is used and a Personal Access Token is not required.*

#### Docker Container Access:

**Important:** Epic's Unreal Engine container images are hosted in a private organization and require explicit access. Follow these steps carefully:

1. **Link Epic Games account to GitHub:**
   - Visit [www.epicgames.com/account/connections](https://www.epicgames.com/account/connections)
   - Connect your GitHub account to your Epic Games account
   - This is the first step but does NOT automatically grant container access

2. **Join Epic Games GitHub Organization:**
   - After linking accounts, Epic should send you an invitation to join their GitHub organization
   - Check your GitHub notifications or email for the invitation
   - Visit [github.com/EpicGames](https://github.com/EpicGames) and accept the invitation
   - **This step is critical** - without organization membership, you cannot pull containers

3. **Accept Epic EULA for container images:**
   - Visit [Epic's Container Images Documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-container-images)
   - Review and accept the license agreement for using container images

4. **Verify your access:**
   - Visit [github.com/EpicGames](https://github.com/EpicGames) to confirm you're a member
   - You should see "Member" badge if access is granted (visibility depends on organization settings)
   - Note: Access may take a few hours to propagate after accepting the invitation

5. **Authenticate Docker** (for local development only):
   
   **Important:** Replace the placeholder values below with your actual credentials:
   - `YOUR_TOKEN` → Your personal GitHub Personal Access Token (create at github.com/settings/tokens)
   - `YOUR_GITHUB_USERNAME` → Your GitHub username (e.g., "Mittenzx")
   
   ```bash
   # Example command structure (replace placeholders with actual values):
   # The PAT must have 'read:packages' scope
   echo "YOUR_TOKEN" | docker login ghcr.io -u YOUR_GITHUB_USERNAME --password-stdin
   ```
   
   For GitHub Actions workflows, use the built-in `secrets.GITHUB_TOKEN` or configure a custom `secrets.GHCR_TOKEN` secret instead.

**Troubleshooting Access Issues:**
- If you don't receive an organization invitation after linking accounts, contact Epic Games support
- If you get "denied" errors when pulling containers, verify organization membership
- Some accounts may require manual approval by Epic - this can take 24-48 hours

#### Available Actions:
- [UE5-Build-Project](https://github.com/marketplace/actions/ue5-build-project) - Specialized action for UE builds
- Official Epic UE containers from GitHub Container Registry

#### Sample Workflow:
```yaml
name: Unreal Engine Build

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build:
    runs-on: ubuntu-latest
    
    steps:
      - name: Checkout Repository
        uses: actions/checkout@v4
      
      - name: Login to GitHub Container Registry
        run: echo ${{ secrets.GITHUB_TOKEN }} | docker login ghcr.io -u $GITHUB_ACTOR --password-stdin
      
      - name: Build Unreal Project
        # Use UE5-Build-Project action from marketplace
        # See: https://github.com/marketplace/actions/ue5-build-project
        run: |
          # Custom build commands here
          # Or use marketplace UE5 action when available
          echo "Build using Unreal Automation Tool"
          
      - name: Upload Build Artifacts
        uses: actions/upload-artifact@v4
        with:
          name: Build
          path: Build/
```

**Implementation Resources:**
- [UE Container Images Quick Start](https://dev.epicgames.com/documentation/en-us/unreal-engine/quick-start-guide-for-using-container-images-in-unreal-engine)
- [ue4-docker Documentation](https://ue4-docker.docs.adamrehn.com/)
- [GitHub Actions with Docker Guide](https://docs.docker.com/guides/gha/)

---

### 4. TensorWorks Admiral

**Description:** Managed Unreal Engine cloud CI/CD service with specialized UE expertise.

**Key Features:**
- Purpose-built for Unreal Engine projects
- Managed infrastructure - no DevOps required
- Includes Pixel Streaming capabilities
- Container-based deployments
- Expert support from TensorWorks team

**Best For:**
- Teams wanting turnkey solutions
- Projects needing Pixel Streaming
- Studios without dedicated DevOps engineers

**Getting Started:**
- Website: [https://tensorworks.com.au/](https://tensorworks.com.au/)
- Contact TensorWorks for consultation
- Pricing based on usage and requirements

**Implementation:**
1. Contact TensorWorks sales team
2. Discuss project requirements and scale
3. Onboard to Admiral platform
4. Configure CI/CD pipelines with TensorWorks support
5. Monitor builds through Admiral dashboard

---

### 5. AWS Game Tech for Unreal Engine

**Description:** Comprehensive AWS services configured for Unreal Engine development.

**Key Features:**
- Virtual workstations with pre-configured UE5
- Perforce Helix Core hosting
- Build pipelines with Incredibuild integration
- GameLift for server hosting with UE5 plugins
- Scalable compute for distributed builds
- Full AWS ecosystem integration

**Best For:**
- Enterprises and large studios
- Teams needing complete infrastructure
- Projects requiring dedicated servers
- Teams already using AWS

**Getting Started:**
- Information: [AWS Game Tech - Unreal Engine](https://aws.amazon.com/gametech/partners/unreal/)
- AWS account required
- Follow AWS Game Tech setup guides

**Implementation Steps:**
1. Set up AWS account and billing
2. Deploy virtual workstations or build agents
3. Configure Perforce or Git integration
4. Set up build automation with AWS CodeBuild or CodePipeline
5. Integrate with S3 for artifact storage
6. Configure GameLift for multiplayer (if needed)

**Cost Considerations:**
- Pay-per-use model
- Free tier available for new accounts
- Estimate costs with AWS Pricing Calculator

---

### 6. Unreal Containers CI/CD

**Description:** Community-maintained containerized build approach using ue4-docker.

**Key Features:**
- Open-source and community-driven
- Supports Linux and Windows containers
- Automation test support
- Offscreen rendering capabilities
- Can integrate with any CI system

**Best For:**
- Teams wanting open-source solutions
- Advanced users comfortable with Docker
- Custom CI/CD requirements
- Budget-conscious teams

**Getting Started:**
- Documentation: [UnrealContainers.com](https://unrealcontainers.com/docs/use-cases/continuous-integration)
- GitHub: [ue4-docker](https://github.com/adamrehn/ue4-docker)

**Implementation:**
1. Install ue4-docker on build machine
2. Build custom UE5 container images
3. Configure CI system (Jenkins, GitLab CI, etc.)
4. Create build scripts using containerized UE
5. Store images privately (Epic EULA requirement)

**Important Note:**
- Built UE container images must remain private per Epic's EULA
- Cannot publicly distribute built images

---

## Comparison Matrix

| Service | Setup Complexity | Cost | Best For | Self-Hosted |
|---------|-----------------|------|----------|-------------|
| **Horde** | High | Variable | Large studios | Yes (AWS recommended) |
| **Unreal Cloud Build** | Low | Subscription | Small/Medium teams | No |
| **GitHub Actions + Docker** | Medium | Free/Paid | GitHub projects | Optional |
| **TensorWorks Admiral** | Low | Subscription | Turnkey solution | No |
| **AWS Game Tech** | High | Pay-per-use | Enterprises | Yes |
| **Unreal Containers** | High | Infrastructure only | Custom needs | Yes |

## Recommendations for Adastrea

Based on the project characteristics:
- **Current State:** Open-source, hosted on GitHub, UE5.6
- **Team Size:** Variable (open-source contributors)
- **Budget:** Likely limited

### Recommended Approach:

#### Option 1: GitHub Actions with Docker (Recommended for Adastrea)
**Why:**
- Free for public repositories
- Integrated with existing GitHub workflow
- Flexible and customizable
- Good documentation and community support
- Can start small and scale

**Implementation Priority:**
1. Set up GitHub Actions workflow with UE Docker containers
2. Configure for development branch builds
3. Add build status badges to README
4. Expand to automated testing as needed

#### Option 2: Unreal Cloud Build (Alternative)
**Why:**
- Simplest setup
- Managed service
- Good for teams without DevOps expertise

**Consideration:** May require subscription cost

#### Option 3: Self-Hosted with Horde (Future Growth)
**Why:**
- Most powerful for scaling
- Full control
- Best performance for large projects

**Consideration:** Requires significant setup and AWS costs

## Implementation Guide for GitHub Actions

### Step 1: Prepare Repository

> **Note:** An example workflow file is provided in the repository as `.github/workflows/ue-build-example.yml.disabled`. You can copy and rename this file to `.github/workflows/unreal-build.yml` to use it as a starting point for your workflow.

Create `.github/workflows/unreal-build.yml`:

```yaml
name: Unreal Engine Build

on:
  push:
    branches: [ main, develop ]
    paths:
      - 'Source/**'
      - 'Content/**'
      - 'Config/**'
      - '*.uproject'
  pull_request:
    branches: [ main, develop ]
  workflow_dispatch:  # Manual trigger

jobs:
  build-windows:
    name: Build Windows
    runs-on: windows-latest
    
    steps:
      - name: Checkout Repository
        uses: actions/checkout@v4
        with:
          lfs: true
      
      - name: Setup Build Environment
        run: |
          echo "Setting up Unreal Engine build environment"
          # Add UE build tool setup here
      
      - name: Build Project
        run: |
          echo "Building Adastrea project"
          # Add build commands here
      
      - name: Upload Artifacts
        if: success()
        uses: actions/upload-artifact@v4
        with:
          name: Windows-Build
          path: Build/Windows/
          retention-days: 7

  build-linux:
    name: Build Linux
    runs-on: ubuntu-latest
    
    steps:
      - name: Checkout Repository
        uses: actions/checkout@v4
        with:
          lfs: true
      
      - name: Login to GitHub Container Registry
        run: echo ${{ secrets.GITHUB_TOKEN }} | docker login ghcr.io -u $GITHUB_ACTOR --password-stdin
      
      - name: Build with UE Container
        run: |
          docker pull ghcr.io/epicgames/unreal-engine:dev-slim-5.6
          # Add container build commands here
      
      - name: Upload Artifacts
        if: success()
        uses: actions/upload-artifact@v4
        with:
          name: Linux-Build
          path: Build/Linux/
          retention-days: 7
```

### Step 2: Configure Secrets

Add these secrets to your GitHub repository (Settings → Secrets and variables → Actions):
- `UE_LICENSE_KEY` - Unreal Engine license (if applicable)
- Add any API keys or credentials needed

### Step 3: Test Workflow

1. Commit the workflow file
2. Push to trigger the build
3. Monitor in Actions tab
4. Review build logs and artifacts

## Additional Resources

### Documentation:
- [Unreal Engine CI/CD Community Tutorial](https://dev.epicgames.com/community/learning/tutorials/1wVK/let-s-do-your-first-ci-with-unreal-engine)
- [Scalable UE CI/CD Pipeline (GitHub)](https://github.com/AGreatPigeon/ScalableUECICDPipeline)
- [UnrealContainers Documentation](https://unrealcontainers.com/)

### Build Tools:
- Unreal Build Tool (UBT) - Included with UE5
- Unreal Automation Tool (UAT) - Included with UE5
- Unreal Build Accelerator (UBA) - Distributed build acceleration

### Community:
- [Unreal Engine Forums - Build Systems](https://forums.unrealengine.com/)
- [Epic Games Developer Community](https://dev.epicgames.com/community/)

## Testing and Validation

Before implementing a cloud build service:

1. **Test locally first:**
   ```bash
   ./SetupCheck.sh
   # or
   python SetupCheck.py
   ```

2. **Verify build requirements:**
   - Check UE5.6 compatibility
   - Verify all source files compile
   - Test with minimal configuration

3. **Start small:**
   - Begin with basic compilation
   - Add cooking and packaging later
   - Expand to multi-platform builds incrementally

4. **Monitor costs:**
   - Track build minutes (GitHub Actions)
   - Monitor AWS/cloud costs
   - Optimize build times to reduce costs

## Troubleshooting

### Common Issues:

**Issue:** Docker login fails with "Cannot perform an interactive login from a non TTY device"
**Root Cause:** GHCR_TOKEN secret is not configured or is empty
**Solution:** 
  1. Create a GitHub Personal Access Token (PAT) at https://github.com/settings/tokens
  2. Token must have `read:packages` scope
  3. Add the token as a secret named `GHCR_TOKEN` in repository settings
  4. Or update workflow to use `secrets.GITHUB_TOKEN` instead (automatic fallback is now implemented)

**Issue:** Docker pull fails with "denied" error (even though login succeeds)
**Root Cause:** Not a member of Epic Games GitHub organization
**Solution:** 
  1. Link Epic account at https://www.epicgames.com/account/connections
  2. Accept Epic Games organization invitation (check GitHub notifications/email)
  3. Verify membership at https://github.com/EpicGames
  4. Wait 1-2 hours for permissions to propagate
  5. If no invitation received, contact Epic Games support

**Issue:** Build times are too long
**Solution:** Use build caching, distributed builds (UBA), or faster runners

**Issue:** Insufficient disk space
**Solution:** Clean build artifacts, use larger runners, or optimize project

**Issue:** Container pull rate limits
**Solution:** Authenticate properly, use self-hosted runners, or cache images

## Next Steps

1. Choose build service based on team needs and budget
2. Follow implementation guide for chosen service
3. Set up basic build workflow
4. Test with current Adastrea project state
5. Document any project-specific configuration
6. Train team members on using the build system
7. Monitor and optimize build performance

## Conclusion

Multiple viable cloud build services exist for Adastrea's Unreal Engine 5.6 project. GitHub Actions with Docker containers provides the best balance of cost, flexibility, and integration for an open-source project hosted on GitHub. Teams can start with this approach and migrate to more powerful solutions like Horde as the project scales.

For immediate implementation in a GitHub Actions environment, focus on:
1. Setting up basic GitHub Actions workflow
2. Configuring UE Docker containers
3. Testing build pipeline with Adastrea project
4. Documenting results and optimizations

---

**Document Version:** 1.0  
**Last Updated:** 2025-11-21  
**Maintained By:** Adastrea Development Team
