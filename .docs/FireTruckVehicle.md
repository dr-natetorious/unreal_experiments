# Fire Truck Vehicle — Aerial Ladder Truck

## **Key clarificataion from your questions**

These instructions are gold standard and override any confusion from below:

 I've noticed you do best by     
  assemblying in peices and then stitching them together. To your questions: 1/ 
  The Ambulance and Police car already exist and need to hold our Duder and     
  Dudette (see /apps/git/unreal_experiments/.docs/BlenderToUEWorkflow.md;       
  people are always same model/size). 2/ Previous sessions kept defaulting to   
  British (e.g., spelling and putting steering wheel on wrong side). This is    
  for an American kid. 3/ Good callout on the ladder. The goal is that it'll be 
  a game mechanic for getting over obstacles and would need to extend and       
  swivle. 4/ The cardboard was reference using planes over solid objects. With  
  solid objects we need to hallow them out (e.g., in the cab) which is very     
  fragile. 5/ For wheel count base it on the reference images 6/ Body vs cab    
  seam - the last session was flakey so I asked it to dump the state into that  
  .md file and we'd start fresh here. 6/ For collection count the important     
  part is we align with the                                                     
  /apps/git/unreal_experiments/Source/DudeWalks/Vehicle/VehicleBase.cpp and how 
  that works for Ambulance and Police car.

## Concept

A **Cartoon-style aerial ladder truck** for puzzle/navigation gameplay. Player drives the truck into position, deploys outriggers, aims the ladder, and sprays water from the ladder tip to extinguish animated cartoon fires. Dual-player support: Player 1 drives from the cab, Player 2 rides/operates from the basket tip. Single-player can swap between cab and basket.

**Aesthetic:** Toy-like, colorful, approachable. Inspired by Pierce aerial ladder trucks.

**Design Requirement**: 
 - Construct the vehicle like cardboard instead of solid objects. Then we don't need to hallow out afterward
 - Use Scene Collection tree to nest child objects (Body, Wheels, LightBar, ...)
 - Tires (spelled Tires-- not Tires) are perpendicular to vehicle and will rotate in unreal
 - Steering wheel is on the (American) left side
 - The driver and passenger need to be visible in the cab
 - Don't over-engineer geometry and ensure peices connect at natural vertices
 - Include unit and integration tests like the existing police car (working reference)
 - Use 1 meter = 1 meter scale 
 - see .docs/BlenderToUEWorkflow.md for export instructions

---

## Status

Branch: `feature/firetruck-vehicle` (not yet created).

| Area | Status |
|------|--------|
| Reference gathering (exterior, interior, basket, specs) | ✅ Done |
| Design spec / implementation plan | ✅ Done (this doc) |
| Blender model (cab, body, ladder, basket, outriggers) | ❌ Pending |
| UE import | ❌ Pending |
| Materials | ❌ Pending |
| FireTruck C++ / Blueprint (extends VehicleBase) | ❌ Pending |
| Ladder animation (raise + extend) | ❌ Pending |
| Outrigger deploy mechanism | ❌ Pending |
| Water spray Niagara system (ladder tip) | ❌ Pending |
| Fire particle system + extinguish logic | ❌ Pending |
| Dual-player camera (cab + basket rigs) | ❌ Pending |
| Player transition (cab ↔ basket) | ❌ Pending |
| Level design (fires, obstacles, puzzle layout) | ❌ Pending |
| Merge review + push to master | ❌ Pending |

---

## Vehicle Dimensions & Specs

All values from real-world aerial ladder truck specifications. Scale to UE in cm (1 Blender meter = 100 UE units).

### Overall Truck

| Dimension | Value | UE (cm) |
|-----------|-------|---------|
| Length | 35-51 ft | 1067-1554 |
| Width | 8-8.5 ft (96-100") | 244-254 |
| Height | 10-13 ft | 305-396 |
| Gross weight | 53,000-80,000 lbs | (N/A) |
| Axles | 3 (1 front, 2 rear tandem) | 6 wheels |

### Ladder

| Spec | Value | UE (cm) |
|------|-------|---------|
| Length (stored) | ~75-105 ft | 2286-3200 |
| Length (extended) | up to 137 ft | 4176 |
| Vertical reach | up to 13 stories | ~4000 |
| Tip weight capacity | 500-750 lbs | (N/A) |
| Waterway flow | 1000-1500 GPM | (N/A) |
| Max elevation angle | ~70° | 70 |
| Pivot location | Behind cab, above body module | TBD during modeling |

### Cab

- Wide cockpit, two front-facing bucket seats (driver + officer)
- Elevated seating — floor well above road (~300-350 cm)
- Center console with pump controls, gear shifter, comms
- Flat utilitarian dash with analog gauge cluster
- Overhead console (lights, sirens, radio)
- Massive vertical windshield
- Grab handles on pillars, step-up entrance

---

## Visual Design

### Color Scheme

- **Cab body:** Red lower panels, white upper cab
- **Body module:** Bright red roll-up compartment doors
- **Ladder:** White/silver aluminum truss
- **Accents:** Chrome grille, chrome wheel rims, white reflective stripe along body
- **Lights:** Red (primary), amber (secondary)

### Truck Breakdown (front to rear)

```
[Front Bumper] → [Cab] → [Pump Panel] → [Body Module / Compartments] → [Rear]
                     ↑          ↑                ↑                           ↑
                   Cab interior,       Gauges/         4-5 roll-up          Diamond plate
                   steering wheel      valves          doors with           tool tray,
                   dash, seats,        Comms           hose bays,           stored hand
                   radio               Shifter         SCBA, nozzles        ladders,
                   Light bar on roof   Switch panel    Emergency            step,
                     above             Radio           release handles      handrails,
                   Windshield (tall)   Overhead        Rear tool tray       "BACK 500 FT"
                   Side mirrors on     console                        marking
                   stalks

On top: [Ladder stored horizontally, raises from rear pivot housing]
         Small light bar on ladder housing
         Coiled hose on body next to pivot
```

### Rear Profile

- Diamond plate tool tray platform
- Black tubular handrails on top
- Two hand ladders stored vertically on rear doors
- Rear step for climbing onto tool tray
- "BACK 500 FEET" marking
- Vertical light clusters (brake/turn/backup) on each corner
- Red clearance lights on top of body and ladder housing
- Outrigger deployment from lower rear quarter panel

---

## Reference Images

Reference photos were gathered during brainstorming (June 2, 2026). Key references:

- **Pierce aerial ladder truck (front 3/4, stored ladder):** Full truck detail — red/white cab, chrome grille, side compartments, ladder over cab
- **Pierce aerial ladder truck (side profile, ladder deployed):** Sunset Beach / Bay Leaf — ladder raised ~70°, outriggers deployed, full side view
- **Aerial tower with basket (side, deployed):** Bay Leaf tower — shows basket/platform at tip, outrigger feet, tandem axles
- **Rear 3/4 view (Cincinnati Ladder 19):** Rear tool tray, hand ladders, compartment doors, ladder base housing, outrigger location, "BACK 500 FEET" marking
- **Cab interiors (3 variants):** Modern digital dash, traditional analog gauge cluster, Pierce cab with pump controls — center console, steering wheel, overhead console, grab handles
- **Basket interior (3 variants):** Enclosed platform with control joystick, seats, safety rails, red/white/blue color scheme
- Ask for them when needed

---

## Gameplay Mechanics

### Core Loop

1. **Drive** — navigate the truck to a position near the fire
2. **Deploy** — lower outriggers for stability (must be on level ground)
3. **Aim** — raise and extend the ladder toward the fire
4. **Spray** — activate water from the ladder tip to extinguish the fire
5. **Repeat** — move to next fire or navigate to new areas

### Ladder Controls

| Input | Action | Value Type |
|-------|--------|------------|
| `IA_LadderAngle` | Raise/lower ladder (0° to ~70° elevation) | AXIS1D |
| `IA_LadderExtend` | Telescoping extend/retract | AXIS1D |
| `IA_WaterSpray` | Activate/deactivate water stream | BOOLEAN |
| `IA_Outriggers` | Deploy/retract outriggers | BOOLEAN |

**Constraints:**
- Ladder cannot raise unless outriggers are deployed
- Ladder has a maximum reach arc — target must be within range
- Basket tip has a weight limit (cosmetic — just prevents climbing out if over capacity)

### Water Spray

- **Emitter:** Niagara particle system attached to basket tip transform
- **Visual:** Cone of white water with gravity arc, mist, and ground splash
- **Hit detection:** Volume-based — fire has a bounding sphere; when water stream overlaps, fire begins dousing
- **Arc behavior:** Natural arc follows ladder angle. Higher angle = water falls closer to truck. Lower angle = water reaches further.

### Fire System

- **Visual:** Niagara particle system — orange/red flames + smoke rising from source
- **Health:** Fire has a `Health` value. Water stream reduces health over time
- **States:**
  1. `Burning` — full flames, visible smoke column
  2. `Dousing` — flames shrink, more smoke/steam
  3. `Extinguished` — harmless steam/smoke puff, then fades
- **Feedback:** Bright → dim transition is satisfying. Steam/smoke clears area visibility when fire goes out.

### Fires (Kid-Friendly)

- Animated, cartoon-style fire sprites
- Not scary — more "dancing flames" than inferno
- Placed on rooftops, in towers, atop elevated platforms
- Smoke columns serve as visual waypoints (spot fires from distance)

### Outriggers

- **Visual:** Stabilizer legs slide out from the truck body (two per side, front and rear)
- **Deploy animation:** Slide outward and downward
- **Requirement:** Must be deployed before ladder can raise
- **Ground check:** Only work on flat, level ground

---

## Dual-Player Camera Model

### Default Configuration

```
Player 1 (Driver in Cab)                          Player 2 (Basket Operator)
┌──────────────────────────┐                     ┌──────────────────────────┐
│ Camera: Driver seat      │                     │ Camera: Basket tip       │
│ Spring arm from seat     │                     │ Spring arm from basket   │
│                          │                     │                          │
│ Controls:                │                     │ Controls:                │
│  - Vehicle drive         │                     │  - Fine ladder aim       │
│  - Ladder angle          │                     │  - Water monitor         │
│  - Ladder extension      │                     │  - Character climb out   │
│  - Outriggers            │                     │  - Interact with objects │
│  - Water spray (primary) │                     │  - Water spray (fine)    │
└──────────────────────────┘                     └──────────────────────────┘
         │                                                   │
         └──── FireTruck Actor (shared, networked) ─────────┘
```

### Single-Player Mode

Same setup, one character swaps between positions:

| Transition | Description |
|------------|-------------|
| Cab → Basket | Park truck, deploy outriggers, climb to basket. Character animation: exit cab → climb ladder → enter basket |
| Basket → Cab | Ride basket down, climb back to cab |

**Transition trigger:** Button press when truck is stationary and outriggers are deployed.

---

## UE Architecture

### Asset Path

`/Game/Vehicles/FireTruck/`

### Mesh Hierarchy

Separate FBX files by functional group:

```
FireTruck/
├── Body/             Cab shell, body module, compartments, rear tool tray, pump panel
├── Interior/         Dash, seats, steering wheel, center console, overhead console, pillars, windshield glass
├── Wheels/           Single wheel mesh (reused × 6)
├── Ladder/           Ladder base housing + truss sections (telescoping)
├── Basket/           Platform at ladder tip with controls, seats, safety rails
└── Outriggers/       Stabilizer legs (front pair, rear pair)
```

**Why separated:** Ladder and basket move (animated hierarchy). Interior needs separate geometry for visibility (see Pitfalls below).

### Blueprint: BP_FireTruck

**Parent:** `VehicleBase` (same as police car — inherits drive physics, tire system, seats, camera)

**Additional Components:**

| Component | Type | Attachment | Purpose |
|-----------|------|------------|---------|
| `LadderBase` | `USceneComponent` | BodyMesh | Pivot point for ladder raise |
| `LadderMain` | `USceneComponent` | LadderBase | Primary ladder section |
| `LadderExtend` | `USceneComponent` | LadderMain | Telescoping extension section |
| `BasketTip` | `USceneComponent` | LadderExtend | Platform at ladder end (basket + water emitter) |
| `BasketCameraBoom` | `USpringArmComponent` | BasketTip | Camera arm for basket operator |
| `BasketCamera` | `UCameraComponent` | BasketCameraBoom | Camera for basket view |
| `OutriggerFL/FR` | `USceneComponent` | BodyMesh | Front stabilizers (animated) |
| `OutriggerRL/RR` | `USceneComponent` | BodyMesh | Rear stabilizers (animated) |
| `WaterEmitter` | `UNiagaraComponent` | BasketTip | Water spray particle system |
| `TireRL2/RR2` | `UStaticMeshComponent` | BodyMesh | Additional rear tandem wheels (VehicleBase has 4, truck needs 6) |

### Additional Seats

| Seat | Purpose |
|------|---------|
| `SeatDriver` | Driver position (Player 1 default) |
| `SeatOfficer` | Passenger position (unused / NPC) |
| `SeatBasket` | Basket operator position (Player 2 default, or P1 when swapped) |

### C++ Extensions to VehicleBase

New `AFireTruck : AVehicleBase` with:

```cpp
// Ladder control
UFUNCTION(BlueprintCallable, Category = "FireTruck")
void SetLadderAngle(float AngleDegrees);   // 0° (stored) to 70° (max elevation)

UFUNCTION(BlueprintCallable, Category = "FireTruck")
void SetLadderExtension(float ExtensionRatio);  // 0.0 (stored) to 1.0 (full extend)

UFUNCTION(BlueprintCallable, Category = "FireTruck")
void ToggleOutriggers();  // Deploy/retract stabilizers

UFUNCTION(BlueprintCallable, Category = "FireTruck")
void ToggleWaterSpray();  // Activate/deactivate Niagara water

// State
UFUNCTION(BlueprintCallable, Category = "FireTruck")
bool AreOutriggersDeployed() const;

// Basket camera
UFUNCTION(BlueprintCallable, Category = "FireTruck")
void SetViewMode(EViewMode Mode);  // DRIVER, BASKET, FREE

// Player transition
UFUNCTION(BlueprintCallable, Category = "FireTruck")
void MovePlayerToBasket(ACharacter* Character);
void MovePlayerToCab(ACharacter* Character);
```

**Ladder animation:**
- `LadderAngle` rotates `LadderMain` around its pivot axis (Y-axis rotation relative to stored position)
- `LadderExtension` scales or translates `LadderExtend` along the ladder's local forward axis
- `BasketTip` follows automatically via component hierarchy
- `WaterEmitter` follows basket tip naturally

**Outrigger animation:**
- `ToggleOutriggers` → Timeline that slides each outrigger outward along local X and downward along local Z
- Ground check: only allow if terrain beneath is flat (simple raycast down from each outrigger position)

### Input Mapping

New input mapping context: `IMC_FireTruck` (priority 1, replaces IMC_Vehicle when in fire truck)

| Action | Key (gamepad) | Value Type | Purpose |
|--------|---------------|------------|---------|
| `IA_Throttle` | Right Trigger Axis (R2) | AXIS1D | Drive forward |
| `IA_Reverse` | Left Trigger Axis (L2) | AXIS1D | Drive reverse |
| `IA_Steer` | Left Thumbstick X (LeftX) | AXIS1D | Steering |
| `IA_ExitVehicle` | Face Button Right (Circle) | BOOLEAN | Exit current position |
| `IA_LadderAngle` | Right Thumbstick Y (RightY) | AXIS1D | Raise/lower ladder |
| `IA_LadderExtend` | Right Trigger / Left Trigger (contextual) | AXIS1D | Extend/retract |
| `IA_WaterSpray` | Right Bumper (R1) | BOOLEAN | Water on/off |
| `IA_Outriggers` | Left Bumper (L1) | BOOLEAN | Deploy/retract outriggers |
| `IA_SwitchPosition` | Face Button Top (Triangle) | BOOLEAN | Cab ↔ basket |

**NOTE:** `IA_LadderExtend` may conflict with `IA_Throttle/IA_Reverse` when in basket position. Use input blocking or context-dependent binding:
- **Driver view:** R2 = throttle, L2 = reverse
- **Basket view:** R2 = extend ladder, L2 = retract ladder (driving disabled)

### DudeWalksCharacter Changes

```cpp
// New enum
UENUM(BlueprintType)
enum class EViewMode : uint8
{
    OnFoot UMETA(DisplayName = "On Foot"),
    Driver UMETA(DisplayName = "Driver"),
    Basket UMETA(DisplayName = "Basket")
};

// New property
UPROPERTY()
EViewMode CurrentViewMode;

// New method
UFUNCTION()
void SwitchFireTruckPosition();  // Toggle between cab and basket
```

---

## Cab Interior — Visibility Fix

**Problem (from ambulance):** Solid shell mesh hides character inside cab. Cannot see driver from outside.

**Solution:**

1. **Interior as separate mesh** — dash, floor, ceiling, A-pillars, windshield glass are a separate `Interior` mesh from the exterior `Body`. Interior geometry is the *inside surfaces* of the cab.

2. **Open roof or translucent roof** — either remove roof triangles entirely, or use a separate roof mesh with a semi-transparent material. This lets exterior cameras (top-down, side) see the driver character.

3. **Backface material option** — if using a single mesh, use `two_sided = True` on all interior materials AND ensure the mesh has both exterior-facing and interior-facing geometry (doubles the polygon count but guarantees visibility).

4. **Validate early** — during blocking phase (simple box geometry), place a Duder character at the driver seat and verify visibility from exterior angles. Only add detail after this works.

### Cab Interior Layout (from references)

```
     [overhead console: lights, sirens, radio]
     ┌─────────────────────────────────────┐
     │                                     │
     │  [grab]                           [grab]
     │    │                                 │
     │    │  ____________  ____________    │
     │    │ |  windshield|  windshield |   │
     │    │ | (tall glass| (tall glass)|   │
     │    │ |            |            | |   │
     │    │ |  [steering | [gauges]  | |   │
     │    │ |  wheel ]   | switches  | |   │
     │    │ |            |__________| |   │
     │    │ |________________________| |   │
     │    │ | [dash] [center console] | |   │
     │    │ |                      [radio] |│
     │    │ |____________________________| |
     │    │                                │
     │    │ [seat]   [console]   [seat]    │
     │    │ driver   pump/shift officer    │
     │    │                                │
     │    │____________  ________________  │
     │             \  /  \  /             │
     │              \/    \/              │
     │           [floor] [floor]          │
     │                                    │
     └────────────────────────────────────┘
          step-up entrance on each side
```

---

## Basket / Platform

### Design (from references)

- Enclosed platform at ladder tip
- 2-3 person capacity with seats
- Control joystick for fine ladder adjustment
- Safety rails on all sides
- Red/white/blue color scheme
- Water monitor/nozzle mounted at the front
- Entry/exit from the side (firefighter climbs in from ladder)

### Camera

- `BasketCameraBoom` (SpringArm) attached to `BasketTip`
- `BasketCamera` at boom end, pointing outward and slightly downward
- Spring arm length: ~150-200 cm (pushes camera outside basket for good view)
- Camera can look freely (not locked to ladder direction)

---

## Niagara Particle Systems

### Water Spray (`Niagara_WaterSpray`)

- **Source:** Attached to `BasketTip` transform
- **Visual:** White cone/stream with gravity arc
- **Behavior:**
  - Particles emit from basket nozzle forward/downward based on basket orientation
  - Gravity pulls stream into natural arc
  - Secondary mist particles at the stream edges
  - Splash particles where stream hits ground surface (use collision or distance-based spawning)
- **Controls:** Toggle via `ToggleWaterSpray()`, activate/deactivate the system
- **Tuning:**
  - Emit rate: high (~500-1000/sec) for thick stream look
  - Velocity: matches ladder angle (use Niagara parameter to drive initial velocity direction from ladder transform)
  - Size: medium, toy-like (not photorealistic)
  - Color: white with slight blue tint

### Fire (`Niagara_Fire`)

- **Source:** At fire location (actor or world placement)
- **States:**
  1. **Burning:** Full orange/red flames, rising smoke
  2. **Dousing:** Flames shrink, more white steam/smoke
  3. **Extinguished:** Steam puff, then fade
- **Controls:** Exposed `Health` parameter (0.0 = out, 1.0 = full fire) drives visual state
- **Volume:** Fire has a `USphereComponent` for collision with water stream

### Smoke Column (`Niagara_SmokeColumn`)

- **Source:** Rising from burning fire
- **Purpose:** Visual waypoint — tells player where fires are from a distance
- **Appearance:** Wispy gray smoke, rises and dissipates
- **Scales with fire health** (more smoke when dousing, less when burning)

---

## Modeling Approach

### Phase 1: Blocking

1. Create simple box primitives for each major component
2. Assemble in UE at correct scale using the dimensions table above
3. Place Duder character at driver seat position
4. Place camera rigs at both driver and basket positions
5. Verify:
   - Driver character visible from exterior cameras
   - Chase camera has clear view
   - Basket camera has good outward view
   - Ladder pivot point feels correct
   - Overall scale reads right compared to environment

### Phase 2: Detail

1. Model each component in Blender with proper topology
2. Organize into collections (Body, Interior, Wheels, Ladder, Basket, Outriggers)
3. Recalculate face normals before export (UE axis hints flip horizontal normals)
4. Export as separate FBX files
5. Import to UE, assign materials immediately after import
6. Reassemble in Blueprint, position all components

### Phase 3: Animation

1. Wire ladder raise animation (rotate LadderMain around pivot)
2. Wire ladder extension (translate/scale LadderExtend along local forward)
3. Wire outrigger deployment (slide outward + downward)
4. Wire player transition animations (climb from cab to basket)

### Phase 4: Polish

1. Niagara particle systems
2. Lighting (movable point lights for emergency lights)
3. Sound (engine, siren, ladder hydraulics, water spray)
4. Level design

---

## Material Plan

Estimated material slots in `/Game/Vehicles/FireTruck/Materials/`:

| Material | Purpose | Two-Sided? |
|----------|---------|------------|
| `M_RedPaint` | Cab lower body, body module, compartments | Yes |
| `M_WhitePaint` | Cab upper body, ladder truss, basket | Yes |
| `M_Chrome` | Grille, wheel rims, mirrors, bumpers | Yes |
| `M_Glass` | Windshield, cab windows | No (translucent) |
| `M_TireRubber` | All 6 wheels | Yes |
| `M_InteriorBlack` | Dash, seats, console, floor | Yes |
| `M_Gauge` | Dashboard gauges, switch panels | Yes |
| `M_LightRed` | Emergency lights, clearance lights | Yes |
| `M_LightAmber` | Turn signals, secondary lights | Yes |
| `M_DiamondPlate` | Tool tray, bumper, steps | Yes |
| `M_ReflectiveStripe` | White stripe along body | Yes |
| `M_Aluminum` | Ladder truss, outriggers, pump panel | Yes |

---

## Level Design (Initial Concepts)

### Theme

Cartoon city/town with fires at various heights and positions. Player drives the fire truck, aims the ladder, and sprays water to extinguish each fire.

### Puzzle Elements

- **Ground fires** — extinguishable from truck monitor without ladder
- **Elevated fires** — require ladder raise + extension to reach
- **Sequence puzzles** — put out fire A to clear smoke blocking fire B
- **Navigation challenges** — fallen obstacles block roads, ladder provides alternative routes
- **Height challenges** — fires on tall towers require precise positioning and outrigger placement

### Environment

- Flat areas for outrigger deployment (marked with fire lanes or clear pavement)
- Elevated platforms, rooftops, towers with fires
- Obstacles: fallen trees, debris, smoke
- Collectibles: hidden at heights only reachable by ladder
- Water sources: hydrants (cosmetic — truck carries its own water)

### Progression

1. Clear all fires in an area → level complete
2. Victory condition: lights flash, siren chirps, smoke clears
3. Timer or damage meter (fires grow if ignored too long)
4. Bonus collectibles for exploration

---

## Acceptance Test (Full Cycle)

1. Duder walks to front of truck → press Triangle → enters driver seat; overhead camera activates
2. Driver controls mirror police car mechanics
3. Drive truck to a fire zone
4. Press L1 → outriggers deploy (slide outward and downward)
5. Duder walks to back of truck → press Triangle → enters basket; dedicated basket camera activates
6. Left stick → rotates ladder; L2/R2 → up/down; X → sprays water
7. Aim basket at fire → press X → water stream hits fire → fire health decreases → extinguishes with steam puff
8. Hold Circle → exits back to ground from either position
9. Drive to next fire, repeat

---

## Build

```bash
cd /apps/git/unreal_experiments
make DudeWalksEditor  # ~12s incremental
```

---

## Pipeline Pitfalls (Carried from Police Car + New)

| Pitfall | Fix |
|---------|-----|
| FBX reimport wipes material slot assignments | Reassign via Python immediately after import |
| Input actions default to BOOLEAN | Set `value_type = AXIS1D` for gamepad axis/trigger actions |
| FBX axis hints flip horizontal normals | Run `bmesh.ops.recalc_face_normals` in Blender before export |
| BodyMesh blocks ECC_Camera → SpringArm collapses | `SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore)` |
| Stationary lights cause baked lighting warnings | Set to Movable in Python or editor |
| **Solid cab hides driver character** | Separate interior mesh, open/translucent roof, validate early |
| **Ladder hierarchy breaks if pivot is wrong** | Test with blocking geometry first, verify pivot before modeling detail |
| **Niagara water needs orientation from ladder** | Expose Niagara parameter for velocity direction, drive from basket tip transform |
| **Water stream hit detection** | Use volume overlap (fire sphere component), not raycast |
| **Outrigger ground check** | Simple raycast down from each outrigger; reject if terrain is sloped or elevated |

---

## Blender to UE Workflow

Follow `.docs/BlenderToUEWorkflow.md` for FBX export/import. Key settings:

```python
bpy.ops.export_scene.fbx(
    filepath='RawAssets/Vehicle/FireTruck_{collection}.fbx',
    use_active_collection=True,
    apply_unit_scale=True,
    apply_scale_options='FBX_SCALE_ALL',
    axis_forward='-Z',
    axis_up='Y',
    global_scale=1.0,
)
```

**Collections:** Body, Interior, Wheels, Ladder, Basket, Outriggers

**UE import path:** `/Game/Vehicles/FireTruck/`

---

## Blocking Scaffold (Blender Phase 1)

First pass: simple cubes at approximate positions. All coordinates in Blender meters (+X = forward, +Z = up, Y=0 = center).

```
                    ┌──────── Ladder (stored on top) ────────┐
                    │                                        │
           ┌────────┴────────┐                               │
           │   Cab           │  ┌── Body ceiling ────────────┤
     3.5m ┼─┤                 ├─┤                              │
           │  ┌────────┐ ┌──┐ │  │                              │
     3.0m ┼──│ windshield│ │cab│ ├─┤                              │
           │  └────────┘ │cab│ │  │                              │
     2.5m ┼──┤            ├────┤  ├──────────────────────────────┤
           │  │  interior │    │  │                              │
     2.0m ┼──┤  (seats)  │    │  ├──────────────────────────────┤
           │  │           │    │  │  ┌─┐  ┌─┐  ┌─┐  ┌─┐         │
     1.8m ┼──┘ └─────────┘    └──┘  │D1│  │D2│  │D3│  │D4│ ┌───┤
           │       ↑      │         │  │  │  │  │  │  │  │   │
     1.0m ┼──│   floor    │         └──┴──┴──┴──┴──┴──┴──┴──┴──┤
           │    at 1.8m   │                                    │
     0.5m ┼───────────────┘                                    ┼─┤
           │      bumper                                       │ │
     0.3m ┼────────────────────────────────────────────────────┼─┘
           │                                                   
     0m  ──┼─────────────────────────────────────────────────────┼── ground
           │                                                   
          -1m   0m    1m    2m    3m    4m    5m    6m    7m    8m    9m   10m
               rear                                              front
```

### Block Positions (cube center, Blender meters)

These were the initial blocking targets. The **actual modeled measurements** are below in the Investigation Findings section.

| Piece | X | Y | Z | Dimensions (X×Y×Z) | Notes |
|-------|---|---|---|---|---|
| **Cab** | 4.0 | 0 | 3.0 | 2.5 × 2.6 × 2.0 | Floor at ~2.0m (1.8m ground clearance) |
| **Body** | 7.5 | 0 | 1.4 | 6.0 × 2.8 × 2.0 | Compartments, pump panel, rear tool tray |
| **Bumper** | 5.3 | 0 | 0.35 | 0.3 × 2.6 × 0.4 | Front bumper, low to ground |
| **Ladder** | 7.0 | 0 | 3.6 | 6.5 × 0.8 × 0.5 | Stored horizontally on body top |

### Connection Points
- Cab rear face (X=2.75) **touches** Body front face (X=4.5) — 1.25m overlap so they merge
- Cab bottom (Z=2.0) sits **1.8m above ground** (fire truck cab floor height)
- Body bottom (Z=0.4) sits **0.3m above ground** (body floor height)
- Ladder rests on top of body module
- Bumper sits in front of cab, low

### Lessons Learned (Session Pitfalls)
- Start with **simple aligned cubes** before adding any detail
- **Verify alignment visually** (side view screenshot) before proceeding
- Cab and body must **touch/overlap** — gaps look broken
- Bumper must be **low to ground** — don't float it at cab height
- Keep it to **4-5 blocks** initially, then subdivide

---

## 🔍 Investigation Findings (Session 2026-06-02)

**File:** `Content/Animations/Vehicle/FireTruck.blend`
**Investigation time:** ~25 min. Don't repeat — skip to this section next time.

### Actual Modeled Geometry (Measured from Blender bounding boxes)

All objects at `location=(0,0,0)`. Bounding boxes are world-space mesh extents.

| Piece | Center (X,Y,Z) | Dimensions (X×Y×Z) | Verts | Faces | Materials Assigned |
|-------|------|------|------|------|------|
| **Cab** | (3.05, 0.00, 1.70) | 3.15×3.22×3.14 | 702 | 648 | M_RedPaint (648 faces), M_WhitePaint (0), M_Glass (0) |
| **Body** | (6.59, 0.00, 1.26) | 8.59×3.36×2.51 | 15694 | 16098 | M_RedPaint (16098 faces), M_WhitePaint (0), M_DiamondPlate (0) |
| **FrontBumper** | (4.19, 0.00, 1.75) | 1.52×2.60×3.20 | ? | ? | ? |
| **Ladder** | (6.45, 0.00, 2.98) | 5.31×0.53×1.16 | 280 | 210 | M_Aluminum, M_WhitePaint |
| **Basket** | (3.20, 0.00, 3.70) | 1.55×1.12×0.83 | 104 | 78 | M_WhitePaint, M_RedPaint, M_Aluminum |
| **Outriggers** | (8.47, 0.00, 0.29) | 2.35×3.55×0.32 | 96 | 72 | M_Aluminum |
| **Wheel_FL** | (0.00, 0.00, 0.00)* | 0.40×0.76×0.76 | 540 | 546 | M_TireRubber, M_Hub |

*Wheel_FL is in the `Wheel` collection at origin. 5 preview instances (FR, RL1, RR1, RL2, RR2) placed in Scene Collection at axle positions.

### Overall Truck Dimensions (Blender meters → UE cm)

| Metric | Value | UE cm | Spec Range | Status |
|--------|-------|-------|------------|--------|
| **Length** | 11.1m | 1109 cm | 1067-1554 | ✓ Within spec |
| **Width** | 3.6m | 355 cm | 244-254 | ⚠️ Wide (includes outriggers) |
| **Height** | 4.5m | 449 cm | 305-396 | ⚠️ Tall (includes basket) |
| **Wheel radius** | 0.38m | 38 cm | 38 (TireRadius) | ✓ Matches VehicleBase |

### Wheel Orientation — CORRECT AS-IS

- Wheel mesh has axle along Blender **local Z** (lying flat like a disc in viewport)
- FBX export uses `axis_forward='-Z', axis_up='Y'`
- This remaps Blender Z-axle → UE X-axle (forward)
- UE `Tick()` rotates tires around **Pitch (X)** → wheel spins correctly ✓
- **Do NOT rotate the wheel mesh.** The "flat" appearance in Blender is the correct orientation for export.
- Wheel diameter = 0.76m, radius = 38cm matches `VehicleBase.TireRadius = 38.f`

### Cab Interior

- **702 vertices, 648 faces** — simple shell geometry
- **244 interior faces** exist (normals face inward toward cab center) — good, character will be visible from inside
- **404 exterior faces** face outward
- **No detailed interior:** no separate dash, seats, or steering wheel geometry
- Consistent with PoliceCar approach (simple shell, no interior detail)
- **Solid red roof** (28 faces at Z > 3.12) — character will NOT be visible from above
- **Windshield is solid red** — character will NOT be visible from front

### Material Assignment Issues

**Critical:** All cab faces use `M_RedPaint`. `M_WhitePaint` and `M_Glass` slots exist but have 0 faces assigned.
Same for body — all faces use `M_RedPaint`, `M_DiamondPlate` has 0 faces.

| Object | Should be White | Should be Glass | Should be DiamondPlate |
|--------|----------------|----------------|---------------------|
| Cab | Upper cab (Z > ~2.5) | Windshield (front, X < 1.65, Z > 2.0) | — |
| Body | — | — | Rear tool tray (X > ~10) |

### Collection Structure (Ready for Export)

```
Scene Collection
├── Cab → [Cab]              ← Export as Cab.fbx (combine_meshes=True)
├── Body → [Body]            ← Export as Body.fbx (combine_meshes=True)
├── FrontBumper → [FrontBumper]  ← Export as FrontBumper.fbx
├── Ladder → [Ladder]        ← Export as Ladder.fbx
├── Basket → [Basket]        ← Export as Basket.fbx
├── Outriggers → [Outriggers] ← Export as Outriggers.fbx
├── Wheel → [Wheel_FL]       ← Export as Wheel.fbx (single mesh, reused ×6 in UE)
├── Wheel_FR (preview instance, NOT for export)
├── Wheel_RL1 (preview instance)
├── Wheel_RR1 (preview instance)
├── Wheel_RL2 (preview instance)
└── Wheel_RR2 (preview instance)
```

### Cab/Body Connection

- **Cab bounding box:** X: 1.47 → 4.62
- **Body bounding box:** X: 2.30 → 10.89
- **Overlap:** 4.62 - 2.30 = **2.33m** ✓ (cab rear extends well into body front)
- Cab bottom Z: 0.13, Body top Z: 2.51 → cab sits on and above body
- Cab floor at ~0.13m above ground (low, body fills underneath)
- **Will merge cleanly** on FBX import with `combine_meshes=True`

### Blender-to-UE Mapping (Verified)

Per `BlenderToUEWorkflow.md`:
- Blender +X = UE +X (forward) — car faces +X ✓
- Blender +Z = UE +Z (up) — gravity direction ✓
- Blender +Y = UE +Y (right) — but note: American left-side driver means **negative Y**
- Export: `global_scale=1.0`, `apply_scale_options='FBX_SCALE_NONE'`, `axis_forward='-Z'`, `axis_up='Y'`
- FBX natively converts Blender meters → cm. UE imports cm as UE units.

### Pending Fixes Before Export

1. **Material assignment** — select upper cab faces → `M_WhitePaint`; windshield → `M_Glass`; rear tool tray → `M_DiamondPlate`
2. **Cab visibility** — delete roof faces (open roof per cardboard-style spec) OR make translucent
3. **Verify** — no gaps at cab/body seam, wheel positions match body edges

### How to Skip This Investigation Next Time

Jump straight to this section. The geometry is modeled and positioned. Skip to:
1. Fix materials (select faces by Z/X region, assign correct material slot)
2. Fix cab visibility (delete roof or make translucent)
3. Export collections per the Collection Structure above
4. Import to UE per `BlenderToUEWorkflow.md`

---

## Open Questions

1. **Ladder mesh strategy** — single truss mesh with skeletal bones, or multiple static meshes for telescoping sections?
2. **Water stream physics** — simple Niagara particles with overlap detection, or actual water stream with collision?
3. **Fire extinguish feedback** — just visual, or audio cue too?
4. **Co-op networking** — is this split-screen or online? Affects replication strategy for ladder state.
5. **Basket character animation** — simple teleport transition, or climb animation along ladder?
6. **Pump panel interactivity** — just visual detail, or do we want the player to interact with gauges/switches?
7. **Level scale** — one large town, or small distinct fire zones?
