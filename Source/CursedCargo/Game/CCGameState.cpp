#include "Game/CCGameState.h"

#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

ACCGameState::ACCGameState()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
}

void ACCGameState::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        RemainingSeconds = MissionDurationSeconds;
    }
}

void ACCGameState::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority() || MissionState != ECCMissionState::Active)
    {
        return;
    }

    SecondAccumulator += DeltaSeconds;
    while (SecondAccumulator >= 1.0f)
    {
        SecondAccumulator -= 1.0f;
        RemainingSeconds = FMath::Max(0, RemainingSeconds - 1);
        ForceNetUpdate();

        if (RemainingSeconds == 0)
        {
            FailMission();
            break;
        }
    }
}

void ACCGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACCGameState, RemainingSeconds);
    DOREPLIFETIME(ACCGameState, MissionState);
}

void ACCGameState::FailMission()
{
    if (!HasAuthority() || MissionState == ECCMissionState::Failed)
    {
        return;
    }

    MissionState = ECCMissionState::Failed;
    ForceNetUpdate();
    OnMissionStateChanged(MissionState);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, TEXT("MISSION FAILED"));
    }
}

void ACCGameState::CompleteMission()
{
    if (!HasAuthority() || MissionState != ECCMissionState::Active)
    {
        return;
    }

    MissionState = ECCMissionState::Succeeded;
    ForceNetUpdate();
    OnMissionStateChanged(MissionState);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Green, TEXT("MISSION COMPLETE"));
    }
}

void ACCGameState::OnRep_MissionState()
{
    OnMissionStateChanged(MissionState);
}
