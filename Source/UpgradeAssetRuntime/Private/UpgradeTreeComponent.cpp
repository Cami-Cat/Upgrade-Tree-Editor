#include "UpgradeTreeComponent.h"
#include "UpgradeAsset.h"

UUpgradeTrackerComponent::UUpgradeTrackerComponent() {
    PrimaryComponentTick.bCanEverTick = false;
}

int32 UUpgradeTrackerComponent::GetUpgradeCount(FGuid NodeID) const
{
    if (const FUpgradeNodeRuntimeProgress* Progress = UpgradeProgressMap.Find(NodeID)) {
        return Progress->UpgradeCount;
    }
    return 0; 
}

bool UUpgradeTrackerComponent::IsNodeUnlocked(FGuid NodeID) const {
    if (const FUpgradeNodeRuntimeProgress* Progress = UpgradeProgressMap.Find(NodeID)) {
        return Progress->bIsUnlocked;
    }
    return false;
}

bool UUpgradeTrackerComponent::IsNodeMaxLevel(FGuid NodeID) const {
    if (!UpgradeTree) { return false; }

    const FUpgradeNodeInfo* NodeInfo = UpgradeTree->NodeData.FindByPredicate(
        [&NodeID](const FUpgradeNodeInfo& Node) { return Node.NodeId == NodeID; });

    if (!NodeInfo || !NodeInfo->attachedData) { return false; }

    return GetUpgradeCount(NodeID) >= NodeInfo->attachedData->maxLevel;
}

void UUpgradeTrackerComponent::UnlockNode(FGuid NodeID) {
    FUpgradeNodeRuntimeProgress& Progress = UpgradeProgressMap.FindOrAdd(NodeID);
    Progress.bIsUnlocked = true;
}

void UUpgradeTrackerComponent::SetUpgradeProgress(FGuid NodeID, int32 NewUpgradeCount, bool bNewIsUnlocked) {
    FUpgradeNodeRuntimeProgress& Progress = UpgradeProgressMap.FindOrAdd(NodeID);
    Progress.UpgradeCount = NewUpgradeCount;
    Progress.bIsUnlocked = bNewIsUnlocked;
}

bool UUpgradeTrackerComponent::TryIncrementUpgrade(FGuid NodeID, UPARAM(ref) TMap<FName, int32>& CurrencyMap)
{
    if (!UpgradeTree) return false;

    // Get the node data by finding through a predicate. This works by saying, through the lambda:
    // Look at the &NodeID (our first function param), this is outside your scope but I'm allowing you to see it.
    // We then define what it's iterating over, since we're using FindByPredicate we're looping over a TArray of FUpgradeNodeInfo*
    // We pass in a copy so it doesn't make more memory, but we don't need a pointer either.
    // If the nodeId is then equal to our parsed NodeID, that is the node we want, and it has found the matched element.
    const FUpgradeNodeInfo* NodeInfo = UpgradeTree->NodeData.FindByPredicate(
        [&NodeID](const FUpgradeNodeInfo& Node) { return Node.NodeId == NodeID; });

    if (!NodeInfo || !NodeInfo->attachedData) { return false; }

    if (IsNodeMaxLevel(NodeID)) { return false; }

    // Get the current cost for a single upgrade at its current level.
    float UpgradeCost = GetCurrentUpgradeCost(NodeID);
    // Perform the same checks as in other functions to get whether the currency exists.
    FName TargetCurrency = NodeInfo->attachedData->nodeBuyingCurrency->CurrencyName;
    int32* BalancePtr = CurrencyMap.Find(TargetCurrency);
    // Check whether the user can afford the currency, or whether the user actually has the currency.
    if (!BalancePtr || *BalancePtr < UpgradeCost) { return false; }
    // Then deduct the cost of the upgrade from the user's balance.
    *BalancePtr -= FMath::FloorToInt(UpgradeCost);

    // Update the progress by an increment of one.
    FUpgradeNodeRuntimeProgress& Progress = UpgradeProgressMap.FindOrAdd(NodeID);
    Progress.UpgradeCount++;
    Progress.bIsUnlocked = true;

    // Define bMetUnlockNodeRequirements.
    bool bMetNodeUnlockRequirements = true;

    // Emit delegates at different stages.
    if (OnNodeProgressChanged.IsBound()) {
        OnNodeProgressChanged.Broadcast(*NodeInfo);
    }
    
    if (Progress.UpgradeCount >= NodeInfo->attachedData->maxLevel) {
        if (OnNodeMaxedOut.IsBound()) {
            OnNodeMaxedOut.Broadcast(*NodeInfo);
        }
    }
    
    if (!NodeInfo->attachedData->bFullyUpgradeToProceed) { bMetNodeUnlockRequirements = false; }
    if (bMetNodeUnlockRequirements) { OnUnlockNodeRequirementsMet.Broadcast(*NodeInfo); }

    return true;
}

FUpgradeBuyDef UUpgradeTrackerComponent::GetTotalNumBuyable(FGuid NodeID, const TMap<FName, int32>& CurrencyMap) {
    FUpgradeBuyDef TotalBuyDef{};

    if (!UpgradeTree) { return TotalBuyDef; }
    
    const FUpgradeNodeInfo* NodeInfo = UpgradeTree->NodeData.FindByPredicate(
        [&NodeID](const FUpgradeNodeInfo& Node) { return Node.NodeId == NodeID; }
    );

    if (!NodeInfo || !NodeInfo->attachedData) { return TotalBuyDef; }
    if (IsNodeMaxLevel(NodeID)) { return TotalBuyDef; } 

    FName targetCurrencyName = NodeInfo->attachedData->nodeBuyingCurrency->CurrencyName;
    const int32 TotalOwnedCurrency = CurrencyMap.FindRef(targetCurrencyName);

    // We establish all of our important quick-access variables.

    float UpgradeCost = NodeInfo->attachedData->upgradeCost;
    float Increment = NodeInfo->attachedData->upgradeCostIncrease;
    int32 MaxUpgradeCount = NodeInfo->attachedData->maxLevel;
    int32 CurrentLevel = GetUpgradeCount(NodeID);
    int32 UpgradesRemaining = FMath::Max(0, MaxUpgradeCount - CurrentLevel);

    if (UpgradesRemaining <= 0) { return TotalBuyDef; }
    
    int32 CalculatedBuyable = 0;

    if (FMath::IsNearlyZero(Increment)) {
        // There is no flat increase, but it also has a cost.
        if (UpgradeCost > 0.0f) {
            CalculatedBuyable = FMath::FloorToInt(TotalOwnedCurrency / UpgradeCost);
        }
        else { // This means the upgrade is free, therefore we'll buy all of it.
            CalculatedBuyable = UpgradesRemaining;
        }
    }

    
    else {
        switch (NodeInfo->attachedData->upgradeCostIncrementType) {
            case EIncrementType::Linear: {
                // Quadratic formula my NEMESIS...
                double a = Increment;
                double b = (2.0 * UpgradeCost) - Increment;
                double c = -2.0 * TotalOwnedCurrency;
            
                // b^2 - 4ac
                double d = FMath::Pow(b, 2) - (4.0 * a * c);
                if (d >= 0.0) {
                    // (b - sqrt(b^2 - 4ac)) / 2.0 * a
                    double SolvedBuyable = (-b + FMath::Sqrt(d)) / (2.0 * a);
                    CalculatedBuyable = FMath::FloorToInt(SolvedBuyable);
                }
                break;
            }

            case EIncrementType::Exponential: {
                double LogTop = 1.0 + ((TotalOwnedCurrency * (Increment - 1.0)) / UpgradeCost);
                if (LogTop > 0.0) {
                    double SolvedBuyable = FMath::Loge(LogTop) / FMath::Loge(Increment);
                    CalculatedBuyable = FMath::FloorToInt(SolvedBuyable);
                }
                break;
            }

            default: {
                break;
            }
        }
    }

    TotalBuyDef.TotalBuyableAmount = FMath::Clamp(CalculatedBuyable, 0, UpgradesRemaining);
    int32 N = TotalBuyDef.TotalBuyableAmount;

    if (N <= 0) {
        TotalBuyDef.TotalPrice = 0;
    }
    else if (FMath::IsNearlyZero(Increment) || (NodeInfo->attachedData->upgradeCostIncrementType == EIncrementType::Exponential && FMath::IsNearlyEqual(Increment, 1.0f))) {
        TotalBuyDef.TotalPrice = N * FMath::RoundToInt(UpgradeCost);
    }
    else {
        switch (NodeInfo->attachedData->upgradeCostIncrementType)
        {
            case EIncrementType::Linear: {
                // This is some Gaussian maths (WE LOVE GAUSS IN THIS HOUSEHOLD)
                // Basically, the sum of the first and last number in a list multiplied by half the number of total items in the list is equal to the sum of all of them added together.
                TotalBuyDef.TotalPrice = (N / 2.0f) * ((2.0f * UpgradeCost) + ((N - 1) * Increment));
                break;
            }

            case EIncrementType::Exponential: {
                double GeometricSum = UpgradeCost * ((FMath::Pow(Increment, N) - 1.0) / (Increment - 1.0));
                TotalBuyDef.TotalPrice = FMath::RoundToInt(GeometricSum);
                break;
            }

            default: {
                TotalBuyDef.TotalPrice = 0;
                break;
            }
        }
    }

    TotalBuyDef.CurrencyName = targetCurrencyName;
    return TotalBuyDef;
}

FUpgradeBoughtDef UUpgradeTrackerComponent::TryBuyMaxUpgrades(FGuid NodeID, UPARAM(ref) TMap<FName, int32>& CurrencyMap) {
    FUpgradeBoughtDef BoughtDef{};
    if (!UpgradeTree) { return BoughtDef; }
    
    const FUpgradeNodeInfo* NodeInfo = UpgradeTree->NodeData.FindByPredicate(
        [&NodeID](const FUpgradeNodeInfo& Node) { return Node.NodeId == NodeID; }
    );

    if (!NodeInfo || !NodeInfo->attachedData) { return BoughtDef; }
    FUpgradeBuyDef BuyDef = GetTotalNumBuyable(NodeID, CurrencyMap);

    if (BuyDef.TotalBuyableAmount <= 0) { return BoughtDef; }
    FName TargetCurrency = NodeInfo->attachedData->nodeBuyingCurrency->CurrencyName;
    if (int32* BalancePtr = CurrencyMap.Find(TargetCurrency)) {
        *BalancePtr -= BuyDef.TotalPrice;
    }

    
    FUpgradeNodeRuntimeProgress& Progress = UpgradeProgressMap.FindOrAdd(NodeID);
    Progress.UpgradeCount += BuyDef.TotalBuyableAmount;
    Progress.bIsUnlocked = true;

    BoughtDef.TotalCost = BuyDef.TotalPrice;
    BoughtDef.PurchasedAmount = BuyDef.TotalBuyableAmount;
    BoughtDef.CurrencyName = BuyDef.CurrencyName;
    BoughtDef.bBought = true;

    if (OnNodeProgressChanged.IsBound()) {
        OnNodeProgressChanged.Broadcast(*NodeInfo);
    }

    if (Progress.UpgradeCount >= NodeInfo->attachedData->maxLevel) {
        if (OnNodeMaxedOut.IsBound()) {
            OnNodeMaxedOut.Broadcast(*NodeInfo);
        }
    }

    bool bHasMetNodeUnlockRequirements = NodeInfo->attachedData->bFullyUpgradeToProceed;
    if (bHasMetNodeUnlockRequirements && Progress.UpgradeCount >= NodeInfo->attachedData->maxLevel)
    {
        OnUnlockNodeRequirementsMet.Broadcast(*NodeInfo);
    }

    return BoughtDef;
}

FUpgradeNodeRuntimeProgress UUpgradeTrackerComponent::GetUpgradeProgress(FGuid NodeID) { 
    if (FUpgradeNodeRuntimeProgress* Progress = UpgradeProgressMap.Find(NodeID)) {
        return *Progress;
    }
    return FUpgradeNodeRuntimeProgress();
}

float UUpgradeTrackerComponent::GetCurrentUpgradeCost(FGuid NodeID) const
{
    if (!UpgradeTree) { return 0.0f; }

    const FUpgradeNodeInfo* NodeInfo = UpgradeTree->NodeData.FindByPredicate(
        [&NodeID](const FUpgradeNodeInfo& Node) { return Node.NodeId == NodeID; });

    if (!NodeInfo || !NodeInfo->attachedData) { return 0.0f; }

    int32 CurrentLevel = 0;
    if (const FUpgradeNodeRuntimeProgress* ProgressPtr = UpgradeProgressMap.Find(NodeID)) {
        CurrentLevel = ProgressPtr->UpgradeCount;
    }

    if (CurrentLevel >= NodeInfo->attachedData->maxLevel) { return 0.0f; }

    float BaseCost = NodeInfo->attachedData->upgradeCost;
    float Increment = NodeInfo->attachedData->upgradeCostIncrease;

    switch (NodeInfo->attachedData->upgradeCostIncrementType) {
        case EIncrementType::Linear: {
            return BaseCost + (CurrentLevel * Increment);
        }
        case EIncrementType::Exponential: {
            return BaseCost * FMath::Pow(Increment, CurrentLevel);
        }
        default: {
            return BaseCost;
        }
    }
}

bool UUpgradeTrackerComponent::AreParentNodeRequirementsMet(FGuid NodeID) {
    if (!UpgradeTree) { return false; }
    
    const FUpgradeNodeInfo* NodeInfo = UpgradeTree->NodeData.FindByPredicate(
        [&NodeID](const FUpgradeNodeInfo& Node) { return Node.NodeId == NodeID; }
    );

    // There is no info, this cannot return anything.
    if (!NodeInfo) { return false; }

    const TArray<FGuid>& ParentGuids = NodeInfo->NodeParents; 
    
    // There is no data, this cannot be unlocked.
    if (!NodeInfo->attachedData) { return false; }

    // This can be unlocked since it has no parents.
    if (ParentGuids.Num() == 0) { return true; }

    // We then check each parent for any upgrades.
    int32 UpgradedParentsCount = 0;
    for (const FGuid& ParentId : ParentGuids) {
        if (GetUpgradeCount(ParentId) > 0) {
            UpgradedParentsCount++;
        }
    }

    switch (NodeInfo->attachedData->ParentRequirement) {
        case EUpgradeParentRequirement::All: {
            return (UpgradedParentsCount == ParentGuids.Num());
        }
        case EUpgradeParentRequirement::Any: {
            return (UpgradedParentsCount > 0);
        }
        default: {
            return false;
        }
    }
}

FUpgradeStat UUpgradeTrackerComponent::GetUpgradeStatAtLevel(FGuid NodeID, int32 inLevel) {
    FUpgradeStat UpgradeStat{};
    if (!UpgradeTree) { return UpgradeStat; }
    
    const FUpgradeNodeInfo* NodeInfo = UpgradeTree->NodeData.FindByPredicate(
        [&NodeID](const FUpgradeNodeInfo& Node) { return Node.NodeId == NodeID; }
    );

    // There is no info, this cannot return anything.
    if (!NodeInfo || !NodeInfo->attachedData) { return UpgradeStat; }   
    
    UpgradeStat = NodeInfo->attachedData->StatDat;
    
    float BaseValue = UpgradeStat.statValue;
    float IncrementValue = UpgradeStat.statIncrement;

    UpgradeStat.statValue = BaseValue + (inLevel * IncrementValue);

    return UpgradeStat;
}