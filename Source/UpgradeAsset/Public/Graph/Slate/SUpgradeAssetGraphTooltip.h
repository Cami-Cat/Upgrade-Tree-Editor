#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Graph/Node/UpgradeAssetGraphNode.h"

class SUpgradeAssetGraphTooltip : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SUpgradeAssetGraphTooltip) {}
    SLATE_END_ARGS()
    
    TWeakObjectPtr<UUpgradeAssetGraphNode> BoundNode;

    void Construct(const FArguments& InArgs, UEdGraphNode* InNode);
    FText GetTooltipNodeName() const; 
    FText GetTooltipStatId() const;
};