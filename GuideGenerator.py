#!/usr/bin/env python3
"""
Adastrea - Guide Generator Script

This script generates markdown documentation and guide files for various game systems.
Can be run from command line or imported in Unreal Editor Python environment.

Features:
- Generate system setup guides
- Generate workflow documentation
- Generate API reference guides
- Generate quick reference guides
- Interactive and batch modes

Usage:
    # Command line - Interactive mode
    python GuideGenerator.py
    
    # Command line - Batch mode
    python GuideGenerator.py --type system --name "MySystem" --output Assets/
    
    # In Unreal Editor Python console
    import GuideGenerator
    GuideGenerator.generate_system_guide("MySystem")
"""

import os
import sys
from pathlib import Path
from typing import Dict, Any, List, Optional
from datetime import datetime


class GuideGenerator:
    """Generator for creating markdown documentation files for Adastrea game systems"""
    
    def __init__(self, project_root: Optional[Path] = None):
        """
        Initialize the guide generator
        
        Args:
            project_root: Path to project root. If None, uses current working directory
        """
        self.project_root = project_root or Path.cwd()
        self.docs_dir = self.project_root / "Assets"
        
        # Ensure docs directory exists
        self.docs_dir.mkdir(parents=True, exist_ok=True)
    
    def generate_system_guide(self, system_name: str, output_dir: Optional[Path] = None) -> Path:
        """
        Generate a comprehensive system guide
        
        Args:
            system_name: Name of the system
            output_dir: Output directory (defaults to Assets/)
            
        Returns:
            Path to the created guide file
        """
        if output_dir is None:
            output_dir = self.docs_dir
        output_dir.mkdir(parents=True, exist_ok=True)
        
        clean_name = system_name.replace(" ", "")
        filename = f"{clean_name}SystemGuide.md"
        filepath = output_dir / filename
        
        template_content = f"""# {system_name} System - Complete Guide

**Last Updated**: {datetime.now().strftime('%Y-%m-%d')}  
**System Version**: 1.0  
**Author**: Adastrea Development Team

---

## Table of Contents

1. [Overview](#overview)
2. [Core Concepts](#core-concepts)
3. [Architecture](#architecture)
4. [Data Assets](#data-assets)
5. [Blueprint Integration](#blueprint-integration)
6. [C++ API Reference](#c-api-reference)
7. [Workflow](#workflow)
8. [Examples](#examples)
9. [Best Practices](#best-practices)
10. [Troubleshooting](#troubleshooting)

---

## Overview

### What is the {system_name} System?

[Provide a high-level description of what this system does, its purpose in the game, and why it exists.]

### Key Features

- **Feature 1**: [Description]
- **Feature 2**: [Description]
- **Feature 3**: [Description]
- **Feature 4**: [Description]

### When to Use This System

Use the {system_name} system when you need to:

- [Use case 1]
- [Use case 2]
- [Use case 3]

---

## Core Concepts

### Fundamental Principles

1. **Data-Driven Design**: All configuration through Data Assets
2. **Blueprint-First**: Full Blueprint exposure for designers
3. **Modular Architecture**: Independent, reusable components
4. **Performance**: Optimized for real-time gameplay

### Key Components

#### Component 1: [Name]

[Describe the component, its purpose, and how it fits into the system]

#### Component 2: [Name]

[Describe the component, its purpose, and how it fits into the system]

#### Component 3: [Name]

[Describe the component, its purpose, and how it fits into the system]

---

## Architecture

### System Diagram

```
[Add system architecture diagram or flowchart here]

┌─────────────────────┐
│   Data Assets       │
│   Configuration     │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│   C++ Core Logic    │
│   Processing        │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│   Blueprint API     │
│   Gameplay Logic    │
└─────────────────────┘
```

### Class Hierarchy

```
U{system_name}DataAsset (Base Data Asset)
├── U{system_name}Component (Actor Component)
├── A{system_name}Actor (Actor Class)
└── U{system_name}Subsystem (Game Instance Subsystem)
```

### File Organization

```
Source/Adastrea/{system_name}/
├── {system_name}DataAsset.h/.cpp
├── {system_name}Component.h/.cpp
├── {system_name}Actor.h/.cpp
└── {system_name}Subsystem.h/.cpp

Content/{system_name}/
├── Blueprints/
│   ├── BP_{system_name}_Base
│   └── BP_{system_name}_Example
└── DataAssets/
    └── DA_{system_name}_Example
```

---

## Data Assets

### Creating a Data Asset

1. **In Content Browser**:
   - Right-click in desired folder
   - Select `Miscellaneous → Data Asset`
   - Choose `{system_name}DataAsset`
   - Name it `DA_{system_name}_YourName`

2. **Configure Properties**:
   - Open the Data Asset
   - Fill in required fields
   - Set optional parameters
   - Save the asset

### Data Asset Properties

#### Basic Info

| Property | Type | Description | Default |
|----------|------|-------------|---------|
| DisplayName | FText | Human-readable name | "" |
| Description | FText | Detailed description | "" |
| UniqueID | FName | Unique identifier | None |

#### Configuration

| Property | Type | Description | Default |
|----------|------|-------------|---------|
| [Property1] | [Type] | [Description] | [Default] |
| [Property2] | [Type] | [Description] | [Default] |

### YAML Template Reference

For quick setup, see YAML templates in `Assets/{system_name}Templates/`

Example:
```yaml
# {system_name} Template Example
BasicInfo:
  Name: "Example"
  Description: "Example configuration"
  
Properties:
  Value1: 100
  Value2: "Setting"
```

---

## Blueprint Integration

### Setting Up Blueprints

#### Step 1: Create Blueprint Class

1. Right-click in Content Browser
2. Select `Blueprint Class`
3. Choose parent: `{system_name}Actor` or `Actor` with `{system_name}Component`
4. Name: `BP_{system_name}_YourName`

#### Step 2: Add Components

In the Blueprint editor:
- Add `{system_name}Component` to Components panel
- Configure component properties
- Reference your Data Asset

#### Step 3: Implement Logic

Common Blueprint nodes:

- `Initialize {system_name}`: Set up the system
- `Update {system_name}`: Update system state
- `Get {system_name} Data`: Retrieve data from asset
- `On {system_name} Event`: React to system events

### Blueprint Examples

#### Example 1: Basic Setup

```
Event BeginPlay
└─ Initialize {system_name}
   ├─ Data Asset: DA_{system_name}_Example
   └─ On Success
      └─ Print String: "System initialized"
```

#### Example 2: Runtime Updates

```
Event Tick
└─ Update {system_name}
   ├─ Delta Time
   └─ Get Updated Values
      └─ [Use values in gameplay]
```

---

## C++ API Reference

### Core Classes

#### U{system_name}DataAsset

```cpp
UCLASS(BlueprintType)
class ADASTREA_API U{system_name}DataAsset : public UDataAsset
{{
    GENERATED_BODY()

public:
    // Initialize the data asset
    UFUNCTION(BlueprintCallable, Category="{system_name}")
    void Initialize();
    
    // Get configuration value
    UFUNCTION(BlueprintPure, Category="{system_name}")
    float GetConfigValue() const;
}};
```

#### U{system_name}Component

```cpp
UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class ADASTREA_API U{system_name}Component : public UActorComponent
{{
    GENERATED_BODY()

public:
    // Tick function
    virtual void TickComponent(float DeltaTime, ...) override;
    
    UFUNCTION(BlueprintCallable, Category="{system_name}")
    void UpdateSystem(float DeltaTime);
}};
```

### Key Functions

#### Initialization

```cpp
// Initialize with data asset
void Initialize(U{system_name}DataAsset* DataAsset);
```

#### Update

```cpp
// Update system state
void Update(float DeltaTime);
```

#### Queries

```cpp
// Get current state
F{system_name}State GetCurrentState() const;

// Check if active
bool IsActive() const;
```

---

## Workflow

### Designer Workflow

1. **Create Data Asset**
   - Use Content Browser or YAML template
   - Configure all properties
   - Save asset

2. **Create Blueprint**
   - Derive from system base class
   - Add required components
   - Reference Data Asset

3. **Implement Gameplay**
   - Use Blueprint nodes
   - Handle events
   - Test in editor

4. **Test and Iterate**
   - Play in editor
   - Adjust values
   - Refine behavior

### Programmer Workflow

1. **Extend System**
   - Subclass base classes
   - Add new properties/methods
   - Expose to Blueprints

2. **Implement Logic**
   - Write C++ implementation
   - Add validation
   - Optimize performance

3. **Document Changes**
   - Update API docs
   - Add examples
   - Update this guide

---

## Examples

### Example 1: Basic Usage

**Goal**: Create a simple {system_name} setup

**Steps**:
1. Create Data Asset `DA_{system_name}_Basic`
2. Set properties: [list specific properties]
3. Create Blueprint `BP_{system_name}_Basic`
4. Add to level and test

**Result**: [Expected outcome]

### Example 2: Advanced Configuration

**Goal**: Use advanced features

**Steps**:
1. Create Data Asset with custom settings
2. Implement Blueprint logic for [specific feature]
3. Add event handlers
4. Test edge cases

**Result**: [Expected outcome]

### Example 3: Integration with Other Systems

**Goal**: Connect {system_name} with [Other System]

**Steps**:
1. Set up both systems
2. Create communication interface
3. Handle cross-system events
4. Verify integration

**Result**: [Expected outcome]

---

## Best Practices

### Do's ✓

- **Use Data Assets**: Keep all configuration in Data Assets, not hardcoded
- **Name Consistently**: Follow naming conventions (DA_, BP_, etc.)
- **Document Changes**: Add comments to complex logic
- **Test Thoroughly**: Test in editor before committing
- **Optimize**: Profile performance-critical code

### Don'ts ✗

- **Don't Hardcode**: Avoid magic numbers and hardcoded strings
- **Don't Skip Validation**: Always validate input data
- **Don't Ignore Errors**: Handle all error cases
- **Don't Overcomplicate**: Keep it simple when possible
- **Don't Break Existing Assets**: Maintain backward compatibility

### Performance Tips

- Cache frequently accessed data
- Use object pooling for spawned actors
- Minimize Blueprint tick usage
- Profile before optimizing
- Use LOD for distant objects

### Common Patterns

#### Pattern 1: Lazy Initialization

```cpp
if (!bInitialized) {{
    Initialize();
    bInitialized = true;
}}
```

#### Pattern 2: Data Asset Reference

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly)
U{system_name}DataAsset* DataAsset;
```

---

## Troubleshooting

### Common Issues

#### Issue: System not initializing

**Symptoms**: No response when calling Initialize()

**Solutions**:
- Verify Data Asset is set
- Check logs for error messages
- Ensure BeginPlay is called
- Validate all required properties

#### Issue: Performance problems

**Symptoms**: Frame rate drops, stuttering

**Solutions**:
- Profile with Unreal Insights
- Reduce tick frequency
- Use async operations
- Optimize Blueprint logic

#### Issue: Data not saving

**Symptoms**: Changes lost after editor restart

**Solutions**:
- Ensure Data Asset is saved
- Check Content Browser for unsaved assets
- Verify file permissions
- Use source control

### Debug Commands

Enable debug visualization:
```
{system_name}.Debug 1
```

Show detailed logs:
```
{system_name}.LogLevel Verbose
```

### Getting Help

- Check [ARCHITECTURE.md](../ARCHITECTURE.md) for system design
- Review [CONTRIBUTING.md](../CONTRIBUTING.md) for contribution guidelines
- See other system guides in Assets/ for patterns
- Ask in team Discord/Slack

---

## Additional Resources

### Related Documentation

- [{system_name}DesignerWorkflow.md](./{system_name}DesignerWorkflow.md) - Step-by-step workflow
- [{system_name}APIReference.md](./{system_name}APIReference.md) - Complete API docs
- [{system_name}Templates.md](./{system_name}Templates.md) - YAML templates

### External References

- [Unreal Engine Documentation](https://docs.unrealengine.com/)
- [Data Assets in UE5](https://docs.unrealengine.com/5.0/en-US/data-assets-in-unreal-engine/)
- [Blueprint Best Practices](https://docs.unrealengine.com/5.0/en-US/blueprint-best-practices-in-unreal-engine/)

### Video Tutorials

- [Add YouTube/training links here]

---

**Questions or feedback?** Contact the Adastrea development team or submit an issue on GitHub.

---

*This guide is maintained by the Adastrea development team and updated regularly. Last update: {datetime.now().strftime('%Y-%m-%d')}*
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(template_content)
        
        print(f"✓ Created system guide: {filepath}")
        return filepath
    
    def generate_workflow_guide(self, system_name: str, output_dir: Optional[Path] = None) -> Path:
        """
        Generate a designer workflow guide
        
        Args:
            system_name: Name of the system
            output_dir: Output directory (defaults to Assets/)
            
        Returns:
            Path to the created guide file
        """
        if output_dir is None:
            output_dir = self.docs_dir
        output_dir.mkdir(parents=True, exist_ok=True)
        
        clean_name = system_name.replace(" ", "")
        filename = f"{clean_name}DesignerWorkflow.md"
        filepath = output_dir / filename
        
        template_content = f"""# {system_name} - Designer Workflow Guide

**Last Updated**: {datetime.now().strftime('%Y-%m-%d')}  
**Target Audience**: Game Designers, Content Creators  
**Difficulty**: Beginner to Intermediate

---

## Introduction

This guide provides a step-by-step workflow for designers to create and configure {system_name} content without writing code.

### What You'll Learn

- How to create {system_name} Data Assets
- How to set up {system_name} in Blueprints
- How to test and iterate on your designs
- Common patterns and best practices

### Prerequisites

- Basic Unreal Engine knowledge
- Familiarity with Content Browser
- Understanding of Blueprint basics

**Estimated Time**: 30-45 minutes

---

## Step-by-Step Workflow

### Phase 1: Planning

#### 1.1 Define Your Goals

Before creating assets, answer these questions:

- What is the purpose of this {system_name} instance?
- What behavior should it have?
- How will it interact with other systems?
- What are the key parameters?

**Example Goals**:
- Create a [specific example] for [use case]
- Configure [feature] with [specific behavior]

#### 1.2 Choose a Template

Review existing templates in `Assets/{system_name}Templates/`:

- `Template1.yaml` - For [use case]
- `Template2.yaml` - For [use case]
- `Template3.yaml` - For [use case]

**Tip**: Start with the template closest to your needs.

---

### Phase 2: Create Data Asset

#### 2.1 From YAML Template (Recommended)

If using a YAML template:

1. Copy template file from `Assets/{system_name}Templates/`
2. Rename to match your asset name
3. Edit values in your text editor
4. Use TemplateGenerator.py to convert to Data Asset (optional)

**Command**:
```bash
python TemplateGenerator.py --type {system_name.lower()} --name "YourName"
```

#### 2.2 From Content Browser

If creating directly in Unreal:

1. **Open Content Browser**
2. **Navigate** to `Content/DataAssets/{system_name}/`
3. **Right-Click** → `Miscellaneous → Data Asset`
4. **Select**: `{system_name}DataAsset`
5. **Name**: `DA_{system_name}_YourName`
6. **Double-Click** to open

#### 2.3 Configure Properties

Fill in all required fields:

**Basic Info**:
- DisplayName: User-facing name
- Description: What this asset does
- UniqueID: System identifier (auto-generated or custom)

**Primary Settings**:
- [Property1]: [Purpose and recommended values]
- [Property2]: [Purpose and recommended values]
- [Property3]: [Purpose and recommended values]

**Advanced Settings** (Optional):
- [Advanced Property1]: [When to use]
- [Advanced Property2]: [When to use]

**Validation Checklist**:
- [ ] All required fields filled
- [ ] Values are within valid ranges
- [ ] UniqueID is truly unique
- [ ] Description is clear and complete

---

### Phase 3: Create Blueprint

#### 3.1 Create Blueprint Class

1. **Right-Click** in Content Browser
2. **Select**: `Blueprint Class`
3. **Choose Parent**: 
   - Option A: `{system_name}Actor` (if standalone)
   - Option B: `Actor` with components (if custom)
4. **Name**: `BP_{system_name}_YourName`
5. **Open** the Blueprint

#### 3.2 Add Components

In Blueprint Editor:

1. **Click** `Add Component` button
2. **Search** for `{system_name}Component`
3. **Add** the component
4. **Select** the component in Components panel
5. **In Details Panel**:
   - Set `Data Asset` → Your `DA_{system_name}_YourName`
   - Configure component-specific settings

#### 3.3 Implement Event Graph

Common events to implement:

**Event BeginPlay**:
```
Event BeginPlay
├─ Initialize {system_name}
│  └─ On Success
│     └─ [Your initialization logic]
└─ On Failure
   └─ Print String: "Failed to initialize"
```

**Event Tick** (if needed):
```
Event Tick
└─ Update {system_name}
   ├─ Delta Seconds
   └─ [Process updated state]
```

**Custom Events**:
```
[Your Custom Event]
├─ Do {system_name} Action
└─ Handle Result
```

---

### Phase 4: Testing

#### 4.1 Place in Level

1. **Drag** your Blueprint into the level
2. **Position** appropriately
3. **Configure** instance-specific settings in Details panel

#### 4.2 Test in Editor

1. **Click** Play (Alt+P)
2. **Observe** behavior
3. **Check** Output Log for messages
4. **Verify** expected functionality

**Common Test Cases**:
- [ ] System initializes without errors
- [ ] Expected behavior occurs
- [ ] Performance is acceptable
- [ ] Interactions with other systems work

#### 4.3 Debug Issues

If something doesn't work:

1. **Check Output Log**:
   - Look for error messages
   - Filter by `{system_name}`

2. **Verify Setup**:
   - Is Data Asset set correctly?
   - Are all required properties filled?
   - Is component added to Blueprint?

3. **Use Debug Tools**:
   - Enable debug visualization: `{system_name}.Debug 1`
   - Add print statements to Blueprint
   - Use breakpoints in Blueprint debugger

---

### Phase 5: Iteration

#### 5.1 Adjust Values

Based on testing:

1. **Open** Data Asset
2. **Modify** properties
3. **Save** changes
4. **Test** again

**Tip**: Keep a notes file tracking what changes improve gameplay.

#### 5.2 Variant Creation

To create variations:

1. **Duplicate** existing Data Asset
2. **Rename** appropriately
3. **Modify** specific values
4. **Document** differences

**Example Variants**:
- `DA_{system_name}_Fast` - High speed, low durability
- `DA_{system_name}_Tank` - Low speed, high durability
- `DA_{system_name}_Balanced` - Medium everything

#### 5.3 Organize Assets

Keep your content organized:

```
Content/
└── {system_name}/
    ├── Blueprints/
    │   ├── BP_{system_name}_Type1
    │   ├── BP_{system_name}_Type2
    │   └── BP_{system_name}_Type3
    └── DataAssets/
        ├── DA_{system_name}_Type1
        ├── DA_{system_name}_Type2
        └── DA_{system_name}_Type3
```

---

## Common Workflows

### Workflow 1: Simple Configuration

**Use Case**: Basic setup with default behavior

**Steps**:
1. Use default template
2. Change only name and basic stats
3. Create simple Blueprint with just the component
4. Place in level
5. Done!

**Time**: 5-10 minutes

---

### Workflow 2: Custom Behavior

**Use Case**: Unique behavior requiring Blueprint logic

**Steps**:
1. Start with closest template
2. Modify Data Asset for your needs
3. Create Blueprint with custom event graph
4. Implement special logic
5. Test thoroughly

**Time**: 30-60 minutes

---

### Workflow 3: System Integration

**Use Case**: Integrate with other game systems

**Steps**:
1. Create Data Asset for {system_name}
2. Create Data Assets for related systems
3. Create Blueprint that connects both
4. Implement communication logic
5. Test interactions

**Time**: 1-2 hours

---

## Best Practices

### Naming Conventions

✓ **Good Names**:
- `DA_{system_name}_MercenaryFighter`
- `BP_{system_name}_FastScout`
- `DA_{system_name}_HeavyFreighter`

✗ **Bad Names**:
- `DataAsset_12`
- `NewBlueprint`
- `Test123`

### Organization

- **Group by Type**: Keep similar assets together
- **Use Folders**: Don't dump everything in root
- **Version Control**: Use meaningful commit messages

### Documentation

- Add descriptions to all Data Assets
- Comment complex Blueprint logic
- Keep a design doc for major features
- Update templates when you find better patterns

### Collaboration

- Share templates with team
- Document unique configurations
- Ask for feedback before finalizing
- Test with multiplayer if relevant

---

## Troubleshooting

### Problem: Can't find Data Asset class

**Solution**: 
1. Ensure project is compiled
2. Check class is properly defined in C++
3. Restart Unreal Editor if needed

### Problem: Changes don't appear in game

**Solution**:
1. Save Data Asset
2. Compile Blueprint
3. Restart PIE session
4. Check logs for errors

### Problem: Performance issues

**Solution**:
1. Disable Tick if not needed
2. Use timers instead of constant updates
3. Profile with Unreal Insights
4. Reduce complexity

---

## Next Steps

After mastering this workflow:

1. **Read**: [{system_name}SystemGuide.md](./{system_name}SystemGuide.md) for deeper understanding
2. **Explore**: Advanced features and customization
3. **Create**: Your own templates and patterns
4. **Share**: Best practices with the team

---

## Quick Reference Card

### Creating Data Asset
1. Right-click → Miscellaneous → Data Asset
2. Choose `{system_name}DataAsset`
3. Name `DA_{system_name}_Name`
4. Configure properties
5. Save

### Creating Blueprint
1. Right-click → Blueprint Class
2. Choose `{system_name}Actor`
3. Name `BP_{system_name}_Name`
4. Add components
5. Set Data Asset reference
6. Compile & Save

### Testing
1. Drag Blueprint to level
2. Play (Alt+P)
3. Check Output Log
4. Iterate and improve

---

**Need Help?** Check [{system_name}SystemGuide.md](./{system_name}SystemGuide.md) or ask the team!

---

*This workflow guide is maintained by the Adastrea design team. Last update: {datetime.now().strftime('%Y-%m-%d')}*
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(template_content)
        
        print(f"✓ Created workflow guide: {filepath}")
        return filepath
    
    def generate_quick_reference(self, system_name: str, output_dir: Optional[Path] = None) -> Path:
        """
        Generate a quick reference guide
        
        Args:
            system_name: Name of the system
            output_dir: Output directory (defaults to Assets/)
            
        Returns:
            Path to the created guide file
        """
        if output_dir is None:
            output_dir = self.docs_dir
        output_dir.mkdir(parents=True, exist_ok=True)
        
        clean_name = system_name.replace(" ", "")
        filename = f"{clean_name}QuickReference.md"
        filepath = output_dir / filename
        
        template_content = f"""# {system_name} - Quick Reference

**Version**: 1.0 | **Last Updated**: {datetime.now().strftime('%Y-%m-%d')}

---

## At a Glance

**Purpose**: [One sentence description of what this system does]

**Key Classes**:
- `U{system_name}DataAsset` - Configuration data
- `U{system_name}Component` - Runtime component
- `A{system_name}Actor` - Actor class

**Common Use Cases**:
- [Use case 1]
- [Use case 2]
- [Use case 3]

---

## Quick Start (5 Minutes)

### 1. Create Data Asset
```
Right-click → Miscellaneous → Data Asset → {system_name}DataAsset
Name: DA_{system_name}_MyName
```

### 2. Create Blueprint
```
Right-click → Blueprint Class → {system_name}Actor
Name: BP_{system_name}_MyName
```

### 3. Configure
```
Add {system_name}Component
Set Data Asset reference
Compile and Save
```

### 4. Test
```
Drag to level
Press Play (Alt+P)
Check Output Log
```

---

## Essential Properties

### Data Asset Properties

| Property | Type | Range/Values | Description |
|----------|------|--------------|-------------|
| DisplayName | Text | Any | User-facing name |
| [Property1] | [Type] | [Range] | [Description] |
| [Property2] | [Type] | [Range] | [Description] |
| [Property3] | [Type] | [Range] | [Description] |

### Component Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| DataAsset | DataAsset | None | Reference to configuration |
| bAutoActivate | Bool | true | Start active |
| [Setting1] | [Type] | [Default] | [Description] |

---

## Common Blueprint Nodes

### Initialization
```
Initialize {system_name}
├─ Input: DataAsset (optional)
└─ Output: Success (bool)
```

### Updates
```
Update {system_name}
├─ Input: Delta Time (float)
└─ Output: Updated State
```

### Queries
```
Get {system_name} Value
├─ Input: Property Name
└─ Output: Value (variant)
```

```
Is {system_name} Active
└─ Output: Active (bool)
```

### Events
```
On {system_name} Changed
└─ Output: New Value
```

---

## YAML Template Structure

```yaml
# Minimal {system_name} Template
BasicInfo:
  Name: "Example"
  Description: "Example configuration"
  ID: "Example_01"

Properties:
  Value1: 100
  Value2: true
  Value3: "Setting"

Advanced:
  Feature1: 1.0
  Feature2: false
```

---

## C++ Quick Reference

### Include
```cpp
#include "{system_name}/{system_name}DataAsset.h"
#include "{system_name}/{system_name}Component.h"
```

### Create Data Asset Reference
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="{system_name}")
U{system_name}DataAsset* MyDataAsset;
```

### Initialize
```cpp
if (MyDataAsset)
{{
    MyDataAsset->Initialize();
}}
```

### Get Component
```cpp
U{system_name}Component* Comp = 
    GetOwner()->FindComponentByClass<U{system_name}Component>();
```

---

## Console Commands

### Debug
```
{system_name}.Debug 1          # Enable debug visualization
{system_name}.Debug 0          # Disable debug visualization
```

### Logging
```
{system_name}.LogLevel Verbose # Detailed logs
{system_name}.LogLevel Log     # Normal logs
{system_name}.LogLevel Warning # Warnings only
```

### Stats
```
stat {system_name}             # Show performance stats
```

---

## Common Patterns

### Pattern 1: Basic Setup
```
1. Create DA_{system_name}_Name
2. Create BP_{system_name}_Name
3. Reference Data Asset in Blueprint
4. Place in level
```

### Pattern 2: Runtime Configuration
```
1. Create Data Asset with base settings
2. Create Blueprint with logic
3. Override values at runtime via Blueprint
4. Respond to events
```

### Pattern 3: Multiple Instances
```
1. Create one Data Asset
2. Create multiple Blueprint instances
3. Share Data Asset reference
4. Each has unique runtime state
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Not initializing | Check Data Asset is set and valid |
| No response | Verify BeginPlay is called |
| Performance slow | Disable Tick if not needed |
| Changes not saving | Save Data Asset and Blueprint |
| Errors in log | Check all required fields are filled |

---

## File Locations

### Templates
```
Assets/{system_name}Templates/*.yaml
```

### Documentation
```
Assets/{system_name}SystemGuide.md
Assets/{system_name}DesignerWorkflow.md
```

### Example Assets
```
Content/{system_name}/DataAssets/Examples/
Content/{system_name}/Blueprints/Examples/
```

---

## Related Systems

- **[System1]**: [How they integrate]
- **[System2]**: [How they integrate]
- **[System3]**: [How they integrate]

---

## Keyboard Shortcuts

| Action | Shortcut |
|--------|----------|
| Play in Editor | Alt+P |
| Compile Blueprint | F7 |
| Save All | Ctrl+Shift+S |
| Output Log | Ctrl+Shift+O |

---

## Best Practices ✓

- ✓ Use Data Assets for all configuration
- ✓ Name assets with proper prefixes (DA_, BP_)
- ✓ Test in editor before committing
- ✓ Add descriptions to all assets
- ✓ Keep Blueprints simple and readable

---

## Anti-Patterns ✗

- ✗ Hardcoding values in Blueprints
- ✗ Using Tick for everything
- ✗ Ignoring error messages
- ✗ Not saving before testing
- ✗ Creating unnamed or poorly named assets

---

## Need More Detail?

- **Full Guide**: [{system_name}SystemGuide.md](./{system_name}SystemGuide.md)
- **Workflow**: [{system_name}DesignerWorkflow.md](./{system_name}DesignerWorkflow.md)
- **Templates**: [{system_name}Templates.md](./{system_name}Templates.md)

---

*Quick reference maintained by Adastrea team • Updated {datetime.now().strftime('%Y-%m-%d')}*
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(template_content)
        
        print(f"✓ Created quick reference: {filepath}")
        return filepath
    
    def generate_api_reference(self, system_name: str, output_dir: Optional[Path] = None) -> Path:
        """
        Generate an API reference guide
        
        Args:
            system_name: Name of the system
            output_dir: Output directory (defaults to Assets/)
            
        Returns:
            Path to the created guide file
        """
        if output_dir is None:
            output_dir = self.docs_dir
        output_dir.mkdir(parents=True, exist_ok=True)
        
        clean_name = system_name.replace(" ", "")
        filename = f"{clean_name}APIReference.md"
        filepath = output_dir / filename
        
        template_content = f"""# {system_name} - Complete API Reference

**Version**: 1.0  
**Last Updated**: {datetime.now().strftime('%Y-%m-%d')}  
**Engine**: Unreal Engine 5.5

---

## Table of Contents

1. [Overview](#overview)
2. [Data Asset Classes](#data-asset-classes)
3. [Component Classes](#component-classes)
4. [Actor Classes](#actor-classes)
5. [Subsystem Classes](#subsystem-classes)
6. [Structs and Enums](#structs-and-enums)
7. [Blueprint Function Library](#blueprint-function-library)
8. [Events and Delegates](#events-and-delegates)

---

## Overview

This document provides complete API documentation for the {system_name} system, including all classes, functions, properties, and events.

### Module Information

- **Module Name**: Adastrea
- **API Macro**: ADASTREA_API
- **Include Path**: `{system_name}/{system_name}DataAsset.h`

---

## Data Asset Classes

### U{system_name}DataAsset

**Purpose**: Configuration data for {system_name} instances

**Inherits From**: `UDataAsset`

#### Header
```cpp
#include "{system_name}/{system_name}DataAsset.h"
```

#### Class Declaration
```cpp
UCLASS(BlueprintType)
class ADASTREA_API U{system_name}DataAsset : public UDataAsset
{{
    GENERATED_BODY()

public:
    U{system_name}DataAsset();
    
    // Public interface...
}};
```

#### Properties

##### DisplayName
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
FText DisplayName;
```
- **Type**: FText
- **Access**: EditAnywhere, BlueprintReadOnly
- **Description**: Human-readable display name
- **Default**: Empty

##### Description
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info", meta=(MultiLine=true))
FText Description;
```
- **Type**: FText
- **Access**: EditAnywhere, BlueprintReadOnly
- **Description**: Detailed description of this configuration
- **Default**: Empty
- **Meta Tags**: MultiLine=true

##### UniqueID
```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Info")
FName UniqueID;
```
- **Type**: FName
- **Access**: EditAnywhere, BlueprintReadOnly
- **Description**: Unique identifier for this instance
- **Default**: NAME_None

[Add more properties as needed...]

#### Functions

##### Initialize
```cpp
UFUNCTION(BlueprintCallable, Category="{system_name}")
void Initialize();
```
- **Returns**: void
- **Description**: Initializes the data asset and validates all properties
- **Blueprint**: BlueprintCallable
- **Category**: {system_name}

##### GetConfigValue
```cpp
UFUNCTION(BlueprintPure, Category="{system_name}")
float GetConfigValue() const;
```
- **Returns**: float - The configuration value
- **Description**: Retrieves a configuration value
- **Blueprint**: BlueprintPure (no exec pins)
- **Category**: {system_name}
- **Thread Safe**: Yes

[Add more functions...]

---

## Component Classes

### U{system_name}Component

**Purpose**: Runtime component for {system_name} functionality

**Inherits From**: `UActorComponent`

#### Header
```cpp
#include "{system_name}/{system_name}Component.h"
```

#### Class Declaration
```cpp
UCLASS(BlueprintType, ClassGroup=({system_name}), meta=(BlueprintSpawnableComponent))
class ADASTREA_API U{system_name}Component : public UActorComponent
{{
    GENERATED_BODY()

public:
    U{system_name}Component();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ...) override;
}};
```

#### Properties

##### DataAsset
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="{system_name}")
U{system_name}DataAsset* DataAsset;
```
- **Type**: U{system_name}DataAsset*
- **Access**: EditAnywhere, BlueprintReadWrite
- **Description**: Reference to configuration Data Asset
- **Default**: nullptr

##### bAutoActivate
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Activation")
bool bAutoActivate;
```
- **Type**: bool
- **Access**: EditAnywhere, BlueprintReadWrite
- **Description**: Whether component activates automatically on BeginPlay
- **Default**: true

[Add more properties...]

#### Functions

##### InitializeWithDataAsset
```cpp
UFUNCTION(BlueprintCallable, Category="{system_name}")
bool InitializeWithDataAsset(U{system_name}DataAsset* InDataAsset);
```
- **Parameters**:
  - `InDataAsset`: Data Asset to use for configuration
- **Returns**: bool - true if initialization successful
- **Description**: Initializes the component with the provided Data Asset
- **Blueprint**: BlueprintCallable

##### UpdateSystem
```cpp
UFUNCTION(BlueprintCallable, Category="{system_name}")
void UpdateSystem(float DeltaTime);
```
- **Parameters**:
  - `DeltaTime`: Time since last update in seconds
- **Returns**: void
- **Description**: Updates the system state
- **Blueprint**: BlueprintCallable

##### IsActive
```cpp
UFUNCTION(BlueprintPure, Category="{system_name}")
bool IsActive() const;
```
- **Returns**: bool - true if component is active
- **Description**: Checks if the component is currently active
- **Blueprint**: BlueprintPure

[Add more functions...]

---

## Actor Classes

### A{system_name}Actor

**Purpose**: Actor class for {system_name} instances

**Inherits From**: `AActor`

#### Header
```cpp
#include "{system_name}/{system_name}Actor.h"
```

#### Class Declaration
```cpp
UCLASS(BlueprintType, Blueprintable)
class ADASTREA_API A{system_name}Actor : public AActor
{{
    GENERATED_BODY()

public:
    A{system_name}Actor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
}};
```

#### Components

##### {system_name}Component
```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
U{system_name}Component* {system_name}Component;
```
- **Type**: U{system_name}Component*
- **Access**: VisibleAnywhere, BlueprintReadOnly
- **Description**: Main component for {system_name} functionality
- **Default**: Created in constructor

[Add more components...]

#### Functions

[Add actor-specific functions...]

---

## Subsystem Classes

### U{system_name}Subsystem

**Purpose**: Game instance subsystem for managing {system_name} globally

**Inherits From**: `UGameInstanceSubsystem`

#### Header
```cpp
#include "{system_name}/{system_name}Subsystem.h"
```

#### Class Declaration
```cpp
UCLASS()
class ADASTREA_API U{system_name}Subsystem : public UGameInstanceSubsystem
{{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
}};
```

#### Functions

##### GetInstance
```cpp
UFUNCTION(BlueprintPure, Category="{system_name}", meta=(WorldContext="WorldContextObject"))
static U{system_name}Subsystem* GetInstance(const UObject* WorldContextObject);
```
- **Parameters**:
  - `WorldContextObject`: Object with world context
- **Returns**: U{system_name}Subsystem* - The subsystem instance
- **Description**: Gets the {system_name} subsystem instance
- **Blueprint**: BlueprintPure
- **Meta**: WorldContext="WorldContextObject"

[Add more subsystem functions...]

---

## Structs and Enums

### F{system_name}State

**Purpose**: Represents the current state of a {system_name} instance

```cpp
USTRUCT(BlueprintType)
struct F{system_name}State
{{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Value1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Value2;

    F{system_name}State()
        : Value1(0.0f)
        , Value2("")
    {{}}
}};
```

#### Members

- **Value1** (float): [Description]
- **Value2** (FString): [Description]

---

### E{system_name}Status

**Purpose**: Enum representing status states

```cpp
UENUM(BlueprintType)
enum class E{system_name}Status : uint8
{{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Processing  UMETA(DisplayName = "Processing"),
    Error       UMETA(DisplayName = "Error")
}};
```

#### Values

- **Inactive**: System is not active
- **Active**: System is running normally
- **Processing**: System is performing an operation
- **Error**: System encountered an error

---

## Blueprint Function Library

### U{system_name}BlueprintLibrary

**Purpose**: Static Blueprint-callable utility functions

#### Header
```cpp
#include "{system_name}/{system_name}BlueprintLibrary.h"
```

#### Functions

##### HelperFunction1
```cpp
UFUNCTION(BlueprintCallable, Category="{system_name}|Utilities")
static bool HelperFunction1(AActor* Actor);
```
- **Parameters**:
  - `Actor`: The actor to process
- **Returns**: bool - Success status
- **Description**: [Description of what this helper does]
- **Blueprint**: BlueprintCallable

[Add more library functions...]

---

## Events and Delegates

### FOn{system_name}Changed

**Purpose**: Delegate called when {system_name} state changes

#### Declaration
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOn{system_name}Changed, 
    float, NewValue
);
```

#### Usage in C++
```cpp
{system_name}Component->On{system_name}Changed.AddDynamic(
    this, 
    &AMyActor::Handle{system_name}Changed
);
```

#### Usage in Blueprint
```
Event Graph → On {system_name} Changed
└─ [Handle the event]
```

---

### FOn{system_name}Complete

**Purpose**: Delegate called when {system_name} operation completes

#### Declaration
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOn{system_name}Complete,
    bool, bSuccess,
    const FString&, Message
);
```

[Add more delegates...]

---

## Usage Examples

### C++ Example 1: Basic Setup
```cpp
// In your actor's BeginPlay
U{system_name}Component* Comp = FindComponentByClass<U{system_name}Component>();
if (Comp && Comp->DataAsset)
{{
    Comp->InitializeWithDataAsset(Comp->DataAsset);
}}
```

### C++ Example 2: Runtime Updates
```cpp
// In Tick or custom update
if ({system_name}Component && {system_name}Component->IsActive())
{{
    {system_name}Component->UpdateSystem(DeltaTime);
}}
```

### Blueprint Example
```
Event BeginPlay
├─ Get {system_name} Component
└─ Initialize With Data Asset
   ├─ Data Asset: DA_{system_name}_MyConfig
   └─ Branch
      ├─ True: Print "Success"
      └─ False: Print "Failed"
```

---

## Performance Considerations

### Tick Frequency
- Default tick rate: 60 Hz
- Consider reducing for non-critical systems
- Use timers for periodic updates instead of Tick

### Memory Usage
- Data Assets are loaded on demand
- Components cached for performance
- Use object pooling for frequently spawned actors

### Threading
- Most functions are game thread only
- Marked thread-safe functions can be called from worker threads
- Check documentation for each function

---

## Version History

### Version 1.0 ({datetime.now().strftime('%Y-%m-%d')})
- Initial API implementation
- Core classes and functions
- Basic event system

---

## Additional Resources

- **System Guide**: [{system_name}SystemGuide.md](./{system_name}SystemGuide.md)
- **Workflow Guide**: [{system_name}DesignerWorkflow.md](./{system_name}DesignerWorkflow.md)
- **Quick Reference**: [{system_name}QuickReference.md](./{system_name}QuickReference.md)

---

*API Reference maintained by Adastrea development team • Last update: {datetime.now().strftime('%Y-%m-%d')}*
"""
        
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(template_content)
        
        print(f"✓ Created API reference: {filepath}")
        return filepath
    
    def interactive_mode(self):
        """Run the guide generator in interactive mode"""
        print("=" * 60)
        print("Adastrea Guide Generator - Interactive Mode")
        print("=" * 60)
        print()
        print("Available guide types:")
        print("  1. System Guide (Complete documentation)")
        print("  2. Designer Workflow Guide (Step-by-step)")
        print("  3. Quick Reference (One-page cheat sheet)")
        print("  4. API Reference (Complete API docs)")
        print("  0. Exit")
        print()
        
        while True:
            choice = input("Select guide type (0-4): ").strip()
            
            if choice == "0":
                print("Exiting...")
                break
            elif choice == "1":
                name = input("Enter system name: ").strip()
                self.generate_system_guide(name)
            elif choice == "2":
                name = input("Enter system name: ").strip()
                self.generate_workflow_guide(name)
            elif choice == "3":
                name = input("Enter system name: ").strip()
                self.generate_quick_reference(name)
            elif choice == "4":
                name = input("Enter system name: ").strip()
                self.generate_api_reference(name)
            else:
                print("Invalid choice. Please select 0-4.")
            
            print()


def main():
    """Main entry point for command line usage"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description='Generate markdown documentation guides for Adastrea game systems',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Interactive mode
  python GuideGenerator.py
  
  # Generate specific guides
  python GuideGenerator.py --type system --name "MySystem"
  python GuideGenerator.py --type workflow --name "MySystem"
  python GuideGenerator.py --type quickref --name "MySystem"
  python GuideGenerator.py --type api --name "MySystem"
  
  # Specify custom output directory
  python GuideGenerator.py --type system --name "MySystem" --output Docs/
        """
    )
    
    parser.add_argument('--type', choices=['system', 'workflow', 'quickref', 'api'],
                       help='Type of guide to generate')
    parser.add_argument('--name', help='Name of the system')
    parser.add_argument('--output', help='Output directory (optional)')
    
    args = parser.parse_args()
    
    generator = GuideGenerator()
    
    # If no arguments provided, run interactive mode
    if not args.type:
        generator.interactive_mode()
        return
    
    # Validate that name is provided for batch mode
    if not args.name:
        print("Error: --name is required when using --type")
        sys.exit(1)
    
    output_dir = Path(args.output) if args.output else None
    
    # Generate based on type
    if args.type == 'system':
        generator.generate_system_guide(args.name, output_dir)
    elif args.type == 'workflow':
        generator.generate_workflow_guide(args.name, output_dir)
    elif args.type == 'quickref':
        generator.generate_quick_reference(args.name, output_dir)
    elif args.type == 'api':
        generator.generate_api_reference(args.name, output_dir)


if __name__ == '__main__':
    main()
