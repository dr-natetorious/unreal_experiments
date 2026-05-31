"""
DudeWalks content bootstrap — run once from the editor Output Log.

Switch the Cmd dropdown to Python, then paste:
    exec(open('/apps/git/unreal_experiments/Tools/setup_dudewalls.py').read())

Creates:
  /Game/Maps/EmptyLevel         — floor, sun, sky, player start
  /Game/Input/                  — IA_Move, IA_Look, IA_Sprint, IMC_DudeWalks
  /Game/Props/                  — SK_CityCharacter (imported from FBX)
  /Game/Animations/             — AS_Idle, AS_Walk, AS_Run, BS_Locomotion, ABP_DudeWalks
  /Game/Characters/             — BP_DudeWalks, GM_DudeWalks
"""

import unreal, os

at  = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary
lvl = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
aas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

FBX_ROOT = '/apps/git/unreal_experiments/Content'

# ── helpers ────────────────────────────────────────────────────────────────────

def ensure_dir(path):
    if not eal.does_directory_exist(path):
        eal.make_directory(path)

def asset_exists(pkg):
    return eal.does_asset_exist(pkg)

def load_or_none(pkg):
    return unreal.load_asset(pkg) if asset_exists(pkg) else None

def make_key(name):
    k = unreal.Key()
    k.set_editor_property('key_name', name)
    return k

def make_mapping(imc, action, key_name, mods=None):
    m = unreal.EnhancedActionKeyMapping()
    m.set_editor_property('action', action)
    m.set_editor_property('key', make_key(key_name))
    if mods:
        m.set_editor_property('modifiers', mods)
    return m

def negate(outer):
    return unreal.new_object(unreal.InputModifierNegate, outer)

def swizzle_yxz(outer):
    m = unreal.new_object(unreal.InputModifierSwizzleAxis, outer)
    m.set_editor_property('order', unreal.InputAxisSwizzle.YXZ)
    return m

def import_fbx_mesh(src_path, dest_pkg, dest_name, is_skeletal=True):
    if asset_exists(f'{dest_pkg}/{dest_name}'):
        print(f'[import] {dest_name}: already exists — skipped')
        return unreal.load_asset(f'{dest_pkg}/{dest_name}')

    task = unreal.AssetImportTask()
    task.set_editor_property('filename', src_path)
    task.set_editor_property('destination_path', dest_pkg)
    task.set_editor_property('destination_name', dest_name)
    task.set_editor_property('replace_existing', True)
    task.set_editor_property('automated', True)
    task.set_editor_property('save', True)

    opts = unreal.FbxImportUI()
    opts.set_editor_property('import_mesh', is_skeletal)
    opts.set_editor_property('import_as_skeletal', is_skeletal)
    opts.set_editor_property('import_animations', False)
    opts.set_editor_property('import_textures', False)
    opts.set_editor_property('import_materials', False)
    opts.set_editor_property('create_physics_asset', False)
    task.set_editor_property('options', opts)

    at.import_asset_tasks([task])
    result = unreal.load_asset(f'{dest_pkg}/{dest_name}')
    if result:
        print(f'[import] {dest_name}: OK')
    else:
        print(f'[import] {dest_name}: FAILED — check the FBX path: {src_path}')
    return result

def import_fbx_anim(src_path, dest_pkg, dest_name, skeleton):
    if asset_exists(f'{dest_pkg}/{dest_name}'):
        print(f'[import] {dest_name}: already exists — skipped')
        return unreal.load_asset(f'{dest_pkg}/{dest_name}')

    task = unreal.AssetImportTask()
    task.set_editor_property('filename', src_path)
    task.set_editor_property('destination_path', dest_pkg)
    task.set_editor_property('destination_name', dest_name)
    task.set_editor_property('replace_existing', True)
    task.set_editor_property('automated', True)
    task.set_editor_property('save', True)

    opts = unreal.FbxImportUI()
    opts.set_editor_property('import_mesh', False)
    opts.set_editor_property('import_as_skeletal', False)
    opts.set_editor_property('import_animations', True)
    opts.set_editor_property('import_textures', False)
    opts.set_editor_property('import_materials', False)
    opts.set_editor_property('skeleton', skeleton)
    task.set_editor_property('options', opts)

    at.import_asset_tasks([task])
    result = unreal.load_asset(f'{dest_pkg}/{dest_name}')
    if result:
        print(f'[import] {dest_name}: OK')
    else:
        print(f'[import] {dest_name}: FAILED — check {src_path}')
    return result

# ── Step 1: Level setup ────────────────────────────────────────────────────────

print('\n=== Step 1: Level lighting + floor ===')

ensure_dir('/Game/Maps')

actors     = aas.get_all_level_actors()
cls_names  = {a.get_class().get_name() for a in actors}

def has_actor(cls_name):
    return cls_name in cls_names

def spawn_if_missing(cls_path, label, loc=unreal.Vector(0,0,0)):
    cls = unreal.load_class(None, cls_path)
    if not cls:
        print(f'[level] WARNING: cannot load {cls_path}')
        return None
    a = aas.spawn_actor_from_class(cls, loc)
    if a:
        a.set_actor_label(label)
        print(f'[level] Spawned {label}')
    return a

if not has_actor('DirectionalLight'):
    sun = spawn_if_missing('/Script/Engine.DirectionalLight', 'SunLight', unreal.Vector(0,0,300))
    if sun:
        sun.set_actor_rotation(unreal.Rotator(-45, -60, 0), False)
        comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
        if comp:
            comp.set_editor_property('intensity', 10.0)
            comp.set_editor_property('atmosphere_sun_light', True)
else:
    print('[level] DirectionalLight: already present')

if not has_actor('SkyAtmosphere'):
    spawn_if_missing('/Script/Engine.SkyAtmosphere', 'SkyAtmosphere')
else:
    print('[level] SkyAtmosphere: already present')

if not has_actor('SkyLight'):
    sky = spawn_if_missing('/Script/Engine.SkyLight', 'SkyLight')
    if sky:
        comp = sky.get_component_by_class(unreal.SkyLightComponent)
        if comp:
            comp.set_editor_property('real_time_capture', True)
else:
    print('[level] SkyLight: already present')

if not has_actor('ExponentialHeightFog'):
    spawn_if_missing('/Script/Engine.ExponentialHeightFog', 'HeightFog', unreal.Vector(0,0,100))
else:
    print('[level] ExponentialHeightFog: already present')

if not any(a.get_actor_label() == 'TestFloor' for a in actors):
    floor_mesh = unreal.load_object(None, '/Engine/BasicShapes/Plane.Plane')
    floor_cls  = unreal.load_class(None, '/Script/Engine.StaticMeshActor')
    if floor_cls and floor_mesh:
        floor = aas.spawn_actor_from_class(floor_cls, unreal.Vector(0,0,0))
        if floor:
            floor.set_actor_label('TestFloor')
            floor.set_actor_scale3d(unreal.Vector(100, 100, 1))
            mesh_comp = floor.get_component_by_class(unreal.StaticMeshComponent)
            if mesh_comp:
                mesh_comp.set_static_mesh(floor_mesh)
            print('[level] TestFloor: 100x100m plane')
else:
    print('[level] TestFloor: already present')

ps_spawned = False
for actor in aas.get_all_level_actors():
    if actor.get_class().get_name() == 'PlayerStart':
        actor.set_actor_location(unreal.Vector(0, 0, 100), False, False)
        print('[level] PlayerStart: moved to (0,0,100)')
        ps_spawned = True
        break
if not ps_spawned:
    ps_cls = unreal.load_class(None, '/Script/Engine.PlayerStart')
    if ps_cls:
        ps = aas.spawn_actor_from_class(ps_cls, unreal.Vector(0, 0, 100))
        if ps:
            ps.set_actor_label('PlayerStart')
            print('[level] PlayerStart: spawned at (0,0,100)')

lvl.save_current_level()
print('[level] Saved.')

# ── Step 2: Input ──────────────────────────────────────────────────────────────

print('\n=== Step 2: Input actions + mapping context ===')

ensure_dir('/Game/Input')

def get_or_create_action(name, value_type):
    pkg = f'/Game/Input/{name}'
    if asset_exists(pkg):
        return unreal.load_asset(pkg)
    a = at.create_asset(name, '/Game/Input', unreal.InputAction, unreal.InputAction_Factory())
    a.set_editor_property('value_type', value_type)
    eal.save_asset(pkg)
    print(f'[input] Created {name}')
    return a

ia_move   = get_or_create_action('IA_Move',   unreal.InputActionValueType.AXIS2D)
ia_look   = get_or_create_action('IA_Look',   unreal.InputActionValueType.AXIS2D)
ia_sprint = get_or_create_action('IA_Sprint', unreal.InputActionValueType.BOOLEAN)

imc_pkg = '/Game/Input/IMC_DudeWalks'
if asset_exists(imc_pkg):
    imc = unreal.load_asset(imc_pkg)
    print('[input] IMC_DudeWalks: already exists')
else:
    imc = at.create_asset('IMC_DudeWalks', '/Game/Input',
                          unreal.InputMappingContext, unreal.InputMappingContext_Factory())
    imc.set_editor_property('mappings', [
        # Move: WASD — Axis2D (X=right, Y=forward)
        make_mapping(imc, ia_move, 'W',    [swizzle_yxz(imc)]),
        make_mapping(imc, ia_move, 'S',    [negate(imc), swizzle_yxz(imc)]),
        make_mapping(imc, ia_move, 'D'),
        make_mapping(imc, ia_move, 'A',    [negate(imc)]),
        # Look: mouse
        make_mapping(imc, ia_look, 'Mouse2D'),
        # Sprint: shift
        make_mapping(imc, ia_sprint, 'LeftShift'),
    ])
    eal.save_asset(imc_pkg)
    print('[input] IMC_DudeWalks: WASD move, Mouse2D look, LShift sprint')

# ── Step 3: Import character mesh ─────────────────────────────────────────────

print('\n=== Step 3: Import skeletal mesh ===')

ensure_dir('/Game/Props')
sk_mesh = import_fbx_mesh(
    f'{FBX_ROOT}/Props/SK_CityCharacter.fbx',
    '/Game/Props', 'SK_CityCharacter',
    is_skeletal=True
)

skeleton = None
if sk_mesh:
    skeleton = sk_mesh.get_editor_property('skeleton') if hasattr(sk_mesh, 'get_editor_property') else None
    if not skeleton:
        skel_pkg = '/Game/Props/SK_CityCharacter_Skeleton'
        if asset_exists(skel_pkg):
            skeleton = unreal.load_asset(skel_pkg)
    print(f'[mesh] Skeleton: {skeleton.get_name() if skeleton else "NOT FOUND — animation import will be skipped"}')

# ── Step 4: Import animations ─────────────────────────────────────────────────

print('\n=== Step 4: Import animations ===')

ensure_dir('/Game/Animations')

as_idle = as_walk = as_run = None
if skeleton:
    as_idle = import_fbx_anim(f'{FBX_ROOT}/Animations/Source/AS_Idle.fbx', '/Game/Animations', 'AS_Idle', skeleton)
    as_walk = import_fbx_anim(f'{FBX_ROOT}/Animations/Source/AS_Walk.fbx', '/Game/Animations', 'AS_Walk', skeleton)
    as_run  = import_fbx_anim(f'{FBX_ROOT}/Animations/Source/AS_Run.fbx',  '/Game/Animations', 'AS_Run',  skeleton)
else:
    print('[anim] Skeleton not found — skipping animation import.')

# ── Step 5: BlendSpace1D ───────────────────────────────────────────────────────

print('\n=== Step 5: Blend space ===')

bs_pkg = '/Game/Animations/BS_Locomotion'
if asset_exists(bs_pkg):
    print('[blend] BS_Locomotion: already exists')
else:
    if skeleton:
        factory = unreal.BlendSpaceFactory1D()
        factory.set_editor_property('target_skeleton', skeleton)
        bs = at.create_asset('BS_Locomotion', '/Game/Animations', None, factory)
        if bs:
            # Configure axis: Speed 0→375
            axis = unreal.BlendParameter()
            axis.set_editor_property('display_name', 'Speed')
            axis.set_editor_property('min', 0.0)
            axis.set_editor_property('max', 375.0)
            axis.set_editor_property('grid_num', 4)
            bs.set_editor_property('blend_parameters', axis)

            # Add sample points
            samples = []
            for anim, val in [(as_idle, 0.0), (as_walk, 200.0), (as_run, 375.0)]:
                if anim:
                    s = unreal.BlendSample()
                    s.set_editor_property('animation', anim)
                    s.set_editor_property('sample_value', val)
                    samples.append(s)
            if samples:
                bs.set_editor_property('sample_data', samples)

            eal.save_asset(bs_pkg)
            print('[blend] BS_Locomotion: Speed axis 0-375 with Idle/Walk/Run samples')
        else:
            print('[blend] BS_Locomotion: creation failed')
    else:
        print('[blend] Skipping — no skeleton')

# ── Step 6: AnimBP ────────────────────────────────────────────────────────────

print('\n=== Step 6: Animation Blueprint ===')

ensure_dir('/Game/Animations')
abp_pkg = '/Game/Animations/ABP_DudeWalks'
if asset_exists(abp_pkg):
    print('[abp] ABP_DudeWalks: already exists')
else:
    if skeleton:
        parent_anim_class = unreal.load_class(None, '/Script/DudeWalks.DudeWalksAnimInstance')
        factory = unreal.AnimBlueprintFactory()
        factory.set_editor_property('target_skeleton', skeleton)
        if parent_anim_class:
            factory.set_editor_property('parent_class', parent_anim_class)
            print('[abp] Parent class: UDudeWalksAnimInstance')
        else:
            print('[abp] WARNING: UDudeWalksAnimInstance not found — using default UAnimInstance parent')
        abp = at.create_asset('ABP_DudeWalks', '/Game/Animations', None, factory)
        if abp:
            eal.save_asset(abp_pkg)
            print('[abp] ABP_DudeWalks: created')
            print('[abp] TODO: Open ABP_DudeWalks → AnimGraph → add BlendSpace Player node')
            print('[abp]       Set BlendSpace = BS_Locomotion, Speed pin = Speed variable')
            print('[abp]       Connect to Output Pose. Compile & Save.')
        else:
            print('[abp] Creation failed')
    else:
        print('[abp] Skipping — no skeleton')

# ── Step 7: Character Blueprint ───────────────────────────────────────────────

print('\n=== Step 7: Character Blueprint ===')

ensure_dir('/Game/Characters')
bp_pkg = '/Game/Characters/BP_DudeWalks'
if asset_exists(bp_pkg):
    print('[bp] BP_DudeWalks: already exists')
else:
    char_class = unreal.load_class(None, '/Script/DudeWalks.DudeWalksCharacter')
    if char_class:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property('parent_class', char_class)
        bp = at.create_asset('BP_DudeWalks', '/Game/Characters', None, factory)
        if bp:
            eal.save_asset(bp_pkg)
            print('[bp] BP_DudeWalks: created with ADudeWalksCharacter parent')
            print('[bp] TODO: Open BP_DudeWalks → Components → Mesh:')
            print('[bp]   Skeletal Mesh = SK_CityCharacter')
            print('[bp]   Anim Class    = ABP_DudeWalks')
            print('[bp] TODO: Class Defaults:')
            print('[bp]   Default Mapping Context = IMC_DudeWalks')
            print('[bp]   Move Action             = IA_Move')
            print('[bp]   Look Action             = IA_Look')
            print('[bp]   Sprint Action           = IA_Sprint')
        else:
            print('[bp] Creation failed')
    else:
        print('[bp] WARNING: ADudeWalksCharacter not found — is DudeWalks module compiled?')

# ── Step 8: GameMode ──────────────────────────────────────────────────────────

print('\n=== Step 8: GameMode ===')

gm_pkg = '/Game/Characters/GM_DudeWalks'
if asset_exists(gm_pkg):
    print('[gm] GM_DudeWalks: already exists')
else:
    gm_base = unreal.load_class(None, '/Script/Engine.GameModeBase')
    if gm_base:
        factory = unreal.BlueprintFactory()
        factory.set_editor_property('parent_class', gm_base)
        gm = at.create_asset('GM_DudeWalks', '/Game/Characters', None, factory)
        if gm and asset_exists(bp_pkg):
            bp_class = unreal.load_class(None, f'{bp_pkg}.BP_DudeWalks_C')
            if bp_class:
                gm_cdo = unreal.get_default_object(gm.generated_class())
                if gm_cdo:
                    gm_cdo.set_editor_property('default_pawn_class', bp_class)
            eal.save_asset(gm_pkg)
            print('[gm] GM_DudeWalks: created')
        elif gm:
            eal.save_asset(gm_pkg)
            print('[gm] GM_DudeWalks: created (set DefaultPawnClass manually after BP_DudeWalks is ready)')

# ── Step 9: Wire GameMode into world settings ─────────────────────────────────

print('\n=== Step 9: World settings ===')

gm_class = unreal.load_class(None, f'{gm_pkg}.GM_DudeWalks_C')
if gm_class:
    ws = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_level_viewport_world()
    if ws:
        ws.world_settings.set_editor_property('default_game_mode', gm_class)
        lvl.save_current_level()
        print('[world] DefaultGameMode = GM_DudeWalks')
    else:
        print('[world] Could not get world — set DefaultGameMode manually in World Settings')
else:
    print('[world] GM_DudeWalks_C not compiled yet — set DefaultGameMode manually after first compile')

# ── Done ──────────────────────────────────────────────────────────────────────

print("""
=== Setup complete ===

Automated:
  ✓ Level: sun, sky, fog, 100m floor, PlayerStart
  ✓ Input: IA_Move (Axis2D), IA_Look (Axis2D), IA_Sprint (bool), IMC_DudeWalks (WASD+Shift)
  ✓ Mesh:  SK_CityCharacter imported
  ✓ Anims: AS_Idle, AS_Walk, AS_Run imported
  ✓ Blend: BS_Locomotion (Speed 0→375)
  ✓ ABP:   ABP_DudeWalks created (parent: UDudeWalksAnimInstance)
  ✓ BP:    BP_DudeWalks created (parent: ADudeWalksCharacter)
  ✓ GM:    GM_DudeWalks

Manual steps remaining:
  1. Open ABP_DudeWalks
     - AnimGraph: Add "Blend Space Player" node → set BS_Locomotion
     - Promote Speed pin to variable (it comes from C++ parent automatically)
     - Connect to Output Pose → Compile & Save

  2. Open BP_DudeWalks → Components → Mesh
     - Skeletal Mesh = SK_CityCharacter
     - Anim Class    = ABP_DudeWalks_C
     → Class Defaults:
     - Default Mapping Context = IMC_DudeWalks
     - Move Action             = IA_Move
     - Look Action             = IA_Look
     - Sprint Action           = IA_Sprint
     → Compile & Save

  3. Press Play — WASD moves, Shift sprints, idle at rest.
""")
