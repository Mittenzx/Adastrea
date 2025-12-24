# Wiki Setup Instructions

This document explains how to publish the `wiki/` directory content to GitHub's Wiki feature.

## Overview

The `wiki/` directory contains organized documentation designed to be published to GitHub Wiki. GitHub Wiki is a separate Git repository that provides an easy-to-navigate web interface for documentation.

## Method 1: Manual Wiki Publishing (Recommended for First Time)

### Step 1: Enable GitHub Wiki

1. Go to repository settings: https://github.com/Mittenzx/Adastrea/settings
2. Scroll to "Features" section
3. Check the "Wikis" option
4. Click "Save"

### Step 2: Clone the Wiki Repository

GitHub Wiki is a separate Git repository. Clone it:

```bash
# Clone the wiki repository
git clone https://github.com/Mittenzx/Adastrea.wiki.git

# Navigate to the wiki directory
cd Adastrea.wiki
```

### Step 3: Copy Wiki Content

Copy content from the main repository's `wiki/` directory:

```bash
# From the wiki repository directory
cp -r ../Adastrea/wiki/* .

# Or if repositories are in different locations:
cp -r /path/to/Adastrea/wiki/* /path/to/Adastrea.wiki/
```

### Step 4: Commit and Push

```bash
# Stage all files
git add .

# Commit with descriptive message
git commit -m "Initial wiki setup with comprehensive documentation"

# Push to GitHub
git push origin master
```

### Step 5: Verify on GitHub

1. Visit: https://github.com/Mittenzx/Adastrea/wiki
2. Check that all pages appear
3. Test navigation using the sidebar
4. Verify all links work

## Method 2: Automated Wiki Sync (Future Implementation)

A GitHub Action can be set up to automatically sync `wiki/` directory to GitHub Wiki on each commit.

### Create GitHub Action

Create `.github/workflows/wiki-sync.yml`:

```yaml
name: Sync Wiki

on:
  push:
    branches:
      - main
    paths:
      - 'wiki/**'

jobs:
  sync-wiki:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout main repository
        uses: actions/checkout@v3

      - name: Checkout wiki repository
        uses: actions/checkout@v3
        with:
          repository: ${{ github.repository }}.wiki
          path: wiki-repo

      - name: Sync wiki content
        run: |
          rsync -av --delete wiki/ wiki-repo/
          cd wiki-repo
          git config user.name "GitHub Actions"
          git config user.email "actions@github.com"
          git add .
          git diff --quiet && git diff --staged --quiet || git commit -m "Auto-sync from main repository"
          git push

      - name: Cleanup
        run: rm -rf wiki-repo
```

**Note**: This requires write access to the wiki repository. Set up repository secrets if needed.

## Wiki Structure Verification

After publishing, verify the following structure exists on GitHub Wiki:

### Home Page
- URL: `/wiki/Home`
- File: `Home.md`
- Purpose: Landing page with quick navigation

### Getting Started
- `/wiki/Getting-Started/Quick-Start`
- `/wiki/Getting-Started/Installation`
- `/wiki/Getting-Started/Designer-Onboarding`
- `/wiki/Getting-Started/Content-Creation`
- `/wiki/Getting-Started/First-Steps`

### Core Systems
- `/wiki/Core-Systems/README`
- `/wiki/Core-Systems/Spaceship-System`
- `/wiki/Core-Systems/Faction-System`
- `/wiki/Core-Systems/Personnel-System`
- (etc.)

### Developer Guides
- `/wiki/Developer-Guides/Architecture`
- `/wiki/Developer-Guides/Contributing`
- `/wiki/Developer-Guides/Code-Style`
- (etc.)

### Sidebar Navigation
- File: `_Sidebar.md`
- Appears on all wiki pages
- Provides quick navigation

## Maintaining the Wiki

### Updating Existing Pages

**In Main Repository**:
1. Edit files in `wiki/` directory
2. Commit changes to main repository
3. Manually sync or wait for automated sync

**Directly in Wiki** (not recommended):
1. Edit pages directly on GitHub Wiki
2. Changes will NOT sync back to main repository
3. May cause conflicts

**Recommendation**: Always edit in main repository `wiki/` directory, then sync to GitHub Wiki.

### Adding New Pages

1. Create markdown file in appropriate `wiki/` subdirectory
2. Use hyphenated naming: `New-Page-Name.md`
3. Update `_Sidebar.md` with navigation link
4. Add entry to `wiki/Reference/Documentation-Index.md`
5. Commit to main repository
6. Sync to GitHub Wiki

### Keeping in Sync

To keep wiki synchronized with main repository:

```bash
# Pull latest from main repository
cd /path/to/Adastrea
git pull

# Copy updated content
cd /path/to/Adastrea.wiki
cp -r ../Adastrea/wiki/* .

# Commit and push
git add .
git commit -m "Sync wiki updates from main repository"
git push
```

## Wiki vs Repository Documentation

### Wiki Contains
- High-level overviews
- Getting started guides
- Navigation-focused organization
- Quick references
- Cross-linked pages

### Repository Contains
- Complete technical guides (`Assets/` folder)
- YAML templates (`Assets/*/Templates/`)
- Source code (`Source/` folder)
- Build configurations
- Detailed API references

### Relationship
- Wiki provides **entry points** and **navigation**
- Repository provides **depth** and **technical detail**
- Wiki pages **link to** repository docs for complete information
- Both are maintained and kept in sync

## Troubleshooting

### "Permission denied" when pushing to wiki
- Ensure you have write access to the repository
- Check if wiki is enabled in repository settings
- Verify you're authenticated with GitHub

### "Wiki not found"
- Enable wiki in repository settings
- Create at least one page manually first
- Then clone and sync

### Links not working
- Use relative paths: `../Getting-Started/Quick-Start.md`
- GitHub Wiki converts to web URLs automatically
- Test links in repository first

### Images not displaying
- Store images in wiki subdirectories (e.g., `assets/`)
- Use relative paths
- Ensure images are committed to wiki repository

## Best Practices

### Naming Conventions
- Use hyphens, not underscores: `Quick-Start.md` not `Quick_Start.md`
- Capitalize properly: `Getting-Started` not `getting-started`
- Match directory structure to navigation

### Organization
- Group related pages in subdirectories
- Keep directory depth reasonable (max 2-3 levels)
- Use README.md for directory overview pages

### Documentation
- Update `_Sidebar.md` for new pages
- Update `Documentation-Index.md` for new pages
- Cross-reference related pages
- Keep wiki README updated

### Version Control
- Commit wiki/ changes to main repository first
- Then sync to GitHub Wiki
- Document major wiki changes in commit messages

## Publishing Checklist

Before publishing to GitHub Wiki:

- [ ] All markdown files have proper formatting
- [ ] All internal links use relative paths
- [ ] _Sidebar.md is complete and organized
- [ ] Home.md provides clear navigation
- [ ] No broken links (test locally first)
- [ ] Images and assets are included
- [ ] README.md explains wiki structure
- [ ] Documentation-Index.md is complete

After publishing to GitHub Wiki:

- [ ] Home page loads correctly
- [ ] Sidebar navigation appears on all pages
- [ ] All links work as expected
- [ ] Pages are properly categorized
- [ ] Search functionality works
- [ ] Images display correctly
- [ ] Mobile view is usable

## Future Enhancements

### Planned Improvements
- 📸 Screenshot tutorials
- 🎥 Video embed support
- 📊 Architecture diagrams
- 🗺️ Interactive roadmap
- 📱 Mobile-optimized layouts
- 🔍 Enhanced search functionality

### Potential Tools
- **GitBook** - Alternative documentation platform
- **Docusaurus** - Static site generator for documentation
- **MkDocs** - Python-based documentation generator
- **Jekyll** - GitHub Pages compatible static site

## Resources

- **GitHub Wiki Documentation**: https://docs.github.com/en/communities/documenting-your-project-with-wikis
- **Markdown Guide**: https://www.markdownguide.org/
- **GitHub Flavored Markdown**: https://github.github.com/gfm/

## Questions?

- **How do I publish?** Follow Method 1 above
- **Can I automate it?** Yes, see Method 2
- **Do I edit in wiki or repo?** Always edit in main repository
- **How often to sync?** After significant changes or weekly

---

**The wiki system provides organized, easy-to-navigate documentation for all Adastrea users.**

*Last Updated: November 23, 2025*
