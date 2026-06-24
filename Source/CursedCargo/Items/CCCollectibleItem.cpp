#include "Items/CCCollectibleItem.h"

#include "AI/CCMonster.h"
#include "Characters/CCursedCargoCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ACCCollectibleItem::ACCCollectibleItem()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    SetReplicateMovement(true);

    CargoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CargoMesh"));
    SetRootComponent(CargoMesh);
    CargoMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    CargoMesh->SetSimulatePhysics(true);
    CargoMesh->SetIsReplicated(true);
    CargoMesh->SetNotifyRigidBodyCollision(true);
    CargoMesh->SetRelativeScale3D(FVector(0.4f));
    CargoMesh->OnComponentHit.AddDynamic(this, &ACCCollectibleItem::HandleCargoHit);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        CargoMesh->SetStaticMesh(CubeMesh.Object);
    }
}

void ACCCollectibleItem::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        CurrentDurability = MaxDurability;
        GetWorldTimerManager().SetTimer(
            ImpactDamageGraceTimer,
            this,
            &ACCCollectibleItem::EnableImpactDamage,
            1.25f,
            false);
    }
}

void ACCCollectibleItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACCCollectibleItem, CurrentDurability);
    DOREPLIFETIME(ACCCollectibleItem, Carrier);
}

bool ACCCollectibleItem::CanInteract_Implementation(AActor* Interactor) const
{
    return Interactor != nullptr && CanBePickedUp();
}

void ACCCollectibleItem::Interact_Implementation(AActor* Interactor)
{
    ACCursedCargoCharacter* Character = Cast<ACCursedCargoCharacter>(Interactor);
    if (Character && HasAuthority())
    {
        Character->TryCarryItem(this);
    }
}

bool ACCCollectibleItem::CanBePickedUp() const
{
    return Carrier == nullptr && !IsActorBeingDestroyed();
}

int32 ACCCollectibleItem::GetCargoValue() const
{
    const float DurabilityRatio = GetDurabilityPercent();
    const float ValueMultiplier = FMath::Lerp(0.25f, 1.0f, DurabilityRatio);
    return FMath::RoundToInt(static_cast<float>(CargoValue) * ValueMultiplier);
}

float ACCCollectibleItem::GetDurabilityPercent() const
{
    return MaxDurability > 0.0f ? FMath::Clamp(CurrentDurability / MaxDurability, 0.0f, 1.0f) : 0.0f;
}

void ACCCollectibleItem::ConfigurePrototypeCargo(
    int32 NewValue,
    float NewDurability,
    float NewWeight,
    float NewImpactThreshold)
{
    check(HasAuthority());

    CargoValue = FMath::Max(0, NewValue);
    MaxDurability = FMath::Max(1.0f, NewDurability);
    CurrentDurability = MaxDurability;
    Weight = FMath::Max(0.0f, NewWeight);
    ImpactDamageThreshold = FMath::Max(0.0f, NewImpactThreshold);
    DropNoiseLoudness = FMath::Clamp(Weight / 5.0f, 0.4f, 1.5f);

    const float VisualScale = FMath::Clamp(0.3f + Weight * 0.02f, 0.32f, 0.55f);
    CargoMesh->SetRelativeScale3D(FVector(VisualScale));
    ForceNetUpdate();
}

void ACCCollectibleItem::PickUp(ACCursedCargoCharacter* NewCarrier)
{
    check(HasAuthority());

    if (!NewCarrier || !CanBePickedUp())
    {
        return;
    }

    Carrier = NewCarrier;
    ApplyCarriedState();
    ForceNetUpdate();
    OnPickedUp();
}

void ACCCollectibleItem::Drop(const FVector& DropLocation, const FVector& Impulse)
{
    check(HasAuthority());

    if (!Carrier)
    {
        return;
    }

    ACCursedCargoCharacter* PreviousCarrier = Carrier;
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    Carrier = nullptr;
    SetActorLocation(DropLocation, false, nullptr, ETeleportType::TeleportPhysics);
    ApplyDroppedState();
    CargoMesh->AddImpulse(Impulse, NAME_None, true);
    ForceNetUpdate();
    OnDropped(DropNoiseLoudness);

    for (TActorIterator<ACCMonster> It(GetWorld()); It; ++It)
    {
        It->ReactToNoise(DropLocation, DropNoiseLoudness, PreviousCarrier);
    }
}

void ACCCollectibleItem::OnRep_Carrier()
{
    if (Carrier)
    {
        ApplyCarriedState();
        OnPickedUp();
    }
    else
    {
        ApplyDroppedState();
        OnDropped(DropNoiseLoudness);
    }
}

void ACCCollectibleItem::OnRep_CurrentDurability()
{
    OnDurabilityChanged(GetDurabilityPercent());
}

void ACCCollectibleItem::HandleCargoHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (!HasAuthority() || !bImpactDamageEnabled || Carrier || CurrentDurability <= 0.0f)
    {
        return;
    }

    const float Mass = FMath::Max(1.0f, CargoMesh->GetMass());
    const float ImpactSeverity = NormalImpulse.Size() / Mass;
    if (ImpactSeverity <= ImpactDamageThreshold)
    {
        return;
    }

    const float Damage = (ImpactSeverity - ImpactDamageThreshold) * ImpactDamageScale;
    CurrentDurability = FMath::Max(0.0f, CurrentDurability - Damage);
    ForceNetUpdate();
    OnRep_CurrentDurability();

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            2.5f,
            CurrentDurability <= 0.0f ? FColor::Red : FColor::Yellow,
            FString::Printf(
                TEXT("Cargo damaged: %.0f%% - value $%d"),
                GetDurabilityPercent() * 100.0f,
                GetCargoValue()));
    }
}

void ACCCollectibleItem::ApplyCarriedState()
{
    if (!Carrier || !Carrier->GetCarryAnchor())
    {
        return;
    }

    CargoMesh->SetSimulatePhysics(false);
    CargoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AttachToComponent(
        Carrier->GetCarryAnchor(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void ACCCollectibleItem::ApplyDroppedState()
{
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    CargoMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    CargoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CargoMesh->SetSimulatePhysics(true);
}

void ACCCollectibleItem::EnableImpactDamage()
{
    bImpactDamageEnabled = true;
}
