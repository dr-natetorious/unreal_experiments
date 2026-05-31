#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DudeWalksAnimInstance.generated.h"

/**
 * Parent class for ABP_DudeWalks.
 * In editor: Class Settings -> Parent Class -> DudeWalksAnimInstance.
 * Remove all EventGraph Blueprint nodes — NativeUpdateAnimation drives it.
 */
UCLASS()
class DUDEWALKS_API UDudeWalksAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    // Horizontal speed in cm/s. BlendSpace X axis: 0=idle, 200=walk, 375=run.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DudeWalks")
    float Speed = 0.f;

    // True while Shift is held (MaxWalkSpeed >= RunSpeed). Drives run blend.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DudeWalks")
    bool bIsSprinting = false;

    // True when character has any horizontal velocity (Speed > 10 cm/s).
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DudeWalks")
    bool bIsMoving = false;

    // 0.0=idle, 1.0=walking/running. Direct float for TwoWayBlend Alpha.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DudeWalks")
    float MoveAlpha = 0.f;

    // 0.0=walk, 1.0=run. Direct float for Walk->Run TwoWayBlend Alpha.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DudeWalks")
    float SprintAlpha = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DudeWalks")
    bool bIsInAir = false;

    // 0.0=grounded, 1.0=in air. Drives jump TwoWayBlend Alpha.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DudeWalks")
    float JumpAlpha = 0.f;

protected:
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
