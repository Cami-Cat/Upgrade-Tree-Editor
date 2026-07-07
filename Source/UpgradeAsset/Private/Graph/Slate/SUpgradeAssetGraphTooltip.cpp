#include "Graph/Slate/SUpgradeAssetGraphTooltip.h"
    
void SUpgradeAssetGraphTooltip::Construct(const FArguments& InArgs, UEdGraphNode* InNode) {
    UUpgradeAssetGraphNode* CastNode = Cast<UUpgradeAssetGraphNode>(InNode);
    
    if (!CastNode || !CastNode->GetAttachedData()) {
        ChildSlot [ SNew(STextBlock).Text(FText::FromString(TEXT("No Data Available"))) ];
        return;
    }

    UUpgradeNodeDataAsset* Data = CastNode->GetAttachedData();
    BoundNode = CastNode;

    ChildSlot [
        SNew(SBorder)
        .Padding(FMargin(12.0f, 8.0f))
        [
            SNew(SVerticalBox)
                
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 0.0f, 0.0f, 4.0f)
            [
                SNew(STextBlock)
                .Text(this, &SUpgradeAssetGraphTooltip::GetTooltipNodeName)
                .ColorAndOpacity(FLinearColor::White)
            ]

            // Row 2: Node ID
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 2.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Node ID: ")))
                    .ColorAndOpacity(FLinearColor::Gray)
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(CastNode->GetNodeId().ToString(EGuidFormats::Short)))
                    .ColorAndOpacity(FLinearColor::Gray)
                ]
            ]

            // Row 3: Node Stat ID
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0.0f, 2.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TEXT("Stat ID: ")))
                    .ColorAndOpacity(FLinearColor::Gray)
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(STextBlock)
                    .Text(this, &SUpgradeAssetGraphTooltip::GetTooltipStatId)
                    .ColorAndOpacity(FLinearColor(0.4f, 0.8f, 1.0f))
                ]
            ]
        ]
    ];
}

FText SUpgradeAssetGraphTooltip::GetTooltipNodeName() const {
    if (BoundNode.IsValid() && BoundNode->GetAttachedData()) {
        return BoundNode->GetAttachedData()->nodeName;
    }
    return FText::FromString(TEXT("No Data"));
}

FText SUpgradeAssetGraphTooltip::GetTooltipStatId() const {
    if (BoundNode.IsValid() && BoundNode->GetAttachedData()) {
        return FText::FromString(BoundNode->GetAttachedData()->StatDat.statId);
    }
    return FText::FromString(TEXT("None"));
}