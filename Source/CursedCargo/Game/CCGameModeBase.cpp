#include "Game/CCGameModeBase.h"

#include "AI/CCMonster.h"
#include "Characters/CCursedCargoCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Game/CCGameState.h"
#include "GameFramework/PlayerStart.h"
#include "Items/CCCollectibleItem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/CCPrototypeHUD.h"
#include "World/CCExtractionVan.h"

ACCGameModeBase::ACCGameModeBase()
{
    DefaultPawnClass = ACCursedCargoCharacter::StaticClass();
    HUDClass = ACCPrototypeHUD::StaticClass();
    GameStateClass = ACCGameState::StaticClass();
}

void ACCGameModeBase::RestartPlayer(AController* NewPlayer)
{
    if (!NewPlayer)
    {
        return;
    }

    const AActor* BaseStart = UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass());
    if (!BaseStart)
    {
        Super::RestartPlayer(NewPlayer);
        return;
    }

    const int32 SpawnIndex = PlayerSpawnSequence++;
    FVector SpawnOffset = FVector::ZeroVector;
    if (SpawnIndex > 0)
    {
        constexpr int32 RingPositions = 5;
        const float Angle = static_cast<float>(SpawnIndex - 1) * (2.0f * PI / RingPositions);
        SpawnOffset = FVector(0.0f, FMath::Cos(Angle) * 180.0f, 0.0f);
        SpawnOffset.X = FMath::Sin(Angle) * 180.0f;
    }

    const FTransform SpawnTransform(
        BaseStart->GetActorRotation(),
        BaseStart->GetActorLocation() + SpawnOffset);
    RestartPlayerAtTransform(NewPlayer, SpawnTransform);
}

void ACCGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    if (bSpawnPrototypeActors)
    {
        SpawnPrototypeActors();
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            15.0f,
            FColor::Yellow,
            TEXT("CURSED CARGO PROTOTYPE | WASD: Move | Mouse: Look | E: Pick up / Drop"));
    }
}

void ACCGameModeBase::SpawnPrototypeActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FVector GroundOrigin = FVector::ZeroVector;
    if (const AActor* PlayerStart = UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass()))
    {
        // APlayerStart is positioned at capsule-centre height, not ground height.
        GroundOrigin = PlayerStart->GetActorLocation() - FVector(0.0f, 0.0f, 90.0f);
    }

    const FVector CargoOrigin = GroundOrigin + FVector(0.0f, 0.0f, 150.0f);

    const FActorSpawnParameters SpawnParameters = []
    {
        FActorSpawnParameters Parameters;
        Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        return Parameters;
    }();

    const FVector CargoOffsets[] =
    {
        FVector(250.0f, -120.0f, 0.0f),
        FVector(330.0f, 0.0f, 0.0f),
        FVector(250.0f, 120.0f, 0.0f)
    };

    struct FPrototypeCargoConfig
    {
        int32 Value;
        float Durability;
        float Weight;
        float ImpactThreshold;
    };

    const FPrototypeCargoConfig CargoConfigs[] =
    {
        {100, 100.0f, 4.0f, 420.0f},
        {125, 70.0f, 6.0f, 340.0f},
        {150, 45.0f, 8.0f, 270.0f}
    };

    for (int32 Index = 0; Index < UE_ARRAY_COUNT(CargoOffsets); ++Index)
    {
        ACCCollectibleItem* Cargo = World->SpawnActor<ACCCollectibleItem>(
            ACCCollectibleItem::StaticClass(),
            CargoOrigin + CargoOffsets[Index],
            FRotator::ZeroRotator,
            SpawnParameters);

        if (Cargo)
        {
            const FPrototypeCargoConfig& Config = CargoConfigs[Index];
            Cargo->ConfigurePrototypeCargo(
                Config.Value,
                Config.Durability,
                Config.Weight,
                Config.ImpactThreshold);
        }
    }

    World->SpawnActor<ACCExtractionVan>(
        ACCExtractionVan::StaticClass(),
        GroundOrigin + FVector(750.0f, 0.0f, 50.0f),
        FRotator::ZeroRotator,
        SpawnParameters);

    World->SpawnActor<ACCMonster>(
        ACCMonster::StaticClass(),
        GroundOrigin + FVector(550.0f, 550.0f, 110.0f),
        FRotator::ZeroRotator,
        SpawnParameters);
}
