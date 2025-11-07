# Using the Adastrea Custom Agent - Quick Guide

This guide shows you how to use the Adastrea Developer Expert custom agent to accelerate your development.

## What is the Custom Agent?

The Adastrea Developer Expert is a specialized AI assistant that knows:
- Every system in the Adastrea codebase
- All coding standards and design patterns
- The exact file locations and class names
- How systems integrate with each other
- Best practices specific to this project

Unlike general-purpose AI, this agent provides **specific, accurate answers** based on the actual Adastrea implementation.

## How to Use It

### In VS Code with GitHub Copilot

1. Open the Chat panel
   - Windows/Linux: `Ctrl+Shift+I` (may vary by VS Code version)
   - Mac: `Cmd+Shift+I` (may vary by VS Code version)
   - Alternative: View menu → Command Palette → "GitHub Copilot: Open Chat"
2. Type `@workspace` followed by your question
3. Reference the agent: `/agent adastrea-developer`

**Example:**
```
@workspace /agent adastrea-developer How do I create a new spaceship with custom stats?
```

### In GitHub Copilot Chat (Web/IDE)

Simply mention the agent in your question:
```
@adastrea-developer Explain the faction trait system
```

## Example Use Cases

### 1. Creating New Systems
```
@workspace /agent adastrea-developer 
I want to create a new quest system. Show me how to:
1. Create the Data Asset class
2. Expose it to Blueprints
3. Follow the project's design patterns
```

### 2. Understanding Existing Code
```
@workspace /agent adastrea-developer 
Explain how the faction relationship system works and which files I need to look at
```

### 3. Integrating Systems
```
@workspace /agent adastrea-developer 
How do I integrate the trading system with the faction reputation system?
```

### 4. Following Best Practices
```
@workspace /agent adastrea-developer 
I'm implementing a new personnel skill type. What's the correct way to do this following Adastrea patterns?
```

### 5. Troubleshooting
```
@workspace /agent adastrea-developer 
My Blueprint can't access my new C++ function. What am I missing?
```

### 6. Documentation
```
@workspace /agent adastrea-developer 
I added a new ship module type. What documentation files do I need to update?
```

## What Makes This Agent Different?

### Generic AI Assistant
❌ "You can create a data asset by inheriting from UDataAsset..."
❌ Generic Unreal Engine advice
❌ Doesn't know your project structure

### Adastrea Custom Agent
✅ "Create a new class inheriting from UPrimaryDataAsset in `Source/Adastrea/[System]/`, following the pattern used by `USpaceshipDataAsset`..."
✅ Specific to Adastrea's architecture
✅ References actual files and classes in the project
✅ Knows the project's coding standards and patterns

## Best Practices for Using the Agent

### 1. Be Specific
**Good:**
```
How do I add a new faction trait category beyond the existing 5?
```

**Better:**
```
I want to add a "Technological" trait category to FFactionTrait. 
What files do I modify and what's the pattern for adding trait categories?
```

### 2. Ask for Examples
```
Show me an example of creating a PersonnelDataAsset with all the required fields, 
similar to the Captain template in Assets/PersonnelTemplates/
```

### 3. Request Integration Guidance
```
I'm implementing weapon systems. How should this integrate with:
1. The spaceship system
2. The combat AI
3. The faction relationships
```

### 4. Verify Against Patterns
```
I wrote this Blueprint-callable function. Does it follow Adastrea's conventions?
[paste your code]
```

## Common Questions the Agent Can Answer

### Architecture Questions
- "What's the data flow for creating and using a faction?"
- "How do the AI, faction, and trading systems interact?"
- "Where should I put my new gameplay system?"

### Implementation Questions
- "What's the correct way to create a BlueprintNativeEvent function?"
- "How do I add a new enum for ship types?"
- "What's the pattern for creating YAML templates?"

### Integration Questions
- "How do I make my new system work with existing factions?"
- "What's the right way to query player reputation?"
- "How do I add a new stat category to spaceships?"

### Standards Questions
- "What naming convention should I use for this class?"
- "How should I document this new system?"
- "What metadata tags should I use on this UPROPERTY?"

## When NOT to Use the Custom Agent

The custom agent is specialized for Adastrea development. For these topics, use general tools:

- ❌ General Unreal Engine tutorials
- ❌ C++ language questions unrelated to Unreal
- ❌ Git commands and workflows (unless Adastrea-specific)
- ❌ Visual Studio setup and configuration
- ❌ Non-Adastrea Unreal Engine projects

## Keeping the Agent Updated

The custom agent is maintained as the project evolves. When you add major systems:

1. Update `.github/agents/adastrea-developer.md`
2. Add new system descriptions
3. Include new file locations and class names
4. Update examples and integration patterns

See `.github/agents/README.md` for maintenance guidelines.

## Tips for Maximum Productivity

### Combine with Workspace Context
```
@workspace /agent adastrea-developer 
Looking at SpaceshipDataAsset.h and SpaceshipDataAsset.cpp,
how would I add a new "Stealth" stat category?
```

### Ask for Complete Workflows
```
@workspace /agent adastrea-developer 
Walk me through the complete process of adding a new personnel role,
from C++ class to documentation to YAML template
```

### Request Code Review
```
@workspace /agent adastrea-developer 
Review this implementation. Does it follow Adastrea's patterns?
[paste your code]
```

### Get File Locations
```
@workspace /agent adastrea-developer 
Where are all the files I need to modify to add a new module type to space stations?
```

## Learning the Codebase

The custom agent is also a great learning tool:

### Explore Systems
```
Explain the personnel management system architecture and give me a tour of the key files
```

### Understand Patterns
```
Show me examples of the BlueprintNativeEvent pattern used throughout the codebase
```

### Find Examples
```
Where can I find good examples of trait systems to learn from?
```

## Support and Feedback

If the custom agent gives incorrect information or you have suggestions:

1. Check if the agent definition needs updating (`.github/agents/adastrea-developer.md`)
2. Open an issue with the `documentation` label
3. Include what you asked and what the correct answer should be

---

**Remember:** The custom agent is trained on the actual Adastrea codebase, so it provides specific, actionable guidance rather than generic advice. Use it to accelerate your development and maintain consistency across the project!
