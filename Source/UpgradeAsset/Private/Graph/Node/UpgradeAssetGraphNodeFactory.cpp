#include "Graph/Node/UpgradeAssetGraphNodeFactory.h"

// This is where we enforce node creation, when creating an Upgrade Node type, we will create a new slate widget of class SUpgradeAssetGraphNode with the data of our UpgradeNode.
TSharedPtr<SGraphNode> FUpgradeAssetGraphNodeFactory::CreateNode(UEdGraphNode* Node) const
{
    if (UUpgradeAssetGraphNode* UpgradeNode = Cast<UUpgradeAssetGraphNode>(Node))
    {
        return SNew(SUpgradeAssetGraphNode, UpgradeNode);
    }
    // Should something go wrong, we return nullptr so nothing else can go wrong.
    return nullptr;
}
