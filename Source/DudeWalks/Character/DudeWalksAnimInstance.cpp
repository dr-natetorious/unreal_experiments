#include "Character/DudeWalksAnimInstance.h"
#include "Character/DudeWalksCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UDudeWalksAnimInstance::NativeUpdateAnimation(float /*DeltaSeconds*/)
{
    const ADudeWalksCharacter* Owner = Cast<ADudeWalksCharacter>(TryGetPawnOwner());
    if (!Owner)
    {
        Speed = 0.f;
        bIsSprinting = false;
        return;
    }

    const float RawSpeed = Owner->GetCharacterMovement()->Velocity.Size2D();
    Speed = FMath::GetMappedRangeValueClamped(
        FVector2D(0.f, Owner->RunSpeed), FVector2D(0.f, 100.f), RawSpeed);
    bIsMoving = RawSpeed > 10.f;
    MoveAlpha = bIsMoving ? 1.f : 0.f;
    bIsSprinting = Owner->GetCharacterMovement()->MaxWalkSpeed >= Owner->RunSpeed;
    SprintAlpha = bIsSprinting ? 1.f : 0.f;
}
