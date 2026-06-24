#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CCMonster.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class ECCMonsterState : uint8
{
    Patrolling,
    Chasing
};

UCLASS(Blueprintable)
class CURSEDCARGO_API ACCMonster : public ACharacter
{
    GENERATED_BODY()

public:
    ACCMonster();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void ReactToNoise(const FVector& NoiseLocation, float Loudness, AActor* NoiseInstigator);

    UFUNCTION(BlueprintPure, Category = "Monster")
    ECCMonsterState GetMonsterState() const { return MonsterState; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype")
    TObjectPtr<UStaticMeshComponent> PrototypeBody;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster", meta = (ClampMin = "100.0"))
    float PatrolRadius = 450.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster", meta = (ClampMin = "100.0"))
    float HearingRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster", meta = (ClampMin = "0.0"))
    float PatrolSpeed = 170.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster", meta = (ClampMin = "0.0"))
    float ChaseSpeed = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster", meta = (ClampMin = "0.1"))
    float ChaseDuration = 7.0f;

    UPROPERTY(ReplicatedUsing = OnRep_MonsterState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Monster")
    ECCMonsterState MonsterState = ECCMonsterState::Patrolling;

    UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Monster")
    TObjectPtr<AActor> ChaseTarget;

    UFUNCTION()
    void OnRep_MonsterState();

    UFUNCTION(BlueprintImplementableEvent, Category = "Monster")
    void OnMonsterStateChanged(ECCMonsterState NewState);

private:
    FVector SpawnOrigin = FVector::ZeroVector;
    FVector PatrolTarget = FVector::ZeroVector;
    float ChaseTimeRemaining = 0.0f;
    float PatrolPauseRemaining = 0.0f;

    void SelectPatrolTarget();
    void StopChasing();
    void MoveToward(const FVector& Destination, float Speed, float DeltaSeconds);
};

