# Blueprint Display Cheatsheet

> **Quick reference for documenting Blueprints in markdown**  
> Full guide: [BLUEPRINT_DISPLAY_GUIDE.md](BLUEPRINT_DISPLAY_GUIDE.md)

---

## Quick Format Decision

```
Is it complex with branches? → Use Mermaid
Is it linear and simple? → Use Enhanced Text
Need detailed reference? → Use Node Card
Need quick overview? → Use Table
Teaching/Tutorial? → Use Hybrid (Multiple formats)
```

---

## Mermaid Quick Start (Blueprint-Inspired Style)

> **Note:** The following Mermaid template uses colors and shapes optimized for readability in markdown documentation. These do **not** match Unreal Engine's Blueprint editor exactly, but are inspired by common Blueprint node concepts.

### Basic Template

````markdown
```mermaid
graph LR
    A[["⚡ Event Start"]] --> B[["⚙️ Action Node"]]
    B --> C{{"✓ Decision?"}}
    C -->|"✓ Yes"| D[["⚙️ True Path"]]
    C -->|"✗ No"| E[["❌ False Path"]]
    D --> F(( ))
    E --> F
    
    style A fill:#1a1a1a,stroke:#DC3545,stroke-width:4px,color:#fff,rx:10,ry:10
    style B fill:#1a1a1a,stroke:#0d6efd,stroke-width:3px,color:#fff,rx:10,ry:10
    style C fill:#1a1a1a,stroke:#17a2b8,stroke-width:3px,color:#17a2b8
    style D fill:#1a1a1a,stroke:#0d6efd,stroke-width:3px,color:#fff,rx:10,ry:10
    style E fill:#1a1a1a,stroke:#DC3545,stroke-width:3px,color:#fff,rx:10,ry:10
    style F fill:#1a1a1a,stroke:#6c757d,stroke-width:2px,color:#fff
```
````

### Blueprint Node Shapes

| Code | Shape | Use For | Node Type |
|------|-------|---------|-----------|
| `[["Text"]]` | Rounded Rectangle | Functions, actions | Standard nodes |
| `{{"Text?"}}` | Diamond | Decisions, branches | Branch nodes |
| `(( ))` | Circle | End points | Flow control |
| `[["⚡ Text"]]` | Thick bordered | Events | Event nodes (4px border) |

### Common Arrows

| Code | Arrow | Meaning |
|------|-------|---------|
| `-->` | Solid | Execution flow |
| `-.->` | Dashed | Optional/Error path |
| `==>` | Thick | Important path |
| `-- Text -->` | Labeled | Condition label |

---

## Blueprint-Inspired Color Codes (Copy-Paste Ready)

```
Event (Red):      style Node fill:#1a1a1a,stroke:#DC3545,stroke-width:4px,color:#fff,rx:10,ry:10
Function (Blue):  style Node fill:#1a1a1a,stroke:#0d6efd,stroke-width:3px,color:#fff,rx:10,ry:10
Branch (Cyan):    style Node fill:#1a1a1a,stroke:#17a2b8,stroke-width:3px,color:#17a2b8
Loop (Teal):      style Node fill:#1a1a1a,stroke:#20c997,stroke-width:3px,color:#fff,rx:10,ry:10
Variable (Purple):style Node fill:#1a1a1a,stroke:#6f42c1,stroke-width:3px,color:#fff,rx:10,ry:10
Pure (Cyan):      style Node fill:#1a1a1a,stroke:#17a2b8,stroke-width:3px,color:#fff,rx:10,ry:10
Error (Red):      style Node fill:#1a1a1a,stroke:#DC3545,stroke-width:3px,color:#fff,rx:10,ry:10
Success (Green):  style Node fill:#1a1a1a,stroke:#28a745,stroke-width:3px,color:#fff,rx:10,ry:10
End (Gray):       style Node fill:#1a1a1a,stroke:#6c757d,stroke-width:2px,color:#fff
```

**Key Styling Rules:**
- All nodes: Dark background `#1a1a1a` for professional appearance
- Events: 4px stroke width (emphasized for visual hierarchy)
- Standard nodes: 3px stroke width
- White text: `color:#fff` for contrast
- Rounded corners: `rx:10,ry:10` on rectangular nodes
- Branch nodes: No rx/ry (diamond shape in flowcharts)

---

## Emoji Reference

| Category | Emojis | Usage |
|----------|--------|-------|
| **Events** | 📅 🎮 🖱️ | Events, inputs |
| **Actions** | ⚙️ 🔧 ➕ | Functions, operations |
| **Data** | 📦 📊 💾 | Variables, storage |
| **Decisions** | ❓ 🔍 | Branches, checks |
| **Status** | ✅ ❌ ⚠️ | Success, error, warning |
| **Flow** | 🔄 ⏱️ 🔀 | Loops, delays, switches |
| **UI** | 🎨 🖼️ 📝 | Visual, display |
| **Audio** | 🎵 🔊 | Sound, music |
| **Economy** | 💰 💳 💲 | Credits, transactions |
| **Combat** | 🎯 💥 🔫 | Targeting, damage |
| **Output** | 📤 🏁 | Return, end |

---

## Enhanced Text Template

```
🎯 Function: [Name]
📥 Inputs: [Parameters]
📤 Outputs: [Returns]

──────────────────────────────────
EXECUTION FLOW
──────────────────────────────────

🟢 START
    │
    ├─→ [Step 1]
    │     └─→ [Detail]
    │
    ├─→ ❓ [Decision]
    │     ├─→ ✅ True Path
    │     └─→ ❌ False Path
    │
    └─→ 🏁 END
```

---

## Node Card Template

````markdown
<details>
<summary>⚙️ <b>Function Name</b> - Type</summary>

```
┌─────────────────────────────────────────┐
│ FUNCTION NAME                           │
│ Type: [Type]                            │
│ Category: [Category]                    │
├─────────────────────────────────────────┤
│ 📥 INPUTS                                │
│   • Param (Type) - Description          │
│                                          │
│ 📤 OUTPUTS                               │
│   • Return (Type) - Description         │
├─────────────────────────────────────────┤
│ 📋 DESCRIPTION                           │
│   [What it does]                        │
│                                          │
│ 💡 USAGE                                 │
│   [When to use it]                      │
└─────────────────────────────────────────┘
```

</details>
````

---

## Table Template

```markdown
| Function | Inputs | Outputs | Purpose |
|----------|--------|---------|---------|
| Name | Params | Returns | Description |
```

---

## Complete Example

````markdown
### My Function Documentation

```mermaid
graph TD
    A[📅 Start] --> B[⚙️ Process]
    B --> C{✓ Valid?}
    C -->|Yes| D[✅ Success]
    C -->|No| E[❌ Error]
    
    style A fill:#4CAF50,stroke:#2E7D32,color:#fff
    style C fill:#2196F3,stroke:#1565C0,color:#fff
    style D fill:#4CAF50,stroke:#2E7D32,color:#fff
    style E fill:#F44336,stroke:#C62828,color:#fff
```

<details>
<summary>⚙️ <b>Process Data</b> - Function</summary>

```
┌─────────────────────────────────────────┐
│ PROCESS DATA                            │
│ Type: Pure Function                     │
├─────────────────────────────────────────┤
│ 📥 INPUTS                                │
│   • Data (Array) - Input data           │
│                                          │
│ 📤 OUTPUTS                               │
│   • Result (Boolean) - Success status   │
└─────────────────────────────────────────┘
```

</details>
````

---

## Testing Your Diagrams

1. **Copy your mermaid code**
2. **Paste into** https://mermaid.live/
3. **Verify it renders correctly**
4. **Copy back to markdown**
5. **Test on GitHub preview**

---

## Common Patterns (Blueprint-Inspired Style)

### Simple Function

```mermaid
graph LR
    A[["⚡ Start"]] --> B[["⚙️ Action"]]
    B --> C(( ))
    
    style A fill:#1a1a1a,stroke:#DC3545,stroke-width:4px,color:#fff,rx:10,ry:10
    style B fill:#1a1a1a,stroke:#0d6efd,stroke-width:3px,color:#fff,rx:10,ry:10
    style C fill:#1a1a1a,stroke:#6c757d,stroke-width:2px,color:#fff
```

### Branch Pattern

```mermaid
graph LR
    A[["⚡ Start"]] --> B{{"✓ Check?"}}
    B -->|"✓ Yes"| C[["⚙️ True Path"]]
    B -->|"✗ No"| D[["❌ False Path"]]
    C --> E(( ))
    D --> E
    
    style A fill:#1a1a1a,stroke:#DC3545,stroke-width:4px,color:#fff,rx:10,ry:10
    style B fill:#1a1a1a,stroke:#17a2b8,stroke-width:3px,color:#17a2b8
    style C fill:#1a1a1a,stroke:#0d6efd,stroke-width:3px,color:#fff,rx:10,ry:10
    style D fill:#1a1a1a,stroke:#DC3545,stroke-width:3px,color:#fff,rx:10,ry:10
    style E fill:#1a1a1a,stroke:#6c757d,stroke-width:2px,color:#fff
```

### Loop Pattern

```mermaid
graph LR
    A[["⚡ Start"]] --> B[["🔄 ForEach Loop"]]
    B --> C[["⚙️ Action"]]
    C --> D{{"✓ More?"}}
    D -->|"✓ Yes"| B
    D -->|"✗ No"| E(( ))
    
    style A fill:#1a1a1a,stroke:#DC3545,stroke-width:4px,color:#fff,rx:10,ry:10
    style B fill:#1a1a1a,stroke:#20c997,stroke-width:3px,color:#fff,rx:10,ry:10
    style C fill:#1a1a1a,stroke:#0d6efd,stroke-width:3px,color:#fff,rx:10,ry:10
    style D fill:#1a1a1a,stroke:#17a2b8,stroke-width:3px,color:#17a2b8
    style E fill:#1a1a1a,stroke:#6c757d,stroke-width:2px,color:#fff
```

### Error Handling

```mermaid
graph LR
    A[["⚙️ Try Operation"]] --> B{{"✓ Success?"}}
    B -->|"✓ Yes"| C[["✅ Continue"]]
    B -->|"✗ No"| D[["❌ Error"]]
    D --> E[["⚙️ Handle Error"]]
    E --> F(( ))
    C --> F
    
    style A fill:#1a1a1a,stroke:#0d6efd,stroke-width:3px,color:#fff,rx:10,ry:10
    style B fill:#1a1a1a,stroke:#17a2b8,stroke-width:3px,color:#17a2b8
    style C fill:#1a1a1a,stroke:#28a745,stroke-width:3px,color:#fff,rx:10,ry:10
    style D fill:#1a1a1a,stroke:#DC3545,stroke-width:3px,color:#fff,rx:10,ry:10
    style E fill:#1a1a1a,stroke:#0d6efd,stroke-width:3px,color:#fff,rx:10,ry:10
    style F fill:#1a1a1a,stroke:#6c757d,stroke-width:2px,color:#fff
```

---

## Don't Forget

✅ Use consistent colors  
✅ Add emoji for quick scanning  
✅ Label branches clearly  
✅ Show error paths  
✅ Test on GitHub  
✅ Keep diagrams under 30 nodes  
✅ Split complex flows  
✅ Include text alternatives  

---

## VS Code Snippets (Optional)

Add to your `.vscode/markdown.code-snippets`:

```json
{
  "Mermaid Blueprint": {
    "prefix": "bp-mermaid",
    "body": [
      "```mermaid",
      "graph TD",
      "    A[📅 ${1:Start}] --> B[⚙️ ${2:Action}]",
      "    B --> C{✓ ${3:Check?}}",
      "    C -->|Yes| D[✅ ${4:Success}]",
      "    C -->|No| E[❌ ${5:Error}]",
      "    ",
      "    style A fill:#4CAF50,stroke:#2E7D32,color:#fff",
      "    style C fill:#2196F3,stroke:#1565C0,color:#fff",
      "    style D fill:#4CAF50,stroke:#2E7D32,color:#fff",
      "    style E fill:#F44336,stroke:#C62828,color:#fff",
      "```"
    ]
  }
}
```

---

## Need More Help?

- **Full Guide**: [BLUEPRINT_DISPLAY_GUIDE.md](BLUEPRINT_DISPLAY_GUIDE.md)
- **Examples**: [BLUEPRINT_DISPLAY_EXAMPLES.md](BLUEPRINT_DISPLAY_EXAMPLES.md)
- **Mermaid Docs**: https://mermaid.js.org/
- **Live Editor**: https://mermaid.live/

---

**Quick Tip**: Start simple. A basic Mermaid diagram is better than complex ASCII art!

**Last Updated**: November 19, 2025
