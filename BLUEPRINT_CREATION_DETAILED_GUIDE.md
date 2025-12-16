# Blueprint Creation Detailed Guide - For Beginners

**Last Updated**: December 16, 2025  
**For**: Complete beginners to Unreal Engine Blueprints  
**Time**: Each blueprint tutorial takes 10-30 minutes

> **Note**: This guide assumes you've never created blueprints before. Every step is explained in detail.

---

## Table of Contents

1. [Introduction to Blueprints](#introduction-to-blueprints)
2. [Universe Map - Complete Tutorial](#universe-map---complete-tutorial)
3. [Sector Map - Complete Tutorial](#sector-map---complete-tutorial)
4. [Ship Status Screen - Basic to Polished](#ship-status-screen---basic-to-polished)
5. [Common Mistakes and How to Fix Them](#common-mistakes-and-how-to-fix-them)
6. [Next Steps](#next-steps)

---

## Introduction to Blueprints

### What Are Blueprints?

Blueprints are Unreal Engine's visual scripting system. Instead of writing C++ code, you connect nodes together to create game logic. Think of it like connecting LEGO blocks - each block (node) does something, and you connect them to build functionality.

### Why Use Blueprints?

- **No coding required** - Visual interface for game logic
- **Instant testing** - See changes immediately in-game
- **Designer-friendly** - Artists and designers can create gameplay
- **Blueprint + C++** - Adastrea uses C++ for systems, Blueprints for content

### Key Terms

- **Widget Blueprint**: A UI element (like a menu or HUD)
- **Actor Blueprint**: A game object (like a ship or station)
- **Data Asset**: Configuration data (like ship stats)
- **Parent Class**: The C++ class your blueprint is based on
- **Event Graph**: Where you add logic/behavior
- **Designer View**: Where you design UI layout

---

## Universe Map - Complete Tutorial

### What You're Building

A full-screen map showing all sectors in the universe, with:
- Grid layout of all sectors
- Icons for discovered/undiscovered sectors
- Player location indicator
- Sector selection
- Exploration progress display
- Close button

**Time**: 30 minutes for complete version

---

### Part 1: Create the Widget Blueprint (5 minutes)

#### Step 1.1: Navigate to Content Browser

1. At the bottom of the Unreal Editor, find the **Content Browser** panel
2. Look for a folder tree on the left side
3. Click on **Content** folder at the top
4. If you see an `UI` folder, double-click it. If not, we'll create it in the next step

#### Step 1.2: Create UI Folder (If Needed)

1. Right-click in an empty area of the Content Browser
2. Select **New Folder**
3. Type: `UI` (all capitals)
4. Press Enter

#### Step 1.3: Create the Widget Blueprint

1. Navigate into the `UI` folder (double-click it)
2. Right-click in an empty area
3. Hover over **User Interface** in the menu
4. Click **Widget Blueprint**
5. A window appears: "Pick Parent Class"
6. In the search box at the top, type: `UniverseMapWidget`
7. Click on **UniverseMapWidget** in the results
8. Click **Select** button at the bottom
9. Name the blueprint: `WBP_UniverseMap`
10. Press Enter to confirm

**What just happened?** You created a UI widget based on the C++ `UniverseMapWidget` class. The "WBP_" prefix stands for "Widget Blueprint".

#### Step 1.4: Open Your New Blueprint

1. Double-click `WBP_UniverseMap` in the Content Browser
2. The Blueprint editor opens with two main areas:
   - **Designer** tab (top left) - for UI layout
   - **Graph** tab (top left) - for logic/behavior
3. You should see the **Designer** tab active

---

### Part 2: Design the UI Layout (15 minutes)

#### Step 2.1: Set Up the Canvas

The Designer should show a large gray area - this is your canvas.

1. On the left, find the **Palette** panel (list of widgets you can add)
2. If you don't see it, go to **Window** → **Palette**
3. In the **Hierarchy** panel (usually top-left), you should see **Canvas Panel** as the root

**If Canvas Panel is missing:**
1. In Palette, search for "Canvas Panel"
2. Drag it onto the gray canvas area
3. It becomes the root of your hierarchy

#### Step 2.2: Add Background Image

1. In **Palette**, search for "Image"
2. Drag **Image** onto **Canvas Panel** in the Hierarchy
3. The image appears on the canvas
4. With the Image selected, look at **Details** panel (usually right side)
5. Find **Anchors** (click the flower-like icon)
6. Select the **bottom-right anchor preset** (fills entire screen)
7. Find **Color and Opacity** in Details
8. Click the color box
9. Set:
   - **R**: 0.0, **G**: 0.0, **B**: 0.05 (very dark blue)
   - **A**: 0.9 (slightly transparent)
10. In Hierarchy, rename it to "Background" (F2 key or right-click → Rename)

#### Step 2.3: Add Map Container Border

1. In Palette, search for "Border"
2. Drag **Border** onto **Canvas Panel** in Hierarchy
3. With Border selected, in Details:
   - **Position X**: 200
   - **Position Y**: 100
   - **Size X**: 1520
   - **Size Y**: 880
4. Set **Anchors** to center-center
5. Under **Appearance** → **Brush Color**:
   - **R**: 0.1, **G**: 0.1, **B**: 0.15
   - **A**: 0.8
6. Rename to "MapContainer"

#### Step 2.4: Add Sector Grid Display

1. In Palette, search for "Canvas Panel"
2. Drag **Canvas Panel** onto **MapContainer** (as a child)
3. With this new Canvas Panel selected:
   - Set **Anchors** to fill (bottom-right preset)
   - **Position**: 0, 0
   - **Size**: 0, 0 (this fills the parent)
4. Rename to "SectorGridPanel"

**What is this for?** This is where sector icons will be created dynamically via Blueprint code.

#### Step 2.5: Add Info Panel

1. In Palette, search for "Vertical Box"
2. Drag **Vertical Box** onto **Canvas Panel** (NOT inside MapContainer)
3. With Vertical Box selected:
   - **Position X**: 1580
   - **Position Y**: 100
   - **Size X**: 300
   - **Size Y**: 880
4. Set **Anchors** to center-right
5. Rename to "InfoPanel"

#### Step 2.6: Add Sector Name Display

1. In Palette, search for "Text Block"
2. Drag **Text Block** onto **InfoPanel** (as a child)
3. With Text Block selected in Details:
   - **Text**: "Sector Name"
   - **Font Size**: 24
   - **Justification**: Center
4. Check the box **Is Variable** (top of Details panel)
5. Name the variable: `Text_SectorName`

**Why "Is Variable"?** This allows us to update the text from Blueprint code later.

#### Step 2.7: Add Sector Description

1. Drag another **Text Block** onto **InfoPanel**
2. In Details:
   - **Text**: "Sector description will appear here..."
   - **Font Size**: 14
   - **Justification**: Left
   - **Auto Wrap Text**: ☑ Checked
3. Check **Is Variable**
4. Name: `Text_SectorDescription`

#### Step 2.8: Add Exploration Progress Bar

1. In Palette, search for "Progress Bar"
2. Drag **Progress Bar** onto **InfoPanel**
3. In Details:
   - **Percent**: 0.5 (50% for preview)
   - **Fill Color and Opacity**: Bright blue (R:0.2, G:0.5, B:1.0)
4. Check **Is Variable**
5. Name: `ProgressBar_Exploration`

#### Step 2.9: Add Exploration Progress Text

1. Drag another **Text Block** onto **InfoPanel**
2. In Details:
   - **Text**: "Exploration: 0%"
   - **Font Size**: 16
   - **Justification**: Center
3. Check **Is Variable**
4. Name: `Text_ExplorationPercent`

#### Step 2.10: Add Close Button

1. In Palette, search for "Button"
2. Drag **Button** onto **Canvas Panel** (top level)
3. With Button selected:
   - **Position X**: 1850
   - **Position Y**: 50
   - **Size X**: 50
   - **Size Y**: 50
4. Set **Anchors** to top-right
5. Drag a **Text Block** onto the **Button** (as a child)
6. With this Text Block selected:
   - **Text**: "X"
   - **Font Size**: 28
   - **Justification**: Center
7. Select the Button again (not the text)
8. Check **Is Variable**
9. Name: `Button_Close`

#### Step 2.11: Preview Your Layout

Click **Compile** button at the top of the Blueprint editor (looks like a checkmark). You should see your layout with all elements arranged. If anything looks wrong, adjust positions and sizes in Details panel.

---

### Part 3: Add Blueprint Logic (10 minutes)

Now we'll make it work!

#### Step 3.1: Switch to Graph View

1. Click the **Graph** tab at the top (next to Designer)
2. You'll see a grid with some existing nodes
3. This is where we add behavior

#### Step 3.2: Initialize the Map on Open

1. In the **My Blueprint** panel (left side), find **Functions** section
2. Under **Functions**, you should see **Initialize Universe Map**
3. Double-click it to open
4. You'll see a node **Initialize Universe Map (Event)**

**Add this simple implementation:**

1. Right-click in empty space
2. Search for "Print String"
3. Click **Print String**
4. Connect the white execution pin from **Initialize Universe Map** to **Print String**
5. In the Print String node, set the text to: "Universe Map Initialized"

**What this does:** For now, it just prints a message when the map opens. We'll enhance this later.

#### Step 3.3: Handle Sector Selection

1. In **My Blueprint** panel, find **On Sector Selected** under Functions
2. Double-click it
3. You'll see **On Sector Selected (Event)** node with a "Selected Sector" output

**Add this logic:**

1. Right-click in empty space
2. Search for "Is Valid"
3. Click **Is Valid**
4. Connect **Selected Sector** pin to **Is Valid** input
5. From **Is Valid** node, drag from the **Is Valid** pin (checkmark output)
6. Search for "Branch"
7. Connect **Branch** node
8. From **True** pin of Branch:
   - Right-click → **Print String**
   - In the string box, type: "Sector Selected!"
9. From **False** pin:
   - Right-click → **Print String**
   - In the string box, type: "No sector selected"

**What this does:** Checks if a valid sector was clicked, and prints a message.

#### Step 3.4: Update Sector Info Display

Let's make it update the UI when a sector is selected.

1. Still in **On Sector Selected** function
2. After the **Is Valid → Branch** node
3. From the **True** pin, right-click and search for "Get Sector Name"
4. Drag from **Selected Sector** to the **Target** input
5. Now right-click and search for "Set Text (Text Block)"
6. Drag from your **Text_SectorName** variable (left panel) to the **Target** input
7. Connect **Get Sector Name** output to **Text** input

Your graph should flow like:
```
On Sector Selected → Is Valid (Selected Sector) → Branch (True) → Get Sector Name → Set Text_SectorName
```

#### Step 3.5: Update Exploration Progress

1. In **My Blueprint** panel, find **Update Universe Grid** function
2. Double-click it
3. You'll see **Update Universe Grid (Event)** node

**Add this logic:**

1. From the event node, right-click and search for "Get Exploration Progress"
2. This node has no inputs (it's a pure function)
3. Drag from the output → Right-click → "Multiply (*)"
4. In the multiply node, set the bottom value to: `100.0`
5. Drag from multiply output → Right-click → "Format Text"
6. In Format Text, set the format to: "Exploration: {0}%"
7. Connect the multiply result to the first input
8. Drag from Format Text output → Right-click → "Set Text (Text Block)"
9. Drag **Text_ExplorationPercent** variable to Target input
10. From Update Universe Grid event, also do:
    - Right-click → "Get Exploration Progress"
    - Right-click → "Set Percent (Progress Bar)"
    - Connect **ProgressBar_Exploration** to Target
    - Connect exploration progress value to Percent input

#### Step 3.6: Add Close Button Functionality

1. In the **Designer** tab, click on **Button_Close**
2. In the **Details** panel, scroll down to **Events**
3. Find **On Clicked**
4. Click the **+** button next to it
5. This opens the **Graph** tab with a new **On Clicked (Button_Close)** event

**Add this logic:**

1. From the event, right-click and search for "Get Player Controller"
2. Drag from output → Right-click → "Toggle Universe Map Visibility"
3. In the **b Visible** input, set to: **false** (unchecked)

Your graph:
```
On Clicked (Button_Close) → Get Player Controller → Toggle Universe Map Visibility (false)
```

#### Step 3.7: Compile and Save

1. Click **Compile** button (top toolbar)
2. Look for errors in the **Compiler Results** at the bottom
3. If no errors, click **Save** button
4. Close the Blueprint editor

---

### Part 4: Connect to Player Controller (5 minutes)

#### Step 4.1: Find or Create Player Controller

1. In Content Browser, navigate to `Content/Blueprints/Controllers/`
2. Look for `BP_SpaceshipController` or `BP_AdastreaPlayerController`
3. If it doesn't exist:
   - Right-click in Controllers folder
   - **Blueprint Class**
   - Search parent class: `AdastreaPlayerController`
   - Name it: `BP_AdastreaPlayerController`
4. Double-click to open it

#### Step 4.2: Create Map Widget on Begin Play

1. Click **Open Full Blueprint Editor** if needed
2. In **Event Graph**, find or create **Event BeginPlay** node
3. From BeginPlay execution pin:
   - Right-click → "Create Widget"
   - In the **Class** dropdown, select **WBP_UniverseMap**
   - From **Return Value** → Right-click → "Promote to Variable"
   - Name it: `UniverseMapWidget`
4. From **Create Widget** return value:
   - Right-click → "Add to Viewport"

5. From **Add to Viewport**:
   - Right-click → "Toggle Universe Map Visibility"
   - Connect **UniverseMapWidget** to Target
   - Set **b Visible** to: **false**

Your graph:
```
Event BeginPlay → Create Widget (WBP_UniverseMap) → Add to Viewport → Toggle Universe Map Visibility (false)
                       ↓
                 Store in UniverseMapWidget variable
```

#### Step 4.3: Add Input Binding

**Option A: Enhanced Input (Recommended)**

1. In the Event Graph, add new nodes:
2. Right-click → "Enhanced Input Action (IA_ToggleMap)" or create a new Input Action
3. From the action event:
   - Get **UniverseMapWidget** variable
   - Right-click → "Toggle Universe Map Visibility"
   - Leave **b Visible** unconnected (will toggle)

**Option B: Keyboard Input (Quick Method)**

1. Right-click → "M" (keyboard key)
2. From **Pressed** pin:
   - Get **UniverseMapWidget** variable
   - Check if valid
   - Call **Toggle Universe Map Visibility**

Your graph:
```
Keyboard M (Pressed) → Get UniverseMapWidget → Is Valid? → Toggle Universe Map Visibility
```

#### Step 4.4: Save and Test

1. **Compile** and **Save** the Player Controller
2. In the main editor, click **Play** button
3. Press **M** key
4. Universe Map should appear!
5. Press **M** again to close
6. Click sectors to see info update

---

### Part 5: Add Sector Icons (Optional - Advanced)

This section shows how to create visual icons for each sector on the map.

#### Step 5.1: Create Sector Icon Widget

1. Create a new Widget Blueprint: `WBP_SectorIcon`
2. Parent class: `UserWidget`
3. Layout:
   - **Button** as root
   - **Image** inside button (sector icon graphic)
   - **Text Block** under image (sector name)
4. Make the Button **Is Variable**: `Button_SectorIcon`
5. Save and close

#### Step 5.2: Update Universe Map to Spawn Icons

1. Open `WBP_UniverseMap` Blueprint
2. Go to **Graph** tab
3. Find or override **Update Universe Grid** function
4. Add this logic:

```blueprint
Update Universe Grid (Event)
    ↓
Get Universe Sector Info (pure function)
    ↓
ForEach Loop (array of sector info)
    ↓
    Body:
    ├─ Create Widget (WBP_SectorIcon)
    ├─ Add to Panel (SectorGridPanel)
    ├─ Set Position
    │    ├─ Get Grid Coordinates from sector info
    │    ├─ Multiply X by 100 (spacing)
    │    ├─ Multiply Y by 100 (spacing)
    │    └─ Set Position in Parent (Canvas Panel)
    └─ Bind Button OnClicked
         └─ Call OnSectorSelected with this sector
```

**Detailed steps:**

1. **Get Universe Sector Info**:
   - Right-click → "Get Universe Sector Info"
   - This returns an array

2. **ForEach Loop**:
   - Drag from array output → "ForEach Loop"
   - This processes each sector

3. **Inside Loop Body**:
   - Create Widget (WBP_SectorIcon)
   - Get the Button from the created widget
   - Bind OnClicked event:
     - Right-click button → "Bind Event to On Clicked"
     - In the bound event, call **On Sector Selected** with the current sector

4. **Position the icon**:
   - Get Grid Coordinates from current sector info
   - Break the IntVector into X, Y, Z
   - Multiply X by 120 (icon width + spacing)
   - Multiply Y by 120 (icon height + spacing)
   - Create a Vector2D from these values
   - Call "Set Position in Viewport" on the widget

5. **Add to grid**:
   - Get **SectorGridPanel** variable
   - Call "Add Child to Canvas" with the created widget
   - Connect the position Vector2D to the Position input

This is advanced! Don't worry if it seems complex - the basic map works without icons.

---

### Troubleshooting Universe Map

**Problem**: Map doesn't appear when pressing M

**Solutions**:
- Check Player Controller is set in World Settings → Game Mode → Player Controller Class
- Verify Input binding is correct
- Check Output Log for errors
- Make sure **Add to Viewport** was called in BeginPlay

**Problem**: Sectors don't show up

**Solutions**:
- Ensure you have ASpaceSectorMap actors placed in your level
- Call **Refresh Universe Map** after creating sectors
- Check that **Initialize Universe Map** is called on first open

**Problem**: Close button doesn't work

**Solutions**:
- Verify OnClicked event is bound
- Check **Toggle Universe Map Visibility** is called with **false**
- Make sure the button isn't behind another UI element (Z-Order)

---

## Sector Map - Complete Tutorial

### What You're Building

A detailed view of a single sector showing:
- Sector name and boundaries
- List of objects in the sector
- Sector statistics
- Navigation options

**Time**: 20 minutes

---

### Part 1: Create Sector Map Widget (3 minutes)

#### Step 1.1: Create the Blueprint

1. Navigate to `Content/UI/` folder
2. Right-click → **User Interface** → **Widget Blueprint**
3. Parent class: `SectorMapWidget`
4. Name: `WBP_SectorMap`
5. Double-click to open

#### Step 1.2: Design Basic Layout

**Root Structure:**
```
Canvas Panel
├─ Image (Background) - Dark space theme
├─ Border (Info Container)
│  ├─ Vertical Box
│  │  ├─ Text Block (Sector Name) - Variable: Text_SectorName
│  │  ├─ Text Block (Description) - Variable: Text_SectorDescription
│  │  └─ Text Block (Object Count) - Variable: Text_ObjectCount
├─ Scroll Box (Object List) - Variable: ScrollBox_Objects
│  └─ (Objects will be added via Blueprint)
└─ Button (Close) - Variable: Button_Close
```

**Quick Setup:**

1. Add Image background (fill screen, dark color)
2. Add Border at left side (Position X: 50, Size X: 400, Size Y: 900)
3. Inside Border, add Vertical Box (fill)
4. Add 3 Text Blocks to Vertical Box
5. Mark each as Variable with names above
6. Add Scroll Box on right side (Position X: 500, Size X: 600)
7. Mark as Variable: `ScrollBox_Objects`
8. Add Close Button (top right)
9. Mark as Variable: `Button_Close`

---

### Part 2: Add Sector Map Logic (10 minutes)

#### Step 2.1: Initialize Sector Map

1. Switch to **Graph** tab
2. Find **Initialize Sector Map** function
3. Add:

```blueprint
Initialize Sector Map (Event)
    ↓
Print String: "Sector Map Initialized"
```

#### Step 2.2: Update Sector Info

1. Find **Update Sector Info** function
2. You'll see **Sector Info** input (a struct)

Add this logic:

```blueprint
Update Sector Info (Event)
    ↓
Break FSectorDisplayInfo (Sector Info struct)
    ↓
Set Text_SectorName → Text = Sector Name
    ↓
Set Text_SectorDescription → Text = Description
    ↓
Set Text_ObjectCount → Text = Format("Objects: {0}", Object Count)
```

**Detailed steps:**

1. Drag from **Sector Info** pin → "Break FSectorDisplayInfo"
2. This exposes all fields: Sector Name, Description, etc.
3. Drag from **Sector Name** → "Set Text (Text Block)"
4. Connect **Text_SectorName** variable to Target
5. Repeat for Description
6. For Object Count:
   - Drag from **Object Count** → "Format Text"
   - Set format: "Objects: {0}"
   - Connect to "Set Text (Text Block)" for **Text_ObjectCount**

#### Step 2.3: Update Object List

1. Find **Update Object List** function
2. You'll see **Object Names** array and **Object Count** inputs

Add this logic:

```blueprint
Update Object List (Event)
    ↓
Clear Children (ScrollBox_Objects)
    ↓
ForEach Loop (Object Names array)
    ↓
    Loop Body:
    ├─ Create Widget (Text Block widget or custom object entry widget)
    ├─ Set Text = current object name
    └─ Add Child (ScrollBox_Objects)
```

**Detailed steps:**

1. Get **ScrollBox_Objects** variable
2. Call **Clear Children** to remove old entries
3. Drag from **Object Names** → "ForEach Loop"
4. In Loop Body:
   - Right-click → "Construct Object from Class"
   - Class: **Text Block** (UTextBlock)
   - From **Return Value** → "Set Text"
   - Connect current array element to Text input
   - From the text block → "Add Child to Scroll Box"
   - Connect **ScrollBox_Objects** to Target

#### Step 2.4: Add Close Button

1. In **Designer** tab, select **Button_Close**
2. In Details → Events → **On Clicked** → Click **+**
3. In the opened graph:

```blueprint
On Clicked (Button_Close)
    ↓
Remove from Parent (self)
```

This removes the widget from screen.

#### Step 2.5: Set Target Sector

1. Find **Set Target Sector** function (it's a C++ function)
2. This is already implemented in C++ - you just need to call it!

Create a helper function to open the map:

1. **My Blueprint** panel → **Functions** → Click **+** to add new function
2. Name it: `OpenWithSector`
3. Add an input parameter:
   - Click **+** on **Inputs**
   - Type: **Space Sector Map** (Object Reference)
   - Name: `TargetSector`
4. In the function graph:

```blueprint
OpenWithSector function
    ↓
Set Target Sector (TargetSector)
    ↓
Refresh Sector Data
    ↓
Add to Viewport
```

---

### Part 3: Integrate with Universe Map (5 minutes)

Now let's make the Universe Map open the Sector Map when you click a sector.

#### Step 3.1: Update Universe Map

1. Open `WBP_UniverseMap`
2. Go to **Graph** tab
3. Find **On Sector Selected** function
4. After printing the message, add:

```blueprint
On Sector Selected (Event)
    ↓
Is Valid (Selected Sector)?
    ↓ True
    ├─ Create Widget (WBP_SectorMap)
    ├─ Call OpenWithSector (pass Selected Sector)
    └─ (Store widget in variable if you want to close it later)
```

**Detailed steps:**

1. After your existing sector selected logic
2. Right-click → "Create Widget"
3. Class: **WBP_SectorMap**
4. From Return Value → "Call Function" → Search "OpenWithSector"
5. Connect **Selected Sector** to the function input

#### Step 3.2: Test Both Maps

1. **Compile** and **Save** both widgets
2. **Play** the game
3. Press **M** to open Universe Map
4. Click a sector
5. Sector Map should appear with sector details!

---

### Part 4: Polish Sector Map (Optional - 10 minutes)

#### Add Sector Visualization

Create a visual representation of the sector:

1. In **Designer** tab, add a **Border** widget
2. Size: 400x400
3. Position it between Info Panel and Object List
4. Inside the Border, add a **Canvas Panel**
5. Name it: `Canvas_SectorView`

In **Graph** tab, **Update Sector Info** function, add:

```blueprint
After setting text fields
    ↓
Spawn simple visual elements in Canvas_SectorView
(This could be stars, planets, stations as images)
```

#### Add Object Icons

Instead of plain text for objects, create an icon + text widget:

1. Create new widget: `WBP_ObjectListItem`
2. Layout: Horizontal Box with Image + Text
3. In Sector Map's **Update Object List**, create this widget instead of TextBlock

#### Add Navigation Button

Add a button to navigate player to this sector:

1. Add Button to layout: "Navigate Here"
2. On Click:
   - Get player ship
   - Set destination to sector center
   - Enable autopilot

---

### Troubleshooting Sector Map

**Problem**: Sector Map doesn't open

**Solutions**:
- Check **Create Widget** is called with correct class
- Verify **Add to Viewport** is called
- Check Output Log for errors

**Problem**: No objects in list

**Solutions**:
- Ensure sector has objects placed in it
- Check **Update Object List** is called
- Verify objects have names set

**Problem**: Sector info doesn't update

**Solutions**:
- Check **Update Sector Info** is properly implemented
- Verify **Refresh Sector Data** is called
- Ensure sector reference is valid

---

## Ship Status Screen - Basic to Polished

### Overview

We'll create the ship status screen in stages:
1. **Basic** (10 min) - Simple stats display
2. **Intermediate** (15 min) - All stat categories
3. **Polished** (20 min) - Visual polish with bars, icons, animations

---

### Stage 1: Basic Ship Status (10 minutes)

#### Step 1.1: Create the Widget

1. Navigate to `Content/UI/HUD/` folder
2. Create new Widget Blueprint
3. Parent class: `ShipStatusWidget`
4. Name: `WBP_ShipStatus_Basic`

#### Step 1.2: Basic Layout

**Structure:**
```
Canvas Panel
├─ Image (Background - semi-transparent overlay)
├─ Border (Content Container - centered)
│  └─ Vertical Box
│     ├─ Text Block (Ship Name) - Variable: Text_ShipName
│     ├─ Text Block (Ship Class) - Variable: Text_ShipClass
│     ├─ Text Block (Description) - Variable: Text_Description
│     ├─ Progress Bar (Hull) - Variable: ProgressBar_Hull
│     └─ Text Block (Hull Text) - Variable: Text_Hull
└─ Button (Close) - Variable: Button_Close
```

**Quick Setup:**

1. Add Image as background (fill screen, Color: Black, Alpha: 0.7)
2. Add Border centered (Size: 600x400)
3. Inside Border, add Vertical Box
4. Add Text Blocks for Ship Name, Class, Description
5. Set fonts: Name=32, Class=24, Description=16
6. Add Progress Bar (height=30)
7. Add Text Block under progress bar for numbers
8. Add Close Button in top-right corner

**Make all widgets Variables** using the names above.

#### Step 1.3: Implement Update Functions

Switch to **Graph** tab.

**Update Basic Info:**

1. Find **Update Basic Info** function
2. You'll see inputs: Ship Name, Ship Class, Description

Add:
```blueprint
Update Basic Info (Event)
    ↓
Set Text_ShipName → Text = ShipName
    ↓
Set Text_ShipClass → Text = ShipClass
    ↓
Set Text_Description → Text = Description
```

**Update Core Stats:**

1. Find **Update Core Stats** function
2. Inputs: Current Hull, Max Hull, Cargo Capacity, etc.

Add:
```blueprint
Update Core Stats (Event)
    ↓
Divide: CurrentHull / MaxHull
    ↓
Set ProgressBar_Hull → Percent = result
    ↓
Format Text: "{0} / {1}" (CurrentHull, MaxHull)
    ↓
Set Text_Hull → Text = formatted text
```

**Detailed steps:**

1. Drag from **Current Hull** → "Float / Float"
2. Connect **Max Hull** to bottom input
3. Drag from result → "Set Percent (Progress Bar)"
4. Connect **ProgressBar_Hull** to Target
5. For the text:
   - Right-click → "Format Text"
   - Format: "{0} / {1}"
   - Connect **Current Hull** to first input
   - Connect **Max Hull** to second input
   - Drag from output → "Set Text (Text Block)"
   - Connect **Text_Hull** to Target

**Close Button:**

1. In Designer, select Button_Close
2. Details → Events → On Clicked → **+**
3. In graph:

```blueprint
On Clicked (Button_Close)
    ↓
Get Player Controller → Cast to AdastreaPlayerController
    ↓
Toggle Ship Status
```

#### Step 1.4: Connect to Player Controller

1. Open your Player Controller Blueprint
2. In **Class Defaults** panel:
   - Find **Ship Status** category
   - Set **Ship Status Widget Class** = WBP_ShipStatus_Basic
3. In **Event Graph**, add input binding:

```blueprint
Keyboard I (Pressed)
    ↓
Get Owning Player → Cast to AdastreaPlayerController
    ↓
Toggle Ship Status
```

**Or with Enhanced Input:**

```blueprint
Enhanced Input Action IA_ToggleShipStatus (Started)
    ↓
Get Owning Player → Cast to AdastreaPlayerController
    ↓
Toggle Ship Status
```

#### Step 1.5: Test Basic Version

1. **Compile** and **Save** everything
2. **Play** the game
3. Press **I** key (or your bound key)
4. You should see ship name, class, description, and hull integrity!
5. Press **I** again or click Close to hide

---

### Stage 2: Intermediate Ship Status (15 minutes)

Now let's add all stat categories.

#### Step 2.1: Expand Layout

Open `WBP_ShipStatus_Basic` (or create new: `WBP_ShipStatus_Intermediate`).

**New Structure:**
```
Canvas Panel
├─ Image (Background)
├─ Border (Main Container - larger: 1200x800)
│  └─ Vertical Box
│     ├─ Horizontal Box (Header)
│     │  ├─ Text Block (Ship Name)
│     │  └─ Button (Close)
│     ├─ Text Block (Ship Class)
│     ├─ Horizontal Box (Stat Panels)
│     │  ├─ Vertical Box (Core Stats Panel)
│     │  │  ├─ Text Block "Core Stats"
│     │  │  ├─ Progress Bar (Hull)
│     │  │  ├─ Text Block (Cargo)
│     │  │  └─ Text Block (Crew)
│     │  ├─ Vertical Box (Combat Stats Panel)
│     │  │  ├─ Text Block "Combat Stats"
│     │  │  ├─ Progress Bar (Shields)
│     │  │  ├─ Text Block (Armor)
│     │  │  └─ Text Block (Weapons)
│     │  └─ Vertical Box (Mobility Stats Panel)
│     │     ├─ Text Block "Mobility Stats"
│     │     ├─ Text Block (Speed)
│     │     ├─ Text Block (Acceleration)
│     │     └─ Text Block (Maneuverability)
│     └─ Horizontal Box (Ratings)
│        ├─ Progress Bar (Combat Rating)
│        ├─ Progress Bar (Mobility Rating)
│        └─ Progress Bar (Utility Rating)
```

**Create this layout:**

1. Resize Border to 1200x800
2. Create Horizontal Box for header (Ship Name + Close Button)
3. Create Horizontal Box for three stat panels
4. Inside each panel, add:
   - Title text (bold, larger font)
   - Text blocks for each stat
   - Progress bars where appropriate

**Important:** Make ALL stat text blocks and progress bars **Variables**:
- `Text_Cargo`, `Text_Crew`, `Text_Armor`, `Text_Shields`, etc.
- `ProgressBar_Shields`, `ProgressBar_CombatRating`, etc.

#### Step 2.2: Implement All Update Functions

**Update Combat Stats:**

```blueprint
Update Combat Stats (Event with 6 inputs)
    ↓
Set ProgressBar_Shields → Percent = (ShieldStrength / MaxShieldStrength)
    ↓
Set Text_Armor → Text = Format("Armor: {0}", ArmorRating)
    ↓
Set Text_Weapons → Text = Format("Weapons: {0} slots", WeaponSlots)
```

**Update Mobility Stats:**

```blueprint
Update Mobility Stats (Event with 6 inputs)
    ↓
Set Text_Speed → Text = Format("Speed: {0}", MaxSpeed)
    ↓
Set Text_Acceleration → Text = Format("Accel: {0}", Acceleration)
    ↓
Set Text_Maneuverability → Text = Format("Maneuv: {0}", Maneuverability)
```

**Update Ratings:**

```blueprint
Update Ratings (Event with 3 inputs: CombatRating, MobilityRating, UtilityRating)
    ↓
Set ProgressBar_CombatRating → Percent = (CombatRating / 100.0)
    ↓
Set Text_CombatRating → Text = Format("Combat: {0}%", CombatRating)
    ↓
(Repeat for Mobility and Utility ratings)
```

**For each stat category** (Utility, Operational, Advanced, Lore), create similar text updates.

---

### Stage 3: Polished Ship Status (20 minutes)

Let's make it look professional!

#### Step 3.1: Visual Improvements

**Color Coding:**

1. Hull bar: Green when > 75%, Yellow 50-75%, Red < 50%
2. Shield bar: Blue
3. Rating bars: Combat=Red, Mobility=Yellow, Utility=Green

**In Graph tab, Update Core Stats:**

```blueprint
After setting ProgressBar_Hull Percent
    ↓
Branch: CurrentHull / MaxHull > 0.75?
    True → Set Fill Color = Green (0, 1, 0)
    False → Branch: > 0.5?
        True → Set Fill Color = Yellow (1, 1, 0)
        False → Set Fill Color = Red (1, 0, 0)
```

**Step-by-step:**

1. After setting Percent on progress bar
2. Get the percent value again (CurrentHull / MaxHull)
3. Compare with 0.75 (Greater than?)
4. Add Branch node
5. If True: Get ProgressBar_Hull → Set Fill Color and Opacity
   - Create LinearColor with R=0, G=1, B=0 (green)
6. If False: Add another comparison (> 0.5?)
   - True path: Yellow color
   - False path: Red color

#### Step 3.2: Add Icons

**Download or create icon images** for:
- Hull icon
- Shield icon
- Weapon icon
- Speed icon
- etc.

**Add to layout:**

1. In each stat section, add **Image** widget before the text
2. Size: 32x32
3. Import your icon images to Content Browser
4. In Image widget Details → **Brush** → **Image**, select your icon

#### Step 3.3: Add Tabs

For a polished look, create tabs to switch between stat categories.

**Create Tab System:**

1. Add Horizontal Box for tab buttons
2. Add 4 Buttons: "Core", "Combat", "Systems", "Lore"
3. Make each button **IsVariable**
4. Add corresponding Vertical Boxes for each tab's content
5. Initially, hide all except the first tab

**Tab Switching Logic:**

```blueprint
Button_CoreTab → On Clicked
    ↓
Set Visibility (CoreStatsPanel) → Visible
    ↓
Set Visibility (CombatStatsPanel) → Collapsed
    ↓
Set Visibility (SystemsStatsPanel) → Collapsed
    ↓
Set Visibility (LorePanel) → Collapsed
```

Repeat for each tab button.

#### Step 3.4: Add Animations

**Create fade-in animation:**

1. Click **+ Animation** button at bottom
2. Name it: `FadeIn`
3. In Animation timeline:
   - Select root widget (Canvas Panel)
   - Click **+ Track** → Render Opacity
   - Set keyframe at 0.0 seconds: Opacity = 0.0
   - Set keyframe at 0.3 seconds: Opacity = 1.0
4. In **Event Graph**, in **Event Construct**:

```blueprint
Event Construct
    ↓
Play Animation (FadeIn)
```

**Create pulse animation for low hull:**

1. Create animation: `PulseRed`
2. Animate Hull progress bar color pulsing red
3. In Update Core Stats, check if hull < 25%:
   - If true, Play Animation (PulseRed, looping)
   - If false, Stop Animation (PulseRed)

#### Step 3.5: Add Ship Image/3D Preview

**Option A: 2D Ship Image**

1. Add large **Image** widget in center or top
2. Size: 400x400
3. Import ship PNG/texture
4. Set image to your ship texture

**Option B: 3D Ship Preview (Advanced)**

1. Add **Scene Capture Component 2D** to your ship
2. Create Render Target texture
3. In Ship Status Widget, add Image
4. Set Image source to the Render Target
5. Now you have a live 3D preview!

**Simplified approach:**

Just add a placeholder image for now. You can enhance this later.

#### Step 3.6: Add Sound Effects

**Add button click sounds:**

1. Import sound file (WAV or MP3) to Content Browser
2. In each button's On Clicked:
   - Before other logic, add "Play Sound 2D"
   - Select your click sound

**Add open/close sounds:**

1. In Event Construct (when opening):
   - Add "Play Sound 2D" with opening whoosh sound
2. In Close Button (when closing):
   - Add "Play Sound 2D" with closing whoosh sound

---

### Final Ship Status Features

#### Real-Time Updates

To update stats while the screen is open:

1. Override **Tick** function
2. Add a timer (e.g., update every 0.5 seconds)
3. Call **Refresh Ship Status**

```blueprint
Event Tick
    ↓
Get Time Seconds
    ↓
Modulo (%) 0.5 (updates every 0.5 sec)
    ↓
Branch: result < Delta Seconds?
        True → Refresh Ship Status
```

#### Tooltips

Add tooltips to explain stats:

1. Select any widget (e.g., Text_Armor)
2. In Details → **Tooltip** → **Tooltip Widget**
3. Create simple Text Block widget with explanation
4. Hovering over the stat now shows the tooltip!

#### Keyboard Navigation

Make it keyboard-friendly:

1. Enable **Is Focusable** on Close Button
2. Bind **Escape** key to close:

```blueprint
Keyboard Escape (Pressed)
    ↓
Close the widget
```

---

### Complete Ship Status Checklist

✅ **Basic (Stage 1):**
- [ ] Widget created based on ShipStatusWidget
- [ ] Ship name, class, description displayed
- [ ] Hull integrity bar working
- [ ] Close button functional
- [ ] Bound to player controller
- [ ] Input key working

✅ **Intermediate (Stage 2):**
- [ ] All stat categories added (Core, Combat, Mobility)
- [ ] All Update functions implemented
- [ ] Combat, Mobility, Utility ratings displayed
- [ ] Organized into panels
- [ ] All values update correctly

✅ **Polished (Stage 3):**
- [ ] Color-coded bars (hull, shields, ratings)
- [ ] Icons added for each stat
- [ ] Tab system for organization
- [ ] Fade-in animation
- [ ] Pulse animation for critical states
- [ ] Ship image/preview
- [ ] Sound effects
- [ ] Real-time updates
- [ ] Tooltips
- [ ] Keyboard navigation

---

### Troubleshooting Ship Status

**Problem**: Widget doesn't open

**Solution:**
- Check **Ship Status Widget Class** is set in Player Controller
- Verify you're controlling a spaceship with ShipDataAsset
- Check Output Log: `LogAdastrea: ShipStatusWidget`

**Problem**: Stats show as 0 or empty

**Solution:**
- Ensure your spaceship has **ShipDataAsset** assigned
- Check that the Data Asset has values filled in
- Verify **Refresh Ship Status** is being called

**Problem**: Progress bars don't fill

**Solution:**
- Check you're dividing by max value: `CurrentValue / MaxValue`
- Ensure max value is not zero
- Verify you're setting **Percent** property (0.0 to 1.0 range)

**Problem**: Colors don't update

**Solution:**
- Check **Set Fill Color and Opacity** is called on Progress Bar
- Verify color values are in 0.0-1.0 range
- Ensure the branch conditions are correct (> 0.75, > 0.5, etc.)

---

## Common Mistakes and How to Fix Them

### Mistake 1: Forgetting to Make Widgets Variables

**Problem:** You can't access UI elements in Blueprint code.

**Solution:**
1. In Designer, select the widget
2. In Details panel (top), check ☑ **Is Variable**
3. Name it descriptively

### Mistake 2: Wrong Anchor Settings

**Problem:** UI elements don't scale with screen resolution.

**Solution:**
1. Select widget in Designer
2. Click **Anchors** button (flower icon)
3. Choose appropriate preset:
   - Full screen: bottom-right
   - Centered: center-center
   - Top-left corner: top-left
   - etc.

### Mistake 3: Execution Pins Not Connected

**Problem:** Blueprint code doesn't run.

**Solution:**
- White execution pins (arrows) MUST be connected
- Check the flow from start to end
- Missing connections = code doesn't execute

### Mistake 4: Wrong Data Type Connections

**Problem:** Can't connect pins, or compiler error.

**Solution:**
- Pins have types (Int, Float, String, etc.)
- Only compatible types can connect
- Use conversion nodes if needed:
  - "To Text" for converting values to text
  - "To String" for converting to string
  - etc.

### Mistake 5: Forgetting to Compile

**Problem:** Changes don't show in game.

**Solution:**
- Click **Compile** button after every change
- Check for errors in Compiler Results
- Fix errors before testing

### Mistake 6: Not Calling Parent Implementation

**Problem:** C++ logic doesn't run.

**Solution:**
- When overriding BlueprintNativeEvents
- Right-click → Add call to parent function
- This runs the C++ version first

### Mistake 7: Null Reference Errors

**Problem:** Game crashes or widget doesn't work.

**Solution:**
- Always check if references are valid
- Use **Is Valid?** node before accessing
- Initialize variables in BeginPlay/Construct

### Mistake 8: Wrong Widget Parent Class

**Problem:** Functions and variables are missing.

**Solution:**
- When creating Widget Blueprint, select correct parent:
  - **UniverseMapWidget** for universe map
  - **SectorMapWidget** for sector map
  - **ShipStatusWidget** for ship status
- Can't change after creation - must recreate

### Mistake 9: Z-Order Issues

**Problem:** Buttons don't click, widgets behind others.

**Solution:**
- In Hierarchy, drag widgets to reorder
- Lower in list = on top
- Or set **Z-Order** in Details panel

### Mistake 10: Forgetting Add to Viewport

**Problem:** Widget created but not visible.

**Solution:**
- After **Create Widget**, call **Add to Viewport**
- Or add to specific parent with **Add Child**

---

## Next Steps

### After Creating These Blueprints

1. **Test thoroughly** - Play the game, test all buttons and features
2. **Add more content** - Create more ships, factions, sectors
3. **Enhance UI** - Add icons, animations, sound effects
4. **Create more widgets** - Inventory, Trading, Station Management
5. **Learn Blueprint debugging** - Use breakpoints and Print String

### Learning Resources

**Unreal Engine Official:**
- Unreal Engine 5 Documentation
- UMG UI Designer Quick Start
- Blueprint Visual Scripting

**Adastrea Project:**
- `BLUEPRINT_GUIDE_FOR_BEGINNERS.md` - More Blueprint basics
- `Assets/MapSystemGuide.md` - Complete map system documentation
- `Source/Adastrea/UI/ShipStatusWidget_Usage.md` - Ship status C++ reference

### Getting Help

**Debugging:**
1. Use **Print String** nodes to see values
2. Check **Output Log** at bottom of editor
3. Use **Breakpoints** in Blueprint (F9 key)
4. Check **Compiler Results** for errors

**Common Error Messages:**
- "Accessed None" = Null reference, check Is Valid
- "Index out of bounds" = Array access error, check array size
- "Widget not in viewport" = Forgot Add to Viewport
- "Pure node error" = Wrong connection type, check pins

---

## Icon and Asset Requirements

### Universe Map Icons

**Required Icons:**
- Sector icon (discovered) - 64x64px, PNG with transparency
- Sector icon (undiscovered) - 64x64px, PNG, grayed out
- Player location marker - 32x32px, PNG, bright color
- Selected sector highlight - 64x64px, PNG, glow effect

**Where to get:**
- Create in Photoshop/GIMP
- Use free icon sites (Game-icons.net, Flaticon)
- Generate in AI tool (Midjourney, DALL-E)
- Commission an artist

**How to import:**
1. In Content Browser, right-click → **Import**
2. Select your PNG files
3. In Texture Settings:
   - Compression: UserInterface2D
   - sRGB: ☑ Checked
4. Use in Image widgets

### Sector Map Assets

**Required:**
- Background star field - Tileable, 1024x1024
- Object icons - Ship, station, asteroid, planet (32x32px each)
- Border decoration - For panels

### Ship Status Assets

**Required:**
- Hull icon - 32x32px
- Shield icon - 32x32px
- Weapon icon - 32x32px
- Speed icon - 32x32px
- Cargo icon - 32x32px
- Crew icon - 32x32px
- Ship preview image - 512x512px (optional)

**Quick Placeholder Assets:**

You can use simple colored squares as placeholders:

1. Create 32x32px PNG files in your image editor
2. Fill with solid colors:
   - Hull = Green
   - Shield = Blue
   - Weapon = Red
   - Speed = Yellow
   - Cargo = Brown
   - Crew = White
3. Import to Unreal
4. Use in Image widgets

Replace with proper icons later!

---

## Summary

You've learned how to create three major UI widgets:

1. **Universe Map** (30 min)
   - Shows all sectors in grid
   - Player location tracking
   - Sector selection
   - Exploration progress

2. **Sector Map** (20 min)
   - Detailed sector view
   - Object listing
   - Sector information
   - Navigation integration

3. **Ship Status** (10-45 min depending on polish level)
   - Basic: Name, class, description, hull (10 min)
   - Intermediate: All stat categories (25 min)
   - Polished: Colors, icons, animations, tabs (45 min)

**Total time**: 60-95 minutes for all three widgets from scratch!

### Key Takeaways

✅ Blueprints are visual - drag and connect nodes  
✅ Always make UI elements **Variables** to access them  
✅ Use correct **Parent Class** when creating  
✅ **Compile** after every change  
✅ Test in **PIE** (Play In Editor) frequently  
✅ Check **Output Log** for errors  
✅ Use **Print String** for debugging  

### What's Next?

With these foundational UI widgets, you can now:
- Navigate the universe
- View sector details
- Check ship status
- Create similar widgets for other systems

Continue with:
- Inventory UI (similar to Ship Status)
- Trading UI (similar to Sector Map)
- Station Management UI
- Combat HUD elements

You're now ready to create any UI in Adastrea! 🚀

---

**Questions?** Check the existing guides:
- `BLUEPRINT_GUIDE_FOR_BEGINNERS.md` - More Blueprint basics
- `Assets/MapSystemGuide.md` - Advanced map features
- `Assets/ShipStatusScreenSetup.md` - Additional ship status info
- `TROUBLESHOOTING.md` - Common problems and solutions

**Good luck creating awesome UIs!** 🎨
