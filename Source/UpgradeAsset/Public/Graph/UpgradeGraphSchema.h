#pragma once

#include "CoreMinimal.h"
#include "SGraphPanel.h"
#include "EdGraph/EdGraph.h"
#include "ScopedTransaction.h"
#include "EdGraph/EdGraphPin.h"
#include "Graph/Node/UpgradeAssetGraphNode.h"
#include "Graph/Slate/UpgradeAssetConnectionDrawingPolicy.h"
#include "Framework/Application/SlateApplication.h"
#include "UpgradeGraphSchema.generated.h"

class UUpgradeNodeDataAsset;

UCLASS()
class UUpgradeGraphSchema : public UEdGraphSchema {
    GENERATED_BODY()

public:
    virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
    virtual void GetGraphContextActions(FGraphContextMenuBuilder& contextMenuBuilder) const override;
    virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InGridLineStyle, int32 InActivePinStyle, float InZoomAmount, const FSlateRect& InViewerRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const override;
    virtual bool TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const override;
};

USTRUCT()
struct FNewNodeAction : public FEdGraphSchemaAction {
    GENERATED_BODY()

public:
    FNewNodeAction() {}
    FNewNodeAction(FText inNodeCategory, FText inMenuDesc, FText inToolTip, const int32 inGrouping)
        : FEdGraphSchemaAction(inNodeCategory, inMenuDesc, inToolTip, inGrouping) {}

    virtual UEdGraphNode* PerformAction(UEdGraph* parentGraph, UEdGraphPin* fromPin, const FVector2D location, bool bSelectNewNode = true);
};