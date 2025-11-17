# Mittenzx Tasks - Next 2 Weeks
## Detailed Checklist for New Team Member

**Welcome to the Adastrea project!** This document provides your tasks for the next two weeks with detailed context, learning resources, and step-by-step guidance.

**Your Role**: Visual/Content Lead  
**Period**: November 16-30, 2025 (Week 1-2)  
**Estimated Core Workload**: 11-14 days  
**Focus Areas**: Visual effects, content creation, UX/tutorials, audio design

---

## 📚 Getting Started Resources

Before diving into tasks, familiarize yourself with these essential documents:

### Quick Start (Read These First!)
- **CONTENT_CREATION_QUICKSTART.md** - How to create content in Adastrea
- **Assets/DesignerOnboarding.md** - Designer onboarding guide
- **README.md** - Project overview

### Reference Materials
- **ARCHITECTURE.md** - Understand how systems connect
- **ROADMAP.md** - Current project status
- **Assets/TutorialSystemGuide.md** - Tutorial system documentation
- **Assets/AudioSystemGuide.md** - Audio system documentation
- **PROCEDURAL_GENERATION_GUIDE.md** - Using procedural tools

### Tools and Scripts
- **TESTING_AUTOMATION_GUIDE.md** - How to test your work
- **SCENE_POPULATION_GUIDE.md** - Populating scenes with content

---

## 🎯 Overview of Your Two-Week Goals

You'll be working on the **visual polish and content creation** that will make Adastrea come alive! Your work will directly impact how players experience combat, space flight, and learning the game.

**Week 1 Focus**: Create stunning visual effects for combat and start building content (achievements, tutorials, audio)  
**Week 2 Focus**: Polish the overall visual presentation and finalize alpha build preparation

---

## Week 1: November 16-23, 2025

### 🔥 P0.1: Combat Visual Effects (CRITICAL - Must Complete)

**Time Estimate**: 3-4 days  
**Why This Matters**: Combat is a core part of the game - players need visual feedback!

#### What You're Creating
Visual effects for 6 weapon types, impacts, shields, and heat management.

**Tasks**:
- [ ] Design weapon fire effects for 6 types
  - [ ] Energy weapons: Laser beam with glow and heat distortion
  - [ ] Projectile weapons: Tracer rounds with muzzle flash
  - [ ] Missile weapons: Smoke trail and engine glow
  - [ ] Torpedo weapons: Plasma trail effect
  - [ ] Beam weapons: Sustained beam with start/end points
  - [ ] Plasma weapons: Ball lightning with arcing
- [ ] Create impact effects
  - [ ] Shield impact: Hexagonal ripple with energy dissipation
  - [ ] Hull impact: Sparks, debris, damage decals
  - [ ] Explosion effects: Small/Medium/Large scales
- [ ] Design shield recharge feedback
  - [ ] Shield down: Red glow on damaged facing
  - [ ] Recharging: Blue pulse
  - [ ] Full shield: Green steady glow
- [ ] Add weapon heat effects
  - [ ] Cooling vents with steam/heat shimmer
  - [ ] Overheating warning: Orange→red gradient

**Where to Start**:
1. Open Unreal Editor
2. Create the directory structure `Content/VFX/Combat/Weapons/` (if it does not already exist)
3. Navigate to `Content/VFX/Combat/Weapons/`
4. Study existing particle system examples
5. Create your first effect (Energy Weapon - easiest to start)
6. Test in a combat test map located in `Content/Maps/` (e.g., `TestMap_Combat.umap` if available)

**Performance Goal**: <5ms per effect

**Deliverables**:
- Particle system blueprints for all 6 weapon types
- Shield effect materials with directional support
- Impact effect templates (3 sizes: S/M/L)
- Heat management visual feedback system

**Testing**: Load combat test map, fire all weapons, verify effects visible at various distances

**Need Help?**: Check `Assets/CombatSystemGuide.md` and Unreal's Niagara documentation

---

### ⚡ P1.1: Achievement System Content (High Priority)

**Time Estimate**: 2 days  
**Target**: 30-40 achievements (minimum viable scope)  
**Why This Matters**: Achievements guide players and provide goals!

#### What You're Creating
Achievement content across different gameplay areas.

**Tasks**:
- [ ] Design achievement categories
  - [ ] Exploration (5 achievements)
  - [ ] Combat (5 achievements)
  - [ ] Trading (5 achievements)
  - [ ] Faction (8 achievements)
  - [ ] Station Building (5 achievements)
  - [ ] Personnel Management (5 achievements)
  - [ ] Secret Achievements (2-5 hidden)
- [ ] Create achievement Data Assets
  - [ ] Define unlock criteria
  - [ ] Set reward values (XP, credits, reputation)
  - [ ] Design achievement icons (placeholder OK)
  - [ ] Write descriptions and flavor text
- [ ] Implement progression milestones
  - [ ] Early game (0-5 hours)
  - [ ] Mid game (5-20 hours)
  - [ ] Late game (20+ hours)
- [ ] Test achievement unlocking

**How to Create**:
1. If the `Content/DataAssets/Achievements/` directory does not exist, create it in the Unreal Editor's Content Browser.
2. Navigate to `Content/DataAssets/Achievements/`
3. Right-click → Blueprint → Achievement Data Asset
4. Name it (e.g., `DA_Achievement_FirstContact`)
5. Fill in properties
6. Save and test!

**Example Achievement**:
- **Name**: "First Contact"
- **Description**: "Visit your first space station"
- **Criteria**: OnStationDocked event
- **Rewards**: 100 XP, 1000 credits
- **Icon**: Placeholder blue square

**Deliverables**:
- 30-40 achievement Data Assets
- Achievement icon placeholders
- Achievement progression guide document

**Testing**: Trigger each achievement manually, verify notification appears, check rewards delivered

---

### ⚡ P1.2: Tutorial Content Creation (High Priority)

**Time Estimate**: 2-3 days  
**Target**: 3 core tutorials minimum  
**Why This Matters**: New players need guidance or they'll quit!

#### What You're Creating
Interactive tutorial sequences teaching game basics.

**Tasks**:
- [ ] Design 3 core tutorials
  - [ ] Tutorial 1: Basic Controls & Space Flight (5-7 min)
  - [ ] Tutorial 2: Combat Basics (5-7 min)
  - [ ] Tutorial 3: Trading Fundamentals (3-5 min)
- [ ] Create tutorial step Data Assets (5-8 steps per tutorial)
  - [ ] Write instructions
  - [ ] Define UI highlights
  - [ ] Set completion criteria
- [ ] Implement skip/replay
  - [ ] "Skip Tutorial" option in settings
  - [ ] "Replay Tutorial" in help menu
  - [ ] Track completion progress
- [ ] Test effectiveness

**Tutorial 1 Example Steps**:
1. "Press W to move forward"
2. "Press S to slow down"
3. "Move mouse to turn"
4. "Press Space to boost"
5. "Navigate to waypoint"
6. "Approach station"
7. "Press F to dock"

**How to Create**:
1. If `Content/DataAssets/Tutorials/` does not exist, create the directory.
2. Navigate to `Content/DataAssets/Tutorials/`
3. Create Tutorial Step Data Asset
4. Fill in instruction text, UI highlight, completion criteria
5. Link steps together (Step1 → Step2 → Step3)

**Deliverables**:
- 3 tutorial sequences (25-30 total steps)
- Tutorial step Data Assets
- Tutorial UI flow documented

**Testing**: Complete each tutorial start-to-finish, verify all steps work

---

### ⚡ P1.3: Audio Asset Creation (If Time Permits)

**Time Estimate**: 2-3 days  
**Target**: 40-50 sounds, 4-5 music tracks (minimum viable scope)  
**Why This Matters**: Audio brings the game to life!

#### What You're Creating
Sound effects and music for combat, UI, engines.

**Priority 1 - Combat Sounds** (10 sounds):
- [ ] Weapon fire (energy, projectile, missile, explosion)
- [ ] Impacts (shield, hull)
- [ ] Warnings (low shield, hull breach)

**Priority 2 - UI Sounds** (8 sounds):
- [ ] Button click, hover
- [ ] Achievement unlock
- [ ] Notifications
- [ ] Menu open/close
- [ ] Error/success sounds

**Priority 3 - Engine Sounds** (5 sounds):
- [ ] Main engine thrust
- [ ] RCS thrusters
- [ ] Jump drive charge/activation
- [ ] Engine damage

**Music Tracks** (4-5 essential):
- [ ] Combat music (intense, 2-3 min)
- [ ] Exploration music (ambient, 3-4 min)
- [ ] Menu music (neutral, 2-3 min)
- [ ] Trading music (upbeat, 2-3 min)

**Where to Source Audio**:
- **Freesound.org** - CC0 or CC-BY licensed sounds
- **OpenGameArt.org** - Game-ready SFX
- **Incompetech.com** - Royalty-free music
- **BBC Sound Effects** - Professional quality

**Technical Requirements**:
- Format: WAV, 16-bit, 44.1kHz
- Max size: 5MB per SFX, 20MB per music track
- Naming: `SE_Combat_LaserFire_01.wav`, `MUS_Exploration_01.wav`

**How to Implement**:
1. Create the directory `Content/Audio/` in your project if it does not already exist.
2. Import WAV files to `Content/Audio/`
3. Create SoundCue assets for complex sounds
4. Set up attenuation for 3D sounds
5. Configure music transitions

**Deliverables**:
- 40-50+ sound effects
- 4-5+ music tracks
- SoundCue assets
- Audio integration documentation

**Testing**: Play all sounds in-game, verify 3D positioning, check volume balance

---

## Week 2: November 24-30, 2025

### 🔥 P0.4: Post-Processing and Visual Polish (CRITICAL)

**Time Estimate**: 2-3 days  
**Why This Matters**: Final layer of polish for professional look!

#### What You're Creating
Post-processing effects and environment-specific visual profiles.

**Tasks**:
- [ ] Configure post-process volume settings
  - [ ] Bloom (makes lights glow)
  - [ ] Motion blur (subtle, optional)
  - [ ] Lens flares (for stars)
  - [ ] Screen space reflections (cockpit glass)
- [ ] Create 5 environment profiles
  - [ ] Deep Space: Cool tones, high contrast
  - [ ] Near Star: Warm tones, strong lens flares
  - [ ] Nebula: Vibrant colors, atmospheric
  - [ ] Station Interior: Neutral, well-lit
  - [ ] Combat Mode: Desaturated, red tint
- [ ] Implement camera effects
  - [ ] Camera shake for weapon fire
  - [ ] Camera shake for impacts
  - [ ] Vignette for intense moments
- [ ] Polish lighting
  - [ ] Adjust directional light (main star)
  - [ ] Add fill lights for visibility
  - [ ] Configure skylight (ambient)
  - [ ] Set up light shafts (god rays)
- [ ] Test across quality settings (Low/Med/High/Ultra)

**How to Configure**:
1. Add Post Process Volume to level
2. Set to "Infinite Extent"
3. Adjust settings in Details panel
4. Press Play to see effects real-time

**Performance Testing**:
- Low: 60 FPS on GTX 1060
- High: 60 FPS on RTX 3060
- No visual regression on low settings

**Deliverables**:
- 5 post-process volume presets
- Camera shake blueprints
- Lighting configuration guide
- Visual quality settings documentation

---

### 🔥 P0.6: Alpha Build Preparation (CRITICAL - Collaborative)

**Time Estimate**: 2 days  
**Why This Matters**: Final checkpoint before alpha release!

#### What You're Doing
Test everything, fix critical bugs, prepare for alpha testing.

**Manual Testing Tasks** (1 day):
- [ ] Core Gameplay Loop (2 hours)
  - [ ] Start new game → Tutorial → Quest → Combat → Save/Load
  - [ ] Goal: Complete full loop without crashes
- [ ] Ship System (30 min)
  - [ ] Test all 6 ship templates
- [ ] Combat System (30 min)
  - [ ] Test all 6 weapon types
- [ ] Trading System (20 min)
- [ ] Quest System (20 min)
- [ ] Station Building (20 min)
- [ ] Tutorial System (20 min)
- [ ] UI/HUD (20 min)

**Create Bug List**:
- Document every bug found
- Categorize: P0 (blocks alpha), P1 (should fix), P2 (defer)
- Include steps to reproduce

**Content Verification** (0.5 days):
- [ ] All VFX present and working
- [ ] All achievements present
- [ ] All tutorials working
- [ ] All audio playing
- [ ] Post-processing active

**Create Alpha Release Notes** (0.5 days):
- [ ] Document visual features
- [ ] List content features (achievements, tutorials, audio)
- [ ] Note known issues
- [ ] Player-facing content description

**Your Sections**:
- Visual Features (VFX, post-processing)
- Content Features (achievements, tutorials, audio)
- Known Issues (bugs deferred)
- Testing Guidelines

**Deliverables**:
- Bug list (comprehensive, categorized)
- Content verification checklist
- Alpha release notes (your sections)
- Alpha testing guidelines

**Collaboration**: Work closely with Copilot! Daily check-ins during these 2 days.

---

### ⚡ P1.5: Content Creation Pipeline Documentation (If Time Permits)

**Time Estimate**: 1-2 days  
**Why This Matters**: Prepares for Phase 5 content creation sprint!

#### What You're Creating
Documentation for all content creation workflows.

**Tasks**:
- [ ] Create content creation guide
  - [ ] Ship creation workflow
  - [ ] Station module creation workflow
  - [ ] Character/personnel creation workflow
  - [ ] Quest creation workflow
  - [ ] Audio asset integration workflow
  - [ ] Visual effect creation workflow
- [ ] Document procedural generation usage
  - [ ] Using ProceduralGenerators.py
  - [ ] Customizing templates
  - [ ] Importing generated content
  - [ ] Quality checking
- [ ] Create asset standards guide
  - [ ] 3D model standards (poly counts, resolutions)
  - [ ] Texture standards (formats, compression)
  - [ ] Audio standards (format, quality)
  - [ ] File organization
- [ ] Document QA process
  - [ ] Asset validation checklist
  - [ ] Playtesting guidelines
  - [ ] Bug reporting template
  - [ ] Performance testing requirements

**Deliverables**:
- Content Creation Pipeline Guide
- Asset Standards Guide
- QA and Testing Guidelines
- Procedural Generation Usage Guide

**Purpose**: Essential for Phase 5 when content creation scales up!

---

## 🎯 Your Success Criteria

### Must Complete (P0 - Critical)
- ✅ P0.1: Combat Visual Effects (all 6 types + impacts + shields)
- ✅ P0.4: Post-Processing and Visual Polish (5 profiles)
- ✅ P0.6: Alpha Build Preparation (testing, release notes)

### Should Complete (P1 - Minimum Viable Scope)
- ✅ P1.1: Achievement System Content (30-40 achievements)
- ✅ P1.2: Tutorial Content (3 core tutorials)
- ⭕ P1.3: Audio Assets (40-50 sounds, 4-5 music) - if time permits
- ⭕ P1.5: Content Pipeline Docs - if time permits

### Success = All P0 Complete + Minimum Viable P1 Goals

---

## 📋 Daily Check-In Template

```
Date: [Today's Date]
Person: Mittenzx

Yesterday:
- [What you completed]

Today:
- [What you're working on]

Blockers:
- [Any issues]

Help Needed:
- [Ask Copilot]
```

---

## 🆘 When You Need Help

**Technical Issues**: Check guides in `Assets/` or ask Copilot  
**Design Questions**: Review examples, check similar games, discuss with Copilot  
**Scope Questions**: Focus P0 → P1 (minimum viable) → P2 (if time)

**Remember**: Ask for help early! Better to ask than get stuck.

---

## 🎉 After 2 Weeks

- Update ROADMAP.md with completed tasks
- Update CHANGELOG.md with contributions
- Celebrate completing Phase 4! 🎊

**Then**: Phase 5 begins - Content Creation Sprint!

---

## 📚 Quick Reference Links

**Essential Docs**:
- [CONTENT_CREATION_QUICKSTART.md](CONTENT_CREATION_QUICKSTART.md)
- [Assets/DesignerOnboarding.md](Assets/DesignerOnboarding.md)
- [Assets/CombatSystemGuide.md](Assets/CombatSystemGuide.md)
- [Assets/TutorialSystemGuide.md](Assets/TutorialSystemGuide.md)
- [Assets/AudioSystemGuide.md](Assets/AudioSystemGuide.md)
- [ROADMAP.md](ROADMAP.md)

**Content Directories**:
- Visual Effects: `Content/VFX/` (create if needed)
- Audio: `Content/Audio/` (create if needed)
- Data Assets: `Content/DataAssets/` (create if needed)
- Blueprints: `Content/Blueprints/` (exists)
- Materials: `Content/Materials/` (create if needed)
- UI: `Content/UI/` (exists)

---

**Document Version**: 1.0  
**Created**: November 17, 2025  
**Target Completion**: November 30, 2025  
**Your work will bring Adastrea to life!** 🚀
