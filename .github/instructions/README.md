# Path-Scoped Copilot Instructions

This directory contains path-scoped custom instructions for GitHub Copilot. These files provide specialized guidance for specific parts of the codebase.

## What Are Path-Scoped Instructions?

Path-scoped instructions allow you to provide targeted guidance to GitHub Copilot for specific directories or file patterns. This is useful for:
- Enforcing coding standards for specific subsystems
- Providing context about complex modules
- Documenting integration patterns for specific code areas
- Sharing architectural decisions for particular components

## File Format

Each instruction file must:
1. End with `.instructions.md` extension
2. Include an `applyTo` comment at the top specifying which files it applies to
3. Use glob patterns to match file paths

### Example

```markdown
# My Subsystem Instructions

<!-- applyTo: Source/MyModule/**/*.cpp, Source/MyModule/**/*.h -->

...instructions here...
```

## Current Path-Scoped Instructions

| File | Applies To | Purpose |
|------|-----------|---------|
| `ai-system.instructions.md` | `Source/Adastrea/AI/**` | AI system architecture and BlueprintNativeEvent patterns |
| `data-assets.instructions.md` | `**/*DataAsset.cpp`, `**/*DataAsset.h` | Data Asset design patterns and best practices |

## How Copilot Uses These Files

When you're working on a file that matches an `applyTo` pattern:
1. GitHub Copilot automatically includes the relevant instruction file in its context
2. The instructions influence code generation, completion, and suggestions
3. Path-scoped instructions take precedence over repository-wide instructions

## Priority Order

GitHub Copilot applies instructions in this order (most specific first):
1. **Path-scoped instructions** (this directory)
2. **Repository instructions** (`.github/copilot-instructions.md`)
3. **Organization instructions** (if configured)
4. **User instructions** (personal settings)

## Best Practices

### Keep Instructions Focused
- Target specific technical concerns for the code area
- Avoid repeating general repository standards
- Focus on what's unique about this subsystem

### Use Clear Patterns
- Be specific with glob patterns
- Test patterns match the intended files
- Consider subdirectories carefully

### Document Integration Points
- Explain how this code integrates with other systems
- Note dependencies and coupling
- Highlight common pitfalls

### Maintain Consistency
- Follow the same structure across instruction files
- Use similar headings and organization
- Keep tone and style consistent

## Creating New Path-Scoped Instructions

To add instructions for a new code area:

1. Create a new `.instructions.md` file in this directory
2. Add the `applyTo` comment with appropriate patterns
3. Write clear, focused instructions for that code area
4. Update this README with the new file
5. Test that Copilot picks up the instructions when editing matching files

### Template

```markdown
# [Subsystem Name] Custom Instructions

<!-- applyTo: [glob pattern], [glob pattern] -->

## Overview
Brief description of this code area

## Key Classes
List main classes and their purposes

## Coding Standards
Specific standards for this subsystem

## Integration Points
How this integrates with other systems

## Common Patterns
Code patterns specific to this area

## Testing
Testing requirements for this subsystem

## Common Pitfalls
Things to avoid in this code area

---
Additional context or notes
```

## Related Documentation

- `.github/copilot-instructions.md` - Repository-wide Copilot instructions
- `.github/copilot-knowledge.md` - Repository structure knowledge base
- `CODE_STYLE.md` - General coding standards
- `ARCHITECTURE.md` - Overall system architecture

## References

- [GitHub Docs: Path-scoped custom instructions](https://github.blog/changelog/2025-09-03-copilot-code-review-path-scoped-custom-instruction-file-support/)
- [GitHub Docs: Adding repository custom instructions](https://docs.github.com/en/copilot/how-tos/configure-custom-instructions/add-repository-instructions)

---

**Note**: Path-scoped instructions are a powerful way to provide context-specific guidance to Copilot without cluttering the main instructions file. Use them to encode domain-specific knowledge and architectural decisions for complex subsystems.
