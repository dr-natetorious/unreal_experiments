#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "DudeWalksTypes.h"
#include "DudeWalksCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class AVehicleBase;

UCLASS()
class DUDEWALKS_API ADudeWalksCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ADudeWalksCharacter();

    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    // --- On-foot input ---
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> SprintAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> EnterVehicleAction;

    // --- Vehicle input ---
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> VehicleMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> HonkAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> ExitVehicleAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> ThrottleAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> ReverseAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> SteerAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> BoostAction;

    // --- Movement speeds ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float WalkSpeed = 200.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float RunSpeed = 375.f;

    // Duration of enter/exit one-shot animations — tune to match actual clip lengths.
    UPROPERTY(EditDefaultsOnly, Category = "Vehicle")
    float EnterAnimDuration = 2.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Vehicle")
    float ExitAnimDuration = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Vehicle")
    float VehicleEnterRadius = 300.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    ECharacterState CharacterState = ECharacterState::OnFoot;

    // Public so Blueprints, level sequences, and tests can drive these directly.
    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void EnterVehicle(AVehicleBase* Vehicle);

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void ExitVehicle();

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void Honk();

    // Exposed for tests so they can skip the timer and assert final state directly.
    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void FinishEnterVehicle();

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    void FinishExitVehicle();

    UFUNCTION(BlueprintCallable, Category = "Vehicle")
    AVehicleBase* FindNearbyVehicle() const;

private:
    void OnMove(const FInputActionValue& Value);
    void OnLook(const FInputActionValue& Value);
    void OnSprintStart();
    void OnSprintEnd();
    void OnEnterVehicle();
    void OnHonk();
    void OnExitVehicle();
    void OnThrottle(const FInputActionValue& Value);
    void OnReverse(const FInputActionValue& Value);
    void OnSteer(const FInputActionValue& Value);
    void OnBoost(const FInputActionValue& Value);

    UPROPERTY()
    TObjectPtr<AVehicleBase> CurrentVehicle;

    FTimerHandle EnterTimerHandle;
    FTimerHandle ExitTimerHandle;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UCameraComponent> FollowCamera;
};
