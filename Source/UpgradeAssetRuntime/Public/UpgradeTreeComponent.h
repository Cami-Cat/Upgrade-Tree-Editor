#pragma once

#include "CoreMinimal.h"
#include "UpgradeAsset.h"
#include "UpgradeTreeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComponentUpgradeHandler, FUpgradeNodeInfo, Upgrade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComponentUpgradedFullyHandler, FUpgradeNodeInfo, Upgrade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnlockConnectedNodeRequirementsMet, FUpgradeNodeInfo, Upgrade);

UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class UPGRADEASSETRUNTIME_API UUpgradeTrackerComponent : public UActorComponent {
    GENERATED_BODY()

    UUpgradeTrackerComponent();
public:
    // Our upgrade tree data.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade Tree")
    TObjectPtr<UUpgradeAsset> UpgradeTree;
    // When we alter our upgrade progress, we want to announce that.
    UPROPERTY(BlueprintAssignable, Category = "Upgrade Tree|Delegates")
    FOnComponentUpgradeHandler OnNodeProgressChanged;
    // Same with when the node is completely maxed, which allow nodes that require fully-upgrading them to unlock others.
    UPROPERTY(BlueprintAssignable, Category = "Upgrade Tree|Delegates")
    FOnComponentUpgradedFullyHandler OnNodeMaxedOut;
    // Once requirements are met, we can then emit this so other nodes can be unlocked!
    UPROPERTY(BlueprintAssignable, Category = "Upgrade Tree|Delegates")
    FOnUnlockConnectedNodeRequirementsMet OnUnlockNodeRequirementsMet;

private:

    // Savable struct map that stores FGuid (Not the data asset, but the ID pointing to it) and the progress.
    UPROPERTY(EditAnywhere, Category = "Upgrade Tree|Runtime")
    TMap<FGuid, FUpgradeNodeRuntimeProgress> UpgradeProgressMap;

public:

    UFUNCTION(BlueprintPure, Category = "Upgrade Tree|Getters")
    int32 GetUpgradeCount(FGuid NodeID) const;

    UFUNCTION(BlueprintPure, Category = "Upgrade Tree|Getters")
    bool IsNodeUnlocked(FGuid NodeID) const;

    UFUNCTION(BlueprintPure, Category = "Upgrade Tree|Getters")
    bool IsNodeMaxLevel(FGuid NodeID) const;

    UFUNCTION(BlueprintPure, Category = "Upgrade Tree|Getters")
    FUpgradeNodeRuntimeProgress GetUpgradeProgress(FGuid NodeID);

    UFUNCTION(BlueprintCallable, Category = "Upgrade Tree|Setters")
    void SetUpgradeProgress(FGuid NodeID, int32 NewUpgradeCount, bool bNewIsUnlocked);

    UFUNCTION(BlueprintCallable, Category = "Upgrade Tree|Mutators")
    bool TryIncrementUpgrade(FGuid NodeID);

};