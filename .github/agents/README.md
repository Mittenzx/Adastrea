# Adastrea Custom Agents

This directory contains custom agent definitions for GitHub Copilot. Custom agents are specialized AI assistants with deep knowledge of specific aspects of the Adastrea project.

## Available Agents

### 1. Adastrea Developer Expert (`adastrea-developer.md`)

A comprehensive development agent with expertise in:
- All 9 core game systems (Spaceships, Stations, Factions, Personnel, AI, Trading, Player, Materials, Editor)
- Unreal Engine 5.5 and C++ best practices
- Blueprint integration patterns
- Data Asset architecture
- Project coding standards and conventions
- Testing and automation tools
- Documentation practices

**Use this agent for:**
- Implementing new game systems
- Integrating existing systems
- Creating Data Assets and Blueprint-callable APIs
- Writing comprehensive documentation
- Troubleshooting build/compilation issues
- Performance optimization
- Code reviews and refactoring

## How to Use Custom Agents

### In GitHub Copilot Chat

When using GitHub Copilot Chat in your IDE, you can reference custom agents to get specialized help:

```
@workspace /agent adastrea-developer How do I create a new faction with traits?
```

### In Pull Requests

Custom agents can be automatically invoked during PR reviews to provide specialized feedback based on the project's standards and patterns.

### Creating New Agents

To create a new custom agent for Adastrea:

1. Create a new `.md` file in this directory
2. Start with a clear definition of the agent's expertise
3. Include relevant project context and guidelines
4. Reference specific files, classes, and patterns from the codebase
5. Provide examples of common tasks the agent can help with
6. Keep the agent focused on a specific domain or set of related tasks

### Agent Guidelines

Good custom agents should:
- **Be specific** - Reference actual code, files, and patterns in Adastrea
- **Be accurate** - Reflect the current state of the implementation
- **Be helpful** - Provide actionable guidance, not generic advice
- **Be focused** - Have a clear area of expertise
- **Be maintained** - Update when systems change significantly

## Updating Agents

When major changes are made to Adastrea's architecture or systems, remember to update the relevant custom agents:

1. Review affected agent definitions
2. Update system descriptions and file references
3. Add new features or systems
4. Remove or deprecate obsolete information
5. Update examples to reflect current best practices

## Agent Maintenance Schedule

Custom agents should be reviewed and updated:
- When new major systems are added
- When existing systems undergo significant refactoring
- After major Unreal Engine version updates
- When coding standards or patterns change
- At least once per major release

## Questions or Suggestions

If you have ideas for new custom agents or improvements to existing ones, please:
1. Open an issue with the `documentation` label
2. Describe the agent's purpose and expertise area
3. Provide examples of tasks it would help with
4. Reference the systems or areas of code it covers

---

**Last Updated**: 2025-11-07
**Maintained by**: Adastrea Development Team
