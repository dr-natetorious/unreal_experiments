#include "Vehicle/VehicleBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

AVehicleBase::AVehicleBase()
{
    PrimaryActorTick.bCanEverTick = true;

    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    SetRootComponent(BodyMesh);
    BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BodyMesh->SetCollisionResponseToAllChannels(ECR_Block);
    // Pawn overlaps so character can snap to the seat without being blocked
    BodyMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Tires — no collision; body handles it. Mesh + axle offset set in Blueprint.
    auto MakeTire = [&](const TCHAR* Name) -> UStaticMeshComponent*
    {
        UStaticMeshComponent* T = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        T->SetupAttachment(BodyMesh);
        T->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        return T;
    };
    TireFL = MakeTire(TEXT("TireFL"));
    TireFR = MakeTire(TEXT("TireFR"));
    TireRL = MakeTire(TEXT("TireRL"));
    TireRR = MakeTire(TEXT("TireRR"));

    // Seat markers — drag to correct position in the Blueprint viewport
    auto MakeSeat = [&](const TCHAR* Name) -> USceneComponent*
    {
        USceneComponent* S = CreateDefaultSubobject<USceneComponent>(Name);
        S->SetupAttachment(BodyMesh);
        return S;
    };
    SeatDriver    = MakeSeat(TEXT("SeatDriver"));
    SeatPassenger = MakeSeat(TEXT("SeatPassenger"));
    SeatRearLeft  = MakeSeat(TEXT("SeatRearLeft"));
    SeatRearRight = MakeSeat(TEXT("SeatRearRight"));
}

void AVehicleBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Apply acceleration from this frame's input
    CurrentSpeed += ThrottleInput * Acceleration * DeltaTime;
    CurrentSpeed -= ReverseInput  * Acceleration * DeltaTime;
    // Coast to stop when no throttle/reverse
    CurrentSpeed  = FMath::FInterpTo(CurrentSpeed, 0.f, DeltaTime, Friction);
    CurrentSpeed  = FMath::Clamp(CurrentSpeed, -MaxSpeed * 0.5f, MaxSpeed);

    if (!FMath::IsNearlyZero(CurrentSpeed, 0.5f))
    {
        AddActorLocalOffset(FVector(CurrentSpeed * DeltaTime, 0.f, 0.f), /*bSweep=*/true);

        if (!FMath::IsNearlyZero(SteerInput, 0.01f))
        {
            // Scale turning by speed ratio so the car doesn't spin on the spot
            const float SpeedRatio = FMath::Clamp(FMath::Abs(CurrentSpeed) / MaxSpeed, 0.1f, 1.f);
            // Reverse steering direction when going in reverse
            const float Dir = CurrentSpeed > 0.f ? 1.f : -1.f;
            const float TurnDeg = SteerInput * TurnSpeed * DeltaTime * SpeedRatio * Dir;
            AddActorLocalRotation(FRotator(0.f, TurnDeg, 0.f));
        }

        // Tire roll: (speed / circumference) * 360 degrees per second
        const float RollDeg = (CurrentSpeed / (2.f * PI * TireRadius)) * 360.f * DeltaTime;
        UStaticMeshComponent* Tires[] = { TireFL.Get(), TireFR.Get(), TireRL.Get(), TireRR.Get() };
        for (UStaticMeshComponent* Tire : Tires)
        {
            if (Tire)
            {
                FRotator Rot = Tire->GetRelativeRotation();
                Rot.Pitch += RollDeg;
                Tire->SetRelativeRotation(Rot);
            }
        }
    }

    // Reset frame inputs — character re-applies each frame while keys held
    ThrottleInput = 0.f;
    ReverseInput  = 0.f;
    SteerInput    = 0.f;
}

void AVehicleBase::ApplyThrottle(float Axis)
{
    ThrottleInput = FMath::Clamp(Axis, 0.f, 1.f);
}

void AVehicleBase::ApplyReverse(float Axis)
{
    ReverseInput = FMath::Clamp(Axis, 0.f, 1.f);
}

void AVehicleBase::ApplySteering(float Axis)
{
    SteerInput = FMath::Clamp(Axis, -1.f, 1.f);
}

FVector AVehicleBase::GetSeatWorldLocation(FName SeatName) const
{
    USceneComponent* Seats[] = { SeatDriver.Get(), SeatPassenger.Get(), SeatRearLeft.Get(), SeatRearRight.Get() };
    for (USceneComponent* Seat : Seats)
    {
        if (Seat && Seat->GetFName() == SeatName)
            return Seat->GetComponentLocation();
    }
    return GetActorLocation();
}
