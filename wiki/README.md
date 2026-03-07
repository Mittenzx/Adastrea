# Adastrea Wiki

This directory contains the GitHub Wiki content for the Adastrea project.

## Wiki Structure

```
wiki/
├── Home.md                     # Wiki landing page
├── _Sidebar.md                 # Navigation sidebar
├── README.md                   # This file
│
├── Getting-Started/
│   ├── Quick-Start.md          # 10-minute setup
│   ├── Installation.md         # Detailed installation
│   ├── Designer-Onboarding.md  # For non-programmers
│   ├── Content-Creation.md     # Content creation guide
│   └── First-Steps.md          # First hour guide
│
├── Core-Systems/
│   ├── README.md               # Systems overview
│   ├── Spaceship-System.md     # Ship system
│   ├── Faction-System.md       # Faction system
│   ├── Personnel-System.md     # Personnel system
│   ├── Trading-System.md       # Trading system
│   ├── Way-System.md           # Way system (guilds)
│   ├── Station-System.md       # Station system
│   ├── Combat-System.md        # Combat system
│   └── AI-System.md            # AI system
│
├── Developer-Guides/
│   ├── Architecture.md         # System architecture
│   ├── Contributing.md         # How to contribute
│   ├── Code-Style.md           # Coding standards
│   ├── Build-Setup.md          # Build configuration
│   ├── Performance.md          # Optimization
│   └── Roadmap.md              # Development roadmap
│
├── Blueprint-Development/
│   ├── Blueprint-Beginners-Guide.md    # Complete beginner guide
│   ├── Blueprint-Requirements.md       # Required Blueprints
│   ├── Blueprint-Quick-Reference.md    # Quick reference
│   ├── Blueprint-Creation.md           # Creation guide
│   └── Blueprint-Workflows.md          # Workflow templates
│
├── Automation-Testing/
│   ├── Testing-Guide.md        # Testing overview
│   ├── Automation-Scripts.md   # Script usage
│   ├── Procedural-Generation.md # Random content
│   ├── Scene-Population.md     # Level population
│   └── Cloud-Build.md          # CI/CD setup
│
└── Reference/
    ├── Documentation-Index.md  # Complete doc index
    ├── API-Reference.md        # API documentation
    ├── YAML-Templates.md       # Template reference
    ├── Quick-References.md     # All quick refs
    └── Cheat-Sheet.md          # Common commands
```

## Wiki vs Repository Documentation

### Wiki Contains
- **High-level overviews** - Easy to navigate summaries
- **Getting started guides** - Beginner-friendly tutorials
- **Quick references** - Fast lookups
- **Cross-linked navigation** - Easy to browse
- **Organized by topic** - Clear categories

### Repository Contains
- **Complete technical guides** - Full system documentation (`Assets/` folder)
- **YAML templates** - Copy-paste configurations (`Assets/*/Templates/`)
- **Source code** - C++ implementation (`Source/` folder)
- **Scripts** - Automation tools (root directory)
- **Build configs** - Project configuration (`Config/` folder)

### How They Work Together
- Wiki pages **link to** repository docs for details
- Repository README **links to** wiki for navigation
- Both are **kept in sync** during updates
- Wiki provides **entry points**, repository provides **depth**

## Using the Wiki

### For New Users
1. Start at [Home](Home.md)
2. Follow [Quick Start](Getting-Started/Quick-Start.md)
3. Choose your role:
   - Designer → [Designer Onboarding](Getting-Started/Designer-Onboarding.md)
   - Developer → [Architecture](Developer-Guides/Architecture.md)
4. Browse [Core Systems](Core-Systems/README.md) for features

### For Contributors
1. Read [Contributing Guide](Developer-Guides/Contributing.md)
2. Check [Code Style](Developer-Guides/Code-Style.md)
3. Review relevant system docs
4. Follow [Blueprint Guidelines](Blueprint-Development/Blueprint-Beginners-Guide.md)

### For Reference
1. Use [Documentation Index](Reference/Documentation-Index.md) to find docs
2. Check [Quick References](Reference/Quick-References.md) for fast lookups
3. Review [Cheat Sheet](Reference/Cheat-Sheet.md) for common commands
4. Browse [YAML Templates](Reference/YAML-Templates.md) for examples

## Maintaining the Wiki

### Adding New Pages
1. Create markdown file in appropriate directory
2. Follow naming convention: `Topic-Name.md` (with hyphens)
3. Update `_Sidebar.md` navigation
4. Add entry to [Documentation Index](Reference/Documentation-Index.md)
5. Link from related pages

### Updating Existing Pages
1. Keep backward compatibility for links
2. Update "Last Updated" date
3. Update related pages if needed
4. Test all internal links

### Wiki Conventions
- **File names**: Use hyphens: `Quick-Start.md` not `Quick_Start.md`
- **Headers**: Use sentence case: `Getting Started` not `GETTING STARTED`
- **Internal Links** (within wiki): Use relative paths: `../Core-Systems/README.md`
- **Repository Links** (to main repo): Use relative paths from wiki root: `../../Assets/SystemGuide.md`
- **Note**: Repository links assume wiki is in `wiki/` subdirectory of main repo
- **Images**: Store in `assets/` subdirectories
- **Code blocks**: Always specify language for syntax highlighting

## Publishing to GitHub Wiki

The contents of this directory can be published to GitHub Wiki:

### Manual Method
1. Clone the wiki repository:
   ```bash
   git clone https://github.com/Mittenzx/Adastrea.wiki.git
   ```
2. Copy files from `wiki/` directory
3. Commit and push:
   ```bash
   git add .
   git commit -m "Update wiki"
   git push origin master
   ```

### Automated Method (Future)
A GitHub Action can automatically sync `wiki/` to the GitHub Wiki repository.

## Documentation Organization

### By Audience
- **Beginners**: Getting-Started/
- **Designers**: Getting-Started/ + Core-Systems/
- **Developers**: Developer-Guides/ + Core-Systems/
- **Contributors**: Developer-Guides/Contributing.md

### By Activity
- **Setup**: Getting-Started/
- **Creating Content**: Core-Systems/ + Blueprint-Development/
- **Contributing Code**: Developer-Guides/
- **Testing**: Automation-Testing/
- **Reference**: Reference/

### By Depth
- **Overview**: Home.md + README files
- **Quick Start**: Getting-Started/ + Quick-Reference files
- **Complete Guide**: Link to repository `Assets/` folder
- **Technical Details**: Link to source code and repository docs

## Search Tips

Use GitHub wiki search to find specific content:
- `"Blueprint"` - Blueprint development guides
- `"Data Asset"` - Data-driven content creation
- `"Quick Start"` - Fast setup guides
- `"API"` - API references
- `"Template"` - YAML templates and examples
- `"Workflow"` - Step-by-step processes
- `"System"` - Game system documentation

## External Resources

- **Main Repository**: https://github.com/Mittenzx/Adastrea
- **Issues**: https://github.com/Mittenzx/Adastrea/issues
- **Discussions**: https://github.com/Mittenzx/Adastrea/discussions
- **Pull Requests**: https://github.com/Mittenzx/Adastrea/pulls

## Contributing to Wiki

Wiki improvements are welcome! To contribute:

1. **Minor Edits**: Edit directly on GitHub wiki
2. **Major Changes**:
   - Clone repository
   - Edit files in `wiki/` directory
   - Submit pull request
   - Wiki will be updated after merge

### What to Contribute
- 📝 Fix typos and errors
- 📚 Add missing documentation
- 🔗 Improve cross-references
- 📊 Add diagrams and visuals
- 📖 Write tutorials and guides
- 🎯 Improve organization

## Wiki Status

**Current Status**: ✅ Active Development

**Completion**:
- ✅ Structure created
- ✅ Home page
- ✅ Navigation sidebar
- ✅ Getting Started section (partial)
- ✅ Core Systems overview
- ✅ Reference section (partial)
- 🔄 Developer Guides (in progress)
- 🔄 Blueprint Development (in progress)
- 🔄 Automation & Testing (in progress)

**Planned**:
- 📸 Screenshot tutorials
- 🎥 Video guides
- 📊 Architecture diagrams
- 🗺️ Visual roadmaps
- 📱 Mobile-friendly formatting

## Questions?

- **Documentation Questions**: Open an issue with "documentation" label
- **Wiki Structure**: Start a discussion
- **Missing Content**: Request in issues
- **Errors**: Report in issues

---

**The wiki is a living document - contributions and improvements are always welcome!**

*Last Updated: November 23, 2025*
