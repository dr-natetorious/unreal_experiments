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
