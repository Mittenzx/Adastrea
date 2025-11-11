# ROADMAP.md Update Guidelines

This document provides clear guidelines on when and how to update the project roadmap.

## Table of Contents

- [When to Update ROADMAP.md](#when-to-update-roadmapmd)
- [What to Update](#what-to-update)
- [How to Update](#how-to-update)
- [Update Checklist](#update-checklist)
- [Examples](#examples)

---

## When to Update ROADMAP.md

Update ROADMAP.md when your pull request includes changes that affect:

### ✅ Update Required

1. **System Completion or Major Progress**
   - Completing a system listed in the "In Progress" section
   - Achieving a significant milestone (e.g., 50% → 80% completion)
   - Moving a system from "Planned" to "In Progress"

2. **Phase Transitions**
   - Completing all milestones for a development phase
   - Starting a new development phase ahead of schedule
   - Changing phase timelines or objectives

3. **New Major Features**
   - Adding a new system not previously mentioned in the roadmap
   - Implementing significant functionality that affects multiple systems
   - Adding new tools or automation that impacts development workflow

4. **Timeline Changes**
   - Significant delays or acceleration of planned features
   - Adjusting quarterly milestone dates
   - Reprioritizing development phases

5. **System Maturity Changes**
   - Updating the System Maturity Matrix (completeness percentage)
   - Changing system status (Complete ✅, In Progress 🔄, Planned ⏳)
   - Updating documentation or testing status for systems

### ⏭️ Update Not Required

1. **Minor Bug Fixes**
   - Small bug fixes that don't affect system status
   - Code refactoring without feature changes
   - Internal code improvements

2. **Documentation-Only Changes**
   - Updating guides, tutorials, or examples
   - Fixing typos or improving clarity
   - Adding code comments

3. **Small Incremental Work**
   - Small contributions toward a larger feature
   - Minor enhancements to existing systems
   - Regular maintenance tasks

4. **Build/CI Changes**
   - Build system improvements
   - CI/CD pipeline updates
   - Testing infrastructure changes

**When in doubt**: Ask yourself, "Would someone looking at the roadmap care about this change?" If yes, update the roadmap.

---

## What to Update

Depending on your changes, update the relevant sections:

### 1. Current Status Section

Update if completing or making significant progress on systems:

```markdown
### ✅ Completed Systems (0.9.0)
- Add newly completed systems here

### 🔄 In Progress (0.9.x - 1.0.0)
- Update progress on active development
- Move completed items to "Completed Systems"

### 📊 System Maturity Matrix
- Update completeness percentages
- Change status icons (✅, 🔄, ⏳, ❌)
- Update "Next Steps" for systems
```

### 2. Development Phases

Update phase completion status:

```markdown
### Phase X: Name (Status)
**Timeline**: ...
**Status**: ✅ Complete / 🔄 In Progress (X% complete) / ⏳ Planned

#### Completed Milestones
- Add completed milestones with ✅ checkmarks

#### Active Development
- Update progress indicators (✅ complete, 🔄 in progress, ⏳ planned)
```

### 3. Detailed Feature Roadmap

Update specific feature implementation progress:

```markdown
#### Phase X: Feature Name (Q4 2025)
- [x] Completed tasks
- [ ] Remaining tasks
- Update timeline if needed
```

### 4. Last Updated Date

**Always update** the "Last Updated" date at the top of ROADMAP.md:

```markdown
**Last Updated**: [Current Date]
**Current Version**: [Current Version]
```

---

## How to Update

### Step-by-Step Process

1. **Review Your Changes**
   - Identify which systems, phases, or features your PR affects
   - Determine the magnitude of impact on the roadmap

2. **Update Relevant Sections**
   - Open `ROADMAP.md` in your editor
   - Find the sections that need updates (see "What to Update" above)
   - Make precise, accurate updates

3. **Update the Date**
   - Change the "Last Updated" date at the top of the file
   - Use format: `Month DD, YYYY` (e.g., "November 11, 2025")

4. **Verify Consistency**
   - Ensure all related sections are updated together
   - Check that percentages and status icons are consistent
   - Verify timeline dates are accurate

5. **Include in Your PR**
   - Add ROADMAP.md to your commit
   - Mention the roadmap update in your PR description
   - Check the "ROADMAP.md updated" box in the PR template

### Best Practices

- **Be Accurate**: Only claim completion when features are truly done
- **Be Specific**: Update exact percentages and milestones
- **Be Consistent**: Keep formatting and style matching existing content
- **Be Concise**: Don't add unnecessary detail; maintain readability
- **Be Honest**: Reflect actual progress, not aspirational goals

---

## Update Checklist

Use this checklist when updating ROADMAP.md:

- [ ] Identified which sections need updates
- [ ] Updated "Current Status" section if applicable
- [ ] Updated "System Maturity Matrix" if system status changed
- [ ] Updated relevant "Development Phase" section
- [ ] Updated "Detailed Feature Roadmap" if specific features were implemented
- [ ] Updated "Last Updated" date at the top of the file
- [ ] Verified all changes are consistent and accurate
- [ ] Checked formatting and markdown rendering
- [ ] Included ROADMAP.md in PR commit
- [ ] Mentioned roadmap update in PR description

---

## Examples

### Example 1: Completing a Major Feature

**Scenario**: You've completed the Combat Weapon System implementation.

**Updates Needed**:

1. **System Maturity Matrix**:
   ```markdown
   | Combat | ✅ Complete | 85% | ✅ Good | ⚠️ Moderate | Polish & optimization |
   ```

2. **Phase 3: Advanced Systems**:
   ```markdown
   **Combat System** (85% complete):
   - ✅ Weapon types and mounting system
   - ✅ Targeting and fire control
   - 🔄 Damage system and ship destruction (next milestone)
   ```

3. **Detailed Feature Roadmap → Combat System**:
   ```markdown
   #### Phase 1: Weapons & Targeting (Q4 2025)
   - [x] Weapon data asset framework
   - [x] Energy weapons (lasers, plasma)
   - [x] Projectile weapons (railguns, missiles)
   - [x] Weapon mounting system
   - [x] Targeting system (lock-on, lead indicators)
   - [x] Fire control computer
   ```

4. **Last Updated Date**:
   ```markdown
   **Last Updated**: November 11, 2025
   ```

### Example 2: Completing a Development Phase

**Scenario**: Phase 3 (Advanced Systems) is complete.

**Updates Needed**:

1. **Phase 3 Header**:
   ```markdown
   ### Phase 3: Advanced Systems (Completed) ✅
   **Timeline**: Q4 2025 - Q1 2026 (November 2025 - March 2026)
   **Status**: ✅ Complete
   ```

2. **Current Status**:
   - Move systems from "In Progress" to "Completed Systems"
   - Update version number if applicable

3. **Last Updated Date**:
   ```markdown
   **Last Updated**: March 15, 2026
   **Current Version**: 1.0.0
   ```

### Example 3: Starting a New System

**Scenario**: You've started implementing the Save System earlier than planned.

**Updates Needed**:

1. **Current Status**:
   ```markdown
   ### 🔄 In Progress (1.0.x)
   - 🔄 **Save System** - Player and world state persistence
   ```

2. **System Maturity Matrix**:
   ```markdown
   | Save System | 🔄 In Progress | 20% | ⚠️ Partial | ❌ Limited | Architecture complete |
   ```

3. **Save System Detailed Section**:
   ```markdown
   **Current Status**: 20% complete (Architecture phase started early)
   **Target Completion**: Q2 2026 (ahead of schedule)

   #### Phase 1: Architecture (Q1 2026)
   - [x] Save system architecture design
   - [x] Serialization strategy (JSON, binary)
   - [ ] Save file versioning
   ```

4. **Last Updated Date**:
   ```markdown
   **Last Updated**: November 11, 2025
   ```

### Example 4: NO UPDATE NEEDED

**Scenario**: You've fixed a minor bug in faction relationship calculations.

**Why No Update**:
- Bug fix doesn't change system completion status
- Faction System is already marked as "Complete"
- No new features or milestones achieved
- Timeline and phases remain unchanged

**Action**: Skip updating ROADMAP.md, but mention in CHANGELOG.md instead.

---

## Questions?

If you're unsure whether your PR requires a roadmap update:

1. **Ask in Your PR**: Mention in the PR description that you're unsure if a roadmap update is needed
2. **Ask Maintainers**: Maintainers can advise during PR review
3. **Err on the Side of Updating**: If truly significant, it's better to update than skip

---

## Related Documentation

- [ROADMAP.md](ROADMAP.md) - The project roadmap
- [CHANGELOG.md](CHANGELOG.md) - Detailed version history (for all changes)
- [CONTRIBUTING.md](CONTRIBUTING.md) - General contribution guidelines
- [.github/PULL_REQUEST_TEMPLATE.md](.github/PULL_REQUEST_TEMPLATE.md) - PR checklist

---

**Remember**: The roadmap is a high-level view of project direction. Keep it accurate, but don't stress over minor changes. Focus on significant milestones and system status updates.
