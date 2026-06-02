#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VehicleBase.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class DUDEWALKS_API AVehicleBase : public AActor
{
    GENERATED_BODY()

public:
    AVehicleBase();

    virtual void Tick(float DeltaTime) override;

    // --- Body ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle")
    TObjectPtr<UStaticMeshComponent> BodyMesh;

    // --- Tires — assign mesh + relative location (axle offset) in Blueprint ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle")
    TObjectPtr<UStaticMeshComponent> TireFL;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle")
    TObjectPtr<UStaticMeshComponent> TireFR;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle")
    TObjectPtr<UStaticMeshComponent> TireRL;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle")
    TObjectPtr<UStaticMeshComponent> TireRR;

    // --- Named seat markers — drag in Blueprint viewport to match seat geometry ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Seats")
    TObjectPtr<USceneComponent> SeatDriver;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Seats")
    TObjectPtr<USceneComponent> SeatPassenger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Seats")
    TObjectPtr<USceneComponent> SeatRearLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vehicle|Seats")
    TObjectPtr<USceneComponent> SeatRearRight;

    // --- Driving physics ---
    UPROPERTY(EditDefaultsOnly, Category="Driving")
    float MaxSpeed = 1200.f;    // cm/s forward

    UPROPERTY(EditDefaultsOnly, Category="Driving")
    float Acceleration = 800.f; // cm/s² applied per axis unit

    UPROPERTY(EditDefaultsOnly, Category="Driving")
    float Friction = 4.f;       // FInterpTo speed toward zero when no input

    UPROPERTY(EditDefaultsOnly, Category="Driving")
    float TurnSpeed = 80.f;     // deg/s at full steer, scales with speed ratio

    UPROPERTY(EditDefaultsOnly, Category="Driving")
    float TireRadius = 38.f;    // cm — from Blender measurement (0.38m)

    // Set each frame by the driving character; read in Tick.
    float CurrentSpeed = 0.f;

    void ApplyThrottle(float Axis);
    void ApplyReverse(float Axis);
    void ApplySteering(float Axis);

    // Returns world location of named seat component ("SeatDriver" etc.).
    // Falls back to actor location if name not found.
    FVector GetSeatWorldLocation(FName SeatName) const;

    // Override in Blueprint for vehicle-specific lights (siren, fire truck bar, etc.).
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Vehicle")
    void ToggleLights();
    virtual void ToggleLights_Implementation() {}

private:
    float ThrottleInput = 0.f;
    float ReverseInput  = 0.f;
    float SteerInput    = 0.f;
};
