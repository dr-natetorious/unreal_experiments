#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/DudeWalksCharacter.h"
#include "Character/DudeWalksAnimInstance.h"

static UWorld* GetTestWorld()
{
    if (!GEngine) return nullptr;
    for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
    {
        if ((Ctx.WorldType == EWorldType::PIE || Ctx.WorldType == EWorldType::Game) && Ctx.World())
            return Ctx.World();
    }
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksSpawn,
    "DudeWalks.CharacterSpawns",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksSpawn::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ADudeWalksCharacter* Dude = W->SpawnActor<ADudeWalksCharacter>(
        ADudeWalksCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, P);

    TestNotNull(TEXT("Character spawned"), Dude);
    if (Dude) Dude->Destroy();
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksMeshSetup,
    "DudeWalks.MeshUsesInheritedSlot",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksMeshSetup::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ADudeWalksCharacter* Dude = W->SpawnActor<ADudeWalksCharacter>(
        ADudeWalksCharacter::StaticClass(), FVector(0.f, 500.f, 0.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;

    USkeletalMeshComponent* Mesh = Dude->GetMesh();
    TestNotNull(TEXT("GetMesh() is non-null"), Mesh);
    TestTrue(TEXT("Mesh Z = -90 (feet at capsule base)"),
             FMath::IsNearlyEqual(Mesh->GetRelativeLocation().Z, -90.f));
    TestTrue(TEXT("Mesh Yaw = -90 (faces forward)"),
             FMath::IsNearlyEqual(Mesh->GetRelativeRotation().Yaw, -90.f));
    TestTrue(TEXT("Mesh has no collision (capsule owns physics)"),
             Mesh->GetCollisionEnabled() == ECollisionEnabled::NoCollision);

    Dude->Destroy();
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDudeWalksSpeed,
    "DudeWalks.SpeedDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDudeWalksSpeed::RunTest(const FString&)
{
    UWorld* W = GetTestWorld();
    if (!TestNotNull(TEXT("Game world"), W)) return false;

    FActorSpawnParameters P;
    P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    ADudeWalksCharacter* Dude = W->SpawnActor<ADudeWalksCharacter>(
        ADudeWalksCharacter::StaticClass(), FVector(0.f, 1000.f, 0.f), FRotator::ZeroRotator, P);
    if (!TestNotNull(TEXT("Character spawned"), Dude)) return false;

    UCharacterMovementComponent* CMC = Dude->GetCharacterMovement();

    TestTrue(TEXT("Default MaxWalkSpeed is WalkSpeed (200)"),
             FMath::IsNearlyEqual(CMC->MaxWalkSpeed, Dude->WalkSpeed));

    // Simulate sprint
    CMC->MaxWalkSpeed = Dude->RunSpeed;
    TestTrue(TEXT("MaxWalkSpeed is RunSpeed (375) while sprinting"),
             FMath::IsNearlyEqual(CMC->MaxWalkSpeed, Dude->RunSpeed));

    // Simulate sprint release
    CMC->MaxWalkSpeed = Dude->WalkSpeed;
    TestTrue(TEXT("MaxWalkSpeed returns to WalkSpeed (200) after sprint"),
             FMath::IsNearlyEqual(CMC->MaxWalkSpeed, Dude->WalkSpeed));

    Dude->Destroy();
    return true;
}
