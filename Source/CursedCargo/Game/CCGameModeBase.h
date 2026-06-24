#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CCGameModeBase.generated.h"

UCLASS()
class CURSEDCARGO_API ACCGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACCGameModeBase();

    virtual void BeginPlay() override;
    virtual void RestartPlayer(AController* NewPlayer) override;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Prototype")
    bool bSpawnPrototypeActors = true;

private:
    void SpawnPrototypeActors();
    int32 PlayerSpawnSequence = 0;
};
