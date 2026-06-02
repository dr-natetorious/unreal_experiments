# Police Car Scenario

## Overview

The police car is the first vehicle to prove the full vehicle mechanics. Firetruck, bus, ambulance, etc. follow the same pattern. Nothing police-specific lives in `VehicleBase` C++ — it stays generic.

**Acceptance test (full cycle):**
1. Duder walks toward parked police car
2. Press Triangle → entering-vehicle animation plays
3. Animation finishes → Duder sits at the Driver seat in driving animation loop
4. R2 = forward, L2 = reverse, left stick = steer → car moves, all 4 tires spin
5. Drive a circle to confirm steering; drive forward and backward
6. Press Circle → exit-vehicle animation plays
7. Duder detaches, steps out left side, walks away normally

---

## Blender Export

The car was modelled as 116 separate objects so the Blender MCP wouldn't corrupt geometry during edits. Export as 3 separate FBX files by Blender collection.

**Collection structure (flat siblings — simplest for per-collection FBX export):**
```
Scene Collection
├── Body           ← create via MCP
├── Wheels         ← create via MCP
└── PoliceLightBar ← already exists ✓
```

**Collection membership:**
- `Wheels` — `Tyre_FL/FR/RL/RR`, `Whitewall_FL/FR/RL/RR`, `Hubcap_FL/FR/RL/RR`, `Hubcap_FL/FR/RL/RR_Dome` (16 objects)
- `PoliceLightBar` — `DomeLightBase`, `PoliceDomeLight` (already grouped)
- `Body` — everything else (~98 objects: shell, doors, glass, interior, seats, grille, lights, etc.)

**Before exporting Wheels:** set each wheel group's origin to geometry center (so they spin around their axle in UE, not around world origin).

**Measured axle positions (Blender meters; multiply × 100 for approximate UE cm):**
```
Tire FL:  X= 1.320, Y= 0.995, Z= 0.380
Tire FR:  X= 1.320, Y=-1.045, Z= 0.380
Tire RL:  X=-1.320, Y= 0.995, Z= 0.380
Tire RR:  X=-1.320, Y=-1.045, Z= 0.380
Siren:    X= 0.300, Y= 0.000, Z= 1.670
Driver seat (approx): X= 0.429, Y=-0.260, Z= 0.729
```

**FBX export settings (per file):**
```python
bpy.ops.export_scene.fbx(
    filepath='RawAssets/Vehicle/PoliceCar_{collection}.fbx',
    use_active_collection=True,
    apply_unit_scale=True,
    apply_scale_options='FBX_SCALE_ALL',
    axis_forward='-Z',
    axis_up='Y',
)
```

**Output files:**
- `RawAssets/Vehicle/PoliceCar_Body.fbx`
- `RawAssets/Vehicle/PoliceCar_Wheels.fbx`
- `RawAssets/Vehicle/PoliceCar_PoliceLightBar.fbx`

---

## UE Import

Import each FBX to `/Game/Vehicles/PoliceCar/` via Python/MCP with `combine_meshes=True`:

| FBX | UAsset | combine_meshes | collision |
|-----|--------|----------------|-----------|
| PoliceCar_Body.fbx | `/Game/Vehicles/PoliceCar/PoliceCar_Body` | ✓ | auto-generate |
| PoliceCar_Wheels.fbx | `/Game/Vehicles/PoliceCar/PoliceCar_Wheel` | ✓ | none (body handles it) |
| PoliceCar_PoliceLightBar.fbx | `/Game/Vehicles/PoliceCar/PoliceCar_PoliceLightBar` | ✓ | none |

Note: A single `PoliceCar_Wheel` mesh is reused for all 4 tire components in the Blueprint.

---

## VehicleBase C++ Changes

### VehicleBase.h

**Add — generic wheel components (all wheeled vehicles share this):**
```cpp
UPROPERTY(VisibleAnywhere, Category="Vehicle") TObjectPtr<UStaticMeshComponent> TireFL;
UPROPERTY(VisibleAnywhere, Category="Vehicle") TObjectPtr<UStaticMeshComponent> TireFR;
UPROPERTY(VisibleAnywhere, Category="Vehicle") TObjectPtr<UStaticMeshComponent> TireRL;
UPROPERTY(VisibleAnywhere, Category="Vehicle") TObjectPtr<UStaticMeshComponent> TireRR;
```

**Add — named seat markers (positionable in Blueprint viewport):**
```cpp
// Vehicles that don't use all seats leave unused ones at origin — they're never claimed
UPROPERTY(VisibleAnywhere, Category="Vehicle|Seats") TObjectPtr<USceneComponent> SeatDriver;
UPROPERTY(VisibleAnywhere, Category="Vehicle|Seats") TObjectPtr<USceneComponent> SeatPassenger;
UPROPERTY(VisibleAnywhere, Category="Vehicle|Seats") TObjectPtr<USceneComponent> SeatRearLeft;
UPROPERTY(VisibleAnywhere, Category="Vehicle|Seats") TObjectPtr<USceneComponent> SeatRearRight;
```

**Add — driving physics state:**
```cpp
UPROPERTY(EditDefaultsOnly, Category="Driving") float MaxSpeed     = 1200.f; // cm/s
UPROPERTY(EditDefaultsOnly, Category="Driving") float Acceleration = 800.f;
UPROPERTY(EditDefaultsOnly, Category="Driving") float Friction     = 4.f;
UPROPERTY(EditDefaultsOnly, Category="Driving") float TurnSpeed    = 80.f;   // deg/s at full steer
UPROPERTY(EditDefaultsOnly, Category="Driving") float TireRadius   = 38.f;   // cm

float CurrentSpeed = 0.f;  // set each frame by ApplyThrottle/ApplyReverse

void ApplyThrottle(float Axis);
void ApplyReverse(float Axis);
void ApplySteering(float Axis);
```

**Add — generic lights hook (police siren, fire truck lights, ambulance, etc.):**
```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Vehicle")
void ToggleLights();
virtual void ToggleLights_Implementation() {}  // base: no-op
```

**Add — named seat lookup:**
```cpp
FVector GetSeatWorldLocation(FName SeatName) const;
```

**Remove:** `DriverSeatOffset`, `GetDriverSeatWorldLocation()`, `GetDriverSeatWorldRotation()`

### VehicleBase.cpp

**Constructor:** create and attach TireFL/FR/RL/RR + all 4 seat SceneComponents to BodyMesh.

**Tick:**
```
ThrottleInput / ReverseInput accumulated from ApplyThrottle/ApplyReverse calls this frame
CurrentSpeed += ThrottleInput * Acceleration * DeltaTime
CurrentSpeed -= ReverseInput * Acceleration * DeltaTime
CurrentSpeed  = FMath::FInterpTo(CurrentSpeed, 0, DeltaTime, Friction)   // coast to stop
CurrentSpeed  = FMath::Clamp(CurrentSpeed, -MaxSpeed * 0.5f, MaxSpeed)
AddActorLocalOffset(FVector(CurrentSpeed * DeltaTime, 0, 0), true)       // sweep for collision
AddActorLocalRotation(FRotator(0, SteerInput * TurnSpeed * DeltaTime * (CurrentSpeed / MaxSpeed), 0))
TireRollDeg  = (CurrentSpeed / (2π * TireRadius)) * 360 * DeltaTime
// Add TireRollDeg to each tire's local RelativeRotation.Y
```

**GetSeatWorldLocation:** finds child USceneComponent by name (`"SeatDriver"` etc.), returns its world location.

---

## DudeWalksCharacter Changes

### DudeWalksCharacter.h

Add driving input action refs:
```cpp
UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<UInputAction> ThrottleAction;
UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<UInputAction> ReverseAction;
UPROPERTY(EditDefaultsOnly, Category="Input") TObjectPtr<UInputAction> SteerAction;
```

### DudeWalksCharacter.cpp

**SetupPlayerInputComponent:** bind ThrottleAction, ReverseAction, SteerAction → private handlers that forward to `CurrentVehicle->ApplyThrottle/Reverse/Steering(axis)`. (No driving-state guard needed — VehicleMappingContext is only active while in vehicle.)

**EnterVehicle:** replace `GetDriverSeatWorldLocation()` → `GetSeatWorldLocation("Driver")`.

---

## IMC_Vehicle — New Input Mappings

Add via MCP or editor:

| Action | Input | Type |
|--------|-------|------|
| `IA_Throttle` | R2 (Gamepad Right Trigger) | Axis1D |
| `IA_Reverse`  | L2 (Gamepad Left Trigger)  | Axis1D |
| `IA_Steer`    | Left Stick X               | Axis1D |

---

## BP_PoliceCar Blueprint

- Parent: `VehicleBase`
- `BodyMesh` → `/Game/Vehicles/PoliceCar/PoliceCar_Body`
- `TireFL/FR/RL/RR` → `/Game/Vehicles/PoliceCar/PoliceCar_Wheel`; set relative locations to axle offsets (tune in viewport)
- **Add** `UStaticMeshComponent SirenMesh` → `PoliceCar_PoliceLightBar` *(police-specific, added in BP)*
- **Add** `UPointLightComponent SirenLight` — hidden by default, attenuation ~200, set red/blue alternating color via Blueprint Timeline
- **Override** `ToggleLights` event: start/stop the flashing Timeline, toggle SirenLight visibility
- Drag `SeatDriver`, `SeatPassenger`, `SeatRearLeft`, `SeatRearRight` scene components to visually match the seat geometry
- Place actor near player start in level

---

## Tests

### New Unit Tests (C++, same pattern as existing 19 in `DudeWalksTest.cpp`)

| Test | Confirms |
|------|----------|
| `VehicleBase.Components.TiresCreated` | TireFL/FR/RL/RR exist and are attached to BodyMesh |
| `VehicleBase.Components.SeatsCreated` | All 4 named seat SceneComponents exist |
| `VehicleBase.Seat.DriverLookup` | `GetSeatWorldLocation("Driver")` returns BodyMesh location + known offset |
| `VehicleBase.Seat.UnknownNameReturnsZero` | Graceful — no crash on unknown seat name |
| `VehicleBase.Driving.ThrottleIncreasesSpeed` | After `ApplyThrottle(1.0)` + Tick, `CurrentSpeed > 0` |
| `VehicleBase.Driving.ReverseDecreasesSpeed` | After `ApplyReverse(1.0)` + Tick, `CurrentSpeed < 0` |
| `VehicleBase.Driving.FrictionCoastsToStop` | No input + N Ticks → speed approaches 0 |
| `VehicleBase.Driving.SpeedClamped` | Many Throttle+Tick frames → speed never exceeds `MaxSpeed` |
| `VehicleBase.Driving.SteeringRotatesActor` | `ApplySteering(1.0)` + Tick at nonzero speed → actor yaw changes |
| `VehicleBase.Driving.TiresRollForward` | After forward Tick, `TireFL` local rotation Y != 0 |
| `Character.EnterVehicle.SitsAtDriverSeat` | After FinishEnterVehicle, character world loc ≈ `GetSeatWorldLocation("Driver")` |

### New Integration Tests

| Test | Confirms |
|------|----------|
| `Integration.FullVehicleCycle` | Enter → FinishEnter (Driving state) → Throttle+Tick (vehicle moved) → Exit → FinishExit (OnFoot) |
| `Integration.VehicleMovesOnThrottle` | Spawn vehicle at origin, throttle + tick → actor X position > 0 |
| `Integration.VehicleStopsOnNoInput` | Throttle to speed, then no input for N ticks → speed < 1 |
| `Regression.ProximityStillWorks` | `FindNearbyVehicle` returns correct vehicle after VehicleBase refactor |

### Manual PIE Only (not unit-testable)
- Entering/exiting animations feel correct and timed right
- Tires visually spinning at correct rate
- Siren light flashing red/blue (Blueprint Timeline)
- Speed, turn, friction tuning feel good for cartoon physics

---

## Build Command

```bash
/apps/git/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh \
  DudeWalksEditor Linux Development \
  -Project=/apps/git/unreal_experiments/DudeWalks.uproject
```

---

## Future Vehicles

Same pattern for firetruck, bus, ambulance:
1. New Blender model → organize into Body/Wheels/SpecialPart collections → export FBXes
2. Import to `/Game/Vehicles/{VehicleName}/`
3. Create `BP_{VehicleName} : VehicleBase`
4. Assign meshes, position seat markers in viewport
5. Override `ToggleLights` for vehicle-specific effect (fire truck light bar, ambulance strobe, etc.)
6. Bus with many seats: extend seat architecture when we get there
