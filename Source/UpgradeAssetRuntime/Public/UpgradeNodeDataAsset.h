#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UpgradeNodeCurrencyDataAsset.h"
#include "UpgradeNodeDataAsset.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIconChangedSignature, UTexture2D*, newIcon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBorderChangedSignature, UTexture2D*, newIcon);

UCLASS(Blueprintable, BlueprintType)
class UPGRADEASSETRUNTIME_API UUpgradeNodeDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

// Function Declarations
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UPROPERTY(BlueprintAssignable, Category = "Node Events")
	FOnIconChangedSignature OnIconChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Node Events")
	FOnBorderChangedSignature OnBorderChanged;
	
public:
	UFUNCTION(BlueprintSetter)
	void SetIcon(UTexture2D* newIcon); // Programmable, internal only.

	UFUNCTION(BlueprintSetter)
	void SetBorder(UTexture2D* newIcon); // Internal only.

public:
	// The name that is displayed in-game.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Details")
	FText nodeName = FText::FromString(TEXT("UNNAMED NODE"));

	// Use <StatValue>TEXT</> to make stats appear green
	// Use <StatName>TEXT</> to make stats appear yellow.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Details",  meta=(MultiLine="true"))
	FText nodeDescription = FText::FromString(TEXT("Node Description."));

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Details")
	UUpgradeNodeDataAsset* nodeDetailsParent = nullptr;

	// The icon shown representing the node.
	UPROPERTY(BlueprintReadWrite, BlueprintSetter = SetIcon, EditDefaultsOnly, Category="Node|Details")
	UTexture2D* nodeIcon = nullptr;
    
	// The border drawn around the node.
	UPROPERTY(BlueprintReadWrite, BlueprintSetter = SetBorder, EditDefaultsOnly, Category="Node|Details")
	UTexture2D* nodeBorder = nullptr;
    
	// This symbol will appear in the bottom left of the icon. Leave blank to disable.
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Details")
	UTexture2D* nodeSymbol = nullptr;

	// How many times can this be upgraded?
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Details")
	int32 maxLevel = 1;

	// Require all upgrades to be processed before we can unlock next?
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Details")
	bool bFullyUpgradeToProceed = false;

	// What type of currency do we accept?
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Buying")
	UUpgradeNodeCurrencyDataAsset* nodeBuyingCurrency = nullptr;

	// What should it cost to upgrade the node?
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Buying")
	float upgradeCost = 0;

	// How much should this increase?
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Buying")
	double upgradeCostIncrease;

	// The Stat ID?
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Stat")
	FString statId = TEXT("");

	// How it increases said stat?
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Stat", meta=(MultiLine="true"))
	double statValue = 0.0;

	// Prefix for the stat, like "$1.0 Extra"
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Stat")
	FString statPrefix = TEXT("");

	// Suffix like "10%"
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Stat")
	FString statSuffix = TEXT("");

	// Is it special?
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Node|Details")
	bool isSpecialNode = false;
};
