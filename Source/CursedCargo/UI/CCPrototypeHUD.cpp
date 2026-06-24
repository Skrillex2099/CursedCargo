#include "UI/CCPrototypeHUD.h"

#include "Characters/CCursedCargoCharacter.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Game/CCGameState.h"
#include "Items/CCCollectibleItem.h"
#include "World/CCExtractionVan.h"

void ACCPrototypeHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas || !GEngine)
    {
        return;
    }

    DrawCrosshair();

    const ACCursedCargoCharacter* Character = Cast<ACCursedCargoCharacter>(GetOwningPawn());
    const ACCGameState* Mission = GetWorld() ? GetWorld()->GetGameState<ACCGameState>() : nullptr;
    const int32 TeamMoney = FindTeamMoney();

    DrawText(
        FString::Printf(TEXT("TEAM MONEY: $%d / $%d"), TeamMoney, Mission ? Mission->GetTargetMoney() : 300),
        FLinearColor(0.2f, 1.0f, 0.35f),
        30.0f,
        30.0f,
        GEngine->GetMediumFont(),
        1.2f);

    DrawText(
        TEXT("WASD Move   |   Shift Sprint   |   E Interact"),
        FLinearColor(0.8f, 0.8f, 0.8f),
        30.0f,
        62.0f,
        GEngine->GetSmallFont(),
        1.0f);

    const int32 RemainingSeconds = Mission ? Mission->GetRemainingSeconds() : 0;
    DrawText(
        FString::Printf(TEXT("TIME: %02d:%02d"), RemainingSeconds / 60, RemainingSeconds % 60),
        RemainingSeconds <= 30 ? FLinearColor::Red : FLinearColor::White,
        30.0f,
        88.0f,
        GEngine->GetMediumFont(),
        1.1f);

    if (!Character)
    {
        return;
    }

    DrawText(
        FString::Printf(TEXT("HEALTH: %d/3"), Character->GetHealth()),
        Character->GetHealth() <= 1 ? FLinearColor::Red : FLinearColor::White,
        30.0f,
        116.0f,
        GEngine->GetMediumFont(),
        1.1f);

    const int32 StaminaPercent = FMath::RoundToInt(Character->GetStaminaPercent() * 100.0f);
    DrawText(
        FString::Printf(TEXT("STAMINA: %d%%"), StaminaPercent),
        StaminaPercent <= 20 ? FLinearColor::Red : FLinearColor(0.25f, 0.75f, 1.0f),
        30.0f,
        144.0f,
        GEngine->GetMediumFont(),
        1.1f);

    if (const ACCCollectibleItem* Cargo = Character->GetRelevantCargo())
    {
        const int32 DurabilityPercent = FMath::RoundToInt(Cargo->GetDurabilityPercent() * 100.0f);
        DrawText(
            FString::Printf(TEXT("CARGO: $%d   DURABILITY: %d%%"), Cargo->GetCargoValue(), DurabilityPercent),
            DurabilityPercent <= 25 ? FLinearColor::Red : FLinearColor(1.0f, 0.75f, 0.1f),
            30.0f,
            172.0f,
            GEngine->GetMediumFont(),
            1.0f);
    }

    if (Mission && Mission->GetMissionState() != ECCMissionState::Active)
    {
        const bool bSucceeded = Mission->GetMissionState() == ECCMissionState::Succeeded;
        const FString ResultText = bSucceeded ? TEXT("MISSION COMPLETE") : TEXT("MISSION FAILED");
        float ResultWidth = 0.0f;
        float ResultHeight = 0.0f;
        GetTextSize(ResultText, ResultWidth, ResultHeight, GEngine->GetMediumFont(), 2.0f);
        DrawText(
            ResultText,
            bSucceeded ? FLinearColor(0.2f, 1.0f, 0.35f) : FLinearColor::Red,
            (Canvas->ClipX - ResultWidth) * 0.5f,
            Canvas->ClipY * 0.35f,
            GEngine->GetMediumFont(),
            2.0f);
    }

    FString Prompt;
    FLinearColor PromptColor = FLinearColor::White;

    if (Character->GetCarriedItem())
    {
        Prompt = TEXT("[E] DROP CARGO");
        PromptColor = FLinearColor(1.0f, 0.75f, 0.1f);
    }
    else if (Character->HasCargoInReach())
    {
        Prompt = TEXT("[E] PICK UP CARGO");
        PromptColor = FLinearColor(0.2f, 1.0f, 0.35f);
    }

    if (!Prompt.IsEmpty())
    {
        float TextWidth = 0.0f;
        float TextHeight = 0.0f;
        GetTextSize(Prompt, TextWidth, TextHeight, GEngine->GetMediumFont(), 1.15f);
        DrawText(
            Prompt,
            PromptColor,
            (Canvas->ClipX - TextWidth) * 0.5f,
            Canvas->ClipY * 0.62f,
            GEngine->GetMediumFont(),
            1.15f);
    }
}

int32 ACCPrototypeHUD::FindTeamMoney() const
{
    if (!GetWorld())
    {
        return 0;
    }

    for (TActorIterator<ACCExtractionVan> It(GetWorld()); It; ++It)
    {
        return It->GetTeamMoney();
    }

    return 0;
}

void ACCPrototypeHUD::DrawCrosshair()
{
    const float CenterX = Canvas->ClipX * 0.5f;
    const float CenterY = Canvas->ClipY * 0.5f;
    const FLinearColor Color(1.0f, 1.0f, 1.0f, 0.85f);

    DrawLine(CenterX - 7.0f, CenterY, CenterX + 7.0f, CenterY, Color, 1.5f);
    DrawLine(CenterX, CenterY - 7.0f, CenterX, CenterY + 7.0f, Color, 1.5f);
}
