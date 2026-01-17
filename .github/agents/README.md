# Adastrea Custom Agent

This directory contains the custom agent definition for GitHub Copilot. The Adastrea Expert is a comprehensive AI assistant with deep knowledge of all aspects of the Adastrea project.

## Available Agent

### Adastrea Expert (`adastrea-expert.agent.md`)

A comprehensive development agent with expertise in:
- **All 15+ Game Systems**: Spaceships, Stations, Factions, Personnel, AI, Trading, Combat, Navigation, Quest, and more
- **Unreal Engine 5.6**: C++ and Blueprint development, optimization, and best practices
- **MCP Automation**: Unreal Engine MCP (Model Context Protocol) for AI-driven workflows
- **Trade Simulator MVP**: Current development focus and priorities (Dec 2025 - Mar 2026)
- **Anti-Patterns**: Lessons learned from past development phases
- **Data Asset Architecture**: PRIMARY design pattern for Adastrea content
- **Project Standards**: Coding conventions, testing, and documentation
- **Blueprint Documentation**: Image-based documentation standard

**This agent combines all previous agents into one comprehensive expert that:**
- Understands the full project context from day one
- Knows the current Trade Simulator MVP mandate
- Has all instruction files integrated
- Provides specific, actionable Adastrea guidance (not generic advice)
- Advocates for simplicity, validation, and rapid iteration

## How to Use the Custom Agent

### In GitHub Copilot Chat

When using GitHub Copilot Chat in your IDE, you can reference the custom agent to get specialized help:

```
@workspace /agent adastrea-expert How do I create a new faction with traits?
@workspace /agent adastrea-expert Help me set up a trading station for the MVP
@workspace /agent adastrea-expert What should I defer until after MVP validation?
```

### In Pull Requests

Custom agents can be automatically invoked during PR reviews to provide specialized feedback based on the project's standards and patterns.

### Common Tasks the Agent Can Help With

**MVP-Critical Tasks (High Priority):**
- Simplifying trading system for MVP
- Creating trading UI/UX
- Implementing cargo management
- Setting up economy simulation
- Designing ship progression
- Station docking mechanics
- MCP automation for content

**Architecture & Technical:**
- Implementing Data Asset systems
- Creating Blueprint-callable functions
- Code reviews and refactoring
- Troubleshooting build issues
- Performance optimization

**Guidance & Standards:**
- MVP scope evaluation
- Anti-pattern prevention
- Blueprint documentation
- Testing strategies
- Integration patterns

## Updating the Agent

When major changes are made to Adastrea's architecture or systems, remember to update the agent:

1. Review `adastrea-expert.agent.md`
2. Update system descriptions and file references
3. Add new features or systems
4. Remove or deprecate obsolete information
5. Update examples to reflect current best practices
6. Ensure all `.github/instructions/*.md` files are still referenced

## Agent Maintenance Schedule

The agent should be reviewed and updated:
- When new major systems are added
- When existing systems undergo significant refactoring
- After major Unreal Engine version updates
- When coding standards or patterns change
- When development priorities shift (like the Trade Simulator MVP pivot)
- At least once per major release

## Questions or Suggestions

If you have ideas for improvements to the agent, please:
1. Open an issue with the `documentation` label
2. Describe what could be improved or clarified
3. Provide examples of where the agent could help better
4. Reference the systems or areas of code it covers

---

**Last Updated**: 2026-01-17  
**Agent Version**: Consolidated Expert v1.0  
**Maintained by**: Adastrea Development Team
