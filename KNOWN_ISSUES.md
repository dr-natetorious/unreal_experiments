# Known Issues

## AnimBP PoseLink pins cannot be connected via MCP or Python

**Symptom:** `manage_blueprint connect_pins` returns `PIN_NOT_FOUND` for any AnimBP Pose/Result pin pair, even when pin names are confirmed correct via `get_pin_details`. Python's `AnimGraphNode` objects do not expose `pins` or `LinkedTo` arrays.

**Root cause:** UE5's Python scripting layer does not expose the `UEdGraphPin::LinkedTo` array for AnimGraph nodes. PoseLink pins (type `struct/PoseLink`) are a special AnimBP-only pin type that the Blueprint graph API doesn't handle.

**Impact:** All AnimBP pose wiring (state machine output → Output Pose, UseCachedPose → StateResult, SequencePlayer → blend node) must be done manually in the editor by dragging wires. Node creation, deletion, and data pin connections (Alpha, Bool, Float) work fine via MCP.

**Workaround:** Drag animation assets from Content Browser directly into a state machine state — UE5 auto-creates the sequence player AND auto-connects it to Output Animation Pose, bypassing the need to wire manually.

---

## BSP PhysicsVolume collision requires center above water surface

**Symptom:** Moving a PhysicsVolume (bWaterVolume=True) so it sits flush with or below the pool floor (e.g. Z=-270) breaks swim detection even after running `RebuildGeometry`. Swimming does not trigger when character enters the water.

**Root cause:** BSP brush collision geometry is unreliable when the volume is positioned/scaled via Python or moved in-editor without a full geometry rebuild at that exact position. Even after `RebuildGeometry`, the collision surface at the volume's top edge is not reliably generated when the top is near Z=0 (the water surface). Native PhysicsVolume overlap events never fire.

**Workaround:** Keep the WaterVolume center at Z=-100 (or higher) with Z scale=3, so the top of the volume sits at Z=+200 — well above the water surface. The character's capsule overlaps the volume reliably on entry. XY bounds are what control the pool footprint; Z only needs to extend above the entry point.

**Working config:** Location=(1500, 0, -100), Scale=(5, 3, 3) → X:1000-2000, Y:-300-300, Z:-400-+200.

**Proper fix (not yet implemented):** C++ Tick-based detection using `GetComponentsBoundingBox()` against feet position bypasses BSP entirely. See `DudeWalksCharacter.cpp` — Tick override was in git stash but was not restored cleanly.

---

## BP_PoliceCar siren toggle not yet wired in Blueprint graph

**Symptom:** Calling `ToggleLights()` on BP_PoliceCar does nothing — the base C++ implementation is a no-op and the Blueprint override event graph hasn't been wired.

**What exists:** `ToggleLights` is a `BlueprintNativeEvent` on VehicleBase. BP_PoliceCar has `SirenMesh` (PoliceCar_PoliceLightBar) and `SirenLight` (PointLight, hidden by default, Intensity=5000) added as SCS components. A key binding and character handler have NOT been added yet.

**To complete:** Open BP_PoliceCar in the Blueprint editor, add an event override for `ToggleLights`, wire it to toggle `SirenLight` visibility and start a Timeline that alternates the light color between red and blue at ~0.15s intervals. Separately, add `IA_Siren` input action + IMC_Vehicle mapping, and a handler in DudeWalksCharacter that calls `CurrentVehicle->ToggleLights()` when driving.

---

## FBX import from Python crashes editor when Interchange is enabled

**Symptom:** Calling `import_asset_tasks()` from Python crashes the editor and drops MCP connection.

**Root cause:** UE5's Interchange import pipeline intercepts FBX imports and crashes when triggered from Python's `FbxImportUI` path.

**Fix:** Call this before any Python FBX import:
```python
unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX 0")
```

**Full workflow (including scale/axis gotchas):** See `.docs/BlenderToUEWorkflow.md`.

---

## import_uniform_scale is silently ignored in automated Python FBX import

**Symptom:** Setting `opts.static_mesh_import_data.import_uniform_scale = 100.0` has no effect — mesh imports at wrong scale.

**Root cause:** The `import_uniform_scale` property exists on `FbxStaticMeshImportData` but is not applied when `task.automated = True`.

**Fix:** Export FBX from Blender with correct scale baked in: use `global_scale=1.0, apply_unit_scale=False, apply_scale_options='FBX_SCALE_NONE'`. Blender's FBX exporter implicitly converts meters→cm (the FBX format's native unit), which UE reads correctly at 1:1. See `.docs/BlenderToUEWorkflow.md`.
