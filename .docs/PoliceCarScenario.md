# Police Car Scenario

## Status

Branch: `feature/police-car-vehicle` — 7 commits ahead of `master`.

| Area | Status |
|------|--------|
| Blender model + export (Body/Wheels/LightBar) | ✅ Done |
| UE import (Body, Wheel, PoliceLightBar meshes) | ✅ Done |
| Materials (16 assets, all assigned + two-sided) | ✅ Done |
| VehicleBase C++ (tires, seats, physics, camera) | ✅ Done |
| DudeWalksCharacter (enter/exit, camera switch, input) | ✅ Done |
| IMC_DudeWalks — IA_EnterVehicle → Triangle | ✅ Done |
| IMC_Vehicle — Throttle/Reverse/Steer/Exit/Honk | ✅ Done |
| Input action value types (AXIS1D for analog axes) | ✅ Done |
| BP_PoliceCar (body, tires, SirenMesh, SirenLight) | ✅ Done |
| Dedicated vehicle chase camera (DriveCameraBoom) | ✅ Done |
| Lighting (DirectionalLight → Movable, no bake warnings) | ✅ Done |
| 19 C++ unit tests passing | ✅ Done |
| **Siren wiring (ToggleLights → flash red/blue)** | ❌ Pending |
| Seat position visual tuning (Z=136cm, may need tweak) | ⚠️ Needs PIE check |
| Merge review + push to master | ⏳ User to approve |

---

## Acceptance Test (full cycle)

1. Duder walks toward parked police car
2. Press Triangle → entering-vehicle animation plays
3. Animation finishes → Duder sits at the Driver seat; dedicated chase camera activates
4. R2 = forward, L2 = reverse, left stick = steer → car moves, all 4 tires spin
5. Drive a circle to confirm steering; drive forward and backward
6. Press Circle → exit-vehicle animation plays; character camera restores
7. Duder detaches, steps out left side, walks away normally

---

## Blender Export

The car was modelled as 116 separate Blender objects (96 in Body collection after grouping). Export as 3 separate FBX files by collection.

**Collection structure:**
```
Scene Collection
├── Body           (96 objects: shell, doors, glass, interior, seats, grille, lights, etc.)
├── Wheels         (16 objects: Tyre/Whitewall/Hubcap × FL/FR/RL/RR)
└── PoliceLightBar (DomeLightBase, PoliceDomeLight)
```

**Before exporting:** run `bmesh.ops.recalc_face_normals` on all Body objects to fix outward normals — the axis-hint transform (`axis_forward='-Z'`, `axis_up='Y'`) flips normals on horizontal surfaces (hood, roof) otherwise.

**FBX export settings:**
```python
bpy.ops.export_scene.fbx(
    filepath='RawAssets/Vehicle/PoliceCar_{collection}.fbx',
    use_active_collection=True,
    apply_unit_scale=True,
    apply_scale_options='FBX_SCALE_ALL',
    axis_forward='-Z',
    axis_up='Y',
    global_scale=1.0,
)
```

**Output files:**
- `RawAssets/Vehicle/PoliceCar_Body.fbx`
- `RawAssets/Vehicle/PoliceCar_Wheels.fbx`
- `RawAssets/Vehicle/PoliceCar_PoliceLightBar.fbx`

**Axle positions (Blender meters; × 100 = UE cm):**
```
Tire FL:  X= 1.320, Y= 0.995, Z= 0.380
Tire FR:  X= 1.320, Y=-1.045, Z= 0.380
Tire RL:  X=-1.320, Y= 0.995, Z= 0.380
Tire RR:  X=-1.320, Y=-1.045, Z= 0.380
Siren:    X= 0.300, Y= 0.000, Z= 1.670
```

---

## UE Import

Import each FBX to `/Game/Vehicles/PoliceCar/` with `combine_meshes=True`.

| FBX | UAsset | Notes |
|-----|--------|-------|
| PoliceCar_Body.fbx | `/Game/Vehicles/PoliceCar/PoliceCar_Body` | 14 material slots |
| PoliceCar_Wheels.fbx | `/Game/Vehicles/PoliceCar/PoliceCar_Wheel` | Single mesh reused for all 4 tires |
| PoliceCar_PoliceLightBar.fbx | `/Game/Vehicles/PoliceCar/PoliceCar_PoliceLightBar` | |

**Critical:** every reimport wipes material slot assignments on the static mesh. After any reimport, immediately reassign all slots via Python:
```python
mesh = unreal.load_asset('/Game/Vehicles/PoliceCar/PoliceCar_Body')
slot_map = {
    'M_Chrome': 'M_Chrome', 'M_PoliceBadge': 'M_PoliceBadge',
    'M_BlackPaint': 'M_BlackPaint', 'M_WhitePaint': 'M_WhitePaint',
    'M_Dash': 'M_Dash', 'M_Floor': 'M_Floor', 'M_Gauge': 'M_Gauge',
    'M_Glass': 'M_Glass', 'M_Headlight': 'M_Headlight',
    'M_Cage': 'M_Cage', 'M_Plate': 'M_Plate', 'M_Seat': 'M_Seat',
    'M_Taillight': 'M_Taillight', 'M_BlackPaint_001': 'M_BlackPaint_001',
}
for i, sm in enumerate(mesh.get_editor_property('static_materials')):
    slot = str(sm.get_editor_property('material_slot_name'))
    if slot in slot_map:
        mesh.set_material(i, unreal.load_asset(f'/Game/Vehicles/PoliceCar/Materials/{slot_map[slot]}'))
unreal.EditorLoadingAndSavingUtils.save_packages([mesh.get_outer()], False)
```

---

## Materials

16 material assets in `/Game/Vehicles/PoliceCar/Materials/`:

**Body (14):** `M_WhitePaint`, `M_BlackPaint`, `M_BlackPaint_001`, `M_Chrome`, `M_Glass`, `M_Headlight`, `M_Taillight`, `M_Seat`, `M_Dash`, `M_Floor`, `M_Gauge`, `M_Cage`, `M_Plate`, `M_PoliceBadge`

**Wheels (2):** `M_Rubber`, `M_Whitewall`

All 15 opaque materials: `two_sided = True` (belt-and-suspenders for normals). `M_Glass` left one-sided (translucent blend mode).

---

## VehicleBase C++

**Files:** `Source/DudeWalks/Vehicle/VehicleBase.h` / `VehicleBase.cpp`

**Components added:**
- `TireFL/FR/RL/RR` — `UStaticMeshComponent`, no collision, attached to BodyMesh
- `SeatDriver/Passenger/RearLeft/RearRight` — `USceneComponent` seat markers
- `DriveCameraBoom` — `USpringArmComponent` (600cm arm, -15° pitch, yaw-only, 120cm above body)
- `DriveCamera` — `UCameraComponent` attached to boom end

**Collision on BodyMesh:**
- `ECC_Pawn → ECR_Overlap` (character snaps to seat without being blocked)
- `ECC_Camera → ECR_Ignore` (chase cam never collapses into car body)

**Seat positions (C++ constructor):** Z = Blender seat cushion Z × 100 + 90cm (character mesh has -90cm root offset):
```cpp
SeatDriver->SetRelativeLocation(FVector(50.f, -26.f, 136.f));
SeatPassenger->SetRelativeLocation(FVector(50.f,  26.f, 136.f));
SeatRearLeft->SetRelativeLocation(FVector(-52.f, -26.f, 136.f));
SeatRearRight->SetRelativeLocation(FVector(-52.f,  26.f, 136.f));
```

**Driving physics (Tick):**
```
CurrentSpeed += ThrottleInput * Acceleration * dt
CurrentSpeed -= ReverseInput  * Acceleration * dt
CurrentSpeed  = FMath::FInterpTo(CurrentSpeed, 0, dt, Friction)
CurrentSpeed  = FMath::Clamp(CurrentSpeed, -MaxSpeed * 0.5f, MaxSpeed)
AddActorLocalOffset(FVector(CurrentSpeed * dt, 0, 0), sweep=true)
TurnDeg = SteerInput * TurnSpeed * dt * (|CurrentSpeed| / MaxSpeed) * Dir
AddActorLocalRotation(FRotator(0, TurnDeg, 0))
TireRollDeg = (CurrentSpeed / (2π * TireRadius)) * 360 * dt  → add to each tire's Pitch
```

**Defaults:** `MaxSpeed=1200`, `Acceleration=800`, `Friction=4`, `TurnSpeed=80`, `TireRadius=38`

---

## DudeWalksCharacter Changes

**EnterVehicle:** moves character to `GetSeatWorldLocation("SeatDriver")`, attaches to vehicle, swaps IMC (removes DefaultMappingContext, adds VehicleMappingContext priority 1), calls `PC->SetViewTarget(Vehicle)`.

**FinishExitVehicle:** detaches, steps left 150cm, calls `PC->SetViewTarget(this)`, restores DefaultMappingContext.

**Binding:** ThrottleAction/ReverseAction/SteerAction have null-guards in `SetupPlayerInputComponent` (the actions are optional; driving-only bindings).

---

## Input Mapping

### IMC_DudeWalks (on-foot)

| Action | Key |
|--------|-----|
| `IA_EnterVehicle` | Gamepad Face Button Top (Triangle) |
| Move/Look/Sprint/Jump | (existing) |

### IMC_Vehicle (driving, priority 1)

| Action | Key | Value Type |
|--------|-----|------------|
| `IA_Throttle` | Gamepad Right Trigger Axis (R2) | **AXIS1D** |
| `IA_Reverse` | Gamepad Left Trigger Axis (L2) | **AXIS1D** |
| `IA_Steer` | Gamepad Left Thumbstick X (LeftX) | **AXIS1D** |
| `IA_ExitVehicle` | Gamepad Face Button Right (Circle) | Boolean |
| `IA_Honk` | Gamepad Face Button Bottom (Cross) | Boolean |

**Critical:** IA_Throttle, IA_Reverse, IA_Steer must be `AXIS1D` (not `BOOLEAN`). Boolean types don't carry analog float values — the axis will always read 0.

---

## BP_PoliceCar Blueprint

- Parent: `VehicleBase`
- `BodyMesh` → `PoliceCar_Body`; all 14 material slots assigned
- `TireFL/FR/RL/RR` → `PoliceCar_Wheel`; relative locations set to axle offsets (in cm)
- **SCS component:** `SirenMesh (StaticMeshComponent)` → `PoliceCar_PoliceLightBar`, attached to BodyMesh
- **SCS component:** `SirenLight (PointLightComponent)` — hidden by default, attached to BodyMesh

### Pending: Siren Wiring

`ToggleLights` Blueprint event override not yet wired. Still needed:
1. Create `IA_Siren` input action (Boolean)
2. Add `IA_Siren` mapping to IMC_Vehicle (e.g., Left Bumper / L1)
3. Add `SirenAction` UPROPERTY to DudeWalksCharacter + bind in SetupPlayerInputComponent
4. Handler calls `CurrentVehicle->ToggleLights()` (only while Driving)
5. In BP_PoliceCar, override `ToggleLights` event: start/stop a flashing Timeline that alternates SirenLight color (red/blue) and toggles visibility

---

## Pipeline Pitfalls

| Pitfall | Fix |
|---------|-----|
| Reimport wipes material slot assignments | Always reassign via Python immediately after import |
| Input actions default to BOOLEAN | Explicitly set `value_type = AXIS1D` for any gamepad axis/trigger |
| FBX axis hints flip horizontal normals in UE | Run `bmesh.ops.recalc_face_normals` in Blender before export |
| `EditorAssetLibrary.save_asset()` doesn't flush binary | Use `EditorLoadingAndSavingUtils.save_packages([pkg], False)` |
| C++ `CreateDefaultSubobject` seats aren't SCS nodes | Set positions in C++ constructor; `manage_blueprint set_scs_transform` only works on BP-added components |
| BodyMesh blocks ECC_Camera → SpringArm collapses | `SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore)` in constructor |
| Stationary lights cause baked lighting rebuild warnings | Set to Movable in Python or editor |
| Hot reload leaves stale CDO (HOTRELOAD_VehicleBase_0) | Don't access CDO via Python after hot reload; encode defaults in C++ constructor instead |

---

## Tests

19 C++ unit tests passing in `Source/DudeWalks/DudeWalksTest.cpp`.

Coverage includes: tires created, seats created, seat lookup, unknown seat fallback, throttle increases speed, reverse decreases speed, friction coasts to stop, speed clamped at MaxSpeed, steering rotates actor, tires roll, character enters/exits, proximity check.

**Manual PIE only (not unit-testable):**
- Animations feel correct and timed right
- Tires visually spinning at correct rate
- Siren light flashing red/blue (once wired)
- Speed/turn/friction tuning feel good

---

## Build

```bash
cd /apps/git/unreal_experiments
make DudeWalksEditor  # ~12s incremental
```

---

## Future Vehicles

Same pattern for firetruck, bus, ambulance:
1. New Blender model → organize into Body/Wheels/SpecialPart collections → recalc normals → export FBXes
2. Import to `/Game/Vehicles/{VehicleName}/`; reassign materials immediately after import
3. Create `BP_{VehicleName} : VehicleBase`
4. Assign meshes; position seat markers visually in viewport
5. Override `ToggleLights` for vehicle-specific effect
6. Bus/multi-seat vehicles: unused seat markers stay at origin — never claimed
