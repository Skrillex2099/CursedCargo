#include "World/CCExtractionVan.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Game/CCGameState.h"
#include "Items/CCCollectibleItem.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ACCExtractionVan::ACCExtractionVan()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    VanMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VanMesh"));
    VanMesh->SetupAttachment(SceneRoot);
    VanMesh->SetCollisionProfileName(TEXT("BlockAll"));
    VanMesh->SetRelativeScale3D(FVector(2.5f, 1.5f, 1.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        VanMesh->SetStaticMesh(CubeMesh.Object);
    }

    ExtractionPad = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExtractionPad"));
    ExtractionPad->SetupAttachment(SceneRoot);
    ExtractionPad->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ExtractionPad->SetRelativeLocation(FVector(0.0f, -250.0f, -45.0f));
    ExtractionPad->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.1f));
    if (CubeMesh.Succeeded())
    {
        ExtractionPad->SetStaticMesh(CubeMesh.Object);
    }

    ExtractionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("ExtractionZone"));
    ExtractionZone->SetupAttachment(SceneRoot);
    ExtractionZone->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
    ExtractionZone->SetRelativeLocation(FVector(0.0f, -250.0f, 50.0f));
    ExtractionZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ExtractionZone->SetCollisionResponseToAllChannels(ECR_Ignore);
    ExtractionZone->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
    ExtractionZone->OnComponentBeginOverlap.AddDynamic(this, &ACCExtractionVan::HandleExtractionOverlap);
}

void ACCExtractionVan::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACCExtractionVan, TeamMoney);
}

void ACCExtractionVan::HandleExtractionOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
        return;
    }

    ACCCollectibleItem* Cargo = Cast<ACCCollectibleItem>(OtherActor);
    if (!Cargo || Cargo->IsCarried())
    {
        return;
    }

    const int32 ExtractedValue = Cargo->GetCargoValue();
    TeamMoney += ExtractedValue;
    OnCargoExtracted(ExtractedValue, TeamMoney);
    OnTeamMoneyChanged(TeamMoney);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.0f,
            FColor::Green,
            FString::Printf(TEXT("Cargo extracted: +$%d"), ExtractedValue));
    }

    if (ACCGameState* Mission = GetWorld()->GetGameState<ACCGameState>())
    {
        if (TeamMoney >= Mission->GetTargetMoney())
        {
            Mission->CompleteMission();
        }
    }

    Cargo->Destroy();
    ForceNetUpdate();
}

void ACCExtractionVan::OnRep_TeamMoney()
{
    OnTeamMoneyChanged(TeamMoney);
}
