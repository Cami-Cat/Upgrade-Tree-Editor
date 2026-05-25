#include "Utils/UpgradeAssetDragDrop.h"
#include "App/UpgradeAssetEditorApp.h"
#include "Graph/Node/UpgradeAssetGraphNode.h"
#include "UpgradeNodeDataAsset.h"

// This ensures that we can only interact with this slate overlay IF we're currently dragging an asset.
EVisibility SUpgradeGraphCanvasDropOverlay::GetVisibility() const {
    // So here we check the drag-drop operation.
    TSharedPtr<FDragDropOperation> DragDropOp = FSlateApplication::Get().GetDragDroppingContent();
    // Here we validate, does it exist and is it an asset we're carrying?
    if (DragDropOp.IsValid() && DragDropOp->IsOfType<FAssetDragDropOp>()){
        // We then create an operation
        TSharedPtr<FAssetDragDropOp> AssetOp = StaticCastSharedPtr<FAssetDragDropOp>(DragDropOp);
        // Get the assets
        for (const FAssetData& AssetData : AssetOp->GetAssets()){
            // If the asset is a TSubclassOf (or UUpgradeNodeDataAsset) then we're fine to proceed, and we allow interactions.
            if (AssetData.GetClass() && AssetData.GetClass()->IsChildOf(UUpgradeNodeDataAsset::StaticClass())){
                // Force interactions with visibility.
                return EVisibility::Visible; 
            }
        }
    } 
    // Completely invisible to click-intercepts otherwise. Standard grid selection passes straight through!
    return EVisibility::HitTestInvisible; 
}

// Here we'll override the on-enter functionality of the grid. This needs to be a void, as is the parent function.
void SUpgradeGraphCanvasDropOverlay::OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
    // Make sure our drag-drop op is valid as always.
    TSharedPtr<FDragDropOperation> DragDropOp = DragDropEvent.GetOperation();
    if (DragDropOp.IsValid() && DragDropOp->IsOfType<FAssetDragDropOp>()) {
        // Create our asset drag drop.
        TSharedPtr<FAssetDragDropOp> AssetOp = StaticCastSharedPtr<FAssetDragDropOp>(DragDropOp);
        // And create a neat little tooltip.
        AssetOp->SetToolTip(
            FText::FromString("Release to Create Upgrade Node"),
            // We get a little tick box if we do this.
            FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.OK"))
        );
        return;
    }
    return;
}

// We keep the function going, through this.
FReply SUpgradeGraphCanvasDropOverlay::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
    return FReply::Handled();
}

// Default to whatever the drag and drop operation actually wants.
void SUpgradeGraphCanvasDropOverlay::OnDragLeave(const FDragDropEvent& DragDropEvent) {
    TSharedPtr<FDragDropOperation> DragDropOp = DragDropEvent.GetOperation();
    if (DragDropOp.IsValid() && DragDropOp->IsOfType<FAssetDragDropOp>()) {
        StaticCastSharedPtr<FAssetDragDropOp>(DragDropOp)->ResetToDefaultToolTip();
    }
}

// Create all nodes that we've dragged safely.
FReply SUpgradeGraphCanvasDropOverlay::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
    // Ensure validity.
    TSharedPtr<FDragDropOperation> DragDropOp = DragDropEvent.GetOperation();
    if (!DragDropOp.IsValid() || !BoundGraph) return FReply::Unhandled();

    // Get the operation and validate that as-well.
    TSharedPtr<FAssetDragDropOp> AssetOp = StaticCastSharedPtr<FAssetDragDropOp>(DragDropOp);
    if (!AssetOp.IsValid()) return FReply::Unhandled();

    // Turn our mouse coordinates into local coordinates on the grid. 
    FVector2D LocalDropPos = MyGeometry.AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition());

    FVector2D GraphViewOffset = FVector2D::ZeroVector;
    float GraphZoom = 1.0f;

    if (TSharedPtr<UpgradeAssetEditorApp> PinApp = WeakApp.Pin()) {
        if (TSharedPtr<SGraphEditor> GraphPanel = PinApp->GraphEditorWidget) {
            GraphPanel->GetViewLocation(GraphViewOffset, GraphZoom);
        }
    }

    // Here we'll create an offset and drop it exactly where our mouse is, relative to the zoom on the grid.
    FVector2D GraphDropPos = (LocalDropPos / GraphZoom) + GraphViewOffset;

    // We create this just for checking.
    bool bDroppedAnyValidAssets = false;

    // We can now loop over all of the asset data in our drag-drop-operations asset list.
    for (const FAssetData& AssetData : AssetOp->GetAssets()) {
        // We'll cast the asset to our UUpgradeNodeDataAsset, and if it's invalid we'll just move onto the next object.
        UUpgradeNodeDataAsset* LoadedAsset = Cast<UUpgradeNodeDataAsset>(AssetData.GetAsset());
        if (!LoadedAsset) continue;

        // If we havent dropped any assets, we'll drop it and modify the graph. We'll also update bDroppedAnyValidAssets and begin our transaction here, since this is the starting point.
        if (!bDroppedAnyValidAssets) {
            GEditor->BeginTransaction(TEXT("UpgradeGraphEditor"), FText::FromString("Drop Upgrade Asset"), BoundGraph);
            BoundGraph->Modify();
            bDroppedAnyValidAssets = true;
        }
        // We'll now create a new ui node using our factory.
        UUpgradeAssetGraphNode* NewUiNode = NewObject<UUpgradeAssetGraphNode>(BoundGraph, UUpgradeAssetGraphNode::StaticClass(), NAME_None, RF_Transactional);
        // Force set the position to where our mouse was.
        NewUiNode->NodePosX = GraphDropPos.X;
        NewUiNode->NodePosY = GraphDropPos.Y;
        NewUiNode->SetNodeId(FGuid::NewGuid());
        NewUiNode->SetAttachedData(LoadedAsset);
        // We'll now add a new node to our graph that we got through our constructor.
        BoundGraph->AddNode(NewUiNode, true, true);
        // And we'll force-redraw the node.
        NewUiNode->ReconstructNode();
        // We'll also add a little offset to the drop, for any future assets.
        GraphDropPos.X += 40;
        GraphDropPos.Y += 40;
    }
    // If we've dropped any.
    if (bDroppedAnyValidAssets) {
        // We can now end our transaction and go out of scope.
        GEditor->EndTransaction();
        BoundGraph->NotifyGraphChanged();
        return FReply::Handled();
    }
    // If nothing happened, we didn't handle it.
    return FReply::Unhandled();
}