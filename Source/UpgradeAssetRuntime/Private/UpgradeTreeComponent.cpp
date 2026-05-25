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

    const FUpgradeNodeInfo* LayoutInfo = UpgradeTree->NodeData.FindByPredicate(
        [&NodeID](const FUpgradeNodeInfo& Node) { return Node.NodeId == NodeID; });

    if (!LayoutInfo || !LayoutInfo->attachedData) { return false; }

    return GetUpgradeCount(NodeID) >= LayoutInfo->attachedData->maxLevel;
}

void UUpgradeTrackerComponent::SetUpgradeProgress(FGuid NodeID, int32 NewUpgradeCount, bool bNewIsUnlocked) {
    FUpgradeNodeRuntimeProgress& Progress = UpgradeProgressMap.FindOrAdd(NodeID);
    Progress.UpgradeCount = NewUpgradeCount;
    Progress.bIsUnlocked = bNewIsUnlocked;
}

bool UUpgradeTrackerComponent::TryIncrementUpgrade(FGuid NodeID)
{
    if (!UpgradeTree) return false;

    const FUpgradeNodeInfo* LayoutInfo = UpgradeTree->NodeData.FindByPredicate(
        [&NodeID](const FUpgradeNodeInfo& Node) { return Node.NodeId == NodeID; });

    if (!LayoutInfo || !LayoutInfo->attachedData) { return false; }

    if (IsNodeMaxLevel(NodeID)) { return false; }

    FUpgradeNodeRuntimeProgress& Progress = UpgradeProgressMap.FindOrAdd(NodeID);
    Progress.UpgradeCount++;
    Progress.bIsUnlocked = true;

    bool bMetNodeUnlockRequirements = true;

    if (OnNodeProgressChanged.IsBound()) {
        OnNodeProgressChanged.Broadcast(*LayoutInfo);
    }
    
    if (Progress.UpgradeCount >= LayoutInfo->attachedData->maxLevel) {
        if (OnNodeMaxedOut.IsBound()) {
            OnNodeMaxedOut.Broadcast(*LayoutInfo);
        }
    }
    
    if (!LayoutInfo->attachedData->bFullyUpgradeToProceed) { bMetNodeUnlockRequirements = false; }
    if (bMetNodeUnlockRequirements) { OnUnlockNodeRequirementsMet.Broadcast(*LayoutInfo); }

    return true;
}

FUpgradeNodeRuntimeProgress UUpgradeTrackerComponent::GetUpgradeProgress(FGuid NodeID) { 
    if (FUpgradeNodeRuntimeProgress* Progress = UpgradeProgressMap.Find(NodeID)) {
        return *Progress;
    }
    return FUpgradeNodeRuntimeProgress();
}