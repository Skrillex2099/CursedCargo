#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CCursedCargoCharacter.generated.h"

class ACCCollectibleItem;
class UCameraComponent;
class USceneComponent;
class USpringArmComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class CURSEDCARGO_API ACCursedCargoCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ACCursedCargoCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void TryCarryItem(ACCCollectibleItem* Item);

    UFUNCTION(BlueprintPure, Category = "Interaction")
    USceneComponent* GetCarryAnchor() const { return CarryAnchor; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    ACCCollectibleItem* GetCarriedItem() const { return CarriedItem; }

    UFUNCTION(BlueprintPure, Category = "Interaction")
    bool HasCargoInReach() const;

    UFUNCTION(BlueprintPure, Category = "Interaction")
    ACCCollectibleItem* GetRelevantCargo() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    int32 GetHealth() const { return Health; }

    UFUNCTION(BlueprintPure, Category = "Movement")
    float GetStaminaPercent() const { return MaxStamina > 0.0f ? Stamina / MaxStamina : 0.0f; }

    UFUNCTION(BlueprintPure, Category = "Movement")
    bool IsSprinting() const { return bIsSprinting; }

    void ReceiveMonsterHit();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    /** Temporary visible body used until the final skeletal character is assigned. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype")
    TObjectPtr<UStaticMeshComponent> PrototypeBody;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    TObjectPtr<USceneComponent> CarryAnchor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "50.0"))
    float InteractionDistance = 550.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0"))
    float DropForwardOffset = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0"))
    float ThrowImpulse = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
    FName CarrySocketName = TEXT("hand_rSocket");

    UPROPERTY(ReplicatedUsing = OnRep_CarriedItem, VisibleInstanceOnly, BlueprintReadOnly, Category = "Interaction")
    TObjectPtr<ACCCollectibleItem> CarriedItem;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1"))
    int32 MaxHealth = 3;

    UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleInstanceOnly, BlueprintReadOnly, Category = "Health")
    int32 Health = 3;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
    float WalkSpeed = 350.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
    float SprintSpeed = 600.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "1.0"))
    float MaxStamina = 100.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
    float StaminaDrainPerSecond = 25.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
    float StaminaRecoveryPerSecond = 18.0f;

    UPROPERTY(ReplicatedUsing = OnRep_Stamina, VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement")
    float Stamina = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_IsSprinting, VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement")
    bool bIsSprinting = false;

    UFUNCTION()
    void OnRep_CarriedItem();

    UFUNCTION()
    void OnRep_Health();

    UFUNCTION()
    void OnRep_Stamina();

    UFUNCTION()
    void OnRep_IsSprinting();

    UFUNCTION(BlueprintImplementableEvent, Category = "Health")
    void OnHealthChanged(int32 NewHealth);

    UFUNCTION(Server, Reliable)
    void ServerTryInteract(AActor* Candidate);

    UFUNCTION(Server, Reliable)
    void ServerDropCarriedItem();

    UFUNCTION(Server, Reliable)
    void ServerSetSprinting(bool bNewSprinting);

private:
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void StartSprinting();
    void StopSprinting();
    void HandleInteractPressed();
    void DropCarriedItem();

    AActor* FindFocusedInteractable() const;
    bool IsInteractionCandidateValid(const AActor* Candidate) const;
    void ResetDamageCooldown();
    void ApplySprintState(bool bNewSprinting);

    bool bDamageCooldownActive = false;
    FTimerHandle DamageCooldownTimer;
    float RunningNoiseAccumulator = 0.0f;
};
