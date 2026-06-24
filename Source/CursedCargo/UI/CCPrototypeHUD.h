#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CCPrototypeHUD.generated.h"

UCLASS()
class CURSEDCARGO_API ACCPrototypeHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

private:
    int32 FindTeamMoney() const;
    void DrawCrosshair();
};
