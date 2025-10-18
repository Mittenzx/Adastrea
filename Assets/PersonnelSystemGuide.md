# Personnel Management System Guide

## Overview

The Personnel Management System allows you to create, manage, and track crew members and staff throughout your game. Personnel can be assigned to ships, stations, and various roles, with detailed attributes tracking their skills, relationships, performance, and status.

This system is built using **Blueprint Data Assets** similar to the Faction system, making it easy for designers to create and modify personnel without writing code.

## Key Features

### Comprehensive Personnel Attributes
- **Identity**: Name, age, gender, species, nationality, biography
- **Role & Assignment**: 31 different role types from Captain to Janitor
- **Skills**: Multiple skill categories with levels and experience tracking
- **Status**: Health, morale, fatigue, loyalty, reputation
- **Relationships**: Track friendships, rivalries, mentorships between personnel
- **Performance**: Multiple metrics tracking effectiveness and trends
- **Career History**: Past assignments with ratings and notes
- **Traits**: Unique characteristics that provide gameplay modifiers
- **Employment**: Salary, contract duration, and terms

### Available Roles

The system supports 31 different personnel roles:

**Command Roles:**
- Captain
- Executive Officer (XO)
- Station Manager

**Flight Operations:**
- Pilot
- Navigator

**Technical:**
- Engineer
- AI Operator
- Researcher

**Combat:**
- Gunnery Chief
- Gunner
- Security Officer

**Support Services:**
- Medical Officer
- Quartermaster
- Chef
- Bartender
- Janitor

**Specialized:**
- Communications Officer
- Science Officer
- Diplomat
- Ambassador
- Salvage Specialist
- Smuggler
- Mining Specialist
- Trader

**General:**
- Entertainment
- Civilian
- Crew Member
- Station Staff
- Module Staff
- Other

## Quick Start Guide

### Creating Your First Personnel Asset

1. **Open the Unreal Editor**
2. **Navigate to Content Browser**
3. **Right-click** in the folder where you want to create the personnel
4. **Select** Miscellaneous > Data Asset
5. **Choose** PersonnelDataAsset as the class
6. **Name** the asset (e.g., `DA_Personnel_CaptainSmith`)
7. **Double-click** to open and configure

### Configuring Personnel Properties

The Personnel Data Asset is organized into logical categories:

#### Personnel | Identity
Basic information about the person:
- **Personnel Name**: Display name shown in UI
- **Personnel ID**: Unique identifier for code references
- **Biography**: Background story (can be multiple lines)
- **Age**: Numeric age (typically 18-200)
- **Gender**: Text field (Male, Female, Non-Binary, etc.)
- **Species**: Race or species (Human, Alien, etc.)
- **Nationality**: Origin faction or planet
- **Tags**: Array of strings for filtering/searching

#### Personnel | Role
Assignment and hierarchy:
- **Primary Role**: Selected from enum (Captain, Pilot, etc.)
- **Current Assignment**: Ship/station name
- **Department**: Which department they belong to
- **Manager ID**: Their supervisor's Personnel ID
- **Direct Reports**: Array of Personnel IDs reporting to them

#### Personnel | Skills
Abilities and experience:
- **Overall Skill Level**: 1-10 rating
- **Total Experience**: Cumulative experience points
- **Skills**: Array of individual skills with levels and XP
- **Specialties**: Array of specialty areas

#### Personnel | Status
Current condition:
- **Morale**: 0-100 (happiness and satisfaction)
- **Health**: 0-100 (physical condition)
- **Fatigue**: 0-100 (tiredness, 0 = well-rested)
- **Loyalty**: 0-100 (commitment to organization)
- **Reputation**: -100 to 100 (standing in galaxy)

#### Personnel | Employment
Contract details:
- **Salary**: Monthly pay in credits
- **Contract Duration**: Total months (0 = indefinite)
- **Contract Months Remaining**: Time left on contract

#### Personnel | Personality
Character traits:
- **Personality Type**: Quick descriptor
- **Personality Description**: Detailed character description
- **Traits**: Array of trait structures with modifiers

#### Personnel | Relationships
Interpersonal connections:
- Array of relationship structures including:
  - Target Personnel ID
  - Relationship Type (Friend, Rival, Mentor, etc.)
  - Strength (-100 to 100)
  - Notes

#### Personnel | Performance
Effectiveness tracking:
- Array of performance metrics with:
  - Metric Name
  - Current Value (0-100)
  - Trend (positive = improving)
- Array of past assignments with:
  - Location
  - Role Held
  - Duration
  - Performance Rating (1-5 stars)
  - Notes

## Using YAML Templates

We provide YAML templates for common roles to make personnel creation faster and more consistent. These templates are located in `Assets/PersonnelTemplates/`.

### Available Templates

- **Captain.yaml** - Ship captain template
- **Pilot.yaml** - Pilot/helmsman template
- **Gunner.yaml** - Weapons operator template
- **Engineer.yaml** - Chief engineer template
- **MedicalOfficer.yaml** - Doctor/medic template
- **StationManager.yaml** - Space station manager template
- **CrewMember_Generic.yaml** - General crew member template

### Using a Template

1. Open the appropriate YAML file from `Assets/PersonnelTemplates/`
2. Copy the content
3. Fill in the bracketed placeholders with your custom values
4. Use the filled template as a reference when creating your Data Asset in Unreal
5. The YAML structure maps directly to the Data Asset properties

**Note**: YAML templates are reference documents for designers. Unreal Engine uses Blueprint Data Assets, not YAML files directly. Copy the values from the template into your Data Asset in the editor.

## Blueprint Integration

### Accessing Personnel in Blueprints

All personnel functions are Blueprint-callable and can be used in your game logic:

#### Trait Queries
```
GetTraits() -> Array of Traits
HasTrait(TraitID) -> Boolean
GetTraitByID(TraitID) -> Trait struct (if found)
GetTraitModifier(TraitID) -> Float modifier value
```

#### Skill Queries
```
GetSkillByName(SkillName) -> Skill struct (if found)
GetSkillLevel(SkillName) -> Integer (1-10)
HasSpecialty(Specialty) -> Boolean
```

#### Relationship Queries
```
GetRelationship(OtherPersonnelID) -> Relationship struct
GetRelationshipStrength(OtherPersonnelID) -> Integer (-100 to 100)
IsFriendsWith(OtherPersonnelID) -> Boolean
IsRivalWith(OtherPersonnelID) -> Boolean
```

#### Performance Queries
```
GetPerformanceMetric(MetricName) -> Metric struct
GetAveragePerformance() -> Float (0-100)
IsPerformingWell() -> Boolean (average > 70)
```

#### Status Queries
```
IsInGoodCondition() -> Boolean (health > 70, fatigue < 30)
NeedsRest() -> Boolean (fatigue > 70)
IsLoyal() -> Boolean (loyalty > 70)
HasLowMorale() -> Boolean (morale < 30)
```

#### Role Queries
```
IsInLeadershipRole() -> Boolean
IsInCombatRole() -> Boolean
GetDirectReportCount() -> Integer
```

### Example Blueprint Usage

**Checking if a personnel needs medical attention:**
```
Get Personnel Data Asset
-> IsInGoodCondition()
-> Branch (False = send to medical bay)
```

**Checking if someone can lead a mission:**
```
Get Personnel Data Asset
-> IsInLeadershipRole()
-> AND -> GetSkillLevel("Tactical") > 7
-> Branch (True = assign as mission leader)
```

**Evaluating crew relationships:**
```
Get Personnel Data Asset (Pilot)
-> IsFriendsWith(Gunner Personnel ID)
-> Branch (True = apply teamwork bonus)
```

## Best Practices

### Personnel Design

1. **Start with a Template**: Use the provided YAML templates as starting points
2. **Balanced Stats**: Most crew should have 4-6 overall skill level; specialists 7-8; elite 9-10
3. **Meaningful Traits**: Limit to 2-4 traits per person; make them impactful
4. **Realistic Relationships**: Not everyone needs to have relationships with everyone
5. **Performance Trends**: Use trend values to show if someone is improving or declining
6. **Contract Variety**: Mix permanent crew (0 duration) with contract workers

### Skill Levels Guide

- **1-3**: Novice (trainee, inexperienced)
- **4-6**: Competent (average crew member)
- **7-8**: Expert (specialist in their field)
- **9-10**: Master (best in their field, rare)

### Morale Management

Consider morale in your game design:
- **80-100**: Excellent (bonus performance)
- **60-79**: Good (normal performance)
- **40-59**: Fair (slight performance penalty)
- **20-39**: Poor (significant penalties)
- **0-19**: Critical (may quit or sabotage)

### Relationship Strength Guide

- **70-100**: Close friends/strong allies
- **30-69**: Friendly/positive
- **-29 to 29**: Neutral
- **-30 to -69**: Dislike/rivalry
- **-70 to -100**: Enemies/bitter rivals

## Integration with Game Systems

### Ships and Stations

Personnel can be assigned to:
- Ships (via CurrentAssignment property)
- Space Stations (via CurrentAssignment property)
- Specific modules (ModuleStaff role)

### Command Hierarchy

Build management trees using:
- **ManagerID**: Link to supervisor
- **DirectReports**: Array of subordinates

This allows you to query organizational structure and implement:
- Chain of command mechanics
- Promotion systems
- Management effectiveness calculations

### Dynamic Systems

Personnel attributes can drive gameplay:
- **Skill levels** affect success rates
- **Fatigue** impacts performance
- **Morale** influences loyalty and efficiency
- **Relationships** create bonuses/penalties for team tasks
- **Traits** provide unique gameplay modifiers

## Playtesting Tips

1. **Create a Test Crew**: Make 5-10 test personnel covering different roles
2. **Verify Relationships**: Ensure relationship links use correct Personnel IDs
3. **Test Functions**: Use Blueprint debugging to verify query functions work
4. **Balance Check**: Ensure no one is too overpowered with stacked traits
5. **Performance Tracking**: Monitor how performance metrics affect gameplay

## Troubleshooting

### Common Issues

**Problem**: Relationship not working
- **Solution**: Verify Target Personnel ID exactly matches the other person's Personnel ID (case-sensitive)

**Problem**: Trait modifier not applying
- **Solution**: Check Trait ID matches exactly in both the trait definition and query function

**Problem**: Skill not found
- **Solution**: Skill name comparison is case-insensitive, but check for typos

**Problem**: Personnel not showing in lists
- **Solution**: Ensure Personnel ID is unique and not NAME_None

## Advanced Topics

### Creating Custom Traits

When creating new traits:
1. Give each trait a unique Trait ID (e.g., "CombatVeteran")
2. Use consistent naming conventions
3. Document what the modifier value represents
4. Test modifier stacking if multiple traits have the same ID

### Performance Metrics

Standard metrics to consider:
- Task completion rate
- Error rate
- Efficiency
- Quality
- Speed
- Safety compliance
- Team cooperation

### Career Progression

Track personnel growth:
1. Award experience points for completed tasks
2. Update skill levels as XP thresholds are met
3. Add new past assignments when roles change
4. Update traits as personnel develop
5. Adjust salary based on skill level

### Integration Examples

**Mission Assignment System:**
- Query personnel skills to find qualified candidates
- Check fatigue and health before assignment
- Consider relationships when forming teams
- Apply trait modifiers to mission success chances

**Crew Morale System:**
- Track morale changes based on events
- Use trait modifiers to affect morale gain/loss
- Check for low morale and trigger events
- Implement morale improvement activities

**Promotion System:**
- Check performance metrics for eligibility
- Verify minimum skill levels for role
- Update role, salary, and direct reports
- Add past assignment entry for promotion

## Next Steps

1. **Read PersonnelTemplates.md** for detailed role descriptions and example configurations
2. **Read PersonnelDesignerWorkflow.md** for step-by-step creation instructions
3. **Start with generic crew** and work up to specialized roles
4. **Create relationships** between your personnel to add depth
5. **Integrate with your game systems** using the Blueprint functions

## Support

For more information:
- See `PersonnelTemplates.md` for all role templates
- See `PersonnelDesignerWorkflow.md` for detailed workflows
- See faction system docs for similar patterns
- Check Unreal Engine documentation for Data Assets
