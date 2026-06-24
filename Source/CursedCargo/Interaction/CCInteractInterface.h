#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CCInteractInterface.generated.h"

UINTERFACE(BlueprintType)
class CURSEDCARGO_API UCCInteractInterface : public UInterface
{
    GENERATED_BODY()
};

class CURSEDCARGO_API ICCInteractInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    bool CanInteract(AActor* Interactor) const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
    void Interact(AActor* Interactor);
};

