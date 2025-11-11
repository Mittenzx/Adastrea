# Faction System Setup Guide

## Overview
The Faction System allows designers to create and manage large political and military organizations within the game using Blueprint Data Assets. Each faction has unique properties that define its identity, political relationships, and military/technological attributes.

**Important:** Factions focus on **politics, warfare, and diplomacy**. For economic activities, trade, and industry specialization, see the [Way System](WaySystemGuide.md) which handles specialized guilds and micro-alliances.

## Setup Instructions

### 1. Creating a Faction Data Asset

1. In the Unreal Editor, navigate to the Content Browser
2. Right-click in the desired folder
3. Select **Miscellaneous > Data Asset**
4. Choose **FactionDataAsset** as the class
5. Name the asset (e.g., `DA_Faction_SolarisUnion`)
6. Double-click to open and configure the faction properties

### 2. Assigning Factions to Space Stations

1. Select a Space Station actor in the level or open a Space Station Blueprint
2. In the Details panel, find the **Station** category
3. Set the **Faction** property to the desired Faction Data Asset
4. The station will now belong to that faction

### 3. Accessing Faction Data in Blueprints

Use the following functions to work with factions:
- `Get Faction` - Returns the faction data asset for a station
- `Set Faction` - Assigns a faction to a station

## Faction Properties Reference

### Faction Info
- **Faction Name** (Text): Display name of the faction
- **Description** (Multi-line Text): Brief description of the faction's purpose and characteristics
- **Primary Color** (Linear Color): Main faction color for UI and visual elements
- **Secondary Color** (Linear Color): Accent color for faction branding
- **Faction ID** (Name): Unique identifier for programmatic access

### Faction Relations
- **Initial Reputation** (Integer -100 to 100): Starting reputation with the player
  - -100 to -50: Hostile
  - -49 to -1: Unfriendly
  - 0 to 49: Neutral
  - 50 to 100: Friendly
- **Hostile By Default** (Boolean): Whether the faction attacks player on sight

### Faction Attributes
- **Technology Level** (Integer 1-10): Technological advancement (affects available equipment/modules)
- **Military Strength** (Integer 1-10): Combat capability and fleet size
- **Economic Power** (Integer 1-10): Overall economic scale and resources (Note: Day-to-day trade and industry is handled by Ways within the faction)

---

## Faction Templates

Below are templates for 10 factions. Copy these into your data assets and adjust as needed.

### 1. Solaris Union

**Faction Properties:**
- **Faction Name:** Solaris Union
- **Faction ID:** `SolarisUnion`
- **Description:** 
  ```
  The Solaris Union is a democratic federation of solar systems dedicated to exploration, 
  scientific advancement, and peaceful cooperation. Known for their diplomatic prowess and 
  advanced research facilities, they maintain a vast network of trading posts and research stations.
  ```
- **Primary Color:** Gold (R: 1.0, G: 0.84, B: 0.0)
- **Secondary Color:** Deep Blue (R: 0.0, G: 0.2, B: 0.4)
- **Initial Reputation:** 25
- **Hostile By Default:** False
- **Technology Level:** 8
- **Military Strength:** 6
- **Economic Power:** 9

---

### 2. Ironclad Consortium

**Faction Properties:**
- **Faction Name:** Ironclad Consortium
- **Faction ID:** `IroncladConsortium`
- **Description:**
  ```
  A powerful corporate alliance controlling the majority of heavy industry and mining operations 
  across the outer rim. The Ironclad Consortium values profit and efficiency above all else, 
  maintaining order through military contracts and economic leverage.
  ```
- **Primary Color:** Steel Gray (R: 0.5, G: 0.5, B: 0.5)
- **Secondary Color:** Industrial Orange (R: 0.9, G: 0.5, B: 0.1)
- **Initial Reputation:** 0
- **Hostile By Default:** False
- **Technology Level:** 7
- **Military Strength:** 8
- **Economic Power:** 10

---

### 3. Verdant Horizon

**Faction Properties:**
- **Faction Name:** Verdant Horizon
- **Faction ID:** `VerdantHorizon`
- **Description:**
  ```
  An eco-focused collective dedicated to terraforming and preserving habitable worlds. 
  The Verdant Horizon operates bio-domes and agricultural stations, providing food and 
  environmental technology to struggling colonies across the galaxy.
  ```
- **Primary Color:** Forest Green (R: 0.13, G: 0.55, B: 0.13)
- **Secondary Color:** Earth Brown (R: 0.55, G: 0.27, B: 0.07)
- **Initial Reputation:** 15
- **Hostile By Default:** False
- **Technology Level:** 7
- **Military Strength:** 3
- **Economic Power:** 6

---

### 4. Nova Vanguard

**Faction Properties:**
- **Faction Name:** Nova Vanguard
- **Faction ID:** `NovaVanguard`
- **Description:**
  ```
  An elite military organization sworn to protect frontier colonies from pirate raids and 
  alien threats. The Nova Vanguard operates with strict codes of honor and maintains the 
  most advanced fleet of warships in known space.
  ```
- **Primary Color:** Crimson Red (R: 0.86, G: 0.08, B: 0.24)
- **Secondary Color:** Military Gray (R: 0.3, G: 0.3, B: 0.3)
- **Initial Reputation:** 10
- **Hostile By Default:** False
- **Technology Level:** 9
- **Military Strength:** 10
- **Economic Power:** 5

---

### 5. Celestial Syndicate

**Faction Properties:**
- **Faction Name:** Celestial Syndicate
- **Faction ID:** `CelestialSyndicate`
- **Description:**
  ```
  A shadowy criminal organization controlling smuggling routes and black markets throughout 
  the galaxy. The Celestial Syndicate operates through fear and bribery, offering 'protection' 
  to those who pay and swift retribution to those who don't.
  ```
- **Primary Color:** Deep Purple (R: 0.29, G: 0.0, B: 0.51)
- **Secondary Color:** Dark Red (R: 0.5, G: 0.0, B: 0.0)
- **Initial Reputation:** -20
- **Hostile By Default:** False
- **Technology Level:** 6
- **Military Strength:** 7
- **Economic Power:** 8

---

### 6. Stellar Accord

**Faction Properties:**
- **Faction Name:** Stellar Accord
- **Faction ID:** `StellarAccord`
- **Description:**
  ```
  A pan-galactic peacekeeping organization dedicated to mediating conflicts and maintaining 
  interstellar law. The Stellar Accord operates neutral zones and diplomatic stations, 
  working to prevent wars and promote cooperation among all factions.
  ```
- **Primary Color:** Peaceful Blue (R: 0.25, G: 0.5, B: 0.9)
- **Secondary Color:** White (R: 0.95, G: 0.95, B: 0.95)
- **Initial Reputation:** 30
- **Hostile By Default:** False
- **Technology Level:** 7
- **Military Strength:** 5
- **Economic Power:** 7

---

### 7. Titan Watch

**Faction Properties:**
- **Faction Name:** Titan Watch
- **Faction ID:** `TitanWatch`
- **Description:**
  ```
  An isolationist faction controlling the resource-rich Titan sector. Known for their massive 
  fortress stations and defensive fleets, Titan Watch fiercely protects their territory from 
  any outsiders, viewing all other factions with suspicion.
  ```
- **Primary Color:** Dark Teal (R: 0.0, G: 0.5, B: 0.5)
- **Secondary Color:** Gunmetal (R: 0.42, G: 0.47, B: 0.50)
- **Initial Reputation:** -10
- **Hostile By Default:** False
- **Technology Level:** 8
- **Military Strength:** 9
- **Economic Power:** 6

---

### 8. Aurora Collective

**Faction Properties:**
- **Faction Name:** Aurora Collective
- **Faction ID:** `AuroraCollective`
- **Description:**
  ```
  A confederation of artists, philosophers, and scientists seeking to push the boundaries 
  of consciousness and reality. The Aurora Collective operates experimental stations studying 
  exotic phenomena, often conducting controversial research into AI and genetic modification.
  ```
- **Primary Color:** Aurora Pink (R: 0.9, G: 0.4, B: 0.7)
- **Secondary Color:** Cosmic Blue (R: 0.2, G: 0.4, B: 0.8)
- **Initial Reputation:** 5
- **Hostile By Default:** False
- **Technology Level:** 10
- **Military Strength:** 2
- **Economic Power:** 4

---

### 9. Obsidian Order

**Faction Properties:**
- **Faction Name:** Obsidian Order
- **Faction ID:** `ObsidianOrder`
- **Description:**
  ```
  A mysterious authoritarian regime operating from hidden bases in uncharted space. The 
  Obsidian Order seeks to impose their vision of 'perfect order' across the galaxy, using 
  advanced surveillance technology and ruthless military force to eliminate dissent.
  ```
- **Primary Color:** Black (R: 0.1, G: 0.1, B: 0.1)
- **Secondary Color:** Blood Red (R: 0.7, G: 0.0, B: 0.0)
- **Initial Reputation:** -40
- **Hostile By Default:** True
- **Technology Level:** 9
- **Military Strength:** 9
- **Economic Power:** 7

---

### 10. Galactic Guild

**Faction Properties:**
- **Faction Name:** Galactic Guild
- **Faction ID:** `GalacticGuild`
- **Description:**
  ```
  An ancient trade organization with roots predating modern space travel. The Galactic Guild 
  maintains neutral trading hubs throughout the galaxy, offering banking services, commodity 
  exchanges, and contract work to anyone with credits to spend.
  ```
- **Primary Color:** Merchant Gold (R: 0.83, G: 0.69, B: 0.22)
- **Secondary Color:** Royal Blue (R: 0.25, G: 0.41, B: 0.88)
- **Initial Reputation:** 20
- **Hostile By Default:** False
- **Technology Level:** 6
- **Military Strength:** 4
- **Economic Power:** 10

---

## Best Practices

### Balancing Factions
- Avoid making all attributes high (max 8) or low (min 3) for any single faction
- Consider faction relationships when designing their territories
- Use reputation values to create interesting player choices

### Visual Design
- Choose distinct color combinations for easy identification
- Consider color theory (complementary colors work well for primary/secondary)
- Ensure colors are accessible and visible in space environments

### Narrative Integration
- Reference faction relationships in descriptions
- Create logical connections between attributes and lore
- Leave room for faction development through gameplay

### Technical Considerations
- Always use unique Faction IDs (no spaces or special characters)
- Test faction colors in-game before finalizing
- Keep descriptions concise but evocative (2-4 sentences)

---

## Advanced Usage

### Blueprint Examples

**Checking Station Faction:**
```
Get Faction (Space Station Reference)
  ↓
Branch (Is Valid?)
  ↓ True
  Get Faction Name
  ↓ False
  "No Faction"
```

**Setting Faction Reputation:**
```
Get Faction (Space Station Reference)
  ↓
Set Initial Reputation (New Value)
```

### Future Expansion Ideas
- Faction-specific missions and contracts
- Dynamic reputation system
- Inter-faction diplomacy and alliances
- Faction-themed equipment and modules
- Territory control mechanics

---

## Troubleshooting

**Q: My faction data isn't showing up in the station properties**
- Ensure the FactionDataAsset is saved
- Check that the asset is in a Content folder (not in Source)
- Verify the asset class is correct (should show FactionDataAsset)

**Q: Colors look different in-game vs. the editor**
- Check your post-processing settings
- Verify lighting doesn't wash out colors
- Test in different environments

**Q: Can I change faction properties at runtime?**
- Data Assets are meant to be immutable templates
- For runtime changes, create a separate faction state system
- Consider duplicating the data asset for modified versions

---

## Support

For questions or issues with the faction system, contact the development team or refer to the engine documentation for Data Assets and Blueprint integration.

---

## Advanced Features

### Traits System

Factions can now have traits that provide unique bonuses or modifiers. See [BlueprintFactionAssetGuide.md](BlueprintFactionAssetGuide.md) for detailed trait creation instructions.

**Example Traits:**
```
Elite Training
  - Trait ID: EliteTraining
  - Modifier: 0.25
  - Effect: +25% combat effectiveness

Trade Hub
  - Trait ID: TradeHub
  - Modifier: 0.15
  - Effect: +15% trade income
```

### Diplomacy System

Factions can define **political relationships** with other factions including alliances, wars, and diplomatic agreements. See [BlueprintFactionAssetGuide.md](BlueprintFactionAssetGuide.md) for diplomacy setup.

**Note:** While factions have political alliances, **economic relationships and trade networks** are handled by the [Way System](WaySystemGuide.md). Ways are specialized guilds that form micro-alliances (2-5 members) within or across factions for economic cooperation.

**Example Relationships:**
```
Nova Vanguard ↔ Solaris Union
  - Relationship: 60 (friendly)
  - Allied: Yes
  - Trade Modifier: 1.3
```

---

## Additional Documentation

**For comprehensive guides, see:**

- **[BlueprintFactionAssetGuide.md](BlueprintFactionAssetGuide.md)** - Complete step-by-step faction creation guide
  - Includes trait system setup
  - Includes diplomacy configuration
  - Includes Blueprint integration examples
  
- **[BlueprintWorkflowTemplates.md](BlueprintWorkflowTemplates.md)** - Blueprint UI workflows
  - Faction selection UI
  - Sample event graphs
  - Best practices
  
- **[PlaytestingChecklist.md](PlaytestingChecklist.md)** - Verification and testing guide
  - Asset verification
  - Trait system testing
  - Diplomacy testing
  
- **[IterationGuide.md](IterationGuide.md)** - Expanding faction systems
  - Runtime trait system
  - Dynamic diplomacy
  - Future features
  
- **[DesignerOnboarding.md](DesignerOnboarding.md)** - Team onboarding guide
  - Quick start tutorials
  - Best practices
  - Common pitfalls
