#pragma once

#include "DragAndDrop/AssetDragDropOp.h"
#include "Input/DragAndDrop.h"

class SUpgradeGraphCanvasDropOverlay : public SLeafWidget
{
public:
    // Here we can define our own arguments for our constructor. Since we're a slate widget, this is how we do that.
    SLATE_BEGIN_ARGS(SUpgradeGraphCanvasDropOverlay) {}
        SLATE_ARGUMENT(UEdGraph*, GraphToEdit)
        SLATE_ARGUMENT(TWeakPtr<class UpgradeAssetEditorApp>, AppContext)
    SLATE_END_ARGS()
    
    // We need to store some little doohickeys here for access in our Drag and Drop operations.
    UEdGraph* BoundGraph = nullptr;
    TWeakPtr<class UpgradeAssetEditorApp> WeakApp; 

    void Construct(const FArguments& InArgs){
        BoundGraph = InArgs._GraphToEdit;
        WeakApp = InArgs._AppContext;
    }

    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
    virtual int32 OnPaint(const FPaintArgs&, const FGeometry&, const FSlateRect&, FSlateWindowElementList&, int32 LayerId, const FWidgetStyle&, bool) const override { return LayerId; }

    virtual EVisibility GetVisibility() const;
    virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
    virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
    virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
    virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
};