#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeTreeWidget.generated.h"

class UUpgradeTrackerComponent;

UCLASS()
class UPGRADEASSETRUNTIME_API UUpgradeTreeWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, Category = "Upgrades", meta = (ExposeOnSpawn = "true"))
    UUpgradeTrackerComponent* UpgradeTracker;
};