#include "Vehicle/VehicleBase.h"
#include "Components/StaticMeshComponent.h"

AVehicleBase::AVehicleBase()
{
    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    SetRootComponent(BodyMesh);
    BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BodyMesh->SetCollisionResponseToAllChannels(ECR_Block);
    // Pawn can overlap (not block) so the character can snap to the seat
    BodyMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

FVector AVehicleBase::GetDriverSeatWorldLocation() const
{
    return GetActorTransform().TransformPosition(DriverSeatOffset);
}

FRotator AVehicleBase::GetDriverSeatWorldRotation() const
{
    return GetActorRotation();
}
