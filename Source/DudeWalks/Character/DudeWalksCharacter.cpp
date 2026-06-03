#include "Character/DudeWalksCharacter.h"
#include "Vehicle/VehicleBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"

ADudeWalksCharacter::ADudeWalksCharacter()
{
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->GroundFriction = 8.f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->NavAgentProps.bCanSwim = true;
    GetCharacterMovement()->MaxSwimSpeed = 200.f;

    bUseControllerRotationYaw = false;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void ADudeWalksCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Sub =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Sub->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void ADudeWalksCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
    EIC->BindAction(MoveAction,         ETriggerEvent::Triggered,  this, &ADudeWalksCharacter::OnMove);
    EIC->BindAction(LookAction,         ETriggerEvent::Triggered,  this, &ADudeWalksCharacter::OnLook);
    EIC->BindAction(SprintAction,       ETriggerEvent::Started,    this, &ADudeWalksCharacter::OnSprintStart);
    EIC->BindAction(SprintAction,       ETriggerEvent::Completed,  this, &ADudeWalksCharacter::OnSprintEnd);
    EIC->BindAction(JumpAction,         ETriggerEvent::Started,    this, &ACharacter::Jump);
    EIC->BindAction(JumpAction,         ETriggerEvent::Completed,  this, &ACharacter::StopJumping);
    EIC->BindAction(EnterVehicleAction, ETriggerEvent::Started,    this, &ADudeWalksCharacter::OnEnterVehicle);
    EIC->BindAction(HonkAction,         ETriggerEvent::Started,    this, &ADudeWalksCharacter::OnHonk);
    EIC->BindAction(ExitVehicleAction,  ETriggerEvent::Triggered,  this, &ADudeWalksCharacter::OnExitVehicle);
    if (ThrottleAction)
        EIC->BindAction(ThrottleAction, ETriggerEvent::Triggered,  this, &ADudeWalksCharacter::OnThrottle);
    if (ReverseAction)
        EIC->BindAction(ReverseAction,  ETriggerEvent::Triggered,  this, &ADudeWalksCharacter::OnReverse);
    if (SteerAction)
        EIC->BindAction(SteerAction,    ETriggerEvent::Triggered,  this, &ADudeWalksCharacter::OnSteer);
    if (BoostAction)
        EIC->BindAction(BoostAction,    ETriggerEvent::Triggered,  this, &ADudeWalksCharacter::OnBoost);
}

// ---------------------------------------------------------------------------
// On-foot movement
// ---------------------------------------------------------------------------

void ADudeWalksCharacter::OnMove(const FInputActionValue& Value)
{
    if (CharacterState != ECharacterState::OnFoot) return;
    const FVector2D Axis = Value.Get<FVector2D>();
    if (!Controller || Axis.IsZero()) return;

    const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
    AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), Axis.Y);
    AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), Axis.X);
}

void ADudeWalksCharacter::OnLook(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    AddControllerYawInput(Axis.X);
    AddControllerPitchInput(Axis.Y);
}

void ADudeWalksCharacter::OnSprintStart()
{
    if (CharacterState == ECharacterState::OnFoot)
        GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void ADudeWalksCharacter::OnSprintEnd()
{
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

// ---------------------------------------------------------------------------
// Vehicle — enter
// ---------------------------------------------------------------------------

void ADudeWalksCharacter::OnEnterVehicle()
{
    if (CharacterState != ECharacterState::OnFoot) return;
    if (AVehicleBase* V = FindNearbyVehicle())
        EnterVehicle(V);
}

AVehicleBase* ADudeWalksCharacter::FindNearbyVehicle() const
{
    AVehicleBase* Nearest = nullptr;
    float BestDistSq = VehicleEnterRadius * VehicleEnterRadius;
    for (TActorIterator<AVehicleBase> It(GetWorld()); It; ++It)
    {
        const float DSq = FVector::DistSquared(GetActorLocation(), It->GetActorLocation());
        if (DSq < BestDistSq)
        {
            BestDistSq = DSq;
            Nearest = *It;
        }
    }
    return Nearest;
}

void ADudeWalksCharacter::EnterVehicle(AVehicleBase* Vehicle)
{
    CurrentVehicle = Vehicle;
    CharacterState = ECharacterState::EnteringVehicle;

    GetCharacterMovement()->DisableMovement();

    const FVector SeatLoc = Vehicle->GetSeatWorldLocation("SeatDriver");
    const FRotator SeatRot = Vehicle->GetActorRotation();
    SetActorLocationAndRotation(SeatLoc, SeatRot);
    AttachToActor(Vehicle, FAttachmentTransformRules::KeepWorldTransform);

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Sub =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Sub->RemoveMappingContext(DefaultMappingContext);
            Sub->AddMappingContext(VehicleMappingContext, 1);
        }
        PC->SetViewTarget(Vehicle);
    }

    GetWorld()->GetTimerManager().SetTimer(
        EnterTimerHandle, this, &ADudeWalksCharacter::FinishEnterVehicle,
        EnterAnimDuration, false);
}

void ADudeWalksCharacter::FinishEnterVehicle()
{
    CharacterState = ECharacterState::Driving;
}

// ---------------------------------------------------------------------------
// Vehicle — honk
// ---------------------------------------------------------------------------

void ADudeWalksCharacter::OnHonk()
{
    Honk();
}

void ADudeWalksCharacter::Honk()
{
    if (CharacterState != ECharacterState::Driving) return;
    CharacterState = ECharacterState::Honking;
    GetWorld()->GetTimerManager().SetTimer(
        ExitTimerHandle, [this]()
        {
            if (CharacterState == ECharacterState::Honking)
                CharacterState = ECharacterState::Driving;
        }, 1.f, false);
}

// ---------------------------------------------------------------------------
// Vehicle — exit
// ---------------------------------------------------------------------------

void ADudeWalksCharacter::OnExitVehicle()
{
    if (CharacterState != ECharacterState::Driving && CharacterState != ECharacterState::Honking) return;
    ExitVehicle();
}

void ADudeWalksCharacter::ExitVehicle()
{
    if (CharacterState != ECharacterState::Driving && CharacterState != ECharacterState::Honking) return;
    GetWorld()->GetTimerManager().ClearTimer(ExitTimerHandle);
    CharacterState = ECharacterState::ExitingVehicle;

    GetWorld()->GetTimerManager().SetTimer(
        ExitTimerHandle, this, &ADudeWalksCharacter::FinishExitVehicle,
        ExitAnimDuration, false);
}

// ---------------------------------------------------------------------------
// Vehicle — driving inputs (forwarded to CurrentVehicle each frame)
// ---------------------------------------------------------------------------

void ADudeWalksCharacter::OnThrottle(const FInputActionValue& Value)
{
    if (CurrentVehicle)
        CurrentVehicle->ApplyThrottle(Value.Get<float>());
}

void ADudeWalksCharacter::OnReverse(const FInputActionValue& Value)
{
    if (CurrentVehicle)
        CurrentVehicle->ApplyReverse(Value.Get<float>());
}

void ADudeWalksCharacter::OnSteer(const FInputActionValue& Value)
{
    if (CurrentVehicle)
        CurrentVehicle->ApplySteering(Value.Get<float>());
}

void ADudeWalksCharacter::OnBoost(const FInputActionValue& Value)
{
    if (CurrentVehicle)
        CurrentVehicle->ApplyBoost(Value.Get<float>());
}

void ADudeWalksCharacter::FinishExitVehicle()
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    if (CurrentVehicle)
    {
        // Step out to the left of the vehicle
        const FVector ExitOffset = CurrentVehicle->GetActorRightVector() * -150.f;
        SetActorLocation(GetActorLocation() + ExitOffset, false, nullptr, ETeleportType::TeleportPhysics);
    }

    CurrentVehicle = nullptr;
    GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
    CharacterState = ECharacterState::OnFoot;

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetViewTarget(this);
        if (UEnhancedInputLocalPlayerSubsystem* Sub =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Sub->RemoveMappingContext(VehicleMappingContext);
            Sub->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}
