#pragma once

#include "EdGraphUtilities.h"
#include "Graph/Node/UpgradeAssetGraphNode.h"
#include "Graph/Slate/SUpgradeAssetGraphNode.h"

class FUpgradeAssetGraphNodeFactory : public FGraphPanelNodeFactory {
public:
    virtual TSharedPtr<SGraphNode> CreateNode(UEdGraphNode* Node) const override;
};
