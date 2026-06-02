#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VehicleBase.generated.h"

UCLASS()
class DUDEWALKS_API AVehicleBase : public AActor
{
    GENERATED_BODY()

public:
    AVehicleBase();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> BodyMesh;

    // Relative offset from vehicle root where the driver sits.
    // Tune this per vehicle Blueprint to align with the seat mesh.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vehicle")
    FVector DriverSeatOffset = FVector(0.f, -50.f, 90.f);

    FVector GetDriverSeatWorldLocation() const;
    FRotator GetDriverSeatWorldRotation() const;
};
