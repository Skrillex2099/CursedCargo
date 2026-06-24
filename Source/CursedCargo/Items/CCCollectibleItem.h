#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/CCInteractInterface.h"
#include "CCCollectibleItem.generated.h"

class ACCursedCargoCharacter;
class UPrimitiveComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class CURSEDCARGO_API ACCCollectibleItem : public AActor, public ICCInteractInterface
{
    GENERATED_BODY()

public:
    ACCCollectibleItem();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual bool CanInteract_Implementation(AActor* Interactor) const override;
    virtual void Interact_Implementation(AActor* Interactor) override;

    bool CanBePickedUp() const;
    void PickUp(ACCursedCargoCharacter* NewCarrier);
    void Drop(const FVector& DropLocation, const FVector& Impulse);
    void ConfigurePrototypeCargo(int32 NewValue, float NewDurability, float NewWeight, float NewImpactThreshold);

    UFUNCTION(BlueprintPure, Category = "Cargo")
    bool IsCarried() const { return Carrier != nullptr; }

    UFUNCTION(BlueprintPure, Category = "Cargo")
    int32 GetCargoValue() const;

    UFUNCTION(BlueprintPure, Category = "Cargo")
    float GetDurabilityPercent() const;

    UFUNCTION(BlueprintPure, Category = "Cargo")
    ACCursedCargoCharacter* GetCarrier() const { return Carrier; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> CargoMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cargo", meta = (ClampMin = "0"))
    int32 CargoValue = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cargo", meta = (ClampMin = "0.0"))
    float Weight = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cargo", meta = (ClampMin = "1.0"))
    float MaxDurability = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentDurability, VisibleAnywhere, BlueprintReadOnly, Category = "Cargo")
    float CurrentDurability = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cargo", meta = (ClampMin = "0.0"))
    float ImpactDamageThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cargo", meta = (ClampMin = "0.0"))
    float ImpactDamageScale = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cargo", meta = (ClampMin = "0.0"))
    float DropNoiseLoudness = 1.0f;

    UPROPERTY(ReplicatedUsing = OnRep_Carrier, VisibleInstanceOnly, BlueprintReadOnly, Category = "Cargo")
    TObjectPtr<ACCursedCargoCharacter> Carrier;

    UFUNCTION()
    void OnRep_Carrier();

    UFUNCTION()
    void OnRep_CurrentDurability();

    UFUNCTION()
    void HandleCargoHit(
        UPrimitiveComponent* HitComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComponent,
        FVector NormalImpulse,
        const FHitResult& Hit);

    UFUNCTION(BlueprintImplementableEvent, Category = "Cargo")
    void OnPickedUp();

    UFUNCTION(BlueprintImplementableEvent, Category = "Cargo")
    void OnDropped(float NoiseLoudness);

    UFUNCTION(BlueprintImplementableEvent, Category = "Cargo")
    void OnDurabilityChanged(float NewDurabilityPercent);

private:
    void ApplyCarriedState();
    void ApplyDroppedState();
    void EnableImpactDamage();

    bool bImpactDamageEnabled = false;
    FTimerHandle ImpactDamageGraceTimer;
};
