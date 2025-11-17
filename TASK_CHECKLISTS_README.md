# Task Checklists Guide

This document explains the new task checklist structure for the Adastrea project.

## Overview

The original `NEXT_2_WEEKS_TASKS.md` has been split into two specialized checklists:

1. **COPILOT_TASKS.md** - Optimized for AI agents
2. **MITTENZX_TASKS.md** - Optimized for human team members

## File Descriptions

### COPILOT_TASKS.md (17KB)
**Purpose**: Technical implementation tasks for AI agents

**Optimized For**:
- AI parsing and automation
- Structured, concise format
- Clear acceptance criteria
- Technical specifications

**Contains**:
- P0 (Critical) and P1 (High Priority) technical tasks
- Command-line examples and test scripts
- Performance metrics and benchmarks
- Dependencies and technical requirements
- Testing protocols and validation commands

**Best For**:
- Copilot AI agent task execution
- Automated testing and validation
- Performance optimization work
- Plugin development
- Technical implementation

### MITTENZX_TASKS.md (15KB)
**Purpose**: Visual and content creation tasks for human team members

**Optimized For**:
- New team member onboarding
- Detailed context and explanations
- Step-by-step guidance
- Learning resources

**Contains**:
- P0 (Critical) and P1 (High Priority) visual/content tasks
- "Why This Matters" sections for each task
- Detailed walkthroughs with examples
- Learning resources and documentation links
- Troubleshooting tips and help sections
- Encouragement and success criteria

**Best For**:
- Visual effects creation (combat VFX, engine effects)
- Content creation (achievements, tutorials, audio)
- Post-processing and visual polish
- Alpha build preparation (manual testing)
- Documentation creation

## Original File

**NEXT_2_WEEKS_TASKS.md** remains as a comprehensive reference containing:
- Complete context and planning information
- Detailed task descriptions for both roles
- Sprint planning and risk mitigation
- Daily standup templates
- Post-sprint preview

## How to Use

### For Copilot (AI Agent):
```bash
# Read your task list
cat COPILOT_TASKS.md

# Execute tasks with clear acceptance criteria
# Run automated tests
python3 Scripts/AutomationRunner.py --benchmark
```

### For Mittenzx (Human Team Member):
1. Open `MITTENZX_TASKS.md`
2. Read "Getting Started Resources" section
3. Follow tasks week by week
4. Use detailed examples and guidance
5. Refer to documentation links as needed

### For Planning/Reference:
- Use `NEXT_2_WEEKS_TASKS.md` for comprehensive overview
- Review success criteria and sprint goals
- Check task assignment summary
- Reference daily standup templates

## Task Priority System

All three files use the same priority system:

- **P0 (Critical)** - Must complete for alpha build
- **P1 (High Priority)** - Should complete, minimum viable scope
- **P2 (Medium Priority)** - Nice to have, if time permits

## Quick Reference

| Need | Use This File |
|------|---------------|
| AI agent tasks | COPILOT_TASKS.md |
| Human visual/content tasks | MITTENZX_TASKS.md |
| Complete planning overview | NEXT_2_WEEKS_TASKS.md |
| Sprint context and goals | NEXT_2_WEEKS_TASKS.md |
| Technical implementation | COPILOT_TASKS.md |
| Content creation guidance | MITTENZX_TASKS.md |

## Updates

When updating tasks:
1. Update the relevant specialized file (COPILOT_TASKS.md or MITTENZX_TASKS.md)
2. Consider updating NEXT_2_WEEKS_TASKS.md if planning context changes
3. Keep all three files in sync for shared tasks (P0.6 Alpha Build Preparation)

## Feedback

If you find these checklists helpful or have suggestions for improvement:
- Create a GitHub issue
- Update this README with your findings
- Share best practices with the team

---

**Document Version**: 1.0  
**Created**: November 17, 2025  
**Last Updated**: November 17, 2025
