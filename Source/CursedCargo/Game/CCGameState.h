#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CCGameState.generated.h"

UENUM(BlueprintType)
enum class ECCMissionState : uint8
{
    Active,
    Succeeded,
    Failed
};

UCLASS()
class CURSEDCARGO_API ACCGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ACCGameState();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void FailMission();
    void CompleteMission();

    UFUNCTION(BlueprintPure, Category = "Mission")
    int32 GetRemainingSeconds() const { return RemainingSeconds; }

    UFUNCTION(BlueprintPure, Category = "Mission")
    ECCMissionState GetMissionState() const { return MissionState; }

    UFUNCTION(BlueprintPure, Category = "Mission")
    int32 GetTargetMoney() const { return TargetMoney; }

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission", meta = (ClampMin = "10"))
    int32 MissionDurationSeconds = 180;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission", meta = (ClampMin = "1"))
    int32 TargetMoney = 300;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Mission")
    int32 RemainingSeconds = 180;

    UPROPERTY(ReplicatedUsing = OnRep_MissionState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Mission")
    ECCMissionState MissionState = ECCMissionState::Active;

    UFUNCTION()
    void OnRep_MissionState();

    UFUNCTION(BlueprintImplementableEvent, Category = "Mission")
    void OnMissionStateChanged(ECCMissionState NewState);

private:
    float SecondAccumulator = 0.0f;
};
