#include "Characters/CCursedCargoCharacter.h"

#include "AI/CCMonster.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Game/CCGameState.h"
#include "Interaction/CCInteractInterface.h"
#include "Items/CCCollectibleItem.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ACCursedCargoCharacter::ACCursedCargoCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    PrototypeBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeBody"));
    PrototypeBody->SetupAttachment(RootComponent);
    PrototypeBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PrototypeBody->SetRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
    PrototypeBody->SetRelativeScale3D(FVector(0.45f, 0.45f, 1.5f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
        TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderMesh.Succeeded())
    {
        PrototypeBody->SetStaticMesh(CylinderMesh.Object);
    }

    CarryAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("CarryAnchor"));
    CarryAnchor->SetupAttachment(RootComponent);
    CarryAnchor->SetRelativeLocation(FVector(90.0f, 0.0f, 25.0f));
}

void ACCursedCargoCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        Health = MaxHealth;
        Stamina = MaxStamina;
    }

    ApplySprintState(bIsSprinting);

    if (GetMesh() && GetMesh()->DoesSocketExist(CarrySocketName))
    {
        CarryAnchor->AttachToComponent(
            GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            CarrySocketName);
    }
}

void ACCursedCargoCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority() || Health <= 0)
    {
        return;
    }

    const bool bIsMoving = GetVelocity().SizeSquared2D() >= FMath::Square(100.0f);
    if (bIsSprinting && bIsMoving && Stamina > 0.0f)
    {
        Stamina = FMath::Max(0.0f, Stamina - StaminaDrainPerSecond * DeltaSeconds);
        if (Stamina <= 0.0f)
        {
            ApplySprintState(false);
        }
    }
    else
    {
        Stamina = FMath::Min(MaxStamina, Stamina + StaminaRecoveryPerSecond * DeltaSeconds);
    }

    if (!bIsSprinting || !bIsMoving)
    {
        RunningNoiseAccumulator = 0.0f;
        return;
    }

    RunningNoiseAccumulator += DeltaSeconds;
    if (RunningNoiseAccumulator < 0.75f)
    {
        return;
    }

    RunningNoiseAccumulator = 0.0f;
    for (TActorIterator<ACCMonster> It(GetWorld()); It; ++It)
    {
        It->ReactToNoise(GetActorLocation(), 0.45f, this);
    }
}

void ACCursedCargoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    check(PlayerInputComponent);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
    PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &ACCursedCargoCharacter::HandleInteractPressed);
    PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &ACCursedCargoCharacter::StartSprinting);
    PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &ACCursedCargoCharacter::StopSprinting);

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACCursedCargoCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACCursedCargoCharacter::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &ACCursedCargoCharacter::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ACCursedCargoCharacter::LookUp);
}

void ACCursedCargoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACCursedCargoCharacter, CarriedItem);
    DOREPLIFETIME(ACCursedCargoCharacter, Health);
    DOREPLIFETIME(ACCursedCargoCharacter, Stamina);
    DOREPLIFETIME(ACCursedCargoCharacter, bIsSprinting);
}

void ACCursedCargoCharacter::TryCarryItem(ACCCollectibleItem* Item)
{
    if (!HasAuthority() || CarriedItem || !Item || !Item->CanBePickedUp() || !IsInteractionCandidateValid(Item))
    {
        return;
    }

    CarriedItem = Item;
    Item->PickUp(this);
    ForceNetUpdate();

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            2.0f,
            FColor::Green,
            TEXT("Cargo picked up - press E to drop"));
    }
}

bool ACCursedCargoCharacter::HasCargoInReach() const
{
    return CarriedItem == nullptr && FindFocusedInteractable() != nullptr;
}

ACCCollectibleItem* ACCursedCargoCharacter::GetRelevantCargo() const
{
    return CarriedItem ? CarriedItem.Get() : Cast<ACCCollectibleItem>(FindFocusedInteractable());
}

void ACCursedCargoCharacter::ReceiveMonsterHit()
{
    if (!HasAuthority() || Health <= 0 || bDamageCooldownActive)
    {
        return;
    }

    Health = FMath::Max(0, Health - 1);
    bDamageCooldownActive = true;
    ForceNetUpdate();
    OnRep_Health();

    if (Health == 0)
    {
        GetCharacterMovement()->DisableMovement();
        if (ACCGameState* Mission = GetWorld()->GetGameState<ACCGameState>())
        {
            Mission->FailMission();
        }
        return;
    }

    GetWorldTimerManager().SetTimer(
        DamageCooldownTimer,
        this,
        &ACCursedCargoCharacter::ResetDamageCooldown,
        2.0f,
        false);
}

void ACCursedCargoCharacter::OnRep_CarriedItem()
{
    // The item owns its attachment RepNotify. This callback exists for UI/Blueprint observation.
}

void ACCursedCargoCharacter::OnRep_Health()
{
    OnHealthChanged(Health);

    if (GEngine && Health > 0)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            2.0f,
            FColor::Red,
            FString::Printf(TEXT("Monster hit! Health: %d/%d"), Health, MaxHealth));
    }
}

void ACCursedCargoCharacter::OnRep_Stamina()
{
    if (Stamina <= 0.0f && bIsSprinting)
    {
        ApplySprintState(false);
    }
}

void ACCursedCargoCharacter::OnRep_IsSprinting()
{
    GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
}

void ACCursedCargoCharacter::ServerTryInteract_Implementation(AActor* Candidate)
{
    if (!Candidate || CarriedItem || !IsInteractionCandidateValid(Candidate))
    {
        return;
    }

    if (Candidate->GetClass()->ImplementsInterface(UCCInteractInterface::StaticClass()) &&
        ICCInteractInterface::Execute_CanInteract(Candidate, this))
    {
        ICCInteractInterface::Execute_Interact(Candidate, this);
    }
}

void ACCursedCargoCharacter::ServerDropCarriedItem_Implementation()
{
    DropCarriedItem();
}

void ACCursedCargoCharacter::ServerSetSprinting_Implementation(bool bNewSprinting)
{
    ApplySprintState(bNewSprinting && Stamina > 0.0f && Health > 0);
}

void ACCursedCargoCharacter::MoveForward(float Value)
{
    if (Controller && !FMath::IsNearlyZero(Value))
    {
        const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
    }
}

void ACCursedCargoCharacter::MoveRight(float Value)
{
    if (Controller && !FMath::IsNearlyZero(Value))
    {
        const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
        AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
    }
}

void ACCursedCargoCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void ACCursedCargoCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ACCursedCargoCharacter::StartSprinting()
{
    if (Stamina <= 0.0f || Health <= 0)
    {
        return;
    }

    ApplySprintState(true);
    if (!HasAuthority())
    {
        ServerSetSprinting(true);
    }
}

void ACCursedCargoCharacter::StopSprinting()
{
    ApplySprintState(false);
    if (!HasAuthority())
    {
        ServerSetSprinting(false);
    }
}

void ACCursedCargoCharacter::HandleInteractPressed()
{
    if (CarriedItem)
    {
        if (HasAuthority())
        {
            DropCarriedItem();
        }
        else
        {
            ServerDropCarriedItem();
        }
        return;
    }

    AActor* Candidate = FindFocusedInteractable();
    if (!Candidate)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                2.0f,
                FColor::Red,
                TEXT("No cargo in reach"));
        }
        return;
    }

    if (HasAuthority())
    {
        ServerTryInteract_Implementation(Candidate);
    }
    else
    {
        ServerTryInteract(Candidate);
    }
}

void ACCursedCargoCharacter::DropCarriedItem()
{
    if (!HasAuthority() || !CarriedItem)
    {
        return;
    }

    ACCCollectibleItem* ItemToDrop = CarriedItem;
    CarriedItem = nullptr;

    const FVector Forward = GetActorForwardVector();
    const FVector DropLocation = GetActorLocation() + Forward * DropForwardOffset + FVector(0.0f, 0.0f, 30.0f);
    ItemToDrop->Drop(DropLocation, Forward * ThrowImpulse);
    ForceNetUpdate();

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            2.0f,
            FColor::Yellow,
            TEXT("Cargo dropped"));
    }
}

AActor* ACCursedCargoCharacter::FindFocusedInteractable() const
{
    if (!GetWorld())
    {
        return nullptr;
    }

    FVector ViewLocation;
    FRotator ViewRotation;
    if (Controller)
    {
        Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
    }
    else
    {
        GetActorEyesViewPoint(ViewLocation, ViewRotation);
    }

    FHitResult Hit;
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CursedCargoInteraction), false, this);
    const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * InteractionDistance;

    if (GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, TraceEnd, ECC_Visibility, QueryParams))
    {
        AActor* HitActor = Hit.GetActor();
        if (HitActor && HitActor->GetClass()->ImplementsInterface(UCCInteractInterface::StaticClass()))
        {
            return HitActor;
        }
    }

    // A precise camera trace is awkward without a crosshair. For the prototype,
    // fall back to the best nearby cargo item in the player's forward hemisphere.
    AActor* BestCandidate = nullptr;
    float BestScore = -UE_BIG_NUMBER;
    const FVector ViewDirection = ViewRotation.Vector();
    const float SearchDistance = InteractionDistance + 75.0f;

    for (TActorIterator<ACCCollectibleItem> It(GetWorld()); It; ++It)
    {
        ACCCollectibleItem* Item = *It;
        if (!Item || !Item->CanBePickedUp())
        {
            continue;
        }

        const FVector ToItem = Item->GetActorLocation() - ViewLocation;
        const float Distance = ToItem.Size();
        if (Distance > SearchDistance || Distance <= UE_SMALL_NUMBER)
        {
            continue;
        }

        const float Facing = FVector::DotProduct(ViewDirection, ToItem / Distance);
        // Very close cargo remains interactable even when it is beside or just
        // behind the camera after a physics drop.
        const bool bIsVeryClose = Distance <= 250.0f;
        if (!bIsVeryClose && Facing < 0.1f)
        {
            continue;
        }

        const float Score = (bIsVeryClose ? 2.5f : Facing * 2.0f) - Distance / SearchDistance;
        if (Score > BestScore)
        {
            BestScore = Score;
            BestCandidate = Item;
        }
    }

    return BestCandidate;
}

bool ACCursedCargoCharacter::IsInteractionCandidateValid(const AActor* Candidate) const
{
    if (!Candidate || Candidate == this)
    {
        return false;
    }

    const float AllowedDistance = InteractionDistance + 75.0f;
    if (FVector::DistSquared(GetActorLocation(), Candidate->GetActorLocation()) > FMath::Square(AllowedDistance))
    {
        return false;
    }

    FHitResult Hit;
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CursedCargoInteractionValidation), false, this);
    const FVector Start = GetPawnViewLocation();
    const FVector End = Candidate->GetActorLocation();
    const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams);
    return !bHit || Hit.GetActor() == Candidate;
}

void ACCursedCargoCharacter::ResetDamageCooldown()
{
    bDamageCooldownActive = false;
}

void ACCursedCargoCharacter::ApplySprintState(bool bNewSprinting)
{
    bIsSprinting = bNewSprinting;
    GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;

    if (HasAuthority())
    {
        ForceNetUpdate();
    }
}
