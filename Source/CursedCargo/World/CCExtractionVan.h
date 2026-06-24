#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CCExtractionVan.generated.h"

class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class CURSEDCARGO_API ACCExtractionVan : public AActor
{
    GENERATED_BODY()

public:
    ACCExtractionVan();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure, Category = "Extraction")
    int32 GetTeamMoney() const { return TeamMoney; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> VanMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> ExtractionPad;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> ExtractionZone;

    UPROPERTY(ReplicatedUsing = OnRep_TeamMoney, VisibleInstanceOnly, BlueprintReadOnly, Category = "Extraction")
    int32 TeamMoney = 0;

    UFUNCTION()
    void HandleExtractionOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComponent,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnRep_TeamMoney();

    UFUNCTION(BlueprintImplementableEvent, Category = "Extraction")
    void OnCargoExtracted(int32 CargoValue, int32 NewTeamMoney);

    UFUNCTION(BlueprintImplementableEvent, Category = "Extraction")
    void OnTeamMoneyChanged(int32 NewTeamMoney);
};
