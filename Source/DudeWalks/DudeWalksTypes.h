#pragma once
#include "CoreMinimal.h"
#include "DudeWalksTypes.generated.h"

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
    OnFoot          UMETA(DisplayName = "On Foot"),
    EnteringVehicle UMETA(DisplayName = "Entering Vehicle"),
    Driving         UMETA(DisplayName = "Driving"),
    Honking         UMETA(DisplayName = "Honking"),
    ExitingVehicle  UMETA(DisplayName = "Exiting Vehicle"),
};
