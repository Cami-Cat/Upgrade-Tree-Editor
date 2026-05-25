#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "SGraphPin.h"
#include "SGraphPanel.h" 
#include "GraphEditorSettings.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Rendering/DrawElements.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Graph/Node/UpgradeAssetGraphNode.h"
#include "Utils/UpgradeTreePalette.h"
#include "Utils/Maths/CamiMaths.h"
#include "UpgradeNodeDataAsset.h"

class SUpgradeAssetGraphNode : public SGraphNode
{
public:
    SLATE_BEGIN_ARGS(SUpgradeAssetGraphNode) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UEdGraphNode* InNode);

    virtual const FSlateBrush* GetNodeBodyBrush() const override;
    virtual const FSlateBrush* GetNodeOutlineBorderBrush() const;
    FSlateColor GetCustomNodeBodyColor();
    FSlateColor GetRimHighlightColor() const;

    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
    
    // We override the renderer here by forcing our own OnPaint method.
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
    virtual void UpdateGraphNode() override;

private:

    // State rules.
    bool bIsHoveringRim = false;
    bool bIsActivelyDraggingWire = false;
    bool bIsSelected = false;

    // Global private members for accessing data.
    FVector2D LiveDragMousePos = FVector2D::ZeroVector;

    // Slate brushes for drawing icons and borders.
    mutable TSharedPtr<FSlateBrush> DynamicNodeIconBrush;
    mutable TSharedPtr<FSlateBrush> DynamicNodeBorderBrush;
};
