# Blender → Unreal Engine 5 FBX Workflow

Reference for all vehicle (and general mesh) export/import to avoid re-discovering edge cases.

---

## Scale

**Use `global_scale=1.0`, `apply_unit_scale=False`, `apply_scale_options='FBX_SCALE_NONE'`.**

FBX format natively uses centimeters. Blender's FBX exporter automatically converts
Blender's meter positions to FBX cm. UE imports FBX cm values at 1:1 as UE units (cm).
No manual scale factor needed.

Traps to avoid:
- `apply_scale_options='FBX_SCALE_ALL'` with `apply_unit_scale=True` bakes a
  UnitScaleFactor=100 into the FBX header. UE then *divides* vertex positions by 100,
  making the mesh 100× too small.
- `global_scale=100` scales vertices 100× in Blender; FBX then converts those m→cm
  (another ×100), making the mesh 10,000× too large.
- `static_mesh_import_data.import_uniform_scale = 100.0` in Python **does not apply**
  during automated import — the property exists but is silently ignored.

**Correct export call:**
```python
bpy.ops.export_scene.fbx(
    ...
    global_scale=1.0,
    apply_unit_scale=False,
    apply_scale_options='FBX_SCALE_NONE',
    ...
)
```

---

## Axis Orientation

UE: X=forward, Y=right, Z=up. This project's car faces Blender +X.

Export settings:
```python
axis_forward='-Z',
axis_up='Y',
```

**Empirical result** (verified with PoliceCar_Body import): UE's FBX importer reads
Blender vertex coordinates directly — Blender X→UE X, Y→UE Y, Z→UE Z. The axis_forward/
axis_up hints affect object transforms but not raw vertex positions. The car whose front
faces Blender +X will face UE +X (forward) — no Blueprint rotation correction needed.

Coordinate conversion (Blender meters → UE cm) is just ×100 on all axes:
```
Blender (bx, by, bz) meters  →  UE (bx×100, by×100, bz×100) cm
```

Example — police car tire axle positions in UE (from Blender measurements):
```
Wheel FL:  UE ( 132,  99.5, 38) cm
Wheel FR:  UE ( 132, -104.5, 38) cm
Wheel RL:  UE (-132,  99.5, 38) cm
Wheel RR:  UE (-132, -104.5, 38) cm
Siren:     UE (  30,   0,  167) cm
SeatDriver:     UE (50, -26, 46) cm
SeatPassenger:  UE (50,  26, 46) cm
SeatRearLeft:   UE (-52, -26, 46) cm
SeatRearRight:  UE (-52,  26, 46) cm
```

---

## Python Import — Disable Interchange First

UE5's Interchange import pipeline crashes when called from Python.
**Always disable it before importing any FBX from Python:**

```python
import unreal
unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX 0")
```

Full import task setup:

```python
import unreal

unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX 0")

task = unreal.AssetImportTask()
task.filename = '/path/to/Mesh.fbx'
task.destination_path = '/Game/TargetFolder'
task.replace_existing = True
task.automated = True
task.save = True

opts = unreal.FbxImportUI()
opts.import_mesh = True
opts.import_as_skeletal = False
opts.import_animations = False
opts.import_materials = True
opts.import_textures = True
opts.static_mesh_import_data.combine_meshes = True        # merge multi-part FBX
opts.static_mesh_import_data.generate_lightmap_u_vs = True
opts.static_mesh_import_data.auto_generate_collision = True  # False for non-body meshes
task.options = opts

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
print(task.imported_object_paths)
```

---

## Multi-Part Models (e.g., 116-part car)

Split into logical FBX files by Blender Collection, exported separately:
- **Body** — all body/interior parts → `combine_meshes=True`, `auto_collision=True`
- **Wheel** — one wheel (FL parts), origin set to axle center → used for all 4 tire components
- **SpecialPart** — e.g., PoliceLightBar → `combine_meshes=True`, `auto_collision=False`

Why one shared wheel mesh: UE Blueprint uses 4 UStaticMeshComponent instances of
the same asset, each at the correct axle offset. Spinning works because the mesh
origin is at the axle center.

```python
# Set wheel origin to axle center before export:
bpy.ops.object.select_all(action='DESELECT')
for name in ['Tyre_FL', 'Whitewall_FL', 'Hubcap_FL', 'Hubcap_FL_Dome']:
    bpy.data.objects[name].select_set(True)
bpy.context.view_layer.objects.active = bpy.data.objects['Tyre_FL']
bpy.ops.object.duplicate()
bpy.ops.object.join()
bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='BOUNDS')
bpy.ops.object.location_clear(clear_delta=False)
# ... export selected ... then bpy.ops.object.delete()
```

---

## Collection-Based Export

Flat collection structure (siblings, NOT nested) is simplest:
```
Scene Collection
├── Body
├── Wheels
└── PoliceLightBar   ← already existed
```

Export one collection at a time:
```python
def find_layer_col(parent, name):
    for lc in parent.children:
        if lc.name == name: return lc
        found = find_layer_col(lc, name)
        if found: return found
    return None

lc = find_layer_col(bpy.context.view_layer.layer_collection, 'Body')
bpy.context.view_layer.active_layer_collection = lc

bpy.ops.export_scene.fbx(
    filepath='/path/to/output.fbx',
    use_active_collection=True,
    global_scale=100.0,
    apply_unit_scale=False,
    apply_scale_options='FBX_SCALE_NONE',
    axis_forward='-Z',
    axis_up='Y',
    use_mesh_modifiers=True,
    mesh_smooth_type='FACE',
    object_types={'MESH'},
)
```

---

## Police Car Interior — Can 4 Characters Fit?

Measured from Blender (meters), converted to cm for UE:

| Measurement | Value |
|-------------|-------|
| Floor pan top | Z = 31.5 cm |
| Roof bottom | Z = 155 cm |
| Interior headroom | **123.5 cm** (≈ 48.6 in) |
| Seat cushion top | Z = 54.2 cm |
| Head height seated (cushion + 90cm torso/head) | **144 cm** |
| Roof clearance seated | **~11 cm** ✓ (tight, OK with driving anim) |
| Each front/rear seat width | 40 cm |
| Gap between driver & passenger seats | 12 cm |
| Police cage divider height | 93–97 cm |

**Verdict: Yes, 4 characters fit.** Each seat is 40 cm wide; slight mesh clipping between
adjacent characters is acceptable at game distances. The driving animation keeps characters
slightly forward so roof clearance is fine.

Seat centers in Blender (X, Y, Z) meters:
```
Front Driver:    (0.50, -0.26, 0.46)
Front Passenger: (0.50,  0.26, 0.46)
Rear Driver:     (-0.52, -0.26, 0.46)
Rear Passenger:  (-0.52,  0.26, 0.46)
```

In UE cm (using conversion above: ux=-bz×100, uy=bx×100, uz=by×100):
```
Front Driver:    UE (-46, 50, -26)   ← relative to body mesh origin
Front Passenger: UE (-46, 50,  26)
Rear Driver:     UE (-46, -52, -26)
Rear Passenger:  UE (-46, -52,  26)
```
*Note: UE Z is up, so seat Z=-26 is wrong — see note below.*

> **Tuning note**: Seat Z positions will need visual adjustment in the Blueprint
> viewport since the axis mapping puts Blender Y (seat depth) into UE Z. Drag the
> SeatDriver/Passenger/RearLeft/RearRight scene components to sit inside the seat
> cushion geometry after the car body is imported.
