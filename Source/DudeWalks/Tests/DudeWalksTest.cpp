#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/DudeWalksCharacter.h"
#include "Character/DudeWalksAnimInstance.h"
#include "Vehicle/VehicleBase.h"
#include "DudeWalksTypes.h"

// Returns any available world (PIE > Game > Editor) — editor world is spawnable for unit tests.
static UWorld* GetTestWorld()
{
    if (!GEngine) return nullptr;
    UWorld* EditorWorld = nullptr;
    for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
    {
        if (!Ctx.World()) continue;
        if (Ctx.WorldType == EWorldType::PIE || Ctx.WorldType == EWorldType::Game)
            return Ctx.World();
        if (Ctx.WorldType == EWorldType::Editor)
            EditorWorld = Ctx.World();
    }
    return EditorWorld;
}

static ADudeWalksCharacter* SpawnDude(UWorld* W, FVector Loc = FVector::ZeroVector)
{
    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    return W->SpawnActor<ADudeWalksCharacter>(ADudeWalksCharacter::StaticClass(), Loc, FRotator::ZeroRotator, P);
}

static AVehicleBase* SpawnVehicle(UWorld* W, FVector Loc)
{
    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    return W->SpawnActor<AVehicleBase>(AVehicleBase::StaticClass(), Loc, FRotator::ZeroRotator, P);
}

// =============================================================================
// Existing tests
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksSpawn,
    "DudeWalks.CharacterSpawns",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksSpawn::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W);
    TestNotNull(TEXT("Character spawned"), Dude);
    if (Dude) Dude->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksMeshSetup,
    "DudeWalks.MeshUsesInheritedSlot",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksMeshSetup::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 500.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;

    USkeletalMeshComponent* Mesh = Dude->GetMesh();
    TestNotNull(TEXT("GetMesh() is non-null"), Mesh);
    TestTrue(TEXT("Mesh Z = -90"), FMath::IsNearlyEqual(Mesh->GetRelativeLocation().Z, -90.f));
    TestTrue(TEXT("Mesh Yaw = -90"), FMath::IsNearlyEqual(Mesh->GetRelativeRotation().Yaw, -90.f));
    TestTrue(TEXT("Mesh has no collision"), Mesh->GetCollisionEnabled() == ECollisionEnabled::NoCollision);

    Dude->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksSpeed,
    "DudeWalks.SpeedDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksSpeed::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 1000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;

    UCharacterMovementComponent* CMC = Dude->GetCharacterMovement();
    TestTrue(TEXT("Default MaxWalkSpeed is WalkSpeed"), FMath::IsNearlyEqual(CMC->MaxWalkSpeed, Dude->WalkSpeed));
    CMC->MaxWalkSpeed = Dude->RunSpeed;
    TestTrue(TEXT("MaxWalkSpeed is RunSpeed while sprinting"), FMath::IsNearlyEqual(CMC->MaxWalkSpeed, Dude->RunSpeed));
    CMC->MaxWalkSpeed = Dude->WalkSpeed;
    TestTrue(TEXT("MaxWalkSpeed returns to WalkSpeed after sprint"), FMath::IsNearlyEqual(CMC->MaxWalkSpeed, Dude->WalkSpeed));

    Dude->Destroy();
    return true;
}

// =============================================================================
// Vehicle — initial state
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleInitialState,
    "DudeWalks.Vehicle.InitialState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleInitialState::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 2000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;

    TestTrue(TEXT("CharacterState starts as OnFoot"),
        Dude->CharacterState == ECharacterState::OnFoot);
    TestTrue(TEXT("bCanSwim enabled from constructor"),
        Dude->GetCharacterMovement()->NavAgentProps.bCanSwim);

    Dude->Destroy();
    return true;
}

// =============================================================================
// Vehicle — proximity
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleProximityInRange,
    "DudeWalks.Vehicle.ProximityInRange",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleProximityInRange::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 3000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(200.f, 3000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) { Dude->Destroy(); return false; }

    AVehicleBase* Found = Dude->FindNearbyVehicle();
    TestEqual(TEXT("Vehicle within radius is found"), Found, V);

    Dude->Destroy(); V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleProximityOutOfRange,
    "DudeWalks.Vehicle.ProximityOutOfRange",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleProximityOutOfRange::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 4000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(400.f, 4000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) { Dude->Destroy(); return false; }

    AVehicleBase* Found = Dude->FindNearbyVehicle();
    TestNull(TEXT("Vehicle beyond radius not found"), Found);

    Dude->Destroy(); V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleProximityNoVehicles,
    "DudeWalks.Vehicle.ProximityNoVehicles",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleProximityNoVehicles::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 5000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;

    TestNull(TEXT("No vehicle in world returns null"), Dude->FindNearbyVehicle());

    Dude->Destroy();
    return true;
}

// =============================================================================
// Vehicle — enter flow
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleEnterSetsState,
    "DudeWalks.Vehicle.EnterSetsState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleEnterSetsState::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 6000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(100.f, 6000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) { Dude->Destroy(); return false; }

    Dude->EnterVehicle(V);
    TestTrue(TEXT("State = EnteringVehicle after EnterVehicle()"),
        Dude->CharacterState == ECharacterState::EnteringVehicle);
    TestTrue(TEXT("Movement disabled while entering"),
        Dude->GetCharacterMovement()->MovementMode == MOVE_None);

    Dude->Destroy(); V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleEnterAttaches,
    "DudeWalks.Vehicle.EnterAttachesToVehicle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleEnterAttaches::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 7000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(100.f, 7000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) { Dude->Destroy(); return false; }

    Dude->EnterVehicle(V);
    TestEqual(TEXT("Character attached to vehicle"), Dude->GetAttachParentActor(), (AActor*)V);

    Dude->Destroy(); V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleEnterFinish,
    "DudeWalks.Vehicle.EnterFinishTransitionsToDriving",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleEnterFinish::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 8000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(100.f, 8000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) { Dude->Destroy(); return false; }

    Dude->EnterVehicle(V);
    Dude->FinishEnterVehicle();
    TestTrue(TEXT("State = Driving after FinishEnterVehicle()"),
        Dude->CharacterState == ECharacterState::Driving);

    Dude->Destroy(); V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleEnterGuard,
    "DudeWalks.Vehicle.EnterGuardNotOnFoot",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleEnterGuard::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 9000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(100.f, 9000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) { Dude->Destroy(); return false; }

    Dude->EnterVehicle(V);
    TestTrue(TEXT("First enter: EnteringVehicle"), Dude->CharacterState == ECharacterState::EnteringVehicle);

    AVehicleBase* V2 = SpawnVehicle(W, FVector(100.f, 9100.f, 0.f));
    Dude->EnterVehicle(V2);
    TestTrue(TEXT("Re-enter guard: state unchanged"), Dude->CharacterState == ECharacterState::EnteringVehicle);

    Dude->Destroy(); V->Destroy(); if (V2) V2->Destroy();
    return true;
}

// =============================================================================
// Vehicle — honk flow
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleHonkSetsState,
    "DudeWalks.Vehicle.HonkSetsState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleHonkSetsState::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 10000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(100.f, 10000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) { Dude->Destroy(); return false; }

    Dude->EnterVehicle(V);
    Dude->FinishEnterVehicle();
    Dude->Honk();
    TestTrue(TEXT("State = Honking after Honk()"), Dude->CharacterState == ECharacterState::Honking);

    Dude->Destroy(); V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleHonkGuard,
    "DudeWalks.Vehicle.HonkGuardNotDriving",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleHonkGuard::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 11000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;

    Dude->Honk();
    TestTrue(TEXT("Honk guard: state unchanged when OnFoot"),
        Dude->CharacterState == ECharacterState::OnFoot);

    Dude->Destroy();
    return true;
}

// =============================================================================
// Vehicle — exit flow
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleExitSetsState,
    "DudeWalks.Vehicle.ExitSetsState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleExitSetsState::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 12000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(100.f, 12000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) { Dude->Destroy(); return false; }

    Dude->EnterVehicle(V);
    Dude->FinishEnterVehicle();
    Dude->ExitVehicle();
    TestTrue(TEXT("State = ExitingVehicle after ExitVehicle()"),
        Dude->CharacterState == ECharacterState::ExitingVehicle);

    Dude->Destroy(); V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleExitFinish,
    "DudeWalks.Vehicle.ExitFinishDetachesAndRestoresOnFoot",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleExitFinish::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 13000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(100.f, 13000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) { Dude->Destroy(); return false; }

    Dude->EnterVehicle(V);
    Dude->FinishEnterVehicle();
    Dude->ExitVehicle();
    Dude->FinishExitVehicle();
    TestTrue(TEXT("State = OnFoot after FinishExitVehicle()"),
        Dude->CharacterState == ECharacterState::OnFoot);
    TestNull(TEXT("Character detached from vehicle"), Dude->GetAttachParentActor());

    Dude->Destroy(); V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleExitGuard,
    "DudeWalks.Vehicle.ExitGuardNotInVehicle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleExitGuard::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 14000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;

    Dude->ExitVehicle();
    TestTrue(TEXT("Exit guard: state unchanged when OnFoot"),
        Dude->CharacterState == ECharacterState::OnFoot);

    Dude->Destroy();
    return true;
}

// =============================================================================
// Vehicle — move blocked
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksVehicleMoveBlocked,
    "DudeWalks.Vehicle.MoveBlockedWhileInVehicle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksVehicleMoveBlocked::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 15000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(100.f, 15000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) { Dude->Destroy(); return false; }

    Dude->EnterVehicle(V);
    Dude->FinishEnterVehicle();

    TestTrue(TEXT("Movement disabled while driving"),
        Dude->GetCharacterMovement()->MovementMode == MOVE_None);

    Dude->Destroy(); V->Destroy();
    return true;
}

// =============================================================================
// Regression — existing behaviors survive
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksRegressionSwimEnabled,
    "DudeWalks.Regression.SwimStillEnabled",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksRegressionSwimEnabled::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 16000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;

    TestTrue(TEXT("bCanSwim still true after refactor"),
        Dude->GetCharacterMovement()->NavAgentProps.bCanSwim);
    TestTrue(TEXT("MaxSwimSpeed = 200"),
        FMath::IsNearlyEqual(Dude->GetCharacterMovement()->MaxSwimSpeed, 200.f));

    Dude->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksRegressionSprintGuard,
    "DudeWalks.Regression.SprintGuardedInVehicle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksRegressionSprintGuard::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 17000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(100.f, 17000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) { Dude->Destroy(); return false; }

    Dude->EnterVehicle(V);
    Dude->FinishEnterVehicle();

    TestTrue(TEXT("CharacterState is Driving (sprint guard would block)"),
        Dude->CharacterState == ECharacterState::Driving);

    Dude->Destroy(); V->Destroy();
    return true;
}
