#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UpgradeNodeCurrencyDataAsset.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UPGRADEASSETRUNTIME_API UUpgradeNodeCurrencyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	/** The name that is displayed in-game. */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	FText CurrencyName;

	/** The icon shown representing the currency. */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default", meta=(MultiLine="true"))
	TObjectPtr<UTexture2D> CurrencyIcon;

	/** A prefix, leave blank if none. For example, a currency with a prefix of $ would show as $1000. */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	FString CurrencyPrefix;
};
