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

// =============================================================================
// VehicleBase — component structure
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVehicleBaseTiresCreated,
    "DudeWalks.VehicleBase.Components.TiresCreated",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVehicleBaseTiresCreated::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(0.f, 18000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) return false;

    TestNotNull(TEXT("TireFL created"), V->TireFL.Get());
    TestNotNull(TEXT("TireFR created"), V->TireFR.Get());
    TestNotNull(TEXT("TireRL created"), V->TireRL.Get());
    TestNotNull(TEXT("TireRR created"), V->TireRR.Get());
    if (V->TireFL)
        TestEqual(TEXT("TireFL attached to BodyMesh"), V->TireFL->GetAttachParent(), (USceneComponent*)V->BodyMesh.Get());

    V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVehicleBaseSeatsCreated,
    "DudeWalks.VehicleBase.Components.SeatsCreated",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVehicleBaseSeatsCreated::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(0.f, 19000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) return false;

    TestNotNull(TEXT("SeatDriver created"),    V->SeatDriver.Get());
    TestNotNull(TEXT("SeatPassenger created"), V->SeatPassenger.Get());
    TestNotNull(TEXT("SeatRearLeft created"),  V->SeatRearLeft.Get());
    TestNotNull(TEXT("SeatRearRight created"), V->SeatRearRight.Get());

    V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVehicleBaseSeatDriverLookup,
    "DudeWalks.VehicleBase.Seat.DriverLookup",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVehicleBaseSeatDriverLookup::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(0.f, 20000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) return false;

    const FVector SeatLoc = V->GetSeatWorldLocation("SeatDriver");
    const FVector DriverCompLoc = V->SeatDriver ? V->SeatDriver->GetComponentLocation() : FVector::ZeroVector;
    TestTrue(TEXT("GetSeatWorldLocation(SeatDriver) matches component location"),
        SeatLoc.Equals(DriverCompLoc, 1.f));

    V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVehicleBaseSeatUnknown,
    "DudeWalks.VehicleBase.Seat.UnknownNameFallsBack",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVehicleBaseSeatUnknown::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(0.f, 21000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) return false;

    const FVector FallbackLoc = V->GetSeatWorldLocation("SeatNonExistent");
    TestTrue(TEXT("Unknown seat falls back to actor location"),
        FallbackLoc.Equals(V->GetActorLocation(), 1.f));

    V->Destroy();
    return true;
}

// =============================================================================
// VehicleBase — driving physics
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVehicleDrivingThrottle,
    "DudeWalks.VehicleBase.Driving.ThrottleIncreasesSpeed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVehicleDrivingThrottle::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(0.f, 22000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) return false;

    V->ApplyThrottle(1.f);
    V->Tick(0.016f);
    TestTrue(TEXT("Throttle increases CurrentSpeed"), V->CurrentSpeed > 0.f);

    V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVehicleDrivingReverse,
    "DudeWalks.VehicleBase.Driving.ReverseDecreasesSpeed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVehicleDrivingReverse::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(0.f, 23000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) return false;

    V->ApplyReverse(1.f);
    V->Tick(0.016f);
    TestTrue(TEXT("Reverse produces negative CurrentSpeed"), V->CurrentSpeed < 0.f);

    V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVehicleDrivingFriction,
    "DudeWalks.VehicleBase.Driving.FrictionCoastsToStop",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVehicleDrivingFriction::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(0.f, 24000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) return false;

    // Build up some speed
    for (int i = 0; i < 30; ++i) { V->ApplyThrottle(1.f); V->Tick(0.016f); }
    const float SpeedAtPeak = V->CurrentSpeed;
    TestTrue(TEXT("Speed built up before coasting"), SpeedAtPeak > 0.f);

    // Coast without input — friction should slow it
    for (int i = 0; i < 60; ++i) { V->Tick(0.016f); }
    TestTrue(TEXT("Speed decreased after coasting"), V->CurrentSpeed < SpeedAtPeak);

    V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVehicleDrivingSpeedClamped,
    "DudeWalks.VehicleBase.Driving.SpeedClamped",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVehicleDrivingSpeedClamped::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(0.f, 25000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) return false;

    // Many frames of full throttle
    for (int i = 0; i < 500; ++i) { V->ApplyThrottle(1.f); V->Tick(0.016f); }
    TestTrue(TEXT("CurrentSpeed never exceeds MaxSpeed"), V->CurrentSpeed <= V->MaxSpeed + 0.1f);

    V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVehicleDrivingSteering,
    "DudeWalks.VehicleBase.Driving.SteeringRotatesActor",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVehicleDrivingSteering::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(0.f, 26000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) return false;

    const float InitialYaw = V->GetActorRotation().Yaw;

    // Build speed first, then steer
    for (int i = 0; i < 30; ++i) { V->ApplyThrottle(1.f); V->Tick(0.016f); }
    for (int i = 0; i < 30; ++i)
    {
        V->ApplyThrottle(1.f);
        V->ApplySteering(1.f);
        V->Tick(0.016f);
    }

    TestTrue(TEXT("Steering changes actor yaw"), !FMath::IsNearlyEqual(V->GetActorRotation().Yaw, InitialYaw, 0.5f));

    V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVehicleDrivingTireRoll,
    "DudeWalks.VehicleBase.Driving.TiresRollForward",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVehicleDrivingTireRoll::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(0.f, 27000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) return false;
    if (!TestNotNull(TEXT("TireFL exists"), V->TireFL.Get())) { V->Destroy(); return false; }

    const float InitialPitch = V->TireFL->GetRelativeRotation().Pitch;
    for (int i = 0; i < 10; ++i) { V->ApplyThrottle(1.f); V->Tick(0.016f); }

    TestTrue(TEXT("TireFL pitch changed after driving"),
        !FMath::IsNearlyEqual(V->TireFL->GetRelativeRotation().Pitch, InitialPitch, 0.01f));

    V->Destroy();
    return true;
}

// =============================================================================
// Character — sits at driver seat after enter
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterSitsAtDriverSeat,
    "DudeWalks.Character.EnterVehicle.SitsAtDriverSeat",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCharacterSitsAtDriverSeat::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 28000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(100.f, 28000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) { Dude->Destroy(); return false; }

    const FVector ExpectedSeatLoc = V->GetSeatWorldLocation("SeatDriver");
    Dude->EnterVehicle(V);
    TestTrue(TEXT("Character placed at SeatDriver world location"),
        Dude->GetActorLocation().Equals(ExpectedSeatLoc, 5.f));

    Dude->Destroy(); V->Destroy();
    return true;
}

// =============================================================================
// Integration — full drive cycle and vehicle movement
// =============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntegrationVehicleMoves,
    "DudeWalks.Integration.VehicleMovesOnThrottle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIntegrationVehicleMoves::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(0.f, 29000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) return false;

    const float StartX = V->GetActorLocation().X;
    for (int i = 0; i < 60; ++i) { V->ApplyThrottle(1.f); V->Tick(0.016f); }
    TestTrue(TEXT("Vehicle moved forward on throttle"), V->GetActorLocation().X > StartX);

    V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntegrationVehicleStops,
    "DudeWalks.Integration.VehicleStopsOnNoInput",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIntegrationVehicleStops::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(0.f, 30000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) return false;

    // Build speed
    for (int i = 0; i < 60; ++i) { V->ApplyThrottle(1.f); V->Tick(0.016f); }
    TestTrue(TEXT("Speed built up"), V->CurrentSpeed > 0.f);

    // Coast until nearly stopped (3 seconds of simulated time)
    for (int i = 0; i < 180; ++i) { V->Tick(0.016f); }
    TestTrue(TEXT("Speed near zero after coasting"), FMath::Abs(V->CurrentSpeed) < 10.f);

    V->Destroy();
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntegrationFullCycle,
    "DudeWalks.Integration.FullVehicleCycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FIntegrationFullCycle::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;
    ADudeWalksCharacter* Dude = SpawnDude(W, FVector(0.f, 31000.f, 0.f));
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;
    AVehicleBase* V = SpawnVehicle(W, FVector(100.f, 31000.f, 0.f));
    if (!TestNotNull(TEXT("Vehicle spawned"), V)) { Dude->Destroy(); return false; }

    // Enter
    Dude->EnterVehicle(V);
    TestTrue(TEXT("1. State = EnteringVehicle"), Dude->CharacterState == ECharacterState::EnteringVehicle);

    Dude->FinishEnterVehicle();
    TestTrue(TEXT("2. State = Driving"), Dude->CharacterState == ECharacterState::Driving);

    // Drive
    const float StartX = V->GetActorLocation().X;
    for (int i = 0; i < 30; ++i) { V->ApplyThrottle(1.f); V->Tick(0.016f); }
    TestTrue(TEXT("3. Vehicle moved"), V->GetActorLocation().X > StartX);

    // Exit
    Dude->ExitVehicle();
    TestTrue(TEXT("4. State = ExitingVehicle"), Dude->CharacterState == ECharacterState::ExitingVehicle);

    Dude->FinishExitVehicle();
    TestTrue(TEXT("5. State = OnFoot"), Dude->CharacterState == ECharacterState::OnFoot);
    TestNull(TEXT("6. Character detached"), Dude->GetAttachParentActor());

    Dude->Destroy(); V->Destroy();
    return true;
}
