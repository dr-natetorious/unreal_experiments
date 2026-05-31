#include "Character/DudeWalksCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

ADudeWalksCharacter::ADudeWalksCharacter()
{
    // Use ACharacter::GetMesh() — never a separate component.
    // TryGetPawnOwner() in the ABP only resolves when the mesh IS the character's
    // inherited mesh slot. A custom USkeletalMeshComponent breaks that link.
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
    EIC->BindAction(MoveAction,   ETriggerEvent::Triggered,  this, &ADudeWalksCharacter::OnMove);
    EIC->BindAction(LookAction,   ETriggerEvent::Triggered,  this, &ADudeWalksCharacter::OnLook);
    EIC->BindAction(SprintAction, ETriggerEvent::Started,    this, &ADudeWalksCharacter::OnSprintStart);
    EIC->BindAction(SprintAction, ETriggerEvent::Completed,  this, &ADudeWalksCharacter::OnSprintEnd);
    EIC->BindAction(JumpAction,   ETriggerEvent::Started,    this, &ACharacter::Jump);
    EIC->BindAction(JumpAction,   ETriggerEvent::Completed,  this, &ACharacter::StopJumping);
}

void ADudeWalksCharacter::OnMove(const FInputActionValue& Value)
{
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
    GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void ADudeWalksCharacter::OnSprintEnd()
{
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}
