# unreal_experiments / 01 DudeWalks

Isolated UE 5.5 proof-of-concept: WASD locomotion with idle, walk, and run animations driven entirely from C++.

**Goal:** Prove the UE5 locomotion pipeline in a minimal project with no interference from other systems (created because OpenCity had unexplained input/animation conflicts that were hard to isolate there).

---

## Controls

| Input | Result |
|---|---|
| WASD | Walk |
| Shift + WASD | Run |
| Standing still | Idle |
| Mouse | Look / camera |

---

## How to run

1. Open project in UE 5.5: `UnrealEditor /apps/git/unreal_experiments/DudeWalks.uproject`
2. Hit Play In Editor — character spawns in EmptyLevel
3. WASD to move, Shift to run

**Build C++ (hot reload, ~4–7s):**
```
make DudeWalksEditor
# then in UE Output Log:
HotReload DudeWalks
```

---

## Architecture

### C++ (`Source/DudeWalks/Character/`)

**DudeWalksCharacter** — `ACharacter` subclass. Enhanced Input handles WASD + Shift sprint. Uses `GetMesh()` (the inherited slot), not a custom `USkeletalMeshComponent` — this matters because `TryGetPawnOwner()` in the AnimInstance only resolves when the mesh is the inherited slot.

**DudeWalksAnimInstance** — `UAnimInstance` subclass. `NativeUpdateAnimation` reads `CharacterMovement->Velocity` each tick and sets:

| Property | Type | Value |
|---|---|---|
| `Speed` | float | Normalized 0–100 (unused by AnimGraph, kept for reference) |
| `bIsMoving` | bool | `RawSpeed > 10 cm/s` |
| `bIsSprinting` | bool | `MaxWalkSpeed >= RunSpeed` |
| `MoveAlpha` | float | `0.0` = idle, `1.0` = moving |
| `SprintAlpha` | float | `0.0` = walk, `1.0` = sprint |

### AnimGraph (`ABP_DudeWalks`)

Nested `TwoWayBlend` nodes — both in float-alpha mode (`bAlphaBoolEnabled = false`):

```
SP_Idle (A) ───────────────────────────────── TwoWayBlend_0 ──► OutputPose
                                                    ▲
                                               MoveAlpha (0/1)
SP_Walk (A) ──┐
              TwoWayBlend_1 ──────────── (B)
SP_Run  (B) ──┘     ▲
               SprintAlpha (0/1)
```

---

## Key findings

### BlendSpacePlayer does not work via MCP-created AnimGraph

`AnimGraphNode_BlendSpacePlayer` with a correctly referenced `BlendSpace1D` and correct Speed input (confirmed 53.3 at runtime) always evaluated at the idle sample — the blend never transitioned regardless of the X input value. Root cause is unknown (suspected: compiled AnimGraph bytecode doesn't bake the external BlendSpace reference when the node is created programmatically). **Workaround: use explicit `TwoWayBlend` + `SequencePlayer` nodes instead.**

### AnimGraph variable access requires C++ floats, not AnimGraph math nodes

Computing derived values (e.g. `Speed / 100`, bool-to-float) inside the AnimGraph via function call nodes is painful via MCP — node creation, function resolution, and pin connections are all fragile. **Solution: compute everything in `NativeUpdateAnimation` and expose as `UPROPERTY BlueprintReadOnly` floats. VariableGet nodes in the AnimGraph then just work.**

### `TwoWayBlend.bAlphaBoolEnabled` is a mode switch, not a condition

Connecting a bool variable to the `bAlphaBoolEnabled` pin does not drive the blend. The pin switches between float-alpha mode and an internal bool-blend system whose actual condition is never exposed as a pin. **Always use float-alpha mode (`bAlphaBoolEnabled = false`) and drive `Alpha` with a 0.0/1.0 float from C++.**

### MCP `connect_pins` requires `fromPinName`/`toPinName`

The C++ handler reads `fromPinName` and `toPinName`. Using `fromPin`/`toPin` (also listed in the schema) silently passes through but is never read — the pin appears connected in graph inspection but the compiled bytecode has a null wire. This was the cause of multiple wasted debugging sessions.

### SequencePlayer/BlendSpacePlayer asset refs must be set via Python after node creation

`create_node` with `properties: {"node.sequence": "..."}` does not set the sequence. Must follow up with:
```python
node.modify()
node.get_editor_property('node').set_editor_property('sequence', asset)
```

### MCP requires editor running before Claude Code session starts

The MCP server (`Unreal_mcp`) connects to the editor's in-process McpAutomationBridge on startup. If the editor isn't running when the session starts, the tool list is empty and cached empty for the session. Restart with `claude --resume <id>` after launching the editor.

### FBX import requires Interchange disabled

UE 5.5 routes FBX through the Interchange framework by default, which crashes with a `TaskGraph RecursionGuard` signal 11 when called from MCP threads. Launch the editor with:
```
-ExecCmds="Interchange.FeatureFlags.Import.FBX 0"
```

---

## Plugin: McpAutomationBridge

`Plugins/McpAutomationBridge` is an **unmodified copy** of `/apps/git/Unreal_mcp/plugins/McpAutomationBridge`. It is the in-process MCP server that listens on port 8091 and bridges Claude Code's MCP tool calls into the editor's Python/C++ automation APIs.

It is committed here so the project is self-contained. To update it, replace the folder with a fresh copy from the upstream repo.
