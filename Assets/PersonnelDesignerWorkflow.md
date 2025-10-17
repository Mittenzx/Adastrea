# Personnel Designer Workflow Guide

## Overview

This guide provides step-by-step workflows for common personnel management tasks. Follow these workflows to efficiently create, manage, and integrate personnel into your game.

## Table of Contents

1. [Creating Individual Personnel](#workflow-1-creating-individual-personnel)
2. [Building a Ship Crew](#workflow-2-building-a-ship-crew)
3. [Populating a Space Station](#workflow-3-populating-a-space-station)
4. [Creating Command Hierarchies](#workflow-4-creating-command-hierarchies)
5. [Establishing Relationships](#workflow-5-establishing-relationships)
6. [Managing Personnel Teams](#workflow-6-managing-personnel-teams)
7. [Tracking Career Progression](#workflow-7-tracking-career-progression)
8. [Integrating with Blueprints](#workflow-8-integrating-with-blueprints)

---

## Workflow 1: Creating Individual Personnel

### Step-by-Step Process

#### 1. Choose a Role Template
- Open `Assets/PersonnelTemplates/` folder
- Select the template matching your desired role
- Review the template to understand typical values

#### 2. Create the Data Asset
1. Open Unreal Editor
2. Navigate to your Content Browser
3. Create a folder structure (recommended):
   ```
   Content/
     Personnel/
       Ship_[ShipName]/
         Command/
         Flight/
         Engineering/
         Medical/
         Support/
       Station_[StationName]/
         Management/
         Staff/
   ```
4. Right-click in the appropriate folder
5. Select **Miscellaneous > Data Asset**
6. Choose **PersonnelDataAsset** as the class
7. Name using convention: `DA_Personnel_[Role]_[Name]`
   - Example: `DA_Personnel_Captain_Smith`

#### 3. Configure Identity
1. Open the newly created Data Asset
2. Expand **Personnel | Identity** category
3. Fill in basic information:
   - **Personnel Name**: Full display name
   - **Personnel ID**: Unique code-friendly identifier (no spaces)
   - **Biography**: Write a brief background story
   - **Age**: Appropriate for role (reference template)
   - **Gender, Species, Nationality**: Set as desired
   - **Tags**: Add relevant tags for filtering

#### 4. Set Role and Assignment
1. Expand **Personnel | Role** category
2. **Primary Role**: Select from dropdown
3. **Current Assignment**: Enter ship/station name
4. **Department**: Enter appropriate department
5. Leave **Manager ID** and **Direct Reports** empty for now (set later)

#### 5. Configure Skills
1. Expand **Personnel | Skills** category
2. **Overall Skill Level**: Set 1-10 based on role (see template)
3. **Total Experience**: Set appropriate value
4. **Skills Array**: Click '+' to add skills
   - For each skill:
     - Enter Skill Name
     - Set Skill Level (1-10)
     - Set Experience Points
5. **Specialties**: Add 2-4 specialty areas

#### 6. Set Status Values
1. Expand **Personnel | Status** category
2. Set initial values:
   - **Morale**: 50-80 (higher for happy crew)
   - **Health**: 80-100 (lower if injured)
   - **Fatigue**: 0-30 (higher if tired)
   - **Loyalty**: 50-80 (higher for dedicated crew)
   - **Reputation**: -20 to 60 (based on background)

#### 7. Configure Employment
1. Expand **Personnel | Employment** category
2. **Salary**: Set based on role (reference template)
3. **Contract Duration**: 0 for permanent, or months for contract
4. **Contract Months Remaining**: Equal to duration initially

#### 8. Add Personality and Traits
1. Expand **Personnel | Personality** category
2. **Personality Type**: One-word descriptor
3. **Personality Description**: Detailed character notes
4. **Traits Array**: Click '+' to add 2-4 traits
   - For each trait:
     - **Trait Name**: Display name
     - **Trait Description**: What it does
     - **Trait ID**: Code-friendly identifier
     - **Modifier Value**: Numeric effect (1.0 = no change)

#### 9. Save and Test
1. Click **Save** in the Data Asset editor
2. Close the editor
3. Verify the asset appears in Content Browser

---

## Workflow 2: Building a Ship Crew

### Creating a Complete Ship Crew

#### Phase 1: Plan the Crew
1. Determine ship size and required roles:
   - **Small Ship** (10-20 crew): Captain, Pilot, Engineer, Medic, 2-3 Gunners, Support
   - **Medium Ship** (30-50 crew): Add XO, Navigator, Sci Officer, Security, More specialists
   - **Large Ship** (100+ crew): Full department heads, junior officers, large support staff

#### Phase 2: Create Command Staff
1. Create Captain (see Workflow 1)
2. Create XO (if applicable)
3. Create Department Heads:
   - Chief Engineer
   - Chief Medical Officer
   - Gunnery Chief
   - Head of Security

#### Phase 3: Create Specialists
1. Create technical specialists:
   - Pilot(s)
   - Navigator
   - Engineers (2-4)
   - Medical Officers
2. Create combat specialists:
   - Gunners (2-6)
   - Security Officers (2-4)

#### Phase 4: Create Support Staff
1. Create support roles:
   - Quartermaster
   - Communications Officer
   - Chef
   - Maintenance Crew (2-4)

#### Phase 5: Establish Hierarchy
1. Open Captain's Data Asset
2. In **Direct Reports**, add Personnel IDs of:
   - XO
   - All department heads
3. For each department head:
   - Set their **Manager ID** to Captain's ID
   - Add their team members to **Direct Reports**
4. For each team member:
   - Set **Manager ID** to their department head's ID

#### Phase 6: Create Relationships
- See [Workflow 5: Establishing Relationships](#workflow-5-establishing-relationships)

#### Phase 7: Set Current Assignment
1. Decide on ship name
2. For each crew member:
   - Set **Current Assignment** to ship name
   - Verify **Department** is appropriate

---

## Workflow 3: Populating a Space Station

### Creating Station Personnel

#### Phase 1: Determine Station Type
- **Trading Station**: Emphasize traders, quartermasters, staff
- **Military Station**: Security, gunnery, tactical personnel
- **Research Station**: Scientists, researchers, engineers
- **Mixed Station**: Balanced mix of all types

#### Phase 2: Create Management
1. Create Station Manager
2. Create Deputy Managers (if large station):
   - Operations Manager
   - Security Chief
   - Trade Coordinator

#### Phase 3: Create Department Staff
1. **Security Department**:
   - Security Chief
   - 4-10 Security Officers (based on size)
2. **Operations Department**:
   - Engineers (2-5)
   - Maintenance Staff (3-8)
   - Quartermaster
3. **Commerce Department**:
   - Trade Coordinator
   - Traders (2-4)
   - Bartender
   - Chef

#### Phase 4: Create Module Staff
1. For each station module, create Module Staff:
   - Docking Bay Staff (2 per bay)
   - Trade Hub Staff (2-3)
   - Medical Bay Staff (1-2)
   - Power Module Staff (1-2)

#### Phase 5: Create Support Personnel
1. Station Staff (general) (5-15)
2. Janitors (2-5)
3. Entertainment (1-3)
4. Civilians (variable)

#### Phase 6: Establish Reporting Structure
1. Station Manager oversees:
   - All department heads
   - Direct station staff
2. Department heads oversee:
   - Their respective teams
3. Module Staff report to:
   - Operations Manager or relevant department head

---

## Workflow 4: Creating Command Hierarchies

### Establishing Organizational Structure

#### Step 1: Design the Hierarchy
Draw out your organizational chart:
```
Captain
├── XO
│   ├── Operations Officer
│   │   ├── Crew Member 1
│   │   └── Crew Member 2
│   └── Communications Officer
├── Chief Engineer
│   ├── Engineer 1
│   ├── Engineer 2
│   └── Engineer 3
└── Chief Medical Officer
    └── Medic 1
```

#### Step 2: Create Personnel Bottom-Up
1. Create lowest-level personnel first (crew members)
2. Then create their managers
3. Finally create top-level command

This ensures you have Personnel IDs available when setting up hierarchy.

#### Step 3: Link Direct Reports
For each manager:
1. Open their Data Asset
2. Scroll to **Personnel | Role**
3. In **Direct Reports** array:
   - Click '+' for each person reporting to them
   - Enter the exact Personnel ID of each subordinate
4. Save

#### Step 4: Link Managers
For each subordinate:
1. Open their Data Asset
2. Find **Personnel | Role** > **Manager ID**
3. Enter their manager's Personnel ID
4. Save

#### Step 5: Verify Hierarchy
Create a test Blueprint to verify:
1. Query a manager's Direct Reports count
2. Query a subordinate's Manager ID
3. Ensure IDs match correctly

---

## Workflow 5: Establishing Relationships

### Creating Personnel Relationships

#### Planning Relationships
Consider these relationship types:
- **Friends**: Positive working relationship
- **Rival**: Competitive or antagonistic
- **Mentor/Mentee**: Teaching relationship
- **Romantic**: Personal involvement
- **Professional**: Neutral working relationship
- **Family**: Related personnel

#### Step 1: Map Relationships
Create a relationship map:
```
Captain <-- Trusted Friend --> XO
Pilot <-- Rival --> Gunner 1
Engineer 1 <-- Mentor --> Engineer 2
Medical Officer <-- Professional --> Captain
```

#### Step 2: Add Relationship to First Person
1. Open first person's Data Asset
2. Scroll to **Personnel | Relationships**
3. Click '+' to add new relationship
4. Fill in:
   - **Target Personnel ID**: ID of related person
   - **Relationship Type**: Type of relationship
   - **Relationship Strength**: -100 to 100
     - Friends: 50-100
     - Rivals: -100 to -30
     - Professional: 0-30
   - **Notes**: Additional context

#### Step 3: Add Reciprocal Relationship
Relationships should usually be reciprocal:
1. Open second person's Data Asset
2. Add matching relationship entry
3. Strength can differ (A likes B more than B likes A)

#### Step 4: Create Relationship Networks
For team cohesion:
1. Create positive relationships between team members
2. One or two rivalries add drama
3. Mentor relationships for skill progression
4. Command relationships (captain → crew)

#### Example: Ship Bridge Team
```
Captain (ID: Capt_Smith)
├── Friend: XO_Jones (Strength: 80)
├── Professional: Pilot_Chen (Strength: 60)
└── Mentor: Navigator_Lee (Strength: 70)

XO (ID: XO_Jones)
├── Friend: Capt_Smith (Strength: 85)
└── Professional: Engineer_Kim (Strength: 55)

Pilot (ID: Pilot_Chen)
├── Professional: Capt_Smith (Strength: 65)
└── Rival: Gunner_Reed (Strength: -40)

Gunner (ID: Gunner_Reed)
└── Rival: Pilot_Chen (Strength: -35)
```

---

## Workflow 6: Managing Personnel Teams

### Organizing Personnel into Functional Teams

#### Team Types
- **Bridge Crew**: Command, pilot, navigator, comms
- **Engineering Team**: Chief engineer + engineers
- **Medical Team**: Medical officer + staff
- **Security Team**: Security chief + officers
- **Away Team**: Mixed roles for missions

#### Step 1: Define Team Composition
Decide what roles each team needs:

**Example Bridge Team:**
- Captain (leader)
- XO (deputy)
- Pilot
- Navigator
- Communications Officer

#### Step 2: Tag Team Members
1. Open each team member's Data Asset
2. Add team identifier to **Tags** array
3. Example tags:
   - "BridgeCrew"
   - "EngineeringTeam"
   - "AwayTeamAlpha"

#### Step 3: Create Team Relationships
1. Establish positive relationships between team members
2. Higher relationship strength = better team performance
3. Consider adding Team Player trait to key members

#### Step 4: Balance Team Skills
Ensure team has necessary skills:
1. List required skills for team function
2. Check each member's skills
3. Ensure coverage of all critical skills
4. Add specialists if gaps exist

#### Step 5: Blueprint Integration
In Blueprints:
1. Query all personnel with specific tag
2. Check their skills and status
3. Form team dynamically based on availability
4. Apply team bonuses based on relationships

---

## Workflow 7: Tracking Career Progression

### Managing Personnel Development Over Time

#### Phase 1: Initial Setup
When creating personnel:
1. Set realistic starting skill levels
2. Set initial experience points
3. Create first Past Assignment entry (current role)

#### Phase 2: Experience Tracking
As personnel complete tasks:
1. Award experience points
2. Update specific skill experience
3. When skill XP threshold reached:
   - Increase skill level
   - Update total experience

**Example Thresholds:**
- Level 1→2: 500 XP
- Level 2→3: 1000 XP
- Level 3→4: 1500 XP
- Level 4→5: 2000 XP
- Level 5→6: 3000 XP
- Level 6→7: 4000 XP
- Level 7→8: 5000 XP
- Level 8→9: 6000 XP
- Level 9→10: 7000 XP

#### Phase 3: Performance Updates
Regularly update performance metrics:
1. Open Data Asset
2. Update **Performance Metrics** values
3. Set **Trend** to reflect improvement/decline
4. Document in notes why performance changed

#### Phase 4: Promotions
When promoting personnel:
1. Create new Past Assignment entry:
   - **Assignment Location**: Current location
   - **Role Held**: Old role
   - **Duration**: Months in old role
   - **Performance Rating**: 1-5 stars
   - **Notes**: Reason for leaving/promotion
2. Update **Primary Role** to new role
3. Update **Salary** to reflect new position
4. Update **Department** if changed
5. Update **Direct Reports** and **Manager ID**

#### Phase 5: Trait Addition
As personnel develop:
1. Add new traits reflecting growth
2. Update existing trait modifiers
3. Remove traits if no longer applicable

**Example Progression:**
- Junior Engineer starts with "Eager Learner" trait
- After 12 months, add "Competent" trait
- After promotion, replace with "Master Technician" trait

---

## Workflow 8: Integrating with Blueprints

### Using Personnel Data in Game Logic

#### Setup: Creating Personnel Manager Blueprint

1. **Create Manager Actor**:
   - Create new Blueprint Actor: `BP_PersonnelManager`
   - Add array variable: `Personnel` (PersonnelDataAsset references)

2. **Load Personnel**:
   - In Construction Script or BeginPlay
   - Load all personnel assets from directory
   - Store in array

#### Common Blueprint Patterns

##### Pattern 1: Finding Qualified Personnel
```
Purpose: Find crew member with specific skill

Blueprint Flow:
1. Get All Personnel (from manager)
2. For Each Loop
   ├── Get Skill Level ("Piloting")
   ├── Branch (Level >= 7)
   │   └── Add to Qualified Array
3. Return Qualified Array
```

##### Pattern 2: Checking Team Readiness
```
Purpose: Verify team is fit for duty

Blueprint Flow:
1. Get Team Members (by tag or role)
2. For Each Loop
   ├── Is In Good Condition
   ├── Needs Rest
   ├── Has Low Morale
   ├── Branch (any negative)
   │   └── Add to Problem List
3. Return Ready? (Problem List empty)
```

##### Pattern 3: Relationship Bonus
```
Purpose: Apply bonus when friends work together

Blueprint Flow:
1. Get Person A Data Asset
2. Get Relationship Strength (Person B ID)
3. Branch (Strength > 50)
   ├── True: Apply 1.1x bonus
   └── False: No bonus
```

##### Pattern 4: Role Availability Check
```
Purpose: Check if ship has required crew

Blueprint Flow:
1. Get All Personnel
2. For Each Loop
   ├── Get Primary Role
   ├── Branch (Role == Captain)
   │   └── Set Has Captain = True
   ├── Branch (Role == Pilot)
   │   └── Set Has Pilot = True
   └── ... (repeat for required roles)
3. Return All Required Present?
```

##### Pattern 5: Morale Impact Event
```
Purpose: Event affects crew morale

Blueprint Flow:
1. Get All Personnel
2. For Each Loop
   ├── Get Morale
   ├── Has Trait ("Optimistic")
   │   ├── True: Adjust +5
   │   └── False: Adjust -5
   ├── Set Morale (New Value)
3. Update UI
```

#### Integration with Ship/Station Systems

**Ship Example:**
1. ASpaceShip class holds array of UPersonnelDataAsset*
2. Add function: GetAssignedPersonnel()
3. Add function: AssignPersonnel(PersonnelAsset)
4. Add function: RemovePersonnel(PersonnelID)
5. Query for captain to check leadership traits
6. Query for pilot to check flying skills

**Station Example:**
1. ASpaceStation class holds array of UPersonnelDataAsset*
2. Add function: GetStationManager()
3. Query manager traits for station bonuses
4. Count staff for efficiency calculations
5. Check security staff for threat response

#### UI Integration

**Personnel Roster UI:**
1. Create Widget Blueprint
2. Add List View of personnel
3. For each entry:
   - Display: Name, Role, Status
   - Show: Health, Morale, Fatigue bars
   - Button: View Details
4. Detail view shows full stats

**Assignment UI:**
1. Create Widget Blueprint  
2. List available personnel
3. Filter by:
   - Role
   - Skills
   - Current Assignment
   - Status
4. Drag-and-drop to assign

---

## Best Practices Summary

### DO:
✓ Use consistent naming conventions  
✓ Reference templates for typical values  
✓ Create command hierarchies bottom-up  
✓ Tag personnel for easy filtering  
✓ Balance relationships (not all friends)  
✓ Update performance metrics regularly  
✓ Document personality in biography  

### DON'T:
✗ Create overpowered personnel (too many high stats)  
✗ Forget to set reciprocal relationships  
✗ Use spaces in Personnel IDs  
✗ Leave Manager ID/Direct Reports inconsistent  
✗ Overuse high-level traits on everyone  
✗ Create crews without support roles  
✗ Neglect to update career history  

---

## Quick Reference: Common Tasks

| Task | Location | Steps |
|------|----------|-------|
| Create new personnel | Content Browser | Right-click > Data Asset > PersonnelDataAsset |
| Add new skill | Personnel\|Skills | Click + on Skills array |
| Set manager | Personnel\|Role | Enter Manager ID field |
| Add direct report | Personnel\|Role | Click + on Direct Reports array |
| Create relationship | Personnel\|Relationships | Click + on Relationships array |
| Add trait | Personnel\|Personality | Click + on Traits array |
| Update performance | Personnel\|Performance | Edit Performance Metrics array |
| Record promotion | Personnel\|Performance | Add Past Assignments entry |

---

## Troubleshooting

### Issue: Relationship not working
**Check:** Personnel IDs match exactly (case-sensitive)  
**Check:** Both parties have reciprocal entries  
**Check:** Relationship strength is set  

### Issue: Hierarchy broken
**Check:** Manager ID set on subordinates  
**Check:** Direct Reports list includes subordinates  
**Check:** IDs are correct with no typos  

### Issue: Skills not appearing
**Check:** Skill Name spelled correctly  
**Check:** Skill Level is 1-10  
**Check:** Skills array has entries  

### Issue: Can't find personnel in Blueprint
**Check:** Asset is saved  
**Check:** Loading from correct directory  
**Check:** Personnel ID is not NAME_None  

---

## Next Steps

1. **Start Small**: Create 2-3 test personnel to learn the system
2. **Build a Team**: Create a small ship crew (5-10 personnel)
3. **Add Relationships**: Connect your crew with relationships
4. **Integrate**: Use in Blueprints to verify functionality
5. **Expand**: Create larger crews and station populations
6. **Iterate**: Update and improve based on playtesting

## Additional Resources

- `PersonnelSystemGuide.md` - Complete system documentation
- `PersonnelTemplates.md` - All role descriptions and examples
- `Assets/PersonnelTemplates/` - YAML template files
- Faction system docs - Similar patterns and workflows
